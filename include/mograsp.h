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
				double parents;
				unsigned childs;

				MOGRASP() :
					archive_size(50),creation_cycles(5), parents(1),
					childs(2)
				{ }
				MOGRASP(unsigned c) :
					archive_size(c*10),creation_cycles(c), parents(1),
					childs(2)
				{ }
				MOGRASP(unsigned c, double p, unsigned h) :
					archive_size(c*10),creation_cycles(c), parents(p),
					childs(h)
				{ }
				MOGRASP(unsigned a, unsigned c, double p, unsigned h) :
					archive_size(a),creation_cycles(c), parents(p),
					childs(h)
				{ }

				virtual Population optimize(GameState init, unsigned iterations) const;
		};
	}
}

#endif