#ifndef FILE_READ_FILE_H
#define FILE_READ_FILE_H

#include <build-order/optimizers/optimizer.h>
#include <build-order/state/game-state.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <utility>

namespace BuildOrderOptimizer::File {

void ignoreComments(std::fstream &file, std::string &buffer);
void ignoreComments(std::fstream &file, std::string &buffer, std::stringstream &sstream);

std::pair<unsigned, unsigned> initializeRelation(std::stringstream &sstream);

void createRules(char const *s);
State::GameState createState(char const *s);
void initializeOptimizer(Optimizers::Optimizer &, char const *s);

} // namespace BuildOrderOptimizer::File

#endif // FILE_READ_FILE_H
