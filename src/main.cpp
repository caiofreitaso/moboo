#include <iostream>
#include "../include/knee.h"
#include "../include/nsga2.h"
#include "../include/mograsp.h"
#include "../include/ants.h"

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
	BuildOrder::Optimizer::Solution test;
	contiguous<BuildOrder::Optimizer::Population> total;
	BuildOrder::Optimizer::Population ret, front;

	std::chrono::time_point<std::chrono::system_clock> a;

	/////////////////////////////////////////////////////
	a = std::chrono::system_clock::now();
	BuildOrder::Rules::init(argv[1]);
	BuildOrder::Rules::initGraph();
	
	BuildOrder::createState(state);
	BuildOrder::initState(state, argv[2]);
	std::cout << "INITIAL STATE:\n";
	state.print();
	std::cout << "--------------\n\n";
	/////////////////////////////////////////////////////

	#ifdef OPT_NSGA2
	BuildOrder::Optimizer::NSGA2 solver(50,50);
	#else
	//BuildOrder::Optimizer::Ants solver(8, 5, 1, 1, 1, 0.3);
	//BuildOrder::Optimizer::NSGA2 solver(8,5);
	BuildOrder::Optimizer::MOGRASP solver(8, 5, 1, 1);
	//BuildOrder::Optimizer::ExactOpt solver(1000);
	#endif

	//solver.neighborhood = BuildOrder::Optimizer::one_swap;
	//solver.neighborhood = BuildOrder::Optimizer::swap_and_delete;
	
	solver.stop_chance = 0.3;
	BuildOrder::Optimizer::initOptimizer(solver,argv[3]);
	std::cout << solver.print();
	solver.update();

	std::chrono::duration<double> tt = std::chrono::system_clock::now() - a;
	std::cout << "\n--------------\nPREPROCESSING TIME = " << tt.count() << "\n--------------\n";

	test.orders.push_back(14);test.orders.push_back(3);
	test.update(state,solver.maximum_time);
	BuildOrder::print(test.orders);

	/*a = std::chrono::system_clock::now();
	front.push_back(BuildOrder::Optimizer::create_exact(state,solver));
	tt = std::chrono::system_clock::now() - a;
	std::cout << "TIME = " << tt.count() << "\n";
	print(front[0].orders);
	a = std::chrono::system_clock::now();
	for (unsigned i = 0; i < 100; i++)
		ret.push_back(BuildOrder::Optimizer::create(state,solver,1));
	tt = std::chrono::system_clock::now() - a;
	std::cout << "TIME = " << tt.count() << "\n";
	

	front = solver.nonDominated(front);	
	ret = solver.nonDominated(ret);

	print(front[0].orders);
	std::cout << "Time: " << front[0].final_state.time << "\n";

	print(ret[0].orders);
	std::cout << "Time: " << ret[0].final_state.time << "\n";

	*/for (unsigned i = 0; i < 300; i++)
	{
		a = std::chrono::system_clock::now();
		auto lastfront = solver.optimize(state, 5);
		total.push_back(lastfront);

		tt = std::chrono::system_clock::now() - a;
		double duration = tt.count();
		std::cout << "TIME = " << duration << "\n";

		for (unsigned k = 0; k < lastfront.size(); k++)
			std::cerr << solver.print(lastfront[k]) << "\n";

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