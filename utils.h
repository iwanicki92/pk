#pragma once
#include <cstdint>
#include <ostream>
#include <iostream>
#include <vector>
#include <array>
#include <string>

using Hash = std::array<uint8_t, 16>;

template <class T1, class T2>
T2 mod(T1 a, T2 b) {
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

constexpr uint64_t power(uint64_t Base, uint64_t Exponent) {
    uint64_t pow = Base;
    for (size_t i = 1; i < Exponent; ++i) {
        pow *= Base;
    }
    return pow;
}

template <class T, size_t SET_SIZE, uint8_t CHOICES>
class PermutationWithRepetition {
public:
    constexpr PermutationWithRepetition(const std::array<T, SET_SIZE> &set) : set(set), number_of_permutations(power(SET_SIZE, CHOICES)) {}

    /// @brief Get n-th permutation
    /// @param index which permutation to return
    /// @return std::array with n-th permutation
    std::array<T, CHOICES> getPermutation(uint64_t index) {
        index = index % number_of_permutations;
        std::array<T, CHOICES> permutation;
        for (size_t i = 0; i < CHOICES; ++i) {
            permutation[i] = set[index % SET_SIZE];
            index /= SET_SIZE;
        }
        return permutation;
    }

    size_t getN() const {
        return SET_SIZE;
    }

    size_t getK() const {
        return CHOICES;
    }

    uint64_t getNumberOfPermutations() const {
        return number_of_permutations;
    }

private:
    const std::array<T, SET_SIZE> set;
    uint64_t number_of_permutations;
};
