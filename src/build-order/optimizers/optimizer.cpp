#include <build-order/optimizers/optimizer.h>

bool
BuildOrderOptimizer::Optimizers::Optimizer::dominates(Solution a, Solution b) const {
    if (weights.size() && use_weights) {
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
    } else {
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
    }
}

bool
BuildOrderOptimizer::Optimizers::Optimizer::valid(Solution s) const {
    for (unsigned i = 0; i < restrictions[0].row.size(); i++) {
        unsigned index = restrictions[0].row[i].index;
        Restriction value;
        value.less_than = restrictions[0].row[i].value.less_than;
        value.greater_than = restrictions[0].row[i].value.greater_than;

        int usable = s.final_state.resources[index].usable();

        if (value.less_than != -1)
            if (usable > 0)
                if ((unsigned)usable > value.less_than)
                    return false;

        if (value.greater_than) {
            if (usable > 0) {
                if ((unsigned)usable < value.greater_than)
                    return false;
            } else
                return false;
        }
    }

    for (unsigned i = 0; i < restrictions[1].row.size(); i++) {
        unsigned index = restrictions[1].row[i].index;
        Restriction value;
        value.less_than = restrictions[1].row[i].value.less_than;
        value.greater_than = restrictions[1].row[i].value.greater_than;

        if (value.less_than != -1) {
            if (s.final_state.resources[index].quantity > value.less_than)
                return false;
        }
        if (value.greater_than) {
            if (s.final_state.resources[index].quantity < value.greater_than)
                return false;
        }
    }

    for (unsigned i = 0; i < restrictions[2].row.size(); i++) {
        unsigned index = restrictions[2].row[i].index;
        Restriction value;
        value.less_than = restrictions[2].row[i].value.less_than;
        value.greater_than = restrictions[2].row[i].value.greater_than;

        if (value.less_than != -1) {
            if (s.final_state.resources[index].used > value.less_than)
                return false;
        }
        if (value.greater_than) {
            if (s.final_state.resources[index].used < value.greater_than)
                return false;
        }
    }

    return true;
}

unsigned
BuildOrderOptimizer::Optimizers::Optimizer::producesMaximize(unsigned task) const {
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
BuildOrderOptimizer::Optimizers::Optimizer::producesGreaterThan(unsigned task) const {
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
BuildOrderOptimizer::Optimizers::Optimizer::nonDominated(Population p) const {
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
BuildOrderOptimizer::Optimizers::Optimizer::numberObjectives() const {
    return objectives[0].row.size() + objectives[1].row.size() + objectives[2].row.size();
}
unsigned
BuildOrderOptimizer::Optimizers::Optimizer::numberRestrictions() const {
    return restrictions[0].row.size() + restrictions[1].row.size() + restrictions[2].row.size();
}

void
BuildOrderOptimizer::Optimizers::Optimizer::update() {
    _objV.resize(State::RULES.tasks.size(), 0);
    _incV.resize(State::RULES.tasks.size());
    _decV.resize(State::RULES.tasks.size());

    // INITIAL MAP
    for (unsigned t = 0; t < State::RULES.tasks.size(); t++) {
        auto &task = State::RULES.tasks[t];
        // RESOURCE BONUS/PENALTY
        for (unsigned p = 0; p < State::RULES.resources.size(); p++) {
            double resource_reduce = 0;
            double resource_increase = 0;

            resource_reduce += task.costs.get(p) > 0 ? 1 : 0;
            resource_reduce += task.consume.get(p) ? 1 : 0;

            resource_increase += task.costs.get(p) < 0 ? 1 : 0;
            resource_increase += task.produce.get(p) ? 1 : 0;

            resource_increase += State::RULES.taskValuePerEvent[p].get(t);

            _incV[t].set(p, resource_increase);
            _decV[t].set(p, resource_reduce);

            if (resource_reduce > 0 || resource_increase > 0) {
                // OBJECTIVES
                {
                    double objective_value = 0;

                    if (objectives[0].get(p)) {
                        if (objectives[0].get(p) == MINIMIZE) {
                            objective_value += resource_reduce;
                            objective_value -= resource_increase / 2;
                        } else {
                            objective_value -= resource_reduce / 2;
                            objective_value += resource_increase;
                        }
                    }

                    if (objectives[1].get(p)) {
                        if (objectives[1].get(p) == MINIMIZE)
                            objective_value -= resource_increase / 2;
                        else
                            objective_value += resource_increase;
                    }

                    if (objectives[2].get(p)) {
                        if (objectives[2].get(p) == MINIMIZE)
                            objective_value -= resource_reduce / 2;
                        else
                            objective_value += resource_reduce;
                    }

                    _objV[t] += objective_value; // * objective_multiplier;
                }
            }
        }

        // COLLATERAL DAMAGE (EVENT DESTRUCTION)
        {
            // COSTS
            for (unsigned k = 0; k < task.costs.row.size(); k++) {
                unsigned index = task.costs.row[k].index;

                for (unsigned e = 0; e < State::RULES.resourceValueLost[index].row.size(); e++) {
                    unsigned collateral = State::RULES.resourceValueLost[index].row[e].index;
                    double damage = 1;

                    double objective_value = 0;

                    for (unsigned o = 0; o < objectives[0].row.size(); o++) {
                        unsigned obj = objectives[0].row[o].index;
                        Objective type = objectives[0].row[o].value;

                        if (collateral == obj) {
                            if (type == MINIMIZE)
                                objective_value += damage;
                            else
                                objective_value -= damage / 2;
                        }
                    }

                    for (unsigned o = 0; o < objectives[2].row.size(); o++) {
                        unsigned obj = objectives[2].row[o].index;
                        Objective type = objectives[2].row[o].value;

                        if (collateral == obj) {
                            if (type == MINIMIZE)
                                objective_value -= damage / 2;
                            else
                                objective_value += damage;
                        }
                    }

                    _objV[t] += objective_value; // * objective_multiplier;
                }
            }

            // CONSUME
            for (auto &consume : task.consume.row) {
                unsigned index = consume.index;

                for (auto &lost : State::RULES.resourceValueLost[index].row) {
                    unsigned collateral = lost.index;
                    double damage = 1;

                    double objective_value = 0;

                    for (auto &objective : objectives[0].row) {
                        auto obj = objective.index;
                        auto type = objective.value;

                        if (collateral == obj) {
                            objective_value += type == MINIMIZE ? damage : -(damage / 2);
                        }
                    }

                    for (auto &objective : objectives[2].row) {
                        auto obj = objective.index;
                        auto type = objective.value;
                        if (collateral == obj) {
                            objective_value += type == MINIMIZE ? -(damage / 2) : damage;
                        }
                    }

                    _objV[t] += objective_value; // * objective_multiplier;
                }
            }
        }
    }
}

BuildOrderOptimizer::Data::Contiguous<double>
BuildOrderOptimizer::Optimizers::Optimizer::initialMap(double o, double r, State::GameState initial) const {
    Data::Contiguous<double> taskValue(_objV.size(), 0);

    // RESTRICTIONS
    for (unsigned t = 0; t < taskValue.size(); t++) {
        for (unsigned p = 0; p < State::RULES.resources.size(); p++) {
            double restriction_value = 0;
            for (auto &restriction : restrictions) {
                if (restriction.get(p)) {
                    unsigned final = initial.resources[p].usable();

                    if (restriction.get(p).less_than)
                        if (final >= restriction.get(p).less_than) {
                            restriction_value += _decV[t].get(p);
                            restriction_value -= _incV[t].get(p) / 2;
                        }

                    if (restriction.get(p).greater_than)
                        if (final <= restriction.get(p).greater_than) {
                            restriction_value -= _decV[t].get(p) / 2;
                            restriction_value += _incV[t].get(p);
                        }
                }
            }

            taskValue[t] += restriction_value * r; // * restriction_multiplier;
        }
    }
    for (unsigned t = 0; t < State::RULES.tasks.size(); t++) {
        taskValue[t] += _objV[t] * o; // + _resV[t] * r;
    }

    return taskValue;
}

BuildOrderOptimizer::Data::Contiguous<unsigned>
BuildOrderOptimizer::Optimizers::Optimizer::toVector(Solution a) const {
    Data::Contiguous<unsigned> ret;
    auto &resources = a.final_state.resources;

    ret.reserve(numberObjectives());

    if (time_as_objective) {
        ret.push_back(a.final_state.time);
    }

    for (auto &objective : objectives[0].row) {
        unsigned index = objective.index;

        ret.push_back(resources[index].usable());
    }

    for (auto &objective : objectives[1].row) {
        unsigned index = objective.index;

        ret.push_back(resources[index].quantity);
    }

    for (auto &objective : objectives[2].row) {
        unsigned index = objective.index;

        ret.push_back(resources[index].used);
    }

    return ret;
}

BuildOrderOptimizer::Data::Contiguous<BuildOrderOptimizer::Data::Contiguous<unsigned>>
BuildOrderOptimizer::Optimizers::Optimizer::toVector(Population a) const {
    Data::Contiguous<Data::Contiguous<unsigned>> ret;

    ret.reserve(a.size());

    for (auto p : a) {
        ret.push_back(toVector(p));
    }

    return ret;
}

BuildOrderOptimizer::Data::Contiguous<double>
BuildOrderOptimizer::Optimizers::Optimizer::toDVector(Solution a) const {
    Data::Contiguous<double> ret;
    auto &resources = a.final_state.resources;

    ret.reserve(numberObjectives());

    if (time_as_objective) {
        ret.push_back(a.final_state.time);
    }

    for (auto &objective : objectives[0].row) {
        unsigned index = objective.index;

        ret.push_back(resources[index].usable());
    }

    for (auto &objective : objectives[1].row) {
        unsigned index = objective.index;

        ret.push_back(resources[index].quantity);
    }

    for (auto &objective : objectives[2].row) {
        unsigned index = objective.index;

        ret.push_back(resources[index].used);
    }

    return ret;
}

BuildOrderOptimizer::Data::Contiguous<BuildOrderOptimizer::Data::Contiguous<double>>
BuildOrderOptimizer::Optimizers::Optimizer::toDVector(Population a) const {
    Data::Contiguous<Data::Contiguous<double>> ret;

    ret.reserve(a.size());

    for (auto &p : a) {
        ret.push_back(toDVector(p));
    }

    return ret;
}

BuildOrderOptimizer::Data::Contiguous<bool>
BuildOrderOptimizer::Optimizers::Optimizer::objectivesVector() const {
    Data::Contiguous<bool> min;

    if (time_as_objective) {
        min.push_back(true);
    }

    for (auto &objective : objectives[0].row) {
        min.push_back(objective.value == MINIMIZE);
    }

    for (auto &objective : objectives[1].row) {
        min.push_back(objective.value == MINIMIZE);
    }

    for (auto &objective : objectives[2].row) {
        min.push_back(objective.value == MINIMIZE);
    }

    return min;
}
