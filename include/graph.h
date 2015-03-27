#ifndef GRAPHH
#define GRAPHH

#include "rules.h"
#include "disjointset.h"

namespace BuildOrder
{
	namespace Rules
	{
		enum relation_type
		{
			RT_COST,
			RT_CONSUME,
			RT_MAXIMUM,
			RT_BORROW,
			RT_PREREQUISITE
		};

		struct Dependency
		{
			relation_type type;
			MatrixRow<double> bonus;
			MatrixRow<double> event;
			MatrixRow<double> weight;
		};

		extern Graph::MultiGraph<Dependency> graph;

		std::vector<Dependency> value(unsigned needed, unsigned needs);

		void initGraph();

		inline double best_function_ever(double a, double b)
		{ return a * tanh(a/b); }
	}
}

#endif