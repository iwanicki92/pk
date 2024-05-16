#include <iostream>
#include <sstream>
#include <format>
#include <bit>
#include <stdexcept>
#include <chrono>
#include <cmath>
#include <omp.h>

#include "utils.h"

class Keccak {
public:
    static constexpr int ArraySize = 5;
    static constexpr int MessageBlockLength = 20;

    using Type = uint16_t;
    using Row = std::array<Type, ArraySize>;
    using State = std::array<Row, ArraySize>;
    using Hash = std::array<uint8_t, 16>;
    using Message = std::vector<char>;
    using MessageBlock = std::array<char, MessageBlockLength>;

protected:
    State a{};
    std::vector<Type> r;
private:
    uint8_t round = 0;
    bool logging = false;

    void step1() {
        // Step θ
        Row c{};
        Row d{};

        for (int64_t i = 0; i < ArraySize; ++i)
        {
            for (auto a_val : this->a[i])
            {
                c[i] ^= a_val;
            }
        }
        for (auto i = 0; i < ArraySize; ++i)
        {
            d[i] = c[mod(i-1, ArraySize)] ^ (std::rotl<uint16_t>(c[(i+1) % ArraySize], 1));
        }
        for (auto i = 0; i < ArraySize; ++i)
        {
            for (auto j = 0; j < ArraySize; ++j)
            {
                a[i][j] ^= d[i];
            }
        }
    }

    void step2() {
        // step ρ
        for (auto i = 0; i < ArraySize; ++i)
        {
            for (auto j = 0; j < ArraySize; ++j)
            {
                a[i][j] = std::rotl<uint16_t>(a[i][j], (7*i + j) % 16);
            }
        }
    }

    State step3() {
        // step π
        State b{};

        for (auto i = 0; i < ArraySize; ++i)
        {
            for (auto j = 0; j < ArraySize; ++j)
            {
                b[(3*i + 2*j) % ArraySize][i] = a[i][j];
            }
        }
        return b;
    }

    void step4(const State &b) {
        // step χ
        for (auto i = 0; i < ArraySize; ++i)
        {
            for (auto j = 0; j < ArraySize; ++j)
            {
                a[i][j] = b[i][j] ^ (~b[(i+1) % ArraySize][j] & b[(i+2) % ArraySize][j]);
            }
        }
    }

    void step5() {
        // step ι
        a[0][0] ^= r[round++];
    }

    void nextRound()
    {
        step1();
        step2();
        State b = step3();
        step4(b);
        step5();
    }

    auto base_function() {
        while (round < r.size())
        {
            nextRound();
        }

        Hash hash;
        for (auto i = 0; i < 5; ++i)
        {
            hash[2*i] = (a[0][i] >> 8) & 0xff;
            hash[2*i + 1] = a[0][i] & 0xff;
        }
        return hash;
    }

    void padding(Message& message) {
        uint8_t padding_size = MessageBlockLength - (message.size() % MessageBlockLength);
        for (auto i = 0; i < padding_size; ++i)
        {
            message.push_back(padding_size);
        }
    }

public:
    Keccak(std::vector<Type> r) : r(r) {}
    Keccak() : Keccak({0x3EC2, 0x738D, 0xB119, 0xC5E7, 0x86C6, 0xDC1B, 0x57D6, 0xDA3A, 0x7710, 0x9200}) {}

    auto digest(Message message)
    {
        padding(message);
        a = State{};
        Hash hash;
        for (auto iter = message.begin(); iter < message.end(); iter += MessageBlockLength)
        {
            round = 0;
            auto message_block = Message(iter, iter + MessageBlockLength);
            for (auto i = 0; i < 5; ++i)
            {
                a[0][i] ^= (message_block[2*i] << 8) | message_block[2*i+1];
                a[1][i] ^= (message_block[2*(i+5)] << 8) | message_block[2*(i+5)+1];
            }
            hash = base_function();
        }
        round = 0;
        auto hash2 = base_function();
        for (int i = 10; i < 16; ++i)
        {
            hash[i] = hash2[i-10];
        }
        return hash;
    }
};

Keccak::Message findMessageFromHash(Keccak::Hash hash, uint8_t message_length)
{
    static constexpr std::array possible_characters{
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', 'a', 's',
        'd', 'f', 'g', 'h', 'j', 'k', 'l', 'z', 'x', 'c', 'v', 'b',
        'n', 'm', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
        'A', 'S', 'D', 'F', 'G', 'H', 'J', 'L', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        '0', '!', '@', '#', '%', '^', '-', '_', '=', '+', '(',
        '[', '{', '<', ')', ']', '}', '>', ' '};

    auto perm = PermutationWithRepetition(possible_characters, message_length);
    Keccak keccak = Keccak();

    std::cout << "Searching for message of length " << std::dec << (int)message_length << " with hash:\n";
    std::cout << std::hex << hash;
    std::cout << "Number of possible permutations: " << std::dec << perm.getNumberOfPermutations() << "\n";
    std::cout << "Progress:";
    auto no_permutations = perm.getNumberOfPermutations();
    auto perm_per_percent = static_cast<uint64_t>(no_permutations / 100);
    int percent = 0;
    Keccak::Message found_message;

    #pragma omp parallel default(none) shared(perm,found_message,hash,percent,std::cout) \
            private(keccak) firstprivate(perm_per_percent,no_permutations)
    #pragma omp for
    for (uint64_t i = 0; i < no_permutations; ++i) {
        // Display progress percentage.
        if ((i % perm_per_percent) == 0) {
            int current_percent;
            #pragma omp atomic read
            current_percent = percent;
            auto percent_str = std::string(" ") + std::to_string(current_percent) + "%";
            percent_str += std::string(percent_str.length(), '\b');
            #pragma omp critical
            {
                #pragma omp atomic update
                ++percent;
                std::cout << percent_str << std::flush;
            }
        }

        Keccak::Message message;
        // TODO: remove this critical section, for more than a couple threads it makes
        // this loop slower than a single thread version.
        #pragma omp critical
        message = perm.nextPermutation();
        auto message_hash = keccak.digest(message);
        if (message_hash == hash) {
            found_message = std::move(message);
            #pragma omp cancel for
        }
    }

    std::cout << "\n" << std::flush;
    return found_message;
}

Keccak::Message createMessage(std::string str) {
    Keccak::Message msg;
    for (char character : str) {
        msg.push_back(character);
    }
    return msg;
}

/// convert hash in string format to Hash class
Keccak::Hash strToHash(std::string hash) {
    std::istringstream ss(hash);
    std::string byte;
    Keccak::Hash converted_hash;
    int i = 0;
    while (ss >> byte) {
        if (byte.length() > 2) {
            throw std::runtime_error(std::format("Couldn't parse {}! Each hash byte should have at most 2 hex characters", byte));
        }
        converted_hash[i] = std::stoul(byte, nullptr, 16);
        ++i;
    }
    return converted_hash;
}

void printDigest(std::string str) {
    log(str, Keccak().digest(createMessage(str)));
}

bool test(std::string str, Keccak::Hash hash) {
    if (Keccak().digest(createMessage(str)) != hash) {
        std::cout << str.substr(0, 50) << " hash is wrong!\n";
        return false;
    }
    return true;
}

bool testKeccak() {
    using Hash = Keccak::Hash;
    auto str_repeat = [](unsigned char character, size_t repeat) {
        std::string a_str;
        for (size_t i = 0; i < repeat; ++i) {
            a_str.push_back(character);
        }
        return a_str;
    };

    bool tests_ok = true;
    tests_ok = tests_ok && test("",                                                Hash{0xDB,0x90,0xA9,0x14,0x6C,0xBA,0x12,0x9B,0xF1,0x28,0xD5,0xAC,0xDA,0xA3,0x3B,0xB7});
    tests_ok = tests_ok && test("AbCxYz",                                          Hash{0x3C,0x4F,0x6F,0x30,0xD3,0xB2,0xE2,0xE2,0xFF,0x4E,0x1C,0xE6,0xCA,0xE8,0x16,0x37});
    tests_ok = tests_ok && test("1234567890",                                      Hash{0x62,0xC0,0x15,0xE2,0x0F,0x2E,0xBA,0x6D,0x71,0x48,0xBC,0xB7,0x81,0xF8,0xA8,0xB5});
    tests_ok = tests_ok && test("Ala ma kota, kot ma ale.",                        Hash{0xD0,0xD0,0xE3,0xEE,0x20,0xAF,0xAE,0xB6,0xA1,0xC0,0x9E,0xED,0xBE,0x8B,0x3C,0x63});
    tests_ok = tests_ok && test("Ty, ktory wchodzisz, zegnaj sie z nadzieja.",     Hash{0x61,0x44,0x94,0xCF,0xFE,0x9B,0xEB,0xCB,0xDC,0x72,0xCE,0x38,0x59,0x31,0x0B,0x18});
    tests_ok = tests_ok && test("Litwo, Ojczyzno moja! ty jestes jak zdrowie;",    Hash{0x45,0x00,0x59,0x13,0x9E,0xCF,0x79,0x4A,0x60,0x24,0xBE,0x80,0xA7,0xF0,0x10,0x8E});
    tests_ok = tests_ok && test(str_repeat('a', 48000),                            Hash{0x79,0x41,0xBC,0xD5,0xB9,0xE3,0xF5,0xE9,0x7F,0x41,0xC0,0x63,0x6D,0xD8,0x53,0x96});
    tests_ok = tests_ok && test(str_repeat('a', 48479),                            Hash{0x0D,0x84,0x4A,0xA5,0xD5,0x0C,0xBC,0xE8,0xD2,0x0A,0xCA,0x50,0x58,0x3E,0xF5,0x03});
    tests_ok = tests_ok && test(str_repeat('a', 48958),                            Hash{0x32,0x1F,0x03,0x48,0x91,0xC7,0x35,0x56,0x4D,0x78,0xA9,0x4E,0x01,0xF1,0xF1,0xBE});
    return tests_ok;
}

void findMessages() {
    using Hash = Keccak::Hash;
    auto keccak = Keccak();
    std::array hashes = {
        Hash{0x50,0xF9,0x10,0x74,0xB8,0x57,0xFB,0x7E,0x64,0x8F,0x7C,0xC4,0x31,0xDC,0x5F,0x8A},
        Hash{0x0E,0x39,0xA2,0x16,0xC8,0x34,0xA0,0x1F,0x6C,0x7D,0x69,0x90,0xEE,0xAA,0xE4,0xBE},
        Hash{0xF1,0x6B,0x29,0x69,0xFB,0xDE,0xB8,0xED,0x36,0xD8,0x50,0x67,0xEC,0xB9,0xB9,0x9F},
        Hash{0xF6,0x62,0x77,0x1A,0x39,0x4C,0x3E,0x63,0xA1,0x05,0xF8,0x87,0x78,0xBD,0xA0,0x5F},
        Hash{0x98,0xC3,0xCA,0x3D,0x8B,0x43,0x99,0x5C,0xE1,0xA2,0x07,0xEF,0xCA,0xE5,0x88,0x1E},
        Hash{0xC2,0x70,0x1C,0x6A,0x6A,0x6C,0x18,0x0F,0x23,0x69,0x36,0x28,0x9C,0xDB,0x6A,0x73},
        Hash{0x33,0xFE,0x44,0x57,0xC9,0xFD,0xB4,0xF6,0x2B,0x80,0xA7,0x76,0xBC,0xEA,0x4C,0xE8}
    };

    for (int i = 0; i < 3; ++i) {
        auto begin = std::chrono::steady_clock::now();
        auto message = findMessageFromHash(hashes[i], i + 2);
        auto end = std::chrono::steady_clock::now();

        std::cout << "\nElapsed time: " << std::chrono::duration<float>(end - begin) << ". ";

        if (message.empty())
            std::cout << "Couldn't find message of length " << i << "\n";
        else {
            std::string msg;
            for (auto character : message) {
                msg.push_back(character);
            }

            std::cout << "Message of length " << i + 2 << " is: " << msg << "\n\n";
        }
    }
}

void printHelp() {
    std::cout << "Usage:\n";
    std::cout << "1. ./sha3\n";
    std::cout << "2. ./sha3 message\n";
    std::cout << "3. ./sha3 message_length message_hash\n";
    std::cout << "4. ./sha3 (-h|--help)\n\n";
    std::cout << "1. Find messages from built-in hashes.\n";
    std::cout << "2. Calculate hash of <message>.\n";
    std::cout << "3. Find message of length <message_length> from <message_hash>.\n";
    std::cout << "4. Print this help.\n";
}

int main(int argc, char** argv) {
    /* Liczba znaków + jeden w teście!*/
    if (!testKeccak())
        return -1;

    auto keccak = Keccak();

    if (argc == 3) {
        size_t message_length = std::stoul(argv[1]);
        auto hash = strToHash(argv[2]);
        std::cout << "Message: " << findMessageFromHash(hash, message_length) << "\n";
    }
    else if (argc == 2) {
        std::string message(argv[1]);
        if (message.starts_with("-h") || message.starts_with("--help")) {
            printHelp();
            return 0;
        }
        std::cout << "'" << message << "' hash:\n" << std::hex
            << keccak.digest(std::vector(message.begin(), message.end()));
    }
    else if (argc == 1) {
        findMessages();
    }
    else {
        std::cout << "Wrong number of arguments!\n";
        printHelp();
    }
}
