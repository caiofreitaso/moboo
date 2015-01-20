#include "../include/mograsp.h"

BuildOrder::Optimizer::Population BuildOrder::Optimizer::MOGRASP::optimize(GameState init, unsigned iterations) const
{
	AdaptativeGrid_Archiver population(creation_cycles, this, 32);//(ret);
	
	for (unsigned ii = 0; ii < iterations; ii++)
	{
		std::cout << ii << ": \t" << population.size() << "\n";

		//#pragma omp parallel for num_threads(20)
		for (unsigned i = 0; i < creation_cycles; i++)
		{
			Solution n = create(init,*this,1);
			
			make_valid(n, *this, init);

			#pragma omp critical
			population.insert(n);
		}

		std::cout << "\t" << population.size() << "\n";

		unsigned old_size = 0;
		unsigned count = 1;
		while(count)
		{
			old_size = population.size();
			
			unsigned size = old_size/parents;

			Population neighbors;

			#pragma omp parallel for num_threads(16)
			for (unsigned k = 0; k < size; k++)
				for (unsigned c = 0; c < childs; c++)
				{
					Population n = neighborhood(population[k]);
					for (unsigned t = 0; t < n.size(); t++)
					{
						n[t].update(init, maximum_time);

						trim(n[t], *this, init);
						make_valid(n[t], *this, init);

						#pragma omp critical
						neighbors.push_back(n[t]);
					}
				}
			for (unsigned k = 0; k < neighbors.size(); k++)
				population.insert(neighbors[k]);

			if (old_size == population.size())
				count--;
		}

		std::cout << "\t" << population.size() << "\n";
	}

	return population();
}