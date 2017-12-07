#include "TestLightTSDB.h"
#include "TimeMock.h"

using namespace std;

TestLightTSDB::TestLightTSDB() : TestClass("LightTSDB", this)
{
	addTest("CreateDB", &TestLightTSDB::CreateDB);
	addTest("OpenDB", &TestLightTSDB::OpenDB);
	addTest("ReadWithLimits", &TestLightTSDB::ReadWithLimits);
	addTest("ReadWithResample", &TestLightTSDB::ReadWithResample);
	addTest("Close", &TestLightTSDB::Close);
	addTest("Remove", &TestLightTSDB::Remove);

    LightTSDB::LightTSDB myTSDB;
    if(myTSDB.Remove("MySensor"))
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
        assert(20+i*0.5==it->value);
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
        assert(25-i*0.5==it->value);
        it++;
        i++;
    }

    return true;
}

bool TestLightTSDB::ReadWithLimits()
{
    int i;
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
    while(it!=itEnd)
    {
        cout << it->value << " ";
        //if(i < 6)
        //assert(25-i*0.5==it->value);
        it++;
        i++;
    }
    return true;
}

bool TestLightTSDB::ReadWithResample()
{
    return false;
}

bool TestLightTSDB::Close()
{
    return false;
}

bool TestLightTSDB::Remove()
{
    LightTSDB::LightTSDB myTSDB;
    assert(true==myTSDB.Remove("MySensor"));
    return false;
}
