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
				unsigned archive_size;
				unsigned creation_cycles;
				unsigned parents;
				unsigned childs;

				MOGRASP() :
					archive_size(50),creation_cycles(5), parents(1),
					childs(2)
				{ }
				MOGRASP(unsigned c) :
					archive_size(c*10),creation_cycles(c), parents(1),
					childs(2)
				{ }
				MOGRASP(unsigned c, unsigned p, unsigned h) :
					archive_size(c*10),creation_cycles(c), parents(p),
					childs(h)
				{ }

				virtual Population optimize(GameState init, unsigned iterations) const;
		};
	}
}

#endif