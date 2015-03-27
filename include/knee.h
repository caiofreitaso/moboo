#ifndef KNEEH
#define KNEEH

#include "knee-archiver.h"
#include "nsga2.h"

namespace BuildOrder
{
	namespace Optimizer
	{
		class Knee : public NSGA2
		{
			public:
				Knee()
				: NSGA2()
				{ }

				Knee(unsigned pop)
				: NSGA2(pop)
				{ }

				Knee(unsigned pop, unsigned cc)
				: NSGA2(pop,cc)
				{ }

				virtual Population optimize(GameState initial, unsigned iterations) const;
		};
	}
}

#endif