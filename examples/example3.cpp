/***********************************************************************************************/
/**                                                                                           **/
/** EXAMPLE 3                                                                                 **/
/**                                                                                           **/
/** Forced time writing                                                                       **/
/**                                                                                           **/
/***********************************************************************************************/

#include <iostream>
#include "LightTSDB.h"

using namespace std;

int main()
{
    LightTSDB::LightTSDB myTSDB;
    int numberOfPeople;


    ///*** Write int value 10 min ago
    numberOfPeople = 10;
    myTSDB.WriteTimeValue("ShowRoomPeople", numberOfPeople, time(nullptr)-600);

    ///*** Write int value 5 min ago
    numberOfPeople = 15;
    myTSDB.WriteTimeValue("ShowRoomPeople", numberOfPeople, 300);

    ///*** Write int value for now
    numberOfPeople = 17;
    myTSDB.WriteValue("ShowRoomPeople", numberOfPeople);

    ///*** Clean up
    myTSDB.Close("ShowRoomPeople");
    myTSDB.Remove("ShowRoomPeople");

    return 0;
}
