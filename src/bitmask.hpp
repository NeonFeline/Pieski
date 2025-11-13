#pragma once

#include "boolGen.hpp"
#include <vector>
#include <iostream>
#include <cstring>

class Bitmask {
public:

    Bitmask(size_t elements_n) : 
        size(elements_n),
        bitmask(bitmask_size_from_elements_n(elements_n), 0) {};

    Bitmask(size_t elements_n, BooleanGenerator& gen) : 
        size(elements_n),
        bitmask(bitmask_size_from_elements_n(elements_n)) {
            for (size_t i = 0; i < bitmask.size(); i++) {
                for (size_t j = 0; j < 64; j++) {
                    if (gen.next()) bitmask[i] |= (1ULL << j);
                }
            }
        }

    void toggle(size_t element_number) noexcept {
        bitmask[element_number >> 6] ^= (1ULL << (element_number & 63));
    }

    void set(size_t element_number) noexcept {
        bitmask[element_number >> 6] |= (1ULL << (element_number & 63));
    }

    bool is_set(size_t element_number) const {
        if (element_number >= size) {
            std::cerr << element_number << " > " << size << "\n";
            throw std::runtime_error("indexing error with bitmask");
        } 
        return bitmask[element_number >> 6] & (1ULL << (element_number & 63)); 
    }

    bool operator[](size_t element_number) const noexcept {
        return bitmask[element_number >> 6] & (1ULL << (element_number & 63));
    }

    void reshuffle_bits(BooleanGenerator& gen) noexcept {
        std::memset(bitmask.data(), 0x00, bitmask.size() * sizeof(uint64_t));
        for (size_t i = 0; i < bitmask.size(); i++) {
            for (size_t j = 0; j < 64; j++) {
                if (gen.next()) bitmask[i] |= (1ULL << j);
            }
        }   
    }

    void print() const noexcept {
        for (size_t i = 0; i < bitmask.size(); i++) {
            for (size_t j = 0; j < 64; j++) {
                std::cout << bool(bitmask[i] & (1ULL << j));
            }
            std::cout << ' ';
        }
    }


private:

    const size_t size;
    std::vector<uint64_t> bitmask;

    size_t bitmask_size_from_elements_n(size_t elements_n) const noexcept {
        return (elements_n / 64) + 1;
    }
};