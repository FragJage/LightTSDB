/***********************************************************************************************/
/**                                                                                           **/
/** EXAMPLE 2                                                                                 **/
/**                                                                                           **/
/** Read multiples values in LightTSDB                                                        **/
/**                                                                                           **/
/***********************************************************************************************/

#include <iostream>
#include "LightTSDB.h"

using namespace std;

int main()
{
    struct tm tmtime;
    time_t start = 1462690800;
    LightTSDB::LightTSDB myTSDB;
    LightTSDB::ErrorInfo myError;
    list<LightTSDB::DataValue> myValues;


    myTSDB.SetFolder("test/data");
    if(!myTSDB.ReadValues("LucileBedRoomTemperature", start, myValues))
    {
        myError = myTSDB.GetLastError("LucileBedRoomTemperature");
        cout << "ERROR" << endl << myError.ErrMessage << endl << myError.SysMessage << endl;
        return -1;
    }

    localtime_r(&start, &tmtime);
    cout << "All data for the day " << 1900+tmtime.tm_year << "/" << tmtime.tm_mon << "/" << tmtime.tm_mday << endl;
    for(list<LightTSDB::DataValue>::const_iterator it=myValues.begin(); it!=myValues.end(); ++it)
    {
        localtime_r(&(it->time), &tmtime);
        cout << tmtime.tm_hour << ":" << tmtime.tm_min << ":" << tmtime.tm_sec << " => " << it->value.Float << endl;
    }

    return 0;
}
