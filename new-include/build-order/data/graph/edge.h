#ifndef DATA_GRAPH_EDGE_H
#define DATA_GRAPH_EDGE_H

#include <algorithm>
#include <limits>

namespace BuildOrderOptimizer::Data::Graph {

template <class T>
struct Edge {
    mutable double value;
    T param;

    Edge() : value(std::numeric_limits<double>::quiet_NaN()) {}
    Edge(Edge const &e) : value(e.value), param(e.param) {}
    Edge(Edge &e) : value(e.value), param(e.param) {}
    Edge(double v, T p) : value(v), param(p) {}

    Edge &
    operator=(Edge e) {
        std::swap(value, e.value);
        std::swap(param, e.param);
        return *this;
    }

    bool
    operator==(Edge e) const {
        return value == e.value && param == e.param;
    }

    bool
    operator!=(Edge e) const {
        return !(*this == e);
    }

    T &
    operator()() {
        return param;
    }

    T const &
    operator()() const {
        return param;
    }
};

} // namespace BuildOrderOptimizer::Data::Graph

#endif
