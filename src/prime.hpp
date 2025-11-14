#pragma once
#include <bitset>
#include <cmath>

constexpr size_t MAX_PRIME = 100000;

static constexpr std::bitset<MAX_PRIME + 1> generatePrimeTable() noexcept {
    std::bitset<MAX_PRIME + 1> table;
    table.set();  // Assume all numbers are prime
    table[0] = false;
    table[1] = false;

    for (size_t i = 2; i * i <= MAX_PRIME; ++i) {
        if (table[i]) {
            for (size_t j = i * i; j <= MAX_PRIME; j += i)
                table[j] = false;
        }
    }

    return table;
}

constexpr auto prime_table = generatePrimeTable();
