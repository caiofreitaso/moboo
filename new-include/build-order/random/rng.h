#ifndef RANDOM_RNG_H
#define RANDOM_RNG_H

#include <chrono>
#include <random>

namespace BuildOrderOptimizer::Random {

unsigned rng();
unsigned maximum_random();

inline unsigned
rng(unsigned max, unsigned min = 0) {
    unsigned diff = max - min + 1;
    return (rng() % diff) + min;
}

inline double
drng() {
    double r = rng();
    return r / maximum_random();
}

inline double
drng(double min, double max) {
    double diff = max - min;
    return (drng() * diff) + min;
}

} // namespace BuildOrderOptimizer::Random

#endif
