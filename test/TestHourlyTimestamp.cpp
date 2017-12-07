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

    timeinfo.tm_year = 117;
    timeinfo.tm_mon = 7;
    timeinfo.tm_mday = 15;
    timeinfo.tm_hour = 5;
    timeinfo.tm_min = 0;
    timeinfo.tm_sec = 0;
    timeT = mktime(&timeinfo);

    // timeT/3600 = 417435
    assert(417435==LightTSDB::HourlyTimestamp::FromTimeT(timeT));

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

    assert("2017/10/4 21h"==LightTSDB::HourlyTimestamp::ToString(hourTS));

    return true;
}
