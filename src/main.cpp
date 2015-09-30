#include <iostream>
#include "../include/knee.h"
#include "../include/nsga2.h"
#include "../include/mograsp.h"
#include "../include/ants.h"

std::chrono::time_point<std::chrono::system_clock> a;
std::chrono::duration<double> tt;

BuildOrder::GameState state;
BuildOrder::Optimizer::Optimizer* optm = 0;
unsigned opt_type = 1;
unsigned neighborhood = 2;
unsigned iterations = 20;
unsigned archSize = 20;
double stop_chance = 0.3;

unsigned aco_ants = 5;
double aco_alpha = 0.5;
double aco_beta = 0.5;
double aco_evrate = 0.5;

unsigned creation_cycles = 5;
unsigned children = 1;

double mograsp_parents = 1;



void print_help(char const* program_name)
{
	std::cout << "Usage: " << program_name << " system state model [options]\n\n"
			  << "  system\tFile with the system description.\n"
			  << "  state\t\tFile with the initial state description.\n"
			  << "  model\t\tFile with the problem description.\n\n"
			  << "Options:\n"
			  << "  -o <optimizer>\tChoose the optimizer:\n"
			  << "  \t\t\tnsga2, mograsp, aco, knee\n"
			  << "  -n <neighborhood>\tChoose the neighborhood:\n"
			  << "  \t\t\tdelete_one, delete_tail, one_swap, insert\n"
			  << "  -i <value>\t\tNumber of iterations\n"
			  << "  \t\t\t[1, +INF]\tdefault = 20\n"
			  << "  -s <value>\t\tArchiver size\n"
			  << "  \t\t\t[1, +INF]\tdefault = 20\n"
			  << "  -c <value>\t\tStop chance\n"
			  << "  \t\t\t(0, 1]\tdefault = 0.3\n\n"
			  
			  << "ANT COLONY OPTIMIZER Options\n"
			  << "  -an <value>\t\tNumber of ants\n"
			  << "  \t\t\t[1, +INF]\tdefault = 5\n"
			  << "  -aa <value>\t\tAlpha (heuristics)\n"
			  << "  \t\t\t[0, +INF]\tdefault = 0.5\n"
			  << "  -ab <value>\t\tBeta (pheromones)\n"
			  << "  \t\t\t[0, +INF]\tdefault = 0.5\n"
			  << "  -ae <value>\t\tEvaporation rate\n"
			  << "  \t\t\t[0, 1]\tdefault = 0.4\n"
			  << "  -ac <value>\t\tNumber of children\n"
			  << "  \t\t\t[1, +INF]\tdefault = 1\n\n"
			  
			  << "NSGA-II & \"Knee\" OPTIMIZERS Options\n"
			  << "  -nc <value>\t\tNumber of creation cycles\n"
			  << "  \t\t\t[1, +INF]\tdefault = 5\n\n"

			  << "MO-GRASP OPTIMIZER Options\n"
			  << "  -mc <value>\t\tNumber of creation cycles\n"
			  << "  \t\t\t[1, +INF]\tdefault = 5\n\n"
			  << "  -mp <value>\t\tProportion of parents\n"
			  << "  \t\t\t(0, 1]\tdefault = 1\n\n"
			  << "  -mh <value>\t\tNumber of children\n"
			  << "  \t\t\t[1, +INF]\tdefault = 1\n\n"

			  << "CREATION ALGORITHM Options\n"
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
			  << "  \t\t\t(1, +INF]\tdefault = 1.4\n\n"

			  << "FIX ALGORITHM Options\n"
			  << "  -Fobjm <value>\tMinimum objective multiplier\n"
			  << "  \t\t\t[0, +INF]\tdefault = 1.5\n"
			  << "  -FobjM <value>\tMaximum objective multiplier\n"
			  << "  \t\t\t[0, +INF]\tdefault = 2.5\n"
			  << "  -Fresm <value>\tMinimum restriction multiplier\n"
			  << "  \t\t\t[0, +INF]\tdefault = 2.6\n"
			  << "  -FresM <value>\tMaximum restriction multiplier\n"
			  << "  \t\t\t[0, +INF]\tdefault = 3.5\n"
			  << "  -FdOm <value>\t\tMinimum objective increase rate\n"
			  << "  \t\t\t[1, +INF]\tdefault = 1.1\n"
			  << "  -FdOM <value>\t\tMaximum objective increase rate\n"
			  << "  \t\t\t(1, +INF]\tdefault = 1.3\n"
			  << "  -FdRm <value>\t\tMinimum restriction increase rate\n"
			  << "  \t\t\t[1, +INF]\tdefault = 1.4\n"
			  << "  -FdRM <value>\t\tMaximum restriction increase rate\n"
			  << "  \t\t\t(1, +INF]\tdefault = 2.0\n";
}

void init(char const *argv[])
{
	switch(opt_type)
	{
		case 0: optm = new BuildOrder::Optimizer::NSGA2(archSize, creation_cycles); break;
		case 1: optm = new BuildOrder::Optimizer::MOGRASP(archSize, creation_cycles, mograsp_parents, children); break;
		case 2: optm = new BuildOrder::Optimizer::Ants(archSize, aco_ants, children, aco_alpha, aco_beta, aco_evrate); break;
		case 3: optm = new BuildOrder::Optimizer::Knee(archSize, creation_cycles); break;
	}

	switch(neighborhood)
	{
		case 0: optm->neighborhood = BuildOrder::Optimizer::delete_one; break;
		case 1: optm->neighborhood = BuildOrder::Optimizer::delete_tail; break;
		case 2: optm->neighborhood = BuildOrder::Optimizer::one_swap; break;
		case 3: optm->neighborhood = BuildOrder::Optimizer::insert; break;
	}

	optm->stop_chance = 0.3;

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

	BuildOrder::Optimizer::initOptimizer(*optm,argv[3]);
	std::cout << optm->print();
	optm->update();

	tt = std::chrono::system_clock::now() - a;
	std::cout << "\n--------------\nPREPROCESSING TIME = " << tt.count() << "\n--------------\n";
}

int main(int argc, char const *argv[])

{
	BuildOrder::Optimizer::Solution test;
	contiguous<BuildOrder::Optimizer::Population> total;
	BuildOrder::Optimizer::Population ret, front;
	

	if (argc < 4)
	{
		print_help(argv[0]);
		return 0;
	}

	init(argv);

	test.orders.push_back(14);test.orders.push_back(3);
	test.update(state,optm->maximum_time);
	BuildOrder::print(test.orders);

	for (unsigned i = 0; i < 300; i++)
	{
		a = std::chrono::system_clock::now();
		auto lastfront = optm->optimize(state, 5);
		total.push_back(lastfront);

		tt = std::chrono::system_clock::now() - a;
		double duration = tt.count();
		std::cout << "TIME = " << duration << "\n";

		for (unsigned k = 0; k < lastfront.size(); k++)
			std::cerr << optm->print(lastfront[k]) << "\n";

		std::cerr << "\n";

		ret.insert(ret.begin(), total.back().begin(),total.back().end());
	}

	front = optm->nonDominated(ret);

	for (unsigned i = 0; i < front.size(); i++)
	{
		std::cout << "\n" << (i+1) << "\n";
		std::cout << "BUILD ORDER: ";
		print(front[i].orders);

		std::cout << "Time: " << front[i].final_state.time << "\n";
		std::cout << "Minerals: " << front[i].final_state.resources[0].usable() << "\n";
		std::cout << "Zerglings: " << front[i].final_state.resources[7].usable() << "\n";

		std::cerr << optm->print(front[i]) << "\n";
		//std::cerr << front[i].final_state.time << " -" << front[i].final_state.resources[0].usable() << "\n";
	}

	return 0;
}