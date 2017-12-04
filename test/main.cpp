#include <iostream>
#include <random>
#include <chrono>
#include <vector>
#include "LightTSDB.h"
#include "TimeMock.h"

using namespace std;

//TO DO List
//Add read functions
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



    auto t0 = chrono::high_resolution_clock::now();
    nb = 0;
    for(j=0; j<1000; j++)
    {
        for(i=0; i<1000; i++)
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
    cout << "Write " << nb << " temperatures in " << fs.count() << " s." << endl;


    int automate = 1;
    LightTSDB::HourlyTimestamp_t hourlyTimestamp = 0;
    vector<LightTSDB::DataValue> valuesOri;
    list<LightTSDB::DataValue> valuesDst;

    nb=0;
    int bcl=16;
    for(j=0; j<bcl; j++)
    {
        if(j==bcl/4) myTSDB.Close(sensor);
        for(i=0; i<1000; i++)
        {
            MockAddSecond(rdSecond(gen));
            myTemp += rdTemperature(gen);

            switch(automate)
            {
                case 1 :
                    if((j==bcl/2)&&(i==0))
                    {
                        automate++;
                        hourlyTimestamp = LightTSDB::HourlyTimestamp::FromTimeT(time(0));
                    }
                    break;
                case 2 :
                    if(hourlyTimestamp != LightTSDB::HourlyTimestamp::FromTimeT(time(0)))
                    {
                        automate++;
                        hourlyTimestamp = LightTSDB::HourlyTimestamp::FromTimeT(time(0));
                        time_t tt;
                        time(&tt);
                        cout << "Memo : " << LightTSDB::HourlyTimestamp::ToString(hourlyTimestamp) << endl;
                        //cout << tt <<"->"<< myTemp << " ";
                        valuesOri.emplace_back(tt, myTemp);
                        nb++;
                    }
                    break;
                case 3 :
                    if(hourlyTimestamp == LightTSDB::HourlyTimestamp::FromTimeT(time(0)))
                    {
                        time_t tt;
                        time(&tt);
                        valuesOri.emplace_back(tt, myTemp);
                        //cout << tt <<"->"<< myTemp << " ";
                        nb++;
                    }
                    else
                    {
                        automate++;
                        cout << "Nb values " << nb << endl;
                    }
                    break;
            }
            if(!myTSDB.WriteValue(sensor, myTemp))
            {
                cout << "Failed to write : " << myTSDB.GetLastError(sensor).ErrMessage << endl;
                break;
            }
        }
    }

    if(!myTSDB.ReadValues(sensor, LightTSDB::HourlyTimestamp::ToTimeT(hourlyTimestamp), valuesDst))
    {
        cout << "Failed to read1 : " << myTSDB.GetLastError(sensor).ErrMessage << endl;
    }
    else
    {
        cout << "Nb read " << valuesDst.size() << endl;
        i = 0;
        for (auto& x: valuesDst)
        {
            if((x.time != valuesOri[i].time)||(x.value != valuesOri[i].value))
            {
                cout << endl << "Error reading1 : index " << i << endl;
                cout << valuesOri[i].time << "->" << valuesOri[i].value << endl;
                break;
            }
            i++;
        }
        cout << i << " good reads" << endl;
    }
    myTSDB.Close(sensor);
    if(!myTSDB.ReadValues(sensor, LightTSDB::HourlyTimestamp::ToTimeT(hourlyTimestamp), valuesDst))
    {
        cout << "Failed to read2 : " << myTSDB.GetLastError(sensor).ErrMessage << endl;
    }
    else
    {
        i = 0;
        for (auto& x: valuesDst)
        {
            if((x.time != valuesOri[i].time)||(x.value != valuesOri[i].value))
            {
                cout << "Error reading2 : index " << i << endl;
                break;
            }
            i++;
        }
        cout << i << " good reads" << endl;
    }

    myTSDB.Remove(sensor);

    return 0;
}
