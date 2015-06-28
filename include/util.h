#ifndef UTILH
#define UTILH

#include "contiguous.h"

template<class T>
void pop(contiguous<T>& v, unsigned i = 0)
{
	v.erase(v.begin()+i);
}

#endif