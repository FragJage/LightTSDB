#include "TestOtherTypes.h"
#include "TimeMock.h"

using namespace std;

TestOtherTypes::TestOtherTypes() : TestClass("OtherTypes", this), m_LastTime(0)
{
	addTest("WriteBool", &TestOtherTypes::WriteBool);
	addTest("ReadBool", &TestOtherTypes::ReadBool);
	addTest("WriteInt", &TestOtherTypes::WriteInt);
	addTest("ReadInt", &TestOtherTypes::ReadInt);
	addTest("WriteDouble", &TestOtherTypes::WriteDouble);
	addTest("ReadDouble", &TestOtherTypes::ReadDouble);

    LightTSDB::LightTSDB myTSDB;
    if(myTSDB.Remove("BoolSensor"))
        cout << termcolor::red << "Remove old data." << endl;
    if(myTSDB.Remove("IntSensor"))
        cout << termcolor::red << "Remove old data." << endl;
    if(myTSDB.Remove("DoubleSensor"))
        cout << termcolor::red << "Remove old data." << endl;
}

TestOtherTypes::~TestOtherTypes()
{
    LightTSDB::LightTSDB myTSDB;
    if(!myTSDB.Remove("BoolSensor"))
        cout << termcolor::red << "Unable to remove BoolSensor file." << endl;
    if(!myTSDB.Remove("IntSensor"))
        cout << termcolor::red << "Unable to remove IntSensor file." << endl;
    if(!myTSDB.Remove("DoubleSensor"))
        cout << termcolor::red << "Unable to remove DoubleSensor file." << endl;
}

bool TestOtherTypes::WriteBool()
{
    LightTSDB::LightTSDB myTSDB;
    bool bval;

    SetMockTime(2017, 10, 26, 15, 13, 8);
    MOCK::time(&m_LastTime);
    bval = true;
    assert(true==myTSDB.WriteOldValue("BoolSensor", bval, 500));
    bval = false;
    assert(true==myTSDB.WriteTimeValue("BoolSensor", bval, m_LastTime-250));
    bval = true;
    assert(true==myTSDB.WriteValue("BoolSensor", bval));

    return true;
}

bool TestOtherTypes::ReadBool()
{
    LightTSDB::LightTSDB myTSDB;
    list<LightTSDB::DataValue> dataValues;
    list<LightTSDB::DataValue>::const_iterator it;

    assert(true==myTSDB.ReadValues("BoolSensor", m_LastTime, dataValues));
    assert(3==dataValues.size());

    it = dataValues.begin();
    assert(true==it->value.Bool);
    ++it;
    assert(false==it->value.Bool);
    ++it;
    assert(true==it->value.Bool);

    return true;
}

bool TestOtherTypes::WriteInt()
{
    LightTSDB::LightTSDB myTSDB;
    int ival;

    SetMockTime(2017, 10, 27, 16, 24, 9);
    ival = 1234567898;
    assert(true==myTSDB.WriteOldValue("IntSensor", ival, 900));
    ival = 1234567900;
    assert(true==myTSDB.WriteTimeValue("IntSensor", ival, MOCK::time(nullptr)-400));
    ival = 1234567902;
    assert(true==myTSDB.WriteValue("IntSensor", ival));

    return true;
}

bool TestOtherTypes::ReadInt()
{
    LightTSDB::LightTSDB myTSDB;
    LightTSDB::DataValue dataValues;

    assert(true==myTSDB.ReadLastValue("IntSensor", dataValues));
    assert(1234567902==dataValues.value.Int);

    return true;
}

bool TestOtherTypes::WriteDouble()
{
    LightTSDB::LightTSDB myTSDB;
    double dval;

    SetMockTime(2017, 10, 28, 15, 33, 8);
    MOCK::time(&m_LastTime);
    dval = 2.123456789;
    assert(true==myTSDB.WriteValue("DoubleSensor", dval));
    MockAddSecond(600);
    dval = 3.987654321;
    assert(true==myTSDB.WriteOldValue("DoubleSensor", dval, 400));
    MockAddSecond(600);
    dval = 5.546372819;
    assert(true==myTSDB.WriteTimeValue("DoubleSensor", dval, m_LastTime+400));

    return true;
}

bool TestOtherTypes::ReadDouble()
{
    LightTSDB::LightTSDB myTSDB;
    list<LightTSDB::DataValue> dataValues;
    list<LightTSDB::DataValue>::const_iterator it;

    assert(true==myTSDB.ReadValues("DoubleSensor", m_LastTime, m_LastTime+1000, dataValues));
    assert(3==dataValues.size());

    it = dataValues.begin();
    assert(2.123456789==it->value.Double);
    ++it;
    assert(3.987654321==it->value.Double);
    ++it;
    assert(5.546372819==it->value.Double);

    return true;
}
