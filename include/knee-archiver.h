#ifndef KNEEARCHIVERH
#define KNEEARCHIVERH

#include "nsga2-archiver.h"

namespace BuildOrder
{
	namespace Optimizer
	{
		class Knee_Archiver : public NSGA2_Archiver
		{
			static std::vector<std::vector<double> > _values;
			static unsigned _v_size;
			static unsigned _v_obj;
		public:
			Knee_Archiver(unsigned c, const Optimizer* o);

			static void knees		(std::vector<std::vector<unsigned> >&,
									 std::vector<std::vector<unsigned> >&,
									 std::vector<bool> const&);
		};
	}
}

#endif