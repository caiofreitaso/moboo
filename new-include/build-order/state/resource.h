#ifndef STATE_RESOURCE_H
#define STATE_RESOURCE_H

#include <build-order/event-pointer.h>

namespace BuildOrderOptimizer::State {

struct Resource {
    int quantity;
    int used;
    int borrowed;
    EventList events;

    bool capacity;

    int
    usable() const {
        return quantity - used;
    }

    int
    usableB() const {
        return usable() - borrowed;
    }
};

} // namespace BuildOrderOptimizer::State

#endif
