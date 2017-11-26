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
    short delay;
    time_t now;
    struct tm tmNow;

    filesInfo = getFilesInfo(sensor);
    if(filesInfo == nullptr) return false;

    time(&now);
cout << "NowAv" << now << endl;
    gmtime_r(&now, &tmNow);
    now = mktime(&tmNow);
cout << "NowAp" << now << endl;

    if(difftime(now, filesInfo->startHour)>3599)
    {
        if(filesInfo->startHour>0) filesInfo->data->write(reinterpret_cast<const char *>(&ENDLINE), sizeof(ENDLINE));

        tmNow.tm_min = 0;
        tmNow.tm_sec = 0;
        filesInfo->startHour = mktime(&tmNow);

cout << "Write timestamp" << endl;
        HourlyTimestamp_t hourlyTimestamp = HourlyTimestamp::FromTimeStruct(&tmNow);
        HourlyTimestamp::Write(hourlyTimestamp, filesInfo->index);
        StreamOffset::Write(filesInfo->data->tellg(), filesInfo->index);
        HourlyTimestamp::Write(hourlyTimestamp, filesInfo->data);
    }

    delay = difftime(now, filesInfo->startHour);
cout << "Write offset : " << delay << endl;
    filesInfo->data->write(reinterpret_cast<const char *>(&delay), sizeof(delay));
cout << "Write float" << endl;
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

cout << "Year" << (int) tmpBuffer[0] << endl;
cout << "Mon" << (int) tmpBuffer[1] << endl;
cout << "Day" << (int) tmpBuffer[2] << endl;
cout << "Hour" << (int) tmpBuffer[3] << endl;

}

time_t HourlyTimestamp::ReadLastIndex(std::fstream* pFile)
{
/*
    pFile->seekg(0, std::ios::end);
cout << "pos " << pFile->tellg() << endl;
    if(pFile->tellg() == 0) return 0;
*/
    LightTSDB::HourlyTimestamp_t hourlyTimestamp;
    struct tm tmLast;
/*
cout << "decal " << (sizeof(hourlyTimestamp)+sizeof(streampos)) << endl;
    pFile->seekg(-(sizeof(hourlyTimestamp)+sizeof(streampos)-1), std::ios::end);
cout << "pos " << pFile->tellg() << endl;
*/
    pFile->read(reinterpret_cast<char *>(&hourlyTimestamp), sizeof(hourlyTimestamp));
cout << "hTS " << hourlyTimestamp << endl;
    pFile->seekg(0, std::ios::end);
    ToTimeStruct(&tmLast, hourlyTimestamp);
cout << "NowLu" << mktime(&tmLast) << endl;
    return mktime(&tmLast);
}

bool HourlyTimestamp::Write(LightTSDB::HourlyTimestamp_t hourlyTimestamp, std::fstream* pFile)
{
    pFile->write(reinterpret_cast<const char *>(&hourlyTimestamp), sizeof(hourlyTimestamp));
    return pFile->good();
}

/**************************************************************************************************************/
/***                                                                                                        ***/
/*** Class StreamOffset                                                                                     ***/
/***                                                                                                        ***/
/**************************************************************************************************************/
bool StreamOffset::Write(streampos pos, fstream* pFile)
{
    pFile->write(reinterpret_cast<const char *>(&pos), sizeof(pos));
    return pFile->good();
}
