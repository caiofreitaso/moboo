#include <build-order/optimizers/neighboorhoods.h>

BuildOrderOptimizer::Population
BuildOrderOptimizer::Optimizers::delete_one(Solution const &s) {
    Population new_sol;
    new_sol.push_back(s);

    unsigned index = (Random::rng() % new_sol.back().orders.size());

    Data::pop(new_sol.back().orders, index);

    return new_sol;
}

BuildOrderOptimizer::Population
BuildOrderOptimizer::Optimizers::delete_tail(Solution const &s) {
    Population new_sol;
    new_sol.push_back(s);

    unsigned index = (Random::rng() % new_sol.back().orders.size());

    new_sol.back().orders.erase(new_sol.back().orders.begin() + index, new_sol.back().orders.end());

    return new_sol;
}

BuildOrderOptimizer::Population
BuildOrderOptimizer::Optimizers::one_swap(Solution const &s) {
    Population new_sol;
    new_sol.push_back(s);

    if (s.orders.size() < 2)
        return new_sol;

    Data::Contiguous<unsigned> indexes;
    for (unsigned i = 0; i < s.orders.size(); i++) {
        indexes.push_back(i);
    }

    unsigned a, i_a, b, i_b;

    i_a = Random::rng() % indexes.size();
    a = indexes[i_a];
    Data::pop(indexes, i_a);

    i_b = Random::rng() % indexes.size();
    b = indexes[i_b];

    std::swap(new_sol.back().orders[a], new_sol.back().orders[b]);

    return new_sol;
}

BuildOrderOptimizer::Population
BuildOrderOptimizer::Optimizers::insert(Solution const &s) {
    Population new_sol;
    new_sol.push_back(s);

    unsigned a, t;

    a = Random::rng() % s.orders.size();
    t = Random::rng() % State::RULES.tasks.size();

    new_sol.back().orders.insert(new_sol.back().orders.begin() + a, t);

    return new_sol;
}

BuildOrderOptimizer::Population
BuildOrderOptimizer::Optimizers::swap_and_delete(Solution const &s) {
    Population r;

    r.push_back(one_swap(s)[0]);
    r.push_back(delete_tail(s)[0]);

    return r;
}

BuildOrderOptimizer::Population
BuildOrderOptimizer::Optimizers::swap_and_insert(Solution const &s) {
    Population r;

    r.push_back(one_swap(s)[0]);
    r.push_back(insert(s)[0]);

    return r;
}
