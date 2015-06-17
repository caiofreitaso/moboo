#ifndef RULESH
#define RULESH

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include "matrix.h"
#include <set>

namespace BuildOrder
{
	namespace Rules
	{
		struct Task {
			unsigned time;
			MatrixRow<int> costs;

			MatrixRow<unsigned> prerequisite;
			MatrixRow<unsigned> consume;
			MatrixRow<unsigned> borrow;

			MatrixRow<unsigned> produce;
		};

		struct Event {
			unsigned time;

			MatrixRow<unsigned> bonus;

			unsigned trigger;
		};

		struct Resource {
			unsigned overall_maximum;

			MatrixRow<unsigned> maximum_per_resource;
			MatrixRow<unsigned> equivalence;
		};

		extern std::vector<Task> tasks;
		extern std::vector<Event> events;
		extern std::vector<Resource> resources;

		extern std::vector<MatrixRow<unsigned> > taskProduceByEvent;
		extern std::vector<MatrixRow<unsigned> > taskValuePerEvent;
		extern std::vector<MatrixRow<unsigned> > resourceValueLost;

		bool fillsPrerequisite(unsigned task, unsigned resource, unsigned amount);
		bool fillsBorrow(unsigned task, unsigned resource, unsigned amount);

		extern unsigned mean_time;

		void init(char const* filename);
	}
}

#endif