#include <ctype.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "Rtc.h"

CRtc::CRtc()
{
}

CRtc::~CRtc()
{
}

int CRtc::PN_RTC_Gettime(void *cur_time)
{
    struct tm* TimeNow;
    time_t TimeTmp;
    unsigned int i;

    pn_rtc_time_info* m_time=(pn_rtc_time_info *)cur_time;
    time(&TimeTmp);
    TimeNow = localtime(&TimeTmp);
    sprintf(m_time->TimeBuf,"%4d%2d%2d-%2d:%2d:%2d",TimeNow ->tm_year+1900, TimeNow ->tm_mon+1, TimeNow ->tm_mday, TimeNow ->tm_hour,TimeNow ->tm_min, TimeNow ->tm_sec);

    for(i=0;i<strlen(m_time->TimeBuf);i++){
        if(m_time->TimeBuf[i]==' ')
            m_time->TimeBuf[i]='0';
    }
    debugstr(m_time->TimeBuf);

    return PN_ERR_OK;
}

int CRtc::PN_RTC_Settime(void* cur_time)
{
    pn_rtc_time_info* m_time=(pn_rtc_time_info *)cur_time;
    char tmp[21];
    
    memset(tmp,'\0',sizeof(tmp));
    sprintf(tmp,"date %s%s%s%s%s.%s",m_time->month,m_time->day,m_time->hour,m_time->min,m_time->year,m_time->sec);
    debugstr(tmp);
    system(tmp);
    system("hwclock -w");
    return PN_ERR_OK;
}

int CRtc::PN_RTC_Parsetime(void* cur_time)
{
    pn_rtc_time_info* m_time=(pn_rtc_time_info *)cur_time;

    if((int)(m_time->year)<2000||(int)(m_time->year)>2099)
    {
        printf("Please input the correct year, the year should be in the scope of 2000~2099!\n");
        return PN_ERR_INVALID_MSG;
    }

    if((int)(m_time->month)<1||(int)(m_time->month)>12)
    {
        printf("Please input the correct mouth, the mouth should be in the scope of 1~12!\n");
        return PN_ERR_INVALID_MSG;
    }

    if((int)m_time->day<1||(int)m_time->day>31)
    {
        printf("Please input the correct days, the days should be in the scope of 1~31!\n");
        return PN_ERR_INVALID_MSG;
    }
    else if((int)m_time->month==4||(int)m_time->month==6||(int)m_time->month==9||(int)m_time->month==11)
    {
        if((int)m_time->day<1||(int)m_time->day>30)
        {
            printf("Please input the correct days, the days should be in the scope of 1~30!\n");
            return PN_ERR_INVALID_MSG;
        }
    }
    else if((int)m_time->month==2)
    {
        if(((int)(m_time->year)%4==0)&&((int)(m_time->year)%100!=0)||((int)(m_time->year)%400==0))
        {
            if((int)m_time->day<1||(int)m_time->day>29)
            {
                printf("Please input the correct days, the days should be in the scope of 1~29!\n");
                return PN_ERR_INVALID_MSG;
            }
        }
        else
        {
            if((int)m_time->day<1||(int)m_time->day>28)
            {
                printf("Please input the correct days, the days should be in the scope of 1~28!\n");
                return PN_ERR_INVALID_MSG;
            }
        }

    }

    if((int)m_time->hour<0||(int)m_time->hour>23)
    {
        printf("Please input the correct hour, the hour should be in the scope of 0~23!\n");
        return PN_ERR_INVALID_MSG;
    }

    if((int)m_time->min<0||(int)m_time->min>59)
    {
        printf("Please input the correct minute, the minute should be in the scope of 0~59!\n");
        return PN_ERR_INVALID_MSG;
    }

    if((int)m_time->sec<0||(int)m_time->sec>59)
    {
        printf("Please input the correct second, the second should be in the scope of 0~59!\n");
        return PN_ERR_INVALID_MSG;
    }

    return PN_ERR_OK;
}

