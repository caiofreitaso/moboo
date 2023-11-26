#ifndef IO_READ_FILE_H
#define IO_READ_FILE_H

#include <build-order/optimizers/weighted-problem.h>
#include <build-order/state/game-state.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <utility>

namespace BuildOrderOptimizer::IO {

void ignoreComments(std::fstream &file, std::string &buffer);
void ignoreComments(std::fstream &file, std::string &buffer, std::stringstream &sstream);

std::pair<unsigned, unsigned> initializeRelation(std::stringstream &sstream);

void createRules(char const *s);
State::GameState createState(char const *s);
void initializeProblem(Optimizers::WeightedProblem &, char const *s);

} // namespace BuildOrderOptimizer::IO

#endif // IO_READ_FILE_H
