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
    HourlyOffset_t delay;
    time_t now;
    struct tm tmNow;

    filesInfo = getFilesInfo(sensor);
    if(filesInfo == nullptr) return false;

    time(&now);
    gmtime_r(&now, &tmNow);
    now = mktime(&tmNow);

    if(difftime(now, filesInfo->startHour)>3599)
    {
        if(filesInfo->startHour>0) filesInfo->data->write(reinterpret_cast<const char *>(&ENDLINE), sizeof(ENDLINE));

        tmNow.tm_min = 0;
        tmNow.tm_sec = 0;
        filesInfo->startHour = mktime(&tmNow);

        HourlyTimestamp_t hourlyTimestamp = HourlyTimestamp::FromTimeStruct(&tmNow);
        HourlyTimestamp::Write(hourlyTimestamp, filesInfo->index);
        StreamOffset::Write(filesInfo->data, filesInfo->index);
        HourlyTimestamp::Write(hourlyTimestamp, filesInfo->data);
    }

    delay = difftime(now, filesInfo->startHour);
    filesInfo->data->write(reinterpret_cast<const char *>(&delay), sizeof(delay));
    filesInfo->data->write(reinterpret_cast<const char *>(&value), sizeof(value));

    return true;
}

LightTSDB::FilesInfo* LightTSDB::getFilesInfo(string sensor)
{
    map<string,FilesInfo>::iterator it = m_FilesInfo.find(sensor);
    if(it != m_FilesInfo.end())
    {
        return &(it->second);
    }

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

    filesInfo.startHour = HourlyTimestamp::ReadLastIndex(filesInfo.index);
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

time_t HourlyTimestamp::ReadLastIndex(std::fstream* pFile)
{
    streampos pos;

    pFile->seekg(0, std::ios::end);
    pos = pFile->tellg();
    if(pos == 0) return 0;

    LightTSDB::HourlyTimestamp_t hourlyTimestamp;
    struct tm tmLast;

    pos -= (sizeof(hourlyTimestamp)+sizeof(streampos));
    pFile->seekg(pos, std::ios::beg);
    pFile->read(reinterpret_cast<char *>(&hourlyTimestamp), sizeof(hourlyTimestamp));
    pFile->seekg(0, std::ios::end);
    ToTimeStruct(&tmLast, hourlyTimestamp);
    return mktime(&tmLast);
}

bool HourlyTimestamp::Write(LightTSDB::HourlyTimestamp_t hourlyTimestamp, std::fstream* pFile)
{
    pFile->write(reinterpret_cast<const char *>(&hourlyTimestamp), sizeof(hourlyTimestamp));
    return pFile->good();
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
