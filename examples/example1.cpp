/***********************************************************************************************/
/**                                                                                           **/
/** EXAMPLE 1                                                                                 **/
/**                                                                                           **/
/** Write values into LightTSDB                                                               **/
/** Read values from LightTSDB                                                                **/
/**                                                                                           **/
/***********************************************************************************************/

#include <iostream>
#include "LightTSDB.h"

using namespace std;

int main()
{
    LightTSDB::LightTSDB myTSDB;
    LightTSDB::DataValue myValue;


    ///*** Write float value
    float temperature = 22.35;
    myTSDB.WriteValue("BedRoomTemperature", temperature);

    ///*** Write bool value
    bool lightOn = true;
    myTSDB.WriteValue("BedRoomLight", lightOn);

    ///*** Read float value
    myTSDB.ReadLastValue("BedRoomTemperature", myValue);
    cout << "BedRoom Temperature : " << myValue.value.Float << endl;

    ///*** Read bool value
    myTSDB.ReadLastValue("BedRoomLight", myValue);
    cout << "BedRoom Light : " << myValue.value.Bool << endl;

    ///*** Clean up
    myTSDB.Close("BedRoomTemperature");
    myTSDB.Close("BedRoomLight");
    myTSDB.Remove("BedRoomTemperature");
    myTSDB.Remove("BedRoomLight");

    return 0;
}
