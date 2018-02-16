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

    LightTSDB::DataValue data;

    if(LtsdbFactory::GetInstance(folder).ReadLastValue(sensor, data))
    {
        ostringstream ss;
        ss << TimeHelper::ToString(data.time) << " : " << data.value.Float << endl;
        response.SetStatut(200);
        response.SetContent(ss.str());
    }
    else
    {
        LightTSDB::ErrorInfo myError = LtsdbFactory::GetInstance(folder).GetLastError(sensor);
        response.SetStatut(404);
        response.SetContent(myError.ErrMessage+" "+myError.SysMessage);
    }
    return true;
}
