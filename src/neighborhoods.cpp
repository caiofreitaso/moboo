#include "../include/neighborhoods.h"

std::mt19937 *BuildOrder::Random::r = 0;

unsigned BuildOrder::rng()
{
	static unsigned count = 0;

	if (Random::r == 0)
		Random::r = new std::mt19937(std::chrono::system_clock::now().time_since_epoch().count());
	
	if (count == Random::r->state_size)
	{
		Random::r->seed(std::chrono::system_clock::now().time_since_epoch().count());
		count = 0;
	}

	count++;

	return (*Random::r)();
}

BuildOrder::Optimizer::Population BuildOrder::Optimizer::delete_one(Solution const& s)
{
	Population new_sol;
	new_sol.push_back(s);
	
	unsigned index = (rng() % new_sol.back().orders.size());

	pop(new_sol.back().orders, index);

	return new_sol;
}

BuildOrder::Optimizer::Population BuildOrder::Optimizer::delete_tail(Solution const& s)
{
	Population new_sol;
	new_sol.push_back(s);
	
	unsigned index = (rng() % new_sol.back().orders.size());

	new_sol.back().orders.erase(new_sol.back().orders.begin() + index, new_sol.back().orders.end());

	return new_sol;
}

BuildOrder::Optimizer::Population BuildOrder::Optimizer::one_swap(Solution const& s)
{
	Population new_sol;
	new_sol.push_back(s);

	if (s.orders.size() < 2)
		return new_sol;

	std::vector<unsigned> indexes;
	for (unsigned i = 0; i < s.orders.size(); i++)
		indexes.push_back(i);

	unsigned a, i_a, b, i_b;

	i_a = rng() % indexes.size();
	a = indexes[i_a];
	pop(indexes, i_a);

	i_b = rng() % indexes.size();
	b = indexes[i_b];

	std::swap(new_sol.back().orders[a], new_sol.back().orders[b]);

	return new_sol;
}

BuildOrder::Optimizer::Population BuildOrder::Optimizer::insert(Solution const& s)
{
	Population new_sol;
	new_sol.push_back(s);

	unsigned a, t;

	a = rng() % s.orders.size();
	t = rng() % Rules::tasks.size();

	new_sol.back().orders.insert(new_sol.back().orders.begin()+a, t);

	return new_sol;
}

BuildOrder::Optimizer::Population BuildOrder::Optimizer::swap_and_delete(Solution const& s)
{
	Population r;

	r.push_back(one_swap(s)[0]);
	r.push_back(delete_tail(s)[0]);

	return r;
}

BuildOrder::Optimizer::Population BuildOrder::Optimizer::swap_and_insert(Solution const& s)
{
	Population r;

	r.push_back(one_swap(s)[0]);
	r.push_back(insert(s)[0]);

	return r;
}