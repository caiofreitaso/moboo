#ifndef EXACTH
#define EXACTH

#include <iostream>
#include "agarchiver.h"
#include "create_solution.h"
#include <omp.h>

namespace BuildOrder
{
	namespace Optimizer
	{

		class ExactOpt : public Optimizer
		{
			
			public:
				unsigned population_size;

				ExactOpt(unsigned pop_size) : population_size(pop_size) { }

				virtual Population optimize(GameState initial, unsigned iterations)
				{
					AdaptativeGrid_Archiver population(population_size,this);
					Population previous, actual;

					contiguous<unsigned> valid(Rules::tasks.size(), 0);
					std::chrono::time_point<std::chrono::system_clock> a = std::chrono::system_clock::now();

					for (unsigned t = 0; t < valid.size(); t++)
						valid[t] = hasPrerequisites(t,initial);

					for (unsigned t = 0; t < valid.size(); t++)
						if (valid[t])
						{
							Solution n;
							n.orders.push_back(t);
							n.update(initial, this->maximum_time);
							
							actual.push_back(n);
						}

					for (unsigned s = 0; s < actual.size(); s++)
						if (actual[s].final_state.time > maximum_time)
						{
							pop(actual, s);
							s--;
						}

					std::chrono::duration<double> tt = std::chrono::system_clock::now() - a;

					for (unsigned it = 0; it < iterations && actual.size() > 0; it++)
					{
						std::cout	<< "It" << it << ":\t" << population.size()
								 	<< " [" << previous.size() << ","
								 	<< actual.size() << "] @"
									<< actual.front().orders.size() <<
									"\t" << tt.count() << "\n";

						a = std::chrono::system_clock::now();
						Population n_actual;
						for (unsigned s = 0; s < actual.size(); s++)
						{
							if (this->valid(actual[s]))
								population.insert(actual[s]);
							else
								n_actual.push_back(actual[s]);
						}
						actual = n_actual;
						n_actual.clear();
						tt = std::chrono::system_clock::now() - a;

						std::cout	<< "\t" << population.size()
									<< " [" << actual.size() << "]"
									<< "\t" << tt.count() << "\n";

						a = std::chrono::system_clock::now();
						#pragma omp parallel for num_threads(20)
						for (unsigned s = 0; s < actual.size(); s++)
						{
							bool dom = false;
							for (unsigned k = 0; k < population.size(); k++)
								if (dominates(population[k], actual[s]))
									{
										dom = true;
										break;
									}

							if (!dom)
								#pragma omp critical
								n_actual.push_back(actual[s]);
						}
						actual = n_actual;
						n_actual.clear();
						tt = std::chrono::system_clock::now() - a;

						std::cout	<< "\t" << population.size()
									<< " [" << actual.size() << "]"
									<< "\t" << tt.count() << "\n";

						a = std::chrono::system_clock::now();
						previous = actual;
						actual.clear();

						
						#pragma omp parallel for num_threads(20)
						for (unsigned s = 0; s < previous.size(); s++)
							if (previous[s].orders.size())
							{
								contiguous<unsigned> n_valid(Rules::tasks.size(), 0);

								for (unsigned t = 0; t < n_valid.size(); t++)
									n_valid[t] = ::BuildOrder::Objective::possible(t, previous[s].final_state);

								#pragma omp parallel for num_threads(5)
								for (unsigned t = 0; t < n_valid.size(); t++)
									if (n_valid[t])
									{
										Solution n(previous[s]);
										n.orders.push_back(t);
										n.update(initial, this->maximum_time);

										if (n.final_state.time <= maximum_time)
											if (n.orders.size() > previous[s].orders.size())
											{
												#pragma omp critical
												actual.push_back(n);
											}
									}
							}
						tt = std::chrono::system_clock::now() - a;
					}

					return population();
				}

		};
	}
}

#endif