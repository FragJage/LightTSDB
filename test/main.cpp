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
#include "TestTools.h"

using namespace std;

//TO DO List
//Use state flag
//Tool for check data file
//Tool for repair data file

///                                  write   read    random read
/// Intel Core i7 - SSD - MSVC     : 0.332 - 0.198 - 0.382
/// Intel Core i7 - SSD - Mingw    : 1.647 - 0.271 - 0.515
/// Intel Celeron G540 - SSD - GCC : 0.179 - 0.167 - 0.296
/// AppVeyor Plateform             : 0.390 - 0.245 - 0.459
/// Travis Platform                : 0.286 - 0.165 - 0.230

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

    SetMockTime(2015, 11, 25, 13, 24, 36);
    for(int i=0; i<nbmax; i++)
    {
        RandomIntervalTime.emplace_back(rdSecond(gen));
        myTemp += rdTemperature(gen);
        RandomValuesTemp.emplace_back(myTemp);
        MockAddSecond(RandomIntervalTime[i]);
        RandomValuesTime.emplace_back(MOCK::time(0));
    }
    nbmax = (nbmax/RandomReadSize);
    for(int i=0; i<nbmax; i++)
    {
        RandomOffset.emplace_back(rdOffest(gen));
    }

    SetMockTime(2015, 11, 25, 13, 24, 36);
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
    chrono::duration<float> fs(0);
    int nbok=0;

    it = RandomOffset.begin();
    itEnd = RandomOffset.end();
    while(it!=itEnd)
    {
        auto t0 = chrono::high_resolution_clock::now();
        int i = *it;
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

    cout << termcolor::lightWhite << "    Random read ";
    cout << nbok/RandomReadSize << "x" << RandomReadSize;
    cout << " values      " << termcolor::lightGreen << fs.count() << " s." << endl;
}

void CleanUp()
{
    LightTSDB::LightTSDB myTSDB;
    myTSDB.Remove(SensorName);
}

int main()
{
    cout << termcolor::lightYellow << "- Speed measurement ---------------" << endl;
    BuildRandomValues();
    MeasureWritingTime();
    MeasureContiguousReading();
    MeasureRandomReading();
    CleanUp();
    cout << endl;

    int ret = 0;
    UnitTest unitTest;

    try
    {
        unitTest.addTestClass(new TestLtsdbFile());
        unitTest.addTestClass(new TestHourlyTimestamp());
        unitTest.addTestClass(new TestLightTSDB());
        unitTest.addTestClass(new TestOtherTypes());
        unitTest.addTestClass(new TestTools());
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
