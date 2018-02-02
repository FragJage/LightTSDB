#ifndef READCONTROLLER_H
#define READCONTROLLER_H

#include "IWebController.h"

class ReadController : public MongooseCpp::IWebController
{
    public:
        ReadController();
        ~ReadController();
        bool Process(MongooseCpp::Request& request, MongooseCpp::Response& response);
        bool SensorsList(std::string folder, MongooseCpp::Response& response);
};

#endif // READCONTROLLER_H
