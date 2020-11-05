#ifndef MYSETTING_H
#define MYSETTING_H

#include"comm.h"

class CSysinfo
{
    public:

        CSysinfo();
        ~CSysinfo();

        int PN_Sysinfo_Init();
        int PN_Sysinfo_Getconfig (int config_id, void* config_data);
        int PN_Sysinfo_Setconfig (int config_id, void* config_data);

    private:
        int PN_Sysinfo_Save_Config(int config_id);
        int PN_Sysinfo_Read_Config(int config_id);
        int PN_Sysinfo_Get_NetInfo();

        pn_sysinfo_dev      m_devinfo;
        pn_sysinfo_servers  m_servinfo;
        pn_sysinfo_net      m_netinfo;

};

#endif

