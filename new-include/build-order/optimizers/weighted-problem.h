#ifndef OPTIMIZERS_WEIGHTED_PROBLEM_H
#define OPTIMIZERS_WEIGHTED_PROBLEM_H

#include "problem.h"

namespace BuildOrderOptimizer::Optimizers {

struct WeightedProblem : public Problem {
    bool use_weights;
    Data::Contiguous<double> weights;

    virtual bool dominates(Solution a, Solution b) const;
};

} // namespace BuildOrderOptimizer::Optimizers
#endif
