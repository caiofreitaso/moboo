#include "../include/graph.h"

Graph::MultiGraph<Dependency> graph;

std::vector<Dependency> BuildOrder::Rules::value(unsigned needs, unsigned needed)
{
	std::vector<Dependency> ret(5);
	for (unsigned i = 0; i < 5; i++)
		ret.type = (relation_type) i;

	for (unsigned j = 0; j < tasks[needs].costs.row.size(); j++)
	{
		unsigned idx = tasks[needs].costs.row[j].index;
		int val = tasks[needs].costs.row[j].value;

		if (val > 0)
			for (unsigned i = 0; i < tasks[needed].produce.row.size(); i++)
			{
				unsigned index = tasks[needed].produce.row[i].index;
				unsigned value = tasks[needed].produce.row[i].value;

				if (idx == index)
				{
					ret[0].weight.set(idx, best_function_ever(value, val));
					ret[0].bonus.set(idx, value);
				}

				if (resourceValueLost[index].get(idx))
					ret[0].event.set(idx, 1.5);
			}
	}

	for (unsigned j = 0; j < tasks[needs].consume.row.size(); j++)
	{
		unsigned idx = tasks[needs].consume.row[j].index;
		unsigned val = tasks[needs].consume.row[j].value;

		for (unsigned i = 0; i < tasks[needed].produce.row.size(); i++)
		{
			unsigned index = tasks[needed].produce.row[i].index;
			unsigned value = tasks[needed].produce.row[i].value;
	
			if (idx == index)
			{
				ret[1].weight.set(idx, best_function_ever(value, val));
				ret[1].bonus.set(idx, value);
			}

			if (resourceValueLost[index].get(idx))
				ret[1].event.set(idx, 1.5);
		}
	}

	for (unsigned j = 0; j < tasks[needs].produce.row.size(); j++)
	{
		unsigned idx = tasks[needs].produce.row[j].index;
		unsigned val = tasks[needs].produce.row[j].value;

		for (unsigned i = 0; i < tasks[needed].produce.row.size(); i++)
		{
			unsigned index = tasks[needed].produce.row[i].index;
			unsigned value = tasks[needed].produce.row[i].value;
			
			unsigned max = resources[idx].maximum_per_resource.get(index);
			if (max)
			{
				ret[2].bonus.set(idx, max);
				ret[2].weight.set(idx, best_function_ever(max, value));
			}

			for (unsigned k = 0; k < resources[idx].maximum_per_resource.row.size(); k++)
			{
				unsigned r_idx = resources[idx].maximum_per_resource.row[k].index;
				if (resourceValueLost[index].get(r_idx))
					ret[2].event.set(idx, 1.5);
			}
		}
	}

	for (unsigned j = 0; j < tasks[needs].borrow.row.size(); j++)
	{
		unsigned idx = tasks[needs].borrow.row[j].index;
		unsigned val = tasks[needs].borrow.row[j].value;

		for (unsigned i = 0; i < tasks[needed].produce.row.size(); i++)
		{
			unsigned index = tasks[needed].produce.row[i].index;
			unsigned value = tasks[needed].produce.row[i].value;

			if (idx == index)
			{
				ret[3].weight.set(idx, best_function_ever(value, val));
				ret[3].bonus.set(idx, value);
			}
			else if (resourceValueLost[index].get(idx))
				ret[3].event.set(idx, 1);
		}
	}

	for (unsigned j = 0; j < tasks[needs].prerequisite.row.size(); j++)
	{
		unsigned idx = tasks[needs].prerequisite.row[j].index;
		unsigned val = tasks[needs].prerequisite.row[j].value;

		for (unsigned i = 0; i < tasks[needed].produce.row.size(); i++)
		{
			unsigned index = tasks[needed].produce.row[i].index;
			unsigned value = tasks[needed].produce.row[i].value;

			if (idx == index)
				ret[4].weight.set(idx, 1);
			else if (resourceValueLost[index].get(idx))
				ret[4].event.set(idx, 1);
		}
	}

	for (unsigned i = 0; i < 5; i++)
	{
		if (ret[i].bonus.row.size())
			continue;
		if (ret[i].weight.row.size())
			continue;
		if (ret[i].event.row.size())
			continue;
		ret.erase(ret.begin()+i);
		i--;
	}

	return ret;
}

void BuildOrder::Rules::initGraph()
{
	graph.vertices(tasks.size());

	for (unsigned i = 0; i < tasks.size(); i++)
		for (unsigned j = 0; j < tasks.size(); j++)
			if (i != j)
			{
				std::vector<Dependency> d = value(i,j);
				for (unsigned k = 0; k < d.size(); k++)
					graph.insert(i,j,0,d[k]);
			}
}