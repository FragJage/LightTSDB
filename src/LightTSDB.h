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
///     LightTSDB myTSDB;
///
///     myTSDB.WriteValue("LucileBedRoom", 24.8);
///     myTSDB.Flush();
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
#include <fstream>
#include <ctime>

/// \brief    Light time series class.
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

        /// \brief    Write value into LightTSDB
        /// \details  Add a new value of a sensor into LightTSDB.
        /// \param    sensor       Name of sensor
        /// \param    value        Value of sensor
        /// \return   Iterator on the first key in the section
        bool WriteValue(std::string sensor, float value);

        /// \brief    Get last error
        /// \details  Get the last error message.
        /// \return   Error message string
        std::string GetLastError();

    private:
        struct FilesInfo
        {
            std::fstream* data;
            std::fstream* index;
            std::time_t startHour;
        };

        typedef uint32_t HourlyTimestamp_t;
        typedef uint16_t HourlyOffset_t;
        friend class HourlyTimestamp;
        friend class StreamOffset;
        friend class HourlyOffset;

        FilesInfo* getFilesInfo(std::string sensor);

        std::string m_Folder;
        std::string m_LastError;
        std::map<std::string, FilesInfo> m_FilesInfo;

        static const short ENDLINE;
};

class HourlyTimestamp
{
    public:
        static LightTSDB::HourlyTimestamp_t FromTimeStruct(struct tm* tmHour);
        static void ToTimeStruct(struct tm* tmHour, LightTSDB::HourlyTimestamp_t hourlyTimestamp);
        static std::time_t ReadLastIndex(std::fstream* pIndexFile, std::fstream* pDataFile);
        static bool Write(LightTSDB::HourlyTimestamp_t hourlyTimestamp, std::fstream* pFile);
        static LightTSDB::HourlyTimestamp_t Read(std::fstream* pFile);
        static std::string ToString(LightTSDB::HourlyTimestamp_t hourlyTimestamp);
    private:
        static int VerifyDataHourlyTimestamp(LightTSDB::HourlyTimestamp_t hourIndex, std::streampos pos, std::fstream *pDataFile);
};

class StreamOffset
{
    public:
        static bool Write(std::fstream* pDataFile, std::fstream* pIndexFile);
        static std::streampos Read(std::fstream* pIndexFile);
};

class HourlyOffset
{
    public:
        static bool Read(std::fstream* pDataFile, LightTSDB::HourlyOffset_t* offset, float* value);
        static bool Write(std::fstream* pDataFile, LightTSDB::HourlyOffset_t offset, float value);
        static bool WriteEndLine(std::fstream* pDataFile);
};

#endif // LIGHTTSDB_H
