#include<stdio.h>
#include <stdlib.h>
#include<string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include"Sysinfo.h"
#include "IniFile.h"

CSysinfo::CSysinfo()
{
}

CSysinfo::~CSysinfo()
{
}

int CSysinfo::PN_Sysinfo_Init()
{
    memset(&m_devinfo,0,sizeof(m_devinfo));
    memset(&m_servinfo,0,sizeof(m_servinfo));
    memset(&m_netinfo,0,sizeof(m_netinfo));

    PN_Sysinfo_Read_Config(PN_CFGID_SYSINFO_DEV);
    PN_Sysinfo_Read_Config(PN_CFGID_SYSINFO_SERVERS);
    PN_Sysinfo_Read_Config(PN_CFGID_SYSINFO_NET);

    PN_Sysinfo_Get_NetInfo();
    PN_Sysinfo_Save_Config(PN_CFGID_SYSINFO_NET);

    return PN_ERR_OK;
}

int CSysinfo::PN_Sysinfo_Getconfig (int config_id, void* config_data)
{
    pn_sysinfo_dev *tmp_dev;
    pn_sysinfo_servers *tmp_servers;
    pn_sysinfo_net *tmp_net;

    if(config_data==NULL)
        return PN_ERR_FAILED;

    switch(config_id)
    {
        case PN_CFGID_SYSINFO_DEV:
            tmp_dev = (pn_sysinfo_dev *)config_data;
            strcpy(tmp_dev->dev_manufacturer, m_devinfo.dev_manufacturer);
            strcpy(tmp_dev->dev_model, m_devinfo.dev_model);
            strcpy(tmp_dev->dev_firmware, m_devinfo.dev_firmware);
            tmp_dev->dev_maxcamera=m_devinfo.dev_maxcamera;
            strcpy(tmp_dev->dev_id, m_devinfo.dev_id);
            strcpy(tmp_dev->dev_name, m_devinfo.dev_name);
            strcpy(tmp_dev->dev_pwd, m_devinfo.dev_pwd);
            strcpy(tmp_dev->dev_sipnum, m_devinfo.dev_sipnum);
            break;
        case PN_CFGID_SYSINFO_SERVERS:
            tmp_servers = (pn_sysinfo_servers *)config_data;
            strcpy(tmp_servers->serv_name, m_servinfo.serv_name);
            strcpy(tmp_servers->serv_id, m_servinfo.serv_id);
            strcpy(tmp_servers->serv_ip, m_servinfo.serv_ip);
            strcpy(tmp_servers->serv_port, m_servinfo.serv_port);
            break;
        case PN_CFGID_SYSINFO_NET:
            tmp_net = (pn_sysinfo_net *)config_data;
            strcpy(tmp_net->ip, m_netinfo.ip);
            strcpy(tmp_net->mask, m_netinfo.mask);
            strcpy(tmp_net->gateway, m_netinfo.gateway);
            strcpy(tmp_net->DNS, m_netinfo.DNS);
            strcpy(tmp_net->type, m_netinfo.type);
            break;
        default:
            return PN_ERR_INVALID_MSG;
            break;
    }

    return PN_ERR_OK;
}

int CSysinfo::PN_Sysinfo_Setconfig (int config_id, void* config_data)
{
    pn_sysinfo_dev *tmp_dev;
    pn_sysinfo_servers *tmp_servers;
    pn_sysinfo_net *tmp_net;

    if(config_data==NULL)
        return PN_ERR_FAILED;

    switch(config_id)
    {
        case PN_CFGID_SYSINFO_DEV:
            tmp_dev = (pn_sysinfo_dev *)config_data;
            strcpy(m_devinfo.dev_manufacturer,tmp_dev->dev_manufacturer);
            strcpy(m_devinfo.dev_model,tmp_dev->dev_model);
            strcpy(m_devinfo.dev_firmware,tmp_dev->dev_firmware);
            m_devinfo.dev_maxcamera=tmp_dev->dev_maxcamera;
            strcpy(m_devinfo.dev_id,tmp_dev->dev_id);
            strcpy(m_devinfo.dev_name,tmp_dev->dev_name);
            strcpy(m_devinfo.dev_pwd,tmp_dev->dev_pwd);
            break;
        case PN_CFGID_SYSINFO_SERVERS:
            tmp_servers = (pn_sysinfo_servers *)config_data;
            strcpy(m_servinfo.serv_name,tmp_servers->serv_name);
            strcpy(m_servinfo.serv_id,tmp_servers->serv_id);
            strcpy(m_servinfo.serv_ip,tmp_servers->serv_ip);
            strcpy(m_servinfo.serv_port,tmp_servers->serv_port);
            break;
        case PN_CFGID_SYSINFO_NET:
            tmp_net= (pn_sysinfo_net *)config_data;
            strcpy(m_netinfo.ip,tmp_net->ip);
            strcpy(m_netinfo.mask,tmp_net->mask);
            strcpy(m_netinfo.gateway,tmp_net->gateway);
            strcpy(m_netinfo.DNS,tmp_net->DNS);
            strcpy(m_netinfo.type,tmp_net->type);
            //do something
            break;
        default:
            return PN_ERR_INVALID_MSG;
            break;
    }

    PN_Sysinfo_Save_Config(config_id);
    return PN_ERR_OK;
}

int CSysinfo::PN_Sysinfo_Save_Config(int config_id)
{
    CIniFile m_ini;

    if(config_id==PN_CFGID_SYSINFO_DEV)
    {
        m_ini.ConfigSetKey(CFG_FILE, "DEV_INFO", "manufacturer", m_devinfo.dev_manufacturer);
        m_ini.ConfigSetKey(CFG_FILE, "DEV_INFO", "model", m_devinfo.dev_model);
        m_ini.ConfigSetKey(CFG_FILE, "DEV_INFO", "firmware", m_devinfo.dev_firmware);
        m_ini.ConfigSetKey(CFG_FILE, "DEV_INFO", "name", m_devinfo.dev_name);
        m_ini.ConfigSetKey(CFG_FILE, "DEV_INFO", "id", m_devinfo.dev_id);
        m_ini.ConfigSetKey(CFG_FILE, "DEV_INFO", "pwd", m_devinfo.dev_pwd);

        char tmp[5];
        sprintf(tmp,"%d",m_devinfo.dev_maxcamera);
        m_ini.ConfigSetKey(CFG_FILE, "DEV_INFO", "maxcamera", tmp);
    }

    if(config_id==PN_CFGID_SYSINFO_SERVERS)
    {
        m_ini.ConfigSetKey(CFG_FILE, "SERV_INFO", "name", m_servinfo.serv_name);
        m_ini.ConfigSetKey(CFG_FILE, "SERV_INFO", "id", m_servinfo.serv_id);
        m_ini.ConfigSetKey(CFG_FILE, "SERV_INFO", "ip", m_servinfo.serv_ip);
        m_ini.ConfigSetKey(CFG_FILE, "SERV_INFO", "port", m_servinfo.serv_port);
    }

    if(config_id==PN_CFGID_SYSINFO_NET)
    {
        m_ini.ConfigSetKey(CFG_FILE, "NET_INFO", "ip", m_netinfo.ip);
        m_ini.ConfigSetKey(CFG_FILE, "NET_INFO", "mask", m_netinfo.mask);
        m_ini.ConfigSetKey(CFG_FILE, "NET_INFO", "gateway", m_netinfo.gateway);
        m_ini.ConfigSetKey(CFG_FILE, "NET_INFO", "DNS", m_netinfo.DNS);
        m_ini.ConfigSetKey(CFG_FILE, "NET_INFO", "type", m_netinfo.type);
    }

    return PN_ERR_OK;

}

int CSysinfo::PN_Sysinfo_Read_Config(int config_id)
{
    CIniFile m_ini;

    if(config_id==PN_CFGID_SYSINFO_DEV)
    {
        m_ini.ConfigGetKey(CFG_FILE, "DEV_INFO", "manufacturer", m_devinfo.dev_manufacturer);
        m_ini.ConfigGetKey(CFG_FILE, "DEV_INFO", "model", m_devinfo.dev_model);
        m_ini.ConfigGetKey(CFG_FILE, "DEV_INFO", "firmware", m_devinfo.dev_firmware);
        m_ini.ConfigGetKey(CFG_FILE, "DEV_INFO", "name", m_devinfo.dev_name);
        m_ini.ConfigGetKey(CFG_FILE, "DEV_INFO", "id", m_devinfo.dev_id);
        m_ini.ConfigGetKey(CFG_FILE, "DEV_INFO", "phone_num", m_devinfo.dev_sipnum);
        m_ini.ConfigGetKey(CFG_FILE, "DEV_INFO", "pwd", m_devinfo.dev_pwd);

        char tmp[5];
        m_ini.ConfigGetKey(CFG_FILE, "DEV_INFO", "maxcamera", tmp);
        m_devinfo.dev_maxcamera=atoi(tmp);

        m_ini.ConfigGetKey(CFG_FILE, "DEV_INFO", "name", m_devinfo.dev_name);
        m_ini.ConfigGetKey(CFG_FILE, "DEV_INFO", "id", m_devinfo.dev_id);
        m_ini.ConfigGetKey(CFG_FILE, "DEV_INFO", "pwd", m_devinfo.dev_pwd);
    }

    if(config_id==PN_CFGID_SYSINFO_SERVERS)
    {
        m_ini.ConfigGetKey(CFG_FILE, "SERV_INFO", "name", m_servinfo.serv_name);
        m_ini.ConfigGetKey(CFG_FILE, "SERV_INFO", "id", m_servinfo.serv_id);
        m_ini.ConfigGetKey(CFG_FILE, "SERV_INFO", "ip", m_servinfo.serv_ip);
        m_ini.ConfigGetKey(CFG_FILE, "SERV_INFO", "port", m_servinfo.serv_port);
    }

    if(config_id==PN_CFGID_SYSINFO_NET)
    {
        m_ini.ConfigGetKey(CFG_FILE, "NET_INFO", "ip", m_netinfo.ip);
        m_ini.ConfigGetKey(CFG_FILE, "NET_INFO", "mask", m_netinfo.mask);
        m_ini.ConfigGetKey(CFG_FILE, "NET_INFO", "gateway", m_netinfo.gateway);
        m_ini.ConfigGetKey(CFG_FILE, "NET_INFO", "DNS", m_netinfo.DNS);
        m_ini.ConfigGetKey(CFG_FILE, "NET_INFO", "type", m_netinfo.type);
    }

    return PN_ERR_OK;
}

int CSysinfo::PN_Sysinfo_Get_NetInfo()
{
    struct ifaddrs *ifc, *ifc1;
    char ip[20];
    char mask[20];
    memset(ip,0,sizeof(ip));
    memset(mask,0,sizeof(mask));

    if(0!=getifaddrs(&ifc))
        return PN_ERR_FAILED;
    ifc1 = ifc;

    for(; NULL != ifc; ifc = (*ifc).ifa_next)
    {
        if(strcmp((*ifc).ifa_name,"ppp0")==0)
        {
            if (NULL != (*ifc).ifa_addr)
            {
                inet_ntop(AF_INET, &(((struct sockaddr_in*)((*ifc).ifa_addr))-> sin_addr), ip, sizeof(ip));
                printf( "3G ip=%s \n", ip);

                if(strlen(ip)>8)
                {
                    strcpy(m_netinfo.ip,ip);
                    if (NULL != (*ifc).ifa_netmask)
                    {
                        inet_ntop(AF_INET, &(((struct sockaddr_in*)((*ifc).ifa_netmask))-> sin_addr), mask, sizeof(mask));
                        printf( "3G mask=%s \n", mask);
                        strcpy(m_netinfo.mask,mask);
                        break;
                    }
                }

                memset(ip,0,sizeof(ip));
                memset(mask,0,sizeof(mask));
            }
        }
    }

    freeifaddrs(ifc1);

    strcpy(m_netinfo.type,"3G");

    return PN_ERR_OK;
}
