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
#include "WebServer.h"
#include "WelcomeController.h"
#include "ReadController.h"
#include "LastController.h"
#include "WriteController.h"

using namespace std;
using namespace MongooseCpp;

int main()
{
    ReadController readController;
    LastController lastController;
    WriteController writeController;
    WelcomeController welcomeController;
    WebServer server(8080);

    server.AddRoute("/read/{BaseName}/[SensorId]", &readController);
    server.AddRoute("/lastvalue/{BaseName}/{SensorId}", &lastController);
    server.AddRoute("/write/{BaseName}/{SensorId}", &writeController);
    server.AddRoute("*", &welcomeController);

    server.Start();
    while(true)
    {
        server.Poll();
    }
    server.Stop();
}
