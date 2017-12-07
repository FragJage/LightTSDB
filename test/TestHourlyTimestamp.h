#include <cassert>
#include "LightTSDB.h"
#include "UnitTest/UnitTest.h"

using namespace std;

class TestHourlyTimestamp : public TestClass<TestHourlyTimestamp>
{
public:
    TestHourlyTimestamp();
    ~TestHourlyTimestamp();

    bool FromTimeT();
    bool ToTimeT();
    bool ToString();
};
