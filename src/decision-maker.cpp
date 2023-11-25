#include "../include/decision-maker.h"

unsigned BuildOrder::Optimizer::DecisionMaker::choose(contiguous<contiguous<double> > vec) const
{
	unsigned k = 0;
	double dist = value(vec[0]);
	for (unsigned i = 1; i < vec.size(); i++)
	{
		double d_i = value(vec[i]);
		
		if (d_i < dist)
		{
			dist = d_i;
			k = i;
		}
	}

	return k;
}

contiguous< contiguous<double> > BuildOrder::Optimizer::DecisionMaker::normalize(Population p) const
{
	contiguous<contiguous<double> > vec = target->toDVector(p);
	contiguous<bool> obj = target->objectivesVector();

	for (unsigned j = 0; j < obj.size(); j++)
		if (obj[j])
			for (unsigned i = 0; i < vec.size(); i++)
				vec[i][j] = -vec[i][j];


	contiguous<double> min(vec[0]);
	contiguous<double> max(vec[0]);

	for (unsigned i = 1; i < vec.size(); i++)
		for (unsigned j = 0; j < obj.size(); j++)
			if (vec[i][j] < min[j])
				min[j] = vec[i][j];
	for (unsigned i = 1; i < vec.size(); i++)
		for (unsigned j = 0; j < obj.size(); j++)
			if (vec[i][j] > max[j])
				max[j] = vec[i][j];

	for (unsigned i = 0; i < vec.size(); i++)
		for (unsigned j = 0; j < obj.size(); j++)
			vec[i][j] = (vec[i][j] - min[j])/(max[j] - min[j]);

	return vec;
}