#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <malloc.h>
#include <string.h>
#include "Manage.h"

CManage::CManage()
{
}

CManage::~CManage()
{
}

int CManage::PN_Manage_Init()
{
    m_sysinfo.PN_Sysinfo_Init();
   // m_media.PN_Media_Init();
  //  m_work_order.PN_Work_Order_Init();
  //  m_ftp.PN_Ftp_Init();
    return PN_ERR_OK;
}

int CManage::PN_Manage_Release()
{
    m_media.PN_Media_Release();
    m_work_order.PN_Work_Order_Release();
    // m_ftp.PN_Ftp_Release();
    return PN_ERR_OK;
}

int CManage::PN_Manage_Setconfig (int module_id, int config_id, void* config_data)
{
    int err;

    switch(module_id)
    {
        case MODULE_RTC:
            err=m_rtc.PN_RTC_Settime(config_data);
            break;
        case MODULE_MEDIA:
            err=m_media.PN_Media_Setconfig(config_id,config_data);
            break;
        case MODULE_WORK_ORDER:
            err=m_work_order.PN_Work_Order_Setconfig(config_id,config_data);
            break;
        // case MODULE_FTP:
            // err=m_ftp.PN_Ftp_Setconfig(config_id,config_data);
            // break;
        case MODULE_SYSINFO:
            err=m_sysinfo.PN_Sysinfo_Setconfig(config_id, config_data);
            break;
        default:
            break;
    }
    return err;
}

int CManage::PN_Manage_Getconfig (int module_id, int config_id, void * config_data,void *time)
{
    int err;
    switch(module_id)
    {
        case MODULE_RTC:
            err=m_rtc.PN_RTC_Gettime(config_data);
            break;
        case MODULE_MEDIA:
            err=m_media.PN_Media_Getconfig(config_id,config_data);
            break;
        case MODULE_SYSINFO:
            err=m_sysinfo.PN_Sysinfo_Getconfig(config_id, config_data);
            break;
        default:
            break;
    }
    return err;
}

int CManage::PN_Manage_Control (int module_id, int ctl_id, void* ctl_id2)
{
    int err;
    switch(module_id)
    {
        case MODULE_RTC:
            break;
        case MODULE_MEDIA:
            err=m_media.PN_Media_Control(ctl_id);
            break;
        case MODULE_SYSINFO:
            break;
        default:
            break;
    }
    return err;
}

void CManage::SetCallback(SendCallback callback, void* inst)
{
    m_media.SetCallback(callback,inst);//flh add
    m_work_order.SetCallback(callback,inst);//whz add
    m_ftp.SetCallback(callback,inst);//whz add
}

