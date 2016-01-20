#ifndef AGARCHIVERH
#define AGARCHIVERH

#include "archivers.h"
#include <iostream>
#include <cstdlib>
#include <cassert>

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
			contiguous<bool> obj;
			contiguous<unsigned> _min;
			contiguous<unsigned> _max;

			contiguous<unsigned> cmin(contiguous<contiguous<unsigned> >, unsigned) const;
			contiguous<unsigned> cmax(contiguous<contiguous<unsigned> >, unsigned) const;

			void fixBounds(contiguous<contiguous<unsigned> >, unsigned);
			unsigned getRegionIndex(contiguous<unsigned>) const;

		public:
			AdaptativeGrid_Archiver(unsigned c, const Optimizer* o, unsigned g = 5)
			: _gridLevels(g), _uev(c,0), _region(c,0)
			{
				_capacity = c;
				_data.reserve(c+1);
				_optimizer = o;

				obj.reserve(o->numberObjectives());


				obj.push_back(true);
				for (unsigned i = 0; i < _optimizer->objectives[0].row.size(); i++)
					obj.push_back(_optimizer->objectives[0].row[i].value == MINIMIZE);

				for (unsigned i = 0; i < _optimizer->objectives[1].row.size(); i++)
					obj.push_back(_optimizer->objectives[1].row[i].value == MINIMIZE);

				for (unsigned i = 0; i < _optimizer->objectives[2].row.size(); i++)
					obj.push_back(_optimizer->objectives[2].row[i].value == MINIMIZE);
			}

			virtual void filter(Population& pop) const
			{
			}
			virtual void insert(Solution a);
		};
	}
}

#endif