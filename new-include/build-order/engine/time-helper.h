#ifndef ENGINE_TIME_HELPER_H
#define ENGINE_TIME_HELPER_H

#include <build-order/data/contiguous.h>

namespace BuildOrderOptimizer::Engine {
struct time_helper {
    unsigned pointer;
    unsigned time_remaining;

    time_helper() : pointer(0), time_remaining(0) {}
    time_helper(unsigned p, unsigned t) : pointer(p), time_remaining(t) {}
};

typedef Data::Contiguous<time_helper> RemainingList;

} // namespace BuildOrderOptimizer::Engine
#endif
