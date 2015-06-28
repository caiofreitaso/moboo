#ifndef AGARCHIVERH
#define AGARCHIVERH

#include "archivers.h"

namespace BuildOrder
{
	namespace Optimizer
	{
		class AdaptativeGrid_Archiver : public Archiver
		{
			unsigned _gridLevels;
			contiguous<unsigned> _ub;
			contiguous<unsigned> _lb;
			contiguous<unsigned> _uev;
			contiguous<unsigned> _region;

		public:
			AdaptativeGrid_Archiver(unsigned c, const Optimizer* o, unsigned g = 5)
			: _gridLevels(g), _uev(c), _region(c)
			{
				_capacity = c;
				_data.reserve(c+1);
				_optimizer = o;
			}

			virtual void filter(Population& pop) const
			{
			}
			virtual void insert(Solution a);
		};
	}
}

#endif