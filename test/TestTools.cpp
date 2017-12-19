#include <sstream>
#include "TestTools.h"

using namespace std;

TestTools::TestTools() : TestClass("Tools", this)
{
	addTest("RebuildIndex", &TestTools::RebuildIndex);
}

TestTools::~TestTools()
{
}

bool TestTools::RebuildIndex()
{
    LightTSDB::RebuildIndex myRebuilder;
    myRebuilder.SetFolder("./data/");
    assert(true==myRebuilder.Rebuild("LucileBedRoomTemperature"));
    return true;
}
