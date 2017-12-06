#include <cassert>
#include "LightTSDB.h"
#include "UnitTest/UnitTest.h"

using namespace std;

class TestLtsdbFile : public TestClass<TestLtsdbFile>
{
public:
    TestLtsdbFile();
    ~TestLtsdbFile();

    bool OpenAndClose();
    bool RWStreamOffset();
    bool RWHourlyTimestamp();
    bool RWValue();
    bool RWHeader();
};
