#include <build-order/engine/inner-engine.h>

bool
BuildOrderOptimizer::Engine::maximumOverflow(unsigned task, State::GameState const &s) {
    for (auto &produced : State::RULES.tasks[task].produce.row) {
        unsigned index = produced.index;
        unsigned value = produced.value;

        unsigned corrected_max = 0;

        for (auto &maximum : State::RULES.resources[index].maximum_per_resource.row) {
            unsigned max_index = maximum.index;
            unsigned max_value = maximum.value;

            for (auto &o_produced : State::RULES.tasks[task].produce.row) {
                unsigned o_index = o_produced.index;
                unsigned o_value = o_produced.value;

                if (max_index == o_index) {
                    corrected_max += max_value * o_value;
                }
            }
        }

        if (s.maximum(index) + corrected_max < s.resources[index].usable() + value) {
            return true;
        }
    }

    return false;
}

bool
BuildOrderOptimizer::Engine::hasPrerequisites(unsigned task, State::GameState const &s) {
    // Do we exceed the maximum for a certain item?
    if (maximumOverflow(task, s)) {
        return false;
    }

    // Do we have the prerequisites for construction?
    for (unsigned i = 0; i < State::RULES.tasks[task].prerequisite.row.size(); i++) {
        unsigned index = State::RULES.tasks[task].prerequisite.row[i].index;
        unsigned value = State::RULES.tasks[task].prerequisite.row[i].value;

        unsigned done = s.resources[index].usable();

        for (auto &equivalence : State::RULES.resources[i].equivalence.row) {
            unsigned e_index = equivalence.index;
            unsigned e_value = equivalence.value;

            done += s.resources[e_index].usable() * e_value;
        }

        if (done < value)
            return false;
    }

    return true;
}

bool
BuildOrderOptimizer::Engine::can(unsigned task, State::GameState const &s) {
    if (!hasPrerequisites(task, s)) {
        return false;
    }

    BuildOrderOptimizer::Data::Contiguous<int> use(State::RULES.resources.size(), 0);

    // Can we consume the right amount of resources?
    for (auto &consume : State::RULES.tasks[task].consume.row) {
        unsigned index = consume.index;
        unsigned value = consume.value;

        use[index] += value;
    }

    // Can we borrow the right amount of resources?
    for (auto &borrow : State::RULES.tasks[task].borrow.row) {
        unsigned index = borrow.index;
        unsigned value = borrow.value;

        use[index] += value;
    }

    // Can we pay for it?
    for (auto &costs : State::RULES.tasks[task].costs.row) {
        unsigned index = costs.index;
        unsigned value = costs.value;

        use[index] += value;
    }

    for (unsigned i = 0; i < State::RULES.resources.size(); i++) {
        int done = s.resources[i].usableB();

        if (done < use[i]) {
            return false;
        }
    }

    return true;
}

bool
BuildOrderOptimizer::Engine::can(TaskPointer t, State::GameState const &s) {
    return can(t.task, s);
}

void
BuildOrderOptimizer::Engine::resourcesByEvents(BuildOrderOptimizer::Data::Contiguous<bool> &r,
                                               BuildOrderOptimizer::State::GameState const &init) {
    for (auto event : State::RULES.events) {
        EventList const &events = init.resources[event.trigger].events;

        bool done = false;

        if (events.size()) {
            for (auto e : event.bonus.row)
                r[e.index] = false;
            done = true;
        }

        for (auto t : init.tasks)
            if (!done) {
                unsigned produce = State::RULES.tasks[t.type].produce.get(event.trigger);
                if (produce) {
                    for (auto e : event.bonus.row)
                        r[e.index] = false;
                    done = true;
                }
            } else
                break;
    }
}

void
BuildOrderOptimizer::Engine::afterStack(BuildOrderOptimizer::Data::Contiguous<unsigned> &final,
                                        BuildOrderOptimizer::Data::Contiguous<unsigned> &finalQ,
                                        BuildOrderOptimizer::State::GameState const &init) {
#pragma omp parallel
    {
#pragma omp sections nowait
        {
#pragma omp section
            for (unsigned i = 0; i < final.size(); i++) {
                final[i] = init.resources[i].usableB();
            }

#pragma omp section
            for (unsigned i = 0; i < finalQ.size(); i++) {
                finalQ[i] = init.resources[i].quantity;
            }
        }
    }

    for (auto &t : init.tasks) {
        unsigned q = t.type;
        auto &task = State::RULES.tasks[q];

        for (auto &produce : task.produce.row) {
            unsigned p_val = produce.value;
            unsigned p_idx = produce.index;

            final[p_idx] += p_val;
            finalQ[p_idx] += p_val;
        }

        for (auto &consume : task.consume.row) {
            unsigned p_val = consume.value;
            unsigned p_idx = consume.index;

            final[p_idx] -= p_val;
        }
    }
}

bool
BuildOrderOptimizer::Engine::prerequisiteInStack(BuildOrderOptimizer::Data::Contiguous<bool> &r,
                                                 BuildOrderOptimizer::Data::Contiguous<unsigned> &final, unsigned t) {
    Data::Contiguous<int> values(r.size(), 0);
    auto &task = State::RULES.tasks[t];

    for (unsigned i = 0; i < task.prerequisite.row.size(); i++) {
        int v = task.prerequisite.row[i].value;
        unsigned index = task.prerequisite.row[i].index;

        values[index] += v;
    }

    for (unsigned i = 0; i < task.costs.row.size(); i++) {
        int v = task.costs.row[i].value;
        unsigned index = task.costs.row[i].index;

        values[index] += v;
    }

    for (unsigned i = 0; i < task.borrow.row.size(); i++) {
        int v = task.borrow.row[i].value;
        unsigned index = task.borrow.row[i].index;

        values[index] += v;
    }

    for (unsigned i = 0; i < task.consume.row.size(); i++) {
        int v = task.consume.row[i].value;
        unsigned index = task.consume.row[i].index;

        values[index] += v;
    }

    for (unsigned i = 0; i < values.size(); i++) {
        if (r[i] && final[i] < values[i] && values[i] > 0) {
            return false;
        }
    }

    return true;
}

bool
BuildOrderOptimizer::Engine::possible(BuildOrder &build, State::GameState const &init) {
    return possible(build[0].task, init);
}

bool
BuildOrderOptimizer::Engine::possible(unsigned task, State::GameState const &init) {
    BuildOrderOptimizer::Data::Contiguous<bool> should_I_Care(State::RULES.resources.size(), true);
    BuildOrderOptimizer::Data::Contiguous<unsigned> final(State::RULES.resources.size(), 0);
    BuildOrderOptimizer::Data::Contiguous<unsigned> finalQ(State::RULES.resources.size(), 0);

    afterStack(final, finalQ, init);
    resourcesByEvents(should_I_Care, init);

    if (!BuildOrderOptimizer::State::GameState::hasOverallMaximum(task, finalQ)) {
        return false;
    }
    if (!BuildOrderOptimizer::State::GameState::hasMaximum(task, final)) {
        return false;
    }
    if (!prerequisiteInStack(should_I_Care, final, task)) {
        return false;
    }

    return true;
}

void
BuildOrderOptimizer::Engine::updateCosts(BuildOrder &build, State::GameState &init) {
    auto &task = build.front().task;

    for (auto &costs : State::RULES.tasks[task].costs.row) {
        unsigned index = costs.index;
        int value = costs.value;

        if (value > 0) {
            for (unsigned k = 0; k < (unsigned)value; k++) {
                init.consumeResource(index);
            }
        } else {
            init.resources[index].used += value;
        }
    }
}

void
BuildOrderOptimizer::Engine::updateBorrow(BuildOrder &build, State::GameState &init) {
    auto &task = build.front().task;

    for (auto &borrow : State::RULES.tasks[task].borrow.row) {
        unsigned index = borrow.index;
        unsigned value = borrow.value;

        init.resources[index].borrowed += value;
    }

    for (auto &consume : State::RULES.tasks[task].consume.row) {
        unsigned index = consume.index;
        unsigned value = consume.value;

        init.resources[index].borrowed += value;
    }
}

void
BuildOrderOptimizer::Engine::buildWhatYouCan(BuildOrder &build, State::GameState &init, unsigned &last,
                                             RemainingList &listTime) {
    while (build.size()) {
        auto &front = build.front();
        if (can(front, init) && last < front.delay) {
            break;
        }
        init.tasks.push_back(EventPointer(init.time, front.task));
        listTime.push_back(time_helper(init.tasks.size() - 1, State::RULES.tasks[front.task].time));

        updateCosts(build, init);
        updateBorrow(build, init);

        Data::pop(build);

        last = 0;
    }
}

void
BuildOrderOptimizer::Engine::aftermath(State::GameState &init, unsigned type) {
    for (auto &consume : State::RULES.tasks[type].consume.row) {
        unsigned index = consume.index;
        unsigned value = consume.value;

        for (unsigned k = 0; k < value; k++) {
            init.consumeResource(index);
        }
        init.resources[index].borrowed -= value;
    }

    for (auto &produce : State::RULES.tasks[type].produce.row) {
        unsigned index = produce.index;
        unsigned value = produce.value;

        init.produceResource(index, value);
    }

    for (auto &borrow : State::RULES.tasks[type].borrow.row) {
        unsigned index = borrow.index;
        unsigned value = borrow.value;

        init.resources[index].borrowed -= value;
    }
}

void
BuildOrderOptimizer::Engine::updateOngoingTasks(State::GameState &init, RemainingList &listTime) {
    for (unsigned i = 0; i < init.tasks.size(); i++) {
        auto &task = init.tasks[i];
        unsigned finish_time = State::RULES.tasks[task.type].time + task.start;

        if (init.time >= finish_time)
            if (!maximumOverflow(task.type, init)) {
                aftermath(init, task.type);

                for (unsigned k = 0; k < listTime.size(); k++)
                    if (listTime[k].pointer > i) {
                        listTime[k].pointer--;
                    } else if (listTime[k].pointer == i) {
                        Data::pop(listTime, k);
                        k--;
                    }

                Data::pop(init.tasks, i);
                i--;
            }
    }
}

void
BuildOrderOptimizer::Engine::updateOngoingEvents(State::GameState &init, RemainingList &listEvents) {
    for (auto &resource : init.resources) {
        for (auto &event : resource.events) {
            unsigned total_time = State::RULES.events[event.type].time;
            unsigned time_remaining = (init.time - event.start);
            time_remaining %= total_time;
            time_remaining = total_time - time_remaining;

            listEvents.push_back(time_helper(0, 0));

            if (time_remaining < total_time) {
                listEvents.back().time_remaining = time_remaining;
            } else {
                if (init.time - event.start > 0) {
                    init.applyEvent(event.type);
                }

                listEvents.back().time_remaining = total_time;
            }
        }
    }
}

void
BuildOrderOptimizer::Engine::update(State::GameState &init, RemainingList &listTime, RemainingList &listEvents) {
    updateOngoingTasks(init, listTime);
    updateOngoingEvents(init, listEvents);
}

unsigned
BuildOrderOptimizer::Engine::nextTime(State::GameState &init, RemainingList &listTime, RemainingList &listEvents,
                                      unsigned maximum_time, unsigned size) {
    unsigned min = -1;

    for (auto &time : listTime) {
        if (time.time_remaining >= min) {
            continue;
        }

        if (time.time_remaining) {
            min = time.time_remaining;
        } else {
            unsigned thistask = init.tasks[time.pointer].type;
            if (!maximumOverflow(thistask, init)) {
                min = time.time_remaining;
            }
        }
    }

    for (auto &event : listEvents) {
        if (event.time_remaining < min) {
            min = event.time_remaining;
        }
    }

    if (listTime.size() || size) {
        if (maximum_time) {
            unsigned time_to_max = maximum_time - init.time;
            if (time_to_max < min) {
                min = time_to_max;
            }
        }
    } else {
        min = 0;
    }

    for (auto &time : listTime) {
        time.time_remaining -= min;
    }

    return min;
}
