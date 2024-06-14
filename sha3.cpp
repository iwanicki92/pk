#include <iostream>
#include <sstream>
#include <format>
#include <bit>
#include <chrono>
#include <omp.h>

#include "sha3.h"

template <size_t StartSize, size_t N>
void findMessages(const std::array<Hash, N>& hashes);

bool testKeccak();

void printHelp();

int main(int argc, char** argv) {
    if (!testKeccak())
        return -1;

    constexpr std::array hashes = {
        Hash{0x50,0xF9,0x10,0x74,0xB8,0x57,0xFB,0x7E,0x64,0x8F,0x7C,0xC4,0x31,0xDC,0x5F,0x8A},
        Hash{0x0E,0x39,0xA2,0x16,0xC8,0x34,0xA0,0x1F,0x6C,0x7D,0x69,0x90,0xEE,0xAA,0xE4,0xBE},
        Hash{0xF1,0x6B,0x29,0x69,0xFB,0xDE,0xB8,0xED,0x36,0xD8,0x50,0x67,0xEC,0xB9,0xB9,0x9F},
        Hash{0xF6,0x62,0x77,0x1A,0x39,0x4C,0x3E,0x63,0xA1,0x05,0xF8,0x87,0x78,0xBD,0xA0,0x5F},
        Hash{0x98,0xC3,0xCA,0x3D,0x8B,0x43,0x99,0x5C,0xE1,0xA2,0x07,0xEF,0xCA,0xE5,0x88,0x1E},
        Hash{0xC2,0x70,0x1C,0x6A,0x6A,0x6C,0x18,0x0F,0x23,0x69,0x36,0x28,0x9C,0xDB,0x6A,0x73},
        Hash{0x33,0xFE,0x44,0x57,0xC9,0xFD,0xB4,0xF6,0x2B,0x80,0xA7,0x76,0xBC,0xEA,0x4C,0xE8}
    };

    if (argc == 1) {
        findMessages<2>(hashes);
    }
    else if (argc == 2) {
        std::string message(argv[1]);
        if (message.starts_with("-h") || message.starts_with("--help")) {
            printHelp();
            return 0;
        }
        else {
            std::cout << "Unknown argument!\n";
            printHelp();
        }
    }
    else {
        std::cout << "Wrong number of arguments!\n";
        printHelp();
    }
}

template <std::size_t N>
constexpr std::array<uint8_t, N - 1> createMessage(const char (&str)[N]) {
    std::array<uint8_t, N - 1> message;
    std::copy(str, str + N - 1, message.begin());
    return message;
}

template <uint8_t MessageLength>
constexpr bool test(const char (&str)[MessageLength], Hash hash) {
    auto message = createMessage(str);
    if (Keccak<MessageLength - 1>().digest(message) != hash) {
        std::cout << std::string(str).substr(0, 50) << " hash is wrong!\n";
        return false;
    }
    return true;
}

/// @brief Test if Keccak class calculates correct hashes.
/// @return true if tests passed else false
bool testKeccak() {
    bool tests_ok =
        test("",
            Hash{0xDB,0x90,0xA9,0x14,0x6C,0xBA,0x12,0x9B,0xF1,0x28,0xD5,0xAC,0xDA,0xA3,0x3B,0xB7})
        &
        test("AbCxYz",
            Hash{0x3C,0x4F,0x6F,0x30,0xD3,0xB2,0xE2,0xE2,0xFF,0x4E,0x1C,0xE6,0xCA,0xE8,0x16,0x37})
        &
        test("1234567890",
            Hash{0x62,0xC0,0x15,0xE2,0x0F,0x2E,0xBA,0x6D,0x71,0x48,0xBC,0xB7,0x81,0xF8,0xA8,0xB5})
        &
        test("Ala ma kota, kot ma ale.",
            Hash{0xD0,0xD0,0xE3,0xEE,0x20,0xAF,0xAE,0xB6,0xA1,0xC0,0x9E,0xED,0xBE,0x8B,0x3C,0x63})
        &
        test("Ty, ktory wchodzisz, zegnaj sie z nadzieja.",
            Hash{0x61,0x44,0x94,0xCF,0xFE,0x9B,0xEB,0xCB,0xDC,0x72,0xCE,0x38,0x59,0x31,0x0B,0x18})
        &
        test("Litwo, Ojczyzno moja! ty jestes jak zdrowie;",
            Hash{0x45,0x00,0x59,0x13,0x9E,0xCF,0x79,0x4A,0x60,0x24,0xBE,0x80,0xA7,0xF0,0x10,0x8E});

    return tests_ok;
}

template <uint8_t MessageLength>
Keccak<MessageLength>::Message findMessageFromHash(Hash hash)
{
    using Message = Keccak<MessageLength>::Message;
    static constexpr std::array<uint8_t, 79> possible_characters {
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', 'a', 's',
        'd', 'f', 'g', 'h', 'j', 'k', 'l', 'z', 'x', 'c', 'v', 'b',
        'n', 'm', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
        'A', 'S', 'D', 'F', 'G', 'H', 'J', 'L', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        '0', '!', '@', '#', '%', '^', '-', '_', '=', '+', '(', '[',
        '{', '<', ')', ']', '}', '>', ' '
    };

    PermutationWithRepetition<uint8_t, possible_characters.size(), MessageLength> perm(possible_characters);
    Keccak keccak = Keccak<MessageLength>();

    std::cout << "Searching for message of length " << std::dec << (int)MessageLength << " with hash:\n";
    std::cout << std::hex << hash;
    std::cout << "Number of possible permutations: " << std::dec << perm.getNumberOfPermutations() << "\n";
    std::cout << "Progress:";
    auto no_permutations = perm.getNumberOfPermutations();
    auto perm_per_percent = static_cast<uint64_t>(no_permutations / 100);
    int percent = 0;
    Message found_message;
    bool stop = false;

    #pragma omp parallel for default(none) shared(found_message,hash,percent,std::cout,stop) \
            private(keccak) firstprivate(perm_per_percent,no_permutations,perm)
    for (uint64_t i = 0; i < no_permutations; ++i) {
        bool priv_stop;
        #pragma omp atomic read
        priv_stop = stop;
        if (priv_stop) {
            continue;
        }
        // Display progress percentage.
        if ((i % perm_per_percent) == 0) {
            int current_percent;
            #pragma omp atomic read
            current_percent = percent;
            auto percent_str = std::string(" ") + std::to_string(current_percent) + "%";
            percent_str += std::string(percent_str.length(), '\b');
            #pragma omp critical
            {
                ++percent;
                std::cout << percent_str << std::flush;
            }
        }

        Message message;
        message = perm.getPermutation(i);
        auto message_hash = keccak.digest(message);
        if (message_hash == hash) {
            #pragma omp atomic write
            stop = true;
            found_message = std::move(message);
        }
    }

    std::cout << "\n" << std::flush;
    return found_message;
}

template <uint8_t MessageLength>
void findMessage(Hash hash)
{
    if (MessageLength == 0 && Keccak<0>().digest(std::array<uint8_t, 0>{}) != hash) {
        std::cout << "Couldn't find message of length 0\n";
        return;
    }
    auto begin = std::chrono::steady_clock::now();
    auto message = findMessageFromHash<MessageLength>(hash);
    auto end = std::chrono::steady_clock::now();

    std::cout << "\nElapsed time: " << std::chrono::duration<float>(end - begin) << ". ";

    if (message.empty())
        std::cout << "Couldn't find message of length " << static_cast<int>(MessageLength) << "\n";
    else {
        std::string msg;
        for (auto character : message) {
            msg.push_back(character);
        }

        std::cout << "Message of length " << static_cast<int>(MessageLength) << " is:§" << msg << "§\n\n";
    }
}

template <size_t StartSize, size_t N>
void findMessages(const std::array<Hash, N>& hashes)
{
    [&] <std::uint8_t... Is> (std::integer_sequence<uint8_t, Is...>)
    {
        (findMessage<Is + StartSize>(hashes[Is]), ...);
    }(std::make_integer_sequence<uint8_t, N>{});
}

void printHelp() {
    std::cout << "Usage:\n";
    std::cout << "1. ./sha3\n";
    std::cout << "4. ./sha3 (-h|--help)\n\n";
    std::cout << "1. Find messages from built-in hashes.\n";
    std::cout << "4. Print this help.\n";
}

//----------- Keccak definitions --------------//

template <uint8_t MessageLength>
Keccak<MessageLength>::Keccak(std::array<Type, MaxRounds> r) : r(r) {}

template <uint8_t MessageLength>
Keccak<MessageLength>::Keccak() : Keccak({0x3EC2, 0x738D, 0xB119, 0xC5E7, 0x86C6, 0xDC1B, 0x57D6, 0xDA3A, 0x7710, 0x9200}) {}

template <uint8_t MessageLength>
Hash Keccak<MessageLength>::digest(const Message &message)
{
    auto padded_message = padding(message);
    a = State{};
    Hash hash;
    for (size_t i = 0; i < padded_message.size(); i += MessageBlockLength)
    {
        round = 0;
        for (auto j = 0; j < 5; ++j)
        {
            a[0][j] ^= (padded_message[i + 2*j] << 8) | padded_message[i + 2*j+1];
            a[1][j] ^= (padded_message[i + 2*(j+5)] << 8) | padded_message[i + 2*(j+5)+1];
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

template <uint8_t MessageLength>
void Keccak<MessageLength>::step1() {
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

template <uint8_t MessageLength>
void Keccak<MessageLength>::step2() {
    // step ρ
    for (auto i = 0; i < ArraySize; ++i)
    {
        for (auto j = 0; j < ArraySize; ++j)
        {
            a[i][j] = std::rotl<uint16_t>(a[i][j], (7*i + j) % 16);
        }
    }
}

template <uint8_t MessageLength>
Keccak<MessageLength>::State Keccak<MessageLength>::step3() {
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

template <uint8_t MessageLength>
void Keccak<MessageLength>::step4(const State &b) {
    // step χ
    for (auto i = 0; i < ArraySize; ++i)
    {
        for (auto j = 0; j < ArraySize; ++j)
        {
            a[i][j] = b[i][j] ^ (~b[(i+1) % ArraySize][j] & b[(i+2) % ArraySize][j]);
        }
    }
}

template <uint8_t MessageLength>
void Keccak<MessageLength>::step5() {
    // step ι
    a[0][0] ^= r[round++];
}

template <uint8_t MessageLength>
void Keccak<MessageLength>::nextRound()
{
    step1();
    step2();
    State b = step3();
    step4(b);
    step5();
}

template <uint8_t MessageLength>
Hash Keccak<MessageLength>::base_function() {
    while (round < MaxRounds)
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

template <uint8_t MessageLength>
Keccak<MessageLength>::PaddedMessage Keccak<MessageLength>::padding(const Message& message) const {
    PaddedMessage padded_message{};
    std::copy(message.begin(), message.end(), padded_message.begin());
    for (auto i = 0; i < PaddingSize; ++i)
    {
        padded_message[MessageLength + i] = PaddingSize;
    }
    return padded_message;
}
