#ifndef OPTIMIZERH
#define OPTIMIZERH

#include <algorithm>
#include "neighborhoods.h"

namespace BuildOrder
{
	namespace Optimizer
	{
		enum Objective
		{
			NONE,
			MAXIMIZE,
			MINIMIZE
		};

		enum Target
		{
			USABLE,
			QUANTITY,
			USED
		};

		struct Restriction
		{
			unsigned less_than;
			unsigned greater_than;

			constexpr Restriction()
			: less_than(0), greater_than(0)
			{ }


			constexpr Restriction(unsigned lt, unsigned gt)
			: less_than(lt), greater_than(gt)
			{ }

			constexpr operator bool() const
			{ return less_than > 0 || greater_than > 0; }

			constexpr bool operator==(Restriction r) const
			{ return less_than == r.less_than && greater_than == r. greater_than; }
		};


		class Optimizer
		{
			std::vector<double> _objV;
			std::vector<MatrixRow<double> > _incV;
			std::vector<MatrixRow<double> > _decV;

		public:
			bool time_as_objective;

			unsigned maximum_time;
			MatrixRow<Restriction> restrictions[3];
			MatrixRow<Objective> objectives[3];

			Population (*neighborhood)(Solution const&);

			Optimizer()
			:	time_as_objective(true),
				maximum_time(0),
				neighborhood(delete_tail)
			{ update(); }

			virtual Population optimize(GameState, unsigned) const = 0;

			bool dominates(Solution a, Solution b) const;

			bool valid(Solution s) const;

			unsigned producesMaximize(unsigned task) const;
			unsigned producesGreaterThan(unsigned task) const;

			Population nonDominated(Population p) const;

			unsigned numberObjectives() const;
			unsigned numberRestrictions() const;

			void update();
			std::vector<double> initialMap(double,double,GameState) const;
		};
	}
}

#endif