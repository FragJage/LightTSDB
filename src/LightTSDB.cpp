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
#include <cstring>      //for strerror
#include <dirent.h>     //for opendir & readdir
#include <sstream>
#include <algorithm>
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

void LightTSDB::SetFolder(const string& folder)
{
    m_Folder = folder;
}

bool LightTSDB::GetSensorList(list<string>& sensorList)
{
    DIR *dir;
    struct dirent *ent;
    string file;

    if((dir=opendir(m_Folder.c_str())) == nullptr) return false;

    sensorList.clear();
    while((ent=readdir(dir)) != nullptr)
    {
        file = ent->d_name;
        if(ends_with(file, ".data"))
            sensorList.emplace_back(file.substr(0, file.size()-5));
    }
    closedir(dir);
    return true;
}

template<>
bool LightTSDB::WriteValue<float>(const string& sensor, float value)
{
    return internalWriteValue(sensor, &value, FileDataType::Float);
}

template<>
bool LightTSDB::WriteValue<int>(const string& sensor, int value)
{
    return internalWriteValue(sensor, &value, FileDataType::Int);
}

template<>
bool LightTSDB::WriteValue<double>(const string& sensor, double value)
{
    return internalWriteValue(sensor, &value, FileDataType::Double);
}

template<>
bool LightTSDB::WriteValue<bool>(const string& sensor, bool value)
{
    return internalWriteValue(sensor, &value, FileDataType::Bool);
}

bool LightTSDB::internalWriteValue(const string& sensor, void* value, FileDataType valueType)
{
    FilesInfo* filesInfo = getFilesInfo(sensor, valueType);
    if(filesInfo == nullptr) return false;

    time_t now;
    time(&now);
    return writeTimeValue(filesInfo, value, now);
}

template<>
bool LightTSDB::WriteOldValue<float>(const std::string& sensor, float value, uint32_t offset)
{
    return internalWriteOldValue(sensor, &value, offset, FileDataType::Float);
}

template<>
bool LightTSDB::WriteOldValue<int>(const std::string& sensor, int value, uint32_t offset)
{
    return internalWriteOldValue(sensor, &value, offset, FileDataType::Int);
}

template<>
bool LightTSDB::WriteOldValue<double>(const std::string& sensor, double value, uint32_t offset)
{
    return internalWriteOldValue(sensor, &value, offset, FileDataType::Double);
}

template<>
bool LightTSDB::WriteOldValue<bool>(const std::string& sensor, bool value, uint32_t offset)
{
    return internalWriteOldValue(sensor, &value, offset, FileDataType::Bool);
}

bool LightTSDB::internalWriteOldValue(const std::string& sensor, void* pValue, uint32_t offset, FileDataType valueType)
{
    FilesInfo* filesInfo = getFilesInfo(sensor, valueType);
    if(filesInfo == nullptr) return false;

    time_t oldTime;
    time(&oldTime);
    oldTime -= offset;

    if((filesInfo->maxHour>0)&&(oldTime <= HourlyTimestamp::ToTimeT(filesInfo->maxHour, filesInfo->maxOffset)))
    {
        setLastError(sensor, "WRITE_MRV", "There is a more recent value.");
        return false;
    }

    return writeTimeValue(filesInfo, pValue, oldTime);
}

template<>
bool LightTSDB::WriteTimeValue<float>(const std::string& sensor, float value, time_t oldTime)
{
    return internalWriteTimeValue(sensor, &value, oldTime, FileDataType::Float);
}

template<>
bool LightTSDB::WriteTimeValue<int>(const std::string& sensor, int value, time_t oldTime)
{
    return internalWriteTimeValue(sensor, &value, oldTime, FileDataType::Int);
}

template<>
bool LightTSDB::WriteTimeValue<double>(const std::string& sensor, double value, time_t oldTime)
{
    return internalWriteTimeValue(sensor, &value, oldTime, FileDataType::Double);
}

template<>
bool LightTSDB::WriteTimeValue<bool>(const std::string& sensor, bool value, time_t oldTime)
{
    return internalWriteTimeValue(sensor, &value, oldTime, FileDataType::Bool);
}

bool LightTSDB::internalWriteTimeValue(const std::string& sensor, void* pValue, time_t oldTime, FileDataType valueType)
{
    FilesInfo* filesInfo = getFilesInfo(sensor, valueType);
    if(filesInfo == nullptr) return false;

    if((filesInfo->maxHour>0)&&(oldTime <= HourlyTimestamp::ToTimeT(filesInfo->maxHour, filesInfo->maxOffset)))
    {
        setLastError(sensor, "WRITE_MRV", "There is a more recent value.");
        return false;
    }

    return writeTimeValue(filesInfo, pValue, oldTime);
}

bool LightTSDB::writeTimeValue(FilesInfo* filesInfo, void* pValue, time_t timestamp)
{
    if(difftime(timestamp, filesInfo->startHour)>3599)
    {
        if(filesInfo->startHour>0) filesInfo->data->WriteEndLine();

        HourlyTimestamp_t hourlyTimestamp = HourlyTimestamp::FromTimeT(timestamp);
        filesInfo->startHour = HourlyTimestamp::ToTimeT(hourlyTimestamp);
        filesInfo->index->WriteHourlyTimestamp(hourlyTimestamp);
        filesInfo->index->WriteStreamOffset(filesInfo->data->Tellp());
        filesInfo->indexSize += INDEX_STEP;
        filesInfo->data->WriteHourlyTimestamp(hourlyTimestamp);

        if(filesInfo->minHour == 0) filesInfo->minHour = hourlyTimestamp;
        filesInfo->maxHour = hourlyTimestamp;
    }

    HourlyOffset_t offset = difftime(timestamp, filesInfo->startHour);
    if(!filesInfo->data->WriteValue(offset, pValue, filesInfo->valueSize)) return false;
    filesInfo->maxOffset = offset;

    return true;
}

bool LightTSDB::ReadValues(const std::string& sensor, time_t hour, std::list<DataValue>& values)
{
    FilesInfo* filesInfo = getFilesInfo(sensor, FileDataType::Undefined);
    if(filesInfo == nullptr) return false;

    HourlyTimestamp_t hourlyTimestamp = HourlyTimestamp::FromTimeT(hour);
    values.clear();
    streampos pos = findIndex(filesInfo, hourlyTimestamp);
    if(pos==(streampos)0) return true;

    filesInfo->data->Seekg(pos, std::ios::beg);
    HourlyTimestamp_t dataTimestamp = filesInfo->data->ReadHourlyTimestamp();
    if(dataTimestamp!=hourlyTimestamp) return false;

    HourlyOffset_t offset;
    UValue uvalue;
    while(filesInfo->data->ReadValue(&offset, &uvalue, filesInfo->valueSize))
    {
        if(offset==ENDLINE) break;
        values.emplace_back(HourlyTimestamp::ToTimeT(dataTimestamp, offset), uvalue);
    }

    if(offset!=ENDLINE) filesInfo->data->Clear();;

    return true;
}

bool LightTSDB::ReadValues(const string& sensor, time_t timeBegin, time_t timeEnd, list<DataValue>& values)
{
    FilesInfo* filesInfo = getFilesInfo(sensor, FileDataType::Undefined);
    if(filesInfo == nullptr) return false;

    HourlyTimestamp_t hourlyTimestamp = HourlyTimestamp::FromTimeT(timeBegin);
    values.clear();
    streampos pos = findIndex(filesInfo, hourlyTimestamp);
    if(pos==(streampos)0) return true;

    filesInfo->data->Seekg(pos, std::ios::beg);
    HourlyTimestamp_t dataTimestamp = filesInfo->data->ReadHourlyTimestamp();
    if(dataTimestamp!=hourlyTimestamp) return false;

    HourlyOffset_t offset;
    UValue uvalue;
    time_t ts;
    while(filesInfo->data->ReadValue(&offset, &uvalue, filesInfo->valueSize))
    {
        if(offset==ENDLINE)
        {
            dataTimestamp = filesInfo->data->ReadHourlyTimestamp();
        }
        else
        {
            ts = HourlyTimestamp::ToTimeT(dataTimestamp, offset);
            if(ts>timeEnd) break;
            if(ts>=timeBegin) values.emplace_back(ts, uvalue);
        }
    }

    if(offset!=ENDLINE) filesInfo->data->Clear();;

    return true;
}

bool LightTSDB::ReadLastValue(const string& sensor, DataValue& dataValue)
{
    FilesInfo* filesInfo = getFilesInfo(sensor, FileDataType::Undefined);
    if(filesInfo == nullptr) return false;

    streampos pos = findIndex(filesInfo, filesInfo->maxHour);
    if(pos==(streampos)0) return true;
    filesInfo->data->Seekg(pos, std::ios::beg);
    HourlyTimestamp_t dataTimestamp = filesInfo->data->ReadHourlyTimestamp();
    if(dataTimestamp!=filesInfo->maxHour) return false;

    HourlyOffset_t offset;
    UValue uvalue;

    filesInfo->data->Seekg(0, std::ios::end);
    pos = filesInfo->data->Tellg();
    pos -= (filesInfo->valueSize+sizeof(HourlyOffset_t));
    filesInfo->data->Seekg(pos, std::ios::beg);
    filesInfo->data->ReadValue(&offset, &uvalue, filesInfo->valueSize);
    dataValue.time = HourlyTimestamp::ToTimeT(filesInfo->maxHour, offset);
    dataValue.value = uvalue;
    filesInfo->data->Seekg(0, std::ios::end);
    return true;
}

bool LightTSDB::ResampleValues(const string& sensor, time_t timeBegin, time_t timeEnd, list<DataValue>& values, int interval)
{
    list<DataValue> readValues;
    vector<ResamplingHelper::AverageValue> averages;

    if(!ReadValues(sensor, timeBegin, timeEnd, readValues))
        return false;

    values.clear();
    if(readValues.size()==0) return true;

    ResamplingHelper::Average(timeBegin, readValues, interval, averages);
    ResamplingHelper::PreserveExtremum(averages, values);

    return true;
}


bool LightTSDB::Close(const std::string& sensor)
{
    map<string,FilesInfo>::iterator it = m_FilesInfo.find(sensor);
    if(it == m_FilesInfo.end()) return false;

    cleanUp(&(it->second));
    m_FilesInfo.erase(it);
    return true;
}

bool LightTSDB::Remove(const std::string& sensor)
{
    bool ret = true;

    Close(sensor);
    if(remove(getFileName(sensor, FileType::index).c_str())!=0)
    {
        setLastError(sensor, "REMOVE_NDX", "Unable to remove LightTSDB index file.", strerror(errno));
        ret = false;
    }
    if(remove(getFileName(sensor, FileType::data).c_str())!=0)
    {
        setLastError(sensor, "REMOVE_DAT", "Unable to remove LightTSDB data file.", strerror(errno));
        ret = false;
    }

    return ret;
}

LightTSDB::FilesInfo* LightTSDB::getFilesInfo(const string& sensor, FileDataType valueType)
{
    map<string,FilesInfo>::iterator it = m_FilesInfo.find(sensor);
    if(it == m_FilesInfo.end())
    {
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
            if(valueType==FileDataType::Undefined)
            {
                setLastError(filesInfo.sensor, "NOFILE", "Data file not found.");
                cleanUp(&filesInfo);
                return nullptr;
            }
            if(!createFiles(filesInfo, valueType))
            {
                cleanUp(&filesInfo);
                return nullptr;
            }
        }

        m_FilesInfo[sensor] = filesInfo;
        it = m_FilesInfo.find(sensor);
    }

    if(((valueType!=FileDataType::Undefined))&&(it->second.type!=valueType))
    {
        setLastError(sensor, "MISMATCH", "Type mismatch between value and LightTSDB data file.");
        return nullptr;
    }
    return &(it->second);
}

string LightTSDB::getFileName(const string& sensor, LightTSDB::FileType fileType)
{
    ostringstream oss;
    string ext = getFileExt(fileType);
    if(ext!="") ext = "."+ext;
    oss << m_Folder << "/" << sensor << ext;
    return oss.str();
}

std::string LightTSDB::getFileExt(FileType fileType)
{
    switch(fileType)
    {
        case data : return "data";
        case index : return "index";
        default : return "";
    }
}

bool LightTSDB::openFiles(LightTSDB::FilesInfo& filesInfo)
{
    if(!openDataFile(filesInfo)) return false;
    if(!openIndexFile(filesInfo)) return false;

    int res = HourlyTimestamp::ReadLastIndex(filesInfo.startHour, filesInfo.data, filesInfo.index, filesInfo.valueSize);
    if(res==-1)
    {
        setLastError(filesInfo.sensor, "OPEN_COR1", "Index file is corrupt.");
        return false;
    }
    if(res==-2)
    {
        setLastError(filesInfo.sensor, "OPEN_COR2", "Data file is corrupt.");
        return false;
    }

    filesInfo.data->Clear();
    return true;
}

bool LightTSDB::openDataFile(FilesInfo& filesInfo)
{
    string signature;
    FileState fileState;
    streampos pos;
    UValue uvalue;
    HourlyOffset_t offset;

    filesInfo.data = new LtsdbFile();
    if(!filesInfo.data->Open(getFileName(filesInfo.sensor, FileType::data)))
    {
        setLastError(filesInfo.sensor, "OPEN_DAT1", "Unable to open data file.", strerror(errno));
        return false;
    }
    filesInfo.data->Seekg(0, std::ios::beg);
    if(!filesInfo.data->ReadHeader(&signature, &(filesInfo.version), &(filesInfo.type), &(filesInfo.options), &fileState))
    {
        setLastError(filesInfo.sensor, "OPEN_DAT2", "Unable to read header of data file.", strerror(errno));
        return false;
    }
    filesInfo.valueSize = getValueSize(filesInfo.type);
    if(!checkHeader(filesInfo.sensor, signature, filesInfo.version, fileState, FileType::data)) return false;
    filesInfo.data->Seekg(0, std::ios::end);
    pos = filesInfo.data->Tellg();
    pos -= (sizeof(float)+sizeof(HourlyOffset_t));
    filesInfo.data->ReadValue(&offset, &uvalue, filesInfo.valueSize);
    filesInfo.maxOffset = offset;
    filesInfo.data->Clear();
    filesInfo.data->Seekg(0, std::ios::end);
    return true;
}

bool LightTSDB::openIndexFile(FilesInfo& filesInfo)
{
    string signature;
    uint8_t version;
    FileDataType dataType;
    uint8_t options;
    FileState fileState;
    streampos pos;

    filesInfo.index = new LtsdbFile();
    if(!filesInfo.index->Open(getFileName(filesInfo.sensor, FileType::index)))
    {
        setLastError(filesInfo.sensor, "OPEN_NDX1", "Unable to open index file.", strerror(errno));
        return false;
    }
    filesInfo.index->Seekg(0, std::ios::beg);
    if(!filesInfo.index->ReadHeader(&signature, &version, &dataType, &options, &fileState))
    {
        setLastError(filesInfo.sensor, "OPEN_NDX2", "Unable to read header of index file.", strerror(errno));
        return false;
    }
    if(!checkHeader(filesInfo.sensor, signature, version, fileState, FileType::index)) return false;

    if((filesInfo.type!=dataType)||(filesInfo.options!=options))
    {
        setLastError(filesInfo.sensor, "OPEN_CHK1", "Index file is corrupt, repair it.");
        return false;
    }

    filesInfo.minHour = filesInfo.index->ReadHourlyTimestamp();
    filesInfo.index->Seekg(0, std::ios::end);
    pos = filesInfo.index->Tellg();
    filesInfo.indexSize = pos;
    filesInfo.indexSize -= HEADER_SIZE;
    pos -= INDEX_STEP;
    filesInfo.index->Seekg(pos, std::ios::beg);
    filesInfo.maxHour = filesInfo.index->ReadHourlyTimestamp();

    filesInfo.index->Seekg(0, std::ios::end);
    return true;
}

bool LightTSDB::createFiles(LightTSDB::FilesInfo& filesInfo, FileDataType valueType)
{
    filesInfo.data = new LtsdbFile();
    if(!filesInfo.data->Open(getFileName(filesInfo.sensor, data)))
    {
        setLastError(filesInfo.sensor, "CREATE_DAT1", "Unable to create data file.", strerror(errno));
        return false;
    }
    if(!filesInfo.data->WriteHeader(SIGNATURE, VERSION, valueType, 0, FileState::Stable))
    {
        setLastError(filesInfo.sensor, "CREATE_DAT2", "Unable to write header of data file.", strerror(errno));
        return false;
    }

    filesInfo.index = new LtsdbFile();
    if(!filesInfo.index->Open(getFileName(filesInfo.sensor, index)))
    {
        setLastError(filesInfo.sensor, "CREATE_NDX1", "Unable to create index file.", strerror(errno));
        return false;
    }
    if(!filesInfo.index->WriteHeader(SIGNATURE, VERSION, valueType, 0, FileState::Stable))
    {
        setLastError(filesInfo.sensor, "CREATE_NDX2", "Unable to write header of index file.", strerror(errno));
        return false;
    }

    filesInfo.type = valueType;
    filesInfo.valueSize = getValueSize(valueType);
    filesInfo.startHour = 0;

    return true;
}

void LightTSDB::cleanUp(FilesInfo* pFilesInfo)
{
    if(!pFilesInfo) return;
    if(pFilesInfo->data)
    {
        pFilesInfo->data->Close();
        delete(pFilesInfo->data);
    }
    if(pFilesInfo->index)
    {
        pFilesInfo->index->Close();
        delete(pFilesInfo->index);
    }
}

bool LightTSDB::checkHeader(const std::string& sensor, const std::string& signature, uint8_t version, FileState state, FileType fileType)
{
    if(!checkSignature(sensor, signature, fileType)) return false;
    if(!checkVersion(sensor, version, fileType)) return false;
    if(!checkState(sensor, state, fileType)) return false;
    return true;
}

bool LightTSDB::checkSignature(const std::string& sensor, const std::string& signature, FileType fileType)
{
    if(signature==SIGNATURE) return true;

    string file = getFileExt(fileType);
    if(file=="") file = "unknown";
    setLastError(sensor, "CHECK_SIG", "It's not a LightTSDB "+file+" file.");
    return false;
}

bool LightTSDB::checkVersion(const std::string& sensor, uint8_t version, FileType fileType)
{
    if(version==VERSION) return true;

    string file = getFileExt(fileType);
    if(file=="") file = "unknown";
    setLastError(sensor, "CHECK_VER", "The version of "+file+" file is not supported by LightTSDB library.");
    return false;
}

bool LightTSDB::checkState(const std::string& sensor, FileState state, FileType fileType)
{
    if(state==FileState::Stable) return true;

    string file = getFileExt(fileType);
    if(file=="") file = "unknown";
    setLastError(sensor, "CHECK_STA", "The "+file+" file is not stable, repair it.");
    return false;
}

streampos LightTSDB::findIndex(FilesInfo* filesInfo, HourlyTimestamp_t hourlyTimestamp)
{
    streampos pos;
    HourlyTimestamp_t foundTimestamp;

    if(hourlyTimestamp < filesInfo->minHour) return 0;
    if(hourlyTimestamp > filesInfo->maxHour) return 0;

    if(filesInfo->maxHour == filesInfo->minHour)
        pos = 0;
    else
        pos = filesInfo->indexSize*(hourlyTimestamp-filesInfo->minHour)/(filesInfo->maxHour-filesInfo->minHour+1);

    pos = HEADER_SIZE+INDEX_STEP*(pos/INDEX_STEP);

    filesInfo->index->Seekg(pos, std::ios::beg);
    foundTimestamp = filesInfo->index->ReadHourlyTimestamp();
    if(foundTimestamp == hourlyTimestamp) return filesInfo->index->ReadStreamOffset();

    if(foundTimestamp > hourlyTimestamp)
    {
        while(foundTimestamp != hourlyTimestamp)
        {
            pos -= INDEX_STEP;
            if(!filesInfo->index->Seekg(pos, std::ios::beg)) return 0;
            foundTimestamp = filesInfo->index->ReadHourlyTimestamp();
            if(foundTimestamp < hourlyTimestamp) return 0;
        }
        return filesInfo->index->ReadStreamOffset();
    }

    return 0;
}

void LightTSDB::setLastError(const string& sensor, const string& code, const string& errMessage, const string& sysMessage)
{
    ErrorInfo errorInfo;

    errorInfo.Code = code;
    errorInfo.ErrMessage = errMessage;
    errorInfo.SysMessage = sysMessage;

    m_LastError[sensor] = errorInfo;
}

ErrorInfo LightTSDB::GetLastError(const string& sensor)
{
    ErrorInfo emptyInfo;

    map<string, ErrorInfo>::const_iterator it = m_LastError.find(sensor);
    if(it==m_LastError.end()) return emptyInfo;
    return it->second;
}

inline bool LightTSDB::ends_with(string const& value, string const& ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

int LightTSDB::getValueSize(FileDataType valueType)
{
    switch(valueType)
    {
        case FileDataType::Undefined :
            return 0;
        case FileDataType::Float :
            return sizeof(float);
        case FileDataType::Int :
            return sizeof(int);
        case FileDataType::Double :
            return sizeof(double);
        case FileDataType::Bool :
            return sizeof(bool);
    }
    return 0;
}

/**************************************************************************************************************/
/***                                                                                                        ***/
/*** Class HourlyTimestamp                                                                                  ***/
/***                                                                                                        ***/
/**************************************************************************************************************/
HourlyTimestamp_t HourlyTimestamp::FromTimeT(time_t time)
{
    HourlyTimestamp_t hourlyTimestamp = time/3600;
    return hourlyTimestamp;
}

time_t HourlyTimestamp::ToTimeT(HourlyTimestamp_t hourlyTimestamp, HourlyOffset_t offset)
{
    time_t time = hourlyTimestamp*3600+offset;
    return time;
}

int HourlyTimestamp::ReadLastIndex(time_t& startHour, LtsdbFile* data, LtsdbFile* index, int valueSize)
{
    streampos pos;
    HourlyTimestamp_t hourIndex;

    index->Seekg(0, std::ios::end);
    pos = index->Tellg();
    pos -= (sizeof(hourIndex)+sizeof(streampos));
    index->Seekg(pos, std::ios::beg);
    hourIndex = index->ReadHourlyTimestamp();
    pos = index->ReadStreamOffset();
    index->Seekg(0, std::ios::end);

    int res = VerifyDataHourlyTimestamp(hourIndex, pos, data, valueSize);
    if(res!=0) return res;

    startHour = HourlyTimestamp::ToTimeT(hourIndex);
    return 0;
}

int HourlyTimestamp::VerifyDataHourlyTimestamp(HourlyTimestamp_t hourIndex, streampos pos, LtsdbFile* data, int valueSize)
{
    HourlyTimestamp_t hourData;

    data->Seekg(pos, std::ios::beg);
    hourData = data->ReadHourlyTimestamp();
    if(hourData!=hourIndex) return -1;

    HourlyOffset_t offset;
    HourlyOffset_t offsetMax = 0;
    UValue uvalue;

    while(data->ReadValue(&offset, &uvalue, valueSize)==true)
    {
        if(offset==ENDLINE) break;
        offsetMax = offset;
    }

    time_t tMax = HourlyTimestamp::ToTimeT(hourData, offsetMax);
    time_t tCur = time(0);
    if(tMax>tCur) return -2;

    data->Seekg(0, std::ios::end);
    return 0;
}

std::string HourlyTimestamp::ToString(HourlyTimestamp_t hourlyTimestamp)
{
    time_t ttime = HourlyTimestamp::ToTimeT(hourlyTimestamp);
    struct tm stime;
    ostringstream oss;

    localtime_r(&ttime, &stime);

    oss << 1900+stime.tm_year << "/" << 1+stime.tm_mon << "/" << stime.tm_mday << " " << stime.tm_hour << "h";
    return oss.str();
}

/**************************************************************************************************************/
/***                                                                                                        ***/
/*** Class ResamplingHelper                                                                                 ***/
/***                                                                                                        ***/
/**************************************************************************************************************/
void ResamplingHelper::Average(time_t timeBegin, const list<DataValue>& values, int interval, vector<AverageValue>& averages)
{
    averages.clear();
    if(values.size()==0) return;

    list<DataValue>::const_iterator it, itEnd;
    time_t tBegin, tEnd, tLast;
    float totValue, lastValue, minValue, maxValue;
    bool found = false;

    it = values.begin();
    itEnd = values.end();
    tBegin = timeBegin;
    tEnd = tBegin+interval;
    tLast = tBegin;
    totValue = 0;
    lastValue = it->value.Float;
    minValue = lastValue;
    maxValue = lastValue;

    while(it!=itEnd)
    {
        if(it->time<tEnd)
        {
            totValue += lastValue*(it->time-tLast);
            lastValue = it->value.Float;
            tLast = it->time;
            maxValue = max(maxValue, lastValue);
            minValue = min(minValue, lastValue);
            found = true;
            it++;
        }
        if((found)&&((it->time>=tEnd)||(it==itEnd)))
        {
            totValue += lastValue*(tEnd-tLast);
            averages.emplace_back(tBegin, minValue, maxValue, totValue/interval);
            tBegin += interval;
            tEnd += interval;
            tLast = tBegin;
            totValue = 0;
            minValue = lastValue;
            maxValue = lastValue;
        }
    }
}

void ResamplingHelper::PreserveExtremum(const vector<AverageValue>& averages, list<DataValue>& values)
{
    vector<AverageValue>::const_iterator it, itEnd, itBack, itNext;
    int i = 0;
    int aSize = averages.size();
    UValue uvalue;

    it = averages.begin();
    itEnd = averages.end();
    itBack = it;
    itNext = it;
    ++itNext;
    values.clear();

    while(it!=itEnd)
    {
        if((i==0)||(i==aSize))
        {
            uvalue.Float = it->average;
            values.emplace_back(it->time, uvalue);
        }
        else if((itBack->average>it->average)&&(it->average<itNext->average))
        {
            uvalue.Float = it->mini;
            values.emplace_back(it->time, uvalue);
        }
        else if((itBack->average<it->average)&&(it->average>itNext->average))
        {
            uvalue.Float = it->maxi;
            values.emplace_back(it->time, uvalue);
        }
        else
        {
            uvalue.Float = it->average;
            values.emplace_back(it->time, uvalue);
        }
        if(i>0) ++itBack;
        ++itNext;
        ++it;
        ++i;
    }
}

/**************************************************************************************************************/
/***                                                                                                        ***/
/*** Class LtsdbFile                                                                                        ***/
/***                                                                                                        ***/
/**************************************************************************************************************/
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

void LtsdbFile::Clear()
{
    m_InternalFile.clear();
}

bool LtsdbFile::Seekg(streamoff off, ios_base::seekdir way)
{
    if(!m_InternalFile.seekg(off, way)) return false;
    return true;
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

bool LtsdbFile::ReadValue(HourlyOffset_t* offset, void* pValue, int valueSize)
{
    *offset = 0;
    m_InternalFile.read(reinterpret_cast<char *>(offset), sizeof(HourlyOffset_t));
    if(*offset==ENDLINE) return true;
    m_InternalFile.read(reinterpret_cast<char *>(pValue), valueSize);
    return m_InternalFile.good();
}

bool LtsdbFile::WriteValue(HourlyOffset_t offset, void* pValue, int valueSize)
{
    m_InternalFile.write(reinterpret_cast<const char *>(&offset), sizeof(HourlyOffset_t));
    m_InternalFile.write(reinterpret_cast<const char *>(pValue), valueSize);
    return m_InternalFile.good();
}

bool LtsdbFile::WriteEndLine()
{
    m_InternalFile.write(reinterpret_cast<const char *>(&ENDLINE), sizeof(ENDLINE));
    return m_InternalFile.good();
}

bool LtsdbFile::ReadHeader(std::string* signature, uint8_t* version, FileDataType* type, uint8_t* options, FileState* state)
{
    int sigSize = SIGNATURE.size();
    char charsig[sigSize+1];

    *charsig = '\0';
    m_InternalFile.read(charsig, sigSize);
    charsig[sigSize] = 0;
    *signature = charsig;
    m_InternalFile.read(reinterpret_cast<char *>(version), sizeof(uint8_t));
    m_InternalFile.read(reinterpret_cast<char *>(type), sizeof(uint8_t));
    m_InternalFile.read(reinterpret_cast<char *>(options), sizeof(uint8_t));
    m_InternalFile.read(reinterpret_cast<char *>(state), sizeof(uint8_t));
    return m_InternalFile.good();
}

bool LtsdbFile::WriteHeader(std::string signature, const uint8_t version, const FileDataType type, const uint8_t options, const FileState state)
{
    m_InternalFile.write(signature.c_str(), signature.size());
    m_InternalFile.write(reinterpret_cast<const char *>(&version), sizeof(uint8_t));
    m_InternalFile.write(reinterpret_cast<const char *>(&type), sizeof(uint8_t));
    m_InternalFile.write(reinterpret_cast<const char *>(&options), sizeof(uint8_t));
    m_InternalFile.write(reinterpret_cast<const char *>(&state), sizeof(uint8_t));
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

bool LtsdbFile::Is_Open()
{
    return m_InternalFile.is_open();
}

}
