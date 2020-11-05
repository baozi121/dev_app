#ifndef RTC_H
#define RTC_H

#include "comm.h"

class CRtc
{
    public:
        CRtc();
        ~CRtc();
        int PN_RTC_Settime(void* cur_time);
        int PN_RTC_Gettime(void* cur_time);

    private:
        int PN_RTC_Parsetime(void* cur_time);
};

#endif
