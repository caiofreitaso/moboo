#include <iostream>
#include "../include/nsga2.h"
#include "../include/mograsp.h"
#include "../include/exact.h"

int main(int argc, char const *argv[])
{
	if (argc < 3)
	{
		std::cout << "Usage: " << argv[0] << " [system] [state]\n\n";
		std::cout << "\tsystem\tFile with the system description.\n";
		std::cout << "\tstate\tFile with the initial state description.\n\n";
		return 0;
	}

	BuildOrder::Rules::init(argv[1]);


	BuildOrder::GameState state;

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

	/*BuildOrder::Optimizer::Solution test;
	for (unsigned i = 0; i < 3; i++)
		test.orders.push_back(0);
	test.orders.push_back(1);
	test.orders.push_back(1);
	for (unsigned i = 0; i < 11; i++)
		test.orders.push_back(0);
	test.orders.push_back(3);
	test.orders.push_back(0);
	test.orders.push_back(0);

	test.update(state, 420);
	print(test.orders);

	std::cout << "Time: " << test.final_state.time << "\n";
	std::cout << "Minerals: " << test.final_state.resources[0].usable() << "\n";
	std::cout << "Zerglings: " << test.final_state.resources[6].usable() << "\n";*/


	std::vector<BuildOrder::Optimizer::Population> total;
	BuildOrder::Optimizer::Population ret, front;
	#ifdef OPT_NSGA2
	BuildOrder::Optimizer::NSGA2 solver(5,100);
	#else
	BuildOrder::Optimizer::MOGRASP solver(100, 1, 1);
	BuildOrder::Optimizer::MOGRASP bsolver(100, 1, 1);
	//BuildOrder::Optimizer::ExactOpt solver(1000);
	//BuildOrder::Optimizer::ExactOpt bsolver(1000);
	#endif

	#ifdef ONESWAP
	solver.neighborhood = BuildOrder::Optimizer::one_swap;
	#else
	solver.neighborhood = BuildOrder::Optimizer::swap_and_insert;
	#endif
	
	solver.maximum_time = 0;
	solver.objectives[0].set(0, BuildOrder::Optimizer::MAXIMIZE);
//	solver.objectives[0].set(7, BuildOrder::Optimizer::MAXIMIZE);
//	solver.objectives[0].set(6, BuildOrder::Optimizer::MAXIMIZE);
	solver.restrictions[0].set(6, BuildOrder::Optimizer::Restriction(0,3));
	
	bsolver.neighborhood = BuildOrder::Optimizer::swap_and_insert;
	bsolver.time_as_objective = false;
	bsolver.maximum_time = 420;
	bsolver.objectives[0].set(0, BuildOrder::Optimizer::MAXIMIZE);
	bsolver.restrictions[0].set(6, BuildOrder::Optimizer::Restriction(0,3));

	front = bsolver.optimize(state, 10000);
	for (unsigned i = 0; i < front.size(); i++)
	{
		std::cout << "\n" << (i+1) << "\n";
		std::cout << "BUILD ORDER: ";
		print(front[i].orders);

		std::cout << "Time: " << front[i].final_state.time << "\n";
		std::cout << "Minerals: " << front[i].final_state.resources[0].usable() << "\n";
		std::cout << "Zerglings: " << front[i].final_state.resources[6].usable() << "\n";

		std::cerr << front[i].final_state.time << " -" << front[i].final_state.resources[0].usable() << "\n";
	}

	front.clear();
	std::chrono::time_point<std::chrono::system_clock> a;

	for (unsigned i = 0; i < 1; i++)
	{
		a = std::chrono::system_clock::now();
		#ifdef OPT_NSGA2
		total.push_back(solver.optimize(state, 10000));
		#else
		total.push_back(solver.optimize(state, 10000));
		#endif

		std::chrono::duration<double> tt = std::chrono::system_clock::now() - a;
		std::cout << "TIME = " << tt.count() << "\n";

		for (unsigned k = 0; k < total.back().size(); k++)
			std::cerr << total.back()[k].final_state.time << " -" << total.back()[k].final_state.resources[0].usable() << "\n";

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
		std::cout << "Zerglings: " << front[i].final_state.resources[6].usable() << "\n";

		std::cerr << front[i].final_state.time << " -" << front[i].final_state.resources[0].usable() << "\n";
	}

	return 0;
}