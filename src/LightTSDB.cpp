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

void LightTSDB::SetFolder(const string& folder)
{
    m_Folder = folder;
}

bool LightTSDB::WriteValue(const string& sensor, float value)
{
    FilesInfo* filesInfo;
    time_t now;


    filesInfo = getFilesInfo(sensor);
    if(filesInfo == nullptr) return false;

    time(&now);

    if(difftime(now, filesInfo->startHour)>3599)
    {
        if(filesInfo->startHour>0) filesInfo->data->WriteEndLine();

        HourlyTimestamp_t hourlyTimestamp = HourlyTimestamp::FromTimeT(now);
        filesInfo->startHour = HourlyTimestamp::ToTimeT(hourlyTimestamp);
        filesInfo->index->WriteHourlyTimestamp(hourlyTimestamp);
        filesInfo->index->WriteStreamOffset(filesInfo->data->Tellp());
        filesInfo->data->WriteHourlyTimestamp(hourlyTimestamp);
    }

    HourlyOffset_t offset = difftime(now, filesInfo->startHour);
    filesInfo->data->WriteValue(offset, value);

    return true;
}

bool LightTSDB::ReadValues(const std::string& sensor, time_t hour, std::list<DataValue> values)
{
    return true;
}

bool LightTSDB::ReadValues(const std::string& sensor, time_t hourBegin, time_t hourEnd, std::list<DataValue> values)
{
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

    m_FilesInfo[sensor] = filesInfo;
    return &(m_FilesInfo[sensor]);
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
    HourlyTimestamp_t hourlyTimestamp;

    if(!openDataFile(filesInfo)) return false;
    if(!openIndexFile(filesInfo)) return false;

    hourlyTimestamp = HourlyTimestamp::ReadLastIndex(filesInfo.index, filesInfo.data);
    filesInfo.startHour = HourlyTimestamp::ToTimeT(hourlyTimestamp);
    if(filesInfo.startHour == -1)
    {
        setLastError(filesInfo.sensor, "OPEN_COR1", "Index file is corrupt.");
        return false;
    }
    if(filesInfo.startHour == -2)
    {
        setLastError(filesInfo.sensor, "OPEN_COR2", "Data file is corrupt.");
        return false;
    }

    return true;
}

bool LightTSDB::openDataFile(FilesInfo& filesInfo)
{
    string signature;
    FileState fileState;

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
    if(!checkHeader(filesInfo.sensor, signature, filesInfo.version, fileState, FileType::data)) return false;

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

    filesInfo.index->Seekg(0, std::ios::end);
    return true;
}

bool LightTSDB::createFiles(LightTSDB::FilesInfo& filesInfo)
{
    filesInfo.data = new LtsdbFile();
    if(!filesInfo.data->Open(getFileName(filesInfo.sensor, data)))
    {
        setLastError(filesInfo.sensor, "CREATE_DAT1", "Unable to create data file.", strerror(errno));
        return false;
    }
    if(!filesInfo.data->WriteHeader(SIGNATURE, VERSION, FileDataType::Float, 0, FileState::Stable))
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
    if(!filesInfo.index->WriteHeader(SIGNATURE, VERSION, FileDataType::Float, 0, FileState::Stable))
    {
        setLastError(filesInfo.sensor, "CREATE_NDX2", "Unable to write header of index file.", strerror(errno));
        return false;
    }

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
HourlyTimestamp_t HourlyTimestamp::FromTimeT(time_t time)
{
    HourlyTimestamp_t hourlyTimestamp = time/3600;
    return hourlyTimestamp;
}

time_t HourlyTimestamp::ToTimeT(HourlyTimestamp_t hourlyTimestamp)
{
    time_t time = hourlyTimestamp*3600;
    return time;
}

HourlyTimestamp_t HourlyTimestamp::ReadLastIndex(LtsdbFile* pIndexFile, LtsdbFile* pDataFile)
{
    streampos pos;
    HourlyTimestamp_t hourIndex;

    pIndexFile->Seekg(0, std::ios::end);
    pos = pIndexFile->Tellg();
    pos -= (sizeof(hourIndex)+sizeof(streampos));
    pIndexFile->Seekg(pos, std::ios::beg);
    hourIndex = pIndexFile->ReadHourlyTimestamp();
    pos = pIndexFile->ReadStreamOffset();
    pIndexFile->Seekg(0, std::ios::end);

    int ret = VerifyDataHourlyTimestamp(hourIndex, pos, pDataFile);
    if(ret<0) return ret;

    return hourIndex;
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

    while(pDataFile->ReadValue(&offset, &value)==true)
    {
        if(offset==ENDLINE) break;
        offsetMax = offset;
    }

    time_t tMax = HourlyTimestamp::ToTimeT(hourData)+offsetMax;
    time_t tCur = time(0);

    if(tMax>tCur) return -2;

    pDataFile->Seekg(0, std::ios::end);
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

bool LtsdbFile::ReadValue(HourlyOffset_t* offset, float* value)
{
    m_InternalFile.read(reinterpret_cast<char *>(offset), sizeof(HourlyOffset_t));
    if(*offset==ENDLINE) return true;
    m_InternalFile.read(reinterpret_cast<char *>(value), sizeof(float));
    return m_InternalFile.good();
}

bool LtsdbFile::WriteValue(HourlyOffset_t offset, float value)
{
    m_InternalFile.write(reinterpret_cast<const char *>(&offset), sizeof(HourlyOffset_t));
    m_InternalFile.write(reinterpret_cast<const char *>(&value), sizeof(float));
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
    char* charsig = new char[sigSize+1];

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

#endif
}
