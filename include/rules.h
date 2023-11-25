#ifndef RULESH
#define RULESH

#include "contiguous.h"
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

		extern contiguous<Task> tasks;
		extern contiguous<Event> events;
		extern contiguous<Resource> resources;

		extern contiguous<MatrixRow<unsigned> > taskProduceByEvent;
		extern contiguous<MatrixRow<unsigned> > taskValuePerEvent;
		extern contiguous<MatrixRow<unsigned> > resourceValueLost;

		bool fillsPrerequisite(unsigned task, unsigned resource, unsigned amount);
		bool fillsBorrow(unsigned task, unsigned resource, unsigned amount);
		bool fillsConsume(unsigned task, unsigned resource, unsigned amount);

		extern unsigned mean_time;

		void init(char const* filename);
	}
}

#endif