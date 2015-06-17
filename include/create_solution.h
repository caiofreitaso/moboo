#ifndef CREATESOLUTIONH
#define CREATESOLUTIONH

#include <cmath>
#include <bitset>
#include <limits>
#include <queue>
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
						Rules::MultiGraph const& g,
						std::vector<double>& final,
						std::vector<bool> const& p,
						std::vector<std::bitset<5> >& done);

		void pruneGraph(Rules::MultiGraph& g,
						GameState initial,
						Rules::relation_type r,
						bool (*func)(unsigned, unsigned, unsigned));
		void getValues(Rules::MultiGraph& g, GameState initial,
					   std::vector<double> value);

		std::vector<double> taskWeights(GameState, Optimizer const&,
										double objective_multiplier = 3.0,
										double restriction_multiplier = 4.0,
										double prerequisite_multiplier = 2.0,
										double cost_multiplier = 0.5,
										double maximum_multiplier = 1.0);

		bool nextTask(Solution&, GameState, Optimizer const&,
					  double objective_multiplier = 3.0,
					  double restriction_multiplier = 4.0,
					  double prerequisite_multiplier = 2.0,
					  double cost_multiplier = 0.5,
					  double maximum_multiplier = 1.0);

		Solution create(GameState, Optimizer const&, double stop = 0.1);
		Solution create_exact(GameState const&, Optimizer const&);
		Solution branchnbound(GameState const&, Solution, std::vector<unsigned> const&, unsigned, unsigned, unsigned, unsigned, unsigned);
		void getprerequisites(unsigned, std::vector<unsigned>&, Rules::MultiGraph const&);

		void make_valid(Solution&, Optimizer const&, GameState const&);
		void trim(Solution&, Optimizer const&, GameState const&);

		Population local_search(Population (*neighborhood)(Solution const&), Population const& p, unsigned childs, Optimizer const& opt, GameState init);
		Population local_search(Population (*neighborhood)(Solution const&), Solution const& p, unsigned childs, Optimizer const& opt, GameState init);
	}
}

#endif