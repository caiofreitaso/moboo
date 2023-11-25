#include "../include/knee-archiver.h"

contiguous<contiguous<double> > BuildOrder::Optimizer::Knee_Archiver::_values;
unsigned BuildOrder::Optimizer::Knee_Archiver::_v_size = 0;
unsigned BuildOrder::Optimizer::Knee_Archiver::_v_obj = 0;

BuildOrder::Optimizer::Knee_Archiver::Knee_Archiver(unsigned c, const Optimizer* o)
: NSGA2_Archiver(c,o)
{
	dist_func = Knee_Archiver::knees;
}

void BuildOrder::Optimizer::Knee_Archiver::knees (
	contiguous<contiguous<unsigned> >& F,
	contiguous<contiguous<unsigned> >& P,
	contiguous<bool> const& min)
{
	for (unsigned I = 0; I < F.size(); I++)
		if (F[I].size() > 2)
		{
			contiguous<double> distance(F[I].size(), 0);

			for (unsigned m = 0; m < min.size(); m++)
			{
				quicksort(F[I], 0, F[I].size()-1, m, P);

				distance[0] = distance[distance.size()-1] = 1e37;
			}

			#pragma omp parallel for
			for (unsigned i = 1; i < F[I].size()-1; i++)
				if (distance[i] == 0)
				{
					unsigned closest, second;
					double minimum_dist = 1e37, second_dist = 1e37;
					contiguous<double> A(min.size()), B(min.size());

					for (unsigned j = 0; j < F[I].size(); j++)
						if (j != i)
						{
							contiguous<double> diff(min.size());
							double dist = 0;
							
							for (unsigned m = 0; m < min.size(); m++)
							{
								diff[m] = (double)P[F[I][j]][m] - (double)P[F[I][i]][m];
								dist = diff[m] * diff[m];
							}

							if (dist < minimum_dist)
							{
								closest = j;
								minimum_dist = dist;
								A = diff;
							}
						}

					for (unsigned j = 0; j < F[I].size(); j++)
						if (j != i && j != closest)
						{
							contiguous<double> diff(min.size());
							double dist = 0;
							unsigned eq_count = 0;
							
							for (unsigned m = 0; m < min.size(); m++)
							{
								diff[m] = (double)P[F[I][j]][m] - (double)P[F[I][i]][m];
								if (diff[m] > 0 && A[m] > 0 || diff[m] <= 0 && A[m] <= 0)
									eq_count++;
								dist = diff[m] * diff[m];
							}

							if (eq_count < min.size())
								if (dist < minimum_dist)
								{
									second = j;
									second_dist = dist;
									B = diff;
								}
						}

					double angle = 0;
					double dot = 0;

					for (unsigned m = 0; m < min.size(); m++)
						dot += A[m]*B[m];

					angle = dot/(minimum_dist*second_dist);
					distance[i] = M_PI + M_PI - acos(angle);
				}
			
			quicksort(F[I], 0u, F[I].size()-1, distance);
		}
}