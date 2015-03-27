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
			static bool dominates(std::vector<unsigned> a, std::vector<unsigned> b,
							std::vector<bool> min);

			static void quicksort(std::vector<unsigned>& indexes,
							unsigned begin, unsigned end, unsigned objective,
							std::vector<std::vector<unsigned> >& v);

			static void quicksort(std::vector<unsigned>& v,
							unsigned begin, unsigned end, std::vector<double>& d);

		public:
			void (*dist_func)	(std::vector<std::vector<unsigned> >&,
								 std::vector<std::vector<unsigned> >&,
								 std::vector<bool> const&);

			NSGA2_Archiver(unsigned c, const Optimizer* o);

			virtual void filter(Population& pop) const;

			static void crowding	(std::vector<std::vector<unsigned> >&,
									 std::vector<std::vector<unsigned> >&,
									 std::vector<bool> const&);
		};
	}
}

#endif