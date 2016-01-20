#include <iostream>
#include <cstring>
#include <cstdlib>
#include "../include/knee.h"
#include "../include/nsga2.h"
#include "../include/mograsp.h"
#include "../include/ants.h"

std::chrono::time_point<std::chrono::system_clock> a;
std::chrono::duration<double> tt;

BuildOrder::GameState state;
BuildOrder::Optimizer::Optimizer* optm = 0;
unsigned opt_type = 3;
unsigned neighborhood = 2;
unsigned iterations = 20;
unsigned archSize = 20;
double stop_chance = 0.3;
unsigned runs = 1;
unsigned localsearch_runs = 5;

double aco_alpha = 0.5;
double aco_beta = 0.5;
double aco_evrate = 0.5;

unsigned creation_cycles = 5;
unsigned children = 1;

double mograsp_parents = 1;

bool stdout_time = false;
bool stdout_front = false;
bool stderr_time = false;
bool stderr_front = false;

bool print_orders = false;
bool stdout_filter = false;
bool stderr_filter = false;

int set_options(int argc, char const *argv[]);
void print_help(char const* program_name);
void init(char const *argv[]);

int main(int argc, char const *argv[])

{
	BuildOrder::Optimizer::Solution test;
	contiguous<BuildOrder::Optimizer::Population> total;
	BuildOrder::Optimizer::Population ret, front;
	

	if (!set_options(argc,argv))
		return 0;

	for (unsigned i = 0; i < runs; i++)
	{
		a = std::chrono::system_clock::now();
		auto lastfront = optm->optimize(state, 5);
		total.push_back(lastfront);

		tt = std::chrono::system_clock::now() - a;
		double duration = tt.count();

		if (stdout_time)
			std::cout << duration << "\n";
		else if (stderr_time)
			std::cerr << duration << "\n";

		if (stderr_front)
		{
			for (unsigned k = 0; k < lastfront.size(); k++)
				std::cerr << optm->print(lastfront[k]) << "\n";
			std::cerr << "\n";
		} else if (stdout_front)
		{
			for (unsigned k = 0; k < lastfront.size(); k++)
				std::cout << optm->print(lastfront[k]) << "\n";
			std::cout << "\n";
		}

		ret.insert(ret.begin(), total.back().begin(),total.back().end());
	}

	front = optm->nonDominated(ret);

	for (unsigned i = 0; i < front.size(); i++)
	{
		if (print_orders)
		{
			std::cout << "\n" << (i+1) << "\n";
			std::cout << "BUILD ORDER: ";
			print(front[i].orders);

			std::cout << "Time: " << front[i].final_state.time << "\n";
			std::cout << "Minerals: " << front[i].final_state.resources[0].usable() << "\n";
			std::cout << "Zerglings: " << front[i].final_state.resources[7].usable() << "\n";
		}

		if (stdout_filter)
			std::cout << optm->print(front[i]) << "\n";
		else if (stderr_filter)
			std::cerr << optm->print(front[i]) << "\n";
		//std::cerr << front[i].final_state.time << " -" << front[i].final_state.resources[0].usable() << "\n";
	}

	return 0;
}

int set_options(int argc, char const *argv[])
{
	if (argc < 4 || argc % 2 > 0)
	{
	optionsfail:
		print_help(argv[0]);
		return 0;
	}

	for (int i = 4; i < argc; i = i+2)
	{
		if (!strcmp(argv[i],"-o"))
		{
			if(!strcmp(argv[i+1],"nsga2"))
				opt_type = 0;
			else if(!strcmp(argv[i+1],"knee"))
				opt_type = 1;
			else if(!strcmp(argv[i+1],"aco"))
				opt_type = 2;
			else if(!strcmp(argv[i+1],"mograsp"))
				opt_type = 3;
			else
				goto optionsfail;
		} else if (!strcmp(argv[i],"-n")) {
			if(!strcmp(argv[i+1],"delete_one"))
				neighborhood = 0;
			else if(!strcmp(argv[i+1],"delete_tail"))
				neighborhood = 1;
			else if(!strcmp(argv[i+1],"one_swap"))
				neighborhood = 2;
			else if(!strcmp(argv[i+1],"insert"))
				neighborhood = 3;
			else
				goto optionsfail;
		} else if (!strcmp(argv[i],"-i")) {
			iterations = strtol(argv[i+1],0,10);
			if (iterations <= 0)
				goto optionsfail;
		} else if (!strcmp(argv[i],"-s")) {
			archSize = strtol(argv[i+1],0,10);
			if (archSize <= 0)
				goto optionsfail;
		} else if (!strcmp(argv[i],"-r")) {
			runs = strtol(argv[i+1],0,10);
			if (runs <= 0)
				goto optionsfail;
		} else if (!strcmp(argv[i],"-ls")) {
			localsearch_runs = strtol(argv[i+1],0,10);
			if (localsearch_runs <= 0)
				goto optionsfail;
		} else if (!strcmp(argv[i],"-c")) {
			stop_chance = strtof(argv[i+1],0);
			if (stop_chance <= 0 || stop_chance > 1)
				goto optionsfail;
		} else if (!strcmp(argv[i],"-an")) {
			creation_cycles = strtol(argv[i+1],0,10);
			if (creation_cycles <= 0 || opt_type != 2)
				goto optionsfail;
		} else if (!strcmp(argv[i],"-aa")) {
			aco_alpha = strtof(argv[i+1],0);
			if (aco_alpha < 0 || opt_type != 2)
				goto optionsfail;
		} else if (!strcmp(argv[i],"-ab")) {
			aco_beta = strtof(argv[i+1],0);
			if (aco_beta <= 0 || opt_type != 2)
				goto optionsfail;
		} else if (!strcmp(argv[i],"-ac")) {
			children = strtol(argv[i+1],0,10);
			if (children <= 0 || opt_type != 2)
				goto optionsfail;
		} else if (!strcmp(argv[i],"-ae")) {
			aco_evrate = strtof(argv[i+1],0);
			if (aco_evrate < 0 || aco_evrate > 1 || opt_type != 2)
				goto optionsfail;
		} else if (!strcmp(argv[i],"-nc")) {
			creation_cycles = strtol(argv[i+1],0,10);
			if (creation_cycles <= 0 || opt_type > 1)
				goto optionsfail;
		} else if (!strcmp(argv[i],"-mc")) {
			creation_cycles = strtol(argv[i+1],0,10);
			if (creation_cycles <= 0 || opt_type < 3)
				goto optionsfail;
		} else if (!strcmp(argv[i],"-mh")) {
			children = strtol(argv[i+1],0,10);
			if (children <= 0 || opt_type < 3)
				goto optionsfail;
		} else if (!strcmp(argv[i],"-mp")) {
			mograsp_parents = strtof(argv[i+1],0);
			if (mograsp_parents < 0 || mograsp_parents > 1 || opt_type < 3)
				goto optionsfail;
		}

		else if (!strcmp(argv[i],"-objm")) {
			BuildOrder::Optimizer::Creation::objective_minimum = strtof(argv[i+1],0);
			if (BuildOrder::Optimizer::Creation::objective_maximum < BuildOrder::Optimizer::Creation::objective_minimum)
				std::swap(BuildOrder::Optimizer::Creation::objective_maximum,BuildOrder::Optimizer::Creation::objective_minimum);
		} else if (!strcmp(argv[i],"-objM")) {
			BuildOrder::Optimizer::Creation::objective_maximum = strtof(argv[i+1],0);
			if (BuildOrder::Optimizer::Creation::objective_maximum < BuildOrder::Optimizer::Creation::objective_minimum)
				std::swap(BuildOrder::Optimizer::Creation::objective_maximum,BuildOrder::Optimizer::Creation::objective_minimum);
		}
		else if (!strcmp(argv[i],"-resm")) {
			BuildOrder::Optimizer::Creation::restriction_minimum = strtof(argv[i+1],0);
			if (BuildOrder::Optimizer::Creation::restriction_maximum < BuildOrder::Optimizer::Creation::restriction_minimum)
				std::swap(BuildOrder::Optimizer::Creation::restriction_maximum,BuildOrder::Optimizer::Creation::restriction_minimum);
		} else if (!strcmp(argv[i],"-resM")) {
			BuildOrder::Optimizer::Creation::restriction_maximum = strtof(argv[i+1],0);
			if (BuildOrder::Optimizer::Creation::restriction_maximum < BuildOrder::Optimizer::Creation::restriction_minimum)
				std::swap(BuildOrder::Optimizer::Creation::restriction_maximum,BuildOrder::Optimizer::Creation::restriction_minimum);
		}
		else if (!strcmp(argv[i],"-dOm")) {
			BuildOrder::Optimizer::Creation::delta_o_minimum = strtof(argv[i+1],0);
			if (BuildOrder::Optimizer::Creation::delta_o_maximum < BuildOrder::Optimizer::Creation::delta_o_minimum)
				std::swap(BuildOrder::Optimizer::Creation::delta_o_maximum,BuildOrder::Optimizer::Creation::delta_o_minimum);
		} else if (!strcmp(argv[i],"-dOM")) {
			BuildOrder::Optimizer::Creation::delta_o_maximum = strtof(argv[i+1],0);
			if (BuildOrder::Optimizer::Creation::delta_o_maximum <= BuildOrder::Optimizer::Creation::delta_o_minimum)
				std::swap(BuildOrder::Optimizer::Creation::delta_o_maximum,BuildOrder::Optimizer::Creation::delta_o_minimum);
		}
		else if (!strcmp(argv[i],"-dRm")) {
			BuildOrder::Optimizer::Creation::delta_r_minimum = strtof(argv[i+1],0);
			if (BuildOrder::Optimizer::Creation::delta_r_maximum < BuildOrder::Optimizer::Creation::delta_r_minimum)
				std::swap(BuildOrder::Optimizer::Creation::delta_r_maximum,BuildOrder::Optimizer::Creation::delta_r_minimum);
		} else if (!strcmp(argv[i],"-dRM")) {
			BuildOrder::Optimizer::Creation::delta_r_maximum = strtof(argv[i+1],0);
			if (BuildOrder::Optimizer::Creation::delta_r_maximum <= BuildOrder::Optimizer::Creation::delta_r_minimum)
				std::swap(BuildOrder::Optimizer::Creation::delta_r_maximum,BuildOrder::Optimizer::Creation::delta_r_minimum);
		}

		else if (!strcmp(argv[i],"-obj")) {
			BuildOrder::Optimizer::Creation::objective_minimum =
			BuildOrder::Optimizer::Creation::objective_maximum = strtof(argv[i+1],0);
		} 
		else if (!strcmp(argv[i],"-res")) {
			BuildOrder::Optimizer::Creation::restriction_minimum = 
			BuildOrder::Optimizer::Creation::restriction_maximum = strtof(argv[i+1],0);
		}
		else if (!strcmp(argv[i],"-dO")) {
			BuildOrder::Optimizer::Creation::delta_o_minimum = 
			BuildOrder::Optimizer::Creation::delta_o_maximum = strtof(argv[i+1],0);
		}
		else if (!strcmp(argv[i],"-dR")) {
			BuildOrder::Optimizer::Creation::delta_r_minimum = 
			BuildOrder::Optimizer::Creation::delta_r_maximum = strtof(argv[i+1],0);
		}

		else if (!strcmp(argv[i],"-Fobjm")) {
			BuildOrder::Optimizer::Fix::objective_minimum = strtof(argv[i+1],0);
			if (BuildOrder::Optimizer::Creation::objective_maximum < BuildOrder::Optimizer::Creation::objective_minimum)
				std::swap(BuildOrder::Optimizer::Creation::objective_maximum,BuildOrder::Optimizer::Creation::objective_minimum);
		} else if (!strcmp(argv[i],"-FobjM")) {
			BuildOrder::Optimizer::Fix::objective_maximum = strtof(argv[i+1],0);
			if (BuildOrder::Optimizer::Fix::objective_maximum < BuildOrder::Optimizer::Fix::objective_minimum)
				std::swap(BuildOrder::Optimizer::Creation::objective_maximum,BuildOrder::Optimizer::Creation::objective_minimum);
		} else if (!strcmp(argv[i],"-Fresm")) {
			BuildOrder::Optimizer::Fix::restriction_minimum = strtof(argv[i+1],0);
			if (BuildOrder::Optimizer::Creation::restriction_maximum < BuildOrder::Optimizer::Creation::restriction_minimum)
				std::swap(BuildOrder::Optimizer::Creation::restriction_maximum,BuildOrder::Optimizer::Creation::restriction_minimum);
		} else if (!strcmp(argv[i],"-FresM")) {
			BuildOrder::Optimizer::Fix::restriction_maximum = strtof(argv[i+1],0);
			if (BuildOrder::Optimizer::Fix::restriction_maximum < BuildOrder::Optimizer::Fix::restriction_minimum)
				std::swap(BuildOrder::Optimizer::Creation::restriction_maximum,BuildOrder::Optimizer::Creation::restriction_minimum);
		} else if (!strcmp(argv[i],"-FdOm")) {
			BuildOrder::Optimizer::Fix::delta_o_minimum = strtof(argv[i+1],0);
			if (BuildOrder::Optimizer::Creation::delta_o_maximum < BuildOrder::Optimizer::Creation::delta_o_minimum)
				std::swap(BuildOrder::Optimizer::Creation::delta_o_maximum,BuildOrder::Optimizer::Creation::delta_o_minimum);
		} else if (!strcmp(argv[i],"-FdOM")) {
			BuildOrder::Optimizer::Fix::delta_o_maximum = strtof(argv[i+1],0);
			if (BuildOrder::Optimizer::Fix::delta_o_maximum <= BuildOrder::Optimizer::Fix::delta_o_minimum)
				std::swap(BuildOrder::Optimizer::Creation::delta_o_maximum,BuildOrder::Optimizer::Creation::delta_o_minimum);
		} else if (!strcmp(argv[i],"-FdRm")) {
			BuildOrder::Optimizer::Fix::delta_r_minimum = strtof(argv[i+1],0);
			if (BuildOrder::Optimizer::Creation::delta_r_maximum < BuildOrder::Optimizer::Creation::delta_r_minimum)
				std::swap(BuildOrder::Optimizer::Creation::delta_r_maximum,BuildOrder::Optimizer::Creation::delta_r_minimum);
		} else if (!strcmp(argv[i],"-FdRM")) {
			BuildOrder::Optimizer::Fix::delta_r_maximum = strtof(argv[i+1],0);
			if (BuildOrder::Optimizer::Fix::delta_r_maximum <= BuildOrder::Optimizer::Fix::delta_r_minimum)
				std::swap(BuildOrder::Optimizer::Creation::delta_r_maximum,BuildOrder::Optimizer::Creation::delta_r_minimum);
		}

		else if (!strcmp(argv[i],"-Fobj")) {
			BuildOrder::Optimizer::Fix::objective_minimum = 
			BuildOrder::Optimizer::Fix::objective_maximum = strtof(argv[i+1],0);
		} else if (!strcmp(argv[i],"-Fres")) {
			BuildOrder::Optimizer::Fix::restriction_minimum = 
			BuildOrder::Optimizer::Fix::restriction_maximum = strtof(argv[i+1],0);
		} else if (!strcmp(argv[i],"-FdO")) {
			BuildOrder::Optimizer::Fix::delta_o_minimum = 
			BuildOrder::Optimizer::Fix::delta_o_maximum = strtof(argv[i+1],0);
		} else if (!strcmp(argv[i],"-FdR")) {
			BuildOrder::Optimizer::Fix::delta_r_minimum = 
			BuildOrder::Optimizer::Fix::delta_r_maximum = strtof(argv[i+1],0);
		}






		else if (!strcmp(argv[i],"-std")) {
			if(!strcmp(argv[i+1],"time"))
			{
				stdout_time = true;
				stdout_front = false;
			}
			else if(!strcmp(argv[i+1],"fronts"))
			{
				stdout_front = true;
				stdout_time = false;
			}
			else
				goto optionsfail;
		} else if (!strcmp(argv[i],"-err")) {
			if(!strcmp(argv[i+1],"time"))
			{
				stderr_time = true;
				stderr_front = false;
			}
			else if(!strcmp(argv[i+1],"fronts"))
			{
				stderr_front = true;
				stderr_time = false;
			}
			else
				goto optionsfail;
		} else if (!strcmp(argv[i],"-P"))
			print_orders = true;
		else if (!strcmp(argv[i],"-f")) {
			if(!strcmp(argv[i+1],"std"))
			{
				stdout_filter = true;
				stderr_filter = false;
			}
			else if(!strcmp(argv[i+1],"err"))
			{
				stdout_filter = false;
				stderr_filter = true;
			}
			else
				goto optionsfail;
		}
	}

	init(argv);
	return 1;
}

void print_help(char const* program_name)
{
	std::cout << "Usage: " << program_name << " system state model [options]\n\n"
			  << "  system\tFile with the system description.\n"
			  << "  state\t\tFile with the initial state description.\n"
			  << "  model\t\tFile with the problem description.\n\n"

		  << "Options:\n"
			  << "  -o <optimizer>\tChoose the optimizer:\n"
			  << "  \t\t\tnsga2, knee, aco, mograsp\n"
			  << "  -n <neighborhood>\tChoose the neighborhood:\n"
			  << "  \t\t\tdelete_one, delete_tail, one_swap, insert\n"
			  << "  -i <value>\t\tNumber of iterations\n"
			  << "  \t\t\t[1, +INF]\tdefault = 20\n"
			  << "  -s <value>\t\tArchiver size\n"
			  << "  \t\t\t[1, +INF]\tdefault = 20\n"
			  << "  -r <value>\t\tNumber of runs\n"
			  << "  \t\t\t[1, +INF]\tdefault = 1\n"
			  << "  -ls <value>\t\tMaximum number of local search branches\n"
			  << "  \t\t\t[1, +INF]\tdefault = 5\n"
			  << "  -c <value>\t\tStop chance\n"
			  << "  \t\t\t(0, 1]\tdefault = 0.3\n\n"
		  
		  << "OUTPUT Options:\n"
			  << "  -std <value>\tChoose which value goes to stdout:\n"
			  << "  \t\t\ttime, fronts\n"
			  << "  -err <value>\tChoose which value goes to stderr:\n"
			  << "  \t\t\ttime, fronts\n"
			  << "  -P\t\tPrint the orders from the non-dominated front.\n"
			  << "  -f <output>\tChoose to which output the filtered front goes:\n"
			  << "  \t\t\tstd, err\n"

		  
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
		case 1: optm = new BuildOrder::Optimizer::Knee(archSize, creation_cycles); break;
		case 2: optm = new BuildOrder::Optimizer::Ants(archSize, creation_cycles, children, aco_alpha, aco_beta, aco_evrate); break;
		case 3: optm = new BuildOrder::Optimizer::MOGRASP(archSize, creation_cycles, mograsp_parents, children); break;
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
	//std::cout << "INITIAL STATE:\n";
	//state.print();
	//std::cout << "--------------\n\n";
	/////////////////////////////////////////////////////

	BuildOrder::Optimizer::initOptimizer(*optm,argv[3]);
	//std::cout << optm->print();
	optm->update();

	tt = std::chrono::system_clock::now() - a;
	//std::cout << "\n--------------\nPREPROCESSING TIME = " << tt.count() << "\n--------------\n";
}