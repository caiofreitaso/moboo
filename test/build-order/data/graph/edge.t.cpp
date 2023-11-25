#include <gtest/gtest.h>

#include <build-order/data/graph/edge.h>

using namespace BuildOrderOptimizer::Data::Graph;

TEST(EdgeTest, FnOperator_ShouldReturnParam) {
    Edge<bool> a;

    a.param = true;

    EXPECT_EQ(true, a());
}

TEST(EdgeTest, EqOperator_WhenEqual_ShouldReturnTrue) {
    Edge<bool> a, b;

    a.value = 0.1f;
    b.value = 0.1f;

    a.param = true;
    b.param = true;

    EXPECT_EQ(true, a == b);
}

TEST(EdgeTest, EqOperator_WhenValueDiff_ShouldReturnFalse) {
    Edge<bool> a, b;

    a.value = 0.1f;
    b.value = 0.2f;

    a.param = true;
    b.param = true;

    EXPECT_EQ(false, a == b);
}

TEST(EdgeTest, EqOperator_WhenParamDiff_ShouldReturnFalse) {
    Edge<bool> a, b;

    a.value = 0.1f;
    b.value = 0.1f;

    a.param = true;
    b.param = false;

    EXPECT_EQ(false, a == b);
}

TEST(EdgeTest, AssignOperator_ShouldHaveSameValue) {
    Edge<bool> a, b;

    a.value = 0.1f;
    a.param = true;

    b = a;

    EXPECT_EQ(0.1f, b.value);
}

TEST(EdgeTest, AssignOperator_ShouldHaveSameParam) {
    Edge<bool> a, b;

    a.value = 0.1f;
    a.param = true;

    b = a;

    EXPECT_EQ(true, b.param);
}
