#include "../include/create_solution.h"

double BuildOrder::Optimizer::Creation::objective_minimum = 2.0;
double BuildOrder::Optimizer::Creation::objective_maximum = 4.0;
double BuildOrder::Optimizer::Creation::restriction_minimum = 1.5;
double BuildOrder::Optimizer::Creation::restriction_maximum = 3.5;

double BuildOrder::Optimizer::Creation::delta_o_minimum = 1.01;
double BuildOrder::Optimizer::Creation::delta_o_maximum = 1.1;
double BuildOrder::Optimizer::Creation::delta_r_minimum = 1.8;
double BuildOrder::Optimizer::Creation::delta_r_maximum = 2.5;

double BuildOrder::Optimizer::Fix::objective_minimum = 1.5;
double BuildOrder::Optimizer::Fix::objective_maximum = 2.5;
double BuildOrder::Optimizer::Fix::restriction_minimum = 2.6;
double BuildOrder::Optimizer::Fix::restriction_maximum = 3.5;

double BuildOrder::Optimizer::Fix::delta_o_minimum = 1.1;
double BuildOrder::Optimizer::Fix::delta_o_maximum = 1.3;
double BuildOrder::Optimizer::Fix::delta_r_minimum = 1.4;
double BuildOrder::Optimizer::Fix::delta_r_maximum = 2.0;


void BuildOrder::Optimizer::passWeight(double value, unsigned task,
	Rules::MultiGraph const& g,
	std::vector<double>& final,
	std::vector<bool> const& p,
	std::vector< std::bitset<5> >& done)
{
	unsigned ti = g.find(task);
	if (ti == g.size())
	{
		final[task] += value;
		return;
	}

	std::queue<unsigned> queue;
	std::queue<double> queued;
	std::queue<Rules::relation_type> type;

	done[task].set();

	for (unsigned i = 0; i < g[ti].value.size(); i++)
	{
		type.push(g[ti].value[i].value().type);
		queue.push(g[ti].value[i].index);

		double n_value = value;
				
		if (g[ti].value[i].value.value > 0)
			n_value *= g[ti].value[i].value.value;
		queued.push(n_value);
	}

	while(queue.size())
	{
		unsigned i = queue.front();
		unsigned t = (unsigned) type.front();
		unsigned index = g.find(i);

		done[i][t] = true;
		if (p[i])
			final[i] += queued.front();
		
		for (unsigned e = 0; e < g[index].value.size(); e++)
		{
			i = g[index].value[e].index;
			t = (unsigned) g[index].value[e].value().type;
			if (!done[i][t])
			{
				type.push(g[index].value[e].value().type);
				queue.push(i);

				double n_value = queued.front();
				
				if (g[index].value[e].value.value > 0)
					n_value *= g[index].value[e].value.value;

				queued.push(n_value);
			}
		}

		queue.pop();
		queued.pop();
		type.pop();
	}

	/*
		if (p[task])
		{
			final[task] += value;
			return;
		}

		for (unsigned i = 0; i < g.size(); i++)
		{
			Rules::Forest::c_it it = g[i].find(task);

			if (it != g[i].end())
			{
				Rules::Forest::d_it k;
				for (k = it->dependencies.cbegin(); k != it->dependencies.cend(); k++)
					if (!done[k->task][i])
					{
						double f_value = value;
						if (k->value > 0)
							f_value *= w[i]*k->value;

						done[k->task][i] = 1;
						passWeight( f_value, k->task,g,w,final,p,done );
					}
			}
		}
	*/
}

void BuildOrder::Optimizer::pruneGraph(Rules::MultiGraph& g,
	GameState initial, Rules::relation_type r,
	bool (*func)(unsigned, unsigned, unsigned))
{
	std::vector<unsigned> delete_stack;

	for (unsigned i = 0; i < g.size(); i++)
	{
		Rules::MultiGraph::node& v = g[i].value;
		unsigned vi = g[i].index;
		
		std::vector<Rules::MultiGraph::e_it> stack;
		for (auto e = v.edges.begin(); e != v.edges.end(); e++)
		{
			Rules::Dependency& d = e->value();
			unsigned j = e->index;
			
			if (d.type == r)
			{
				for (unsigned k = 0; k < d.weight.size(); k++)
				{
					unsigned res = d.weight[k].index;
					if (func(vi, res, initial.usable(res)))
					{
						d.weight.set(res,0);
						d.event.set(res,0);
						d.bonus.set(res,0);
					}
				}
				if (d.weight.size() == 0)
					stack.push_back(e);
			}
		}

		for (unsigned i = stack.size()-1; stack.size() > 0; i--)
		{
			v.edges.row.erase(stack[i]);
			stack.pop_back();
		}

		if (v.size() == 0)
			delete_stack.push_back(vi);
	}
	for (unsigned i = delete_stack.size()-1; delete_stack.size() > 0; i--)
	{
		g.soft_erase(delete_stack[i]);
		delete_stack.pop_back();
	}
}

void BuildOrder::Optimizer::getValues(Rules::MultiGraph& g,
	GameState initial, std::vector<double> value)
{
	for (unsigned i = 0; i < g.size(); i++)
	{
		Rules::MultiGraph::node& v = g[i].value;
		unsigned vi = g[i].index;
		
		for (unsigned j = 0; j < v.size(); j++)
		{
			Rules::MultiGraph::edge& e = v[j].value;
			Rules::Dependency& d = v[j].value();
			unsigned ej = v[j].index;

			for (unsigned k = 0; k < d.weight.size(); k++)
			{
				unsigned res = d.weight[k].index;
				double val = d.weight[k].value;

				double deno;
				if (d.type == Rules::RT_MAXIMUM)
					deno = initial.maximum(res) - initial.usable(res);
				else if (d.type == Rules::RT_PREREQUISITE)
					deno = Rules::tasks[vi].prerequisite.get(res) - initial.usable(res);
				else
					deno = initial.usable(res);

				deno += d.bonus[k].value;
				deno = val/deno;
				deno *= deno;
				e.value += deno;
			}

			for (unsigned k = 0; k < d.event.size(); k++)
				e.value += 0.3;

			e.value *= value[(unsigned) d.type];
		}
	}
}

std::vector<double> BuildOrder::Optimizer::taskWeights(GameState initial,
	Optimizer const& solver,
	double o, double r, double p, double c, double m)
{
	std::vector<double> init_value = solver.initialMap(o, r, initial);
	std::vector<double> taskValue(init_value);

	Rules::MultiGraph actual(Rules::graph);
	std::vector<double> weights(5);

	std::vector<bool> possibles(Rules::tasks.size(), false);
	std::vector<bool> ddone(Rules::tasks.size(), false);
	std::vector<std::bitset<5> > done(Rules::tasks.size());

	weights[0] = c;
	weights[1] = c;
	weights[2] = m;
	weights[3] = p;
	weights[4] = p;

	for (unsigned t = 0; t < Rules::tasks.size(); t++)
		possibles[t] = ::BuildOrder::Objective::possible(t,initial);

	//PREREQUISITE EDGES PRUNING
	pruneGraph(actual, initial, Rules::RT_PREREQUISITE, Rules::fillsPrerequisite);
	{

		/*for (it = actual[0].begin(); it != actual[0].end(); it++)
		{
			std::vector<unsigned> stack;
			
			Rules::Forest::d_it k;
			for (k = it->dependencies.cbegin(); k != it->dependencies.cend(); k++)
			{
				for (unsigned i = 0; i < k->w.row.size(); i++)
				{
					unsigned index = k->w.row[i].index;
					if (initial.usable(index) >= Rules::tasks[it->task].prerequisite.get(index))
					{
						k->w.set(index,0);
						k->e.set(index,0);
						k->bonus.set(index,0);
					}
				}

				if (k->w.row.size() == 0)
					stack.push_back(k->task);
			}

			for (unsigned i = stack.size()-1; stack.size() > 0; i--)
			{
				it->dependencies.erase(it->find(stack[i]));
				stack.pop_back();
			}

			if (it->dependencies.size() == 0)
				delete_stack.push_back(it->task);
		}
		for (unsigned i = delete_stack.size()-1; delete_stack.size() > 0; i--)
		{
			actual[0].erase(delete_stack[i]);
			delete_stack.pop_back();
		}*/
	}

	//BORROW EDGES PRUNING
	pruneGraph(actual, initial, Rules::RT_BORROW, Rules::fillsBorrow);
	{
		/*for (it = actual[1].begin(); it != actual[1].end(); it++)
		{
			std::vector<unsigned> stack;
			
			Rules::Forest::d_it k;
			for (k = it->dependencies.cbegin(); k != it->dependencies.cend(); k++)
			{
				for (unsigned i = 0; i < k->w.row.size(); i++)
				{
					unsigned index = k->w.row[i].index;
					if (initial.usable(index) >= Rules::tasks[it->task].borrow.get(index))
					{
						k->w.set(index,0);
						k->e.set(index,0);
						k->bonus.set(index,0);
					}
				}

				if (k->w.row.size() == 0)
					stack.push_back(k->task);
			}

			for (unsigned i = stack.size()-1; stack.size() > 0; i--)
			{
				it->dependencies.erase(it->find(stack[i]));
				stack.pop_back();
			}

			if (it->dependencies.size() == 0)
				delete_stack.push_back(it->task);
		}
		for (unsigned i = delete_stack.size()-1; delete_stack.size() > 0; i--)
		{
			actual[1].erase(delete_stack[i]);
			delete_stack.pop_back();
		}*/
	}

	getValues(actual, initial, weights);
	/*

		//BORROW MAP
		for (it = actual[1].begin(); it != actual[1].end(); it++)
			if (init_value[it->task] > 0)
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

						k->value += deno * init_value[it->task];
					}

					for (unsigned i = 0; i < k->e.row.size(); i++)
						k->value += 1;
				}
			}

		//MAXIMUM MAP
		for (it = actual[2].begin(); it != actual[2].end(); it++)
			if (init_value[it->task] > 0)
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

						k->value += deno * init_value[it->task];
					}

					for (unsigned i = 0; i < k->e.row.size(); i++)
						k->value += 1;
				}
			}

		//COSTS MAP
		for (it = actual[3].begin(); it != actual[3].end(); it++)
			if (init_value[it->task] > 0)
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

						k->value += deno * init_value[it->task];
					}

					for (unsigned i = 0; i < k->e.row.size(); i++)
						k->value += 1;
				}
			}

		//CONSUMES MAP
		for (it = actual[4].begin(); it != actual[4].end(); it++)
			if (init_value[it->task] > 0)
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

						k->value += deno * init_value[it->task];
					}

					for (unsigned i = 0; i < k->e.row.size(); i++)
						k->value += 1;
				}
			}
	*/

	for (unsigned i = 0; i < taskValue.size(); i++)
		if (init_value[i] > 0)
		{
			ddone[i] = true;
			passWeight( init_value[i], i, actual, taskValue, possibles, done );
			for (unsigned j = 0; j < taskValue.size(); j++)
				done[j].reset();
		}
	
	/*for (bool change; change;)
	{
		change = false;
		getValues(actual, initial, taskValue);
		for (unsigned i = 0; i < taskValue.size(); i++)
			if (taskValue[i] > 0 && !ddone[i])
			{
				ddone[i] = true;
				change = true;
				passWeight( taskValue[i], i, actual, weights, taskValue, possibles, done );
				for (unsigned j = 0; j < taskValue.size(); j++)
					if (ddone[j])
						done[j].set();
					else
						done[j].reset();
			}
	}*/

	double sum = DOUBLE_NINF;

	//CLEAN UNFEASIBLE TASKS
	for (unsigned t = 0; t < Rules::tasks.size(); t++)
		if (!possibles[t])
			taskValue[t] = DOUBLE_NINF;

	//INCREASE ZERO-VALUED TASKS : they are tasks that represent conflicting objectives/restrictions
	for (unsigned t = 0; t < Rules::tasks.size(); t++)
		if (taskValue[t] < 0)
			if (sum < taskValue[t])
				sum = taskValue[t];
	if (sum != DOUBLE_NINF)
		for (unsigned t = 0; t < Rules::tasks.size(); t++)
			taskValue[t] -= sum;
	
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
			{
				taskValue[t] = 1;
				sum++;
			}

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

/*
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
		std::vector<double> taskValue = solver.initialMap(objective_multiplier, restriction_multiplier, initial);
		std::vector<double> pr_value(Rules::tasks.size(), 0);
		std::vector<double> bor_value(Rules::tasks.size(), 0);
		std::vector<double> max_value(Rules::tasks.size(), 0);
		std::vector<double> cost_value(Rules::tasks.size(), 0);
		std::vector<double> cons_value(Rules::tasks.size(), 0);

		std::vector<bool> possibles(Rules::tasks.size(), false);
		std::vector<bool> done(Rules::tasks.size(), false);

		Rules::Forest::c_it it;

		for (unsigned t = 0; t < Rules::tasks.size(); t++)
			possibles[t] = ::BuildOrder::Objective::possible(t,initial);

		//PREREQUISITE MAP
		for (unsigned i = 0; i < Rules::prerequisites.hsize(); i++)
		{
			it = Rules::prerequisites[i];
			
			mapPrerequisites(it, initial, taskValue, pr_value);
		}

		//BORROW MAP
		for (it = Rules::borrows.begin(); it != Rules::borrows.end(); it++)
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

						bor_value[k->task] += deno * taskValue[it->task];
					}

					for (unsigned i = 0; i < k->e.row.size(); i++)
						bor_value[k->task] += 1;
				}
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

		double sum = DOUBLE_NINF;

		for (unsigned t = 0; t < Rules::tasks.size(); t++)
			taskValue[t] += pr_value[t] * prerequisite_multiplier;
		for (unsigned t = 0; t < Rules::tasks.size(); t++)
			taskValue[t] += bor_value[t] * prerequisite_multiplier;
		for (unsigned t = 0; t < Rules::tasks.size(); t++)
			taskValue[t] += max_value[t] * maximum_multiplier;
		for (unsigned t = 0; t < Rules::tasks.size(); t++)
			taskValue[t] += cost_value[t] * cost_multiplier;
		for (unsigned t = 0; t < Rules::tasks.size(); t++)
			taskValue[t] += cons_value[t] * cost_multiplier;

		//CLEAN UNFEASIBLE TASKS
		for (unsigned t = 0; t < Rules::tasks.size(); t++)
			if (!possibles[t])
				taskValue[t] = DOUBLE_NINF;

		//INCREASE ZERO-VALUED TASKS : they are tasks that represent conflicting objectives/restrictions
		for (unsigned t = 0; t < Rules::tasks.size(); t++)
			if (taskValue[t] < 0)
				if (sum < taskValue[t])
					sum = taskValue[t];
		if (sum != DOUBLE_NINF)
			for (unsigned t = 0; t < Rules::tasks.size(); t++)
				taskValue[t] -= sum;
		else
		{
			sum = -DOUBLE_NINF;
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
*/

bool BuildOrder::Optimizer::nextTask(Solution& s, GameState init, Optimizer const& solver,
	double objective_multiplier, double restriction_multiplier, double prerequisite_multiplier,
	double cost_multiplier, double maximum_multiplier)
{
	std::vector<double> taskValue, tv;

	taskValue = taskWeights(s.final_state, solver,
						 objective_multiplier,
						 restriction_multiplier,
						 prerequisite_multiplier,
						 cost_multiplier,
						 maximum_multiplier);

	/*tv = mapGoals(s.final_state, solver,
				 objective_multiplier,
				 restriction_multiplier,
				 prerequisite_multiplier,
				 cost_multiplier,
				 maximum_multiplier);*/

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

	double obj_m = Creation::getObjective();
	double res_m = Creation::getRestriction();

	double delta_o = Creation::getDelta_O();
	double delta_r = Creation::getDelta_R();

	unsigned rem = 0;

	double max_o = obj_m * pow(delta_o,99);
	double max_r = res_m * pow(delta_r,99);

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
			if (!change)//ret.final_state.time >= solver.maximum_time)
			{
				if (rem >= ret.orders.size())
					rem = 0;
				else
					rem++;
				
				ret.orders.erase(ret.orders.begin()+ret.orders.size()-rem, ret.orders.end());
				ret.update(initial, solver.maximum_time);
			}
			continue;
		}

		if (ret.orders.size() == 200)
			break;

		double dice = drng();

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

		s.update(init, solver.maximum_time);

		double obj_m = Fix::getObjective();
		double res_m = Fix::getRestriction();

		double delta_o = Fix::getDelta_O();
		double delta_r = Fix::getDelta_R();

		double max_o = obj_m*delta_o*100;
		double max_r = res_m*delta_r*100;
		while(nextTask(s, init, solver, obj_m, res_m))
		{
			if (obj_m < max_o)
				obj_m *= delta_o;
			if (res_m < max_r)
				res_m *= delta_r;

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