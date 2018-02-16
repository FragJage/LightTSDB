/*** LICENCE ***************************************************************************************/
/*
  LightTSDB - Light time series database
  
  This file is part of LightTSDB server.
  
    LightTSDB is free software : you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
	
    LightTSDB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
	
    You should have received a copy of the GNU General Public License
    along with LightTSDB.  If not, see <http://www.gnu.org/licenses/>.
*/
/***************************************************************************************************/
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
