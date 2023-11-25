#ifndef RULES_RESOURCE_H
#define RULES_RESOURCE_H

#include <build-order/data/matrix/sparse-row.h>

namespace BuildOrderOptimizer::Rules {

struct Resource {
    unsigned overall_maximum;

    Data::Matrix::SparseRow<unsigned> maximum_per_resource;
    Data::Matrix::SparseRow<unsigned> equivalence;
};

} // namespace BuildOrderOptimizer::Rules

#endif
