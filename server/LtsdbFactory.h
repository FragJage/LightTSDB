#ifndef LTSDBFACTORY_H
#define LTSDBFACTORY_H

#include <map>
#include <string>
#include <mutex>
#include "LightTSDB.h"

class LtsdbFactory
{
    public:
        static LightTSDB::LightTSDB& GetInstance(std::string folder);

    private:
        static std::map<std::string, LightTSDB::LightTSDB> g_LtsdbMap;
        static std::mutex g_MapMutex;
};

#endif // LTSDBFACTORY_H
