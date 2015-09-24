#include "../include/agarchiver.h"


contiguous<unsigned> BuildOrder::Optimizer::AdaptativeGrid_Archiver::cmin(contiguous<contiguous<unsigned> > P, unsigned sz) const
{
	contiguous<unsigned> min = P[0];

	for (unsigned i = 1; i < sz; i++)
		for (unsigned j = 0; j < obj.size(); j++)
			if (min[j] > P[i][j])
				min[j] = P[i][j];

	return min;
}

contiguous<unsigned> BuildOrder::Optimizer::AdaptativeGrid_Archiver::cmax(contiguous<contiguous<unsigned> > P, unsigned sz) const
{
	contiguous<unsigned> max = P[0];

	for (unsigned i = 1; i < sz; i++)
		for (unsigned j = 0; j < obj.size(); j++)
			if (max[j] < P[i][j])
				max[j] = P[i][j];

	return max;
}

void BuildOrder::Optimizer::AdaptativeGrid_Archiver::fixBounds(contiguous<contiguous<unsigned> > P, unsigned sz)
{
	//RESET UNIQUE EXTREME VECTORS
	for (unsigned i = 0; i < _uev.size(); i++)
		_uev[i] = 0;

	//GET MINIMUM AND MAXIMUM
	_min = this->cmin(P,sz);
	_max = this->cmax(P,sz);


	#pragma omp parallel
	{
		#pragma omp sections nowait
		{
			//SET UNIQUE EXTREME VECTORS
			#pragma omp section
			{
				for (unsigned i = 0; i < sz; i++)
				{
					_uev[i] = 0;
					for (unsigned j = 0; j < obj.size(); j++)
					{
						if (P[i][j] == _min[j])
							_uev[i]++;
						else if (P[i][j] == _max[j])
							_uev[i]++;
					}
				}

				for (unsigned i = 0; i < sz; i++)
					if (_uev[i] == 1)
					{
						bool min = false; unsigned j;
						for (j = 0; j < obj.size(); j++)
							if (P[i][j] == _min[j])
							{
								min = true;
								break;
							} else if (P[i][j] == _max[j])
								break;

						bool done = false;
						for (unsigned k = 0; k < sz; k++)
							if (_uev[k] > 1)
								if (P[k][j] == (min ? _min[j] : _max[j]))
								{
									done = true;
									break;
								}

						if (done)
							_uev[i] = 0;
					}
			}

			//SET RANGE
			#pragma omp section
			{
				contiguous<unsigned> range(obj.size());
				
				for (unsigned i = 0; i < obj.size(); i++)
					range[i] = (_max[i] - _min[i])/(_gridLevels * 2);

				//UPDATE BOUNDARIES
				if (_ub.size() == 0)
				{
					_ub.resize(obj.size());
					_lb.resize(obj.size());
				}

				for (unsigned i = 0; i < obj.size(); i++)
					_ub[i] = _max[i] + range[i];
				for (unsigned i = 0; i < obj.size(); i++)
					_lb[i] = _min[i] - range[i];
			}
		}
	}
}

unsigned BuildOrder::Optimizer::AdaptativeGrid_Archiver::getRegionIndex(contiguous<unsigned> S) const
{
	unsigned region_index = 0;
	unsigned offset = 1;
	for (unsigned i = 0; i < obj.size(); i++)
	{
		if (_ub[i] == _lb[i])
			region_index += offset;
		else
		{
			unsigned coordinate;

			coordinate = S[i] - _lb[i];
			coordinate *= _gridLevels;
			coordinate /= _ub[i] - _lb[i];

			region_index += coordinate * offset;
		}

		offset *= _gridLevels;
	}
	return region_index;
}

void BuildOrder::Optimizer::AdaptativeGrid_Archiver::insert(Solution a)
{
	for (unsigned i = 0; i < size(); i++)
		if (_data[i] == a)
			return;

	_data.push_back(a);

	//NON-DOMINATED SET
	_data = _optimizer->nonDominated(_data);
	bool update = false;
	
	if (_data.size() <= _capacity)
		update = true;
	

	//TRANSLATE INTO VECTORS
	contiguous<contiguous<unsigned> > P;
	
	P = _optimizer->toVector(_data);

	//CALCULATE NUMBER OF HYPERCUBES
	unsigned num_grids = 1, expo = obj.size(), base = _gridLevels;
	while(expo)
	{
		if (expo & 1)
			num_grids *= base;
		expo >>= 1;
		base *= base;
	}

	bool extend = false;
	bool isuev = false;
	bool crowded = false;
	if (!update)
	{
		//FIX BOUNDS
		if (P.size() > 1)
			fixBounds(P, P.size()-1);

		//DOES IT EXTEND
		for (unsigned i = 0; i < obj.size(); i++)
		{
			if (P.back()[i] > _ub[i])
			{
				extend = true;
				break;
			}
			if (P.back()[i] < _lb[i])
			{
				extend = true;
				break;
			}
		}

		//IS IT UNIQUE EXTREME
		for (unsigned i = 0; i < obj.size(); i++)
			if (P.back()[i] > _max[i] || P.back()[i] < _min[i])
			{
				isuev = true;
				break;
			}

		//GET OCCUPIED REGIONS AND THEIR SIZES
		contiguous<unsigned> crowded_regions;
		contiguous<unsigned> crowded_size;
		unsigned maximum_crowd = 0;

		crowded_regions.reserve(_region.size());
		crowded_size.reserve(_region.size());

		//SEPARATE OCCUPIED REGIONS
		for (unsigned i = 0; i < _region.size(); i++)
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

		//IS IT IN A CROWDED REGION
		if (!extend)
		{
			//GET REGION INDEX
			unsigned region_index = getRegionIndex(P.back());

			//CHECK IF IN CROWDED REGION
			for (unsigned i = 0; i < crowded_regions.size(); i++)
				if (crowded_size[i] == maximum_crowd)
					if (region_index == crowded_regions[i])
					{
						crowded = true;
						break;
					}
		}

		if (!crowded || extend || isuev)
		{
			//SET CANDIDATES
			contiguous<unsigned> candidates;
			for (unsigned i = 0; i < crowded_regions.size(); i++)
				if (crowded_size[i] == maximum_crowd)
					for (unsigned j = 0; j < _region.size(); j++)
						if (_region[j] == crowded_regions[i])
							if (_uev[j] == 0)
								candidates.push_back(j);

			//ELIMINATE THE UNFORTUNATE
			if (candidates.size())
			{
				unsigned index = candidates[rng() % candidates.size()];
				pop(_data, index);
				pop(P, index);
			}
		}
		else
		{
			_data.pop_back();
			P.pop_back();
			return;
		}
	}

	assert(P.size() <= _uev.size());

	//UPDATE BOUNDS
	if (P.size())
		fixBounds(P, P.size());

	//UPDATE REGIONS
	for (unsigned i = 0; i < P.size(); i++)
		_region[i] = getRegionIndex(P[i]);

}