#ifndef TIMEHELPER_H
#define TIMEHELPER_H
#include <ctime>
#include <string>


class TimeHelper
{
    public:
        static std::string ToString(time_t tTime);
        static time_t ToTime(std::string strTime);
};

#endif // TIMEHELPER_H
