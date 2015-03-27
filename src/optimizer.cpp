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
				if ((unsigned)usable > value.less_than)
					return false;

		if (value.greater_than)
		{
			if (usable > 0)
			{
				if ((unsigned)usable < value.greater_than)
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
			if (s.final_state.resources[index].quantity > value.less_than)
				return false;
		}
		if (value.greater_than)
		{
			if (s.final_state.resources[index].quantity < value.greater_than)
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
			if (s.final_state.resources[index].used > value.less_than)
				return false;
		}
		if (value.greater_than)
		{
			if (s.final_state.resources[index].used < value.greater_than)
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

unsigned BuildOrder::Optimizer::Optimizer::numberObjectives() const
{
	return objectives[0].row.size()+objectives[1].row.size()+objectives[2].row.size();
}
unsigned BuildOrder::Optimizer::Optimizer::numberRestrictions() const
{
	return restrictions[0].row.size()+restrictions[1].row.size()+restrictions[2].row.size();
}

void BuildOrder::Optimizer::Optimizer::update()
{
	_objV.resize(Rules::tasks.size(),0);
	_incV.resize(Rules::tasks.size());
	_decV.resize(Rules::tasks.size());

	//INITIAL MAP
	for (unsigned t = 0; t < Rules::tasks.size(); t++)
	{
		//RESOURCE BONUS/PENALTY
		for (unsigned p = 0; p < Rules::resources.size(); p++)
		{
			double resource_reduce = 0;
			double resource_increase = 0;

			resource_reduce += Rules::tasks[t].costs.get(p) > 0 ? 1 : 0;
			resource_reduce += Rules::tasks[t].consume.get(p) ? 1 : 0;
			
			resource_increase += Rules::tasks[t].costs.get(p) < 0 ? 1 : 0;
			resource_increase += Rules::tasks[t].produce.get(p) ? 1 : 0;

			resource_increase += Rules::taskValuePerEvent[p].get(t);

			_incV[t].set(p,resource_increase);
			_decV[t].set(p,resource_reduce);

			if (resource_reduce > 0 || resource_increase > 0)
			{
				//OBJECTIVES
				{
					double objective_value = 0;

					if (objectives[0].get(p))
					{
						if (objectives[0].get(p) == MINIMIZE)
						{
							objective_value += resource_reduce;
							objective_value -= resource_increase/2;
						}
						else
						{
							objective_value -= resource_reduce/2;
							objective_value += resource_increase;
						}
					}

					if (objectives[1].get(p))
					{
						if (objectives[1].get(p) == MINIMIZE)
							objective_value -= resource_increase/2;
						else
							objective_value += resource_increase;
					}

					if (objectives[2].get(p))
					{
						if (objectives[2].get(p) == MINIMIZE)
							objective_value -= resource_reduce/2;
						else
							objective_value += resource_reduce;
					}

					_objV[t] += objective_value;// * objective_multiplier;
				}
			}
		}

		//COLLATERAL DAMAGE (EVENT DESTRUCTION)
		{
			//COSTS
			for (unsigned k = 0; k < Rules::tasks[t].costs.row.size(); k++)
			{
				unsigned index = Rules::tasks[t].costs.row[k].index;

				for (unsigned e = 0; e < Rules::resourceValueLost[index].row.size(); e++)
				{
					unsigned collateral = Rules::resourceValueLost[index].row[e].index;
					double damage = 1;
					
					double objective_value = 0;

					for (unsigned o = 0; o < objectives[0].row.size(); o++)
					{
						unsigned obj = objectives[0].row[o].index;
						Objective type = objectives[0].row[o].value;

						if (collateral == obj)
						{
							if (type == MINIMIZE)
								objective_value += damage;
							else
								objective_value -= damage/2;
						}
					}

					for (unsigned o = 0; o < objectives[2].row.size(); o++)
					{
						unsigned obj = objectives[2].row[o].index;
						Objective type = objectives[2].row[o].value;

						if (collateral == obj)
						{
							if (type == MINIMIZE)
								objective_value -= damage/2;
							else
								objective_value += damage;
						}
					}

					_objV[t] += objective_value;// * objective_multiplier;
				}
			}

			//CONSUME
			for (unsigned k = 0; k < Rules::tasks[t].consume.row.size(); k++)
			{
				unsigned index = Rules::tasks[t].consume.row[k].index;

				for (unsigned e = 0; e < Rules::resourceValueLost[index].row.size(); e++)
				{
					unsigned collateral = Rules::resourceValueLost[index].row[e].index;
					double damage = 1;
					
					double objective_value = 0;

					for (unsigned o = 0; o < objectives[0].row.size(); o++)
					{
						unsigned obj = objectives[0].row[o].index;
						Objective type = objectives[0].row[o].value;

						if (collateral == obj)
						{
							if (type == MINIMIZE)
								objective_value += damage;
							else
								objective_value -= damage/2;
						}
					}

					for (unsigned o = 0; o < objectives[2].row.size(); o++)
					{
						unsigned obj = objectives[2].row[o].index;
						Objective type = objectives[2].row[o].value;

						if (collateral == obj)
						{
							if (type == MINIMIZE)
								objective_value -= damage/2;
							else
								objective_value += damage;
						}
					}

					_objV[t] += objective_value;// * objective_multiplier;
				}
			}
		}

	}
}

std::vector<double> BuildOrder::Optimizer::Optimizer::initialMap(double o, double r, GameState initial) const
{
	std::vector<double> taskValue(_objV.size(),0);

	//RESTRICTIONS
	for (unsigned t = 0; t < taskValue.size(); t++)
		for (unsigned p = 0; p < Rules::resources.size(); p++)
		{
			double restriction_value = 0;

			if (restrictions[0].get(p))
			{
				unsigned final = initial.resources[p].usable();

				if (restrictions[0].get(p).less_than)
					if (final >= restrictions[0].get(p).less_than)
					{
						restriction_value += _decV[t].get(p);
						restriction_value -= _incV[t].get(p)/2;
					}
				
				if (restrictions[0].get(p).greater_than)
					if (final <= restrictions[0].get(p).greater_than)
					{
						restriction_value -= _decV[t].get(p)/2;
						restriction_value += _incV[t].get(p);
					}
			}

			if (restrictions[1].get(p))
			{
				unsigned final = initial.resources[p].quantity;
				
				if (restrictions[1].get(p).less_than)
					if (final >= restrictions[1].get(p).less_than)
					{
						restriction_value += _decV[t].get(p);
						restriction_value -= _incV[t].get(p)/2;
					}
				
				if (restrictions[1].get(p).greater_than)
					if (final <= restrictions[1].get(p).greater_than)
					{
						restriction_value -= _decV[t].get(p)/2;
						restriction_value += _incV[t].get(p);
					}
			}

			if (restrictions[2].get(p))
			{
				unsigned final = initial.resources[p].used;
				
				if (restrictions[2].get(p).less_than)
					if (final >= restrictions[2].get(p).less_than)
					{
						restriction_value += _decV[t].get(p);
						restriction_value -= _incV[t].get(p)/2;
					}
				
				if (restrictions[2].get(p).greater_than)
					if (final <= restrictions[2].get(p).greater_than)
					{
						restriction_value -= _decV[t].get(p)/2;
						restriction_value += _incV[t].get(p);
					}
			}

			taskValue[t] += restriction_value * r;// * restriction_multiplier;
		}

	for (unsigned t = 0; t < Rules::tasks.size(); t++)
		taskValue[t] += _objV[t] * o;// + _resV[t] * r;

	return taskValue;
}

void BuildOrder::Optimizer::initOptimizer(Optimizer& o, char const* f)
{
	std::fstream file;
	std::string buffer;
	std::stringstream ss;

	file.open(f);

	//OBJECTIVES
	do
	{
		std::getline(file,buffer);

		Objective min = (buffer[0] == 'm') ? MINIMIZE : MAXIMIZE;

		if (buffer[0] != 'm' && buffer[0] != 'M')
			break;

		unsigned l = 1;
		while (buffer[l] == ' ')
			l++;

		unsigned target;
		switch(buffer[l])
		{
			case 'U' : target = 0; break;
			case 'q' : target = 1; break;
			case 'u' : target = 2; break;
			default:
				file.close();
				return;
		}

		l++;
		ss.str(&buffer[l]);

		unsigned index;
		ss >> index;
		ss.clear();

		o.objectives[target].set(index, min);

	} while (buffer[0] == 'm' || buffer[0] == 'M');

	while(buffer.length() == 0)
		std::getline(file,buffer);

	//TIME RESTRICTION
	{
		if (buffer[0] != 't')
		{
			file.close();
			return;
		}

		unsigned l = 1;
		while (buffer[l] == ' ' || buffer[l] == '<')
			l++;

		ss.str(&buffer[l]);
		unsigned mtime;
		ss >> mtime;
		ss.clear();

		o.maximum_time = mtime;
	}

	do
	{
		std::getline(file,buffer);
		
		unsigned l = 0;
		unsigned target;
		switch(buffer[l])
		{
			case 'U' : target = 0; break;
			case 'q' : target = 1; break;
			case 'u' : target = 2; break;
			default:
				file.close();
				return;
		}

		l++;
		ss.str(&buffer[l]);

		unsigned index;
		ss >> index;
		ss.clear();

		while (buffer[l] != '<' && buffer[l] != '>')
			l++;

		bool less = (buffer[l] == '<');
		l++;

		ss.str(&buffer[l]);

		unsigned value;
		ss >> value;
		ss.clear();

		Restriction r = o.restrictions[target].get(index);
		
		if (less)
			r.less_than = value;
		else
			r.greater_than = value;

		o.restrictions[target].set(index, r);

		if (file.eof())
			break;
	} while (buffer[0] == 'U' || buffer[0] == 'u'|| buffer[0] == 'q');
}