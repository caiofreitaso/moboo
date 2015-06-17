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
			std::vector<unsigned> _ub;
			std::vector<unsigned> _lb;
			std::vector<unsigned> _uev;
			std::vector<unsigned> _region;

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