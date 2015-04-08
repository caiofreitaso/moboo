#ifndef NEIGHBORHOODSH
#define NEIGHBORHOODSH

#include <chrono>
#include <random>
#include "objective.h"

namespace BuildOrder
{
	namespace Random
	{
		extern std::mt19937 *r;
	}

	unsigned rng();

	inline unsigned rng(unsigned max, unsigned min = 0)
	{
		unsigned diff = max - min + 1;
		return (rng() % diff) + min;
	}

	inline double drng()
	{
		double r = rng();
		return r / Random::r->max();
	}

	inline double drng(double min, double max)
	{
		double diff = max - min;
		return (drng() * diff) + min;
	}
}

namespace BuildOrder
{
	namespace Optimizer
	{
		struct Solution
		{
			BuildOrder orders;
			GameState final_state;

			void update(GameState initial, unsigned maximum_time = 0)
			{
				final_state = makespan(initial, orders, maximum_time);
			}

			bool operator==(Solution const& s) const
			{
				if (orders.size() != s.orders.size())
					return false;

				for (unsigned i = 0; i < orders.size(); i++)
				{
					if (orders[i].task != s.orders[i].task)
						return false;
					if (orders[i].delay != s.orders[i].delay)
						return false;
				}

				return true;
			}

			bool operator<(Solution const& s) const
			{ return final_state.time < s.final_state.time; }
		};

		typedef std::vector<Solution> Population;

		Population delete_one(Solution const& s);
		Population delete_tail(Solution const& s);
		Population one_swap(Solution const& s);
		Population insert(Solution const& s);
		Population swap_and_delete(Solution const& s);
		Population swap_and_insert(Solution const& s);
	}
}

#endif