#include <sstream>
#include "ReadController.h"
#include "LtsdbFactory.h"
#include "TimeHelper.h"

using namespace std;
using namespace MongooseCpp;

ReadController::ReadController()
{
    //ctor
}

ReadController::~ReadController()
{
    //dtor
}

bool ReadController::Process(Request& request, Response& response)
{
    string folder = request.GetParameter("BaseName");
    string sensor = request.GetParameter("SensorId");
    if(sensor=="") return SensorsList(folder, response);

    string strStart = request.GetQueryParameter("Start");
    string strEnd = request.GetQueryParameter("End");
    string strInt = request.GetQueryParameter("Interval");

    bool ok = false;
    time_t tStart;
    time_t tEnd = 0;
    int interval = atoi(strInt.c_str());
    list<LightTSDB::DataValue> datas;

    if(strStart=="")
    {
        response.SetStatut(500);
        return true;
    }
    tStart = TimeHelper::ToTime(strStart);

    if(strEnd!="") tEnd = TimeHelper::ToTime(strEnd);

    if(tEnd==0)
        ok = LtsdbFactory::GetInstance(folder).ReadValues(sensor, tStart, datas);
    else if(interval > 0)
        ok = LtsdbFactory::GetInstance(folder).ResampleValues(sensor, tStart, tEnd, datas, interval);
    else
        ok = LtsdbFactory::GetInstance(folder).ReadValues(sensor, tStart, tEnd, datas);


    if(!ok)
    {
        LightTSDB::ErrorInfo myError = LtsdbFactory::GetInstance(folder).GetLastError(sensor);
        response.SetStatut(404);
        response.SetContent(myError.ErrMessage+" "+myError.SysMessage);
        return true;
    }

    list<LightTSDB::DataValue>::const_iterator it, itEnd;
    ostringstream ss;
    it = datas.begin();
    itEnd = datas.end();

    while(it!=itEnd)
    {
        ss << TimeHelper::ToString(it->time) << " : " << it->value.Float << endl;
        it++;
    }
    response.SetStatut(200);
    response.SetContent(ss.str());

    return true;
}

bool ReadController::SensorsList(string folder, Response& response)
{
    list<string> sensorList;

    if(LtsdbFactory::GetInstance(folder).GetSensorList(sensorList))
    {
        list<string>::const_iterator it = sensorList.begin();
        list<string>::const_iterator itEnd = sensorList.end();
        ostringstream ss;
        while(it!=itEnd)
        {
            ss << *it << endl;
            ++it;
        }
        response.SetStatut(200);
        response.SetContent(ss.str());
    }
    else
    {
        response.SetStatut(404);
    }

    return true;
}
