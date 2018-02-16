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
#include "LtsdbFactory.h"

using namespace std;

map<string, LightTSDB::LightTSDB> LtsdbFactory::g_LtsdbMap;
mutex LtsdbFactory::g_MapMutex;

LightTSDB::LightTSDB& LtsdbFactory::GetInstance(string folder)
{
    g_MapMutex.lock();

    if(g_LtsdbMap.find(folder) == g_LtsdbMap.end())
    {
		g_LtsdbMap.emplace(std::piecewise_construct, std::forward_as_tuple(folder), std::forward_as_tuple());
		g_LtsdbMap[folder].SetFolder("./" + folder);
    }

    g_MapMutex.unlock();
    return g_LtsdbMap[folder];
}
