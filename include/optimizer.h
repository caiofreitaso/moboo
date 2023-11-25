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
			: less_than(-1), greater_than(0)
			{ }


			constexpr Restriction(unsigned lt, unsigned gt)
			: less_than(lt), greater_than(gt)
			{ }

			constexpr operator bool() const
			{ return less_than > 0 || greater_than < -1; }

			constexpr bool operator==(Restriction r) const
			{ return less_than == r.less_than && greater_than == r. greater_than; }

			constexpr bool operator!=(Restriction r) const
			{ return !(*this == r); }
		};


		class Optimizer
		{
			contiguous<double> _objV;
			contiguous<MatrixRow<double> > _incV;
			contiguous<MatrixRow<double> > _decV;

		public:
			bool time_as_objective;
			bool use_weights;
			double stop_chance;

			unsigned maximum_time;
			MatrixRow<Restriction> restrictions[3];
			MatrixRow<Objective> objectives[3];
			contiguous<double> weights;

			Population (*neighborhood)(Solution const&);

			Optimizer()
			:	time_as_objective(true),
				stop_chance(1),
				maximum_time(0),
				neighborhood(one_swap)
			{ }

			virtual Population optimize(GameState, unsigned) const = 0;

			bool dominates(Solution a, Solution b) const;

			bool valid(Solution s) const;

			unsigned producesMaximize(unsigned task) const;
			unsigned producesGreaterThan(unsigned task) const;

			Population nonDominated(Population p) const;

			unsigned numberObjectives() const;
			unsigned numberRestrictions() const;

			std::string print() const
			{
				std::stringstream ret;
				for (unsigned i = 0; i < objectives[0].row.size(); i++)
				{
					if (objectives[0].row[i].value == MINIMIZE)
						ret << "m";
					else
						ret << "M";

					ret << " U" << objectives[0].row[i].index << "\n";
				}
				for (unsigned i = 0; i < objectives[1].row.size(); i++)
				{
					if (objectives[1].row[i].value == MINIMIZE)
						ret << "m";
					else
						ret << "M";

					ret << " q" << objectives[1].row[i].index << "\n";
				}
				for (unsigned i = 0; i < objectives[2].row.size(); i++)
				{
					if (objectives[2].row[i].value == MINIMIZE)
						ret << "m";
					else
						ret << "M";

					ret << " u" << objectives[2].row[i].index << "\n";
				}

				ret << "\n";

				ret << "t < " << maximum_time << "\n";
				for (unsigned i = 0; i < restrictions[0].row.size(); i++)
				{
					if (restrictions[0].row[i].value.less_than != -1)
					{
						ret << "U" << restrictions[0].row[i].index << " < ";
						ret << restrictions[0].row[i].value.less_than << "\n";
					}
					if (restrictions[0].row[i].value.greater_than)
					{
						ret << "U" << restrictions[0].row[i].index << " > ";
						ret << restrictions[0].row[i].value.greater_than << "\n";
					}
				}
				for (unsigned i = 0; i < restrictions[1].row.size(); i++)
				{
					if (restrictions[1].row[i].value.less_than != -1)
					{
						ret << "q" << restrictions[1].row[i].index << " < ";
						ret << restrictions[1].row[i].value.less_than << "\n";
					}
					if (restrictions[1].row[i].value.greater_than)
					{
						ret << "q" << restrictions[1].row[i].index << " > ";
						ret << restrictions[1].row[i].value.greater_than << "\n";
					}
				}
				for (unsigned i = 0; i < restrictions[2].row.size(); i++)
				{
					if (restrictions[2].row[i].value.less_than != -1)
					{
						ret << "u" << restrictions[2].row[i].index << " < ";
						ret << restrictions[2].row[i].value.less_than << "\n";
					}
					if (restrictions[2].row[i].value.greater_than)
					{
						ret << "u" << restrictions[2].row[i].index << " > ";
						ret << restrictions[2].row[i].value.greater_than << "\n";
					}
				}
				return ret.str();
			}

			std::string print(Solution const& s) const
			{
				std::stringstream ret;
				ret << s.final_state.time << " ";
				for (unsigned j = 0; j < objectives[0].row.size(); j++)
				{
					unsigned index = objectives[0].row[j].index;
					if (objectives[0].row[j].value == MAXIMIZE)
						ret << "-";

					ret << s.final_state.resources[index].usable() << " ";
				}

				for (unsigned j = 0; j < objectives[1].row.size(); j++)
				{
					unsigned index = objectives[1].row[j].index;
					if (objectives[1].row[j].value == MAXIMIZE)
						ret << "-";

					ret << s.final_state.resources[index].quantity << " ";
				}

				for (unsigned j = 0; j < objectives[2].row.size(); j++)
				{
					unsigned index = objectives[2].row[j].index;
					if (objectives[2].row[j].value == MAXIMIZE)
						ret << "-";

					ret << s.final_state.resources[index].used << " ";
				}

				return ret.str();
			}

			void update();
			contiguous<double> initialMap(double,double,GameState) const;
			contiguous<unsigned> toVector(Solution) const;
			contiguous<contiguous<unsigned> > toVector(Population) const;
			contiguous<double> toDVector(Solution) const;
			contiguous<contiguous<double> > toDVector(Population) const;

			contiguous<bool> objectivesVector() const;
		};

		void initOptimizer(Optimizer&, char const*);

		
	}
}

#endif