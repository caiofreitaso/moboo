#ifndef DATA_GRAPH_NODE_H
#define DATA_GRAPH_NODE_H

#include "edge.h"

namespace BuildOrderOptimizer::Data::Graph {

template <template <class> class C, class T>
struct Node {
    C<Edge<T>> edges;

    Node() {}
    Node(Node const &n) : edges(n.edges) {}

    Node &
    operator=(Node e) {
        std::swap(edges, e.edges);
        return *this;
    }

    bool
    operator==(Node e) const {
        if (edges.size() != e.edges.size()) {
            return false;
        }

        for (unsigned i = 0; i < edges.size(); i++) {
            if (edges[i].value != e.edges[i].value) {
                return false;
            }
        }

        return true;
    }

    bool
    operator!=(Node e) const {
        return !(*this == e);
    }

    void
    insert(unsigned vertex, double value = 0, T param = T()) {
        Edge<T> e(value, param);
        edges.set(vertex, e);
    }

    Edge<T>
    get(unsigned v) const {
        return edges.get(v);
    }

    unsigned
    size() const {
        return edges.size();
    }
};
} // namespace BuildOrderOptimizer::Data::Graph

#endif
