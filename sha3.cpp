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

template <size_t MessageBlockLength = 20, size_t ArraySize = 5>
class Keccak {
public:
    using Type = uint16_t;
    using Row = std::array<Type, ArraySize>;
    using State = std::array<Row, ArraySize>;
    using Hash = std::array<uint8_t, 16>;
    using Message = std::vector<uint8_t>;
    using MessageBlock = std::array<uint8_t, MessageBlockLength>;

private:
    uint8_t round = 0;
    bool logging = false;
protected:
    State a{};
    std::vector<Type> r;

    void nextRound(bool print_steps = false)
    {
        if (round >= r.size()) {
            throw std::runtime_error("Too many rounds!");
        }
        log("Base", a, print_steps);

        // Step θ
        Row c{};
        Row d{};

        for (auto i = 0; i < ArraySize; ++i)
        {
            for (auto a_val : this->a[i])
            {
                c[i] ^= a_val;
            }
        }
        for (auto i = 0; i < ArraySize; ++i)
        {
            d[i] = c[mod(i-1, ArraySize)] ^ (std::rotl(c[(i+1) % ArraySize], 1));
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
                a[i][j] = std::rotl(a[i][j], (7*i + j) % ((ArraySize-1)*(ArraySize-1)));
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

        log("After χ", a, print_steps);

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

    auto digest(Message message, bool second) {
        round = 0;
        for (auto i = 0; i < 5; ++i)
        {
            a[0][i] ^= (message[2*i] << 8) | message[2*i+1];
            a[1][i] ^= (message[2*(i+5)] << 8) | message[2*(i+5)+1];
        }
        return base_function();
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
        int i = 1;
        for (auto iter = message.begin(); iter < message.end(); iter += MessageBlockLength)
        {
            log("Iteration " + std::to_string(i++), Message(iter, iter + MessageBlockLength), logging);
            hash = digest(Message(iter, iter + MessageBlockLength), false);
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
        std::array possible_characters{
            'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', 'a', 's',
            'd', 'f', 'g', 'h', 'j', 'k', 'l', 'z', 'x', 'c', 'v', 'b',
            'n', 'm', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
            'A', 'S', 'D', 'F', 'G', 'H', 'J', 'L', 'Z', 'X', 'C', 'V',
            'B', 'N', 'M', '1', '2', '3', '4', '5', '6', '7', '8', '9',
            '0', '!', '@', '#', '%', '^', '-', '_', '=', '+', '(',
            '[', '{', '<', ')', ']', '}', '>' };

        for (uint8_t character = 30; character < 128; ++character) {
            std::cout << character << ", " << std::flush;
            for (int i = 0; i < 10; ++i) {
                Message msg;
                for (int j = 0; j < i; ++j) {
                    msg.push_back(character);
                }
                auto new_hash = digest(msg);
                if (new_hash[0] == 0xf1 && new_hash[1] == 0x6b && new_hash[2]== 0x29) {
                    std::cout << std::hex << "char: " << character << ", hash: \n" << new_hash;
                }
            }
        }
        return Message();
        for (uint8_t character = 30; character < 128; ++character)   {
            std::cout << (int)character << ", " << std::flush;
            for (uint8_t character2 = 30; character2 < 128; ++character2) {
                for(uint8_t character3 = 30; character3 < 128; ++character3) {
                    auto message = Message{character, character2, character3};
                    auto new_hash = digest(message);
                    if (new_hash[0] == 0xf1 && new_hash[1] == 0x6b && new_hash[2]== 0x29) {
                        std::cout << std::hex << "char: " << character << ", hash: \n" << new_hash;
                    }
                    if (digest(message) == hash)
                    {
                        std::cout << std::hex << message;
                        return message;
                    }
                }
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

void test(std::string str, Keccak<>::Hash hash, bool log = false) {
    if (Keccak<>(log).digest(createMessage(str)) == hash) {
        std::cout << str.substr(0, 50) << " hash is ok\n";
    }
}

int main() {
    auto message = Keccak<>::Message{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13};
    auto keccak = Keccak<>();
    auto message2 = Keccak<>::Message();
    auto message3 = Keccak<>::Message{
        'A', 'l', 'a', 'm', 'a', 'k', 'o', 't', 'a', ',', 'k', 'o', 't', 'm', 'a', 'a', 'l', 'e', '.'
    };
    //auto hash = keccak.digest(message2);
    //log("test", keccak.digest(message));
    using Hash = Keccak<>::Hash;
    test("", Hash{0xDB,0x90,0xA9,0x14,0x6C,0xBA,0x12,0x9B,0xF1,0x28,0xD5,0xAC,0xDA,0xA3,0x3B,0xB7});
    test("AbCxYz", Hash{0x3C,0x4F,0x6F,0x30,0xD3,0xB2,0xE2,0xE2,0xFF,0x4E,0x1C,0xE6,0xCA,0xE8,0x16,0x37});
    test("1234567890", Hash{0x62,0xC0,0x15,0xE2,0x0F,0x2E,0xBA,0x6D,0x71,0x48,0xBC,0xB7,0x81,0xF8,0xA8,0xB5});
    test("Alamakota, kotmaale.", Hash{0xD0,0xD0,0xE3,0xEE,0x20,0xAF,0xAE,0xB6,0xA1,0xC0,0x9E,0xED,0xBE,0x8B,0x3C,0x63}, true);
    test("Alamakota,\tkotmaale.", Hash{0xD0,0xD0,0xE3,0xEE,0x20,0xAF,0xAE,0xB6,0xA1,0xC0,0x9E,0xED,0xBE,0x8B,0x3C,0x63});
    test("Alamakota,kotmaale.", Hash{0xD0,0xD0,0xE3,0xEE,0x20,0xAF,0xAE,0xB6,0xA1,0xC0,0x9E,0xED,0xBE,0x8B,0x3C,0x63});
    test("Alamakota, kotmaale", Hash{0xD0,0xD0,0xE3,0xEE,0x20,0xAF,0xAE,0xB6,0xA1,0xC0,0x9E,0xED,0xBE,0x8B,0x3C,0x63});
    test("Alamakota,\tkotmaale", Hash{0xD0,0xD0,0xE3,0xEE,0x20,0xAF,0xAE,0xB6,0xA1,0xC0,0x9E,0xED,0xBE,0x8B,0x3C,0x63});
    test("Alamakota,kotmaale", Hash{0xD0,0xD0,0xE3,0xEE,0x20,0xAF,0xAE,0xB6,0xA1,0xC0,0x9E,0xED,0xBE,0x8B,0x3C,0x63});
    test("Litwo, Ojczyznomoja!tyjestesjakzdrowie;", Hash{0x45,0x00,0x59,0x13,0x9E,0xCF,0x79,0x4A,0x60,0x24,0xBE,0x80,0xA7,0xF0,0x10,0x8E});
    test("Litwo,Ojczyznomoja!tyjestesjakzdrowie;", Hash{0x45,0x00,0x59,0x13,0x9E,0xCF,0x79,0x4A,0x60,0x24,0xBE,0x80,0xA7,0xF0,0x10,0x8E});

    std::string a_str;
    for (int i = 0; i < 48000; ++i) {
        a_str.push_back('a');
    }
    test(a_str, Hash{0x79,0x41,0xBC,0xD5,0xB9,0xE3,0xF5,0xE9,0x7F,0x41,0xC0,0x63,0x6D,0xD8,0x53,0x96});
    //log("aaa...", keccak.digest(message2));

    /* log("found message", keccak.findMessageFromHash(
        Keccak<>::Hash{
            0x50, 0xf9, 0x10, 0x74, 0xb8, 0x57, 0xfb, 0x7e, 0x64, 0x8f, 0x7c, 0xc4, 0x31, 0xdc, 0x5f, 0x8a
            },
            1
        )
    );
    */

   keccak.findMessageFromHash(Hash{0x50,0xF9,0x10,0x74,0xB8,0x57,0xFB,0x7E,0x64,0x8F,0x7C,0xC4,0x31,0xDC,0x5F,0x8A}, 1);
}
