#ifndef DECISIONMAKER_H
#define DECISIONMAKER_H

#include "optimizer.h"

namespace BuildOrder
{
	namespace Optimizer
	{

		struct DecisionMaker
		{
			Optimizer* target;

			unsigned choose(Population p) const { return choose(normalize(p)); }

			unsigned choose(contiguous<contiguous<double> >) const;

			Solution choose(GameState i0, unsigned iterations) const
			{
				Population front = target->optimize(i0, iterations);
				return front[choose(front)];
			}

			contiguous< contiguous<double> > normalize(Population p) const;
		protected:

			virtual double value(contiguous<double> v) const = 0;

		};


		struct OriginDM : public DecisionMaker
		{
		protected:
			virtual double value(contiguous<double> v) const
			{
				double d = 0;
				for (unsigned i = 0; i < v.size(); i++)
					d += v[i]*v[i];
				return sqrt(d);
			}
		};
	}
}
#endif