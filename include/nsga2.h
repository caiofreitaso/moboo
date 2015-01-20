#ifndef NSGA2H
#define NSGA2H

#include "nsga2-archiver.h"
#include "create_solution.h"

namespace BuildOrder
{
	namespace Optimizer
	{

		class NSGA2 : public Optimizer
		{
			public:
				unsigned creation_cycles;
				unsigned population_size;

				NSGA2()
				: creation_cycles(100),population_size(20)
				{ }

				NSGA2(unsigned pop)
				: creation_cycles(100),population_size(pop)
				{ }

				NSGA2(unsigned pop, unsigned cc)
				: creation_cycles(cc),population_size(pop)
				{ }

				virtual Population optimize(GameState initial, unsigned iterations) const;
		};
	}
}

#endif