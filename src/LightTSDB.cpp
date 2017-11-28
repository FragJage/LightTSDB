/*** LICENCE ***************************************************************************************/
/*
  LightTSDB - Light time series database

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

namespace LightTSDB {

/**************************************************************************************************************/
/***                                                                                                        ***/
/*** Class LightTSDB                                                                                        ***/
/***                                                                                                        ***/
/**************************************************************************************************************/

LightTSDB::LightTSDB()
{
    m_Folder = ".";
}

LightTSDB::~LightTSDB()
{
    map<string,FilesInfo>::iterator it = m_FilesInfo.begin();
    while(it!=m_FilesInfo.end())
    {
        cleanUp(&(it->second));
        ++it;
    }
}

bool LightTSDB::WriteValue(const string& sensor, float value)
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
        if(filesInfo->startHour>0) filesInfo->data->WriteHourlyOffsetEndLine();

        tmNow.tm_min = 0;
        tmNow.tm_sec = 0;
        filesInfo->startHour = mktime(&tmNow);

        HourlyTimestamp_t hourlyTimestamp = HourlyTimestamp::FromTimeStruct(&tmNow);
        filesInfo->index->WriteHourlyTimestamp(hourlyTimestamp);
        filesInfo->index->WriteStreamOffset(filesInfo->data->Tellp());
        filesInfo->data->WriteHourlyTimestamp(hourlyTimestamp);
    }

    HourlyOffset_t offset = difftime(now, filesInfo->startHour);
    filesInfo->data->WriteHourlyOffset(offset, value);

    return true;
}

LightTSDB::FilesInfo* LightTSDB::getFilesInfo(const string& sensor)
{
    map<string,FilesInfo>::iterator it = m_FilesInfo.find(sensor);
    if(it != m_FilesInfo.end()) return &(it->second);

    FilesInfo filesInfo(sensor);

    if(LtsdbFile::FileExists(getFileName(sensor, data)))
    {
        if(!openFiles(filesInfo))
        {
            cleanUp(&filesInfo);
            return nullptr;
        }
    }
    else
    {
        if(!createFiles(filesInfo))
        {
            cleanUp(&filesInfo);
            return nullptr;
        }
    }

    filesInfo.startHour = HourlyTimestamp::ReadLastIndex(filesInfo.index, filesInfo.data);
    if(filesInfo.startHour == -1)
    {
        cleanUp(&filesInfo);
        setLastError(sensor, "COR1", "Index file is corrupt.");
        return nullptr;
    }
    if(filesInfo.startHour == -2)
    {
        cleanUp(&filesInfo);
        setLastError(sensor, "COR2", "Data file is corrupt.");
        return nullptr;
    }

    m_FilesInfo[sensor] = filesInfo;
    return &(m_FilesInfo[sensor]);
}

string LightTSDB::getFileName(const string& sensor, LightTSDB::FileNameType fileNameType)
{
    string ext;

    switch(fileNameType)
    {
        case data :
            ext = ".data";
            break;
        case index :
            ext = ".index";
            break;
        default :
            ext = "";
    }

    return m_Folder+"/"+sensor+ext;
}

bool LightTSDB::openFiles(LightTSDB::FilesInfo& filesInfo)
{
    filesInfo.data = new LtsdbFile();
    if(!filesInfo.data->Open(getFileName(filesInfo.sensor, data)))
    {
        setLastError(filesInfo.sensor, "OPEN1", "Unable to open data file.", strerror(errno));
        return false;
    }

    filesInfo.index = new LtsdbFile();
    if(!filesInfo.index->Open(getFileName(filesInfo.sensor, index)))
    {
        setLastError(filesInfo.sensor, "OPEN2", "Unable to open index file.", strerror(errno));
        return false;
    }

    if(!checkHeaders(filesInfo)) return false;

    return true;
}

bool LightTSDB::createFiles(LightTSDB::FilesInfo& filesInfo)
{
    filesInfo.data = new LtsdbFile();
    if(!filesInfo.data->Open(getFileName(filesInfo.sensor, data)))
    {
        setLastError(filesInfo.sensor, "CREATE1", "Unable to create data file.", strerror(errno));
        return false;
    }

    filesInfo.index = new LtsdbFile();
    if(!filesInfo.index->Open(getFileName(filesInfo.sensor, index)))
    {
        setLastError(filesInfo.sensor, "CREATE2", "Unable to create index file.", strerror(errno));
        return false;
    }

    if(!writeHeaders(filesInfo)) return false;

    return true;
}

bool LightTSDB::writeHeaders(LightTSDB::FilesInfo& filesInfo)
{
    return true;
}

bool LightTSDB::checkHeaders(LightTSDB::FilesInfo& filesInfo)
{
    return true;
}

void LightTSDB::cleanUp(FilesInfo* pFilesInfo)
{
    if(!pFilesInfo) return;
    if(pFilesInfo->data) delete(pFilesInfo->data);
    if(pFilesInfo->index) delete(pFilesInfo->index);
}

void LightTSDB::setLastError(const string& sensor, const string& code, const string& errMessage, const string& sysMessage)
{
    ErrorInfo errorInfo;

    errorInfo.Code = code;
    errorInfo.ErrMessage = errMessage;
    errorInfo.SysMessage = sysMessage;

    m_LastError[sensor] = errorInfo;
}

LightTSDB::ErrorInfo LightTSDB::GetLastError(const string& sensor)
{
    ErrorInfo emptyInfo;

    map<string, ErrorInfo>::const_iterator it = m_LastError.find(sensor);
    if(it==m_LastError.end()) return emptyInfo;
    return it->second;
}

/**************************************************************************************************************/
/***                                                                                                        ***/
/*** Class HourlyTimestamp                                                                                  ***/
/***                                                                                                        ***/
/**************************************************************************************************************/
HourlyTimestamp_t HourlyTimestamp::FromTimeStruct(struct tm* tmHour)
{
    HourlyTimestamp_t hourlyTimestamp;
    char* tmpBuffer=reinterpret_cast<char *>(&hourlyTimestamp);

    tmpBuffer[0] = (char) tmHour->tm_year;
    tmpBuffer[1] = (char) tmHour->tm_mon;
    tmpBuffer[2] = (char) tmHour->tm_mday;
    tmpBuffer[3] = (char) tmHour->tm_hour;

    return hourlyTimestamp;
}

void HourlyTimestamp::ToTimeStruct(struct tm* tmHour, HourlyTimestamp_t hourlyTimestamp)
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

time_t HourlyTimestamp::ReadLastIndex(LtsdbFile* pIndexFile, LtsdbFile* pDataFile)
{
    streampos pos;

    pIndexFile->Seekg(0, std::ios::end);
    pos = pIndexFile->Tellg();
    if(pos == 0) return 0;

    HourlyTimestamp_t hourIndex;

    pos -= (sizeof(hourIndex)+sizeof(streampos));
    pIndexFile->Seekg(pos, std::ios::beg);
    hourIndex = pIndexFile->ReadHourlyTimestamp();
    pos = pIndexFile->ReadStreamOffset();
    pIndexFile->Seekg(0, std::ios::end);

    int ret = VerifyDataHourlyTimestamp(hourIndex, pos, pDataFile);
    if(ret<0) return ret;

    struct tm tmLast;
    ToTimeStruct(&tmLast, hourIndex);
    return mktime(&tmLast);
}

int HourlyTimestamp::VerifyDataHourlyTimestamp(HourlyTimestamp_t hourIndex, streampos pos, LtsdbFile* pDataFile)
{
    HourlyTimestamp_t hourData;

    pDataFile->Seekg(pos, std::ios::beg);
    hourData = pDataFile->ReadHourlyTimestamp();
    if(hourData!=hourIndex) return -1;

    HourlyOffset_t offset;
    HourlyOffset_t offsetMax = 0;
    float value;

    while(pDataFile->ReadHourlyOffset(&offset, &value)==true)
    {
        if(offset==ENDLINE) break;
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

    pDataFile->Seekg(0, std::ios::end);
    return 0;
}

std::string HourlyTimestamp::ToString(HourlyTimestamp_t hourlyTimestamp)
{
    char* tmpBuffer=reinterpret_cast<char *>(&hourlyTimestamp);
    ostringstream oss;

    oss << 1900+(int) tmpBuffer[0] << "/" << 1+(int) tmpBuffer[1] << "/" << (int) tmpBuffer[2] << " " << (int) tmpBuffer[3] << "h";
    return oss.str();
}

/**************************************************************************************************************/
/***                                                                                                        ***/
/*** Class LtsdbFile                                                                                        ***/
/***                                                                                                        ***/
/**************************************************************************************************************/
#ifdef HAVE_LIBUV
LtsdbFile::LtsdbFile()
{
    auto loop = uvw::Loop::getDefault();
    m_InternalFile = loop->resource<uvw::FileReq>();
}

LtsdbFile::~LtsdbFile()
{
    m_InternalFile.close();
}

bool FileAbstract::Open(std::string fileName)
{
    return m_InternalFile.openSync(fileName, O_CREAT | O_APPEND | O_RDWR, 0644);
}

void FileAbstract::Close()
{
    m_InternalFile.closeSync();
}

bool LtsdbFile::FileExists(const string& fileName)
{
    auto loop = uvw::Loop::getDefault();
    auto fsReq = loop->resource<uvw::FsReq>();
    auto statR = fsReq->statSync(filename);
    return statR.first;
}
#else
LtsdbFile::LtsdbFile()
{
}

LtsdbFile::~LtsdbFile()
{
}

bool LtsdbFile::Open(const string& fileName)
{
    m_InternalFile.open(fileName, fstream::binary | fstream::out | fstream::in | fstream::app);
    if(!m_InternalFile) return false;
    return true;
}

void LtsdbFile::Close()
{
    m_InternalFile.close();
}

void LtsdbFile::Seekg(streamoff off, ios_base::seekdir way)
{
    m_InternalFile.seekg(off, way);
}

streampos LtsdbFile::Tellp()
{
    return m_InternalFile.tellp();
}

streampos LtsdbFile::Tellg()
{
    return m_InternalFile.tellg();
}

bool LtsdbFile::WriteStreamOffset(streampos pos)
{
    m_InternalFile.write(reinterpret_cast<const char *>(&pos), sizeof(pos));
    return m_InternalFile.good();
}

streampos LtsdbFile::ReadStreamOffset()
{
    streampos pos;
    m_InternalFile.read(reinterpret_cast<char *>(&pos), sizeof(pos));
    return pos;
}

bool LtsdbFile::WriteHourlyTimestamp(HourlyTimestamp_t hourlyTimestamp)
{
    m_InternalFile.write(reinterpret_cast<const char *>(&hourlyTimestamp), sizeof(hourlyTimestamp));
    return m_InternalFile.good();
}

HourlyTimestamp_t LtsdbFile::ReadHourlyTimestamp()
{
    HourlyTimestamp_t hourlyTimestamp;

    m_InternalFile.read(reinterpret_cast<char *>(&hourlyTimestamp), sizeof(hourlyTimestamp));
    return hourlyTimestamp;
}

bool LtsdbFile::ReadHourlyOffset(HourlyOffset_t* offset, float* value)
{
    m_InternalFile.read(reinterpret_cast<char *>(offset), sizeof(HourlyOffset_t));
    if(*offset==ENDLINE) return true;
    m_InternalFile.read(reinterpret_cast<char *>(value), sizeof(float));
    return m_InternalFile.good();
}

bool LtsdbFile::WriteHourlyOffset(HourlyOffset_t offset, float value)
{
    m_InternalFile.write(reinterpret_cast<const char *>(&offset), sizeof(HourlyOffset_t));
    m_InternalFile.write(reinterpret_cast<const char *>(&value), sizeof(float));
    return m_InternalFile.good();
}

bool LtsdbFile::WriteHourlyOffsetEndLine()
{
    m_InternalFile.write(reinterpret_cast<const char *>(&ENDLINE), sizeof(ENDLINE));
    return m_InternalFile.good();
}

bool LtsdbFile::FileExists(const string& fileName)
{
    if (FILE *file = fopen(fileName.c_str(), "r"))
    {
        fclose(file);
        return true;
    }
    return false;
}
#endif
}
