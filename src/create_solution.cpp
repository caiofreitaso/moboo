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

unsigned BuildOrder::Optimizer::local_search_maximum = 5;


void BuildOrder::Optimizer::passWeight(double value, unsigned task,
	Rules::MultiGraph const& g,
	contiguous<double>& final,
	contiguous<bool> const& p,
	contiguous< std::bitset<5> >& done)
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
}

void BuildOrder::Optimizer::pruneGraph(Rules::MultiGraph& g,
	GameState initial, Rules::relation_type r,
	bool (*func)(unsigned, unsigned, unsigned))
{
	contiguous<unsigned> delete_stack;

	for (unsigned i = 0; i < g.size(); i++)
	{
		Rules::MultiGraph::node& v = g[i].value;
		unsigned vi = g[i].index;
		
		contiguous<Rules::MultiGraph::e_it> stack;
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
	GameState initial, contiguous<double> value)
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
					deno = initial.maximum(res) - initial.usableB(res);
				else if (d.type == Rules::RT_PREREQUISITE)
					deno = Rules::tasks[vi].prerequisite.get(res) - initial.usableB(res);
				else
					deno = initial.usableB(res);

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

contiguous<double> BuildOrder::Optimizer::taskWeights(GameState initial,
	Optimizer const& solver,
	double o, double r, double p, double c, double m)
{
	contiguous<double> init_value = solver.initialMap(o, r, initial);
	contiguous<double> taskValue(init_value);

	Rules::MultiGraph actual(Rules::graph);
	contiguous<double> weights(5);

	contiguous<bool> possibles(Rules::tasks.size(), false);
	contiguous<bool> ddone(Rules::tasks.size(), false);
	contiguous<std::bitset<5> > done(Rules::tasks.size());

	weights[0] = c;
	weights[1] = c;
	weights[2] = m;
	weights[3] = p;
	weights[4] = p;

	#pragma omp parallel for
	for (unsigned t = 0; t < Rules::tasks.size(); t++)
		possibles[t] = ::BuildOrder::Objective::possible(t,initial);

	//PREREQUISITE EDGES PRUNING
	pruneGraph(actual, initial, Rules::RT_PREREQUISITE, Rules::fillsPrerequisite);
	//BORROW EDGES PRUNING
	pruneGraph(actual, initial, Rules::RT_BORROW, Rules::fillsBorrow);

	getValues(actual, initial, weights);

	for (unsigned i = 0; i < taskValue.size(); i++)
		if (init_value[i] > 0)
		{
			ddone[i] = true;
			passWeight( init_value[i], i, actual, taskValue, possibles, done );
			for (unsigned j = 0; j < taskValue.size(); j++)
				done[j].reset();
		}

	double sum = DOUBLE_NINF;

	//CLEAN UNFEASIBLE TASKS
	#pragma omp parallel for
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

bool BuildOrder::Optimizer::nextTask(Solution& s, GameState init, Optimizer const& solver,
	double objective_multiplier, double restriction_multiplier, double prerequisite_multiplier,
	double cost_multiplier, double maximum_multiplier)
{
	contiguous<double> taskValue;//(Rules::tasks.size());

	/*double cc = 0;
	for (unsigned i = 0; i < taskValue.size(); i++)
		if (::BuildOrder::Objective::possible(i, init))
		{
			++taskValue[i];
			++cc;
		}
	for (unsigned i = 0; i < taskValue.size(); i++)
		taskValue[i] /= cc;

	*/
	taskValue = taskWeights(s.final_state, solver,
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

/*BuildOrder::Optimizer::Solution BuildOrder::Optimizer::create_exact(GameState const& initial,
	Optimizer const& solver)
{
	Solution ret;
	ret.final_state = initial;

	contiguous<unsigned> target_tasks;

	//for each restricton: use branch-and-bound to reach the goal
	for (auto r : solver.restrictions[0].row)
	{
		unsigned target = r.index;
		unsigned gt = r.value.greater_than;
		unsigned lt = r.value.less_than;

		contiguous<unsigned> candidates;
		for (unsigned i = 0; i < Rules::tasks.size(); i++)
			if (Rules::tasks[i].produce.get(target))
				candidates.push_back(i);
			else if (Rules::taskProduceByEvent[i].get(target))
				candidates.push_back(i);
		
		Solution s = branchnbound(initial,ret, candidates, target,gt,lt, -1, solver.maximum_time);
	}

	return ret;
}

BuildOrder::Optimizer::Solution BuildOrder::Optimizer::branchnbound(GameState const& initial, Solution s,
	contiguous<unsigned> const& target_tasks, unsigned target, unsigned gt, unsigned lt, unsigned last,
	unsigned maximum_time)
{
	static Solution ret;
	static unsigned MTIME;
	if (last == -1)
		MTIME = maximum_time;

	contiguous<double> taskValue(Rules::tasks.size(), 0);

	for(auto t : target_tasks)
		taskValue[t] = 100;

	{
		Rules::MultiGraph actual(Rules::graph);
		
		//PREREQUISITE EDGES PRUNING
		pruneGraph(actual, s.final_state, Rules::RT_PREREQUISITE, Rules::fillsPrerequisite);
		//BORROW EDGES PRUNING
		pruneGraph(actual, s.final_state, Rules::RT_BORROW, Rules::fillsBorrow);

		{
			contiguous<double> weights(5,0.5);
			weights[3] = weights[4] = 8000;
			getValues(actual, s.final_state, weights);
		}

		{
			contiguous<bool> possibles(Rules::tasks.size(), false);
			contiguous<std::bitset<5> > done(Rules::tasks.size());

			for (unsigned t = 0; t < Rules::tasks.size(); t++)
				possibles[t] = ::BuildOrder::Objective::possible(t,s.final_state);

			for (auto t : target_tasks)
			{
				passWeight( 100, t, actual, taskValue, possibles, done );
				for (auto d : done)
					d.reset();
			}

			for (unsigned t = 0; t < Rules::tasks.size(); t++)
				if (!possibles[t])
					taskValue[t] = DOUBLE_NINF;
		}
	}

	Population nodes;

	{
		struct tmp
		{ double a; unsigned i; bool operator<(tmp s) const { return a > s.a; } };
		contiguous<tmp> list;
		list.reserve(taskValue.size());

		for (unsigned i = 0; i < taskValue.size(); i++)
			if (taskValue[i] > DOUBLE_NINF)
			{
				tmp t;
				t.a = taskValue[i];
				t.i = i;
				list.push_back(t);
			}

		std::sort(list.begin(), list.end());

		for (auto i : list)
		{
			Solution n = s;
			n.orders.push_back(i.i);
			n.update(initial, maximum_time);

			if (n.orders.size() > s.orders.size())
				#pragma omp critical
				if (n.final_state.time <= MTIME)
				{
					if (n.final_state.resources[target].usable() >= gt && n.final_state.resources[target].usable() <= lt)
					{
						MTIME = n.final_state.time;
						ret = n;
						print(ret.orders);
						std::cout << "Time: " << MTIME << "\n";
					} else
						nodes.push_back(n);
				}
		}
	}

	#pragma omp parallel for num_threads(nodes.size())
	for (unsigned i = 0; i < nodes.size(); i++)
		branchnbound(initial,nodes[i],target_tasks,target,gt,lt,nodes[i].orders.back().task,MTIME);

	return ret;
}*/

BuildOrder::Optimizer::Population BuildOrder::Optimizer::local_search(Population (*neighborhood)(Solution const&), Population const& p, unsigned childs, Optimizer const& opt, GameState init)
{
	Population neighbors(p), l(p);
	unsigned iterations = 0;

	for(; neighbors == l && iterations < local_search_maximum; iterations++)
	{
		l = neighbors;
		neighbors.clear();

		#pragma omp parallel for
		for (unsigned k = 0; k < l.size(); k++)
		{
			Population n = local_search(neighborhood,l[k],childs,opt,init);

			for (unsigned t = 0; t < n.size(); t++)
				#pragma omp critical
				neighbors.push_back(n[t]);
		}
		neighbors = opt.nonDominated(neighbors);
	}

	return neighbors;
}

BuildOrder::Optimizer::Population BuildOrder::Optimizer::local_search(Population (*neighborhood)(Solution const&), Solution const& p, unsigned childs, Optimizer const& opt, GameState init)
{
	Population neighbors;

	#pragma omp parallel for
	for (unsigned c = 0; c < childs; c++)
	{
		Population n = neighborhood(p);
		for (unsigned t = 0; t < n.size(); t++)
		{
			n[t].update(init, opt.maximum_time);

			make_valid(n[t], opt, init);
			trim(n[t], opt, init);

			#pragma omp critical
			neighbors.push_back(n[t]);
		}
	}

	return opt.nonDominated(neighbors);
}