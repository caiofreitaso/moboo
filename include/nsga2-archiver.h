#ifndef NSGA2ARCHIVERH
#define NSGA2ARCHIVERH

#include "archivers.h"

namespace BuildOrder
{
	namespace Optimizer
	{
		class NSGA2_Archiver : public Archiver
		{
			bool dominates(std::vector<unsigned> a, std::vector<unsigned> b, std::vector<bool> min) const;

			void quicksort(std::vector<unsigned>& indexes,
						   unsigned begin, unsigned end, unsigned objective,
						   std::vector<std::vector<unsigned> >& v) const;

			void quicksort(std::vector<unsigned>& v,
						   unsigned begin, unsigned end, std::vector<double> dist) const;

		public:
			NSGA2_Archiver(unsigned c, const Optimizer* o);

			virtual void filter(Population& pop) const;

		};
	}
}

#endif