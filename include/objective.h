#ifndef OBJECTIVEH
#define OBJECTIVEH

#include "buildorder.h"

namespace BuildOrder
{

	bool maximumOverflow(unsigned task, GameState const&);
	bool hasPrerequisites(unsigned task, GameState const&);

	bool can(unsigned task, GameState const&);
	bool can(TaskPointer t, GameState const&);

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

		void afterStack(std::vector<unsigned>&, std::vector<unsigned>&, GameState const&);
		bool prerequisiteInStack(std::vector<bool>&, std::vector<unsigned>&, unsigned);
		void resourcesByEvents(std::vector<bool>&, GameState const&);
		bool possible(BuildOrder&, GameState const&);
		bool possible(unsigned, GameState const&);

		void updateCosts(BuildOrder&, GameState&);
		void updateBorrow(BuildOrder&, GameState&);

		void buildWhatYouCan(BuildOrder&, GameState&, unsigned& last, remaining_list& listTime);

		void aftermath(GameState&, unsigned type);

		void updateOngoingTasks(GameState&, remaining_list&);
		void updateOngoingEvents(GameState&, remaining_list&);

		void update(GameState&, remaining_list&, remaining_list&);
		unsigned nextTime(GameState& init, remaining_list&, remaining_list&, unsigned, unsigned);
	}

	GameState makespan(GameState, BuildOrder& original, unsigned maximum_time = 0);

}

#endif
