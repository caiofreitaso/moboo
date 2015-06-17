#ifndef ANTSH
#define ANTSH

#include "agarchiver.h"
#include "create_solution.h"

namespace BuildOrder
{
	namespace Optimizer
	{
		class Ants : public Optimizer
		{
			unsigned population_size;
			unsigned ants;
			unsigned children;
			double a, b;
			double evaporation;
			public:
				Ants()
				: population_size(20),ants(5),children(1), a(0.5), b(0.5), evaporation(0.4)
				{ }
				Ants(double alpha, double beta)
				: population_size(20),ants(5),children(1), a(alpha), b(beta), evaporation(0.4)
				{ }
				Ants(unsigned c, double alpha, double beta)
				: population_size(20),ants(5),children(c), a(alpha), b(beta), evaporation(0.4)
				{ }
				Ants(unsigned p, unsigned a, unsigned c, double alpha, double beta, double e)
				: population_size(p),ants(a),children(c), a(alpha), b(beta), evaporation(e)
				{ }

				virtual Population optimize(GameState initial, unsigned iterations) const;
		};
	}
}

#endif