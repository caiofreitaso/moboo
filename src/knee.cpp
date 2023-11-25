#include "../include/knee.h"

BuildOrder::Optimizer::Population BuildOrder::Optimizer::Knee::optimize(GameState initial, unsigned iterations) const
{
	Knee_Archiver pop(population_size,this);

	#pragma omp parallel for num_threads(16)
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

		Population children;

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
				children.push_back(n[t]);
			}
		}

		for (unsigned k = 0; k < children.size(); k++)
			pop.insert(children[k]);

		//std::cout << "\tnew " << pop.size() << "\n";
	}

	return pop();
}