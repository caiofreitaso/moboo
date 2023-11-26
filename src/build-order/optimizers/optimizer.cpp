#include <build-order/optimizers/optimizer.h>

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
                    auto usable = problem.objectives[Problem::RC_USABLE].get(p);
                    auto quantity = problem.objectives[Problem::RC_QUANTITY].get(p);
                    auto used = problem.objectives[Problem::RC_USED].get(p);

                    if (usable) {
                        if (usable == MINIMIZE) {
                            objective_value += resource_reduce;
                            objective_value -= resource_increase / 2;
                        } else {
                            objective_value -= resource_reduce / 2;
                            objective_value += resource_increase;
                        }
                    }

                    if (quantity) {
                        if (quantity == MINIMIZE)
                            objective_value -= resource_increase / 2;
                        else
                            objective_value += resource_increase;
                    }

                    if (used) {
                        if (used == MINIMIZE)
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

                    for (auto &o : problem.objectives[Problem::RC_USABLE].row) {
                        auto obj = o.index;
                        auto type = o.value;

                        if (collateral == obj) {
                            if (type == MINIMIZE)
                                objective_value += damage;
                            else
                                objective_value -= damage / 2;
                        }
                    }

                    for (auto &o : problem.objectives[Problem::RC_USED].row) {
                        auto obj = o.index;
                        auto type = o.value;

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

                    for (auto &o : problem.objectives[Problem::RC_USABLE].row) {
                        auto obj = o.index;
                        auto type = o.value;

                        if (collateral == obj) {
                            objective_value += type == MINIMIZE ? damage : -(damage / 2);
                        }
                    }

                    for (auto &o : problem.objectives[Problem::RC_USED].row) {
                        auto obj = o.index;
                        auto type = o.value;
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
            for (auto &restriction : problem.restrictions) {
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
