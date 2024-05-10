#include <iostream>
#include <array>
#include <cstdint>
#include <format>
#include <bit>
#include <vector>
#include <stdexcept>

int32_t mod(int32_t a, int32_t b) {
    return (a % b + b) % b;
}

template <class T, size_t SIZE>
std::ostream& operator<<(std::ostream& out, const std::array<T,SIZE>& row) {
    out << "[\t";
    for (const auto& item : row)
    {
        out << (long long int)item << "\t";
    }
    out << "]\n";
    return out;
}

template <class T, size_t ROW_SIZE, size_t COL_SIZE>
std::ostream& operator<<(std::ostream& out, const std::array<std::array<T,COL_SIZE>, ROW_SIZE>& array) {
    out << "[\n";
    for (const auto& row : array)
    {
        out << " " << row;
    }
    out << "]" << "\n";
    return out;
}

template <class T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& vector) {
    out << "[\t";
    for (const auto& item : vector)
    {
        out << (long long int)item << "\t";
    }
    out << "]\n";
    return out;
}

template <class T>
void log(std::string str, const T& object, bool print = true) {
    if (!print)
        return;
    std::cout << str << "\n";
    std::cout << std::hex << object;
}

template <size_t MessageBlockLength = 20, uint16_t ArraySize = 5>
class Keccak {
public:
    using Type = uint16_t;
    using Row = std::array<Type, ArraySize>;
    using State = std::array<Row, ArraySize>;
    using Hash = std::array<uint8_t, 16>;
    using Message = std::vector<uint8_t>;
    using MessageBlock = std::array<uint8_t, MessageBlockLength>;

protected:
    State a{};
    std::vector<Type> r;
private:
    uint8_t round = 0;
    bool logging = false;

    void nextRound(bool print_steps = false)
    {
        if (round >= r.size()) {
            throw std::runtime_error("Too many rounds!");
        }
        log("Base", a, print_steps);

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
        log("After θ", a, print_steps);

        // step ρ
        for (auto i = 0; i < ArraySize; ++i)
        {
            for (auto j = 0; j < ArraySize; ++j)
            {
                a[i][j] = std::rotl<uint16_t>(a[i][j], (7*i + j) % 16);
            }
        }
        log("After ρ", a, print_steps);

        // step π
        State b{};

        for (auto i = 0; i < ArraySize; ++i)
        {
            for (auto j = 0; j < ArraySize; ++j)
            {
                b[(3*i + 2*j) % ArraySize][i] = a[i][j];
            }
        }

        log("After π", a, print_steps);

        // step χ
        for (auto i = 0; i < ArraySize; ++i)
        {
            for (auto j = 0; j < ArraySize; ++j)
            {
                a[i][j] = b[i][j] ^ (~b[(i+1) % ArraySize][j] & b[(i+2) % ArraySize][j]);
            }
        }

        log("After χ", b, print_steps);

        // step ι
        a[0][0] ^= r[round++];

        log("After ι", a, print_steps);
    }

    auto base_function() {
        while (round < r.size())
        {
            //log("Round " + std::to_string(round + 1), "", logging);
            nextRound();
        }
        log("Array a after 10 rounds", a, logging);

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
    Keccak(std::vector<Type> r, bool log) : r(r), logging(log) {}
    Keccak(bool log) : Keccak({0x3EC2, 0x738D, 0xB119, 0xC5E7, 0x86C6, 0xDC1B, 0x57D6, 0xDA3A, 0x7710, 0x9200}, log) {}
    Keccak() : Keccak(false) {}

    auto digest(Message message)
    {
        log("message", message, logging);
        padding(message);
        log("padded", message, logging);
        a = State{};
        Hash hash;
        int iter_num = 1;
        for (auto iter = message.begin(); iter < message.end(); iter += MessageBlockLength)
        {
            log("Iteration " + std::to_string(iter_num++), Message(iter, iter + MessageBlockLength), logging);
            round = 0;
            auto message_block = Message(iter, iter + MessageBlockLength);
            for (auto i = 0; i < 5; ++i)
            {
                a[0][i] ^= (message_block[2*i] << 8) | message_block[2*i+1];
                a[1][i] ^= (message_block[2*(i+5)] << 8) | message_block[2*(i+5)+1];
            }
            hash = base_function();
            log("Hash", hash, logging);
        }
        round = 0;
        auto hash2 = base_function();
        log("Hash2", hash2, logging);
        for (int i = 10; i < 16; ++i)
        {
            hash[i] = hash2[i-10];
        }
        return hash;
    }

    Message findMessageFromHash(Hash hash, uint8_t message_length)
    {
        // std::array possible_characters{
        //     'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', 'a', 's',
        //     'd', 'f', 'g', 'h', 'j', 'k', 'l', 'z', 'x', 'c', 'v', 'b',
        //     'n', 'm', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
        //     'A', 'S', 'D', 'F', 'G', 'H', 'J', 'L', 'Z', 'X', 'C', 'V',
        //     'B', 'N', 'M', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        //     '0', '!', '@', '#', '%', '^', '-', '_', '=', '+', '(',
        //     '[', '{', '<', ')', ']', '}', '>' };

        for (int i = 0; i < 256; ++i) {
            unsigned char character = static_cast<unsigned char>(i);
            auto message = Message{character};
            auto message_hash = digest(message);
            std::cout << "character = " << std::hex << i << "\n" << message_hash << "\n";
            if (message_hash[0] == hash[0]) {
                //std::cout << "char: " << character << ", hash: \n" << std::hex << message_hash;
            }
            if (message_hash == hash)
            {
                std::cout << std::hex << message;
                return message;
            }
        }
        return Message();
    }
};

Keccak<>::Message createMessage(std::string str) {
    Keccak<>::Message msg;
    for (char character : str) {
        msg.push_back(character);
    }
    return msg;
}

void printDigest(std::string str) {
    log(str, Keccak<>().digest(createMessage(str)));
}

bool test(std::string str, Keccak<>::Hash hash, bool log = false) {
    if (Keccak<>(log).digest(createMessage(str)) != hash) {
        std::cout << str.substr(0, 50) << " hash is wrong!\n";
        return false;
    }
    return true;
}

bool testKeccak() {
    auto keccak = Keccak<>();
    using Hash = Keccak<>::Hash;
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

int main() {
    /* Liczba znaków + jeden w teście!*/
    if (!testKeccak())
        return -1;
}
