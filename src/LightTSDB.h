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

/*** MAIN PAGE FOR DOXYGEN *************************************************************************/
/// \mainpage LightTSDB Class Documentation
/// \section intro_sec Introduction
///
/// This class store time series into the file system and can read float values by hours.\n
/// To use, include in your project LightTSDB.cpp and LightTSDB.h.
///
/// \section feature_sec Features
///
/// \li Choice of the store folder
/// \li Indexed read
/// \li Compressed file
/// \li Compile on Linux and Windows, Intel or ARM.
///
/// \section portability_sec Portability
/// Unit tests passed successfully on :
/// \li Windows Seven (CPU Intel Celeron)
/// \li Linux Ubuntu (CPU Intel Atom)
/// \li Linux Raspian on Raspberry Pi (CPU ARM)
/// \li Linux FunPlug on NAS DNS-320 (CPU ARM)\n
/// (Compilation directives define LINUX or WIN only necessary for colours in unit tests)
///
/// \section example_sec Example
/// \code
/// #include <iostream>
/// #include "LightTSDB.h"
///
/// using namespace std;
///
/// int main()
/// {
///     LightTSDB::LightTSDB myTSDB;
///
///     myTSDB.WriteValue("TomBedRoomTemperature", 24.8);
///
///     return 0;
/// }
/// \endcode
///
/// \section licence_sec Licence
///  LightTSDB is free software : you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.\n\n
///  LightTSDB is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.\n\n
///  You should have received a copy of the GNU General Public License along with LightTSDB. If not, see <http://www.gnu.org/licenses/>.
///
/***************************************************************************************************/

#ifndef LIGHTTSDB_H
#define LIGHTTSDB_H

#include <string>
#include <map>
#include <list>
#include <vector>
#include <ctime>
#ifdef HAVE_LIBUV
    #include "uvw.hpp"
#else
    #include <fstream>
#endif

namespace LightTSDB {

#ifdef HAVE_LIBUV
    typedef uv_fs_t ltsdb_fs_t;
#else
    typedef std::fstream ltsdb_fs_t;
#endif

typedef uint32_t HourlyTimestamp_t;
typedef uint16_t HourlyOffset_t;
enum FileState : uint8_t { Stable, Busy };
enum FileDataType : uint8_t { Undefined, Float };

struct DataValue
{
    DataValue() : time(), value() {}
    DataValue(time_t t, float v) : time(t), value(v) {}
    time_t time;
    float value;
};

static const std::string SIGNATURE = "LTSDB";
static const uint8_t VERSION = 1;
static const uint16_t ENDLINE = 0XFFFE;

static const int INDEX_STEP = sizeof(HourlyTimestamp_t)+sizeof(std::streampos);
static const int HEADER_SIZE = SIGNATURE.size()+sizeof(VERSION)+sizeof(FileDataType)+sizeof(uint8_t)+sizeof(FileState);

class LtsdbFile
{
    public:
        LtsdbFile();
        ~LtsdbFile();
        bool Open(const std::string& fileName);
        bool Is_Open();
        void Close();
        void Clear();
        bool Seekp(std::streamoff off, std::ios_base::seekdir way);
        bool Seekg(std::streamoff off, std::ios_base::seekdir way);
        std::streampos Tellp();
        std::streampos Tellg();
        bool WriteStreamOffset(std::streampos pos);
        std::streampos ReadStreamOffset();
        bool WriteHourlyTimestamp(HourlyTimestamp_t hourlyTimestamp);
        HourlyTimestamp_t ReadHourlyTimestamp();
        bool ReadValue(HourlyOffset_t* offset, float* value);
        bool WriteValue(HourlyOffset_t offset, float value);
        bool WriteEndLine();
        bool ReadHeader(std::string* signature, uint8_t* version, FileDataType* type, uint8_t* options, FileState* state);
        bool WriteHeader(std::string signature, const uint8_t version, const FileDataType type, const uint8_t options, const FileState state);
        static bool FileExists(const std::string& fileName);
    private:
        ltsdb_fs_t m_InternalFile;
};

/// \brief    Light time series database class.
/// \details  This class store time series into the file system and can read float values by hours.
class LightTSDB
{
    public:
        struct ErrorInfo
        {
            std::string Code;
            std::string ErrMessage;
            std::string SysMessage;
        };

        /// \brief    Constructor of LightTSDB
        /// \details  Constructor of LightTSDB.
        LightTSDB();

        /// \brief    Destructor of LightTSDB
        /// \details  Destructor of LightTSDB.
        ~LightTSDB();

        /// \brief    Set LightTSDB files folder
        /// \details  Set the folder where data and index files of LightTSDB will be stored.
        /// \param    folder       The folder
        void SetFolder(const std::string& folder);

        /// \brief    Sensors list into LightTSDB
        /// \details  Get the list of the sensors created into LightTSDB.
        /// \param    sensorList       List of sensors
        /// \return   True if sensors are found
        bool GetSensorList(std::list<std::string>& sensorList);

        /// \brief    Write value into LightTSDB
        /// \details  Add a new value of a sensor into LightTSDB at current time.
        /// \param    sensor       Name of sensor
        /// \param    value        Value of sensor
        /// \return   True if value is write
        bool WriteValue(const std::string& sensor, float value);

        /// \brief    Read values into LightTSDB
        /// \details  Read all values of a sensor into LightTSDB for an hour.
        /// \param    sensor       Name of sensor
        /// \param    hour         Hour
        /// \param    values       List of time/value
        /// \return   True if values are found
        bool ReadValues(const std::string& sensor, time_t hour, std::list<DataValue>& values);

        /// \brief    Read values into LightTSDB
        /// \details  Read values of a sensor into LightTSDB between two times.
        /// \param    sensor       Name of sensor
        /// \param    hourBegin    Beginning hour
        /// \param    hourEnd      Ending hour
        /// \param    values       List of time/value
        /// \return   True if values are found
        bool ReadValues(const std::string& sensor, time_t hourBegin, time_t hourEnd, std::list<DataValue>& values);

        /// \brief    Read and resample values into LightTSDB
        /// \details  Read and resample values of a sensor into LightTSDB between two times with a reguler interval.
        /// \param    sensor       Name of sensor
        /// \param    hourBegin    Beginning hour
        /// \param    hourEnd      Ending hour
        /// \param    interval     Interval in seconds
        /// \param    values       List of time/value
        /// \return   True if values are found
        bool ResampleValues(const std::string& sensor, time_t timeBegin, time_t timeEnd, std::list<DataValue>& values, int interval);

        /// \brief    Close LightTSDB files
        /// \details  Close LightTSDB files (data and index) for a sensor.
        /// \param    sensor       The sensor
        /// \return   True if files are closed
        bool Close(const std::string& sensor);

        /// \brief    Remove LightTSDB files
        /// \details  Close and remove LightTSDB files (data and index) for a sensor.
        /// \param    sensor       The sensor
        /// \return   True if files are removed
        bool Remove(const std::string& sensor);

        /// \brief    Get last error
        /// \details  Get the last error.
        /// \return   Error message and error code
        ErrorInfo GetLastError(const std::string& sensor);

    private:
        struct FilesInfo
        {
            FilesInfo() : data(nullptr), index(nullptr), startHour(0), minHour(0), maxHour(0), indexSize(0), sensor(), version(0), type(FileDataType::Undefined), options(0) {}
            FilesInfo(std::string sensor) : data(nullptr), index(nullptr), startHour(0), minHour(0), maxHour(0), indexSize(0), sensor(sensor), version(0), type(FileDataType::Undefined), options(0) {}
            LtsdbFile* data;
            LtsdbFile* index;
            std::time_t startHour;
            HourlyTimestamp_t minHour;
            HourlyTimestamp_t maxHour;
            std::streampos indexSize;
            std::string sensor;
            uint8_t version;
            FileDataType type;
            uint8_t options;
        };

        enum FileType { data, index };

        std::string getFileName(const std::string& sensor, const FileType fileType);
        std::string getFileExt(const FileType fileType);

        FilesInfo* getFilesInfo(const std::string& sensor);
        void cleanUp(FilesInfo* pFileInfo);

        bool openFiles(FilesInfo& filesInfo);
        bool openDataFile(FilesInfo& filesInfo);
        bool openIndexFile(FilesInfo& filesInfo);
        bool createFiles(FilesInfo& filesInfo);

        std::streampos findIndex(FilesInfo* filesInfo, HourlyTimestamp_t hourlyTimestamp);

        bool checkHeader(const std::string& sensor, const std::string& signature, uint8_t version, FileState state, FileType fileType);
        bool checkSignature(const std::string& sensor, const std::string& signature, FileType fileType);
        bool checkVersion(const std::string& sensor, uint8_t version, FileType fileType);
        bool checkState(const std::string& sensor, FileState state, FileType fileType);
        void setLastError(const std::string& sensor, const std::string& code, const std::string& errMessage, const std::string& sysMessage="");

        std::string m_Folder;
        std::map<std::string, ErrorInfo> m_LastError;
        std::map<std::string, FilesInfo> m_FilesInfo;
};

class HourlyTimestamp
{
    public:
        static HourlyTimestamp_t FromTimeT(time_t time);
        static time_t ToTimeT(HourlyTimestamp_t hourlyTimestamp, HourlyOffset_t offset=0);
        static HourlyTimestamp_t ReadLastIndex(LtsdbFile* pIndexFile, LtsdbFile* pDataFile);
        static std::string ToString(HourlyTimestamp_t hourlyTimestamp);
    private:
        static int VerifyDataHourlyTimestamp(HourlyTimestamp_t hourIndex, std::streampos pos, LtsdbFile* pDataFile);
};

class ResamplingHelper
{
    public:
        struct AverageValue
        {
            AverageValue() : time(), mini(0), maxi(0), average(0) {}
            AverageValue(time_t t, float mi, float ma, float av) : time(t), mini(mi), maxi(ma), average(av) {}
            time_t time;
            float mini;
            float maxi;
            float average;
        };
        static void Average(time_t timeBegin, time_t timeEnd, const std::list<DataValue>& values, int interval, std::vector<AverageValue>& averages);
        static void PreserveExtremum(const std::vector<AverageValue>& averages, std::list<DataValue>& values);
};

}
#endif // LIGHTTSDB_H
