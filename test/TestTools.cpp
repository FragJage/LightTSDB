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
    LightTSDB::LtsdbFile backup;
    LightTSDB::LtsdbFile rebuild;

    string filename_backup = "test/data/LucileBedRoomTemperature.index.bak";
    string filename_rebuild = "test/data/LucileBedRoomTemperature.index";
    string signature_backup, signature_rebuild;
    uint8_t version_backup, version_rebuild;
    LightTSDB::FileDataType type_backup, type_rebuild;
    uint8_t options_backup, options_rebuild;
    LightTSDB::FileState state_backup, state_rebuild;
    LightTSDB::HourlyTimestamp_t hour_backup, hour_rebuild;
    streampos pos_backup, pos_rebuild, pos_end, pos_cur;


    myRebuilder.SetFolder("test/data");
    assert(true==myRebuilder.Rebuild("LucileBedRoomTemperature"));

    assert(true==backup.Open(filename_backup));
    assert(true==rebuild.Open(filename_rebuild));

    backup.Seekg(0, ios::end);
    pos_end = backup.Tellg();
    backup.Seekg(0, ios::beg);

    assert(true==backup.ReadHeader(&signature_backup, &version_backup, &type_backup, &options_backup, &state_backup));
    assert(true==rebuild.ReadHeader(&signature_rebuild, &version_rebuild, &type_rebuild, &options_rebuild, &state_rebuild));
    assert(signature_backup==signature_rebuild);
    assert(version_backup==version_rebuild);
    assert(type_backup==type_rebuild);
    assert(options_backup==options_rebuild);

    do
    {
        hour_backup = backup.ReadHourlyTimestamp();
        pos_backup = backup.ReadStreamOffset();
        hour_rebuild = rebuild.ReadHourlyTimestamp();
        pos_rebuild = rebuild.ReadStreamOffset();

        pos_cur = backup.Tellg();

        if(pos_end>=pos_cur)
        {
            assert(hour_backup==hour_rebuild);
            assert(pos_backup==pos_rebuild);
        }
    } while(pos_end>pos_cur);

    rebuild.Close();
    backup.Close();

    if(remove(filename_rebuild.c_str())!=0)
        cout << termcolor::red << "Unable to remove rebuilded index file." << endl;

    if(rename(filename_backup.c_str(), filename_rebuild.c_str())!=0)
        cout << termcolor::red << "Unable to rename backup index file." << endl;


    return true;
}
