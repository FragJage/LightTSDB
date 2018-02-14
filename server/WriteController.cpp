#include <string>
#include "WriteController.h"
#include "LtsdbFactory.h"
#include "LightTSDB.h"

using namespace std;
using namespace MongooseCpp;

WriteController::WriteController()
{
}

WriteController::~WriteController()
{
}

bool WriteController::Process(Request& request, Response& response)
{
    string folder = request.GetParameter("BaseName");
    string sensor = request.GetParameter("SensorId");

    float value = strtof(request.GetQueryParameter("Value").c_str(), nullptr);
    int timeOffset = atoi(request.GetQueryParameter("Offset").c_str());

    bool ok = false;
    if(timeOffset == 0)
        ok = LtsdbFactory::GetInstance(folder).WriteValue(sensor, value);
    else
        ok = LtsdbFactory::GetInstance(folder).WriteOldValue(sensor, value, timeOffset);

    response.SetStatut(200);
    if(!ok)
    {
        LightTSDB::ErrorInfo myError = LtsdbFactory::GetInstance(folder).GetLastError(sensor);
        response.SetStatut(404);
        response.SetContent(myError.ErrMessage+" "+myError.SysMessage);
    }

    return true;
}
