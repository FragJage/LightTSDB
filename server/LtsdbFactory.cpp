#include "LtsdbFactory.h"

using namespace std;

map<string, LightTSDB::LightTSDB> LtsdbFactory::g_LtsdbMap;
mutex LtsdbFactory::g_MapMutex;

LightTSDB::LightTSDB& LtsdbFactory::GetInstance(string folder)
{
    g_MapMutex.lock();

    if(g_LtsdbMap.find(folder) == g_LtsdbMap.end())
    {
        LightTSDB::LightTSDB myTSDB;
        myTSDB.SetFolder("./"+folder);
        g_LtsdbMap[folder] = myTSDB;
    }

    g_MapMutex.unlock();
    return g_LtsdbMap[folder];
}
