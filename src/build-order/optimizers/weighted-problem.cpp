#include <build-order/optimizers/weighted-problem.h>

bool
BuildOrderOptimizer::Optimizers::WeightedProblem::dominates(Solution a, Solution b) const {
    double a_value = 0, b_value = 0;
    unsigned w = 0;
    if (time_as_objective) {
        a_value += weights[w] * a.final_state.time;
        b_value += weights[w] * b.final_state.time;
        w++;
    }

    for (unsigned i = 0; i < objectives[0].row.size(); i++) {
        unsigned index = objectives[0].row[i].index;

        a_value += weights[w] * a.final_state.resources[index].usable();
        b_value += weights[w] * b.final_state.resources[index].usable();
        w++;
    }

    for (unsigned i = 0; i < objectives[1].row.size(); i++) {
        unsigned index = objectives[1].row[i].index;

        a_value += weights[w] * a.final_state.resources[index].quantity;
        b_value += weights[w] * b.final_state.resources[index].quantity;
        w++;
    }

    for (unsigned i = 0; i < objectives[2].row.size(); i++) {
        unsigned index = objectives[2].row[i].index;

        a_value += weights[w] * a.final_state.resources[index].used;
        b_value += weights[w] * b.final_state.resources[index].used;
        w++;
    }

    return a_value > b_value;
}
