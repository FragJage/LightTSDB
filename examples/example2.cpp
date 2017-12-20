/***********************************************************************************************/
/**                                                                                           **/
/** EXAMPLE 2                                                                                 **/
/**                                                                                           **/
/** Read multiples values in LightTSDB                                                        **/
/**                                                                                           **/
/***********************************************************************************************/

#include <iostream>
#include <iomanip>
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

    localtime_r(&start, &tmtime);
    cout << setfill('0');
    cout << "Read data for the day " << 1900+tmtime.tm_year << "/" << setw(2) << tmtime.tm_mon << "/" << setw(2) << tmtime.tm_mday;
    cout << " between " <<  setw(2) << tmtime.tm_hour << ":00 and " <<  setw(2) << tmtime.tm_hour << ":59" << endl;

    cout << endl;
    cout << "### All data ############################################" << endl;
    if(!myTSDB.ReadValues("LucileBedRoomTemperature", start, myValues))
    {
        myError = myTSDB.GetLastError("LucileBedRoomTemperature");
        cout << "ERROR" << endl << myError.ErrMessage << endl << myError.SysMessage << endl;
        return -1;
    }

    for(list<LightTSDB::DataValue>::const_iterator it=myValues.begin(); it!=myValues.end(); ++it)
    {
        localtime_r(&(it->time), &tmtime);
        cout << setw(2) << tmtime.tm_hour << ":" << setw(2) << tmtime.tm_min << ":" << setw(2) << tmtime.tm_sec;
        cout << " => " << it->value.Float << endl;
    }

    cout << endl;
    cout << "### Resample data to have a value for all 5 min #########" << endl;
    if(!myTSDB.ResampleValues("LucileBedRoomTemperature", start, start+3599, myValues, 300))
    {
        myError = myTSDB.GetLastError("LucileBedRoomTemperature");
        cout << "ERROR" << endl << myError.ErrMessage << endl << myError.SysMessage << endl;
        return -1;
    }

    for(list<LightTSDB::DataValue>::const_iterator it=myValues.begin(); it!=myValues.end(); ++it)
    {
        localtime_r(&(it->time), &tmtime);
        cout << setw(2) << tmtime.tm_hour << ":" << setw(2) << tmtime.tm_min << ":" << setw(2) << tmtime.tm_sec;
        cout << " => " << it->value.Float << endl;
    }

    return 0;
}
