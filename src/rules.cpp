#include "../include/rules.h"

std::vector<BuildOrder::Rules::Task> BuildOrder::Rules::tasks;
std::vector<BuildOrder::Rules::Event> BuildOrder::Rules::events;
std::vector<BuildOrder::Rules::Resource> BuildOrder::Rules::resources;

std::vector<MatrixRow<unsigned> > BuildOrder::Rules::taskProduceByEvent;
std::vector<MatrixRow<unsigned> > BuildOrder::Rules::taskValuePerEvent;
std::vector<MatrixRow<unsigned> > BuildOrder::Rules::resourceValueLost;

unsigned BuildOrder::Rules::mean_time = 0;

bool BuildOrder::Rules::fillsPrerequisite(unsigned task, unsigned resource, unsigned amount)
{ return tasks[task].prerequisite.get(resource) <= amount; }

bool BuildOrder::Rules::fillsBorrow(unsigned task, unsigned resource, unsigned amount)
{ return tasks[task].borrow.get(resource) <= amount; }

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

		taskProduceByEvent.reserve(tks_sz);

		taskValuePerEvent.reserve(res_sz);
		resourceValueLost.reserve(res_sz);

		for (unsigned i = 0; i < tasks.capacity(); i++)
		{
			tasks.push_back(Task());
			taskProduceByEvent.push_back(MatrixRow<unsigned>());
		}

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
	if (events.size())
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
				{
					taskValuePerEvent[index].set(k, 1);
					taskProduceByEvent[k].set(index, 1);
				}
		}
}