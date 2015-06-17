#include "../include/knee-archiver.h"

std::vector<std::vector<double> > BuildOrder::Optimizer::Knee_Archiver::_values;
unsigned BuildOrder::Optimizer::Knee_Archiver::_v_size = 0;
unsigned BuildOrder::Optimizer::Knee_Archiver::_v_obj = 0;

BuildOrder::Optimizer::Knee_Archiver::Knee_Archiver(unsigned c, const Optimizer* o)
: NSGA2_Archiver(c,o)
{
	dist_func = Knee_Archiver::knees;
}

void BuildOrder::Optimizer::Knee_Archiver::knees (
	std::vector<std::vector<unsigned> >& F,
	std::vector<std::vector<unsigned> >& P,
	std::vector<bool> const& min)
{
	if (_v_size != P.size() || _v_obj != min.size())
	{
		_v_size = P.size();
		_v_obj = min.size();

		unsigned denom = _v_size;
		if (_v_obj > 1)
			denom /= _v_obj-1;

		double delta = 1;
		if (_v_obj > 1)
			delta /= denom;
		
		double v;
		_values.clear();
		_values.push_back(std::vector<double>(_v_obj, 0));
		_values[0][0] = 1;

		for (unsigned it = 0, pos = 0; it < _values.size(); it++)
		{
			v = _values[it][pos] - delta;

			if (v < 0)
			{
				v = 0;
				pos++;
			}

			for (unsigned k = pos+1; k < _v_obj; k++)
			{
				std::vector<double> n_value(_values[it]);
				n_value[pos] = v;
				n_value[k] += delta;

				bool unique = true;
				for (unsigned j = 0; j < _values.size(); j++)
				{
					unsigned count = 0;
					for (unsigned l = 0; l < _v_obj; l++)
						if (_values[j][l] == n_value[l])
							count++;

					if (count == _v_obj)
					{
						unique = false;
						break;
					}
				}

				if (unique)
					_values.push_back(n_value);
			}

		}
	}
	

	for (unsigned I = 0; I < F.size(); I++)
		if (F[I].size() > 2)
		{
			std::vector<double> distance(F[I].size(), 0);

			for (unsigned i = 0; i < F[I].size(); i++)
			{
				double value = 0;
				for (unsigned v = 0; v < _values.size(); v++)
					for (unsigned m = 0; m < min.size(); m++)
						value +=_values[v][m];

				distance[i] = value / _values.size();
			}

			quicksort(F[I], 0u, F[I].size()-1, distance);
		}
}