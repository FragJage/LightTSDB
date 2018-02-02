#ifndef LASTCONTROLLER_H
#define LASTCONTROLLER_H

#include "IWebController.h"

class LastController : public MongooseCpp::IWebController
{
    public:
        LastController();
        ~LastController();
        bool Process(MongooseCpp::Request& request, MongooseCpp::Response& response);
};

#endif // LASTCONTROLLER_H
