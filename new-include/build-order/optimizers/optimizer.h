#ifndef OPTIMIZERS_OPTIMIZER_H
#define OPTIMIZERS_OPTIMIZER_H

#include <build-order/data/contiguous.h>
#include <build-order/data/matrix/sparse-row.h>
#include <build-order/solution.h>
#include <build-order/state/game-state.h>

#include "neighboorhoods.h"
#include "objective.h"
#include "restriction.h"

namespace BuildOrderOptimizer::Optimizers {

class Optimizer {
    Data::Contiguous<double> _objV;
    Data::Contiguous<Data::Matrix::SparseRow<double>> _incV;
    Data::Contiguous<Data::Matrix::SparseRow<double>> _decV;

  public:
    bool time_as_objective;
    bool use_weights;
    double stop_chance;

    unsigned maximum_time;
    Data::Matrix::SparseRow<Restriction> restrictions[3];
    Data::Matrix::SparseRow<Objective> objectives[3];
    Data::Contiguous<double> weights;
    Population (*neighborhood)(Solution const &);

    Optimizer() : time_as_objective(true), stop_chance(1), maximum_time(0), neighborhood(one_swap) {}

    virtual Population optimize(State::GameState, unsigned) const = 0;

    bool dominates(Solution a, Solution b) const;
    bool valid(Solution s) const;
    unsigned producesMaximize(unsigned task) const;
    unsigned producesGreaterThan(unsigned task) const;
    Population nonDominated(Population p) const;
    unsigned numberObjectives() const;
    unsigned numberRestrictions() const;

    std::string
    print() const {
        std::stringstream ret;
        static const char type[] = "Uqu";

        for (int i = 0; i < 3; i++) {
            auto &obj = objectives[i];
            auto c = type[i];

            for (auto &objective : obj.row) {
                if (objective.value == MINIMIZE) {
                    ret << "m";
                } else {
                    ret << "M";
                }

                ret << " " << c << objective.index << "\n";
            }
        }

        ret << "\n";
        ret << "t < " << maximum_time << "\n";

        for (int i = 0; i < 3; i++) {
            auto &obj = restrictions[i];
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
    print(Solution const &s) const {
        std::stringstream ret;
        ret << s.final_state.time << " ";
        for (int i = 0; i < 3; i++) {
            for (auto &objective : objectives[i].row) {
                if (objective.value == MAXIMIZE) {
                    ret << "-";
                }

                ret << s.final_state.resources[objective.index].usable() << " ";
            }
        }

        return ret.str();
    }

    void update();

    Data::Contiguous<double> initialMap(double, double, State::GameState) const;

    Data::Contiguous<unsigned> toVector(Solution) const;
    Data::Contiguous<Data::Contiguous<unsigned>> toVector(Population) const;

    Data::Contiguous<double> toDVector(Solution) const;
    Data::Contiguous<Data::Contiguous<double>> toDVector(Population) const;

    Data::Contiguous<bool> objectivesVector() const;
};
} // namespace BuildOrderOptimizer::Optimizers
#endif
