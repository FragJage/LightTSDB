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
        bool renameFileIndex();
        bool openFiles();
        bool createHeader();
        bool buildBody();
        void closeFiles();
        void setLastErrorRebuild(const std::string& code, const std::string& errMessage, const std::string& sysMessage="");

        std::string m_Sensor;
        ErrorInfo m_LastError;
        LtsdbFile m_Data;
        LtsdbFile m_Index;
        int m_ValueSize;
};

}
#endif // LTSDB_REBUILDINDEX_H
