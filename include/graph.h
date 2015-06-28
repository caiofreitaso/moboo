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

			bool operator==(Dependency d) const
			{ return type == d.type; }
		};

		class MultiGraph : public Graph::MultiGraph<Dependency>
		{
			virtual void print() const
			{
				for (unsigned i = 0; i < vertices(); i++)
				{
					std::cout << i << ":\n";
					auto it = nodes.find(i);
					
					if (it == nodes.row.end())
						continue;

					for (unsigned j = 0; j < it->value.size(); j++)
						std::cout << "\t" << it->value[j].value().type << ": " << it->value[j].index << " = " << it->value[j].value.value << "\n";
				}
			}
		};
		extern MultiGraph graph;

		contiguous<Dependency> value(unsigned needed, unsigned needs);

		void initGraph();

		inline double best_function_ever(double a, double b)
		{ return a * tanh(a/b); }
	}
}

#endif