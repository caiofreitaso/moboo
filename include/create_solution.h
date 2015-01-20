#ifndef CREATESOLUTIONH
#define CREATESOLUTIONH

#include "optimizer.h"

namespace BuildOrder
{
	namespace Optimizer
	{
		std::vector<double> mapGoals(GameState, Optimizer const&,
									 double objective_multiplier = 3.0,
									 double restriction_multiplier = 4.0,
									 double prerequisite_multiplier = 2.0,
									 double cost_multiplier = 1.0,
									 double maximum_multiplier = 1.0);

		bool nextTask(Solution&, GameState, Optimizer const&,
					  double objective_multiplier = 3.0,
					  double restriction_multiplier = 4.0,
					  double prerequisite_multiplier = 2.0,
					  double cost_multiplier = 1.0,
					  double maximum_multiplier = 1.0);

		Solution create(GameState, Optimizer const&, double stop = 0.1);

		void make_valid(Solution&, Optimizer const&, GameState const&);
		void trim(Solution&, Optimizer const&, GameState const&);
	}
}

#endif