[![Build Status](https://travis-ci.org/FragJage/LightTSDB.svg?branch=master)](https://travis-ci.org/FragJage/LightTSDB)
[![Build status](https://ci.appveyor.com/api/projects/status/8dq5ss34d7w4xdcu?svg=true)](https://ci.appveyor.com/project/FragJage/lighttsdb)
[![Coverage Status](https://coveralls.io/repos/github/FragJage/LightTSDB/badge.svg?branch=master&bust=1)](https://coveralls.io/github/FragJage/LightTSDB?branch=master)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/6a37708054fd4386a781767cf71166fc)](https://www.codacy.com/app/FragJage/LightTSDB?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=FragJage/LightTSDB&amp;utm_campaign=Badge_Grade)

LightTSDB
=========
Light time series database class.

Introduction
============
This class store time series into the file system and can read float values by hours.

Features
========
 - Choice of the store folder
 - Indexed read
 - Supported type float, double, int, bool.
 - Compile on Linux and Windows, Intel or ARM.

Portability
===========
Unit tests passed successfully on :
 - Windows Seven (CPU Intel Celeron)
 - Linux Ubuntu (CPU Intel Atom)
 - (TO DO) Linux Raspian on Raspberry Pi (CPU ARM)
 - (TO DO) Linux FunPlug on NAS DNS-320 (CPU ARM)

How to use
==========
 Add src into your project and see examples folder. For msvc add dependency/dirent folder.
 
 To build, you can see CMakeLists.txt, Code::Blocks or MSVC projects in builds folder.
 
 For example :
    
		int main()
		{
			LightTSDB::LightTSDB myTSDB;
			LightTSDB::DataValue lastValue;
		    list<LightTSDB::DataValue> myValues;


			///*** Write float value
			float temperature = 22.35f;
			myTSDB.WriteValue("BedRoomTemperature", temperature);
			
			///*** Read last float value
			myTSDB.ReadLastValue("BedRoomTemperature", lastValue);

			///*** Read values for hour
			myTSDB.ReadValues("BedRoomTemperature", time(), myValues);
			
			return 0;
		}

Licence
=======
LightTSDB is free software : you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

LightTSDB is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with LightTSDB. If not, see http://www.gnu.org/licenses/.
