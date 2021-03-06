#include "../include/buildorder.h"

void BuildOrder::print(BuildOrder const& b)
{
	for (unsigned k = 0; k < b.size(); k++)
	{
		std::cout << b[k].task;
		if (b[k].delay)
			std::cout << "@" << b[k].delay;
		if (k < b.size()-1)
			std::cout << ", ";
		else
			std::cout << "\n";
	}
}

void BuildOrder::createState(GameState& s)
{
	if (Rules::tasks.size()) {
		s.resources.resize(Rules::resources.size());
		for (unsigned i = 0; i < Rules::resources.size(); i++) {
			s.resources[i].quantity = 0;
			s.resources[i].used = 0;
			s.resources[i].borrowed = 0;

			s.resources[i].capacity = Rules::resources[i].overall_maximum > 0;
		}

		s.time = 0;
	} else
		s.time = -1;
}

void BuildOrder::initState(GameState& s, char const* f)
{
	std::fstream file;
	std::string buffer;
	std::stringstream ss;

	file.open(f);

	unsigned idx;
	unsigned v;

	do
		std::getline(file,buffer);
	while(buffer[0] == '#' || buffer.size() == 0);

	ss.clear();
	ss.str(buffer);

	for(unsigned j,v; !ss.eof(); )
	{
		long p = (long)ss.tellg();
		
		ss >> j;
		while (ss.fail())
		{
			ss.clear();
			ss.seekg(p+1);
			ss >> j;
		}
		
		p = (long)ss.tellg();
		ss >> v;
		while (ss.fail())
		{
			ss.clear();
			ss.seekg(p+1);
			ss >> v;
		}

		s.resources[j].quantity += v;

		for (unsigned k = 0; k < Rules::events.size(); k++)
			if (Rules::events[k].trigger == j)
				for (unsigned q = 0; q < v; q++)
				{
					if (Rules::events[k].time)
						s.resources[j].events.push_back(EventPointer(s.time, k));
					else
						s.applyEvent(k);
				}
	}

	std::getline(file,buffer);
	ss.clear();
	ss.str(buffer);
	for(unsigned j,v; !ss.eof(); )
	{
		long p = (long)ss.tellg();
		
		ss >> j;
		while (ss.fail())
		{
			ss.clear();
			ss.seekg(p+1);
			ss >> j;
		}
		
		p = (long)ss.tellg();
		ss >> v;
		while (ss.fail())
		{
			ss.clear();
			ss.seekg(p+1);
			ss >> v;
		}

		for (unsigned i = 0; i < v; i++)
			s.consumeResource(j);
	}

	file.close();
}