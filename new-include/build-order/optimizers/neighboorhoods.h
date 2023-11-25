#ifndef OPTIMIZERS_NEIGHBOORHOODS_H
#define OPTIMIZERS_NEIGHBOORHOODS_H

#include <build-order/random/rng.h>
#include <build-order/solution.h>

namespace BuildOrderOptimizer::Optimizers {

Population delete_one(Solution const &s);
Population delete_tail(Solution const &s);
Population one_swap(Solution const &s);
Population insert(Solution const &s);
Population swap_and_delete(Solution const &s);
Population swap_and_insert(Solution const &s);

} // namespace BuildOrderOptimizer::Optimizers

#endif
