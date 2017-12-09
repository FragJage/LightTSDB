#include <sstream>
#include "TestHourlyTimestamp.h"

using namespace std;

TestHourlyTimestamp::TestHourlyTimestamp() : TestClass("HourlyTimestamp", this)
{
	addTest("FromTimeT", &TestHourlyTimestamp::FromTimeT);
	addTest("ToTimeT", &TestHourlyTimestamp::ToTimeT);
	addTest("ToString", &TestHourlyTimestamp::ToString);
}

TestHourlyTimestamp::~TestHourlyTimestamp()
{
}

bool TestHourlyTimestamp::FromTimeT()
{
    struct tm timeinfo;
    time_t timeT;
    uint32_t nbHours;

    timeinfo.tm_year = 117;
    timeinfo.tm_mon = 7;
    timeinfo.tm_mday = 15;
    timeinfo.tm_hour = 5;
    timeinfo.tm_min = 0;
    timeinfo.tm_sec = 0;
    timeinfo.tm_isdst = 0;
    timeT = mktime(&timeinfo);

    nbHours = timeT/3600;
    assert(nbHours==LightTSDB::HourlyTimestamp::FromTimeT(timeT));

    return true;
}

bool TestHourlyTimestamp::ToTimeT()
{
    LightTSDB::HourlyTimestamp_t hourTS = 419558;   // 419558*3600 = 1510408800 in secondes since 1970/01/01
    time_t timeT;


    timeT = LightTSDB::HourlyTimestamp::ToTimeT(hourTS);
    assert(1510408800==timeT);

    return true;
}

bool TestHourlyTimestamp::ToString()
{
    LightTSDB::HourlyTimestamp_t hourTS = 418651;   // => 2017/10/04 21h00
    time_t ttime = LightTSDB::HourlyTimestamp::ToTimeT(hourTS);
    struct tm stime;
    ostringstream oss;

    localtime_r(&ttime, &stime);
    oss << "2017/10/4 " << stime.tm_hour << "h";

    assert(oss.str()==LightTSDB::HourlyTimestamp::ToString(hourTS));

    return true;
}
