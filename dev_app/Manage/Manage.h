#ifndef MANAGE_H
#define MANAGE_H

#include    "comm.h"
#include    "Rtc.h"
#include    "Sysinfo.h"
#include    "Media.h"
#include    "Work_Order.h"
#include    "ftp.h"

class CManage
{
    public:
        CManage();
        ~CManage();
        int PN_Manage_Init();
        int PN_Manage_Setconfig (int module_id, int config_id, void* config_data);
        int PN_Manage_Getconfig (int module_id, int config_id, void * config_data,void*time);
        int PN_Manage_Control (int module_id, int ctl_id, void* ctl_id2);
        int PN_Manage_Release ();
        void SetCallback(SendCallback callback, void* inst);

    private:
        CRtc m_rtc;
        CSysinfo m_sysinfo;
        CMedia m_media;
        CWork_Order m_work_order;
        CFTP m_ftp;
        CMSQ m_msg;
};

#endif
