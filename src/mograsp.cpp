#include "../include/mograsp.h"

BuildOrder::Optimizer::Population BuildOrder::Optimizer::MOGRASP::optimize(GameState init, unsigned iterations) const
{
	AdaptativeGrid_Archiver population(archive_size, this, 32);//(ret);
	
	for (unsigned ii = 0; ii < iterations; ii++)
	{
		//std::cout << "\tMOGRASP: " << ii << "\n";
		#pragma omp parallel for num_threads(20)
		for (unsigned i = 0; i < creation_cycles; i++)
		{
			Solution n = create(init, *this, stop_chance);
			
			//make_valid(n, *this, init);

			#pragma omp critical
			{
				//std::cout << "\t\t"<<i <<"\n";
				population.insert(n);
			}
		}

		Population neighbors = local_search(neighborhood, population(), childs, *this, init);

		for (unsigned k = 0; k < neighbors.size(); k++)
			population.insert(neighbors[k]);
	}

	Population p = population();

	return p;
}