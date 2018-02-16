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
#include "WelcomeController.h"

using namespace std;
using namespace MongooseCpp;

WelcomeController::WelcomeController()
{
    //ctor
}

WelcomeController::~WelcomeController()
{
    //dtor
}

bool WelcomeController::Process(Request& request, Response& response)
{
    response.SetStatut(200);
    response.SetContent("<html><head></head><body><h1>Welcome on LightTSDB server !</h1></body>");
    return true;
}
