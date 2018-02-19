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
#include <fstream>
#include <mutex>

namespace LightTSDB {

typedef std::fstream ltsdb_fs_t;
typedef uint32_t HourlyTimestamp_t;
typedef uint16_t HourlyOffset_t;
enum FileState : uint8_t { Stable, Busy };
enum FileDataType : uint8_t { Undefined, Float, Int, Double, Bool };
union UValue
{
    float Float;
    int Int;
    double Double;
    bool Bool;
};

struct DataValue
{
    DataValue() : time(), value() {}
    DataValue(time_t t, UValue v) : time(t), value(v) {}
    time_t time;
    UValue value;
};

struct SensorInfo
{
    time_t minDate;
    time_t maxDate;
    uint8_t version;
    FileDataType type;
    uint8_t options;
};

struct ErrorInfo
{
    std::string Code;
    std::string ErrMessage;
    std::string SysMessage;
};

static const std::string LTSDB_SIGNATURE = "LTSDB";
static const uint8_t LTSDB_VERSION = 1;
static const uint16_t LTSDB_ENDLINE = 0XFFFE;

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
        bool ReadValue(HourlyOffset_t* offset, void* pValue, int valueSize);
        bool WriteValue(HourlyOffset_t offset, void* pValue, int valueSize);
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

        /// \brief    Details of a sensor
        /// \details  Get details of the sensor.
        /// \param    sensor       Name of sensor
        /// \return   True if sensor is found
        bool GetSensorInfo(const std::string& sensor, SensorInfo& sensorInfo);

        /// \brief    Write value into LightTSDB
        /// \details  Add a new value of a sensor into LightTSDB at current time.
        /// \param    sensor       Name of sensor
        /// \param    value        Value of sensor
        /// \return   True if value is write
        template<typename T>
        bool WriteValue(const std::string& sensor, T value);

        /// \brief    Write old value into LightTSDB
        /// \details  Add a new value of a sensor into LightTSDB at old time. (Only if none more recent value are wrote)
        /// \param    sensor       Name of sensor
        /// \param    value        Value of sensor
        /// \param    offset       Offset time in seconds
        /// \return   True if value is write
        template<typename T>
        bool WriteOldValue(const std::string& sensor, T value, uint32_t offset);

        /// \brief    Write old value into LightTSDB
        /// \details  Add a new value of a sensor into LightTSDB at old time. (Only if none more recent value are wrote)
        /// \param    sensor       Name of sensor
        /// \param    value        Value of sensor
        /// \param    time         Time
        /// \return   True if value is write
        template<typename T>
        bool WriteTimeValue(const std::string& sensor, T value, time_t time);

        /// \brief    Read values into LightTSDB
        /// \details  Read all values of a sensor into LightTSDB for an hour.
        /// \param    sensor       Name of sensor
        /// \param    hour         Hour
        /// \param    values       List of time/value
        /// \return   True if values are found
        bool ReadValues(const std::string& sensor, const time_t hour, std::list<DataValue>& values);

        /// \brief    Read values into LightTSDB
        /// \details  Read values of a sensor into LightTSDB between two times.
        /// \param    sensor       Name of sensor
        /// \param    hourBegin    Beginning hour
        /// \param    hourEnd      Ending hour
        /// \param    values       List of time/value
        /// \return   True if values are found
        bool ReadValues(const std::string& sensor, const time_t hourBegin, const time_t hourEnd, std::list<DataValue>& values);

        /// \brief    Read last value into LightTSDB
        /// \details  Read last value of a sensor into LightTSDB.
        /// \param    sensor       Name of sensor
        /// \param    value        Time/value struct
        /// \return   True if values are found
        bool ReadLastValue(const std::string& sensor, DataValue& value);

        /// \brief    Read and resample values into LightTSDB
        /// \details  Read and resample values of a sensor into LightTSDB between two times with a regular interval.
        /// \param    sensor       Name of sensor
        /// \param    hourBegin    Beginning hour
        /// \param    hourEnd      Ending hour
        /// \param    interval     Interval in seconds
        /// \param    values       List of time/value
        /// \param    nbValues     Number of values before resampling
        /// \return   True if values are found
        bool ResampleValues(const std::string& sensor, const time_t timeBegin, const time_t timeEnd, std::list<DataValue>& values, int interval, int* nbValues=nullptr);

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

    protected :
        enum FileType { data, index };
        std::string getFileName(const std::string& sensor, const FileType fileType);
        bool checkHeader(const std::string& sensor, const std::string& signature, uint8_t version, FileState state, FileType fileType);
        int getValueSize(FileDataType valueType);
		std::string getSystemErrorMsg(int errorNumber);

    private:
        struct FilesInfo
        {
            FilesInfo() : data(nullptr), index(nullptr), startHour(0), minHour(0), maxHour(0), maxOffset(0), indexSize(0), sensor(), version(0), type(FileDataType::Undefined), options(0), valueSize(0) {}
            FilesInfo(std::string _sensor) : data(nullptr), index(nullptr), startHour(0), minHour(0), maxHour(0), maxOffset(0), indexSize(0), sensor(_sensor), version(0), type(FileDataType::Undefined), options(0), valueSize(0) {}
            LtsdbFile* data;
            LtsdbFile* index;
            std::time_t startHour;
            HourlyTimestamp_t minHour;
            HourlyTimestamp_t maxHour;
            HourlyOffset_t maxOffset;
            std::streampos indexSize;
            std::string sensor;
            uint8_t version;
            FileDataType type;
            uint8_t options;
            int valueSize;
            std::mutex readMutex;
            std::mutex writeMutex;
        };

        FilesInfo* getFilesInfo(const std::string& sensor, FileDataType valueType);
        std::string getFileExt(const FileType fileType);
        void cleanUp(FilesInfo& pFileInfo);

        bool openFiles(FilesInfo& filesInfo);
        bool openDataFile(FilesInfo& filesInfo);
        bool openIndexFile(FilesInfo& filesInfo);
        bool createFiles(FilesInfo& filesInfo, FileDataType valueType);

        bool internalWriteValue(const std::string& sensor, void* value, FileDataType valueType);
        bool internalWriteOldValue(const std::string& sensor, void *value, uint32_t offset, FileDataType valueType);
        bool internalWriteTimeValue(const std::string& sensor, void* value, time_t time, FileDataType valueType);
        bool writeTimeValue(FilesInfo* filesInfo, void* pValue, time_t timestamp);
        std::streampos findIndex(FilesInfo* filesInfo, HourlyTimestamp_t hourlyTimestamp);

        bool checkSignature(const std::string& sensor, const std::string& signature, FileType fileType);
        bool checkVersion(const std::string& sensor, uint8_t version, FileType fileType);
        bool checkState(const std::string& sensor, FileState state, FileType fileType);
        void setLastError(const std::string& sensor, const std::string& code, const std::string& errMessage, const std::string& sysMessage="");

        bool ends_with(const std::string& value, const std::string& ending);

        std::string m_Folder;
        std::map<std::string, ErrorInfo> m_LastError;
        std::map<std::string, FilesInfo> m_FilesInfo;
        std::mutex m_FilesMap;
        std::mutex m_ErrorMap;
};

class HourlyTimestamp
{
    public:
        static HourlyTimestamp_t FromTimeT(time_t time);
        static time_t ToTimeT(HourlyTimestamp_t hourlyTimestamp, HourlyOffset_t offset=0);
        static int ReadLastIndex(time_t& startHour, LtsdbFile* data, LtsdbFile* index, int valueSize);
        static std::string ToString(HourlyTimestamp_t hourlyTimestamp);
    private:
        static int VerifyDataHourlyTimestamp(HourlyTimestamp_t hourIndex, std::streampos pos, LtsdbFile* data, int valueSize);
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
        static void Average(time_t timeBegin, const std::list<DataValue>& values, int interval, std::vector<AverageValue>& averages);
        static void PreserveExtremum(const std::vector<AverageValue>& averages, std::list<DataValue>& values);
};

}

namespace MOCK {
	time_t time(time_t* ptr);
}

#endif // LIGHTTSDB_H
