#include <cassert>
#include "../tools/RebuildIndex.h"
#include "UnitTest/UnitTest.h"

using namespace std;

class TestTools : public TestClass<TestTools>
{
public:
    TestTools();
    ~TestTools();

    bool RebuildIndex();
};
