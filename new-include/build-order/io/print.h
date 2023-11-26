#ifndef IO_PRINT_H
#define IO_PRINT_H

#include <build-order/optimizers/problem.h>
#include <build-order/rules/dependency-graph.h>
#include <build-order/solution.h>
#include <build-order/state/game-state.h>
#include <build-order/task-pointer.h>

#include <iostream>
#include <sstream>

namespace BuildOrderOptimizer::IO {

void print(BuildOrder const &);
void print(State::GameState const &);
void print(Rules::DependencyGraph const &);
std::string print(Optimizers::Problem const &);
std::string print(Optimizers::Problem const &, Solution const &);

} // namespace BuildOrderOptimizer::IO

#endif // IO_PRINT_H
