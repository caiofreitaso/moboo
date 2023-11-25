#ifndef RULES_MULTIGRAPH_H
#define RULES_MULTIGRAPH_H

#include <build-order/data/graph/meta-graph.h>

#include <iostream>

#include "dependency.h"

namespace BuildOrderOptimizer::Rules {

class DependencyGraph : public Data::Graph::MultiGraph<Dependency> {
    virtual void
    print() const {
        for (unsigned i = 0; i < vertices(); i++) {
            std::cout << i << ":\n";
            auto it = nodes.find(i);

            if (it == nodes.row.end()) {
                continue;
            }

            for (unsigned j = 0; j < it->value.size(); j++) {
                auto current = it->value.edges[j];
                std::cout << "\t" << current.value().type << ": " << current.index << " = " << current.value.value
                          << "\n";
            }
        }
    }
};

} // namespace BuildOrderOptimizer::Rules

#endif
