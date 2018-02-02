#include "WebServer.h"
#include "WelcomeController.h"
#include "ReadController.h"
#include "LastController.h"
#include "WriteController.h"

using namespace std;
using namespace MongooseCpp;

int main()
{
    ReadController readController;
    LastController lastController;
    WriteController writeController;
    WelcomeController welcomeController;
    WebServer server(8080);

    server.AddRoute("/read/{BaseName}/[SensorId]", &readController);
    server.AddRoute("/lastvalue/{BaseName}/{SensorId}", &lastController);
    server.AddRoute("/write/{BaseName}/{SensorId}", &writeController);
    server.AddRoute("*", &welcomeController);

    server.Start();
    while(true)
    {
        server.Poll();
    }
    server.Stop();
}
