#ifndef ENGINE_INNERENGINE_H
#define ENGINE_INNERENGINE_H

#include <build-order/solution.h>
#include <build-order/state/game-state.h>

#include "time-helper.h"

namespace BuildOrderOptimizer::Engine {

bool maximumOverflow(unsigned task, State::GameState const &);
bool hasPrerequisites(unsigned task, State::GameState const &);

bool can(unsigned task, State::GameState const &);
bool can(TaskPointer t, State::GameState const &);

void afterStack(Data::Contiguous<unsigned> &, Data::Contiguous<unsigned> &, State::GameState const &);
bool prerequisiteInStack(Data::Contiguous<bool> &, Data::Contiguous<unsigned> &, unsigned);
void resourcesByEvents(Data::Contiguous<bool> &, State::GameState const &);
bool possible(BuildOrder &, State::GameState const &);
bool possible(unsigned, State::GameState const &);

void updateCosts(BuildOrder &, State::GameState &);
void updateBorrow(BuildOrder &, State::GameState &);

void buildWhatYouCan(BuildOrder &, State::GameState &, unsigned &last, RemainingList &listTime);

void aftermath(State::GameState &, unsigned type);

void updateOngoingTasks(State::GameState &, RemainingList &);
void updateOngoingEvents(State::GameState &, RemainingList &);

void update(State::GameState &, RemainingList &, RemainingList &);
unsigned nextTime(State::GameState &init, RemainingList &, RemainingList &, unsigned, unsigned);

} // namespace BuildOrderOptimizer::Engine
#endif
