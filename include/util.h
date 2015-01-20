#ifndef UTILH
#define UTILH

#include <vector>

template<class T>
void pop(std::vector<T>& v, unsigned i = 0)
{
	v.erase(v.begin()+i);
}

#endif