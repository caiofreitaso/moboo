#ifndef EVENT_POINTER_H
#define EVENT_POINTER_H

#include <build-order/data/contiguous.h>

#include <algorithm>

namespace BuildOrderOptimizer {

struct EventPointer {
    unsigned start;
    unsigned type;

    EventPointer() : start(0), type(0) {}
    EventPointer(EventPointer const &pointer) : start(pointer.start), type(pointer.type) {}
    EventPointer(unsigned start, unsigned type) : start(start), type(type) {}

    ~EventPointer() {}

    EventPointer &
    operator=(EventPointer t) {
        std::swap(start, t.start);
        std::swap(type, t.type);

        return *this;
    }
};

typedef Data::Contiguous<EventPointer> EventList;
} // namespace BuildOrderOptimizer

#endif
