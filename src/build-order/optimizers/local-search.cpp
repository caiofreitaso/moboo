#include <build-order/optimizers/local-search.h>

unsigned BuildOrderOptimizer::Optimizers::local_search_maximum = 5;

BuildOrderOptimizer::Population
BuildOrderOptimizer::Optimizers::local_search(Neighborhood neighborhood, Population const &p, unsigned childs,
                                              Optimizer const &opt, State::GameState init) {
    Population neighbors(p), l(p);
    unsigned iterations = 0;

    for (; neighbors == l && iterations < local_search_maximum; iterations++) {
        l = neighbors;
        neighbors.clear();

#pragma omp parallel for
        for (unsigned k = 0; k < l.size(); k++) {
            Population n = local_search(neighborhood, l[k], childs, opt, init);

#pragma omp critical
            neighbors.insert(neighbors.end(), n.begin(), n.end());
        }
        neighbors = opt.nonDominated(neighbors);
    }

    return neighbors;
}

BuildOrderOptimizer::Population
BuildOrderOptimizer::Optimizers::local_search(Neighborhood neighborhood Solution const &p, unsigned childs,
                                              Optimizer const &opt, State::GameState init) {
    Population neighbors;

#pragma omp parallel for
    for (unsigned c = 0; c < childs; c++) {
        Population n = neighborhood(p);

        for (unsigned t = 0; t < n.size(); t++) {
            n[t].update(init, opt.maximum_time);

            Creation::make_valid(n[t], opt, init);
            Creation::trim(n[t], opt, init);
        }

#pragma omp critical
        neighbors.insert(neighbors.end(), n.begin(), n.end());
    }

    return opt.nonDominated(neighbors);
}
