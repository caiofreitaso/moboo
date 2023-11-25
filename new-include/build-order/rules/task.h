#ifndef RULES_TASK_H
#define RULES_TASK_H

#include <build-order/data/matrix/sparse-row.h>

namespace BuildOrderOptimizer::Rules {

struct Task {
    unsigned time;
    Data::Matrix::SparseRow<int> costs;

    Data::Matrix::SparseRow<unsigned> prerequisite;
    Data::Matrix::SparseRow<unsigned> consume;
    Data::Matrix::SparseRow<unsigned> borrow;

    Data::Matrix::SparseRow<unsigned> produce;
};

} // namespace BuildOrderOptimizer::Rules

#endif
