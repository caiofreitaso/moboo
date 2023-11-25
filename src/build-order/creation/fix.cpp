#include <build-order/creation/fix.h>

BuildOrderOptimizer::Creation::Multipliers BuildOrderOptimizer::Creation::MULTIPLIERS = {
    .objective = 3.0,
    .restriction = 4.0,
    .prerequisite = 2.0,
    .cost = 0.5,
    .maximum = 1.0,
};

void
BuildOrderOptimizer::Creation::make_valid(Solution &s, Optimizers::Optimizer const &solver,
                                          State::GameState const &init) {
    for (unsigned d = 0; !solver.valid(s); d++) {
        if (d > s.orders.size()) {
            s.orders.clear();
            d = 0;
        } else {
            s.orders.erase(s.orders.begin() + s.orders.size() - d, s.orders.end());
        }

        s.update(init, solver.maximum_time);

        Multipliers m = MULTIPLIERS;
        m.objective = CREATION.getObjective();
        m.restriction = CREATION.getRestriction();

        double delta_o = FIX.getDeltaO();
        double delta_r = FIX.getDeltaR();

        double max_o = m.objective * pow(delta_o, 99);
        double max_r = m.restriction * pow(delta_r, 99);

        if (max_o > max_r) {
            std::swap(max_o, max_r);
        }

        while (nextTask(s, init, solver, m)) {
            if (m.objective < max_o) {
                m.objective *= delta_o;
            }

            if (m.restriction < max_r) {
                m.restriction *= delta_r;
            }

            if (solver.valid(s)) {
                break;
            }
        }
    }
}

void
BuildOrderOptimizer::Creation::trim(Solution &s, Optimizers::Optimizer const &solver, State::GameState const &init) {
    Solution tmp = s;

    while (solver.valid(tmp)) {
        s = tmp;
        tmp.orders.pop_back();
        tmp.update(init, solver.maximum_time);
    }
}
