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
    PermutationWithRepetition(const std::array<T, SET_SIZE> &set, size_t elements) : set(set), elements(elements) {
        number_of_permutations = SET_SIZE;
        for (size_t i = 1; i < elements; ++i) {
            number_of_permutations *= SET_SIZE;
        }
    }

    std::vector<T> nextPermutation() {
        std::vector<T> permutation;
        permutation.reserve(elements);

        for (auto index : permutation_indices) {
            permutation.push_back(set[index]);
        }
        ++permutation_indices[0];
        end = true;
        for (size_t i = 0; i < elements; ++i) {
            if (permutation_indices[i] < SET_SIZE) {
                end = false;
                break;
            } else {
                permutation_indices[i] = 0;
                if (i < elements - 1)
                    ++permutation_indices[i+1];
            }
        }
        return permutation;
    }

    bool getEnd() const {
        return end;
    }

        size_t getN() const {
        return SET_SIZE;
    }

    size_t getK() const {
        return elements;
    }

    uint64_t getNumberOfPermutations() const {
        return number_of_permutations;
    }

private:
    const std::array<T, SET_SIZE> set;
    size_t elements;
    uint64_t number_of_permutations;
    std::vector<size_t> permutation_indices = std::vector<size_t>(elements);
    bool end = false;
};