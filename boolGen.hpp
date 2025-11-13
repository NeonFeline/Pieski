#pragma once
#include <random>
#include <iostream>

class BooleanGenerator {
public:
    BooleanGenerator(double probability = 0.5)
        : dist(probability), rng(std::random_device{}()), probDist(0.6f, 1.0f) {}

    bool next() {
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