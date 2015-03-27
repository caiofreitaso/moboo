#include <iostream>
#include "../include/knee.h"
#include "../include/nsga2.h"
#include "../include/mograsp.h"
#include "../include/exact.h"

BuildOrder::Optimizer::Optimizer* optm = 0;
unsigned iterations = 20;
unsigned archSize = 20;

int main(int argc, char const *argv[])
{
	if (argc < 4)
	{
		std::cout << "Usage: " << argv[0] << " system state model [options]\n\n"
				  << "  system\tFile with the system description.\n"
				  << "  state\t\tFile with the initial state description.\n"
				  << "  model\t\tFile with the problem description.\n\n"
				  << "Options:\n"
				  << "  -o <optimizer>\tChoose the optimizer:\n"
				  << "  \t\t\tnsga2, mograsp, exact, knee\n"
				  << "  -n <neighborhood>\tChoose the neighborhood:\n"
				  << "  \t\t\tdelete_one, delete_tail, one_swap, insert\n"
				  << "  -i <value>\t\tNumber of iterations\n"
				  << "  \t\t\t[1, +INF]\tdefault = 20\n"
				  << "  -aS <value>\t\tArchiver size\n"
				  << "  \t\t\t[1, +INF]\tdefault = 20\n"
				  << "  -objm <value>\t\tMinimum objective multiplier\n"
				  << "  \t\t\t[0, +INF]\tdefault = 2.0\n"
				  << "  -objM <value>\t\tMaximum objective multiplier\n"
				  << "  \t\t\t[0, +INF]\tdefault = 4.0\n"
				  << "  -resm <value>\t\tMinimum restriction multiplier\n"
				  << "  \t\t\t[0, +INF]\tdefault = 1.5\n"
				  << "  -resM <value>\t\tMaximum restriction multiplier\n"
				  << "  \t\t\t[0, +INF]\tdefault = 3.5\n"
				  << "  -dOm <value>\t\tMinimum objective increase rate\n"
				  << "  \t\t\t[1, +INF]\tdefault = 1\n"
				  << "  -dOM <value>\t\tMaximum objective increase rate\n"
				  << "  \t\t\t(1, +INF]\tdefault = 1.2\n"
				  << "  -dRm <value>\t\tMinimum restriction increase rate\n"
				  << "  \t\t\t[1, +INF]\tdefault = 1.2\n"
				  << "  -dRM <value>\t\tMaximum restriction increase rate\n"
				  << "  \t\t\t(1, +INF]\tdefault = 1.4\n"
  				  << "  -Fobjm <value>\tMinimum objective multiplier [FIX]\n"
				  << "  \t\t\t[0, +INF]\tdefault = 1.5\n"
				  << "  -FobjM <value>\tMaximum objective multiplier [FIX]\n"
				  << "  \t\t\t[0, +INF]\tdefault = 2.5\n"
				  << "  -Fresm <value>\tMinimum restriction multiplier [FIX]\n"
				  << "  \t\t\t[0, +INF]\tdefault = 2.6\n"
				  << "  -FresM <value>\tMaximum restriction multiplier [FIX]\n"
				  << "  \t\t\t[0, +INF]\tdefault = 3.5\n"
				  << "  -FdOm <value>\t\tMinimum objective increase rate [FIX]\n"
				  << "  \t\t\t[1, +INF]\tdefault = 1.1\n"
				  << "  -FdOM <value>\t\tMaximum objective increase rate [FIX]\n"
				  << "  \t\t\t(1, +INF]\tdefault = 1.3\n"
				  << "  -FdRm <value>\t\tMinimum restriction increase rate [FIX]\n"
				  << "  \t\t\t[1, +INF]\tdefault = 1.4\n"
				  << "  -FdRM <value>\t\tMaximum restriction increase rate [FIX]\n"
				  << "  \t\t\t(1, +INF]\tdefault = 2.0\n";
		return 0;
	}

	BuildOrder::GameState state;

	BuildOrder::Rules::init(argv[1]);
	
	BuildOrder::createState(state);
	BuildOrder::initState(state, argv[2]);

	std::cout << "TIME: " << state.time << "\n";
	std::cout << "MINERALS: " << state.resources[0].usable() << "\n";
	std::cout << "GAS: " << state.resources[1].usable() << "\n";
	std::cout << "LARVAE: " << state.resources[3].usable() << "\n";
	std::cout << "SUPPLY: " << state.quantity(2) << "\n";
	std::cout << "SUPPLY USED: " << state.resources[2].used << "\n";
	
	for (unsigned i = 0; i < state.resources.size(); i++)
		if (state.resources[i].usable())
			std::cout << i << ": " << state.resources[i].usable() << "\n";

	std::cout << "----------------------------\n";
	BuildOrder::Rules::prerequisites.print();
	std::cout << "----------------------------\n";
	BuildOrder::Rules::borrows.print();
	std::cout << "----------------------------\n";
	BuildOrder::Rules::costs.print();
	std::cout << "----------------------------\n";
	BuildOrder::Rules::consumes.print();
	std::cout << "----------------------------\n";
	BuildOrder::Rules::maxima.print();
	std::cout << "----------------------------\n";

	std::vector<BuildOrder::Optimizer::Population> total;
	BuildOrder::Optimizer::Population ret, front;
	#ifdef OPT_NSGA2
	BuildOrder::Optimizer::NSGA2 solver(50,50);
	#else
	BuildOrder::Optimizer::MOGRASP solver(50, 1, 1);
	//BuildOrder::Optimizer::ExactOpt solver(1000);
	#endif

	#ifdef ONESWAP
	solver.neighborhood = BuildOrder::Optimizer::one_swap;
	#else
	solver.neighborhood = BuildOrder::Optimizer::one_swap;//swap_and_delete;
	#endif
	
	solver.stop_chance = 0.3;
	BuildOrder::Optimizer::initOptimizer(solver,argv[3]);
	std::cout << solver.print();
	/*solver.maximum_time = 300;
	solver.objectives[0].set(0, BuildOrder::Optimizer::MAXIMIZE);
	solver.objectives[0].set(7, BuildOrder::Optimizer::MAXIMIZE);
	solver.restrictions[0].set(7, BuildOrder::Optimizer::Restriction(0,1));*/
//	solver.objectives[0].set(6, BuildOrder::Optimizer::MAXIMIZE);
//	solver.restrictions[0].set(6, BuildOrder::Optimizer::Restriction(0,1));
	solver.update();

	std::chrono::time_point<std::chrono::system_clock> a;

	for (unsigned i = 0; i < 30; i++)
	{
		a = std::chrono::system_clock::now();
		total.push_back(solver.optimize(state, 1));

		std::chrono::duration<double> tt = std::chrono::system_clock::now() - a;
		std::cout << "TIME = " << tt.count() << "\n";

		for (unsigned k = 0; k < total.back().size(); k++)
			std::cerr << solver.print(total.back()[k]) << "\n";

		std::cerr << "\n";

		ret.insert(ret.begin(), total.back().begin(),total.back().end());
	}

	front = solver.nonDominated(ret);

	for (unsigned i = 0; i < front.size(); i++)
	{
		std::cout << "\n" << (i+1) << "\n";
		std::cout << "BUILD ORDER: ";
		print(front[i].orders);

		std::cout << "Time: " << front[i].final_state.time << "\n";
		std::cout << "Minerals: " << front[i].final_state.resources[0].usable() << "\n";
		std::cout << "Zerglings: " << front[i].final_state.resources[7].usable() << "\n";

		std::cerr << solver.print(front[i]) << "\n";
		//std::cerr << front[i].final_state.time << " -" << front[i].final_state.resources[0].usable() << "\n";
	}

	return 0;
}