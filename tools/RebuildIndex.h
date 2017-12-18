#ifndef LTSDB_REBUILDINDEX_H
#define LTSDB_REBUILDINDEX_H

#include "LightTSDB.h"

namespace LightTSDB {

class RebuildIndex : public LightTSDB
{
    public:
        RebuildIndex();
        ~RebuildIndex();

        bool Rebuild(const std::string& sensor);
        ErrorInfo GetLastErrorRebuild();

    private:
        bool renameFileIndex(const std::string& sensor);
        bool openFiles(const std::string& sensor);
        bool createHeader();
        bool buildBody();
        void setLastErrorRebuild(const std::string& code, const std::string& errMessage, const std::string& sysMessage="");

        std::string m_Sensor;
        ErrorInfo m_LastError;
        LtsdbFile m_Data;
        LtsdbFile m_Index;
};

}
#endif // LTSDB_REBUILDINDEX_H
