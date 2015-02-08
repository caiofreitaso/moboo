#include "../include/create_solution.h"

void BuildOrder::Optimizer::mapPrerequisites(Rules::Forest::c_it it,
	GameState initial, std::vector<double> const& taskValue,
	std::vector<double>& pr_value)
{
	unsigned t = it->task;

	if (!initial.hasPrerequisites(t) && taskValue[t] > 0)
	{
		Rules::Forest::d_it dt = it->dependencies.cbegin();
		for (; dt != it->dependencies.cend(); dt++)
			pr_value[dt->task] += (taskValue[t] + pr_value[t]);

		dt = it->dependencies.cbegin();
		for (; dt != it->dependencies.cend(); dt++)
		{
			Rules::Forest::c_it k = Rules::prerequisites.find(dt->task);
			if (k != Rules::prerequisites.end())
				mapPrerequisites(k,initial,taskValue,pr_value);
		}
	}
}

std::vector<double> BuildOrder::Optimizer::mapGoals(GameState initial, Optimizer const& solver,
	double objective_multiplier, double restriction_multiplier, double prerequisite_multiplier,
	double cost_multiplier, double maximum_multiplier)
{
	struct step
	{
		unsigned from;
		unsigned to;
	};

	std::vector<double> taskValue = solver.initialMap(objective_multiplier, restriction_multiplier, initial);
	std::vector<double> pr_value(Rules::tasks.size(), 0);
	std::vector<double> max_value(Rules::tasks.size(), 0);
	std::vector<double> cost_value(Rules::tasks.size(), 0);
	std::vector<double> cons_value(Rules::tasks.size(), 0);

	std::vector<bool> possibles(Rules::tasks.size(), false);
	std::vector<bool> done(Rules::tasks.size(), false);

	Rules::Forest::c_it it;

	unsigned worries = solver.numberObjectives()+solver.numberRestrictions();

	for (unsigned t = 0; t < Rules::tasks.size(); t++)
	{
		BuildOrder bo;
		bo.push_back(t);
		
		possibles[t] = ::BuildOrder::Objective::possible(bo,initial);
	}

	//PREREQUISITE MAP
	for (unsigned i = 0; i < Rules::prerequisites.hsize(); i++)
	{
		it = Rules::prerequisites[i];
		
		mapPrerequisites(it, initial, taskValue, pr_value);
	}

	//MAXIMUM MAP
	for (it = Rules::maxima.begin(); it != Rules::maxima.end(); it++)
		if (taskValue[it->task] > 0 && possibles[it->task])
		{
			Rules::Forest::d_it k;
			for (k = it->dependencies.cbegin(); k != it->dependencies.cend(); k++)
			{
				for (unsigned i = 0; i < k->w.row.size(); i++)
				{
					unsigned index = k->w.row[i].index;
					double value = k->w.row[i].value;

					double deno = initial.maximum(index) - initial.usable(index);
					deno += k->bonus.row[i].value;
					deno = value/deno;
					deno *= deno;

					max_value[k->task] += deno * taskValue[it->task];
				}

				for (unsigned i = 0; i < k->e.row.size(); i++)
					max_value[k->task] += 1;
			}
		}

	//COSTS MAP
	for (it = Rules::costs.begin(); it != Rules::costs.end(); it++)
		if (taskValue[it->task] > 0 && possibles[it->task])
		{
			Rules::Forest::d_it k;
			for (k = it->dependencies.cbegin(); k != it->dependencies.cend(); k++)
			{
				for (unsigned i = 0; i < k->w.row.size(); i++)
				{
					unsigned index = k->w.row[i].index;
					double value = k->w.row[i].value;

					double deno = initial.usable(index);
					deno += k->bonus.row[i].value;
					deno = value/deno;
					deno *= deno;

					cost_value[k->task] += deno * taskValue[it->task];
				}

				for (unsigned i = 0; i < k->e.row.size(); i++)
					cost_value[k->task] += 1;
			}
		}

	//CONSUMES MAP
	for (it = Rules::consumes.begin(); it != Rules::consumes.end(); it++)
		if (taskValue[it->task] > 0 && possibles[it->task])
		{
			Rules::Forest::d_it k;
			for (k = it->dependencies.cbegin(); k != it->dependencies.cend(); k++)
			{
				for (unsigned i = 0; i < k->w.row.size(); i++)
				{
					unsigned index = k->w.row[i].index;
					double value = k->w.row[i].value;

					double deno = initial.usable(index);
					deno += k->bonus.row[i].value;
					deno = value/deno;
					deno *= deno;

					cons_value[k->task] += deno * taskValue[it->task];
				}
				
				for (unsigned i = 0; i < k->e.row.size(); i++)
					cons_value[k->task] += 1;
			}
		}

	double sum = -1e37;

	for (unsigned t = 0; t < Rules::tasks.size(); t++)
		taskValue[t] += pr_value[t] * prerequisite_multiplier;
	for (unsigned t = 0; t < Rules::tasks.size(); t++)
		taskValue[t] += max_value[t] * maximum_multiplier;
	for (unsigned t = 0; t < Rules::tasks.size(); t++)
		taskValue[t] += cost_value[t] * cost_multiplier;
	for (unsigned t = 0; t < Rules::tasks.size(); t++)
		taskValue[t] += cons_value[t] * cost_multiplier;

	//CLEAN UNFEASIBLE TASKS
	for (unsigned t = 0; t < Rules::tasks.size(); t++)
		if (!possibles[t])
			taskValue[t] = -1e37;

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

	sum = 0;		
	unsigned count = 0;
	for (unsigned t = 0; t < Rules::tasks.size(); t++)
		if (taskValue[t] > 0)
		{
			sum += taskValue[t];
			count++;
		}

	if (count == 0)
		for (unsigned t = 0; t < Rules::tasks.size(); t++)
			if (taskValue[t] == 0)
				taskValue[t] = 1;

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

	double delta_o = 1.1;
	double delta_r = 1.2;

	dice = drng();
	dice *= 0.2;
	dice -= 0.1;
	delta_o += dice;

	dice = drng();
	dice *= 0.2;

	delta_r += dice;

	unsigned rem = 1;

	double max_o = obj_m * delta_o * 100;
	double max_r = res_m * delta_r * 100;

	if (max_o > max_r)
		std::swap(max_o, max_r);

	for (;;)
	{
			
		bool change = nextTask(ret,initial,solver,obj_m,res_m);

		if (res_m < max_r)
			res_m *= delta_r;
		if (obj_m < max_o)
			obj_m *= delta_o;

		if (!solver.valid(ret))
		{
			if (ret.final_state.time >= solver.maximum_time)
			{
				ret.orders.erase(ret.orders.begin()+ret.orders.size()-rem, ret.orders.end());
				ret.update(initial, solver.maximum_time);
				if (rem >= ret.orders.size())
					rem = 0;
				else
					rem++;
			}
			continue;
		}
		else
			break;

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

		double max_o = 2.5*1.1*100;
		double max_r = 3.0*1.5*100;
		while(nextTask(s, init, solver, obj_m, res_m))
		{
			if (obj_m < max_o)
				obj_m *= 1.1;
			if (res_m < max_r)
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