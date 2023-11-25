#include <build-order/random/rng.h>

std::mt19937 *r = 0;

unsigned
BuildOrderOptimizer::Random::rng() {
    static unsigned count = 0;
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();

    if (r == 0) {
        r = new std::mt19937(seed);
    }

    if (count == r->state_size) {
        r->seed(seed);
        count = 0;
    }

    count++;
    return (*r)();
}

unsigned
BuildOrderOptimizer::Random::maximum_random() {
    return r->max();
}
