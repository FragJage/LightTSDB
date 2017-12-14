#include <cassert>
#include "LightTSDB.h"
#include "UnitTest/UnitTest.h"

using namespace std;

class TestOtherTypes : public TestClass<TestOtherTypes>
{
public:
    TestOtherTypes();
    ~TestOtherTypes();

    bool WriteBool();
    bool ReadBool();
    bool WriteInt();
    bool ReadInt();
    bool WriteDouble();
    bool ReadDouble();
};
