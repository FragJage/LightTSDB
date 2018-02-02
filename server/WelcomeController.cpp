#include "WelcomeController.h"

using namespace std;
using namespace MongooseCpp;

WelcomeController::WelcomeController()
{
    //ctor
}

WelcomeController::~WelcomeController()
{
    //dtor
}

bool WelcomeController::Process(Request& request, Response& response)
{
    response.SetStatut(200);
    response.SetContent("<html><head></head><body><h1>Welcome on LightTSDB server !</h1></body>");
    return true;
}
