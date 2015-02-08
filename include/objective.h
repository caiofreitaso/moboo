#ifndef OBJECTIVEH
#define OBJECTIVEH

#include "buildorder.h"

namespace BuildOrder
{

	bool maximumOverflow(unsigned task, GameState s);
	bool hasPrerequisites(unsigned task, GameState s);

	bool can(unsigned task, GameState s);
	bool can(TaskPointer t, GameState s);

	namespace Objective
	{
		struct time_helper {
			unsigned pointer;
			unsigned time_remaining;

			time_helper(unsigned p, unsigned t)
			: pointer(p), time_remaining(t)
			{ }
		};

		typedef std::vector<time_helper> remaining_list;

		void afterStack(std::vector<unsigned>&, GameState&);
		bool prerequisiteInStack(std::vector<bool>&, std::vector<unsigned>&, unsigned, GameState&);
		void resourcesByEvents(std::vector<bool>&, GameState&);
		bool possible(BuildOrder&, GameState&);

		void updateCosts(BuildOrder&, GameState&);
		void updateBorrow(BuildOrder&, GameState&);

		void buildWhatYouCan(BuildOrder&, GameState&, unsigned& last, remaining_list& listTime);

		void aftermath(GameState&, unsigned type);

		void updateOngoingTasks(GameState&, remaining_list&);
		void updateOngoingEvents(GameState&, remaining_list&);

		unsigned update(GameState&, remaining_list& listTime, unsigned maximum_time);
	}

	GameState makespan(GameState, BuildOrder& original, unsigned maximum_time = 0);

}

#endif
