#include "initializer.h"

using namespace BuildOrderOptimizer::State;

void
BuildOrderOptimizer::File::ignoreComments(std::fstream &file, std::string &buffer) {
    do {
        std::getline(file, buffer);
    } while (buffer[0] == '#' || buffer.size() == 0);
}

void
BuildOrderOptimizer::File::ignoreComments(std::fstream &file, std::string &buffer, std::stringstream &sstream) {
    ignoreComments(file, buffer);

    sstream.clear();
    sstream.str(buffer);
}

std::pair<unsigned, unsigned>
BuildOrderOptimizer::File::initializeRelation(std::stringstream &sstream) {
    unsigned index, value;
    long p = (long)sstream.tellg();

    sstream >> index;
    while (sstream.fail()) {
        sstream.clear();
        sstream.seekg(p + 1);
        sstream >> index;
    }

    p = (long)sstream.tellg();
    sstream >> value;
    while (sstream.fail()) {
        sstream.clear();
        sstream.seekg(p + 1);
        sstream >> value;
    }

    return std::make_pair(index, value);
}

BuildOrderOptimizer::State::GameState
BuildOrderOptimizer::File::createState(char const *filename) {
    std::fstream file;
    std::string buffer;
    std::stringstream ss;
    auto state = State::createState();

    file.open(filename);

    unsigned idx;
    unsigned v;

    ignoreComments(file, buffer, ss);

    // initial state
    while (!ss.eof()) {
        auto pair = File::initializeRelation(ss);
        auto current_resource = pair.first;
        auto value = pair.second;

        state.resources[current_resource].quantity += value;

        for (unsigned k = 0; k < RULES.events.size(); k++) {
            auto current_event = RULES.events[k];

            if (current_event.trigger != current_resource) {
                continue;
            }

            for (unsigned q = 0; q < value; q++) {
                if (current_event.time) {
                    EventPointer event(state.time, k);
                    state.resources[current_resource].events.push_back(event);
                } else {
                    state.applyEvent(k);
                }
            }
        }
    }

    // resources consumed
    std::getline(file, buffer);
    ss.clear();
    ss.str(buffer);
    while (!ss.eof()) {
        auto pair = initializeRelation(ss);
        auto current_resource = pair.first;
        auto value = pair.second;

        for (unsigned i = 0; i < value; i++) {
            state.consumeResource(current_resource);
        }
    }

    file.close();

    return state;
}

void
BuildOrderOptimizer::File::createRules(char const *filename) {
    std::fstream file;
    std::string buffer;
    std::stringstream sstream;

    file.open(filename);

    // ALLOCATING
    ignoreComments(file, buffer, sstream);

    unsigned res_sz, tks_sz, evt_sz;

    sstream >> res_sz;
    sstream >> tks_sz;
    sstream >> evt_sz;
    RULES = GameRules(res_sz, tks_sz, evt_sz);

    initializeResources(file, buffer, sstream);
    initializeMeanTime(file, buffer, sstream);
    initializeTasks(file, buffer, sstream);

    if (RULES.events.size()) {
        initializeEvents(file, buffer, sstream);
    }

    file.close();

    initializeSecondaryAttributes();
}

void
BuildOrderOptimizer::File::initializeOptimizer(Optimizers::Optimizer &o, char const *f) {
    std::fstream file;
    std::string buffer;
    std::stringstream ss;

    file.open(f);

    // OBJECTIVES
    do {
        std::getline(file, buffer);

        Optimizers::Objective min = (buffer[0] == 'm') ? Optimizers::MINIMIZE : Optimizers::MAXIMIZE;

        if (buffer[0] != 'm' && buffer[0] != 'M')
            break;

        unsigned l = 1;
        while (buffer[l] == ' ') {
            l++;
        }

        unsigned target;
        switch (buffer[l]) {
        case 'U':
            target = 0;
            break;
        case 'q':
            target = 1;
            break;
        case 'u':
            target = 2;
            break;
        default:
            file.close();
            return;
        }

        l++;
        ss.str(&buffer[l]);

        unsigned index;
        ss >> index;
        ss.clear();

        o.objectives[target].set(index, min);

    } while (buffer[0] == 'm' || buffer[0] == 'M');

    while (buffer.length() == 0) {
        std::getline(file, buffer);
    }

    // WEIGHTS
    {
        o.weights.reserve(o.numberObjectives() + 1);
        o.weights.resize(0);

        unsigned l = 1, s = 0;
        for (unsigned k = 0; k < o.numberObjectives() + 1; k++) {
            if ((buffer[s] >= '0' && buffer[s] <= '9') || buffer[s] == '-') {
                while (buffer[l] != ' ' && buffer[l] != '\0') {
                    l++;
                }
                ss.str(&buffer[s]);

                double weight;
                ss >> weight;
                ss.clear();

                o.weights.push_back(weight);

                s = l;
                while (buffer[s] == ' ') {
                    s++;
                    l++;
                }
            } else {
                if (k > 0) {
                    file.close();
                    return;
                } else
                    break;
            }
        }
        if ((buffer[0] >= '0' && buffer[0] <= '9') || buffer[0] == '-') {
            std::getline(file, buffer);
        }
    }

    while (buffer.length() == 0) {
        std::getline(file, buffer);
    }

    // TIME RESTRICTION
    {
        if (buffer[0] != 't') {
            file.close();
            return;
        }

        unsigned l = 1;
        while (buffer[l] == ' ' || buffer[l] == '<')
            l++;

        ss.str(&buffer[l]);
        unsigned mtime;
        ss >> mtime;
        ss.clear();

        o.maximum_time = mtime;
    }

    do {
        std::getline(file, buffer);

        unsigned l = 0;
        unsigned target;
        switch (buffer[l]) {
        case 'U':
            target = 0;
            break;
        case 'q':
            target = 1;
            break;
        case 'u':
            target = 2;
            break;
        default:
            file.close();
            return;
        }

        l++;
        ss.str(&buffer[l]);

        unsigned index;
        ss >> index;
        ss.clear();

        while (buffer[l] != '<' && buffer[l] != '>') {
            l++;
        }

        bool less = (buffer[l] == '<');
        l++;

        ss.str(&buffer[l]);

        unsigned value;
        ss >> value;
        ss.clear();

        Optimizers::Restriction r = o.restrictions[target].get(index);

        if (less) {
            r.less_than = value;
        } else {
            r.greater_than = value;
        }

        o.restrictions[target].set(index, r);

        if (file.eof()) {
            break;
        }
    } while (buffer[0] == 'U' || buffer[0] == 'u' || buffer[0] == 'q');
}
