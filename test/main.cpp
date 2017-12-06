#include <iostream>
#include <random>
#include <chrono>
#include <vector>
#include "LightTSDB.h"
#include "TimeMock.h"
#include "TestLtsdbFile.h"

using namespace std;

//TO DO List
//Add resampling function
//Add write value with time offset
//Add uvw (wrapper for libuv)
//Add compression
//Add Write Cache and Flush ?
//Add Read Cache ?
//Use state flag
//Read/Write an others types
//Tool for check data file
//Tool for rebuild index file
//Tool for compress or uncompress file

vector<int> RandomIntervalTime;
vector<time_t> RandomValuesTime;
vector<float> RandomValuesTemp;
vector<int> RandomOffset;
int RandomReadSize = 500;
string SensorName = "LucileBedRoomTemperature";

void BuildRandomValues()
{
    int nbmax = 1000000;
    float myTemp = 21.0;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> rdSecond(15, 45);
    uniform_real_distribution<float> rdTemperature(-0.5, 0.5);
    uniform_int_distribution<int> rdOffest(0, nbmax-RandomReadSize);

    SetMockTime(2017, 11, 25, 13, 24, 36);
    for(int i=0; i<nbmax; i++)
    {
        RandomIntervalTime.emplace_back(rdSecond(gen));
        myTemp += rdTemperature(gen);
        RandomValuesTemp.emplace_back(myTemp);
        MockAddSecond(RandomIntervalTime[i]);
        RandomValuesTime.emplace_back(time(0));
    }
    nbmax = (nbmax/RandomReadSize);
    for(int i=0; i<nbmax; i++)
    {
        RandomOffset.emplace_back(rdOffest(gen));
    }

    SetMockTime(2017, 11, 25, 13, 24, 36);
}

void MeasureWritingTime()
{
    int i,nbok,nbmax;
    LightTSDB::LightTSDB myTSDB;

    auto t0 = chrono::high_resolution_clock::now();
    nbok = 0;
    nbmax = RandomValuesTemp.size();
    for(i=0; i<nbmax; i++)
    {
        MockAddSecond(RandomIntervalTime[i]);
        if(!myTSDB.WriteValue(SensorName, RandomValuesTemp[i]))
        {
            cout << "Failed to write : " << myTSDB.GetLastError(SensorName).ErrMessage << endl;
            break;
        }
        nbok++;
    }
    auto t1 = chrono::high_resolution_clock::now();
    chrono::duration<float> fs = t1 - t0;
    cout << "Write " << nbok << " temperatures in " << fs.count() << " s." << endl;
}

void MeasureContiguousReading()
{
    LightTSDB::LightTSDB myTSDB;
    list<LightTSDB::DataValue> readValues;

    auto t0 = chrono::high_resolution_clock::now();
    if(!myTSDB.ReadValues(SensorName, RandomValuesTime[0], RandomValuesTime[RandomValuesTime.size()-1], readValues))
    {
        cout << "ContiguousReading failed : " << myTSDB.GetLastError(SensorName).ErrMessage << endl;
        return;
    }
    auto t1 = chrono::high_resolution_clock::now();
    chrono::duration<float> fs = t1 - t0;

    int i = 0;
    for (auto& x: readValues)
    {
        if((x.time != RandomValuesTime[i])||(x.value != RandomValuesTemp[i]))
        {
            cout << endl << "Error ContiguousReading : index " << i << endl;
            cout << RandomValuesTime[i] << "->" << RandomValuesTemp[i] << endl;
            break;
        }
        i++;
    }
    cout << "Contiguous read " << i << " temperatures in " << fs.count() << " s." << endl;
}

void MeasureRandomReading()
{
    LightTSDB::LightTSDB myTSDB;
    list<LightTSDB::DataValue> readValues;
    vector<int>::const_iterator it, itEnd;
    chrono::duration<float> fs;
    int i, nbok=0;

    it = RandomOffset.begin();
    itEnd = RandomOffset.end();
    while(it!=itEnd)
    {
        auto t0 = chrono::high_resolution_clock::now();
        i = *it;
        if(!myTSDB.ReadValues(SensorName, RandomValuesTime[i], RandomValuesTime[i+RandomReadSize-1], readValues))
        {
            cout << "RandomReading failed : " << myTSDB.GetLastError(SensorName).ErrMessage << endl;
            return;
        }
        auto t1 = chrono::high_resolution_clock::now();
        fs += (t1 - t0);

        for (auto& x: readValues)
        {
            if((x.time != RandomValuesTime[i])||(x.value != RandomValuesTemp[i]))
            {
                cout << endl << "Error RandomReading : index " << i << endl;
                cout << RandomValuesTime[i] << "->" << RandomValuesTemp[i] << endl;
                break;
            }
            i++;
            nbok++;
        }
        ++it;
    }

    cout << "Random read " << nbok << " temperatures in " << fs.count() << " s." << endl;
}

void CleanUp()
{
    LightTSDB::LightTSDB myTSDB;
    myTSDB.Remove(SensorName);
}

/// Core i7 - SSD - Mingw : 0.469 - 0.266 - 0.359

int main()
{
    BuildRandomValues();
    MeasureWritingTime();
    MeasureContiguousReading();
    MeasureRandomReading();
    CleanUp();

    int ret = 0;
    UnitTest unitTest;

    try
    {
        unitTest.addTestClass(new TestLtsdbFile());
    }
    catch(const exception &e)
    {
        unitTest.displayError(e.what());
        ret = -1;
    }

    if(ret!=-1)
        if(!unitTest.run()) ret = 1;

    return ret;
}
