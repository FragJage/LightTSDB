#ifndef WRITECONTROLLER_H
#define WRITECONTROLLER_H

#include "IWebController.h"

class WriteController : public MongooseCpp::IWebController
{
    public:
        WriteController();
        ~WriteController();
        bool Process(MongooseCpp::Request& request, MongooseCpp::Response& response);
};

#endif // WRITECONTROLLER_H
