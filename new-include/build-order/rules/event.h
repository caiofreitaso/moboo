#ifndef RULES_EVENT_H
#define RULES_EVENT_H

#include <build-order/data/matrix/sparse-row.h>

namespace BuildOrderOptimizer::Rules {

struct Event {
    unsigned time;
    Data::Matrix::SparseRow<unsigned> bonus;
    unsigned trigger;
};

} // namespace BuildOrderOptimizer::Rules

#endif
