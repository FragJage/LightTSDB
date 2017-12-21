#include "TestLightTSDB.h"
#include "TimeMock.h"

using namespace std;

TestLightTSDB::TestLightTSDB() : TestClass("LightTSDB", this)
{
	addTest("CreateDB", &TestLightTSDB::CreateDB);
	addTest("OpenDB", &TestLightTSDB::OpenDB);
	addTest("WriteTimeValue", &TestLightTSDB::WriteTimeValue);
	addTest("WriteOldValue", &TestLightTSDB::WriteOldValue);
	addTest("WriteError", &TestLightTSDB::WriteError);
	addTest("ReadWithLimits", &TestLightTSDB::ReadWithLimits);
	addTest("ReadWithResample", &TestLightTSDB::ReadWithResample);
	addTest("ReadLastValue", &TestLightTSDB::ReadLastValue);
	addTest("Close", &TestLightTSDB::Close);
	addTest("IndexSearch", &TestLightTSDB::IndexSearch);
	addTest("GetSensorList", &TestLightTSDB::GetSensorList);
	addTest("CheckHeader", &TestLightTSDB::CheckHeader);
	addTest("CheckFiles", &TestLightTSDB::CheckFiles);
	addTest("CheckDate", &TestLightTSDB::CheckDate);
	addTest("Remove", &TestLightTSDB::Remove);

    LightTSDB::LightTSDB myTSDB;
    if(myTSDB.Remove("MySensor"))
        cout << termcolor::red << "Remove old data." << endl;
    if(myTSDB.Remove("Sensor2"))
        cout << termcolor::red << "Remove old data." << endl;
}

TestLightTSDB::~TestLightTSDB()
{
}

bool TestLightTSDB::CreateDB()
{
    LightTSDB::LightTSDB myTSDB;
    float temp = 20;
    int i;
    list<LightTSDB::DataValue> values;
    list<LightTSDB::DataValue>::const_iterator it, itEnd;

    SetMockTime(2017, 10, 24, 10, 2, 25);
    time(&m_start1);
    for(i=0; i<20; i++)
    {
        assert(true==myTSDB.WriteValue("MySensor", temp));
        if(i<10)
            temp += 0.5;
        else
            temp -= 0.5;
        MockAddSecond(60*6);
    }

    assert(true==myTSDB.ReadValues("MySensor", m_start1, values));
    assert(10==values.size());

    i = 0;
    it = values.begin();
    itEnd = values.end();
    while(it!=itEnd)
    {
        assert(20+i*0.5==it->value.Float);
        it++;
        i++;
    }

    return true;
}

bool TestLightTSDB::OpenDB()
{
    LightTSDB::LightTSDB myTSDB;
    float temp = 20;
    int i;
    list<LightTSDB::DataValue> values;
    list<LightTSDB::DataValue>::const_iterator it, itEnd;

    SetMockTime(2017, 10, 24, 12, 1, 5);
    time(&m_start2);
    for(i=0; i<20; i++)
    {
        assert(true==myTSDB.WriteValue("MySensor", temp));
        if(i<10)
            temp += 0.5;
        else
            temp -= 0.5;
        MockAddSecond(60*6);
    }

    assert(true==myTSDB.ReadValues("MySensor", m_start2+3600, values));
    assert(10==values.size());

    i = 0;
    it = values.begin();
    itEnd = values.end();
    while(it!=itEnd)
    {
        assert(25-i*0.5==it->value.Float);
        it++;
        i++;
    }

    return true;
}

bool TestLightTSDB::WriteTimeValue()
{
    LightTSDB::LightTSDB myTSDB;
    LightTSDB::ErrorInfo myError;

    SetMockTime(2017, 10, 24, 7, 2, 6);
    assert(true==myTSDB.WriteValue("Sensor2", 19.2f));
    SetMockTime(2017, 10, 24, 7, 11, 6);
    assert(true==myTSDB.WriteTimeValue("Sensor2", 20.5f, time(nullptr)-500));
    assert(false==myTSDB.WriteTimeValue("Sensor2", 20.1f, time(nullptr)-700));
    myError = myTSDB.GetLastError("Sensor2");
    assert("WRITE_MRV"==myError.Code);

    return true;
}

bool TestLightTSDB::WriteOldValue()
{
    LightTSDB::LightTSDB myTSDB;
    LightTSDB::ErrorInfo myError;

    SetMockTime(2017, 10, 24, 14, 1, 5);
    assert(true==myTSDB.WriteValue("Sensor2", 21.3f));
    SetMockTime(2017, 10, 24, 14, 10, 5);
    assert(true==myTSDB.WriteOldValue("Sensor2", 21.5f, 500));
    assert(false==myTSDB.WriteOldValue("Sensor2", 21.1f, 700));
    myError = myTSDB.GetLastError("Sensor2");
    assert("WRITE_MRV"==myError.Code);

    return true;
}

bool TestLightTSDB::WriteError()
{
    LightTSDB::LightTSDB myTSDB;
    LightTSDB::ErrorInfo myError;
    bool value = true;

    assert(false==myTSDB.WriteValue("Sensor2", value));
    myError = myTSDB.GetLastError("Sensor2");
    assert("MISMATCH"==myError.Code);

    return true;
}

bool TestLightTSDB::ReadWithLimits()
{
    int i;
    float temp;
    time_t start;
    LightTSDB::LightTSDB myTSDB;
    list<LightTSDB::DataValue> values;
    list<LightTSDB::DataValue>::const_iterator it, itEnd;

    start = m_start1+3600+3600/2;
    assert(true==myTSDB.ReadValues("MySensor", start, start+3600, values));
    assert(11==values.size());

    i = 0;
    it = values.begin();
    itEnd = values.end();
    temp = 23;
    while(it!=itEnd)
    {
        if(i < 6)
            temp -= 0.5;
        else
            temp += 0.5;

        assert(temp==it->value.Float);
        it++;
        i++;
    }
    return true;
}

bool TestLightTSDB::ReadWithResample()
{
    time_t start;
    float minVal, maxVal;
    LightTSDB::LightTSDB myTSDB;
    list<LightTSDB::DataValue> values;
    list<LightTSDB::DataValue>::const_iterator it, itEnd;

    start = m_start1-2*60-25;

    assert(true==myTSDB.ResampleValues("MySensor", start, start+3600*4, values, 60*10));
    it = values.begin();
    itEnd = values.end();
    minVal = 99;
    maxVal = -99;
    while(it!=itEnd)
    {
        minVal = min(minVal, it->value.Float);
        maxVal = max(maxVal, it->value.Float);
        it++;
    }

    assert(24==values.size());
    assert(20==minVal);
    assert(25==maxVal);

    assert(true==myTSDB.ResampleValues("MySensor", start+3600*10, start+3600*11, values, 60));
    assert(0==values.size());

    assert(false==myTSDB.ResampleValues("ZzSensor", start, start+3600, values, 60));
    LightTSDB::ErrorInfo myError = myTSDB.GetLastError("ZzSensor");
    assert("NOFILE"==myError.Code);
    return true;
}

bool TestLightTSDB::ReadLastValue()
{
    LightTSDB::LightTSDB myTSDB;
    LightTSDB::DataValue timeValue;

    assert(true==myTSDB.ReadLastValue("Sensor2", timeValue));
    assert(21.5==timeValue.value.Float);

    return true;
}

bool TestLightTSDB::Close()
{
    LightTSDB::LightTSDB myTSDB;

    MockAddSecond(60*6);
    assert(false==myTSDB.Close("MySensor"));
    assert(true==myTSDB.WriteValue("MySensor", 22.5f));
    assert(true==myTSDB.Close("MySensor"));

    return true;
}

bool TestLightTSDB::IndexSearch()
{
    LightTSDB::LightTSDB myTSDB;
    list<LightTSDB::DataValue> values;
    time_t start;

    SetMockTime(2017, 10, 25, 14, 2, 7);
    time(&start);
    assert(true==myTSDB.WriteValue("Sensor2", 21.3f));
    assert(true==myTSDB.ReadValues("Sensor2", start, values));
    assert(1==values.size());
    assert(true==myTSDB.ReadValues("Sensor2", start+3600, values));
    assert(0==values.size());
    MockAddSecond(3600);
    assert(true==myTSDB.WriteValue("Sensor2", 21.2f));
    MockAddSecond(3600*3);
    assert(true==myTSDB.WriteValue("Sensor2", 21.5f));
    MockAddSecond(3600);
    assert(true==myTSDB.WriteValue("Sensor2", 21.0f));
    assert(true==myTSDB.ReadValues("Sensor2", start+3600*4, values));
    assert(1==values.size());
    assert(21.5==values.begin()->value.Float);

    return true;
}

bool TestLightTSDB::GetSensorList()
{
    LightTSDB::LightTSDB myTSDB;
    list<string> sensorList;

    assert(true==myTSDB.GetSensorList(sensorList));
    assert(2==sensorList.size());

    sensorList.sort();
    list<string>::const_iterator it = sensorList.begin();
    assert("MySensor"==(*it));
    ++it;
    assert("Sensor2"==*it);
    return true;
}

bool TestLightTSDB::CheckHeader()
{
    LightTSDB::LightTSDB myTSDB;
    LightTSDB::ErrorInfo myError;

    myTSDB.SetFolder("test/data");

    assert(false==myTSDB.WriteValue("SignDataErr", 21.3f));
    myError = myTSDB.GetLastError("SignDataErr");
    assert("CHECK_SIG"==myError.Code);

    assert(false==myTSDB.WriteValue("StateIndexErr", 21.3f));
    myError = myTSDB.GetLastError("StateIndexErr");
    assert("CHECK_STA"==myError.Code);

    assert(false==myTSDB.WriteValue("VersionDataErr", 21.3f));
    myError = myTSDB.GetLastError("VersionDataErr");
    assert("CHECK_VER"==myError.Code);

    return true;
}

bool TestLightTSDB::CheckFiles()
{
    LightTSDB::LightTSDB myTSDB;
    LightTSDB::ErrorInfo myError;

    myTSDB.SetFolder("test/data");

    assert(false==myTSDB.WriteValue("TypeIndexErr", 21.3f));
    myError = myTSDB.GetLastError("TypeIndexErr");
    assert("OPEN_CHK1"==myError.Code);

    assert(false==myTSDB.WriteValue("LostData", 21.3f));
    myError = myTSDB.GetLastError("LostData");
    assert("OPEN_DAT2"==myError.Code);

    assert(false==myTSDB.WriteValue("LostIndex", 21.3f));
    myError = myTSDB.GetLastError("LostIndex");
    assert("OPEN_NDX2"==myError.Code);

    return true;
}

bool TestLightTSDB::CheckDate()
{
    LightTSDB::LightTSDB myTSDB;
    LightTSDB::ErrorInfo myError;

    myTSDB.SetFolder("test/data");

    assert(false==myTSDB.WriteValue("CorruptDate", 21.3f));
    myError = myTSDB.GetLastError("CorruptDate");
    assert("OPEN_COR1"==myError.Code);

    assert(false==myTSDB.WriteValue("TooNewDate", 21.3f));
    myError = myTSDB.GetLastError("TooNewDate");
    assert("OPEN_COR2"==myError.Code);

    return true;
}

bool TestLightTSDB::Remove()
{
    LightTSDB::LightTSDB myTSDB;
    assert(true==myTSDB.Remove("MySensor"));
    assert(true==myTSDB.Remove("Sensor2"));
    return true;
}
