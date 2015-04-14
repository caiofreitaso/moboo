#ifndef BUILDORDERH
#define BUILDORDERH

#include "graph.h"
#include "util.h"

namespace BuildOrder
{

	struct TaskPointer {
		unsigned task;
		unsigned delay;

		TaskPointer()
		: task(0), delay(0)
		{ }

		TaskPointer(TaskPointer const& t)
		: task(t.task), delay(t.delay)
		{ }

		TaskPointer(unsigned t)
		: task(t), delay(0)
		{ }

		TaskPointer(unsigned t, unsigned d)
		: task(t), delay(d)
		{ }
	};

	struct EventPointer {
		unsigned start;
		unsigned type;

		EventPointer()
		: start(0), type(0)
		{ }

		EventPointer(unsigned s, unsigned t)
		: start(s), type(t)
		{ }
	};

	typedef std::vector<TaskPointer> BuildOrder;
	typedef std::vector<EventPointer> EventList;

	void print(BuildOrder const& b);

	struct Resource {
		unsigned quantity;
		unsigned used;
		unsigned borrowed;
		EventList events;

		bool capacity;

		int usable() const { return (int)(quantity - used); }
		int usableB() const { return usable() - (int)borrowed; }
	};

	struct GameState {
		unsigned time;

		std::vector<Resource> resources;
		EventList tasks;

		void print() const
		{
			std::cout << "TIME: " << time << "\n";
			
			for (unsigned i = 0; i < resources.size(); i++)
				if (resources[i].usable())
					std::cout << i << ": " << resources[i].usable() << "\n";
		}

		bool hasPrerequisites(unsigned t) const
		{
			for (unsigned i = 0; i < Rules::tasks[t].prerequisite.row.size(); i++)
			{
				unsigned index = Rules::tasks[t].prerequisite.row[i].index;
				unsigned value = Rules::tasks[t].prerequisite.row[i].value;

				if (resources[index].usable() < value)
					return false;
			}

			return true;
		}

		unsigned quantity(unsigned index) const
		{
			if (Rules::resources[index].overall_maximum)
				if (resources[index].quantity > Rules::resources[index].overall_maximum)
					return Rules::resources[index].overall_maximum;

			return resources[index].quantity;
		}

		int usable(unsigned index) const
		{
			return quantity(index) - resources[index].used;
		}

		static bool hasOMaximum(unsigned task, std::vector<unsigned> r)
		{
			for (unsigned i = 0; i < Rules::tasks[task].produce.row.size(); i++)
			{
				unsigned index = Rules::tasks[task].produce.row[i].index;
				unsigned value = Rules::tasks[task].produce.row[i].value;
		
				if (Rules::resources[index].overall_maximum)
					if (r[index] + value > Rules::resources[index].overall_maximum)
						return false;
			}
			return true;
		}

		static bool hasMaximum(unsigned task, std::vector<unsigned> r)
		{
			for (unsigned i = 0; i < Rules::tasks[task].produce.row.size(); i++)
			{
				unsigned index = Rules::tasks[task].produce.row[i].index;
				unsigned value = Rules::tasks[task].produce.row[i].value;
				
				if (r[index] + value > maximum(index,r))
					return false;
			}
			return true;
		}

		static unsigned maximum(unsigned index, std::vector<unsigned> res)
		{
			if (Rules::resources[index].maximum_per_resource.row.size())
			{
				unsigned max = 0;
				for (unsigned r = 0; r < Rules::resources[index].maximum_per_resource.row.size(); r++)
				{
					unsigned max_index = Rules::resources[index].maximum_per_resource.row[r].index;
					unsigned max_value = Rules::resources[index].maximum_per_resource.row[r].value;

					max += res[max_index] * max_value;
				}
				return max;
			}
			return -1;
		}

		unsigned maximum(unsigned index) const
		{
			if (Rules::resources[index].maximum_per_resource.row.size())
			{
				unsigned max = 0;
				for (unsigned r = 0; r < Rules::resources[index].maximum_per_resource.row.size(); r++)
				{
					unsigned max_index = Rules::resources[index].maximum_per_resource.row[r].index;
					unsigned max_value = Rules::resources[index].maximum_per_resource.row[r].value;

					max += resources[max_index].usable() * max_value;
				}
				return max;
			}
			return -1;
		}

		void applyEvent(unsigned i)
		{
			MatrixRow<unsigned> limited_indexes;

			for (unsigned r = 0; r < Rules::events[i].bonus.row.size(); r++)
			{
				unsigned index = Rules::events[i].bonus.row[r].index;
				if (Rules::resources[index].maximum_per_resource.row.size())
					limited_indexes.set(r,1);
			}

			for (unsigned k = 0; k < Rules::events[i].bonus.row.size(); k++)
			{
				unsigned index = Rules::events[i].bonus.row[k].index;
				unsigned value = Rules::events[i].bonus.row[k].value;

				if (limited_indexes.get(k))
				{
					unsigned max = maximum(index);
					unsigned add = usable(index) + value;

					if (max < add)
						resources[index].quantity += (add - max) - usable(index);
					else
						resources[index].quantity += value;
				}
				else
					resources[index].quantity += value;
			}
		}

		void produceResource(unsigned index, unsigned quantity)
		{
			unsigned added = quantity;

			if (Rules::resources[index].maximum_per_resource.row.size())
			{
				unsigned max = maximum(index);

				if (max > quantity)
					resources[index].quantity += quantity;
				else
				{
					resources[index].quantity = max;
					added = max - quantity;
				}
			}
			else
				resources[index].quantity += quantity;

			for (unsigned k = 0; k < Rules::events.size(); k++)
				if (Rules::events[k].trigger == index)
					for (unsigned q = 0; q < added; q++)
					{
						if (Rules::events[k].time)
							resources[index].events.push_back(EventPointer(time, k));
						else
							applyEvent(k);
					}
		}

		void consumeResource(unsigned type)
		{
			if (resources[type].events.size())
			{
				unsigned step = 0;
				unsigned min_remaining_time[resources[type].usable()];

				for (unsigned i = 0; i < Rules::events.size(); i++)
					if (Rules::events[i].trigger == type)
						step++;

				for (int i = 0; i < resources[type].usable(); i++)
					min_remaining_time[i] = -1;

				for (unsigned i = 0; i < resources[type].events.size(); i++)
				{
					unsigned evt_time = this->time - resources[type].events[i].start;
					evt_time %= Rules::events[resources[type].events[i].type].time;
					evt_time = Rules::events[resources[type].events[i].type].time - evt_time;

					if (evt_time < min_remaining_time[i/step])
						min_remaining_time[i/step] = evt_time;
				}

				unsigned max = 0;
				unsigned origin = -1;

				for (int i = 0; i < resources[type].usable(); i++)
					if (min_remaining_time[i] > max)
					{
						max = min_remaining_time[i];
						origin = i*step;
					}

				for (unsigned i = 0; i < step; i++)
					pop(resources[type].events, origin);
			}

			resources[type].used++;
		}
	};

	void createState(GameState& s);
	void initState(GameState&, char const*);
}

#endif