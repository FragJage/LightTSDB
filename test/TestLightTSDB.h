#include <cassert>
#include "LightTSDB.h"
#include "UnitTest/UnitTest.h"

using namespace std;

class TestLightTSDB : public TestClass<TestLightTSDB>
{
public:
    TestLightTSDB();
    ~TestLightTSDB();

    bool CreateDB();
    bool OpenDB();
    bool ReadWithLimits();
    bool ReadWithResample();
    bool Close();
    bool Remove();
private:
    time_t m_start1;
    time_t m_start2;
};
