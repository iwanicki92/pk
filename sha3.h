#pragma once
#include "utils.h"

template <uint8_t MessageLength>
class Keccak {
public:
    static constexpr int ArraySize = 5;
    static constexpr int MessageBlockLength = 20;
    static constexpr int MaxRounds = 10;
    static constexpr int PaddingSize = MessageBlockLength - (MessageLength % MessageBlockLength);

    using Type = uint16_t;
    using Row = std::array<Type, ArraySize>;
    using State = std::array<Row, ArraySize>;
    using Message = std::array<uint8_t, MessageLength>;
    using MessageBlock = std::array<uint8_t, MessageBlockLength>;
    using PaddedMessage = std::array<uint8_t, MessageLength + PaddingSize>;

protected:
    State a{};
    std::array<Type, MaxRounds> r;
    uint8_t round = 0;

public:
    Keccak(std::array<Type, MaxRounds> r);
    Keccak();

    /// @brief Calculate hash
    /// @param message Message to calculate hash for
    /// @return Hash of message
    Hash digest(Message message);
protected:
    /// @brief Step θ
    inline void step1();

    /// @brief step ρ
    inline void step2();

    /// @brief step π
    /// @return b
    inline State step3();

    /// @brief step χ
    /// @param b b
    inline void step4(const State &b);

    /// @brief step ι
    inline void step5();

    /// @brief Calls all steps
    inline void nextRound();

    inline Hash base_function();

    PaddedMessage padding(const Message& message) const;
};