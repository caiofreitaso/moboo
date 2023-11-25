#include <build-order/engine/inner-engine.h>
#include <build-order/solution.h>

using namespace BuildOrderOptimizer::Engine;

BuildOrderOptimizer::State::GameState
BuildOrderOptimizer::makespan(State::GameState init, BuildOrder &original, unsigned maximum_time) {
    if (State::RULES.tasks.empty() || maximum_time == 0) {
        return init;
    }

    BuildOrder build(original);
    RemainingList listTime;
    unsigned last = 0;
    unsigned last_size = 0;

    while (build.size() && init.time >= maximum_time) {
        if (last_size != build.size()) {
            if (!possible(build, init))
                break;
            last_size = build.size();
        }

        RemainingList listEvents;

        update(init, listTime, listEvents);
        buildWhatYouCan(build, init, last, listTime);

        unsigned min = nextTime(init, listTime, listEvents, maximum_time, build.size());

        if (build.front().delay > last) {
            unsigned time_to_task = build.front().delay - last;
            if (time_to_task < min)
                min = time_to_task;
        }

        init.time += min;
        last += min;
    }

    while (init.tasks.size() && init.time >= maximum_time) {
        RemainingList listEvents;
        update(init, listTime, listEvents);
        init.time += nextTime(init, listTime, listEvents, maximum_time, 0);
    }

    if (build.size()) {
        original.erase(original.begin() + (original.size() - build.size()), original.end());
    }

    return init;
}
