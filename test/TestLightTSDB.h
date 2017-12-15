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
    bool WriteOldValue();
    bool WriteError();
    bool ReadWithLimits();
    bool ReadWithResample();
    bool ReadLastValue();
    bool Close();
    bool IndexSearch();
    bool GetSensorList();
    bool CheckHeader();
    bool CheckFiles();
    bool CheckDate();
    bool Remove();
private:
    time_t m_start1;
    time_t m_start2;
};
