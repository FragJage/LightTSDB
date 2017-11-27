/*** LICENCE ***************************************************************************************/
/*
  LightTSDB - Simple class for configuration file like .ini

  This file is part of LightTSDB.

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
#include <iostream>


#include <cstring>      //for strerror
#include <sstream>
#include "LightTSDB.h"

using namespace std;

/**************************************************************************************************************/
/***                                                                                                        ***/
/*** Class LightTSDB                                                                                        ***/
/***                                                                                                        ***/
/**************************************************************************************************************/

const short LightTSDB::ENDLINE = 0XFFFE;

LightTSDB::LightTSDB()
{
    m_Folder = ".";
}

LightTSDB::~LightTSDB()
{
    map<string,FilesInfo>::iterator it = m_FilesInfo.begin();
    while(it!=m_FilesInfo.end())
    {
        it->second.data->close();
        it->second.index->close();
        delete it->second.data;
        delete it->second.index;
        ++it;
    }
}

bool LightTSDB::WriteValue(string sensor, float value)
{
    FilesInfo* filesInfo;
    time_t now;
    struct tm tmNow;

    filesInfo = getFilesInfo(sensor);
    if(filesInfo == nullptr) return false;

    time(&now);
    gmtime_r(&now, &tmNow);
    now = mktime(&tmNow);

    if(difftime(now, filesInfo->startHour)>3599)
    {
        if(filesInfo->startHour>0) HourlyOffset::WriteEndLine(filesInfo->data);

        tmNow.tm_min = 0;
        tmNow.tm_sec = 0;
        filesInfo->startHour = mktime(&tmNow);

        HourlyTimestamp_t hourlyTimestamp = HourlyTimestamp::FromTimeStruct(&tmNow);
        HourlyTimestamp::Write(hourlyTimestamp, filesInfo->index);
        StreamOffset::Write(filesInfo->data, filesInfo->index);
        HourlyTimestamp::Write(hourlyTimestamp, filesInfo->data);
    }

    HourlyOffset_t offset = difftime(now, filesInfo->startHour);
    HourlyOffset::Write(filesInfo->data, offset, value);

    return true;
}

LightTSDB::FilesInfo* LightTSDB::getFilesInfo(string sensor)
{
    map<string,FilesInfo>::iterator it = m_FilesInfo.find(sensor);
    if(it != m_FilesInfo.end()) return &(it->second);

    FilesInfo filesInfo;
    filesInfo.data = new fstream();
    filesInfo.index = new fstream();

    filesInfo.data->open(m_Folder+"/"+sensor+".data", fstream::binary | fstream::out | fstream::in | fstream::app);
    if(!(*filesInfo.data))
    {
        m_LastError = strerror(errno);
        return nullptr;
    }

    filesInfo.index->open(m_Folder+"/"+sensor+".index", fstream::binary | fstream::out | fstream::in | fstream::app);
    if(!(*filesInfo.index))
    {
        m_LastError = strerror(errno);
        return nullptr;
    }

    filesInfo.startHour = HourlyTimestamp::ReadLastIndex(filesInfo.index, filesInfo.data);
    if(filesInfo.startHour == -1)
    {
        m_LastError = "Index file "+sensor+".index is corrupt.";
        return nullptr;
    }
    if(filesInfo.startHour == -2)
    {
        m_LastError = "Data file "+sensor+".data is corrupt.";
        return nullptr;
    }

    m_FilesInfo[sensor] = filesInfo;
    return &(m_FilesInfo[sensor]);
}

string LightTSDB::GetLastError()
{
    return m_LastError;
}

/**************************************************************************************************************/
/***                                                                                                        ***/
/*** Class HourlyTimestamp                                                                                  ***/
/***                                                                                                        ***/
/**************************************************************************************************************/
LightTSDB::HourlyTimestamp_t HourlyTimestamp::FromTimeStruct(struct tm* tmHour)
{
    LightTSDB::HourlyTimestamp_t hourlyTimestamp;
    char* tmpBuffer=reinterpret_cast<char *>(&hourlyTimestamp);

    tmpBuffer[0] = (char) tmHour->tm_year;
    tmpBuffer[1] = (char) tmHour->tm_mon;
    tmpBuffer[2] = (char) tmHour->tm_mday;
    tmpBuffer[3] = (char) tmHour->tm_hour;

    return hourlyTimestamp;
}

void HourlyTimestamp::ToTimeStruct(struct tm* tmHour, LightTSDB::HourlyTimestamp_t hourlyTimestamp)
{
    char* tmpBuffer=reinterpret_cast<char *>(&hourlyTimestamp);
    time_t now;


    time(&now);
    gmtime_r(&now, tmHour);

    tmHour->tm_year = (int) tmpBuffer[0];
    tmHour->tm_mon  = (int) tmpBuffer[1];
    tmHour->tm_mday = (int) tmpBuffer[2];
    tmHour->tm_hour = (int) tmpBuffer[3];
    tmHour->tm_min  = 0;
    tmHour->tm_sec  = 0;
}

time_t HourlyTimestamp::ReadLastIndex(std::fstream* pIndexFile, std::fstream* pDataFile)
{
    streampos pos;

    pIndexFile->seekg(0, std::ios::end);
    pos = pIndexFile->tellg();
    if(pos == 0) return 0;

    LightTSDB::HourlyTimestamp_t hourIndex;

    pos -= (sizeof(hourIndex)+sizeof(streampos));
    pIndexFile->seekg(pos, std::ios::beg);
    hourIndex = HourlyTimestamp::Read(pIndexFile);
    pos = StreamOffset::Read(pIndexFile);
    pIndexFile->seekg(0, std::ios::end);

    int ret = VerifyDataHourlyTimestamp(hourIndex, pos, pDataFile);
    if(ret<0) return ret;

    struct tm tmLast;
    ToTimeStruct(&tmLast, hourIndex);
    return mktime(&tmLast);
}

int HourlyTimestamp::VerifyDataHourlyTimestamp(LightTSDB::HourlyTimestamp_t hourIndex, streampos pos, fstream *pDataFile)
{
    LightTSDB::HourlyTimestamp_t hourData;

    pDataFile->seekg(pos, std::ios::beg);
    hourData = HourlyTimestamp::Read(pDataFile);
    if(hourData!=hourIndex) return -1;

    LightTSDB::HourlyOffset_t offset;
    LightTSDB::HourlyOffset_t offsetMax = 0;
    float value;

    while(HourlyOffset::Read(pDataFile, &offset, &value)==true)
    {
        if(offset==LightTSDB::ENDLINE) break;
        offsetMax = offset;
    }

    struct tm tmp;
    time_t tMax;
    time_t tCur;

    ToTimeStruct(&tmp, hourData);
    tMax = mktime(&tmp);
    tMax += offsetMax;

    time(&tCur);
    gmtime_r(&tCur, &tmp);
    tCur = mktime(&tmp);

    if(tMax>tCur) return -2;

    pDataFile->seekg(0, std::ios::end);
    return 0;
}

bool HourlyTimestamp::Write(LightTSDB::HourlyTimestamp_t hourlyTimestamp, std::fstream* pFile)
{
    pFile->write(reinterpret_cast<const char *>(&hourlyTimestamp), sizeof(hourlyTimestamp));
    return pFile->good();
}

LightTSDB::HourlyTimestamp_t HourlyTimestamp::Read(std::fstream* pFile)
{
    LightTSDB::HourlyTimestamp_t hourlyTimestamp;

    pFile->read(reinterpret_cast<char *>(&hourlyTimestamp), sizeof(hourlyTimestamp));
    return hourlyTimestamp;
}

std::string HourlyTimestamp::ToString(LightTSDB::HourlyTimestamp_t hourlyTimestamp)
{
    char* tmpBuffer=reinterpret_cast<char *>(&hourlyTimestamp);
    ostringstream oss;

    oss << 1900+(int) tmpBuffer[0] << "/" << 1+(int) tmpBuffer[1] << "/" << (int) tmpBuffer[2] << " " << (int) tmpBuffer[3] << "h";
    return oss.str();
}

/**************************************************************************************************************/
/***                                                                                                        ***/
/*** Class StreamOffset                                                                                     ***/
/***                                                                                                        ***/
/**************************************************************************************************************/
bool StreamOffset::Write(fstream* pDataFile, fstream* pIndexFile)
{
    pDataFile->seekg(0, std::ios::end);
    streampos pos = pDataFile->tellg();
    pIndexFile->write(reinterpret_cast<const char *>(&pos), sizeof(pos));
    return pIndexFile->good();
}

streampos StreamOffset::Read(fstream* pIndexFile)
{
    streampos pos;

    pIndexFile->read(reinterpret_cast<char *>(&pos), sizeof(pos));
    return pos;
}

/**************************************************************************************************************/
/***                                                                                                        ***/
/*** Class HourlyOffset                                                                                     ***/
/***                                                                                                        ***/
/**************************************************************************************************************/
bool HourlyOffset::Read(std::fstream* pDataFile, LightTSDB::HourlyOffset_t* offset, float* value)
{
    pDataFile->read(reinterpret_cast<char *>(offset), sizeof(LightTSDB::HourlyOffset_t));
    if(*offset==LightTSDB::ENDLINE) return true;
    pDataFile->read(reinterpret_cast<char *>(value), sizeof(float));
    return pDataFile->good();
}

bool HourlyOffset::Write(std::fstream* pDataFile, LightTSDB::HourlyOffset_t offset, float value)
{
    pDataFile->write(reinterpret_cast<const char *>(&offset), sizeof(LightTSDB::HourlyOffset_t));
    pDataFile->write(reinterpret_cast<const char *>(&value), sizeof(float));
    return pDataFile->good();
}

bool HourlyOffset::WriteEndLine(std::fstream* pDataFile)
{
    pDataFile->write(reinterpret_cast<const char *>(&LightTSDB::ENDLINE), sizeof(LightTSDB::ENDLINE));
    return pDataFile->good();
}

/**************************************************************************************************************/
/***                                                                                                        ***/
/*** Class FileAbstract                                                                                     ***/
/***                                                                                                        ***/
/**************************************************************************************************************/
#ifdef HAVE_LIBUV
ltsdb_fs_t FileAbstract::NewHandle()
{
    auto loop = uvw::Loop::getDefault();
    return loop->resource<uvw::FileReq>();
}

void DeleteHandle(ltsdb_fs_t hfs)
{
}

bool Open(ltsdb_fs_t hfs, std::string fileName)
{
    return hfs->openSync(fileName, O_CREAT | O_APPEND | O_RDWR, 0644);
}

void Close(ltsdb_fs_t hfs)
{
    hfs->closeSync();
}
#else
ltsdb_fs_t FileAbstract::NewHandle()
{
    return new fstream();
}

void DeleteHandle(ltsdb_fs_t hfs)
{
    delete hfs;
}

bool Open(ltsdb_fs_t hfs, std::string fileName)
{
    hfs->open(fileName, fstream::binary | fstream::out | fstream::in | fstream::app);
    if(!(*hfs))
    {
        m_LastError = strerror(errno);
        return false;
    }
    return true;
}

void Close(ltsdb_fs_t hfs)
{
    hfs->close();
}
#endif
