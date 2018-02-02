#ifndef WELCOMECONTROLLER_H
#define WELCOMECONTROLLER_H

#include "IWebController.h"

class WelcomeController : public MongooseCpp::IWebController
{
    public:
        WelcomeController();
        ~WelcomeController();
        bool Process(MongooseCpp::Request& request, MongooseCpp::Response& response);
};

#endif // WELCOMECONTROLLER_H
