#include <iostream>
#include <random>
#include <chrono>
#include <vector>
#include "LightTSDB.h"
#include "TimeMock.h"
#include "TestLtsdbFile.h"
#include "TestHourlyTimestamp.h"
#include "TestLightTSDB.h"
#include "TestOtherTypes.h"

using namespace std;

//TO DO List
//Read/Write an others types double, bool, int ...
//Add compression
//Use state flag
//Add uvw (wrapper for libuv)
//Add Write Cache and Flush ?
//Add Read Cache ?
//Tool for check data file
//Tool for rebuild index file
//Tool for compress or uncompress file

vector<int> RandomIntervalTime;
vector<time_t> RandomValuesTime;
vector<float> RandomValuesTemp;
vector<int> RandomOffset;
int RandomReadSize = 100;
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
            cout << termcolor::lightRed << "Failed to write : " << myTSDB.GetLastError(SensorName).ErrMessage << endl;
            break;
        }
        nbok++;
    }
    auto t1 = chrono::high_resolution_clock::now();
    chrono::duration<float> fs = t1 - t0;
    cout << termcolor::lightWhite << "    Write " << nbok << " values              " << termcolor::lightGreen << fs.count() << " s." << endl;
}

void MeasureContiguousReading()
{
    LightTSDB::LightTSDB myTSDB;
    list<LightTSDB::DataValue> readValues;

    auto t0 = chrono::high_resolution_clock::now();
    if(!myTSDB.ReadValues(SensorName, RandomValuesTime[0], RandomValuesTime[RandomValuesTime.size()-1], readValues))
    {
        cout << termcolor::lightRed << "ContiguousReading failed : " << myTSDB.GetLastError(SensorName).ErrMessage << endl;
        return;
    }
    auto t1 = chrono::high_resolution_clock::now();
    chrono::duration<float> fs = t1 - t0;

    int i = 0;
    for (auto& x: readValues)
    {
        if((x.time != RandomValuesTime[i])||(x.value.Float != RandomValuesTemp[i]))
        {
            cout << endl << termcolor::lightRed << "Error ContiguousReading : index " << i << endl;
            cout << RandomValuesTime[i] << "->" << RandomValuesTemp[i] << endl;
            break;
        }
        i++;
    }
    cout << termcolor::lightWhite<< "    Contiguous read " << i << " values    " << termcolor::lightGreen<< fs.count() << " s." << endl;
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
            cout << termcolor::lightRed << "RandomReading failed : " << myTSDB.GetLastError(SensorName).ErrMessage << endl;
            return;
        }
        auto t1 = chrono::high_resolution_clock::now();
        fs += (t1 - t0);

        for (auto& x: readValues)
        {
            if((x.time != RandomValuesTime[i])||(x.value.Float != RandomValuesTemp[i]))
            {
                cout << endl << termcolor::lightRed << "Error RandomReading : index " << i << endl;
                cout << RandomValuesTime[i] << "->" << RandomValuesTemp[i] << endl;
                break;
            }
            i++;
            nbok++;
        }
        ++it;
    }

    cout << termcolor::lightWhite << "    Random read " << nbok/RandomReadSize << "x" << RandomReadSize << " values      " << termcolor::lightGreen << fs.count() << " s." << endl;
}

void CleanUp()
{
    LightTSDB::LightTSDB myTSDB;
    myTSDB.Remove(SensorName);
}

/// Intel Core i7 - SSD - Mingw : 0.469 - 0.266 - 0.359
/// Celeron G540  - SSD - GCC   : 0.272 - 0.179 - 0.292

int main()
{
/*
    cout << termcolor::lightYellow << "- Speed measurement ---------------" << endl;
    BuildRandomValues();
    MeasureWritingTime();
    MeasureContiguousReading();
    MeasureRandomReading();
    CleanUp();
    cout << endl;
*/
    int ret = 0;
    UnitTest unitTest;

    try
    {
        unitTest.addTestClass(new TestLtsdbFile());
        unitTest.addTestClass(new TestHourlyTimestamp());
        unitTest.addTestClass(new TestLightTSDB());
        unitTest.addTestClass(new TestOtherTypes());
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
