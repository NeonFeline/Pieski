#pragma once
#include <random>
#include <iostream>

class BooleanGenerator {
public:
    BooleanGenerator(double start_probability = 0.5, double min_probability = 0.5)
        : dist(start_probability), rng(std::random_device{}()), probDist(min_probability, 1.0f) {}

    bool next() noexcept {
        return dist(rng);
    }

    void change_probability() {
        float newProb = probDist(rng);
        dist = std::bernoulli_distribution(newProb);
    }

private:
    std::mt19937 rng;                // fast Mersenne Twister RNG
    std::bernoulli_distribution dist; // generates true with probability
    std::uniform_real_distribution<float> probDist;
};