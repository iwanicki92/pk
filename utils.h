#pragma once
#include <cstdint>
#include <ostream>
#include <iostream>
#include <vector>
#include <array>
#include <string>

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

template <class T, size_t SET_SIZE>
class PermutationWithRepetition {
public:
    PermutationWithRepetition(const std::array<T, SET_SIZE> &set, size_t choices) : set(set), choices(choices) {
        number_of_permutations = SET_SIZE;
        for (size_t i = 1; i < choices; ++i) {
            number_of_permutations *= SET_SIZE;
        }
    }

    std::vector<T> getPermutation(uint64_t index) {
        index = index % number_of_permutations;
        std::vector<T> permutation;
        permutation.reserve(choices);
        for (size_t i = 0; i < choices; ++i) {
            permutation.push_back(set[index % SET_SIZE]);
            index /= SET_SIZE;
        }
        return permutation;
    }

    size_t getN() const {
        return SET_SIZE;
    }

    size_t getK() const {
        return choices;
    }

    uint64_t getNumberOfPermutations() const {
        return number_of_permutations;
    }

private:
    const std::array<T, SET_SIZE> set;
    size_t choices;
    uint64_t number_of_permutations;
};