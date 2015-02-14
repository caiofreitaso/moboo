#include "../include/rules.h"

std::vector<BuildOrder::Rules::Task> BuildOrder::Rules::tasks;
std::vector<BuildOrder::Rules::Event> BuildOrder::Rules::events;
std::vector<BuildOrder::Rules::Resource> BuildOrder::Rules::resources;

std::vector<MatrixRow<unsigned> > BuildOrder::Rules::taskValuePerEvent;
std::vector<MatrixRow<unsigned> > BuildOrder::Rules::resourceValueLost;

BuildOrder::Rules::Forest BuildOrder::Rules::prerequisites;
BuildOrder::Rules::Forest BuildOrder::Rules::maxima;
BuildOrder::Rules::Forest BuildOrder::Rules::costs;
BuildOrder::Rules::Forest BuildOrder::Rules::consumes;

unsigned BuildOrder::Rules::mean_time = 0;

void BuildOrder::Rules::init(char const* filename)
{
	std::fstream file;
	std::string buffer;
	std::stringstream sstream;

	file.open(filename);

	//ALLOCATING
	{
		do
			std::getline(file,buffer);
		while(buffer[0] == '#' || buffer.size() == 0);

		sstream.clear();
		sstream.str(buffer);

		unsigned res_sz, tks_sz, evt_sz;

		sstream >> res_sz;
		sstream >> tks_sz;
		sstream >> evt_sz;

		resources.reserve(res_sz);
		tasks.reserve(tks_sz);
		events.reserve(evt_sz);

		taskValuePerEvent.reserve(res_sz);
		resourceValueLost.reserve(res_sz);

		for (unsigned i = 0; i < tasks.capacity(); i++)
			tasks.push_back(Task());

		for (unsigned i = 0; i < events.capacity(); i++)
			events.push_back(Event());

		for (unsigned i = 0; i < resources.capacity(); i++)
		{
			resources.push_back(Resource());
			taskValuePerEvent.push_back(MatrixRow<unsigned>());
			resourceValueLost.push_back(MatrixRow<unsigned>());
		}

	}

	//RESOURCES
	{
		// OVERALL MAXIMUM
		{
			do
				std::getline(file,buffer);
			while(buffer[0] == '#' || buffer.size() == 0);

			sstream.clear();
			sstream.str(buffer);

			for(unsigned i,v; !sstream.eof(); )
			{
				long p = (long)sstream.tellg();
				
				sstream >> i;
				while (sstream.fail())
				{
					sstream.clear();
					sstream.seekg(p+1);
					sstream >> i;
				}
				
				p = (long)sstream.tellg();
				sstream >> v;
				while (sstream.fail())
				{
					sstream.clear();
					sstream.seekg(p+1);
					sstream >> v;
				}

				resources[i].overall_maximum = v;
			}
		}

		// MAXIMUM PER RESOURCE
		{
			do
				std::getline(file,buffer);
			while(buffer[0] == '#' || buffer.size() == 0);

			for (unsigned i = 0; i < resources.size(); i++) {
				sstream.clear();
				sstream.str(buffer);

				for(unsigned j,v; !sstream.eof(); )
				{
					long p = (long)sstream.tellg();
					
					sstream >> j;
					while (sstream.fail())
					{
						sstream.clear();
						sstream.seekg(p+1);
						sstream >> j;
					}
					
					p = (long)sstream.tellg();
					sstream >> v;
					while (sstream.fail())
					{
						sstream.clear();
						sstream.seekg(p+1);
						sstream >> v;
					}

					resources[i].maximum_per_resource.set(j,v);
				}

				std::getline(file,buffer);
			}
		}

		//EQUIVALENCE
		{
			do
				std::getline(file,buffer);
			while(buffer[0] == '#' || buffer.size() == 0);

			for (unsigned i = 0; i < resources.size(); i++) {
				sstream.clear();
				sstream.str(buffer);

				for(unsigned j,v; !sstream.eof(); )
				{
					long p = (long)sstream.tellg();
					
					sstream >> j;
					while (sstream.fail())
					{
						sstream.clear();
						sstream.seekg(p+1);
						sstream >> j;
					}
					
					p = (long)sstream.tellg();
					sstream >> v;
					while (sstream.fail())
					{
						sstream.clear();
						sstream.seekg(p+1);
						sstream >> v;
					}

					resources[i].equivalence.set(j,v);
				}

				std::getline(file,buffer);
			}
		}
	}

	//TASKS
	{
		//TIME
		{
			do
				std::getline(file,buffer);
			while(buffer[0] == '#' || buffer.size() == 0);

			sstream.clear();
			sstream.str(buffer);

			mean_time = 0;

			for (unsigned i = 0; i < tasks.size(); i++)
			{
				sstream >> tasks[i].time;
				mean_time += tasks[i].time;
			}
			mean_time /= tasks.size();
		}

		//COSTS
		{
			for (unsigned i = 0; i < tasks.size(); i++)
			{
				do
					std::getline(file,buffer);
				while(buffer[0] == '#' || buffer.size() == 0);

				sstream.clear();
				sstream.str(buffer);

				for(unsigned j,v; !sstream.eof(); )
				{
					long p = (long)sstream.tellg();
					
					sstream >> j;
					while (sstream.fail())
					{
						sstream.clear();
						sstream.seekg(p+1);
						sstream >> j;
					}
					
					p = (long)sstream.tellg();
					sstream >> v;
					while (sstream.fail())
					{
						sstream.clear();
						sstream.seekg(p+1);
						sstream >> v;
					}

					tasks[i].costs.set(j,(int)v);
				}
			}
		}

		//PREREQUISITE
		{
			do
				std::getline(file,buffer);
			while(buffer[0] == '#' || buffer.size() == 0);

			for (unsigned i = 0; i < tasks.size(); i++) {
				sstream.clear();
				sstream.str(buffer);

				for(unsigned j,v; !sstream.eof(); )
				{
					long p = (long)sstream.tellg();
					
					sstream >> j;
					while (sstream.fail())
					{
						sstream.clear();
						sstream.seekg(p+1);
						sstream >> j;
					}
					
					p = (long)sstream.tellg();
					sstream >> v;
					while (sstream.fail())
					{
						sstream.clear();
						sstream.seekg(p+1);
						sstream >> v;
					}

					tasks[i].prerequisite.set(j,v);
				}

				std::getline(file,buffer);
			}
		}

		//CONSUME
		{
			do
				std::getline(file,buffer);
			while(buffer[0] == '#' || buffer.size() == 0);

			for (unsigned i = 0; i < tasks.size(); i++) {
				sstream.clear();
				sstream.str(buffer);

				for(unsigned j,v; !sstream.eof(); )
				{
					long p = (long)sstream.tellg();
					
					sstream >> j;
					while (sstream.fail())
					{
						sstream.clear();
						sstream.seekg(p+1);
						sstream >> j;
					}
					
					p = (long)sstream.tellg();
					sstream >> v;
					while (sstream.fail())
					{
						sstream.clear();
						sstream.seekg(p+1);
						sstream >> v;
					}

					tasks[i].consume.set(j,v);
				}

				std::getline(file,buffer);
			}
		}

		//BORROW
		{
			do
				std::getline(file,buffer);
			while(buffer[0] == '#' || buffer.size() == 0);

			for (unsigned i = 0; i < tasks.size(); i++) {
				sstream.clear();
				sstream.str(buffer);

				for(unsigned j,v; !sstream.eof(); )
				{
					long p = (long)sstream.tellg();
					
					sstream >> j;
					while (sstream.fail())
					{
						sstream.clear();
						sstream.seekg(p+1);
						sstream >> j;
					}
					
					p = (long)sstream.tellg();
					sstream >> v;
					while (sstream.fail())
					{
						sstream.clear();
						sstream.seekg(p+1);
						sstream >> v;
					}

					tasks[i].borrow.set(j,v);
				}

				std::getline(file,buffer);
			}
		}

		//PRODUCE
		{
			do
				std::getline(file,buffer);
			while(buffer[0] == '#' || buffer.size() == 0);

			for (unsigned i = 0; i < tasks.size(); i++) {
				sstream.clear();
				sstream.str(buffer);

				for(unsigned j,v; !sstream.eof(); )
				{
					long p = (long)sstream.tellg();
					
					sstream >> j;
					while (sstream.fail())
					{
						sstream.clear();
						sstream.seekg(p+1);
						sstream >> j;
					}
					
					p = (long)sstream.tellg();
					sstream >> v;
					while (sstream.fail())
					{
						sstream.clear();
						sstream.seekg(p+1);
						sstream >> v;
					}

					tasks[i].produce.set(j,v);
				}

				std::getline(file,buffer);
			}
		}
	}

	//EVENTS
	{

		//TIME
		{
			do
				std::getline(file,buffer);
			while(buffer[0] == '#' || buffer.size() == 0);

			sstream.clear();
			sstream.str(buffer);

			for (unsigned i = 0; i < events.size(); i++)
				sstream >> events[i].time;
		}

		//BONUS
		{
			for (unsigned i = 0; i < events.size(); i++)
			{
				do
					std::getline(file,buffer);
				while(buffer[0] == '#' || buffer.size() == 0);

				sstream.clear();
				sstream.str(buffer);

				for(unsigned j,v; !sstream.eof(); )
				{
					long p = (long)sstream.tellg();
					
					sstream >> j;
					while (sstream.fail())
					{
						sstream.clear();
						sstream.seekg(p+1);
						sstream >> j;
					}
					
					p = (long)sstream.tellg();
					sstream >> v;
					while (sstream.fail())
					{
						sstream.clear();
						sstream.seekg(p+1);
						sstream >> v;
					}

					events[i].bonus.set(j,v);
				}

			}
		}

		//TRIGGER
		{
			do
				std::getline(file,buffer);
			while(buffer[0] == '#' || buffer.size() == 0);

			sstream.clear();
			sstream.str(buffer);

			for (unsigned i = 0; i < events.size(); i++)
				sstream >> events[i].trigger;
		}
	}

	file.close();

	for (unsigned i = 0; i < events.size(); i++)
		for (unsigned j = 0; j < events[i].bonus.row.size(); j++)
		{
			unsigned index = events[i].bonus.row[j].index;

			resourceValueLost[events[i].trigger].set(index, 1);

			for (unsigned k = 0; k < tasks.size(); k++)
				if (tasks[k].produce.get(events[i].trigger))
					taskValuePerEvent[index].set(k, 1);
		}

	prerequisites.create(RT_PREREQUISITE);
	maxima.create(RT_MAXIMUM);
	costs.create(RT_COST);
	consumes.create(RT_CONSUME);
}

void BuildOrder::Rules::value(unsigned needed, unsigned needs, relation_type p, Dependency const& d)
{
	switch(p)
	{
		case RT_COST:
			for (unsigned j = 0; j < tasks[needs].costs.row.size(); j++)
			{
				unsigned idx = tasks[needs].costs.row[j].index;
				int val = tasks[needs].costs.row[j].value;

				if (val > 0)
					for (unsigned i = 0; i < tasks[needed].produce.row.size(); i++)
					{
						unsigned index = tasks[needed].produce.row[i].index;
						unsigned value = tasks[needed].produce.row[i].value;

						if (idx == index)
						{
							d.w.set(idx, best_function_ever(value, val));
							d.bonus.set(idx, value);
						}

						if (resourceValueLost[index].get(idx))
							d.e.set(idx, 1.5);
					}
			}
			break;
		case RT_CONSUME:
			for (unsigned j = 0; j < tasks[needs].consume.row.size(); j++)
			{
				unsigned idx = tasks[needs].consume.row[j].index;
				unsigned val = tasks[needs].consume.row[j].value;

				for (unsigned i = 0; i < tasks[needed].produce.row.size(); i++)
				{
					unsigned index = tasks[needed].produce.row[i].index;
					unsigned value = tasks[needed].produce.row[i].value;
			
					if (idx == index)
					{
						d.w.set(idx, best_function_ever(value, val));
						d.bonus.set(idx, value);
					}

					if (resourceValueLost[index].get(idx))
						d.e.set(idx, 1.5);
				}
			}
			break;
		case RT_MAXIMUM:
			for (unsigned j = 0; j < tasks[needs].produce.row.size(); j++)
			{
				unsigned idx = tasks[needs].produce.row[j].index;
				unsigned val = tasks[needs].produce.row[j].value;

				for (unsigned i = 0; i < tasks[needed].produce.row.size(); i++)
				{
					unsigned index = tasks[needed].produce.row[i].index;
					unsigned value = tasks[needed].produce.row[i].value;
					
					unsigned max = resources[idx].maximum_per_resource.get(index);
					if (max)
					{
						d.bonus.set(idx, max);
						d.w.set(idx, best_function_ever(max, value));
					}

					for (unsigned k = 0; k < resources[idx].maximum_per_resource.row.size(); k++)
					{
						unsigned r_idx = resources[idx].maximum_per_resource.row[k].index;
						if (resourceValueLost[index].get(r_idx))
							d.e.set(idx, 1.5);
					}
				}
			}
			break;
		case RT_PREREQUISITE:
			for (unsigned j = 0; j < tasks[needs].prerequisite.row.size(); j++)
			{
				unsigned idx = tasks[needs].prerequisite.row[j].index;
				unsigned val = tasks[needs].prerequisite.row[j].value;

				for (unsigned i = 0; i < tasks[needed].produce.row.size(); i++)
				{
					unsigned index = tasks[needed].produce.row[i].index;
					unsigned value = tasks[needed].produce.row[i].value;

					if (idx == index)
						d.w.set(idx, 1);
					else if (resourceValueLost[index].get(idx))
						d.e.set(idx, 1);
				}
			}
			break;
	}
}

double BuildOrder::Rules::best_function_ever(double a, double b)
{
	return a * tanh(a/b);
}