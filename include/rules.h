#ifndef RULESH
#define RULESH

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include "matrix.h"
#include <set>

namespace BuildOrder
{
	namespace Rules
	{
		struct Task {
			unsigned time;
			MatrixRow<int> costs;

			MatrixRow<unsigned> prerequisite;
			MatrixRow<unsigned> consume;
			MatrixRow<unsigned> borrow;

			MatrixRow<unsigned> produce;
		};

		struct Event {
			unsigned time;

			MatrixRow<unsigned> bonus;

			unsigned trigger;
		};

		struct Resource {
			unsigned overall_maximum;

			MatrixRow<unsigned> maximum_per_resource;
			MatrixRow<unsigned> equivalence;
		};

		enum relation_type
		{
			RT_COST,
			RT_CONSUME,
			RT_MAXIMUM,
			RT_PREREQUISITE
		};

		struct Dependency {
			unsigned task;
			mutable MatrixRow<double> bonus;
			mutable MatrixRow<double> w;
			mutable MatrixRow<double> e;

			bool operator<(Dependency const& d) const
			{ return task < d.task; }
		};

		void value(unsigned, unsigned, relation_type, Dependency const&);
		inline double best_function_ever(double, double);

		struct Item {
			unsigned task;
			mutable std::set<Dependency> dependencies;

			bool operator<(Item const& d) const
			{ return task < d.task; }
		};

		extern std::vector<Task> tasks;
		extern std::vector<Event> events;
		extern std::vector<Resource> resources;

		extern std::vector<MatrixRow<unsigned> > taskValuePerEvent;
		extern std::vector<MatrixRow<unsigned> > resourceValueLost;

		class Forest {
		public:
			typedef typename std::set<Item>::const_iterator c_it;
			typedef typename std::set<Dependency>::const_iterator d_it;
		private:
			std::set<Item> _data;
			relation_type _t;
			std::vector<c_it> _headers;

			void add(unsigned t, unsigned d)
			{
				Dependency dy;
				Item i;

				dy.task = d;
				value(d,t,_t,dy);
				
				if (dy.w.row.size() == 0 && dy.e.row.size() == 0)
					return;

				i.task = t;
				auto ins = _data.insert(i);
				ins.first->dependencies.insert(dy);
			}

			void getHeader(c_it i, std::set<unsigned>& done)
			{
				if (done.find(i->task) != done.end())
					return;
				
				done.insert(i->task);

				unsigned count = 0;
				for (c_it j = begin(); j != end(); j++)
					for (d_it k = j->dependencies.cbegin(); k != j->dependencies.cend(); k++)
						if (k->task == i->task)
							getHeader(j, done);

				if (!count)
					_headers.push_back(i);
			}

			void getHeaders()
			{
				std::set<unsigned> done;

				for (c_it i = begin(); i != end(); i++)
					getHeader(i, done);

			}
		public:
			void create(relation_type t)
			{
				_t = t;

				for (unsigned i = 0; i < tasks.size(); i++)
					for (unsigned j = 0; j < tasks.size(); j++)
						if (i != j)
							add(i,j);

				if (t == RT_PREREQUISITE)
					getHeaders();
			}

			unsigned hsize() const
			{ return _headers.size(); }

			c_it operator[](unsigned i) const
			{ return _headers[i]; }

			c_it begin() const
			{ return _data.cbegin(); }

			c_it end() const
			{ return _data.cend(); }

			c_it find(unsigned t) const
			{
				Item i;
				i.task = t;

				return _data.find(i);
			}

			void print() const
			{
				c_it itt;
				d_it it2;
				for (itt = begin(); itt != end(); itt++)
				{
					std::cout << "[" << itt->task << "]\n";
					for (it2 = itt->dependencies.cbegin(); it2 != itt->dependencies.cend(); it2++)
					{
						std::cout << "\t" << it2->task;
						if (it2->w.row.size())
							std::cout << "\t[";
						for (unsigned i = 0; i < it2->w.row.size(); i++)
						{
							std::cout << it2->w.row[i].index << "@" << it2->w.row[i].value;
							if (i == it2->w.row.size()-1)
								std::cout << "]\n";
							else
								std::cout << ",";
						}
						if (it2->e.row.size())
						{
							if (it2->w.row.size())
								std::cout << "\t\t{";
							else
								std::cout << "\t{";
						}
						for (unsigned i = 0; i < it2->e.row.size(); i++)
						{
							std::cout << it2->e.row[i].index << "@" << it2->e.row[i].value;
							if (i == it2->e.row.size()-1)
								std::cout << "}\n";
							else
								std::cout << ",";
						}
					}
				}
			}
		};

		extern Forest prerequisites;
		extern Forest maxima;
		extern Forest costs;
		extern Forest consumes;

		extern unsigned mean_time;


		void init(char const* filename);
	}
}

#endif