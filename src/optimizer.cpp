#include "../include/optimizer.h"

bool BuildOrder::Optimizer::Optimizer::dominates(Solution a, Solution b) const
{
	std::vector<unsigned> a_obj, b_obj;
	std::vector<bool> min;

	if (time_as_objective)
	{
		min.push_back(true);

		a_obj.push_back(a.final_state.time);
		b_obj.push_back(b.final_state.time);
	}

	for (unsigned i = 0; i < objectives[0].row.size(); i++)
	{
		unsigned index = objectives[0].row[i].index;

		a_obj.push_back(a.final_state.resources[index].usable());
		b_obj.push_back(b.final_state.resources[index].usable());
		min.push_back(objectives[0].row[i].value == MINIMIZE);
	}

	for (unsigned i = 0; i < objectives[1].row.size(); i++)
	{
		unsigned index = objectives[1].row[i].index;

		a_obj.push_back(a.final_state.resources[index].quantity);
		b_obj.push_back(b.final_state.resources[index].quantity);
		min.push_back(objectives[1].row[i].value == MINIMIZE);
	}

	for (unsigned i = 0; i < objectives[2].row.size(); i++)
	{
		unsigned index = objectives[2].row[i].index;

		a_obj.push_back(a.final_state.resources[index].used);
		b_obj.push_back(b.final_state.resources[index].used);
		min.push_back(objectives[2].row[i].value == MINIMIZE);
	}

	bool strictly = false;
	for (unsigned i = 0; i < a_obj.size(); i++)
		if (min[i])
		{
			if (a_obj[i] < b_obj[i])
				strictly = true;
			else if (a_obj[i] > b_obj[i])
				return false;
		}
		else
		{
			if (a_obj[i] > b_obj[i])
				strictly = true;
			else if (a_obj[i] < b_obj[i])
				return false;
		}

	return strictly;
}

bool BuildOrder::Optimizer::Optimizer::valid(Solution s) const
{
	for (unsigned i = 0; i < restrictions[0].row.size(); i++)
	{
		unsigned index = restrictions[0].row[i].index;
		Restriction value;
		value.less_than = restrictions[0].row[i].value.less_than;
		value.greater_than = restrictions[0].row[i].value.greater_than;

		int usable = s.final_state.resources[index].usable();
		
		if (value.less_than)
			if (usable > 0)
				if ((unsigned)usable >= value.less_than)
					return false;

		if (value.greater_than)
		{
			if (usable > 0)
			{
				if ((unsigned)usable <= value.greater_than)
					return false;
			} else
				return false;
		}
	}

	for (unsigned i = 0; i < restrictions[1].row.size(); i++)
	{
		unsigned index = restrictions[1].row[i].index;
		Restriction value;
		value.less_than = restrictions[1].row[i].value.less_than;
		value.greater_than = restrictions[1].row[i].value.greater_than;

		if (value.less_than)
		{
			if (s.final_state.resources[index].quantity >= value.less_than)
				return false;
		}
		if (value.greater_than)
		{
			if (s.final_state.resources[index].quantity <= value.greater_than)
				return false;
		}
	}

	for (unsigned i = 0; i < restrictions[2].row.size(); i++)
	{
		unsigned index = restrictions[2].row[i].index;
		Restriction value;
		value.less_than = restrictions[2].row[i].value.less_than;
		value.greater_than = restrictions[2].row[i].value.greater_than;

		if (value.less_than)
		{
			if (s.final_state.resources[index].used >= value.less_than)
				return false;
		}
		if (value.greater_than)
		{
			if (s.final_state.resources[index].used <= value.greater_than)
				return false;
		}
	}

	return true;

}

unsigned BuildOrder::Optimizer::Optimizer::producesMaximize(unsigned task) const
{
	unsigned ret = 0;
	for (unsigned i = 0; i < Rules::tasks[task].produce.row.size(); i++)
		for (unsigned o = 0; o < 3; o++)
			if (objectives[o].get(Rules::tasks[task].produce.row[i].index) == MAXIMIZE)
				ret++;
	return ret;
}

unsigned BuildOrder::Optimizer::Optimizer::producesGreaterThan(unsigned task) const
{
	unsigned ret = 0;
	for (unsigned i = 0; i < Rules::tasks[task].produce.row.size(); i++)
		for (unsigned o = 0; o < 3; o++)
			if (restrictions[o].get(Rules::tasks[task].produce.row[i].index).greater_than)
				ret++;
	return ret;
}

BuildOrder::Optimizer::Population BuildOrder::Optimizer::Optimizer::nonDominated(Population p) const
{
	Population ret;

	std::sort(p.begin(),p.end());

	ret.push_back(p[0]);

	for (unsigned i = 1; i < p.size(); i++)
	{
		bool included = false;
		
		for (unsigned k = 0; k < ret.size(); k++)
			if (p[i] == ret[k])
			{
				included = true;
				break;
			}

		if (!included)
		{
			unsigned count = 0;
			for (unsigned k = 0; k < ret.size(); k++)
				if (dominates(p[i],ret[k]))
				{
					pop(ret, k);
					k--;
					count++;
				}

			bool dominated = false;
			if (!count)
				for (unsigned k = 0; k < ret.size(); k++)
					if (dominates(ret[k],p[i]))
					{
						pop(p, i);
						i--;
						dominated = true;
						break;
					}
			
			if (!dominated)
				ret.push_back(p[i]);
		}
	}

	return ret;
}