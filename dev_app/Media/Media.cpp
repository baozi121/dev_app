#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include "Media.h"
#include "cJSON.h"
#include "Socket.h"
#include<unistd.h>
using namespace std;

#define NORMAL 0
#define ABNORMAL 1
#define DETECT_MIN  10
#define DETECT_SEC  0
#define FIRST_MIN  10
#define FIRST_SEC  0
CMedia* CMedia::m_this=NULL;
bool GroupBusy = false;//flag for group state
bool BreakDown = false;

struct itimerval itv1;

CMedia::CMedia()
{
    m_prtpsend = NULL;
    m_pAudiortpsend = NULL;
    m_pRing=NULL;
    sipcall_status = SIPCALL_FREE;
    login_status = ABNORMAL;
    sipcall_status_detect = NORMAL;
    listening_thread = 0;
    m_this = this;
}

CMedia::~CMedia()
{
    if(m_prtpsend != NULL)
    {
        delete m_prtpsend;
        m_prtpsend = NULL;
    }

    if(m_pAudiortpsend != NULL)
    {
        delete m_pAudiortpsend;
        m_pAudiortpsend = NULL;
    }

    if(0!=listening_thread)
        while(0==pthread_kill(listening_thread,0))
            pthread_cancel(listening_thread);
    debugstring("sipcall listening thread _released");
}

int CMedia::PN_Media_Init()
{
    m_msq.Msq_Create();
    //PN_Media_SipCall_Listening();
//    PN_Login_Detect_Init();

    PN_Media_Read_Config(PN_CFGID_MEDIA_VIDEO);
    PN_Media_Read_Config(PN_CFGID_MEDIA_AUDIO);
    PN_Media_Read_Config(PN_CFGID_MEDIA_LABEL);
    PN_Media_Read_Config(PN_CFGID_MEDIA_DEV);

    if(pthread_mutex_init(&mutex,NULL))
        return PN_ERR_FAILED;
    return PN_ERR_OK;
}

void CMedia::PN_Login_Detect_Init()
{
    signal(SIGALRM, PN_Media_Timer);
    struct itimerval itv;
    itv.it_interval.tv_sec = DETECT_MIN;
    itv.it_interval.tv_usec = DETECT_SEC;
    itv.it_value.tv_sec = FIRST_MIN;
    itv.it_value.tv_usec = FIRST_SEC;
    setitimer(ITIMER_REAL, &itv, NULL);
    return;
}

void CMedia::PN_Media_Timer(int sig)
{
    m_this->PN_Media_Timer_Proccess();
}

void CMedia::PN_Media_Timer_Proccess()
{
    msg_task msg;
    msg.msg_type = DEVWORKORDER;
    msg.m_type = DEVLOGINSTATUS;

    if(m_this->login_status == NORMAL)
        debugstring("connect to server ok!");

    else
    {
        debugstring("connect to server fail!");
        strcpy(msg.text,"0");
        m_this->m_msq.Msq_Send_Task(msg);
    }

    m_this->login_status = ABNORMAL;
    return;
}

void CMedia::PN_Login_Detect_Success()
{
    msg_task msg;
    msg.msg_type = DEVWORKORDER;
    msg.m_type = DEVLOGINSTATUS;
//printf("connect to server ok!\n");
    debugstring("connect to server ok!\n");
    strcpy(msg.text,"1");

   // m_msq.Msq_Send_Task(msg);
    debugstring("Send msg to qt!");
    debugint(msg.msg_type);
    debugstr(msg.text);
    login_status = NORMAL;
perror("PN_Login_Detect_Success() out ----------");
    return;
}

int CMedia::PN_Media_Release()
{
    return PN_ERR_OK;
}

int CMedia::PN_Media_Setconfig(int config_id ,void* config_data)
{
    int err;

    switch(config_id)
    {
        case PN_CFGID_MEDIA_VIDEO_INVITE:
            err=PN_Media_Setconfig_Video_Invite(config_data);
            break;
        case PN_CFGID_MEDIA_VIDEO_BYE:
            err=PN_Media_Setconfig_Video_BYE(config_data);
            break;
        case PN_CFGID_MEDIA_SIPCALL_INVITE:
            err=PN_Media_Setconfig_SipCall_Invite(config_data);
            break;
        case PN_CFGID_MEDIA_SIPCALL_INVITED:
            err=PN_Media_Setconfig_SipCall_Invited(config_data);
            break;
        case PN_CFGID_MEDIA_SIPCALL_CANCEL:
            err=PN_Media_Setconfig_SipCall_Cancel(config_data);
            break;
        case PN_CFGID_MEDIA_SIPCALL_BYE:
            err=PN_Media_Setconfig_SipCall_Bye(config_data);
            break;
        // case PN_CFGID_MEDIA_GROUP_BUSY_INVITE:
            // err=PN_Media_Setconfig_Group_Busy_Invite(config_data);
            // break;
        case PN_CFGID_MEDIA_GROUP_SIPCALL_INVITE:
            err=PN_Media_Setconfig_Group_SipCall_Invite(config_data);
            break;
        case PN_CFGID_MEDIA_AUDIO:
            err=PN_Media_Setconfig_Audio(config_data);
            break;
        case PN_CFGID_MEDIA_VIDEO:
            err=PN_Media_Setconfig_Video(config_data);
            break;
        default:
            err=PN_ERR_INVALID_MSG;
            break;
    }

    PN_Media_Save_Config(config_id);
    return err;
}

int CMedia::PN_Media_Getconfig(int config_id ,void* config_data)
{
    int err;
    switch(config_id)
    {
        case PN_CFGID_MEDIA_VIDEO_INVITE:
            break;
        case PN_CFGID_MEDIA_VIDEO:
            err=PN_Media_Getconfig_Video(config_data);
            break;
        case PN_CFGID_MEDIA_AUDIO:
            err=PN_Media_Getconfig_Audio(config_data);
            break;
        case PN_CFGID_MEDIA_LABEL:
            err=PN_Media_Getconfig_Label(config_data);
            break;
            // case PN_CFGID_MEDIA_MOTION:
            // err=PN_Media_Getconfig_Motion(config_data);
            // break;
        default:
            err=PN_ERR_INVALID_MSG;
            break;
    }

    return err;

}

int CMedia::PN_Media_Control(int ctrl_id)
{
    int err;
    switch(ctrl_id)
    {
        case PN_CFGID_MEDIA_VIDEO_START:
            err=PN_Media_Start();
            break;
        case PN_CFGID_MEDIA_SIPCALL_START:
            debugstring("start the sipcall");
            debugint(sipcall_status);
            if (sipcall_status==SIPCALL_CALLING || sipcall_status==SIPCALL_CALLED){
                err=PN_Ring_Stop();
                // sleep (1);
                debugstring("start the sipcall---------------------------");
                err=PN_Audio_Start();
				// OnBusy = true;
                pthread_mutex_lock(&mutex);
                sipcall_status=SIPCALL_BUSY;
                pthread_mutex_unlock(&mutex);
                break;
            }
            else
                break;
		case PN_CFGID_MEDIA_GROUP_SIPCALL_START:
            debugstring("start the groupcall=========================\n");
            debugint(sipcall_status);
			if(sipcall_status==SIPCALL_FREE)
			{
				GroupBusy = true;
				err=PN_Audio_Start();
				break;
			}
			else
				break;
        case PN_CFGID_MEDIA_VIDEO_STOP:
            err=PN_Media_Stop();
            break;
        case PN_CFGID_MEDIA_SIPCALL_STOP:
            debugstring("stop the sipcall");
            debugint(sipcall_status);
            err=PN_Ring_Stop();
            err=PN_Audio_Stop();
            pthread_mutex_lock(&mutex);
            sipcall_status=SIPCALL_FREE;
            pthread_mutex_unlock(&mutex);
			if(GroupBusy == false && sipcall_status == SIPCALL_FREE && BreakDown == false)
			{
				PN_Media_SipCall_GroupNum_Query_XML_Send();//request for group
            }
            break;
        case PN_CFGID_MEDIA_BREAKDOWN_STOP:
            debugstring("Breakdown the sipcall");
            debugint(sipcall_status);
            if(GroupBusy == true)
            {
                err=PN_Ring_Stop();
                err=PN_Audio_Stop();
                GroupBusy = false;
            }
            msg_task msg;
            msg.msg_type = MSG2SIPCALL4QT;
            msg.m_type = 'a';
            strcpy(msg.text,"0");
            m_msq.Msq_Send_Task(msg);
            break;
        // case PN_CFGID_MEDIA_GROUPPOLL_STOP:
            // debugstring("stop the sipcall");
            // debugint(sipcall_status);
            
            // BreakDown = false;
            // err=PN_Ring_Stop();
            // err=PN_Audio_Stop();
            // pthread_mutex_lock(&mutex);
            // sipcall_status=SIPCALL_FREE;
            // pthread_mutex_unlock(&mutex);
			// if(GroupBusy == false && sipcall_status == SIPCALL_FREE && BreakDown == false)
			// {
				// PN_Media_SipCall_GroupNum_Query_XML_Send();//request for group
            // }
            // break;
        case PN_CFGID_MEDIA_LOGIN_STATUS:
            PN_Login_Detect_Success();
printf("PN_Login_Detect_Success()  is ok---------------\n");
			if(GroupBusy == false && sipcall_status == SIPCALL_FREE && BreakDown == false)
			{
printf("if(GroupBusy  in --------------------\n");
				PN_Media_SipCall_GroupNum_Query_XML_Send();//request for group
                usleep(5000);
			}
            break;
        default:
            err=PN_ERR_INVALID_MSG;
            break;
    }
printf("PN_CFGID_MEDIA_LOGIN_STATUS:   is ok out Control fun\n");
    return err;
}

void CMedia::PN_Media_Setitimer()
{
    signal(SIGALRM, PN_Media_Timer1);
    // struct itimerval itv1;
    itv1.it_interval.tv_sec = 0;
    itv1.it_interval.tv_usec = 0;
    itv1.it_value.tv_sec = 10;
    itv1.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL,&itv1,NULL);
    return ;
}

void CMedia::PN_Media_Timer1(int sig)
{
    m_this->PN_Media_Timer_Proccess1();
}

void CMedia::PN_Media_Timer_Proccess1()
{
    if(Rec_flag == true)
    {
        itv1.it_interval.tv_sec = 0;
        itv1.it_interval.tv_usec = 0;
        itv1.it_value.tv_sec = 0;
        itv1.it_value.tv_usec = 0;
        debugstring("Received the Group invite response@@@@@@@\n");
    }
    else
    {
        PN_Media_SipCall_GroupNum_Query_XML_Send();
        debugstring("Send other requiter!!!!!!!!!!!!!!!\n");
    }
    
}

int CMedia::PN_Media_Start()
{
    if(m_prtpsend != NULL)
        return PN_ERR_OK;
    else
    {
        m_prtpsend = new CRtpSend();
        if(m_prtpsend == NULL)
            return PN_ERR_FAILED;
        m_prtpsend->video_format=m_media_info.video_info.format;
        if(m_prtpsend->init(m_media_server_info.destip,m_media_server_info.destport,m_media_server_info.ssrc,m_media_server_info.guid))
        {
            debugstring("init RTP error-----------");
            return PN_ERR_FAILED;
        }
        m_prtpsend->run();
        debugstring("new Video rtp//////////////////////////");
    }
    return PN_ERR_OK;
}

int CMedia::PN_Audio_Start()
{
    if(m_pAudiortpsend != NULL)
    {
        debugstring("the speech exists,something goes wrong ");
        return PN_ERR_OK;
    }
    else
    {
        m_pAudiortpsend = new AudioRtpSend();
        if(m_pAudiortpsend == NULL)
            return PN_ERR_FAILED;
        if(m_pAudiortpsend->init(m_audio_server_info.destip,m_audio_server_info.destport,m_audio_server_info.ssrc,m_audio_server_info.guid,m_media_info.dev_info.id,m_media_info.dev_info.level,m_media_info.dev_info.group_num))
        {
            debugstring("init RTP error----------- ");
            return PN_ERR_FAILED;
        }
        // pthread_mutex_lock(&mutex);
        // sipcall_status=SIPCALL_BUSY;
        // pthread_mutex_unlock(&mutex);
        m_pAudiortpsend->run();
        debugstring("new Audio rtp//////////////////////////");
    }
    return PN_ERR_OK;
}

int CMedia::PN_Ring_Start()
{
    if(m_pRing != NULL)
        return PN_ERR_OK;
    else
    {
        m_pRing = new CRing();
        if(m_pRing == NULL)
            return PN_ERR_FAILED;
        if(m_pRing->RingRun())
        {
            debugstring("The sipcall ring error----------- ");
            return PN_ERR_FAILED;
        }
    }
    return PN_ERR_OK;
}

int CMedia::PN_Media_Stop()
{
    if(m_prtpsend != NULL)
    {
        m_prtpsend->stop();
        delete m_prtpsend;
        m_prtpsend = NULL;
    }
    return PN_ERR_OK;
}

int CMedia::PN_Audio_Stop()
{
    if(m_pAudiortpsend != NULL)
    {
        m_pAudiortpsend->stop();
        delete m_pAudiortpsend;
        m_pAudiortpsend = NULL;
    }

	GroupBusy = false;
    return PN_ERR_OK;
}

int CMedia::PN_Ring_Stop()
{
    if(m_pRing != NULL)
    {
        delete m_pRing;
        m_pRing = NULL;
    }
    return PN_ERR_OK;
}

int CMedia::PN_Media_Setconfig_Video_Invite(void* config_data)
{
    int err;
    pn_media_invite_info *pmedia_invite_info;
    pmedia_invite_info = (pn_media_invite_info *)config_data;

    err=GetIpPort((void *)&m_media_server_info,pmedia_invite_info->connect_value);
    GetGuidSsrc((void *)&m_media_server_info,pmedia_invite_info->magic_value);

    return err;
}

int CMedia::PN_Media_Setconfig_SipCall_Invite(void* config_data)
{
    msg_task msg;
    pn_audio_invite_info *paudio_invite_info;
    paudio_invite_info = (pn_audio_invite_info *)config_data;
    msg.msg_type = MSG2SIPCALL4QT;

    strcpy(msg.text,paudio_invite_info->from_number);
    strcat(msg.text,"to");
    strcat(msg.text,paudio_invite_info->to_number);

    if(sipcall_status != SIPCALL_CALLING)
    {
        pthread_mutex_lock(&mutex);
        sipcall_status = SIPCALL_FREE;
        pthread_mutex_unlock(&mutex);
        return PN_ERR_FAILED;
    }

    if(strcmp(paudio_invite_info->sipcall_code,"200") != 0)
    {
        pthread_mutex_lock(&mutex);
        sipcall_status = SIPCALL_FREE;
        pthread_mutex_unlock(&mutex);
        msg.m_type = '2';
        m_msq.Msq_Send_Task(msg);
        return PN_ERR_FAILED;
    }
	
    msg.m_type = '3';
    debugstring("start to send speech response message to QT");
    debugint(sipcall_status);
    m_msq.Msq_Send_Task(msg);
    GetIpPort((void*)&m_audio_server_info,paudio_invite_info->connect_value);
    GetGuidSsrc((void*)&m_audio_server_info,paudio_invite_info->magic_value);
	debugstr(paudio_invite_info->connect_value);

    return PN_ERR_OK;
}

int CMedia::PN_Media_Setconfig_Group_SipCall_Invite(void* config_data)
{
    msg_task msg;
    pn_audio_invite_info *paudio_invite_info;
    paudio_invite_info = (pn_audio_invite_info *)config_data;
     //used in AudioRtpSend.cpp
    strcpy(m_media_info.dev_info.level,paudio_invite_info->level);
    strcpy(m_media_info.dev_info.group_num,paudio_invite_info->to_number);

    if(strcmp(paudio_invite_info->sipcall_code,"200") != 0)
    {
        pthread_mutex_lock(&mutex);
        sipcall_status = SIPCALL_FREE;
        pthread_mutex_unlock(&mutex);
        return PN_ERR_FAILED;
    }
    // Send 5/'a'/group_num to QT
    msg.msg_type = MSG2SIPCALL4QT;
    msg.m_type = 'a';
    strcpy(msg.text,paudio_invite_info->to_number);
    m_msq.Msq_Send_Task(msg);

    Rec_flag = true;
    debugint(sipcall_status);
    GetIpPort((void*)&m_audio_server_info,paudio_invite_info->connect_value);
    GetGuidSsrc((void*)&m_audio_server_info,paudio_invite_info->magic_value);

    return PN_ERR_OK;
}

int CMedia::PN_Media_Setconfig_SipCall_Invited(void* config_data)
{
    int err;
    char code[5];
    msg_task msg;
    pn_audio_invite_info *paudio_invite_info;
    paudio_invite_info = (pn_audio_invite_info *)config_data;

    strcpy(msg.text,paudio_invite_info->from_number);
    strcat(msg.text,"to");
    strcat(msg.text,paudio_invite_info->to_number);
    debugstring("receive the sipcall from server");
    debugint(sipcall_status);
    if(sipcall_status==SIPCALL_FREE){
		PN_Audio_Stop(); //stop group
        msg.msg_type = MSG2SIPCALL4QT;
        msg.m_type = '4';
        debugstring("start to send invited message to QT");
        m_msq.Msq_Send_Task(msg);
        pthread_mutex_lock(&mutex);
        sipcall_status=SIPCALL_CALLED;
        pthread_mutex_unlock(&mutex);
        PN_Ring_Start();

        err=GetIpPort((void*)&m_audio_server_info,paudio_invite_info->connect_value);
        GetGuidSsrc((void*)&m_audio_server_info,paudio_invite_info->magic_value);
		debugstr(paudio_invite_info->connect_value);
    }
    else{
        strcpy(code,"525");
        err=PN_Media_SipCall_Invite_XML_Ack(paudio_invite_info->from_number,paudio_invite_info->to_number,code);
    }

    return err;
}

int CMedia::PN_Media_Setconfig_SipCall_Cancel(void* config_data)
{
    msg_task msg;
    pn_audio_invite_info *paudio_invite_info;

    paudio_invite_info = (pn_audio_invite_info *)config_data;
    debugstring("receive the speech cancel from server");
    debugint(sipcall_status);

    if(sipcall_status==SIPCALL_CALLED){
        msg.msg_type = MSG2SIPCALL4QT;
        msg.m_type = '2';
        strcpy(msg.text,paudio_invite_info->from_number);
        strcat(msg.text,"to");
        strcat(msg.text,paudio_invite_info->to_number);
        debugstring("start to send cancel message to QT");
        m_msq.Msq_Send_Task(msg);
        strcpy(paudio_invite_info->sipcall_code,"487");
        strcpy(paudio_invite_info->cancel_code,"200");
        pthread_mutex_lock(&mutex);
        sipcall_status=SIPCALL_FREE;
        pthread_mutex_unlock(&mutex);
    }
    else
        strcpy(paudio_invite_info->cancel_code,"0");

    return PN_ERR_OK;
}

int CMedia::PN_Media_Setconfig_SipCall_Bye(void* config_data)
{
    msg_task msg;
    pn_audio_invite_info *paudio_invite_info;

    paudio_invite_info = (pn_audio_invite_info *)config_data;
    msg.msg_type = MSG2SIPCALL4QT;
    msg.m_type = '8';
    strcpy(msg.text,paudio_invite_info->from_number);
    strcat(msg.text,"to");
    strcat(msg.text,paudio_invite_info->to_number);
    debugstring("start to send bye message to QT");
    debugint(sipcall_status);
    m_msq.Msq_Send_Task(msg);

    return PN_ERR_OK;
}
#if 0
int CMedia::PN_Media_Setconfig_Group_Busy_Invite(void* config_data)
{
    int err;
    char to_num[20];
    pn_group_busy_invite_info *pgroup_busy_invite_info;
    pgroup_busy_invite_info = (pn_group_busy_invite_info *)config_data;
    CMedia *pMedia;

    sprintf(to_num,"r%s",pgroup_busy_invite_info->group_number);
    debugint(sipcall_status);
    if(sipcall_status==SIPCALL_FREE){
        // msg.msg_type = MSG2SIPCALL4QT;
        debugstring("start to send group invited message to QT");
        PN_Media_SipCall_Invite_XML_Send(pgroup_busy_invite_info->phone_number,to_num);
        pthread_mutex_lock(&mutex);
        sipcall_status=SIPCALL_CALLING;
        pthread_mutex_unlock(&mutex);
    }
    else
        pMedia->PN_Media_SipCall_Abnormal_Status(pgroup_busy_invite_info->phone_number, to_num);

    return err;
}
#endif

int CMedia::PN_Media_Setconfig_Video_BYE(void* config_data)
{
    int err;
    pn_media_invite_info *pmedia_invite_info;
    pmedia_invite_info = (pn_media_invite_info *)config_data;

    err=GetIpPort((void*)&m_media_server_info,pmedia_invite_info->connect_value);
    //GetGuidSsrc((void*)&m_media_server_info,pmedia_invite_info->magic_value);

    return err;
}

int CMedia::PN_Media_Setconfig_Audio(void* config_data)
{
    pn_media_audio_info* pmedia_audia_info;

    pmedia_audia_info=(pn_media_audio_info*)config_data;

    return PN_ERR_OK;
}

int CMedia::PN_Media_Getconfig_Video(void* config_data)
{
    pn_media_video_info* pmedia_video_info;

    pmedia_video_info=(pn_media_video_info*)config_data;
    memcpy(pmedia_video_info,&m_media_info.video_info,sizeof(pn_media_video_info));
    return PN_ERR_OK;
}

int CMedia::PN_Media_Getconfig_Audio(void* config_data)
{
    pn_media_audio_info* pmedia_audia_info;

    pmedia_audia_info=(pn_media_audio_info*)config_data;
    memcpy(pmedia_audia_info,&m_media_info.audio_info,sizeof(pn_media_audio_info));
    return PN_ERR_OK;
}

int CMedia::PN_Media_Getconfig_Label(void* config_data)
{
    pn_media_label_info* pmedia_label_info;

    pmedia_label_info=(pn_media_label_info*)config_data;
    memcpy(pmedia_label_info,&m_media_info.label_info,sizeof(pn_media_label_info));
    return PN_ERR_OK;
}

int CMedia::PN_Media_Setconfig_Video(void* config_data)
{
    int err=0;
    msg_control msg;
    pn_media_video_info* pmedia_video_info;

    pmedia_video_info=(pn_media_video_info*)config_data;

    if(pmedia_video_info->bitrate < 0 || pmedia_video_info->bitrate > m_media_info.video_info.maxbitrate)
    {
        debugstring("the bitrate is too large!");
        return PN_ERR_FAILED;
    }

    debugint(pmedia_video_info->bitrate);
    msg.m_type = DEVBITRATE;
    msg.value = pmedia_video_info->bitrate;
    msg.msg_type = MSGENCCONTROL;
    err=m_msq.Msq_Send_Control(msg);

    if(err!=PN_ERR_OK)
        return err;
    else
        m_media_info.video_info.bitrate=pmedia_video_info->bitrate;

    return PN_ERR_OK;
}

int CMedia::PN_Media_Save_Config(int config_id)
{
    char tmp[10];

    if(config_id==PN_CFGID_MEDIA_VIDEO)
    {
        sprintf(tmp,"%d",m_media_info.video_info.type);
        m_ini.ConfigSetKey(CFG_FILE, "VIDEO_INFO", "type", tmp);
        sprintf(tmp,"%d",m_media_info.video_info.format);
        m_ini.ConfigSetKey(CFG_FILE, "VIDEO_INFO", "format", tmp);
        sprintf(tmp,"%d",m_media_info.video_info.maxbitrate);
        m_ini.ConfigSetKey(CFG_FILE, "VIDEO_INFO", "maxbitrate", tmp);
        sprintf(tmp,"%d",m_media_info.video_info.bitrate);
        m_ini.ConfigSetKey(CFG_FILE, "VIDEO_INFO", "bitrate", tmp);
        sprintf(tmp,"%d",m_media_info.video_info.framerate);
        m_ini.ConfigSetKey(CFG_FILE, "VIDEO_INFO", "framerate", tmp);
        sprintf(tmp,"%d",m_media_info.video_info.bright);
        m_ini.ConfigSetKey(CFG_FILE, "VIDEO_INFO", "bright", tmp);
        sprintf(tmp,"%d",m_media_info.video_info.contrast);
        m_ini.ConfigSetKey(CFG_FILE, "VIDEO_INFO", "contrast", tmp);
        sprintf(tmp,"%d",m_media_info.video_info.saturation);
        m_ini.ConfigSetKey(CFG_FILE, "VIDEO_INFO", "saturation", tmp);
    }

    if(config_id==PN_CFGID_MEDIA_AUDIO)
    {
        sprintf(tmp,"%d",m_media_info.audio_info.type);
        m_ini.ConfigSetKey(CFG_FILE, "AUDIO_INFO", "type", tmp);
        sprintf(tmp,"%d",m_media_info.audio_info.nchannel);
        m_ini.ConfigSetKey(CFG_FILE, "AUDIO_INFO", "nchannel", tmp);
        sprintf(tmp,"%d",m_media_info.audio_info.bitrate);
        m_ini.ConfigSetKey(CFG_FILE, "AUDIO_INFO", "bitrate", tmp);
        sprintf(tmp,"%d",m_media_info.audio_info.samplerate);
        m_ini.ConfigSetKey(CFG_FILE, "AUDIO_INFO", "samplerate", tmp);
        sprintf(tmp,"%d",m_media_info.audio_info.samplebits);
        m_ini.ConfigSetKey(CFG_FILE, "AUDIO_INFO", "samplebits", tmp);
    }

    if(config_id==PN_CFGID_MEDIA_LABEL)
    {
        sprintf(tmp,"%d",m_media_info.label_info.labeltime);
        m_ini.ConfigSetKey(CFG_FILE, "LABEL_INFO", "labeltime", tmp);
        sprintf(tmp,"%d",m_media_info.label_info.labelgps);
        m_ini.ConfigSetKey(CFG_FILE, "LABEL_INFO", "labelgps", tmp);
    }

    return PN_ERR_OK;
}

int CMedia::PN_Media_Read_Config(int config_id)
{
    char tmp[10];
	char dev[20];

    if(config_id==PN_CFGID_MEDIA_VIDEO)
    {
        m_ini.ConfigGetKey(CFG_FILE, "VIDEO_INFO", "type", tmp);
        m_media_info.video_info.type=atoi(tmp);
        m_ini.ConfigGetKey(CFG_FILE, "VIDEO_INFO", "format", tmp);
        m_media_info.video_info.format=atoi(tmp);
        m_ini.ConfigGetKey(CFG_FILE, "VIDEO_INFO", "maxbitrate", tmp);
        m_media_info.video_info.maxbitrate=atoi(tmp);
        m_ini.ConfigGetKey(CFG_FILE, "VIDEO_INFO", "bitrate", tmp);
        m_media_info.video_info.bitrate=atoi(tmp);
        m_ini.ConfigGetKey(CFG_FILE, "VIDEO_INFO", "framerate", tmp);
        m_media_info.video_info.framerate=atoi(tmp);
        m_ini.ConfigGetKey(CFG_FILE, "VIDEO_INFO", "bright", tmp);
        m_media_info.video_info.bright=atoi(tmp);
        m_ini.ConfigGetKey(CFG_FILE, "VIDEO_INFO", "contrast", tmp);
        m_media_info.video_info.contrast=atoi(tmp);
        m_ini.ConfigGetKey(CFG_FILE, "VIDEO_INFO", "saturation", tmp);
        m_media_info.video_info.saturation=atoi(tmp);
    }

    if(config_id==PN_CFGID_MEDIA_AUDIO)
    {
        m_ini.ConfigGetKey(CFG_FILE, "AUDIO_INFO", "type", tmp);
        m_media_info.audio_info.type=atoi(tmp);
        m_ini.ConfigGetKey(CFG_FILE, "AUDIO_INFO", "nchannel", tmp);
        m_media_info.audio_info.nchannel=atoi(tmp);
        m_ini.ConfigGetKey(CFG_FILE, "AUDIO_INFO", "bitrate", tmp);
        m_media_info.audio_info.bitrate=atoi(tmp);
        m_ini.ConfigGetKey(CFG_FILE, "AUDIO_INFO", "samplerate", tmp);
        m_media_info.audio_info.samplerate=atoi(tmp);
        m_ini.ConfigGetKey(CFG_FILE, "AUDIO_INFO", "samplebits", tmp);
        m_media_info.audio_info.samplebits=atoi(tmp);
    }

    if(config_id==PN_CFGID_MEDIA_LABEL)
    {
        m_ini.ConfigGetKey(CFG_FILE, "LABEL_INFO", "labeltime", tmp);
        m_media_info.label_info.labeltime=atoi(tmp);
        m_ini.ConfigGetKey(CFG_FILE, "LABEL_INFO", "labelgps", tmp);
        m_media_info.label_info.labelgps=atoi(tmp);
    }
	
	if(config_id==PN_CFGID_MEDIA_DEV)
	{
		m_ini.ConfigGetKey(CFG_FILE, "DEV_INFO", "id", dev);
		strcpy(m_media_info.dev_info.id,dev);
	}

    return PN_ERR_OK;
}

int CMedia::PN_Media_SipCall_Listening()
{
    int err;

    err = pthread_create (&listening_thread, NULL , (void*(*)(void*))PN_Media_SipCall_Thread_Listening, (void *)this );
    if ( err!= 0)
    {
        printf ( "can't create socket listening thread: %s\n" , strerror(err)) ;
        return PN_ERR_FAILED;
    }

    return PN_ERR_OK;
}

void CMedia::PN_Media_SipCall_Thread_Listening(void* params)
{
    int err;
    char from_num[10]={0};
    char to_num[10]={0};
    char code[5]={0};
    char *p;

    CMedia *pMedia = (CMedia *) params;
    msg_task msg;
    memset(&msg,0,sizeof(msg_task));

    while(1){
        err=pMedia->m_msq.Msq_Read_Task(&msg,4);
        debugstring("==================================================");
        if(err==PN_ERR_OK){
            debugstr(msg.text);
            p=strstr(msg.text,"to");
            if(p==NULL)
            {
                debugstring("the send sipcall number error");
                break;
            }
            memset(from_num, 0x00, sizeof(from_num));
            memset(to_num, 0x00, sizeof(to_num));
            strncpy(from_num,msg.text,p-msg.text);
            strcpy(to_num,p+2);

            debugstring("receive the message from QT");
            switch(msg.m_type){
                case '1':
                    debugstring("Start to send call message");
                    debugstr(from_num);
                    debugstr(to_num);
                    debugint(pMedia->sipcall_status);
                    if(pMedia->sipcall_status==SIPCALL_FREE){
						pMedia->PN_Audio_Stop();//stop group
                        pMedia->PN_Media_SipCall_Invite_XML_Send(from_num,to_num);
                        pthread_mutex_lock(&(pMedia->mutex));
                        pMedia->sipcall_status=SIPCALL_CALLING;
                        pthread_mutex_unlock(&(pMedia->mutex));
                    }
                    else
                        pMedia->PN_Media_SipCall_Abnormal_Status(from_num, to_num);
                    break;

                case '5':
                    debugstring("receive the call");
                    debugstr(from_num);
                    debugstr(to_num);
                    debugint(pMedia->sipcall_status);
                    strcpy(code,"200");
                    if (pMedia->sipcall_status==SIPCALL_CALLED){
                        pMedia->PN_Ring_Stop(); //stop ringing
                        pMedia->PN_Media_SipCall_Invite_XML_Ack(from_num,to_num,code);
                        pMedia->PN_Audio_Start();
                        pthread_mutex_lock(&(pMedia->mutex));
                        pMedia->sipcall_status=SIPCALL_BUSY;
                        pthread_mutex_unlock(&(pMedia->mutex));
                    }
                    else
                        pMedia->PN_Media_SipCall_Abnormal_Status(from_num, to_num);
                    break;

                case '6':
                    debugstring("refuse the call");
                    debugstr(from_num);
                    debugstr(to_num);
                    debugint(pMedia->sipcall_status);
                    strcpy(code,"486");
                    if (pMedia->sipcall_status==SIPCALL_CALLED){
                        pMedia->PN_Ring_Stop(); //stop ringing
                        pMedia->PN_Media_SipCall_Invite_XML_Ack(from_num,to_num,code);
                        pthread_mutex_lock(&(pMedia->mutex));
                        pMedia->sipcall_status=SIPCALL_FREE;
                        pthread_mutex_unlock(&(pMedia->mutex));
                    }
                    else
                        pMedia->PN_Media_SipCall_Abnormal_Status(from_num, to_num);
                    break;

                case '7':
                    debugstring("cancel or bye the call");
                    debugstr(from_num);
                    debugstr(to_num);
                    debugint(pMedia->sipcall_status);
                    if(pMedia->sipcall_status==SIPCALL_CALLING){
                        pMedia->PN_Media_SipCall_Cancel_XML_Send(from_num,to_num);
                        pthread_mutex_lock(&(pMedia->mutex));
                        pMedia->sipcall_status=SIPCALL_FREE;
                        pthread_mutex_unlock(&(pMedia->mutex));
                    }
                    else if(pMedia->sipcall_status==SIPCALL_BUSY){
                        pMedia->PN_Media_SipCall_Bye_XML_Send(from_num,to_num);
                        pMedia->PN_Audio_Stop();
                        pthread_mutex_lock(&(pMedia->mutex));
                        pMedia->sipcall_status=SIPCALL_FREE;
                        debugint(pMedia->sipcall_status);
                        pthread_mutex_unlock(&(pMedia->mutex));
                    }
                    else
                        pMedia->PN_Media_SipCall_Abnormal_Status(from_num, to_num);
                    break;
                default:
                    break;
            }
        }
        sleep(1);
    }
}

void CMedia::PN_Media_SipCall_Abnormal_Status(char* from_num, char* to_num)
{
    debugstring("In Sipcall Abnormal status----Improtant Error");
    msg_task msg;
    msg.msg_type = MSG2SIPCALL4DEV;
    msg.m_type = '9';
    sprintf(msg.text,"%sto%s",from_num,to_num);
    PN_Ring_Stop();
    PN_Audio_Stop();
    pthread_mutex_lock(&mutex);
    sipcall_status=SIPCALL_FREE;
    pthread_mutex_unlock(&mutex);
    m_msq.Msq_Send_Task(msg);
    PN_Media_SipCall_Bye_XML_Send(from_num,to_num);
    return;
}

int CMedia::GetIpPort(void* config_data,char*peer)
{
    char ip_dest[20]= "";
    char port_dest[20]= "";

    unsigned int user_ip = 0;
    unsigned short user_port =0;

    media_server_info *pmedia_server_info;
    pmedia_server_info = (media_server_info *)config_data;

    sscanf(peer,"%s%*s%s",ip_dest,port_dest);
    debugstr(peer);
    debugstr(ip_dest);
    debugstr(port_dest);

    user_ip = inet_addr(ip_dest);
    if (user_ip == INADDR_NONE)
    {
        debugstring("invalid user ip-----------\n ");
        return PN_ERR_FAILED;
    }

    user_ip = ntohl(user_ip);
    user_port= atoi(port_dest);

    pmedia_server_info->destip=user_ip;
    pmedia_server_info->destport=user_port;
    debugint(pmedia_server_info->destip);
    debugint(pmedia_server_info->destport);

    return PN_ERR_OK;
}

int CMedia::GetGuidSsrc(void* config_data,char* magic)
{
    media_server_info *pmedia_server_info;
    pmedia_server_info = (media_server_info *)config_data;

    str2hex(magic,pmedia_server_info->guid,16);
    memcpy((void*)&pmedia_server_info->ssrc,(void *)&pmedia_server_info->guid[12],4);
    pmedia_server_info->ssrc = htonl(pmedia_server_info->ssrc);

    return PN_ERR_OK;
}

void CMedia::str2hex(char hex[], char * output,unsigned int num)
{
    unsigned int i,j;
    if(strlen(hex) <  2* num)
    {
        return;
    }
    for (i = 0; i<num; ++i)
    {
        j= 2*i;
        output[i] = (char) ((hex_digit_to_val(*(hex+j)) << 4) + hex_digit_to_val(*(hex+(j+1))));
    }
}

unsigned CMedia::hex_digit_to_val(unsigned char c)
{
    if (c <= '9')
        return (c-'0') & 0x0F;
    else if (c <= 'F')
        return  (c-'A'+10) & 0x0F;
    else
        return (c-'a'+10) & 0x0F;
}

void CMedia::SetCallback(SendCallback callback,void* inst)
{
    callback_send = callback;
    m_inst=inst;
}

int CMedia::PN_Media_SipCall_Invite_XML_Send(char* from_num,char* to_num)
{
    char buffer[300];
    memset(buffer,'\0',sizeof(buffer));

    sprintf(buffer,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<speech>\n"
            "<audio>G.711</audio>\n"
            "<kbps>8000</kbps>\n"
            "<self>192.168.1.158 UDP 5200</self>\n"
            "<from_number>%s</from_number>\n"
            "<to_number>%s</to_number>\n"
            "</speech>",from_num,to_num);

    if(callback_send!=NULL)
        callback_send(INVITE,buffer,m_inst);
    return 0;
}

int CMedia::PN_Media_SipCall_Invite_XML_Ack(char* from_num,char* to_num,char* code)
{
    char buffer[300];
    memset(buffer,'\0',sizeof(buffer));

    sprintf(buffer,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<speech_response>\n"
            "<code>%s</code>\n"
            "<from_number>%s</from_number>\n"
            "<to_number>%s</to_number>\n"
            "</speech_response>",code,from_num,to_num);

    if(callback_send!=NULL)
        callback_send(OK,buffer,m_inst);
    return 0;
}

int CMedia::PN_Media_SipCall_Cancel_XML_Send(char* from_num,char* to_num)
{
    char buffer[300];
    memset(buffer,'\0',sizeof(buffer));

    sprintf(buffer,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<speech_cancel>\n"
            "<from_number>%s</from_number>\n"
            "<to_number>%s</to_number>\n"
            "</speech_cancel>",from_num,to_num);

    if(callback_send!=NULL)
        callback_send(NOTIFY,buffer,m_inst);
    return 0;
}

int CMedia::PN_Media_SipCall_Bye_XML_Send(char* from_num,char* to_num)
{
    char buffer[300];
    memset(buffer,'\0',sizeof(buffer));

    sprintf(buffer,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<speech_bye>\n"
           "<from_number>%s</from_number>\n"
            "<to_number>%s</to_number>\n"
            "</speech_bye>",from_num,to_num);

    if(callback_send!=NULL)
        callback_send(BYE,buffer,m_inst);
    return 0;
}

int CMedia::PN_Media_SipCall_GroupNum_Query_XML_Send()
{
    char buffer[300] = {0};

    sprintf(buffer,"<?xml version=\"1.0\"?>\n"
            "<subscribe_grouppn>\n"
            "<dev_id>%s</dev_id>\n"
            "</subscribe_grouppn>",m_media_info.dev_info.id);
    if(callback_send!=NULL)
        callback_send(SUBSCRIBE,buffer,m_inst);
    PN_Media_Setitimer();

    return 0;
}

