#ifndef MOGRASP_H
#define MOGRASP_H

#include "agarchiver.h"
#include "create_solution.h"

namespace BuildOrder
{
	namespace Optimizer
	{
		class MOGRASP : public Optimizer
		{
			public:
				unsigned creation_cycles;
				unsigned parents;
				unsigned childs;

				MOGRASP() :
					creation_cycles(5), parents(1),
					childs(2)
				{ }
				MOGRASP(unsigned c) :
					creation_cycles(c), parents(1),
					childs(2)
				{ }
				MOGRASP(unsigned c, unsigned p, unsigned h) :
					creation_cycles(c), parents(p),
					childs(h)
				{ }

				virtual Population optimize(GameState init, unsigned iterations) const;
		};
	}
}

#endif