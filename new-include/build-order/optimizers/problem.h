#ifndef OPTIMIZERS_PROBLEM_H
#define OPTIMIZERS_PROBLEM_H

#include <build-order/data/matrix/sparse-row.h>
#include <build-order/solution.h>

#include "objective.h"
#include "restriction.h"

namespace BuildOrderOptimizer::Optimizers {

struct Problem {
    enum ResourceCategory
    {
        RC_USABLE = 0,
        RC_QUANTITY = 1,
        RC_USED = 2,
    };

    bool time_as_objective;
    unsigned maximum_time;
    Data::Matrix::SparseRow<Restriction> restrictions[3];
    Data::Matrix::SparseRow<Objective> objectives[3];

    Problem() : time_as_objective(true), maximum_time(0) {}

    virtual bool dominates(Solution a, Solution b) const;
    bool valid(Solution s) const;
    unsigned producesMaximize(unsigned task) const;
    unsigned producesGreaterThan(unsigned task) const;
    Population nonDominated(Population p) const;
    unsigned numberObjectives() const;
    unsigned numberRestrictions() const;

    Data::Contiguous<unsigned> toVector(Solution) const;
    Data::Contiguous<Data::Contiguous<unsigned>> toVector(Population) const;

    Data::Contiguous<double> toDVector(Solution) const;
    Data::Contiguous<Data::Contiguous<double>> toDVector(Population) const;

    Data::Contiguous<bool> objectivesVector() const;
};

} // namespace BuildOrderOptimizer::Optimizers

#endif
