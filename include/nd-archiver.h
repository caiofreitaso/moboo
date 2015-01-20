#ifndef NDARCHIVERH
#define NDARCHIVERH

#include "archivers.h"

namespace BuildOrder
{
	namespace Optimizer
	{
		class NonDominated_Archiver : public Archiver
		{
		public:
			NonDominated_Archiver(unsigned c, const Optimizer* o)
			{
				_capacity = c;
				_data.reserve(c+1);
				_optimizer = o;
			}

			virtual void filter(Population& pop) const
			{
				pop = _optimizer->nonDominated(pop);
			}

		};
	}
}

#endif