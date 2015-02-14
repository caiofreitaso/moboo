#include "../include/nsga2-archiver.h"

bool BuildOrder::Optimizer::NSGA2_Archiver::dominates(std::vector<unsigned> a,
	std::vector<unsigned> b, std::vector<bool> min)
{
	bool strictly = false;
	for (unsigned i = 0; i < a.size(); i++)
		if (min[i])
		{
			if (a[i] < b[i])
				strictly = true;
			else if (a[i] > b[i])
				return false;
		}
		else
		{
			if (a[i] > b[i])
				strictly = true;
			else if (a[i] < b[i])
				return false;
		}

	return strictly;
}

void BuildOrder::Optimizer::NSGA2_Archiver::quicksort(std::vector<unsigned>& indexes,
	unsigned begin, unsigned end, unsigned objective,
	std::vector<std::vector<unsigned> >& v)
{
	if (end > indexes.size())
		return;

	if (begin < end)
	{
		unsigned pivotIndex = (begin + end)/2;
		unsigned pivotValue = v[indexes[pivotIndex]][objective];

		std::swap(indexes[pivotIndex],indexes[end]);

		unsigned storeIndex = begin;

		for (unsigned i = storeIndex; i < end; i++)
			if (v[indexes[i]][objective] <= pivotValue)
			{
				std::swap(indexes[i],indexes[storeIndex]);
				storeIndex++;
			}
		std::swap(indexes[storeIndex], indexes[end]);

		quicksort(indexes,begin,storeIndex - 1,objective,v);
		quicksort(indexes,storeIndex + 1,end,objective,v);
	}
}
void BuildOrder::Optimizer::NSGA2_Archiver::quicksort(std::vector<unsigned>& v,
			   unsigned begin, unsigned end, std::vector<double>& dist)
{
	if (end > v.size())
		return;

	if (begin < end)
	{
		unsigned pivotIndex = (begin + end)/2;
		double pivotValue = dist[pivotIndex];

		std::swap(v[pivotIndex],v[end]);
		std::swap(dist[pivotIndex],dist[end]);

		unsigned storeIndex = begin;

		for (unsigned i = storeIndex; i < end; i++)
			if (dist[i] > pivotValue)
			{
				std::swap(v[i],v[storeIndex]);
				std::swap(dist[i],dist[storeIndex]);
				storeIndex++;
			}
		std::swap(v[storeIndex], v[end]);
		std::swap(dist[storeIndex], dist[end]);

		quicksort(v,begin,storeIndex - 1,dist);
		quicksort(v,storeIndex + 1,end,dist);
	}
}

BuildOrder::Optimizer::NSGA2_Archiver::NSGA2_Archiver(unsigned c, const Optimizer* o)
: dist_func(NSGA2_Archiver::crowding)
{
	_capacity = c;
	_data.reserve(c+1);
	_optimizer = o;
}

void BuildOrder::Optimizer::NSGA2_Archiver::filter(Population& pop) const
{
	//TRANSLATE INTO VECTORS
	std::vector<std::vector<unsigned> > P(pop.size());
	std::vector<bool> min;

	min.push_back(true);
	for (unsigned i = 0; i < _optimizer->objectives[0].row.size(); i++)
		min.push_back(_optimizer->objectives[0].row[i].value == MINIMIZE);

	for (unsigned i = 0; i < _optimizer->objectives[1].row.size(); i++)
		min.push_back(_optimizer->objectives[1].row[i].value == MINIMIZE);

	for (unsigned i = 0; i < _optimizer->objectives[2].row.size(); i++)
		min.push_back(_optimizer->objectives[2].row[i].value == MINIMIZE);

	for (unsigned p = 0; p < P.size(); p++)
	{
		P[p].push_back(pop[p].final_state.time);

		for (unsigned i = 0; i < _optimizer->objectives[0].row.size(); i++)
		{
			unsigned index = _optimizer->objectives[0].row[i].index;

			P[p].push_back(pop[p].final_state.resources[index].usable());
		}

		for (unsigned i = 0; i < _optimizer->objectives[1].row.size(); i++)
		{
			unsigned index = _optimizer->objectives[1].row[i].index;

			P[p].push_back(pop[p].final_state.resources[index].quantity);
		}

		for (unsigned i = 0; i < _optimizer->objectives[2].row.size(); i++)
		{
			unsigned index = _optimizer->objectives[2].row[i].index;

			P[p].push_back(pop[p].final_state.resources[index].used);
		}
	}

	//FAST-NON-DOMINATED-SORT
	std::vector<std::vector<unsigned> > S(P.size());
	std::vector<unsigned> n(P.size());

	std::vector<std::vector<unsigned> > F(1);

	for (unsigned p = 0; p < P.size(); p++)
	{
		n[p] = 0;

		for (unsigned q = 0; q < P.size(); q++)
			if (dominates(P[p], P[q], min))
				S[p].push_back(q);
			else if (dominates(P[q],P[p], min))
				n[p]++;

		if (n[p] == 0)
			F[0].push_back(p);
	}

	for (unsigned i = 0; F[i].size(); i++)
	{
		F.push_back(std::vector<unsigned>());

		for (unsigned p = 0; p < F[i].size(); p++)
			for (unsigned q = 0; q < S[F[i][p]].size(); q++)
			{
				n[S[F[i][p]][q]]--;
				if (n[S[F[i][p]][q]] == 0)
					F[i+1].push_back(S[F[i][p]][q]);
			}
	}

	if (!F[F.size()-1].size())
		F.pop_back();

	//CROWDING-DISTANCE-ASSIGNMENT
	/*for (unsigned I = 0; I < F.size(); I++)
		if (F[I].size() > 2)
		{
			std::vector<double> distance(F[I].size(), 0);

			for (unsigned m = 0; m < min.size(); m++)
			{
				quicksort(F[I], 0, F[I].size()-1, m, P);

				distance[0] = distance[distance.size()-1] = 1e37;

				for (unsigned i = 1; i < F[I].size()-1; i++)
				{
					double dist = P[F[I][i+1]][m] - P[F[I][i-1]][m];
					dist /= P[F[I][F[I].size()-1]][m] - P[F[I][0]][m];
					distance[i] += dist;
				}
			}

			quicksort(F[I], 0u, F[I].size()-1, distance);
		}*/
	dist_func(F,P,min);

	//SORTING
	Population new_pop;
	for (unsigned i = 0; i < F.size(); i++)
		for (unsigned p = 0; p < F[i].size(); p++)
			new_pop.push_back(pop[F[i][p]]);

	pop = new_pop;
}

void BuildOrder::Optimizer::NSGA2_Archiver::crowding (
	std::vector<std::vector<unsigned> >& F,
	std::vector<std::vector<unsigned> >& P,
	std::vector<bool> const& min)
{
	for (unsigned I = 0; I < F.size(); I++)
		if (F[I].size() > 2)
		{
			std::vector<double> distance(F[I].size(), 0);

			for (unsigned m = 0; m < min.size(); m++)
			{
				quicksort(F[I], 0, F[I].size()-1, m, P);

				distance[0] = distance[distance.size()-1] = 1e37;

				for (unsigned i = 1; i < F[I].size()-1; i++)
				{
					double dist = P[F[I][i+1]][m] - P[F[I][i-1]][m];
					dist /= P[F[I][F[I].size()-1]][m] - P[F[I][0]][m];
					distance[i] += dist;
				}
			}

			quicksort(F[I], 0u, F[I].size()-1, distance);
		}
}