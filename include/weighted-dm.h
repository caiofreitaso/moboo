#ifndef WEIGHTED_DM_H
#define WEIGHTED_DM_H

#include "decision-maker.h"
namespace BuildOrder
{
	namespace Optimizer
	{
		struct WeightedDecision : public DecisionMaker
		{
			contiguous<double> weights() const { return target->weights; }
		};


		struct WSM : public WeightedDecision
		{
		protected:
			virtual double value(contiguous<double> v) const
			{
				double d = 0;
				for (unsigned i = 0; i < v.size(); i++)
					d += weights()[i]*v[i];
				return d;
			}
		};

		struct WPM : public WeightedDecision
		{
		protected:
			virtual double value(contiguous<double> v) const
			{
				double d = 0;
				for (unsigned i = 0; i < v.size(); i++)
					d *= pow(v[i],weights()[i]);
				return d;
			}
		};
	}
}


#endif