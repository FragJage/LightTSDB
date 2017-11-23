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
#include <cstring>
#include "LightTSDB.h"

using namespace std;

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
}

bool LightTSDB::WriteValue(string sensor, float value)
{
    FilesInfo* filesInfo;

    filesInfo = getFilesInfo(sensor);
    if(filesInfo == nullptr) return false;

    return true;
}

LightTSDB::FilesInfo* LightTSDB::getFilesInfo(string sensor)
{
    FilesInfo filesInfo;
    map<string,FilesInfo>::iterator it = m_FilesInfo.find(sensor);


    if(it != m_FilesInfo.end()) return &(it->second);

    filesInfo.data.open(m_Folder+"/"+sensor+".data", fstream::binary | fstream::in | fstream::app);
    if(!filesInfo.data)
    {
        filesInfo.data.open(m_Folder+"/"+sensor+".data", fstream::binary | fstream::in | fstream::trunc);
        if(!filesInfo.data)
        {
            m_LastError = strerror(errno);
            return nullptr;
        }

        filesInfo.index.open(m_Folder+"/"+sensor+".index", fstream::binary | fstream::in | fstream::trunc);
        if(!filesInfo.index)
        {
            m_LastError = strerror(errno);
            return nullptr;
        }

        //m_FilesInfo[sensor] = filesInfo;
        //m_FilesInfo.insert(std::pair<string,FilesInfo>(sensor,filesInfo));
        m_FilesInfo.insert ( std::make_pair(sensor, filesInfo));
        return &(m_FilesInfo[sensor]);
    }

    filesInfo.index.open(m_Folder+"/"+sensor+".index", fstream::binary | fstream::in | fstream::app);
    if(!filesInfo.index)
    {
        m_LastError = strerror(errno);
        return nullptr;
    }

    return nullptr;
}
