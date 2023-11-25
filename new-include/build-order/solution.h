#ifndef SOLUTION_H
#define SOLUTION_H

#include <build-order/data/contiguous.h>
#include <build-order/state/game-state.h>
#include <build-order/task-pointer.h>

namespace BuildOrderOptimizer {

State::GameState makespan(State::GameState, BuildOrder &original, unsigned maximum_time = 0);

struct Solution {
    BuildOrder orders;
    State::GameState final_state;

    void
    update(State::GameState initial, unsigned maximum_time = 0) {
        final_state = makespan(initial, orders, maximum_time);
    }

    bool
    operator==(Solution const &s) const {
        if (orders.size() != s.orders.size())
            return false;

        for (unsigned i = 0; i < orders.size(); i++) {
            if (orders[i].task != s.orders[i].task)
                return false;
            if (orders[i].delay != s.orders[i].delay)
                return false;
        }

        return true;
    }

    bool
    operator<(Solution const &s) const {
        return final_state.time < s.final_state.time;
    }
};

typedef Data::Contiguous<Solution> Population;

} // namespace BuildOrderOptimizer

#endif
