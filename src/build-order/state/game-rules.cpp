#include <build-order/state/game-rules.h>

BuildOrderOptimizer::State::GameRules BuildOrderOptimizer::State::RULES;

bool
BuildOrderOptimizer::State::GameRules::fillsPrerequisite(unsigned task, unsigned resource, unsigned amount) {
    return tasks[task].prerequisite.get(resource) <= amount;
}

bool
BuildOrderOptimizer::State::GameRules::fillsBorrow(unsigned task, unsigned resource, unsigned amount) {
    return tasks[task].borrow.get(resource) <= amount;
}

bool
BuildOrderOptimizer::State::GameRules::fillsConsume(unsigned task, unsigned resource, unsigned amount) {
    return tasks[task].consume.get(resource) <= amount;
}
