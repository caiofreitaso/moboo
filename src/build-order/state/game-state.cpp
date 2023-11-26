#include <build-order/state/game-state.h>

bool
BuildOrderOptimizer::State::GameState::hasPrerequisites(unsigned t) const {
    auto task = RULES.tasks[t];

    for (unsigned i = 0; i < task.prerequisite.row.size(); i++) {
        unsigned index = task.prerequisite.row[i].index;
        unsigned value = task.prerequisite.row[i].value;

        if (resources[index].usable() < value)
            return false;
    }

    return true;
}

unsigned
BuildOrderOptimizer::State::GameState::quantity(unsigned index) const {
    auto resource_maximum = RULES.resources[index].overall_maximum;

    if (resource_maximum > 0 && resources[index].quantity > resource_maximum) {
        return resource_maximum;
    }

    return resources[index].quantity;
}

bool
BuildOrderOptimizer::State::GameState::hasOverallMaximum(unsigned task, Data::Contiguous<unsigned> r) {
    auto produce = RULES.tasks[task].produce;
    for (unsigned i = 0; i < produce.row.size(); i++) {
        auto index = produce.row[i].index;
        auto value = produce.row[i].value;
        auto resource_maximum = RULES.resources[index].overall_maximum;

        if (resource_maximum > 0 && r[index] + value > resource_maximum) {
            return false;
        }
    }

    return true;
}

bool
BuildOrderOptimizer::State::GameState::hasMaximum(unsigned task, Data::Contiguous<unsigned> r) {
    auto produce = RULES.tasks[task].produce;

    for (unsigned i = 0; i < produce.row.size(); i++) {
        unsigned index = produce.row[i].index;
        unsigned value = produce.row[i].value;

        if (r[index] + value > maximum(index, r)) {
            return false;
        }
    }

    return true;
}

unsigned
BuildOrderOptimizer::State::GameState::maximum(unsigned index, Data::Contiguous<unsigned> res) {
    auto maximum_per_resource = RULES.resources[index].maximum_per_resource;

    if (maximum_per_resource.row.size()) {
        unsigned max = 0;

        for (unsigned r = 0; r < maximum_per_resource.row.size(); r++) {
            unsigned max_index = maximum_per_resource.row[r].index;
            unsigned max_value = maximum_per_resource.row[r].value;

            max += res[max_index] * max_value;
        }

        return max;
    }

    return -1;
}

unsigned
BuildOrderOptimizer::State::GameState::maximum(unsigned index) const {
    auto maximum_per_resource = RULES.resources[index].maximum_per_resource;

    if (maximum_per_resource.row.size()) {
        unsigned max = 0;

        for (unsigned r = 0; r < maximum_per_resource.row.size(); r++) {
            unsigned max_index = maximum_per_resource.row[r].index;
            unsigned max_value = maximum_per_resource.row[r].value;

            max += resources[max_index].usable() * max_value;
        }

        return max;
    }

    return -1;
}

void
BuildOrderOptimizer::State::GameState::applyEvent(unsigned i) {
    Data::Matrix::SparseRow<unsigned> limited_indexes;
    auto bonus = RULES.events[i].bonus.row;

    // get resources with a maximum capacity
    for (unsigned r = 0; r < bonus.size(); r++) {
        unsigned index = bonus[r].index;
        auto maximum_per_resource = RULES.resources[index].maximum_per_resource;

        if (maximum_per_resource.row.size()) {
            limited_indexes.set(r, 1);
        }
    }

    for (unsigned k = 0; k < bonus.size(); k++) {
        unsigned index = bonus[k].index;
        unsigned value = bonus[k].value;

        if (limited_indexes.get(k)) {
            unsigned max = maximum(index);
            unsigned add = usable(index) + value;

            resources[index].quantity += max < add ? (add - max) - usable(index) : value;
        } else {
            resources[index].quantity += value;
        }
    }
}

void
BuildOrderOptimizer::State::GameState::produceResource(unsigned index, unsigned quantity) {
    auto added = quantity;
    auto maximum_per_resource = RULES.resources[index].maximum_per_resource;

    if (maximum_per_resource.row.size()) {
        auto max = maximum(index);

        if (max > quantity) {
            resources[index].quantity += quantity;
        } else {
            resources[index].quantity = max;
            added = max - quantity;
        }
    } else {
        resources[index].quantity += quantity;
    }

    for (unsigned k = 0; k < RULES.events.size(); k++) {
        if (RULES.events[k].trigger == index) {
            for (unsigned q = 0; q < added; q++) {
                if (RULES.events[k].time) {
                    resources[index].events.push_back(EventPointer(time, k));
                } else {
                    applyEvent(k);
                }
            }
        }
    }
}

void
BuildOrderOptimizer::State::GameState::consumeResource(unsigned type) {
    auto current_resource = resources[type];
    auto usable_resource = current_resource.usable();
    auto resource_events = current_resource.events;

    resources[type].used++;

    if (resource_events.size() == 0) {
        return;
    }

    unsigned step = 0;
    unsigned min_remaining_time[usable_resource];

    // add steps for each event triggered by current resource
    for (unsigned i = 0; i < RULES.events.size(); i++) {
        if (RULES.events[i].trigger == type) {
            step++;
        }
    }

    // initialize remaining time array
    for (int i = 0; i < usable_resource; i++) {
        min_remaining_time[i] = -1;
    }

    // rapaz, se eu entender hoje, eu estique
    for (unsigned i = 0; i < resource_events.size(); i++) {
        unsigned event_time = this->time - resource_events[i].start;
        auto event_type = resource_events[i].type;

        event_time %= RULES.events[event_type].time;
        event_time = RULES.events[event_type].time - event_time;

        if (event_time < min_remaining_time[i / step]) {
            min_remaining_time[i / step] = event_time;
        }
    }

    unsigned max = 0;
    unsigned origin = -1;

    for (int i = 0; i < usable_resource; i++) {
        if (min_remaining_time[i] > max) {
            max = min_remaining_time[i];
            origin = i * step;
        }
    }

    for (unsigned i = 0; i < step; i++) {
        Data::pop(resources[type].events, origin);
    }
}

BuildOrderOptimizer::State::GameState
BuildOrderOptimizer::State::createState() {
    GameState state;

    if (RULES.tasks.size()) {
        state.resources.resize(RULES.resources.size());

        for (unsigned i = 0; i < RULES.resources.size(); i++) {
            state.resources[i].quantity = 0;
            state.resources[i].used = 0;
            state.resources[i].borrowed = 0;
            state.resources[i].capacity = RULES.resources[i].overall_maximum > 0;
        }

        state.time = 0;
    } else {
        state.time = -1;
    }

    return state;
}
