#include <iostream>
#include <cstring>      //for strerror
#include "RebuildIndex.h"

using namespace std;

namespace LightTSDB {

RebuildIndex::RebuildIndex()
{
}

RebuildIndex::~RebuildIndex()
{
    m_Data.Close();
    m_Index.Close();
}

bool RebuildIndex::Rebuild(const string& sensor)
{
    m_Sensor = sensor;
    if(!renameFileIndex(sensor)) return false;
    if(!openFiles(sensor)) return false;
    if(!createHeader()) return false;
    if(!buildBody()) return false;

    return true;
}

ErrorInfo RebuildIndex::GetLastErrorRebuild()
{
    return m_LastError;
}

void RebuildIndex::setLastErrorRebuild(const string& code, const string& errMessage, const string& sysMessage)
{
    m_LastError.Code = code;
    m_LastError.ErrMessage = errMessage;
    m_LastError.SysMessage = sysMessage;
}

bool RebuildIndex::renameFileIndex()
{
    string indexFile = getFileName(m_Sensor, index);
    string backupFile = indexFile+".bak";


    if(!LtsdbFile::FileExists(indexFile)) return true;

    if(LtsdbFile::FileExists(backupFile))
    {
        if(remove(backupFile.c_str())!=0)
        {
            setLastErrorRebuild("REMOVE_BAK", "Unable to remove LightTSDB old backup index file.", strerror(errno));
            return false;
        }
    }

    if(rename(indexFile.c_str(), backupFile.c_str())!=0)
    {
        setLastErrorRebuild("RENAME_BAK", "Unable to create LightTSDB backup index file.", strerror(errno));
        return false;
    }

    return true;
}

bool RebuildIndex::openFiles()
{
    if(!m_Data.Open(getFileName(m_Sensor, data)))
    {
        setLastErrorRebuild("OPEN_DAT", "Unable to open LightTSDB data file.", strerror(errno));
        return false;
    }

    if(!m_Index.Open(getFileName(m_Sensor, index)))
    {
        setLastErrorRebuild("CREATE_NDX", "Unable to create LightTSDB index file.", strerror(errno));
        return false;
    }

    return true;
}

bool RebuildIndex::createHeader()
{
    string signature;
    uint8_t version;
    FileDataType type;
    uint8_t options;
    FileState state;

    if(!m_Data.ReadHeader(&signature, &version, &type, &options, &state))
    {
        setLastErrorRebuild("HEADER_REA", "Unable to read header into LightTSDB data file.", strerror(errno));
        return false;
    }

    if(!checkHeader(m_Sensor, signature, version, state, fileType))
    {
        ErrorInfo lastError = GetLastError(m_Sensor);
        setLastErrorRebuild(lastError.Code, lastError.ErrMessage, lastError.SysMessage);
        return false;
    }

    return true;
}

bool RebuildIndex::buildBody()
{
    return true;
}

}
