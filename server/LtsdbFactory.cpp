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
