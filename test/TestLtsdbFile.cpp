#include "TestLtsdbFile.h"

using namespace std;

TestLtsdbFile::TestLtsdbFile() : TestClass("LtsdbFile", this)
{
	addTest("OpenAndClose", &TestLtsdbFile::OpenAndClose);
	addTest("RWStreamOffset", &TestLtsdbFile::RWStreamOffset);
	addTest("RWHourlyTimestamp", &TestLtsdbFile::RWHourlyTimestamp);
	addTest("RWValue", &TestLtsdbFile::RWValue);
	addTest("RWHeader", &TestLtsdbFile::RWHeader);
}

TestLtsdbFile::~TestLtsdbFile()
{
}

bool TestLtsdbFile::OpenAndClose()
{
    LightTSDB::LtsdbFile myFile;


    assert(false==myFile.Is_Open());
    assert(true==myFile.Open("./testfile.dat"));
    assert(true==myFile.Is_Open());
    myFile.Close();
    assert(false==myFile.Is_Open());
    assert(0==remove("./testfile.dat"));

    return true;
}

bool TestLtsdbFile::RWStreamOffset()
{
    return true;
}

bool TestLtsdbFile::RWHourlyTimestamp()
{
    return true;
}

bool TestLtsdbFile::RWValue()
{
    return true;
}

bool TestLtsdbFile::RWHeader()
{
    return true;
}
