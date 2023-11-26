#include "initializer.h"

using namespace BuildOrderOptimizer::State;
using namespace BuildOrderOptimizer::Data;
using namespace BuildOrderOptimizer::IO;
using namespace BuildOrderOptimizer::Rules;
using ResRule = BuildOrderOptimizer::Rules::Resource;

void
initializeResources(std::fstream &file, std::string &buffer, std::stringstream &sstream) {
    initializeOverallMaximum(file, buffer, sstream);
    initializeMaximumPerResource(file, buffer, sstream);
    initializeEquivalence(file, buffer, sstream);
}

void
initializeTasks(std::fstream &file, std::string &buffer, std::stringstream &sstream) {
    initializeTaskCosts(file, buffer, sstream);
    initializeTaskPrerequisites(file, buffer, sstream);
    initializeTaskConsume(file, buffer, sstream);
    initializeTaskBorrow(file, buffer, sstream);
    initializeTaskProduce(file, buffer, sstream);
}

void
initializeEvents(std::fstream &file, std::string &buffer, std::stringstream &sstream) {
    initializeEventTime(file, buffer, sstream);
    initializeEventBonus(file, buffer, sstream);
    initializeEventTrigger(file, buffer, sstream);
}

void
initializeSecondaryAttributes() {
    auto &events = RULES.events;
    auto &tasks = RULES.tasks;

    for (unsigned i = 0; i < RULES.events.size(); i++) {
        auto &bonus = events[i].bonus.row;
        for (unsigned j = 0; j < bonus.size(); j++) {
            unsigned index = bonus[j].index;

            RULES.resourceValueLost[events[i].trigger].set(index, 1);

            for (unsigned k = 0; k < RULES.tasks.size(); k++) {
                auto &task = RULES.tasks[k];
                if (task.produce.get(events[i].trigger)) {
                    RULES.taskValuePerEvent[index].set(k, 1);
                    RULES.taskProduceByEvent[k].set(index, 1);
                }
            }
        }
    }
}

template <class T>
void
initializeMultiline(std::fstream &file, std::string &buffer, std::stringstream &ss, Contiguous<T> &array,
                    void (*fn)(T &, unsigned, unsigned)) {
    ignoreComments(file, buffer);
    for (unsigned i = 0; i < array.size(); i++) {
        T &current = array[i];

        ss.clear();
        ss.str(buffer);

        while (!ss.eof()) {
            auto pair = initializeRelation(ss);
            fn(current, pair.first, pair.second);
        }

        std::getline(file, buffer);
    }
}

void
initializeOverallMaximum(std::fstream &file, std::string &buffer, std::stringstream &ss) {
    ignoreComments(file, buffer, ss);

    while (!ss.eof()) {
        auto pair = initializeRelation(ss);
        RULES.resources[pair.first].overall_maximum = pair.second;
    }
}

void
initializeMaximumPerResource(std::fstream &file, std::string &buffer, std::stringstream &sstream) {
    initializeMultiline<ResRule>(file, buffer, sstream, RULES.resources,
                                 [](ResRule &r, unsigned j, unsigned v) { r.maximum_per_resource.set(j, v); });
}

void
initializeEquivalence(std::fstream &file, std::string &buffer, std::stringstream &sstream) {
    initializeMultiline<ResRule>(file, buffer, sstream, RULES.resources,
                                 [](ResRule &r, unsigned j, unsigned v) { r.equivalence.set(j, v); });
}

void
initializeTaskCosts(std::fstream &file, std::string &buffer, std::stringstream &sstream) {
    initializeMultiline<Task>(file, buffer, sstream, RULES.tasks,
                              [](Task &r, unsigned j, unsigned v) { r.costs.set(j, v); });
}

void
initializeTaskPrerequisites(std::fstream &file, std::string &buffer, std::stringstream &sstream) {
    initializeMultiline<Task>(file, buffer, sstream, RULES.tasks,
                              [](Task &r, unsigned j, unsigned v) { r.prerequisite.set(j, v); });
}

void
initializeTaskConsume(std::fstream &file, std::string &buffer, std::stringstream &sstream) {
    initializeMultiline<Task>(file, buffer, sstream, RULES.tasks,
                              [](Task &r, unsigned j, unsigned v) { r.consume.set(j, v); });
}

void
initializeTaskBorrow(std::fstream &file, std::string &buffer, std::stringstream &sstream) {
    initializeMultiline<Task>(file, buffer, sstream, RULES.tasks,
                              [](Task &r, unsigned j, unsigned v) { r.borrow.set(j, v); });
}

void
initializeTaskProduce(std::fstream &file, std::string &buffer, std::stringstream &sstream) {
    initializeMultiline<Task>(file, buffer, sstream, RULES.tasks,
                              [](Task &r, unsigned j, unsigned v) { r.produce.set(j, v); });
}

void
initializeEventBonus(std::fstream &file, std::string &buffer, std::stringstream &sstream) {
    initializeMultiline<Event>(file, buffer, sstream, RULES.events,
                               [](Event &r, unsigned j, unsigned v) { r.bonus.set(j, v); });
}

void
initializeMeanTime(std::fstream &file, std::string &buffer, std::stringstream &sstream) {
    ignoreComments(file, buffer, sstream);
    RULES.mean_time = 0;

    for (unsigned i = 0; i < RULES.tasks.size(); i++) {
        sstream >> RULES.tasks[i].time;
        RULES.mean_time += RULES.tasks[i].time;
    }
    RULES.mean_time /= RULES.tasks.size();
}

void
initializeEventTime(std::fstream &file, std::string &buffer, std::stringstream &sstream) {
    ignoreComments(file, buffer, sstream);
    for (unsigned i = 0; i < RULES.events.size(); i++) {
        sstream >> RULES.events[i].time;
    }
}

void
initializeEventTrigger(std::fstream &file, std::string &buffer, std::stringstream &sstream) {
    ignoreComments(file, buffer, sstream);
    for (unsigned i = 0; i < RULES.events.size(); i++) {
        sstream >> RULES.events[i].trigger;
    }
}
