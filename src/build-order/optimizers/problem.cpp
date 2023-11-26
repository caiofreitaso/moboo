#include <build-order/optimizers/problem.h>

bool
BuildOrderOptimizer::Optimizers::Problem::dominates(Solution a, Solution b) const {
    // if (weights.size() && use_weights) {
    //     double a_value = 0, b_value = 0;
    //     unsigned w = 0;
    //     if (time_as_objective) {
    //         a_value += weights[w] * a.final_state.time;
    //         b_value += weights[w] * b.final_state.time;
    //         w++;
    //     }

    //     for (unsigned i = 0; i < objectives[0].row.size(); i++) {
    //         unsigned index = objectives[0].row[i].index;

    //         a_value += weights[w] * a.final_state.resources[index].usable();
    //         b_value += weights[w] * b.final_state.resources[index].usable();
    //         w++;
    //     }

    //     for (unsigned i = 0; i < objectives[1].row.size(); i++) {
    //         unsigned index = objectives[1].row[i].index;

    //         a_value += weights[w] * a.final_state.resources[index].quantity;
    //         b_value += weights[w] * b.final_state.resources[index].quantity;
    //         w++;
    //     }

    //     for (unsigned i = 0; i < objectives[2].row.size(); i++) {
    //         unsigned index = objectives[2].row[i].index;

    //         a_value += weights[w] * a.final_state.resources[index].used;
    //         b_value += weights[w] * b.final_state.resources[index].used;
    //         w++;
    //     }

    //     return a_value > b_value;
    // } else {
    Data::Contiguous<unsigned> a_obj = toVector(a);
    Data::Contiguous<unsigned> b_obj = toVector(b);
    Data::Contiguous<bool> min = objectivesVector();

    bool strictly = false;
    for (unsigned i = 0; i < a_obj.size(); i++)
        if (min[i]) {
            if (a_obj[i] <= b_obj[i])
                strictly = true;
            else // if (a_obj[i] > b_obj[i])
                return false;
        } else {
            if (a_obj[i] >= b_obj[i])
                strictly = true;
            else // if (a_obj[i] < b_obj[i])
                return false;
        }

    return strictly;
    // }
}

bool
BuildOrderOptimizer::Optimizers::Problem::valid(Solution s) const {
    for (auto &restriction : restrictions[RC_USABLE].row) {
        unsigned index = restriction.index;
        int usable = s.final_state.resources[index].usable();

        if (restriction.value.less_than != -1 && usable > 0 && (unsigned)usable > restriction.value.less_than) {
            return false;
        }

        if (restriction.value.greater_than) {
            if (usable > 0) {
                if ((unsigned)usable < restriction.value.greater_than)
                    return false;
            } else
                return false;
        }
    }

    for (auto &restriction : restrictions[RC_QUANTITY].row) {
        unsigned index = restriction.index;
        int quantity = s.final_state.resources[index].quantity;

        if (restriction.value.less_than != -1 && quantity > restriction.value.less_than) {
            return false;
        }
        if (restriction.value.greater_than && quantity < restriction.value.greater_than) {
            return false;
        }
    }

    for (auto &restriction : restrictions[RC_USED].row) {
        unsigned index = restriction.index;
        int used = s.final_state.resources[index].used;

        if (restriction.value.less_than != -1 && used > restriction.value.less_than) {
            return false;
        }
        if (restriction.value.greater_than && used < restriction.value.greater_than) {
            return false;
        }
    }

    return true;
}

unsigned
BuildOrderOptimizer::Optimizers::Problem::producesMaximize(unsigned task) const {
    unsigned ret = 0;
    for (auto &produce : State::RULES.tasks[task].produce.row) {
        for (unsigned o = 0; o < 3; o++) {
            if (objectives[o].get(produce.index) == MAXIMIZE) {
                ret++;
            }
        }
    }
    return ret;
}

unsigned
BuildOrderOptimizer::Optimizers::Problem::producesGreaterThan(unsigned task) const {
    unsigned ret = 0;
    for (auto &produce : State::RULES.tasks[task].produce.row) {
        for (unsigned o = 0; o < 3; o++) {
            if (restrictions[o].get(produce.index).greater_than) {
                ret++;
            }
        }
    }
    return ret;
}

BuildOrderOptimizer::Population
BuildOrderOptimizer::Optimizers::Problem::nonDominated(Population p) const {
    Population ret;

    std::sort(p.begin(), p.end());

    ret.push_back(p[0]);

    for (unsigned i = 1; i < p.size(); i++) {
        auto &current = p[i];
        bool included = false;

        for (auto &k : ret) {
            if (current == k) {
                included = true;
                break;
            }
        }

        if (!included) {
            unsigned count = 0;
            for (unsigned k = 0; k < ret.size(); k++) {
                if (dominates(current, ret[k])) {
                    Data::pop(ret, k);
                    k--;
                    count++;
                }
            }

            bool dominated = false;
            if (!count) {
                for (auto &k : ret) {
                    if (dominates(k, current)) {
                        Data::pop(p, i);
                        i--;
                        dominated = true;
                        break;
                    }
                }
            }

            if (!dominated) {
                ret.push_back(current);
            }
        }
    }

    return ret;
}

unsigned
BuildOrderOptimizer::Optimizers::Problem::numberObjectives() const {
    return objectives[RC_USABLE].row.size() + objectives[RC_QUANTITY].row.size() + objectives[RC_USED].row.size();
}
unsigned
BuildOrderOptimizer::Optimizers::Problem::numberRestrictions() const {
    return restrictions[RC_USABLE].row.size() + restrictions[RC_QUANTITY].row.size() + restrictions[RC_USED].row.size();
}

BuildOrderOptimizer::Data::Contiguous<unsigned>
BuildOrderOptimizer::Optimizers::Problem::toVector(Solution a) const {
    Data::Contiguous<unsigned> ret;
    auto &resources = a.final_state.resources;

    ret.reserve(numberObjectives());

    if (time_as_objective) {
        ret.push_back(a.final_state.time);
    }

    for (auto &objective : objectives[RC_USABLE].row) {
        unsigned index = objective.index;

        ret.push_back(resources[index].usable());
    }

    for (auto &objective : objectives[RC_QUANTITY].row) {
        unsigned index = objective.index;

        ret.push_back(resources[index].quantity);
    }

    for (auto &objective : objectives[RC_USED].row) {
        unsigned index = objective.index;

        ret.push_back(resources[index].used);
    }

    return ret;
}

BuildOrderOptimizer::Data::Contiguous<BuildOrderOptimizer::Data::Contiguous<unsigned>>
BuildOrderOptimizer::Optimizers::Problem::toVector(Population a) const {
    Data::Contiguous<Data::Contiguous<unsigned>> ret;

    ret.reserve(a.size());

    for (auto p : a) {
        ret.push_back(toVector(p));
    }

    return ret;
}

BuildOrderOptimizer::Data::Contiguous<double>
BuildOrderOptimizer::Optimizers::Problem::toDVector(Solution a) const {
    Data::Contiguous<double> ret;
    auto &resources = a.final_state.resources;

    ret.reserve(numberObjectives());

    if (time_as_objective) {
        ret.push_back(a.final_state.time);
    }

    for (auto &objective : objectives[RC_USABLE].row) {
        unsigned index = objective.index;

        ret.push_back(resources[index].usable());
    }

    for (auto &objective : objectives[RC_QUANTITY].row) {
        unsigned index = objective.index;

        ret.push_back(resources[index].quantity);
    }

    for (auto &objective : objectives[RC_USED].row) {
        unsigned index = objective.index;

        ret.push_back(resources[index].used);
    }

    return ret;
}

BuildOrderOptimizer::Data::Contiguous<BuildOrderOptimizer::Data::Contiguous<double>>
BuildOrderOptimizer::Optimizers::Problem::toDVector(Population a) const {
    Data::Contiguous<Data::Contiguous<double>> ret;

    ret.reserve(a.size());

    for (auto &p : a) {
        ret.push_back(toDVector(p));
    }

    return ret;
}

BuildOrderOptimizer::Data::Contiguous<bool>
BuildOrderOptimizer::Optimizers::Problem::objectivesVector() const {
    Data::Contiguous<bool> min;

    if (time_as_objective) {
        min.push_back(true);
    }

    for (auto &objective : objectives[RC_USABLE].row) {
        min.push_back(objective.value == MINIMIZE);
    }

    for (auto &objective : objectives[RC_QUANTITY].row) {
        min.push_back(objective.value == MINIMIZE);
    }

    for (auto &objective : objectives[RC_USED].row) {
        min.push_back(objective.value == MINIMIZE);
    }

    return min;
}
