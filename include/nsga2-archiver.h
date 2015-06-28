#ifndef NSGA2ARCHIVERH
#define NSGA2ARCHIVERH

#include "archivers.h"

namespace BuildOrder
{
	namespace Optimizer
	{
		class NSGA2_Archiver : public Archiver
		{
		protected:
			static bool dominates(contiguous<unsigned> a, contiguous<unsigned> b,
							contiguous<bool> min);

			static void quicksort(contiguous<unsigned>& indexes,
							unsigned begin, unsigned end, unsigned objective,
							contiguous<contiguous<unsigned> >& v);

			static void quicksort(contiguous<unsigned>& v,
							unsigned begin, unsigned end, contiguous<double>& d);

		public:
			void (*dist_func)	(contiguous<contiguous<unsigned> >&,
								 contiguous<contiguous<unsigned> >&,
								 contiguous<bool> const&);

			NSGA2_Archiver(unsigned c, const Optimizer* o);

			virtual void filter(Population& pop) const;

			static void crowding	(contiguous<contiguous<unsigned> >&,
									 contiguous<contiguous<unsigned> >&,
									 contiguous<bool> const&);
		};
	}
}

#endif