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
    assert(true==myFile.Open("./testOpenAndClose.dat"));
    assert(true==myFile.Is_Open());
    myFile.Close();
    assert(false==myFile.Is_Open());
    assert(0==remove("./testOpenAndClose.dat"));

    return true;
}

bool TestLtsdbFile::RWStreamOffset()
{
    LightTSDB::LtsdbFile myFile;
    streampos writePos = 394671;
    streampos movePos;


    assert(true==myFile.Open("./testRWStreamOffset.dat"));
    assert(true==myFile.WriteStreamOffset(writePos));
    myFile.Seekg(0, std::ios::end);
    movePos = myFile.Tellg();
    movePos-= sizeof(streampos);
    myFile.Seekg(movePos, std::ios::beg);
    assert(writePos==myFile.ReadStreamOffset());
    myFile.Close();
    assert(0==remove("./testRWStreamOffset.dat"));

    return true;
}

bool TestLtsdbFile::RWHourlyTimestamp()
{
    LightTSDB::LtsdbFile myFile;
    LightTSDB::HourlyTimestamp_t writeHt = LightTSDB::HourlyTimestamp::FromTimeT(time(0));
    streampos movePos;


    assert(true==myFile.Open("./testRWHourlyTimestamp.dat"));
    assert(true==myFile.WriteHourlyTimestamp(writeHt));
    myFile.Seekg(0, std::ios::end);
    movePos = myFile.Tellg();
    movePos-= sizeof(LightTSDB::HourlyTimestamp_t);
    myFile.Seekg(movePos, std::ios::beg);
    assert(writeHt==myFile.ReadHourlyTimestamp());
    myFile.Close();
    assert(0==remove("./testRWHourlyTimestamp.dat"));

    return true;
}

bool TestLtsdbFile::RWValue()
{
    LightTSDB::LtsdbFile myFile;
    LightTSDB::HourlyOffset_t writeHo = 1583;
    float writeValue = 25.45;
    LightTSDB::HourlyOffset_t readHo;
    float readValue;


    assert(true==myFile.Open("./testRWValue.dat"));
    assert(true==myFile.WriteValue(writeHo, writeValue));
    assert(true==myFile.WriteEndLine());
    myFile.Seekg(0, std::ios::beg);
    assert(true==myFile.ReadValue(&readHo, &readValue));
    assert(writeHo==readHo);
    assert(writeValue==readValue);
    assert(true==myFile.ReadValue(&readHo, &readValue));
    assert(LightTSDB::ENDLINE==readHo);
    myFile.Close();
    assert(0==remove("./testRWValue.dat"));

    return true;
}

bool TestLtsdbFile::RWHeader()
{
    LightTSDB::LtsdbFile myFile;
    string signature;
    uint8_t version;
    LightTSDB::FileDataType fileDataType;
    uint8_t options;
    LightTSDB::FileState fileState;


    assert(true==myFile.Open("./testRWHeader.dat"));
    assert(true==myFile.WriteHeader(LightTSDB::SIGNATURE, LightTSDB::VERSION, LightTSDB::FileDataType::Float, 0, LightTSDB::FileState::Stable));
    myFile.Seekg(0, std::ios::beg);
    assert(true==myFile.ReadHeader(&signature, &version, &fileDataType, &options, &fileState));
    assert(LightTSDB::SIGNATURE==signature);
    assert(LightTSDB::VERSION==version);
    assert(LightTSDB::FileDataType::Float==fileDataType);
    assert(0==options);
    assert(LightTSDB::FileState::Stable==fileState);
    myFile.Close();
    assert(0==remove("./testRWHeader.dat"));

    return true;
}
