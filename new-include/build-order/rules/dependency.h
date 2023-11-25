#ifndef RULES_DEPENDENCY_H
#define RULES_DEPENDENCY_H

#include <build-order/data/contiguous.h>
#include <build-order/data/matrix/sparse-row.h>

#include "relation-type.h"

namespace BuildOrderOptimizer::Rules {

struct Dependency {
    RelationType type;
    Data::Matrix::SparseRow<double> bonus;
    Data::Matrix::SparseRow<double> event;
    Data::Matrix::SparseRow<double> weight;

    bool
    operator==(Dependency d) const {
        return type == d.type;
    }
};

Data::Contiguous<Dependency> value(unsigned needed, unsigned needs);

} // namespace BuildOrderOptimizer::Rules

#endif
