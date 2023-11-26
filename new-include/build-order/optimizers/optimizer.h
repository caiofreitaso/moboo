#ifndef OPTIMIZERS_OPTIMIZER_H
#define OPTIMIZERS_OPTIMIZER_H

#include <build-order/archivers/archiver.h>
#include <build-order/data/contiguous.h>
#include <build-order/data/matrix/sparse-row.h>
#include <build-order/solution.h>
#include <build-order/state/game-state.h>

#include "neighboorhoods.h"
#include "problem.h"

namespace BuildOrderOptimizer::Optimizers {

class Optimizer {
    Data::Contiguous<double> _objV;
    Data::Contiguous<Data::Matrix::SparseRow<double>> _incV;
    Data::Contiguous<Data::Matrix::SparseRow<double>> _decV;

  public:
    Problem problem;
    double stop_chance;
    Neighborhood neighborhood;
    Archivers::ArchiverFactory createArchiver;

    Optimizer() : stop_chance(1), neighborhood(one_swap) {}

    virtual Population optimize(State::GameState, unsigned) const = 0;

    void update();

    Data::Contiguous<double> initialMap(double, double, State::GameState) const;
};
} // namespace BuildOrderOptimizer::Optimizers
#endif
