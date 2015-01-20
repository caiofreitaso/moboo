#ifndef ARCHIVERSH
#define ARCHIVERSH

#include "optimizer.h"

namespace BuildOrder
{
	namespace Optimizer
	{
		class Archiver
		{
		protected:
			Population _data;
			const Optimizer* _optimizer;
			unsigned _capacity;
		public:
			Archiver() : _optimizer(0), _capacity(0) { }

			virtual void filter(Population&) const = 0;

			void setOptimizer(Optimizer* o) { _optimizer = o; }
			void setCapacity(unsigned c) { _capacity = c; }
			Population operator()() const { return _data; }
			Solution operator[](unsigned i) const { return _data[i]; }
			unsigned size() const { return _data.size(); }
			Population::const_iterator begin() const { return _data.begin(); }
			Population::const_iterator end() const { return _data.end(); }

			virtual void insert(Solution a)
			{
				for (unsigned i = 0; i < size(); i++)
					if (_data[i] == a)
						return;

				_data.push_back(a);

				filter(_data);
				
				if (_data.size() > _capacity)
					_data.erase(_data.begin()+_capacity, _data.end());
			}

			void unsafeInsert(Solution a) { _data.push_back(a); }
			void unsafeFilter() { filter(_data); }
		};
	}
}


#endif