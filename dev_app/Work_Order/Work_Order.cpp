#include<unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <fcntl.h>
#include <iostream>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <mntent.h>

#include "Work_Order.h"

#define PATH "/etc/profile"
using namespace std;

// extern int recieved;
CWork_Order::CWork_Order()
{
    listening_thread=0;
    usb_thread=0;
}

CWork_Order::~CWork_Order()
{
    if(0!=listening_thread)
        while(0==pthread_kill(listening_thread,0))
            pthread_cancel(listening_thread);
    debugstring("work order listen thread _released");

    if(0!=usb_thread)
        while(0==pthread_kill(usb_thread,0))
            pthread_cancel(usb_thread);
    debugstring("usb detect thread _releasd");
}

int CWork_Order::PN_Work_Order_Init()
{
    m_msq.Msq_Create();
    PN_Work_Order_Listening();
    return PN_ERR_OK;
}

int CWork_Order::PN_Work_Order_Release()
{
    return PN_ERR_OK;
}

int CWork_Order::PN_Work_Order_Setconfig(int config_id ,void* config_data)
{
    int err;

    switch(config_id)
    {
        case PN_CFGID_WIFI_INFO_RESPONSE:
            err=PN_Work_Order_Setconfig_Wifi_Info_Response(config_data);
            break;
        case PN_CFGID_DEV_CALL_INFO_RESPONSE:
            err=PN_Work_Order_Setconfig_Dev_Call_Info_Response(config_data);
            break;
        case PN_CFGID_GROUP_CALL_INFO_RESPONSE:
            err=PN_Work_Order_Setconfig_GroupNum_Info_Response(config_data);
            break;
        default:
            err=PN_ERR_INVALID_MSG;
            break;
    }

    return err;
}

int CWork_Order::PN_Work_Order_Setconfig_Wifi_Info_Response(void* config_data)
{
    int err,i,n;
    msg_task msg;
    pn_work_order_info *pwork_order_info;
    char ssid[5][MAX_MSQ_BUF];
    char psw[5][MAX_MSQ_BUF];
    char wififile[30];

    pwork_order_info = (pn_work_order_info *)config_data;
    debugstr(pwork_order_info->wifi_ssid);
    debugstr(pwork_order_info->wifi_passwd);
    n=m_ini.MsqMsgParse(pwork_order_info->wifi_ssid,ssid);
    n=m_ini.MsqMsgParse(pwork_order_info->wifi_passwd,psw);
    system("rm -rf /mnt/wifi/*");
    for(i=0;i<n;i++)
    {
        sprintf(wififile,"/mnt/wifi/%s",ssid[i]);
        err = m_ini.WifiConfig(wififile,ssid[i],psw[i]);
    }
    strcpy(msg.text,"0");

    msg.msg_type = DEVWORKORDER;
    msg.m_type=DEVWIFICFGRESP;
    m_msq.Msq_Send_Task(msg);
    return PN_ERR_OK;
}

int CWork_Order::PN_Work_Order_Setconfig_Dev_Call_Info_Response(void* config_data)                                                              
{                                                                                                                                               
    // msg_task msg;                                                                                                                            
    FILE * fp;                                                                                                                                  
    // int count,i;                                                                                                                             
    int i;                                                                                                                                      
    char buf[500];                                                                                                                              
    char tmp[60];                                                                                                                               
    pn_dev_call_info *pdev_call_info;                                                                                                           

    pdev_call_info = (pn_dev_call_info *)config_data;                                                                                           
    memset(tmp,0x00,sizeof(tmp));                                                                                                               
    memset(buf,0x00,sizeof(buf));                                                                                                               

    fp=fopen("/mnt/call_info/ningbo.txt","a+");                                                                                         

    if(fp==NULL)                                                                                                                                
    {                                                                                                                                           
        debugstring("Failed to open file!!\n");                                                                                                 
        return 0;                                                                                                                               
    }
    // if(recieved==1){
        // for(i=0;i<pdev_call_info[0].count-1;i++)
        // {
            // sprintf(tmp,"%s\n",pdev_call_info[i].phone_num);
            // strcat(buf,tmp);
        // }
        // sprintf(tmp,"%s",pdev_call_info[i].phone_num);
        // strcat(buf,tmp);
        for(i=0;i<pdev_call_info[0].count;i++)
        {
            sprintf(tmp,"%s\t%s\n",pdev_call_info[i].name,pdev_call_info[i].phone_num);
            strcat(buf,tmp);
        }
        // sprintf(tmp,"%-15s%s",pdev_call_info[i].name,pdev_call_info[i].phone_num);
        // strcat(buf,tmp);
        debugstr(buf);
        // strcat(buf,"\n");
    // }
    // else{
        // for(i=0;i<pdev_call_info[0].count;i++)
        // {
            // sprintf(tmp,"%-15s%s\n",pdev_call_info[i].name,pdev_call_info[i].phone_num);
            // strcat(buf,tmp);
        // }
    // }

    fwrite(buf,strlen(buf),1,fp);
    fclose(fp);

    return 0;
}

int CWork_Order::PN_Work_Order_Setconfig_GroupNum_Info_Response(void* config_data)                                                              
{                                                                                                                                               
    // msg_task msg;                                                                                                                            
    FILE * fp;                                                                                                                                  
    // int count,i;                                                                                                                             
    int i;                                                                                                                                      
    char buf[500];                                                                                                                              
    char tmp[60];                                                                                                                               
    pn_group_call_info *pgroup_call_info;                                                                                                           

    pgroup_call_info = (pn_group_call_info *)config_data;                                                                                           
    memset(tmp,0x00,sizeof(tmp));                                                                                                               
    memset(buf,0x00,sizeof(buf));                                                                                                               

    fp=fopen("/mnt/call_info/groupNum.txt","w");                                                                                         

    if(fp==NULL)                                                                                                                                
    {                                                                                                                                           
        debugstring("Failed to open file!!\n");                                                                                                 
        return 0;                                                                                                                               
    }
    for(i=0;i<pgroup_call_info[0].count;i++)
    {
        sprintf(tmp,"%-20s\t%s\n",pgroup_call_info[i].group_name,pgroup_call_info[i].group_num);
        strcat(buf,tmp);
    }
    fwrite(buf,strlen(buf),1,fp);
    fclose(fp);

    return 0;
}

int CWork_Order::PN_Work_Order_Listening()
{
    int err;

    err = pthread_create (&listening_thread, NULL , (void*(*)(void*))PN_Work_Order_Thread_Listening, (void *)this );
    if ( err!= 0)
    {
        printf ( "can't create socket listening thread: %s\n" , strerror(err)) ;
        return PN_ERR_FAILED;
    }

    err = pthread_create (&usb_thread, NULL , (void*(*)(void*))PN_Work_Order_Usb_Thread_Listening, (void *)this );
    if ( err!= 0)
    {
        printf ( "can't create usb listening thread: %s\n" , strerror(err)) ;
        return PN_ERR_FAILED;
    }

    return PN_ERR_OK;
}

void CWork_Order::PN_Work_Order_Thread_Listening(void* params)
{
    int err;

    CWork_Order *pWork_Order = (CWork_Order *) params;
    msg_task msg;
    memset(&msg,0,sizeof(msg_task));

    while(1){
        err=pWork_Order->m_msq.Msq_Read_Task(&msg,QTWORKORDER);
        if(err==PN_ERR_OK){
            switch(msg.m_type){
                case QTWIFICFG:
                    debugstr(msg.text);
                    pWork_Order->PN_Work_Order_Wifi_Info_XML_Send();
                    break;
                case QTELECINFO:
                    debugstr(msg.text);
                    pWork_Order->PN_Work_Order_Electricity_Information_XML_Send(msg.text);
                    break;
                case QTCALLINFO:
                    debugstr(msg.text);
                    pWork_Order->PN_Work_Order_SipCall_Query_XML_Send(msg.text);
                    break;
                case QTGROUPINFO:
                    debugstr(msg.text);
                    pWork_Order->PN_Work_Order_GroupNum_Query_XML_Send(msg.text);
                    break;
                default:
                    break;
            }
        }
        sleep(1);
    }
}
#if 1
void CWork_Order::PN_Work_Order_Usb_Thread_Listening(void* params)
{
    CWork_Order *pWork_Order = (CWork_Order *) params;
    msg_task msg;   
    memset(&msg,0,sizeof(msg_task));
    char filename[100];
    DIR *d; 
    FILE *fp;
    unsigned int size, free, exists, image_size, video_size; //KB
    int foundusb_prv, foundusb; 
    struct mntent *fs;
    struct statfs vfs;
    struct dirent *de; 
    struct stat buf; 

    foundusb_prv = 0;
    foundusb = 0;
    while(1) {
        fp = setmntent("/etc/mtab", "r");    /* read only */
        if (fp == NULL) {
            // TRACE(PN_ERR_OPEN);
            return;
        }

        /* scan the mounted filesystem for usb,
           update flag foundusb's state */
        while ((fs = getmntent(fp)) != NULL) {
            if (fs->mnt_fsname[0] != '/')    /* skip nonreal filesystems */
                continue;

            if (strcmp(fs->mnt_dir, "/mnt/usb") == 0) {
                if (statfs(fs->mnt_dir, &vfs) != 0) {
                    // TRACE(PN_ERR_FAILED);
                    return;
                }
                size = (vfs.f_blocks * vfs.f_bsize) >> 10;
                free = (vfs.f_bfree * vfs.f_bsize) >> 10;
                foundusb = 1;
                break;
            }

            foundusb = 0;
        }

        /* close the fp for subsequent scanning process */
        endmntent(fp);

        /* rising edge triggers the size caculation and the sending process */
        if (foundusb_prv == 0
            && foundusb == 1) {
            fp = setmntent("/etc/mtab", "r");        /* read only */
            if (fp == NULL) {
                // TRACE(PN_ERR_OPEN);
                return;
            }

            image_size = 0; 
            video_size = 0; 

            /* scan the mounted filesystem for sd card */
            while ((fs = getmntent(fp)) != NULL) {
                if (fs->mnt_fsname[0] != '/')    /* skip nonreal filesystems */
                    continue;

                if (strcmp(fs->mnt_dir, "/mnt/mmc") == 0) {
                    /* caculate the size of image dir and video dir */
                    d = opendir("/mnt/mmc/image/"); 
                    if (d == NULL) { 
                        break;
                    } 

                    for (de = readdir(d); de != NULL; de = readdir(d)) { 
                        if(strcmp(de->d_name, ".") == 0
                           || strcmp(de->d_name, "..") == 0) 
                           continue;
                        strcpy(filename,"/mnt/mmc/image/");                                                                                                        
                        strcat(filename,de->d_name);
                        exists = stat(filename, &buf); 
                        if (exists < 0) { 
                            break;
                        } else { 
                            image_size += (buf.st_size >> 10); 
                        } 
                    } 
                    closedir(d); 

                    d = opendir("/mnt/mmc/video/"); 
                    if (d == NULL) { 
                        break;
                    } 

                    for (de = readdir(d); de != NULL; de = readdir(d)) { 
                        if(strcmp(de->d_name, ".") == 0
                           || strcmp(de->d_name, "..") == 0) 
                           continue;
                        strcpy(filename,"/mnt/mmc/video/");                                                                                                        
                        strcat(filename,de->d_name);
                        exists = stat(filename, &buf); 
                        if (exists < 0) { 
                            break;
                        } else { 
                            video_size += (buf.st_size >> 10); 
                        } 
                    } 
                    closedir(d); 

                    break;
                }
            }
            endmntent(fp);

            /* send the message to qt */
            //printf("usb size : %u\nusb free : %u\nimage    : %u\nvideo    : %u\n", size, free, image_size, video_size);
            msg.msg_type = DEVWORKORDER;
            msg.m_type = USBDETECT;
            msg.text[0] = size;
            msg.text[1] = size >> 8;
            msg.text[2] = size >> 16;
            msg.text[3] = size >> 24;
            msg.text[4] = free;
            msg.text[5] = free >> 8;
            msg.text[6] = free >> 16; 
            msg.text[7] = free >> 24;
            msg.text[8] = image_size;
            msg.text[9] = image_size >> 8;
            msg.text[10] = image_size >> 16;
            msg.text[11] = image_size >> 24;
            msg.text[12] = video_size;
            msg.text[13] = video_size >> 8;
            msg.text[14] = video_size >> 16;
            msg.text[15] = video_size >> 24;
            pWork_Order->m_msq.Msq_Send_Task(msg);
        }

        foundusb_prv = foundusb;
        sleep(5);
    }
}
#endif
void CWork_Order::SetCallback(SendCallback callback,void* inst)
{
    callback_send = callback;
    m_inst=inst;
}

int CWork_Order::PN_Work_Order_Electricity_Information_XML_Send(char* text)
{
    char buffer[300]   = {0};

    sprintf(buffer,"<?xml version=\"1.0\"?>\n"
            "<dev_power_update>\n"
            "<power>%s</power>\n"
            "</dev_power_update>",text);
    if(callback_send!=NULL)
        callback_send(NOTIFY,buffer,m_inst);
    return 0;
}

int CWork_Order::PN_Work_Order_Wifi_Info_XML_Send()
{
    char buffer[300]={0};

    sprintf(buffer,"<?xml version=\"1.0\"?>\n"
            "<wifi_info_query>\n"
            "</wifi_info_query>");
    if(callback_send!=NULL)
        callback_send(NOTIFY,buffer,m_inst);
    return 0;
}

int CWork_Order::PN_Work_Order_SipCall_Query_XML_Send(char* text)
{
    // FILE *fp;

    // fp=fopen("/mnt/call_info/ningbo.text","w+");

    char buffer[300] = {0};

    sprintf(buffer,"<?xml version=\"1.0\"?>\n"
            "<subscribe_speechpn>\n"
            "<dev_id>%s</dev_id>\n"
            "</subscribe_speechpn>",text);
    if(callback_send!=NULL)
        callback_send(SUBSCRIBE,buffer,m_inst);
    // fclose(fp);

    return 0;
}

int CWork_Order::PN_Work_Order_GroupNum_Query_XML_Send(char* text)
{ 
    // FILE *fp;

    // fp=fopen("/mnt/call_info/groupNum.text","w+");

    char buffer[300] = {0};

    sprintf(buffer,"<?xml version=\"1.0\"?>\n"
            "<subscribe_grouppn>\n"
            "<dev_id>%s</dev_id>\n"
            "</subscribe_grouppn>",text);
    if(callback_send!=NULL)
        callback_send(SUBSCRIBE,buffer,m_inst);
    // fclose(fp);

    return 0;
}
