#include <sstream>
#include "LastController.h"
#include "LtsdbFactory.h"
#include "TimeHelper.h"

using namespace std;
using namespace MongooseCpp;

LastController::LastController()
{
    //ctor
}

LastController::~LastController()
{
    //dtor
}

bool LastController::Process(Request& request, Response& response)
{
    string folder = request.GetParameter("BaseName");
    string sensor = request.GetParameter("SensorId");

    LightTSDB::LightTSDB myTSDB = LtsdbFactory::GetInstance(folder);
    LightTSDB::DataValue data;

    if(myTSDB.ReadLastValue(sensor, data))
    {
        ostringstream ss;
        ss << TimeHelper::ToString(data.time) << " : " << data.value.Float << endl;
        response.SetStatut(200);
        response.SetContent(ss.str());
    }
    else
    {
        LightTSDB::ErrorInfo myError = myTSDB.GetLastError(sensor);
        response.SetStatut(404);
        response.SetContent(myError.ErrMessage+" "+myError.SysMessage);
    }
    return true;
}
