#include "../include/nsga2.h"

BuildOrder::Optimizer::Population BuildOrder::Optimizer::NSGA2::optimize(GameState initial, unsigned iterations) const
{
	NSGA2_Archiver pop(population_size,this);

	#pragma omp parallel for num_threads(20)
	for (unsigned i = 0; i < creation_cycles; i++)
	{
		Solution n = create(initial, *this, stop_chance);

		#pragma omp critical
		pop.insert(n);
	}

	for (unsigned i = 0; i < iterations; i++)
	{
		//std::cout << "\tNSGA-II: " << i << "\n";
		//std::cout << "\told " << pop.size() << "\n";

		unsigned max = pop.size();///2;

		//RECREATE STRAIN
		#pragma omp parallel for num_threads(16)
		for (unsigned p = 0; p < max; p++)
		{
			Population n = neighborhood(pop[p]);
			for (unsigned t = 0; t < n.size(); t++)
			{
				n[t].update(initial, maximum_time);

				make_valid(n[t], *this, initial);
				trim(n[t], *this, initial);

				#pragma omp critical
				pop.insert(n[t]);
			}
		}

		//std::cout << "\tnew " << pop.size() << "\n";
	}

	return pop();
}