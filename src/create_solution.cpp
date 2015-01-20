#include "../include/create_solution.h"

std::vector<double> BuildOrder::Optimizer::mapGoals(GameState initial, Optimizer const& solver,
	double objective_multiplier, double restriction_multiplier, double prerequisite_multiplier,
	double cost_multiplier, double maximum_multiplier)
{
	struct step
	{
		unsigned from;
		unsigned to;
	};

	std::vector<double> taskValue(Rules::tasks.size(), 0);
	std::vector<step> next_step;

	//INITIAL MAP
	for (unsigned t = 0; t < Rules::tasks.size(); t++)
	{
		//RESOURCE BONUS/PENALTY
		for (unsigned p = 0; p < Rules::resources.size(); p++)
		{
			double resource_reduce = 0;
			double resource_increase = 0;

			resource_reduce += Rules::tasks[t].costs.get(p) > 0 ? 1 : 0;
			resource_reduce += Rules::tasks[t].consume.get(p) ? 1 : 0;
			
			resource_increase += Rules::tasks[t].costs.get(p) < 0 ? 1 : 0;
			resource_increase += Rules::tasks[t].produce.get(p) ? 1 : 0;

			resource_increase += Rules::taskValuePerEvent[p].get(t);

			if (resource_reduce > 0 || resource_increase > 0)
			{
				//OBJECTIVES
				{
					double objective_value = 0;

					if (solver.objectives[0].get(p))
					{
						if (solver.objectives[0].get(p) == MINIMIZE)
						{
							objective_value += resource_reduce;
							objective_value -= resource_increase;
						}
						else
						{
							objective_value -= resource_reduce;
							objective_value += resource_increase;
						}
					}

					if (solver.objectives[1].get(p))
					{
						if (solver.objectives[1].get(p) == MINIMIZE)
							objective_value -= resource_increase;
						else
							objective_value += resource_increase;
					}

					if (solver.objectives[2].get(p))
					{
						if (solver.objectives[2].get(p) == MINIMIZE)
							objective_value -= resource_reduce;
						else
							objective_value += resource_reduce;
					}

					taskValue[t] += objective_value * objective_multiplier;
				}
				
				//RESTRICTIONS
				{
					double restriction_value = 0;

					if (solver.restrictions[0].get(p))
					{
						unsigned final = initial.resources[p].usable();

						if (solver.restrictions[0].get(p).less_than)
							if (final >= solver.restrictions[0].get(p).less_than)
							{
								restriction_value += resource_reduce;
								restriction_value -= resource_increase;
							}
						
						if (solver.restrictions[0].get(p).greater_than)
							if (final <= solver.restrictions[0].get(p).greater_than)
							{
								restriction_value -= resource_reduce;
								restriction_value += resource_increase;
							}
					}

					if (solver.restrictions[1].get(p))
					{
						unsigned final = initial.resources[p].quantity;
						
						if (solver.restrictions[1].get(p).less_than)
							if (final >= solver.restrictions[1].get(p).less_than)
							{
								restriction_value += resource_reduce;
								restriction_value -= resource_increase;
							}
						
						if (solver.restrictions[1].get(p).greater_than)
							if (final <= solver.restrictions[1].get(p).greater_than)
							{
								restriction_value -= resource_reduce;
								restriction_value += resource_increase;
							}
					}

					if (solver.restrictions[2].get(p))
					{
						unsigned final = initial.resources[p].used;
						
						if (solver.restrictions[2].get(p).less_than)
							if (final >= solver.restrictions[2].get(p).less_than)
							{
								restriction_value += resource_reduce;
								restriction_value -= resource_increase;
							}
						
						if (solver.restrictions[2].get(p).greater_than)
							if (final <= solver.restrictions[2].get(p).greater_than)
							{
								restriction_value -= resource_reduce;
								restriction_value += resource_increase;
							}
					}

					taskValue[t] += restriction_value * restriction_multiplier;
				}
			}
		}

		//COLLATERAL DAMAGE (EVENT DESTRUCTION)
		{
			//COSTS
			for (unsigned k = 0; k < Rules::tasks[t].costs.row.size(); k++)
			{
				unsigned index = Rules::tasks[t].costs.row[k].index;

				for (unsigned e = 0; e < Rules::resourceValueLost[index].row.size(); e++)
				{
					unsigned collateral = Rules::resourceValueLost[index].row[e].index;
					double damage = 1;
					
					double objective_value = 0;

					for (unsigned o = 0; o < solver.objectives[0].row.size(); o++)
					{
						unsigned obj = solver.objectives[0].row[o].index;
						Objective type = solver.objectives[0].row[o].value;

						if (collateral == obj)
						{
							if (type == MINIMIZE)
								objective_value += damage;
							else
								objective_value -= damage;
						}
					}

					for (unsigned o = 0; o < solver.objectives[2].row.size(); o++)
					{
						unsigned obj = solver.objectives[2].row[o].index;
						Objective type = solver.objectives[2].row[o].value;

						if (collateral == obj)
						{
							if (type == MINIMIZE)
								objective_value -= damage;
							else
								objective_value += damage;
						}
					}

					taskValue[t] += objective_value * objective_multiplier;
				}
			}

			//CONSUME
			for (unsigned k = 0; k < Rules::tasks[t].consume.row.size(); k++)
			{
				unsigned index = Rules::tasks[t].consume.row[k].index;

				for (unsigned e = 0; e < Rules::resourceValueLost[index].row.size(); e++)
				{
					unsigned collateral = Rules::resourceValueLost[index].row[e].index;
					double damage = 1;
					
					double objective_value = 0;

					for (unsigned o = 0; o < solver.objectives[0].row.size(); o++)
					{
						unsigned obj = solver.objectives[0].row[o].index;
						Objective type = solver.objectives[0].row[o].value;

						if (collateral == obj)
						{
							if (type == MINIMIZE)
								objective_value += damage;
							else
								objective_value -= damage;
						}
					}

					for (unsigned o = 0; o < solver.objectives[2].row.size(); o++)
					{
						unsigned obj = solver.objectives[2].row[o].index;
						Objective type = solver.objectives[2].row[o].value;

						if (collateral == obj)
						{
							if (type == MINIMIZE)
								objective_value -= damage;
							else
								objective_value += damage;
						}
					}

					taskValue[t] += objective_value * objective_multiplier;
				}
			}
		}

		//PUT PREREQUISITES IN THE STACK
		for (unsigned i = 0; i < Rules::tasks[t].prerequisite.row.size(); i++)
		{
			unsigned index = Rules::tasks[t].prerequisite.row[i].index;
			unsigned value = Rules::tasks[t].prerequisite.row[i].value;
			int usable = initial.resources[index].usable();

			if ((usable > 0 && ((unsigned)usable < value)) || usable <= 0)
				for (unsigned k = 0; k < Rules::tasks.size(); k++)
					if (Rules::tasks[k].produce.get(index))
					{
						step new_step;
						new_step.from = t;
						new_step.to = k;
						next_step.push_back(new_step);
					}
		}
	}

	//PREREQUISITE MAP
	for (unsigned t = 0; t < next_step.size(); )
	{
		//ADD POSITIVE VALUES TO PREREQUISITE
		if (taskValue[next_step[t].from] > 0)
			taskValue[next_step[t].to] += taskValue[next_step[t].from];

		//PREREQUISITE BONUS FOR MAXIMIZING NEEDS
		taskValue[next_step[t].to] += solver.producesMaximize(next_step[t].from) * objective_multiplier * prerequisite_multiplier;
		taskValue[next_step[t].to] += solver.producesGreaterThan(next_step[t].from) * restriction_multiplier * prerequisite_multiplier;
		
		//PREREQUISITE
		for (unsigned i = 0; i < Rules::tasks[next_step[t].to].prerequisite.row.size(); i++)
		{
			unsigned index = Rules::tasks[next_step[t].to].prerequisite.row[i].index;
			unsigned value = Rules::tasks[next_step[t].to].prerequisite.row[i].value;
			int usable = initial.resources[index].usable();

			if ((usable > 0 && ((unsigned)usable < value)) || usable <= 0)
				for (unsigned k = 0; k < Rules::tasks.size(); k++)
					if (Rules::tasks[k].produce.get(index))
					{
						step new_step;
						new_step.from = next_step[t].to;
						new_step.to = k;
						next_step.push_back(new_step);
					}
		}

		pop(next_step);
	}
	
	//MAXIMUM BALANCING [fifo]
	for (unsigned t = 0; t < Rules::tasks.size(); t++)
		if (taskValue[t] >= 0)
			for (unsigned i = 0; i < Rules::tasks[t].produce.row.size(); i++)
			{
				unsigned index = Rules::tasks[t].produce.row[i].index;

				for (unsigned j = 0; j < Rules::resources[index].maximum_per_resource.row.size(); j++)
				{
					unsigned limit = Rules::resources[index].maximum_per_resource.row[j].index;

					for (unsigned k = 0; k < Rules::tasks.size(); k++)
						if (k != t)
							if (Rules::tasks[k].produce.get(limit))
							{
								step new_step;
								new_step.from = t;
								new_step.to = k;
								
								next_step.push_back(new_step);
							}
				}
			}

	//MAXIMUM BALANCING [map]
	for (unsigned t = 0; t < next_step.size(); )
	{
		//GET DISTANCE VALUE
		double dist = 0;
		for (unsigned i = 0; i < Rules::tasks[next_step[t].from].produce.row.size(); i++)
		{
			unsigned index = Rules::tasks[next_step[t].from].produce.row[i].index;
			unsigned value = Rules::tasks[next_step[t].from].produce.row[i].value;

			unsigned bonus = 0;

			for (unsigned k = 0; k < Rules::resources[index].maximum_per_resource.row.size(); k++)
			{
				unsigned limit = Rules::resources[index].maximum_per_resource.row[k].index;
				unsigned max = Rules::resources[index].maximum_per_resource.row[k].value;
				
				if (Rules::tasks[next_step[t].to].produce.get(limit))
				{
					bonus = max;
					break;
				}
			}

			if (bonus)
			{
				double bb = bonus;
				bb /= initial.resources[index].usable() + bonus + value;
				dist += bb * bb;
			}

			dist += Rules::taskValuePerEvent[index].get(next_step[t].from) ? maximum_multiplier : 0;
		}

		//PREREQUISITE BONUS FOR MAXIMIZING NEEDS
		taskValue[next_step[t].to] += maximum_multiplier * dist;

		pop(next_step);
	}

	//COST BALANCING [fifo]
	for (unsigned t = 0; t < Rules::tasks.size(); t++)
		if (taskValue[t] >= 0)
			for (unsigned i = 0; i < Rules::tasks[t].costs.row.size(); i++)
			{
				unsigned index = Rules::tasks[t].costs.row[i].index;

				for (unsigned k = 0; k < Rules::tasks.size(); k++)
					if (Rules::tasks[k].produce.get(index))
					{
						step new_step;
						new_step.from = t;
						new_step.to = k;
						
						next_step.push_back(new_step);
					}
			}

	//COST BALANCING [map]
	for (unsigned t = 0; t < next_step.size(); )
	{
		//GET DISTANCE VALUE
		double dist = 0;
		for (unsigned i = 0; i < Rules::tasks[next_step[t].from].costs.row.size(); i++)
		{
			unsigned index = Rules::tasks[next_step[t].from].costs.row[i].index;
			unsigned value = Rules::tasks[next_step[t].from].costs.row[i].value;

			unsigned bonus = Rules::tasks[next_step[t].to].produce.get(index);

			if (bonus)
			{
				double bb = bonus;
				double dd = initial.resources[index].usable() + bonus - value;
				if (dd > 0)
					bb /= dd;
				dist += bb * bb;
			}

			dist += Rules::taskValuePerEvent[index].get(next_step[t].from) ? cost_multiplier : 0;
		}

		//PREREQUISITE BONUS FOR MAXIMIZING NEEDS
		taskValue[next_step[t].to] += cost_multiplier * dist;

		pop(next_step);
	}

	//CONSUME BALANCING [fifo]
	for (unsigned t = 0; t < Rules::tasks.size(); t++)
		if (taskValue[t] >= 0)
			for (unsigned i = 0; i < Rules::tasks[t].consume.row.size(); i++)
			{
				unsigned index = Rules::tasks[t].consume.row[i].index;

				for (unsigned k = 0; k < Rules::tasks.size(); k++)
					if (Rules::tasks[k].produce.get(index))
					{
						step new_step;
						new_step.from = t;
						new_step.to = k;
						
						next_step.push_back(new_step);
					}
			}

	//CONSUME BALANCING [map]
	for (unsigned t = 0; t < next_step.size(); )
	{
		//GET DISTANCE VALUE
		double dist = 0;
		for (unsigned i = 0; i < Rules::tasks[next_step[t].from].consume.row.size(); i++)
		{
			unsigned index = Rules::tasks[next_step[t].from].consume.row[i].index;
			unsigned value = Rules::tasks[next_step[t].from].consume.row[i].value;

			unsigned bonus = Rules::tasks[next_step[t].to].produce.get(index);

			if (bonus)
			{
				double bb = bonus;
				double dd = initial.resources[index].usable() + bonus - value;
				if (dd > 0)
					bb /= dd;
				dist += bb * bb;
			}

			dist += Rules::taskValuePerEvent[index].get(next_step[t].from) ? cost_multiplier : 0;
		}

		//PREREQUISITE BONUS FOR MAXIMIZING NEEDS
		taskValue[next_step[t].to] += cost_multiplier * dist;

		pop(next_step);
	}

	double sum = -1e37;

	//INCREASE ZERO-VALUED TASKS : they are tasks that represent conflicting objectives/restrictions
	for (unsigned t = 0; t < Rules::tasks.size(); t++)
		if (taskValue[t] < 0)
			if (sum < taskValue[t])
				sum = taskValue[t];
	if (sum != -1e37)
		for (unsigned t = 0; t < Rules::tasks.size(); t++)
			taskValue[t] -= sum;
	else
	{
		sum = 1e37;
		for (unsigned t = 0; t < Rules::tasks.size(); t++)
			if (taskValue[t] > 0)
				if (sum > taskValue[t])
					sum = taskValue[t];
		for (unsigned t = 0; t < Rules::tasks.size(); t++)
			taskValue[t] += sum;
	}

	//CLEAN UNFEASIBLE TASKS
	for (unsigned t = 0; t < Rules::tasks.size(); t++)
		if (!hasPrerequisites(t,initial))
			taskValue[t] = -1e37;

	sum = 0;		
	for (unsigned t = 0; t < Rules::tasks.size(); t++)
		if (taskValue[t] > 0)
			sum += taskValue[t];

	double last = 0;
	for (unsigned t = 0; t < Rules::tasks.size(); t++)
		if (taskValue[t] > 0)
		{
			taskValue[t] /= sum;
			double tmp = taskValue[t];
			
			taskValue[t] += last;
			last += tmp;
		}
	
	return taskValue;
}

bool BuildOrder::Optimizer::nextTask(Solution& s, GameState init, Optimizer const& solver,
	double objective_multiplier, double restriction_multiplier, double prerequisite_multiplier,
	double cost_multiplier, double maximum_multiplier)
{
	std::vector<double> taskValue;

	taskValue = mapGoals(s.final_state, solver,
						 objective_multiplier,
						 restriction_multiplier,
						 prerequisite_multiplier,
						 cost_multiplier,
						 maximum_multiplier);

	double dice = drng();

	for (unsigned i = 0; i < taskValue.size(); i++)
		if (dice < taskValue[i])
		{
			s.orders.push_back(TaskPointer(i));

			unsigned size = s.orders.size();
			
			s.update(init, solver.maximum_time);

			return size == s.orders.size();
		}
	return false;
}

BuildOrder::Optimizer::Solution BuildOrder::Optimizer::create(GameState initial,
	Optimizer const& solver, double stop)
{
	Solution ret;
	ret.final_state = initial;

	double obj_m = 3.0;
	double res_m = 2.5;

	double dice = drng();
	dice *= 2;
	dice -= 1;

	obj_m -= dice;

	dice = drng();
	dice *= 2;
	dice -= 1;

	res_m -= dice;

	double delta_o = 1;
	double delta_r = 1.2;

	dice = drng();
	dice *= 0.2;
	dice -= 0.1;
	delta_r += dice;

	dice = drng();
	dice *= 0.2;

	delta_o += dice;

	while(nextTask(ret,initial,solver,obj_m,res_m))
	{
		res_m *= delta_r;
		obj_m *= delta_o;

		if (!solver.valid(ret))
			continue;

		dice = drng();

		if (dice < stop)
			break;
		else
			stop *= 1.1;
	}

	return ret;
}

void BuildOrder::Optimizer::make_valid(Solution& s, Optimizer const& solver, GameState const& init)
{
	for (unsigned d = 0; !solver.valid(s); d++)
	{
		if (d > s.orders.size())
		{
			s.orders.clear();
			d = 0;
		} else
			s.orders.erase(s.orders.begin()+s.orders.size()-d, s.orders.end());

		double obj_m = 2.5;
		double res_m = 3.0;
		while(nextTask(s, init, solver, obj_m, res_m))
		{
			obj_m *= 1.1;
			res_m *= 1.5;

			if (solver.valid(s))
				break;
		}
	}
}

void BuildOrder::Optimizer::trim(Solution& s, Optimizer const& solver, GameState const& init)
{
	Solution tmp = s;
	while(solver.valid(tmp))
	{
		s = tmp;
		tmp.orders.pop_back();
		tmp.update(init, solver.maximum_time);
	}
}