#include <iostream>
#include <random>
#include <chrono>
#include "LightTSDB.h"
#include "TimeMock.h"

using namespace std;

//Add SetFolder,close,Unlink Methods
//Add uvw (wrapper for libuv)
//Add read functions
//Add compression
//Add Wrtie Cache and Flush ?
//Add Read Cache ?
//Tool for check data file
//Tool for rebuild index file

int main()
{
    string sensor = "LucileBedRoomTemperature";
    int i,j,nb;
    LightTSDB::LightTSDB myTSDB;
    float myTemp = 21.0;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> rdSecond(15, 45);
    uniform_real_distribution<float> rdTemperature(-0.5, 0.5);


    SetMockTime(2017, 10, 25, 12, 10, 42);

    auto t0 = chrono::high_resolution_clock::now();
    nb = 0;
    for(j=0; j<3; j++)
    {
        for(i=0; i<3; i++)
        {
            MockAddSecond(rdSecond(gen));
            myTemp += rdTemperature(gen);
            if(!myTSDB.WriteValue(sensor, myTemp))
            {
                cout << "Failed to write : " << myTSDB.GetLastError(sensor).ErrMessage << endl;
                break;
            }
            else
            {
                nb++;
            }
        }
        myTSDB.Close(sensor);
    }
    auto t1 = chrono::high_resolution_clock::now();
    chrono::duration<float> fs = t1 - t0;
    myTSDB.Remove(sensor);

    cout << "Write " << nb << " temperatures in " << fs.count() << " s." << endl;
    return 0;
}
