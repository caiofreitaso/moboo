#include "../include/nsga2.h"

BuildOrder::Optimizer::Population BuildOrder::Optimizer::NSGA2::optimize(GameState initial, unsigned iterations) const
{
	NSGA2_Archiver pop(population_size,this);

	for (unsigned i = 0; i < creation_cycles; i++)
	{
		Solution n = create(initial,*this,1);
		
		for (unsigned d = 0; !valid(n); d++)
		{
			if (d > n.orders.size())
			{
				n.orders.clear();
				d = 0;
			} else
				n.orders.erase(n.orders.begin()+n.orders.size()-d, n.orders.end());

			double obj_m = 2.5;
			double res_m = 3.0;
			while(nextTask(n, initial, *this, obj_m, res_m))
			{
				obj_m *= 1.1;
				res_m *= 1.5;

				if (valid(n))
					break;
			}
		}

		pop.insert(n);
	}

	for (unsigned i = 0; i < iterations; i++)
	{
		unsigned max = pop.size();///2;

		//RECREATE STRAIN
		for (unsigned p = 0; p < max; p++)
		{
			Population n = neighborhood(pop[p]);
			for (unsigned t = 0; t < n.size(); t++)
			{
				n[t].update(initial, maximum_time);

				for (unsigned d = 0; !valid(n[t]); d++)
				{
					if (d > n[t].orders.size())
					{
						n[t].orders.clear();
						d = 0;
					} else
						n[t].orders.erase(n[t].orders.begin()+n[t].orders.size()-d, n[t].orders.end());

					double obj_m = 2.5;
					double res_m = 3.0;
					while(nextTask(n[t], initial, *this, obj_m, res_m))
					{
						obj_m *= 1.1;
						res_m *= 1.5;

						if (valid(n[t]))
							break;
					}
				}

				pop.insert(n[t]);
			}
		}
	}

	return pop();
}