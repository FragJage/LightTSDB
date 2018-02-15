#include <iostream>
#include <cstring>      //for strerror
#include "RebuildIndex.h"

using namespace std;

namespace LightTSDB {

RebuildIndex::RebuildIndex() : m_ValueSize(0)
{
}

RebuildIndex::~RebuildIndex()
{
    closeFiles();
}

bool RebuildIndex::Rebuild(const string& sensor)
{
    m_Sensor = sensor;
    if(!renameFileIndex()) return false;
    if(!openFiles()) return false;
    if(!createHeader()) return false;
    if(!buildBody()) return false;
    closeFiles();

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
            setLastErrorRebuild("REMOVE_BAK", "Unable to remove LightTSDB old backup index file.", getSystemErrorMsg(errno));
            return false;
        }
    }

    if(rename(indexFile.c_str(), backupFile.c_str())!=0)
    {
        setLastErrorRebuild("RENAME_BAK", "Unable to create LightTSDB backup index file.", getSystemErrorMsg(errno));
        return false;
    }

    return true;
}

bool RebuildIndex::openFiles()
{
    if(!m_Data.Open(getFileName(m_Sensor, data)))
    {
        setLastErrorRebuild("OPEN_DAT", "Unable to open LightTSDB data file.", getSystemErrorMsg(errno));
        return false;
    }

    if(!m_Index.Open(getFileName(m_Sensor, index)))
    {
        setLastErrorRebuild("CREATE_NDX", "Unable to create LightTSDB index file.", getSystemErrorMsg(errno));
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
        setLastErrorRebuild("HEADER_REA", "Unable to read header into LightTSDB data file.", getSystemErrorMsg(errno));
        return false;
    }

    if(!checkHeader(m_Sensor, signature, version, state, FileType::data))
    {
        ErrorInfo lastError = GetLastError(m_Sensor);
        setLastErrorRebuild(lastError.Code, lastError.ErrMessage, lastError.SysMessage);
        return false;
    }

    m_ValueSize = getValueSize(type);
    if(m_ValueSize==0)
    {
        setLastErrorRebuild("HEADER_TYP", "Unsupported type of value in LightTSDB data file.");
        return false;
    }

    if(!m_Index.WriteHeader(signature, version, type, options, FileState::Stable))
    {
        setLastErrorRebuild("HEADER_WRI", "Unable to write header in LightTSDB index file.", getSystemErrorMsg(errno));
        return false;
    }

    return true;
}

bool RebuildIndex::buildBody()
{
    streampos pos;
    HourlyTimestamp_t hour;
    HourlyOffset_t offset;
    DataValue value;

    do
    {
        pos = m_Data.Tellg();
        hour = m_Data.ReadHourlyTimestamp();

        if(!m_Index.WriteHourlyTimestamp(hour))
        {
            setLastErrorRebuild("HOUR_WRI", "Unable to write hour timestamp in LightTSDB index file.", getSystemErrorMsg(errno));
            return false;
        }
        if(!m_Index.WriteStreamOffset(pos))
        {
            setLastErrorRebuild("OFFSET_WRI", "Unable to write offset in LightTSDB index file.", getSystemErrorMsg(errno));
            return false;
        }

        while(m_Data.ReadValue(&offset, &value, m_ValueSize))
        {
            if(offset==LTSDB_ENDLINE) break;
        }
    } while(offset==LTSDB_ENDLINE);
    return true;
}

void RebuildIndex::closeFiles()
{
    m_Data.Close();
    m_Index.Close();
}

}
