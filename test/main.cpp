#include <iostream>
#include <random>
#include <chrono>
#include "LightTSDB.h"
#include "TimeMock.h"

using namespace std;

//Add Header : Signature, Version, Type, Options and State on index and data files
//Add uvw (wrapper for libuv)
//Add compression
//Add Wrtie Cache and Flush ?
//Add Read Cache ?
//Add read functions
//Tool for check data file
//Tool for rebuild index file

int main()
{
    string sensor = "LucileBedRoomTemperature";
    int i;
    LightTSDB::LightTSDB myTSDB;
    float myTemp = 21.0;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> rdSecond(15, 45);
    uniform_real_distribution<float> rdTemperature(-0.5, 0.5);


    SetMockTime(2017, 10, 25, 12, 10, 42);

    auto t0 = chrono::high_resolution_clock::now();
    for(i=0; i<1000000; i++)
    {
        MockAddSecond(rdSecond(gen));
        myTemp += rdTemperature(gen);
        if(!myTSDB.WriteValue(sensor, myTemp))
        {
            cout << "Failed to write : " << myTSDB.GetLastError(sensor).ErrMessage << endl;
            break;
        }
    }
    auto t1 = chrono::high_resolution_clock::now();
    chrono::duration<float> fs = t1 - t0;

    cout << "Write " << i << " temperatures in " << fs.count() << " s." << endl;
    return 0;
}
