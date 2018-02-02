#ifndef TIMEMOCKABLE_H
#define TIMEMOCKABLE_H

extern time_t mocktime(time_t* ptr);

#include <ctime>

namespace MOCK {
	time_t time(time_t* ptr)
	{
#ifdef USE_MOCKS
		return mocktime(ptr);
#else
		return std::time(ptr);
#endif
	}
}
#endif