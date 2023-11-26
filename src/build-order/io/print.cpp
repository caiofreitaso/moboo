#include <build-order/io/print.h>

void
BuildOrderOptimizer::IO::print(BuildOrder const &b) {
    for (unsigned k = 0; k < b.size(); k++) {
        std::cout << b[k].task;
        if (b[k].delay)
            std::cout << "@" << b[k].delay;
        if (k < b.size() - 1)
            std::cout << ", ";
        else
            std::cout << "\n";
    }
}

void
BuildOrderOptimizer::IO::print(State::GameState const &s) {
    std::cout << "TIME: " << s.time << "\n";

    for (unsigned i = 0; i < s.resources.size(); i++) {
        if (s.resources[i].usable()) {
            std::cout << i << ": " << s.resources[i].usable() << "\n";
        }
    }
}

void
BuildOrderOptimizer::IO::print(Rules::DependencyGraph const &g) {
    for (unsigned i = 0; i < g.vertices(); i++) {
        std::cout << i << ":\n";
        auto index = g.find(i);

        if (index == g.size()) {
            continue;
        }

        auto it = g.begin() + index;

        for (auto &current : it->value.edges) {
            std::cout << "\t" << current.value().type << ": " << current.index << " = " << current.value.value << "\n";
        }
    }
}

std::string
BuildOrderOptimizer::IO::print(Optimizers::Problem const &p) {
    std::stringstream ret;
    static const char type[] = "Uqu";

    for (int i = 0; i < 3; i++) {
        auto &obj = p.objectives[i];
        auto c = type[i];

        for (auto &objective : obj.row) {
            if (objective.value == Optimizers::MINIMIZE) {
                ret << "m";
            } else {
                ret << "M";
            }

            ret << " " << c << objective.index << "\n";
        }
    }

    ret << "\n";
    ret << "t < " << p.maximum_time << "\n";

    for (int i = 0; i < 3; i++) {
        auto &obj = p.restrictions[i];
        auto c = type[i];

        for (auto &restriction : obj.row) {
            if (restriction.value.less_than != -1) {
                ret << c << restriction.index << " < " << restriction.value.less_than << "\n";
            }
            if (restriction.value.greater_than) {
                ret << c << restriction.index << " > " << restriction.value.greater_than << "\n";
            }
        }
    }

    return ret.str();
}

std::string
BuildOrderOptimizer::IO::print(Optimizers::Problem const &p, Solution const &s) {
    std::stringstream ret;

    ret << s.final_state.time << " ";

    for (auto &objective : p.objectives[Optimizers::Problem::RC_USABLE].row) {
        if (objective.value == Optimizers::MAXIMIZE) {
            ret << "-";
        }

        ret << s.final_state.resources[objective.index].usable() << " ";
    }

    for (auto &objective : p.objectives[Optimizers::Problem::RC_QUANTITY].row) {
        if (objective.value == Optimizers::MAXIMIZE) {
            ret << "-";
        }

        ret << s.final_state.resources[objective.index].quantity << " ";
    }

    for (auto &objective : p.objectives[Optimizers::Problem::RC_USED].row) {
        if (objective.value == Optimizers::MAXIMIZE) {
            ret << "-";
        }

        ret << s.final_state.resources[objective.index].used << " ";
    }

    return ret.str();
}
