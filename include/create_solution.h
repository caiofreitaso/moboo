#ifndef CREATESOLUTIONH
#define CREATESOLUTIONH

#include <bitset>
#include <limits>
#include "optimizer.h"

const double DOUBLE_NINF = -std::numeric_limits<double>::infinity();

namespace BuildOrder
{
	namespace Optimizer
	{
		namespace Creation
		{
			extern double objective_minimum;
			extern double objective_maximum;

			extern double restriction_minimum;
			extern double restriction_maximum;

			extern double delta_o_minimum;
			extern double delta_o_maximum;

			extern double delta_r_minimum;
			extern double delta_r_maximum;

			inline double getObjective()
			{ return drng(objective_minimum,objective_maximum); }

			inline double getRestriction()
			{ return drng(restriction_minimum,restriction_maximum); }

			inline double getDelta_O()
			{ return drng(delta_o_minimum,delta_o_maximum); }

			inline double getDelta_R()
			{ return drng(delta_r_minimum,delta_r_maximum); }
		}

		namespace Fix
		{
			extern double objective_minimum;
			extern double objective_maximum;

			extern double restriction_minimum;
			extern double restriction_maximum;

			extern double delta_o_minimum;
			extern double delta_o_maximum;

			extern double delta_r_minimum;
			extern double delta_r_maximum;

			inline double getObjective()
			{ return drng(objective_minimum,objective_maximum); }

			inline double getRestriction()
			{ return drng(restriction_minimum,restriction_maximum); }

			inline double getDelta_O()
			{ return drng(delta_o_minimum,delta_o_maximum); }

			inline double getDelta_R()
			{ return drng(delta_r_minimum,delta_r_maximum); }
		}

		void passWeight(double value, unsigned task,
						std::vector<Rules::Forest> const& g,
						std::vector<double> const& w,
						std::vector<double>& final,
						std::vector<bool> const& p,
						std::vector<std::bitset<5> >& done);

		std::vector<double> taskWeights(GameState, Optimizer const&,
										double objective_multiplier = 3.0,
										double restriction_multiplier = 4.0,
										double prerequisite_multiplier = 2.0,
										double cost_multiplier = 1.5,
										double maximum_multiplier = 1.0);

		void mapPrerequisites(Rules::Forest::c_it,
							  GameState,
							  std::vector<double> const&,
							  std::vector<double>&);

		std::vector<double> mapGoals(GameState, Optimizer const&,
									 double objective_multiplier = 3.0,
									 double restriction_multiplier = 4.0,
									 double prerequisite_multiplier = 2.0,
									 double cost_multiplier = 1.5,
									 double maximum_multiplier = 1.0);

		bool nextTask(Solution&, GameState, Optimizer const&,
					  double objective_multiplier = 3.0,
					  double restriction_multiplier = 4.0,
					  double prerequisite_multiplier = 2.0,
					  double cost_multiplier = 1.5,
					  double maximum_multiplier = 1.0);

		Solution create(GameState, Optimizer const&, double stop = 0.1);

		void make_valid(Solution&, Optimizer const&, GameState const&);
		void trim(Solution&, Optimizer const&, GameState const&);
	}
}

#endif