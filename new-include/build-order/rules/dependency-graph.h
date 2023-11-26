#ifndef RULES_MULTIGRAPH_H
#define RULES_MULTIGRAPH_H

#include <build-order/data/graph/meta-graph.h>

#include <iostream>

#include "dependency.h"

namespace BuildOrderOptimizer::Rules {

class DependencyGraph : public Data::Graph::MultiGraph<Dependency> {};

} // namespace BuildOrderOptimizer::Rules

#endif
