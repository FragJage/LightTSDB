#include <sstream>
#include "TimeHelper.h"

using namespace std;

string TimeHelper::ToString(time_t tTime)
{
    char buf[32];
	struct tm sTime;

	#ifdef _MSC_VER
		localtime_s(&sTime, &tTime);
	#else
		localtime_r(&tTime, &sTime);
	#endif
	
	strftime(buf, 32, "%Y-%m-%dT%H:%M:%S", &sTime);
    return string(buf);
}

time_t TimeHelper::ToTime(string strTime)
{
    struct tm t;
    #ifdef WIN32
    istringstream istr(strTime);
    istr >> t.tm_year;
    istr.ignore();
    istr >> t.tm_mon;
    istr.ignore();
    istr >> t.tm_mday;
    istr.ignore();
    istr >> t.tm_hour;
    istr.ignore();
    istr >> t.tm_min;
    istr.ignore();
    istr >> t.tm_sec;
    t.tm_year -= 1900;
    t.tm_mon--;
    t.tm_isdst = 0;
    #else
    strptime(strTime.c_str(), "%FT%T%z", &t);
    #endif // WIN32
    return mktime(&t);
}
