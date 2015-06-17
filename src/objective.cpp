#include "../include/objective.h"

bool BuildOrder::maximumOverflow(unsigned task, GameState const& s)
{
	for (unsigned i = 0; i < Rules::tasks[task].produce.row.size(); i++)
	{
		unsigned index = Rules::tasks[task].produce.row[i].index;
		unsigned value = Rules::tasks[task].produce.row[i].value;

		unsigned corrected_max = 0;

		for (unsigned j = 0; j < Rules::resources[index].maximum_per_resource.row.size(); j++)
		{
			unsigned max_index = Rules::resources[index].maximum_per_resource.row[j].index;
			unsigned max_value = Rules::resources[index].maximum_per_resource.row[j].value;

			for (unsigned k = 0; k < Rules::tasks[task].produce.row.size(); k++)
			{
				unsigned o_index = Rules::tasks[task].produce.row[k].index;
				unsigned o_value = Rules::tasks[task].produce.row[k].value;

				if (max_index == o_index)
					corrected_max += max_value * o_value;
			}
		}

		if (s.maximum(index) + corrected_max < s.resources[index].usable() + value)
			return true;
	}

	return false;
}

bool BuildOrder::hasPrerequisites(unsigned task, GameState const& s)
{
	//Do we exceed the maximum for a certain item?
	if (maximumOverflow(task, s))
		return false;

	//Do we have the prerequisites for construction?
	for (unsigned i = 0; i < Rules::tasks[task].prerequisite.row.size(); i++)
	{
		unsigned index = Rules::tasks[task].prerequisite.row[i].index;
		unsigned value = Rules::tasks[task].prerequisite.row[i].value;

		unsigned done = s.resources[index].usable();

		for (unsigned j = 0; j < Rules::resources[i].equivalence.row.size(); j++)
		{
			unsigned e_index = Rules::resources[i].equivalence.row[i].index;
			unsigned e_value = Rules::resources[i].equivalence.row[i].value;
			
			done += s.resources[e_index].usable() * e_value;
		}

		if (done < value)
			return false;
	}

	return true;
}

bool BuildOrder::can(unsigned task, GameState const& s)
{
	if (!hasPrerequisites(task,s))
		return false;

	std::vector<int> use(Rules::resources.size(),0);

	//Can we consume the right amount of resources?
	for (unsigned i = 0; i < Rules::tasks[task].consume.row.size(); i++)
	{
		unsigned index = Rules::tasks[task].consume.row[i].index;
		unsigned value = Rules::tasks[task].consume.row[i].value;
		
		use[index] += value;
	}

	//Can we borrow the right amount of resources?
	for (unsigned i = 0; i < Rules::tasks[task].borrow.row.size(); i++)
	{
		unsigned index = Rules::tasks[task].borrow.row[i].index;
		unsigned value = Rules::tasks[task].borrow.row[i].value;
		
		use[index] += value;
	}

	//Can we pay for it?
	for (unsigned i = 0; i < Rules::tasks[task].costs.row.size(); i++)
	{
		unsigned index = Rules::tasks[task].costs.row[i].index;
		int value = Rules::tasks[task].costs.row[i].value;

		use[index] += value;
	}

	for (unsigned i = 0; i < Rules::resources.size(); i++)
	{
		int done = s.resources[i].usableB();

		if (done < use[i])
			return false;
	}

	return true;
}

bool BuildOrder::can(TaskPointer t, GameState const& s)
{
	return can(t.task,s);
}

void BuildOrder::Objective::resourcesByEvents(std::vector<bool>& r, GameState const& init)
{
	for (auto event : Rules::events)
	{
		EventList const& events = init.resources[event.trigger].events;

		bool done = false;

		if (events.size())
		{
			for (auto e : event.bonus.row)
				r[e.index] = false;
			done = true;
		}

		for (auto t : init.tasks)
			if (!done)
			{
				unsigned produce = Rules::tasks[t.type].produce.get(event.trigger);
				if (produce)
				{
					for (auto e : event.bonus.row)
						r[e.index] = false;
					done = true;
				}
			} else
				break;
	}
}

void BuildOrder::Objective::afterStack(std::vector<unsigned>& final,
	std::vector<unsigned>& finalQ, GameState const& init)
{
	for (unsigned i = 0; i < final.size(); i++)
		final[i] = init.resources[i].usableB();
	for (unsigned i = 0; i < finalQ.size(); i++)
		finalQ[i] = init.resources[i].quantity;

	for (unsigned k = 0; k < init.tasks.size(); k++)
	{
		unsigned q = init.tasks[k].type;
		Rules::Task& task = Rules::tasks[q];

		for (unsigned p = 0; p < task.produce.row.size(); p++)
		{
			unsigned p_val = task.produce.row[p].value;
			unsigned p_idx = task.produce.row[p].index;

			final[p_idx] += p_val;
			finalQ[p_idx] += p_val;
		}

		for (unsigned p = 0; p < task.consume.row.size(); p++)
		{
			unsigned p_val = task.consume.row[p].value;
			unsigned p_idx = task.consume.row[p].index;

			final[p_idx] -= p_val;
		}
	}
}

bool BuildOrder::Objective::prerequisiteInStack(std::vector<bool>& r,
	std::vector<unsigned>& final, unsigned t)
{
	std::vector<int> values(r.size(), 0);

	for (unsigned i = 0; i < Rules::tasks[t].prerequisite.row.size(); i++)
	{
		int v = Rules::tasks[t].prerequisite.row[i].value;
		unsigned index = Rules::tasks[t].prerequisite.row[i].index;

		values[index] += v;
	}

	for (unsigned i = 0; i < Rules::tasks[t].costs.row.size(); i++)
	{
		int v = Rules::tasks[t].costs.row[i].value;
		unsigned index = Rules::tasks[t].costs.row[i].index;
		
		values[index] += v;
	}

	for (unsigned i = 0; i < Rules::tasks[t].borrow.row.size(); i++)
	{
		int v = Rules::tasks[t].borrow.row[i].value;
		unsigned index = Rules::tasks[t].borrow.row[i].index;
		
		values[index] += v;
	}

	for (unsigned i = 0; i < Rules::tasks[t].consume.row.size(); i++)
	{
		int v = Rules::tasks[t].consume.row[i].value;
		unsigned index = Rules::tasks[t].consume.row[i].index;
		
		values[index] += v;
	}

	for (unsigned i = 0; i < values.size(); i++)
		if (r[i])
			if (final[i] < values[i] && values[i] > 0)
				return false;

	return true;
}

bool BuildOrder::Objective::possible(BuildOrder& build, GameState const& init)
{
	return possible(build[0].task, init);
}

bool BuildOrder::Objective::possible(unsigned task, GameState const& init)
{
	std::vector<bool> should_I_Care(Rules::resources.size(),true);
	std::vector<unsigned> final(Rules::resources.size(), 0);
	std::vector<unsigned> finalQ(Rules::resources.size(), 0);

	afterStack(final, finalQ, init);
	resourcesByEvents(should_I_Care, init);

	if (!GameState::hasOMaximum(task,finalQ))
		return false;
	if (!GameState::hasMaximum(task,final))
		return false;
	if (!prerequisiteInStack(should_I_Care, final, task))
		return false;

	return true;
}

void BuildOrder::Objective::updateCosts(BuildOrder& build, GameState& init)
{
	for (unsigned i = 0; i < Rules::tasks[build.front().task].costs.row.size(); i++)
	{
		unsigned index = Rules::tasks[build.front().task].costs.row[i].index;
		int value = Rules::tasks[build.front().task].costs.row[i].value;

		if (value > 0)
			for (unsigned k = 0; k < (unsigned) value; k++)
				init.consumeResource(index);
		else
			init.resources[index].used += value;
	}
}

void BuildOrder::Objective::updateBorrow(BuildOrder& build, GameState& init)
{
	for (unsigned i = 0; i < Rules::tasks[build.front().task].borrow.row.size(); i++)
	{
		unsigned index = Rules::tasks[build.front().task].borrow.row[i].index;
		unsigned value = Rules::tasks[build.front().task].borrow.row[i].value;

		init.resources[index].borrowed += value;
	}

	for (unsigned i = 0; i < Rules::tasks[build.front().task].consume.row.size(); i++)
	{
		unsigned index = Rules::tasks[build.front().task].consume.row[i].index;
		unsigned value = Rules::tasks[build.front().task].consume.row[i].value;

		init.resources[index].borrowed += value;
	}
}

void BuildOrder::Objective::buildWhatYouCan(BuildOrder& build, GameState& init, unsigned& last, remaining_list& listTime)
{
	while (can(build.front(),init))
		if (last >= build.front().delay)
		{
			init.tasks.push_back( EventPointer(init.time, build.front().task) );
			listTime.push_back( time_helper(init.tasks.size()-1, Rules::tasks[build.front().task].time) );

			updateCosts(build, init);
			updateBorrow(build, init);

			pop(build);

			last = 0;

			if (!build.size())
				break;
		} else
			break;
}

void BuildOrder::Objective::aftermath(GameState& init, unsigned type)
{
	for (unsigned j = 0; j < Rules::tasks[type].consume.row.size(); j++)
	{
		unsigned index = Rules::tasks[type].consume.row[j].index;
		unsigned value = Rules::tasks[type].consume.row[j].value;
		
		for (unsigned k = 0; k < value; k++)
			init.consumeResource(index);
		init.resources[index].borrowed -= value;
	}

	for (unsigned k = 0; k < Rules::tasks[type].produce.row.size(); k++)
	{
		unsigned index = Rules::tasks[type].produce.row[k].index;
		unsigned value = Rules::tasks[type].produce.row[k].value;

		init.produceResource(index, value);
	}

	for (unsigned k = 0; k < Rules::tasks[type].borrow.row.size(); k++)
	{
		unsigned index = Rules::tasks[type].borrow.row[k].index;
		unsigned value = Rules::tasks[type].borrow.row[k].value;

		init.resources[index].borrowed -= value;
	}
}

void BuildOrder::Objective::updateOngoingTasks(GameState& init, remaining_list& listTime)
{
	for (unsigned i = 0; i < init.tasks.size(); i++)
	{
		unsigned finish_time = Rules::tasks[init.tasks[i].type].time + init.tasks[i].start;
		
		if ( init.time >= finish_time )
			if ( !maximumOverflow(init.tasks[i].type, init) )
			{
				aftermath(init, init.tasks[i].type);

				for (unsigned k = 0; k < listTime.size(); k++)
					if (listTime[k].pointer > i)
						listTime[k].pointer--;
					else if (listTime[k].pointer == i)
					{
						pop(listTime, k);
						k--;
					}

				pop(init.tasks, i);
				i--;
			}
	}
}

void BuildOrder::Objective::updateOngoingEvents(GameState& init, remaining_list& listEvents)
{
	for (unsigned r = 0; r < init.resources.size(); r++)
		for (unsigned i = 0; i < init.resources[r].events.size(); i++)
		{
			unsigned total_time = Rules::events[init.resources[r].events[i].type].time;
			unsigned time_remaining = (init.time - init.resources[r].events[i].start);
			time_remaining %= total_time;
			time_remaining = total_time - time_remaining;

			listEvents.push_back(time_helper(0,0));

			if (time_remaining < total_time)
				listEvents.back().time_remaining = time_remaining;
			else
			{
				if (init.time - init.resources[r].events[i].start > 0)
					init.applyEvent(init.resources[r].events[i].type);
				
				listEvents.back().time_remaining = Rules::events[init.resources[r].events[i].type].time;
			}

		}
}

void BuildOrder::Objective::update(GameState& init, remaining_list& listTime, remaining_list& listEvents)
{
	Objective::updateOngoingTasks(init, listTime);
	Objective::updateOngoingEvents(init, listEvents);

}

unsigned BuildOrder::Objective::nextTime(GameState& init, remaining_list& listTime, remaining_list& listEvents,
	unsigned maximum_time, unsigned size)
{
	unsigned min = -1;
	for (unsigned i = 0; i < listTime.size(); i++)
		if (listTime[i].time_remaining < min)
		{
			if (listTime[i].time_remaining)
				min = listTime[i].time_remaining;
			else
			{
				unsigned thistask = init.tasks[listTime[i].pointer].type;
				if (!maximumOverflow(thistask,init))
					min = listTime[i].time_remaining;
			}
		}

	for (unsigned i = 0; i < listEvents.size(); i++)
		if (listEvents[i].time_remaining < min)
			min = listEvents[i].time_remaining;


	if (listTime.size() || size)
	{
		if (maximum_time)
		{
			unsigned time_to_max = maximum_time - init.time;
			if (time_to_max < min)
				min = time_to_max;
		}
	} else
		min = 0;

	for (unsigned i = 0; i < listTime.size(); i++)
		listTime[i].time_remaining -= min;

	return min;
}

BuildOrder::GameState BuildOrder::makespan(GameState init, BuildOrder& original, unsigned maximum_time)
{
	if (Rules::tasks.empty())
		return init;

	BuildOrder build(original);
	Objective::remaining_list listTime;
	unsigned last = 0;
	unsigned last_size = 0;

	while (build.size())
	{
		if (init.time >= maximum_time && maximum_time)
			break;

		if (last_size != build.size())
		{
			if (!Objective::possible(build, init))
				break;
			last_size = build.size();
		}
		
		Objective::remaining_list listEvents;
		
		Objective::update(init, listTime, listEvents);
		Objective::buildWhatYouCan(build, init, last, listTime);
		
		unsigned min = Objective::nextTime(init,listTime,listEvents,maximum_time,build.size());

		if (build.front().delay > last)
		{
			unsigned time_to_task = build.front().delay - last;
			if (time_to_task < min)
				min = time_to_task;
		}


		init.time += min;
		last += min;
	}

	while (init.tasks.size())
	{
		if (init.time >= maximum_time && maximum_time)
			break;

		Objective::remaining_list listEvents;
		Objective::update(init, listTime, listEvents);
		init.time += Objective::nextTime(init,listTime,listEvents,maximum_time,0);
	}

	if (build.size())
		original.erase(original.begin()+(original.size()-build.size()), original.end());

	return init;
}