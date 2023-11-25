#ifndef OPTIMIZERS_FIX_H
#define OPTIMIZERS_FIX_H

#include <build-order/optimizers/optimizer.h>
#include <build-order/solution.h>
#include <build-order/state/game-state.h>

#include "parameters.h"

namespace BuildOrderOptimizer::Creation {

struct Multipliers {
    double objective;
    double restriction;
    double prerequisite;
    double cost;
    double maximum;
};

extern Multipliers MULTIPLIERS;

bool nextTask(Solution &, State::GameState, Optimizers::Optimizer const &, Multipliers);
void make_valid(Solution &, Optimizers::Optimizer const &, State::GameState const &);
void trim(Solution &, Optimizers::Optimizer const &, State::GameState const &);

} // namespace BuildOrderOptimizer::Creation

#endif
