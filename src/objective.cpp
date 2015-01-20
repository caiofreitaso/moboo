#include "../include/objective.h"

bool BuildOrder::maximumOverflow(unsigned task, GameState s)
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

bool BuildOrder::hasPrerequisites(unsigned task, GameState s)
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

bool BuildOrder::can(unsigned task, GameState s)
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
		int done = s.resources[i].usable();

		if (done < use[i])
			return false;
	}

	return true;
}

bool BuildOrder::can(TaskPointer t, GameState s)
{
	return can(t.task,s);
}

void BuildOrder::Objective::resourcesByEvents(std::vector<bool>& r, GameState& init)
{
	for (unsigned i = 0; i < Rules::events.size(); i++)
	{
		unsigned event = Rules::events[i];
		EventList& events = init.resources[event.trigger].events;

		if (events.size())
			for (unsigned k = 0; k < event.bonus.row.size(); k++)
				r[event.bonus.row[k].index] = false;
	}
}

bool BuildOrder::Objective::possible(BuildOrder& build, GameState& init)
{
	std::vector<bool> should_I_Care(Rules::resources.size(),true);

	unsigned task = build[0].task;

	resourcesByEvents(should_I_Care, init);

	for (unsigned i = 0; i < Rules::tasks[task].costs.row.size(); i++)
	{
		if ()
	}

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

void BuildOrder::Objective::updateBurrow(BuildOrder& build, GameState& init)
{
	for (unsigned i = 0; i < Rules::tasks[build.front().task].borrow.row.size(); i++)
	{
		unsigned index = Rules::tasks[build.front().task].borrow.row[i].index;
		unsigned value = Rules::tasks[build.front().task].borrow.row[i].value;

		init.resources[index].burrowed += value;
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
			updateBurrow(build, init);

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

		init.resources[index].burrowed -= value;
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

unsigned BuildOrder::Objective::update(GameState& init, remaining_list& listTime, unsigned maximum_time)
{
	remaining_list listEvents;

	Objective::updateOngoingTasks(init, listTime);
	Objective::updateOngoingEvents(init, listEvents);

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


	if (maximum_time)
	{
		unsigned time_to_max = maximum_time - init.time;
		if (time_to_max < min)
			min = time_to_max;
	}

	for (unsigned i = 0; i < listTime.size(); i++)
		listTime[i].time_remaining -= min;

	return min;
}

BuildOrder::GameState BuildOrder::makespan(GameState init, BuildOrder& original, unsigned maximum_time)
{
	if (Rules::tasks.empty() || Rules::events.empty())
		return init;

	BuildOrder build(original);
	Objective::remaining_list listTime;
	unsigned last = 0;

	while (build.size())
	{
		if (init.time >= maximum_time && maximum_time)
			break;

		Objective::buildWhatYouCan(build, init, last, listTime);

		//CHECK MAXIMUM
		/*
			bool possible = true;
			for (unsigned i = 0; i < Rules::tasks[build[0].task].produce.row.size(); i++)
			{
				unsigned index = Rules::tasks[build[0].task].produce.row[i].index;
				unsigned value = Rules::tasks[build[0].task].produce.row[i].value;

				if (init.usable(index) > 0)
				{
					unsigned usable = init.usable(index) + value;

					if (init.maximum(index) <= usable)
					{
						possible = false;
						
						for (unsigned k = 0; k < Rules::tasks[build[0].task].produce.row.size() && !possible; k++)
							if (i != k)
							{
								unsigned index2 = Rules::tasks[build[0].task].produce.row[k].index;

								for (unsigned m = 0; m < Rules::resources[index].maximum_per_resource.row.size(); m++)
									if (Rules::resources[index].maximum_per_resource.row[m].index == index2)
									{
										possible = true;
										break;
									}
							}

						for (unsigned k = 0; k < init.tasks.size() && !possible; k++)
							for (unsigned p = 0; p < Rules::tasks[init.tasks[k].type].produce.row.size() && !possible; p++)
								for (unsigned m = 0; m < Rules::resources[index].maximum_per_resource.row.size(); m++)
									if (Rules::tasks[init.tasks[k].type].produce.row[p].index ==
										Rules::resources[index].maximum_per_resource.row[m].index)
									{
										possible = true;
										break;
									}
						if (!possible)
							break;
					}
				}
			}

			if (!possible)
				break;
		*/
		
		unsigned min = Objective::update(init, listTime, maximum_time);

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

		init.time += Objective::update(init, listTime, maximum_time);
	}

	if (build.size())
		original.erase(original.begin()+(original.size()-build.size()), original.end());

	return init;
}