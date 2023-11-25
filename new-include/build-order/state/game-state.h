#ifndef STATE_GAME_STATE_H
#define STATE_GAME_STATE_H

#include <build-order/event-pointer.h>

#include <fstream>
#include <iostream>
#include <sstream>

#include "game-rules.h"
#include "resource.h"

namespace BuildOrderOptimizer::State {

struct GameState {
    unsigned time;

    Data::Contiguous<Resource> resources;
    EventList tasks;

    void print() const;

    int
    usable(unsigned index) const {
        return quantity(index) - resources[index].used;
    }

    int
    usableB(unsigned index) const {
        return usable(index) - resources[index].borrowed;
    }

    static bool hasOverallMaximum(unsigned task, Data::Contiguous<unsigned> r);
    static bool hasMaximum(unsigned task, Data::Contiguous<unsigned> r);
    static unsigned maximum(unsigned index, Data::Contiguous<unsigned> res);

    bool hasPrerequisites(unsigned t) const;
    unsigned quantity(unsigned index) const;
    unsigned maximum(unsigned index) const;

    void applyEvent(unsigned i);
    void produceResource(unsigned index, unsigned quantity);
    void consumeResource(unsigned type);
};

GameState createState();

} // namespace BuildOrderOptimizer::State

#endif
