#ifndef STATE_GAME_RULES_H
#define STATE_GAME_RULES_H

#include <build-order/data/contiguous.h>
#include <build-order/data/matrix/sparse-row.h>
#include <build-order/event-pointer.h>
#include <build-order/rules/dependency-graph.h>
#include <build-order/rules/event.h>
#include <build-order/rules/resource.h>
#include <build-order/rules/task.h>

#include <fstream>
#include <iostream>
#include <sstream>

namespace BuildOrderOptimizer::State {

struct GameRules {
    Rules::DependencyGraph graph;

    Data::Contiguous<Rules::Task> tasks;
    Data::Contiguous<Rules::Event> events;
    Data::Contiguous<Rules::Resource> resources;

    Data::Contiguous<Data::Matrix::SparseRow<unsigned>> taskProduceByEvent;
    Data::Contiguous<Data::Matrix::SparseRow<unsigned>> taskValuePerEvent;
    Data::Contiguous<Data::Matrix::SparseRow<unsigned>> resourceValueLost;

    GameRules() {}
    GameRules(unsigned numberResources, unsigned numberTasks, unsigned numberEvents)
        : tasks(numberTasks), events(numberEvents), resources(numberResources), taskProduceByEvent(numberTasks),
          taskValuePerEvent(numberResources), resourceValueLost(numberResources) {}

    unsigned mean_time;

    bool fillsPrerequisite(unsigned task, unsigned resource, unsigned amount);
    bool fillsBorrow(unsigned task, unsigned resource, unsigned amount);
    bool fillsConsume(unsigned task, unsigned resource, unsigned amount);
};

extern GameRules RULES;

} // namespace BuildOrderOptimizer::State

#endif
