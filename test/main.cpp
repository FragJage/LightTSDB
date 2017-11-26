#include <iostream>
#include "LightTSDB.h"

using namespace std;


int main()
{
    LightTSDB myTSDB;



    if(!myTSDB.WriteValue("LucileBedRoomTemperature", 21.8))
        cout << "Failed to write : " << myTSDB.GetLastError() << endl;
    else
        cout << "Write success." << endl;

    return 0;
}
