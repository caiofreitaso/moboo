#include <gtest/gtest.h>

#include <build-order/data/graph/node.h>
#include <build-order/data/matrix/row.h>

using namespace BuildOrderOptimizer::Data;
using namespace BuildOrderOptimizer::Data::Graph;

TEST(NodeTest, Insert_ShouldChangeEdgesSize) {
    Node<Matrix::Row, bool> a;
    auto initial_size = a.edges.size();

    a.insert(1, 0.1f, true);

    EXPECT_NE(initial_size, a.edges.size());
}

TEST(NodeTest, Insert_ShouldHaveExpectedEdge) {
    Node<Matrix::Row, bool> a;

    a.insert(1, 0.1f, true);

    Edge<bool> e(0.1f, true);

    EXPECT_EQ(e, a.edges[0].value);
}
