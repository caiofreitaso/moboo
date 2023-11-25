#ifndef DATA_GRAPH_METAGRAPH_H
#define DATA_GRAPH_METAGRAPH_H

#include <build-order/data/contiguous.h>
#include <build-order/data/matrix/element.h>
#include <build-order/data/matrix/sparse-row.h>

#include "edge.h"
#include "node.h"

namespace BuildOrderOptimizer::Data::Graph {

template <template <class> class C, class T>
class MetaGraph {
  public:
    typedef Node<C, T> node;
    typedef Edge<T> edge;
    typedef typename Matrix::SparseRow<Node<C, T>>::iterator iterator;
    typedef typename Matrix::SparseRow<Node<C, T>>::const_iterator const_iterator;

  protected:
    unsigned total;
    Matrix::SparseRow<node> nodes;

  public:
    MetaGraph() {}
    MetaGraph(MetaGraph const &g) : total(g.total), nodes(g.nodes) {}

    void
    insert(unsigned edge_a, unsigned edge_b, double value = 0, T param = T()) {
        auto it = nodes.find(edge_a);
        if (it == nodes.row.end() || it->index != edge_a) {
            node new_node;
            new_node.insert(edge_b, value, param);
            nodes.row.insert(it, Matrix::Element<Node<C, T>>(edge_a, new_node));
        } else {
            it->value.insert(edge_b, value, param);
        }
    }

    void
    soft_erase(unsigned edge) {
        nodes.row.erase(nodes.find(edge));
    }

    unsigned
    find(unsigned edge) const {
        auto it = nodes.find(edge);
        if (it == nodes.row.end()) {
            return size();
        }
        return it - nodes.row.begin();
    }

    node
    get(unsigned edge) const {
        return nodes.get(edge);
    }

    void
    vertices(unsigned t) {
        total = t;
    }

    unsigned
    vertices() const {
        return total;
    }

    unsigned
    size() const {
        return nodes.size();
    }

    iterator
    begin() {
        return nodes.begin();
    }

    iterator
    end() {
        return nodes.end();
    }

    const_iterator
    begin() const {
        return nodes.begin();
    }

    const_iterator
    end() const {
        return nodes.end();
    }
};

template <class C>
using Graph = MetaGraph<Matrix::SparseRow, C>;

template <class C>
using MultiGraph = MetaGraph<Matrix::Row, C>;

} // namespace BuildOrderOptimizer::Data::Graph

#endif
