#include "../include/agarchiver.h"

void BuildOrder::Optimizer::AdaptativeGrid_Archiver::insert(Solution a)
{
	for (unsigned i = 0; i < size(); i++)
		if (_data[i] == a)
			return;

	unsigned previous = _data.size();
	
	_data.push_back(a);

	//NON-DOMINATED SET
	_data = _optimizer->nonDominated(_data);
	bool update = false;
	if (previous < _capacity)
		return;
	else if (_data.size() < _capacity)
		update = true;
	

	//TRANSLATE INTO VECTORS
	std::vector<std::vector<unsigned> > P(_data.size());
	std::vector<bool> obj;

	obj.push_back(true);
	for (unsigned i = 0; i < _optimizer->objectives[0].row.size(); i++)
		obj.push_back(_optimizer->objectives[0].row[i].value == MINIMIZE);

	for (unsigned i = 0; i < _optimizer->objectives[1].row.size(); i++)
		obj.push_back(_optimizer->objectives[1].row[i].value == MINIMIZE);

	for (unsigned i = 0; i < _optimizer->objectives[2].row.size(); i++)
		obj.push_back(_optimizer->objectives[2].row[i].value == MINIMIZE);

	for (unsigned p = 0; p < P.size(); p++)
	{
		P[p].push_back(_data[p].final_state.time);

		for (unsigned i = 0; i < _optimizer->objectives[0].row.size(); i++)
		{
			unsigned index = _optimizer->objectives[0].row[i].index;

			P[p].push_back(_data[p].final_state.resources[index].usable());
		}

		for (unsigned i = 0; i < _optimizer->objectives[1].row.size(); i++)
		{
			unsigned index = _optimizer->objectives[1].row[i].index;

			P[p].push_back(_data[p].final_state.resources[index].quantity);
		}

		for (unsigned i = 0; i < _optimizer->objectives[2].row.size(); i++)
		{
			unsigned index = _optimizer->objectives[2].row[i].index;

			P[p].push_back(_data[p].final_state.resources[index].used);
		}
	}

	//CALCULATE NUMBER OF HYPERCUBES
	unsigned num_grids = 1, expo = obj.size(), base = _gridLevels;
	while(expo)
	{
		if (expo & 1)
			num_grids *= base;
		expo >>= 1;
		base *= base;
	}

	if (!update)
	{
		//FIX BOUNDS
		if (_ub.size() == 0)
		{
			_ub.resize(obj.size());
			_lb.resize(obj.size());

			//RESET UNIQUE EXTREME VECTORS
			for (unsigned i = 0; i < _uev.size(); i++)
				_uev[i] = 0;

			//GET MINIMUM AND MAXIMUM
			std::vector<unsigned> min(obj.size()), max(obj.size());
			std::vector<unsigned> range(obj.size());

			for (unsigned i = 0; i < obj.size(); i++)
				min[i] = P[0][i];

			for (unsigned i = 0; i < obj.size(); i++)
				max[i] = P[0][i];

			for (unsigned i = 1; i < P.size()-1; i++)
				for (unsigned j = 0; j < obj.size(); j++)
					if (min[j] > P[i][j])
						min[j] = P[i][j];
			for (unsigned i = 1; i < P.size()-1; i++)
				for (unsigned j = 0; j < obj.size(); j++)
					if (max[j] < P[i][j])
						max[j] = P[i][j];

			//SET UNIQUE EXTREME VECTORS
			for (unsigned i = 0; i < P.size()-1; i++)
				for (unsigned j = 0; j < obj.size(); j++)
					if (P[i][j] == min[j] || P[i][j] == max[j])
					{
						_uev[i] = 1;
						break;
					}

			//SET RANGE
			for (unsigned i = 0; i < obj.size(); i++)
				range[i] = max[i] - min[i];

			//UPDATE BOUNDARIES
			for (unsigned i = 0; i < obj.size(); i++)
				_ub[i] = max[i] + (range[i]/(_gridLevels << 1));
			for (unsigned i = 0; i < obj.size(); i++)
				_lb[i] = min[i] - (range[i]/(_gridLevels << 1));
		}

		//DOES IT EXTEND
		bool extend = false;
		for (unsigned i = 0; i < obj.size(); i++)
		{
			if (P[P.size()-1][i] > _ub[i])
			{
				extend = true;
				break;
			}
			if (P[P.size()-1][i] < _lb[i])
			{
				extend = true;
				break;
			}
		}

		//IS IT IN A CROWDED REGION
		bool crowded = false;
		std::vector<unsigned> crowded_regions;
		std::vector<unsigned> crowded_size;
		unsigned maximum_crowd = 0;
		if (!extend)
		{
			//GET REGION INDEX
			unsigned region_index = 0;
			unsigned offset = 1;
			for (unsigned i = 0; i < obj.size(); i++)
			{
				unsigned s;

				s = P[P.size()-1][i] - _lb[i];
				s *= _gridLevels;
				s /= _ub[i] - _lb[i];

				region_index += s * offset;
				offset *= _gridLevels;
			}

			if (region_index >= num_grids)
				region_index = num_grids - 1;

			//SEPARATE OCCUPIED REGIONS
			for (unsigned i = 0; i < P.size() - 1; i++)
			{
				bool done = false;
				for (unsigned j = 0; j < crowded_regions.size(); j++)
					if (_region[i] == crowded_regions[j])
					{
						if (_uev[i] == 0)
							crowded_size[j]++;
						done = true;
						break;
					}

				if (!done)
				{
					crowded_regions.push_back(_region[i]);
					crowded_size.push_back(1);
				}
			}

			//GET CROWDED REGIONS
			for (unsigned i = 0; i < crowded_size.size(); i++)
				if (crowded_size[i] > maximum_crowd)
					maximum_crowd = crowded_size[i];

			//CHECK IF IN CROWDED REGION
			for (unsigned i = 0; i < crowded_regions.size(); i++)
				if (crowded_size[i] == maximum_crowd)
					if (region_index == crowded_regions[i])
					{
						crowded = true;
						break;
					}

		}

		if (!crowded || extend)
		{
			if (!crowded_size.size())
			{
				//SEPARATE OCCUPIED REGIONS
				for (unsigned i = 0; i < P.size() - 1; i++)
				{
					bool done = false;
					for (unsigned j = 0; j < crowded_regions.size(); j++)
						if (_region[i] == crowded_regions[j])
						{
							if (_uev[i] == 0)
								crowded_size[j]++;
							done = true;
							break;
						}

					if (!done)
					{
						crowded_regions.push_back(_region[i]);
						crowded_size.push_back(1);
					}
				}

				//GET CROWDED REGIONS
				for (unsigned i = 0; i < crowded_size.size(); i++)
					if (crowded_size[i] > maximum_crowd)
						maximum_crowd = crowded_size[i];
			}

			//SET CANDIDATES
			std::vector<unsigned> candidates;
			for (unsigned i = 0; i < crowded_regions.size(); i++)
				if (crowded_size[i] == maximum_crowd)
					for (unsigned j = 0; j < P.size()-1; j++)
						if (_region[j] == crowded_regions[i])
							if (_uev[j] == 0)
								candidates.push_back(j);

			//ELIMINATE THE UNFORTUNATE
			unsigned index = candidates[rng() % candidates.size()];
			pop(_data, index);
			pop(P, index);
		}
		else
		{
			_data.pop_back();
			return;
		}
	}

	//RESET UNIQUE EXTREME VECTORS
	for (unsigned i = 0; i < _uev.size(); i++)
		_uev[i] = 0;

	//GET MINIMUM AND MAXIMUM
	std::vector<unsigned> min(obj.size()), max(obj.size());
	std::vector<unsigned> range(obj.size());

	for (unsigned i = 0; i < obj.size(); i++)
		min[i] = P[0][i];

	for (unsigned i = 0; i < obj.size(); i++)
		max[i] = P[0][i];

	for (unsigned i = 1; i < P.size(); i++)
		for (unsigned j = 0; j < obj.size(); j++)
			if (min[j] > P[i][j])
				min[j] = P[i][j];
	for (unsigned i = 1; i < P.size(); i++)
		for (unsigned j = 0; j < obj.size(); j++)
			if (max[j] < P[i][j])
				max[j] = P[i][j];

	//SET UNIQUE EXTREME VECTORS
	for (unsigned i = 0; i < P.size(); i++)
		for (unsigned j = 0; j < obj.size(); j++)
			if (P[i][j] == min[j] || P[i][j] == max[j])
			{
				_uev[i] = 1;
				break;
			}

	//SET RANGE
	for (unsigned i = 0; i < obj.size(); i++)
		range[i] = max[i] - min[i];

	//UPDATE BOUNDARIES
	for (unsigned i = 0; i < obj.size(); i++)
		_ub[i] = max[i] + (range[i]/(_gridLevels << 1));
	for (unsigned i = 0; i < obj.size(); i++)
		_lb[i] = min[i] - (range[i]/(_gridLevels << 1));

	//UPDATE REGIONS
	for (unsigned i = 0; i < P.size(); i++)
	{
		_region[i] = 0;
		unsigned offset = 1;
		for (unsigned i = 0; i < obj.size(); i++)
		{
			unsigned s;

			s = P[P.size()-1][i] - _lb[i];
			s *= _gridLevels;
			s /= _ub[i] - _lb[i];

			_region[i] += s * offset;
			offset *= _gridLevels;
		}

		if (_region[i] >= num_grids)
			_region[i] = num_grids - 1;
	}

}