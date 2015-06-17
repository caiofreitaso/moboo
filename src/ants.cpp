#include "../include/ants.h"

BuildOrder::Optimizer::Population BuildOrder::Optimizer::Ants::optimize(GameState initial, unsigned iterations) const
{
	unsigned size = population_size;
	AdaptativeGrid_Archiver pop(size,this, 32);
	Graph::MultiGraph<unsigned> phero;

	phero.vertices(Rules::tasks.size()+1);

	for (unsigned i = 0; i < Rules::tasks.size(); i++)
		for (unsigned j = 0; j < Rules::tasks.size(); j++)
			phero.insert(i,j,1,0);
	for (unsigned i = 0; i < Rules::tasks.size(); i++)
		phero.insert(Rules::tasks.size(),i,1,0);

	for (unsigned i = 0; i < iterations; i++)
	{
		#pragma omp parallel for
		for (unsigned ant = 0; ant < ants; ant++)
		{
			Solution s;
			s.final_state = initial;

			double obj_m = Creation::getObjective();
			double res_m = Creation::getRestriction();

			double delta_o = Creation::getDelta_O();
			double delta_r = Creation::getDelta_R();

			double max_o = obj_m * pow(delta_o,30);
			double max_r = res_m * pow(delta_r,30);

			if (max_o > max_r)
				std::swap(max_o, max_r);

			unsigned rem = 0, rem_count = 0;
			
			while (!valid(s))
			{
				std::vector<double> heuristic = taskWeights(s.final_state, *this, obj_m,res_m);
				std::vector<double> heuristic_power(heuristic.size());
				std::vector<double> pheromones(heuristic.size());
				
				if (res_m < max_r)
					res_m *= delta_r;
				if (obj_m < max_o)
					obj_m *= delta_o;

				for (unsigned k = 0; k < heuristic.size(); k++)
					heuristic_power[k] = (heuristic[k] != DOUBLE_NINF) ? pow(heuristic[k],a) : heuristic[k];

				if (s.orders.size())
					for (unsigned k = 0; k < heuristic.size(); k++)
						pheromones[k] = phero.get(s.orders.back().task).get(k).value;
				else
					for (unsigned k = 0; k < heuristic.size(); k++)
						pheromones[k] = phero.get(phero.vertices()-1).get(k).value;

				res_m = res_m;

				for (unsigned k = 0; k < heuristic.size(); k++)
					pheromones[k] = pow(pheromones[k],b);

				res_m = res_m;

				for (unsigned k = 0; k < heuristic.size(); k++)
					heuristic[k] = heuristic_power[k] * pheromones[k];

				double sum = 0;		
				unsigned count = 0;
				for (unsigned t = 0; t < Rules::tasks.size(); t++)
					if (heuristic[t] != DOUBLE_NINF)
						if (heuristic[t] > 0)
						{
							sum += heuristic[t];
							count++;
						}

				if (count == 0)
					for (unsigned t = 0; t < Rules::tasks.size(); t++)
						if (heuristic[t] == 0)
						{
							heuristic[t] = 1;
							sum++;
						}

				double last = 0;
				for (unsigned t = 0; t < Rules::tasks.size(); t++)
					if (heuristic[t] != DOUBLE_NINF)
						if (heuristic[t] > 0)
						{
							heuristic[t] /= sum;
							double tmp = heuristic[t];
							
							heuristic[t] += last;
							last += tmp;
						}

				double dice = drng();
				bool equal = false;
				for (unsigned k = 0; k < heuristic.size(); k++)
					if (dice < heuristic[k])
					{
						s.orders.push_back(TaskPointer(k));
						unsigned sz = s.orders.size();
						s.update(initial, maximum_time);
						equal = sz == s.orders.size();
						break;
					}

				if (!equal)
				{
					if (rem >= s.orders.size())
					{
						rem = 0;
						rem_count++;
					}
					else
						rem++;
					
					s.orders.erase(s.orders.begin()+s.orders.size()-rem, s.orders.end());
					s.update(initial, maximum_time);
					s.final_state = initial;
				}
			}

			if (valid(s))
			{
				Population solution(1);
				solution.push_back(s);

				Population ls = local_search(neighborhood, solution, children, *this, initial);
				for (auto n : ls)
					#pragma omp critical
					pop.insert(n);
			}
		}

		for (auto n : pop)
		{
			unsigned t0 = n.orders[0].task;
			phero[phero.vertices()-1].value[t0].value.value *= evaporation;
			phero[phero.vertices()-1].value[t0].value.value++;
			for (unsigned k = 1; k < n.orders.size(); k++)
			{
				phero[n.orders[k-1].task].value[n.orders[k].task].value.value *= evaporation;
				phero[n.orders[k-1].task].value[n.orders[k].task].value.value++;
			}
		}
	}

	return pop();
}