#ifndef OPTIMIZERS_LOCAL_SEARCH_H
#define OPTIMIZERS_LOCAL_SEARCH_H

#include "optimizer.h"

namespace BuildOrderOptimizer::Optimizers {

extern unsigned local_search_maximum;

Population local_search(Neighborhood neighborhood, Population const &p, unsigned childs, Optimizer const &opt,
                        State::GameState init);
Population local_search(Neighborhood neighborhood, Solution const &p, unsigned childs, Optimizer const &opt,
                        State::GameState init);

} // namespace BuildOrderOptimizer::Optimizers

#endif
