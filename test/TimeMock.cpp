#include<iostream>
#include<ctime>

time_t MockTime;

void SetMockTime(int year, int month, int day, int hour, int minute, int seconde)
{
    struct tm myTm;

    myTm.tm_hour = hour;      myTm.tm_min = minute;  myTm.tm_sec = seconde;
    myTm.tm_year = year-1900; myTm.tm_mon = month-1; myTm.tm_mday = day;
    myTm.tm_isdst = 0;

    MockTime = mktime(&myTm);
}

void MockAddSecond(int sec)
{
    MockTime += sec;
}

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
time_t mocktime(time_t* timer)
{
    if(timer!=nullptr) *timer = MockTime;
    return MockTime;
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif