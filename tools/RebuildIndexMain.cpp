#include<iostream>
#include "RebuildIndex.h"

using namespace std;

int main(int argc, char *argv[])
{
    LightTSDB::RebuildIndex toolRI;

    if(argc!=2)
    {
        cout << "Usage : " << argv[0] << " <filename>\n";
        return 0;
    }

    toolRI.Rebuild(argv[1]);

    return 0;
}
