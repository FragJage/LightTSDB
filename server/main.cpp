#include <stdlib.h>
#include <signal.h>
#include <mongoose-cpp/Server.h>
#include <mongoose-cpp/WebController.h>

using namespace std;
using namespace Mongoose;

class MyController : public WebController
{
    public:
        void hello(Request &request, StreamResponse &response)
        {
            response << "Hello " << htmlEntities(request.get("name", "... what's your name ?")) << endl;
        }

        void setup()
        {
            addRoute("GET", "/hello", MyController, hello);
        }
};

class ApiController : public Controller
{
    public:
        void webapi(Request &request, StreamResponse &response)
        {
            response << "Hello api v1" << endl;
        }

        void setup()
        {
            addRoute("GET", "/api/v1/(.+)/(.+)", ApiController, webapi);
        }
};

int main()
{
    MyController myController;
    Server server(8080);
    server.registerController(&myController);

    server.start();

    string key;
    cin >> key;
}
