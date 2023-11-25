#ifndef DECISION_DECISION_MAKER_H
#define DECISION_DECISION_MAKER_H

#include <build-order/data/contiguous.h>
#include <build-order/optimizers/optimizer.h>
#include <build-order/solution.h>
#include <build-order/state/game-state.h>

namespace BuildOrderOptimizer::Decision {

struct DecisionMaker {
    Optimizers::Optimizer *target;

    unsigned choose(Population p) const;
    unsigned choose(Data::Contiguous<Data::Contiguous<double>>) const;
    Solution choose(State::GameState i0, unsigned iterations) const;

    Data::Contiguous<Data::Contiguous<double>> normalize(Population p) const;

    virtual double value(Data::Contiguous<double> v) const = 0;
};

} // namespace BuildOrderOptimizer::Decision

#endif
