/*** LICENCE ***************************************************************************************/
/*
  LightTSDB - Light time series database
  
  This file is part of LightTSDB server.
  
    LightTSDB is free software : you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
	
    LightTSDB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
	
    You should have received a copy of the GNU General Public License
    along with LightTSDB.  If not, see <http://www.gnu.org/licenses/>.
*/
/***************************************************************************************************/
#include <sstream>
#include <cstring> //for memset
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

    memset(&t, 0, sizeof(tm));
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
