#include<unistd.h>
#include "AudioRtpSend.h"
#include <iostream>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include "comm.h"
#include <pthread.h>
#include <signal.h>
#include "cJSON.h"
#include "Socket.h"

using namespace std;
#define UP_AUDIO_SHM_DIR "/shm/audio"
#define UP_AUDIO_SHARE_SIZE 160
#define MEDIA_INVITE    0
#define SIPCALL_INVITE  1

static AudioActiveData m_activedata;
extern bool GroupBusy;
static  bool flag = false;

static void checkerror(int rtperr)
{
    if (rtperr < 0)
    {
        std::cout << "ERROR: " << RTPGetErrorString(rtperr) << std::endl;
        exit(-1);
    }
}

AudioRtpSend::AudioRtpSend()
{
    mUpAudioShm = NULL;
    mUpAudioTemp = NULL;
    portbase = 0;
    m_criticalsection = hw_init_critical();

    activethread_quit = 0;
    activethread_quit_event =  NULL;
    h_activethread = NULL;
    CJSON_listening_thread = 0;
    g_status.start_flag = false;
    g_status.end_flag = false;
    
    // QTPTT_listening_thread = 0;
}

AudioRtpSend::~AudioRtpSend()
{
	if(GroupBusy == true)
    {
		Audiosess.SendPacket((void *)m_groupdata.lout,strlen(m_groupdata.lout),PT_SIPCALL_RTP,false,160);
        close(cJSON_socket);
    }
    activethread_quit = 1;
    hw_wait_event(activethread_quit_event, -1);
    debugstring("quit the thread of active");

    if(activethread_quit_event)
    {
        hw_release_event(activethread_quit_event);
        activethread_quit_event = NULL;
    }

    if(h_activethread)
    {
        hw_release_thread(h_activethread);
        debugstring("release the thread of active");
        h_activethread = NULL;
    }

    hw_release_critical(m_criticalsection);

    if(mUpAudioShm){
        delete mUpAudioShm;
        mUpAudioShm = NULL;
        free(mUpAudioTemp);
    }

    // if(Group_num){
        // delete Group_num;
        // Group_num = NULL;
        // free(Group_num);
    // }
    if(0 != CJSON_listening_thread)
        while(0==pthread_kill(CJSON_listening_thread,0))
            pthread_cancel(CJSON_listening_thread);

    // if(0 != QTPTT_listening_thread)
        // while(0==pthread_kill(QTPTT_listening_thread,0))
            // pthread_cancel(QTPTT_listening_thread);
}

int AudioRtpSend::init(unsigned int destip, unsigned short destport, unsigned int  ssrc, char * guid, char * devid, char * level,char* group_num)
{
    int status;
    strcpy(Group_num,group_num);

    //create Audio shm
    mUpAudioTemp = (char *)malloc(UP_AUDIO_SHARE_SIZE);
    if(mUpAudioTemp == NULL){
        cerr<<"create Up Audiotemp buffer error"<<endl;
        return -1;
    }
    mUpAudioShm = new PnShMem(UP_AUDIO_SHM_DIR,UP_AUDIO_SHARE_SIZE);
    if(mUpAudioShm == NULL){
        cerr<<"create Up audio shm error"<<endl;
        free(mUpAudioTemp);
        return -1;
    }

    PnAudioUploadControl();

    portbase = 5500;
    RTPUDPv4TransmissionParams transparams;
    RTPSessionParams sessparams;

    sessparams.SetOwnTimestampUnit(1.0/8000.0);

    sessparams.SetAcceptOwnPackets(true);
    sessparams.SetUsePollThread(true);

    transparams.SetPortbase(portbase);
    status = Audiosess.Create(sessparams,&transparams);
    checkerror(status);

    RTPIPv4Address addr(destip,destport);

    status = Audiosess.AddDestination(addr);

    Audiosess.SetAudioMTU(1000);
    Audiosess.SetDefaultPayloadType(PT_SIPCALL_RTP);
    Audiosess.SetDefaultMark(false);
    Audiosess.SetDefaultTimestampIncrement(160);
    if(ssrc != 0)
    {
        Audiosess.SetSSRC(ssrc);
    }

     // m_activedata.cmd = htons(1);
     // m_activedata.len = htons(16);
    memset(&(m_activedata.guid),0x55,sizeof(m_activedata.guid));

    // open the active thread
    activethread_quit = 0;
    activethread_quit_event = hw_create_event();
    h_activethread = hw_create_thread((void *)AudioRtpSend::ActivePacketSender, (void *)this);
    if(h_activethread == NULL)
    {
        return -1;
    }
	if(GroupBusy == true)
	{
        CJSON_Socket_Init(destip,destport);
        CJSON_Socket_Listening();
        DevJsonData(devid,level);
		hw_enter_critical(m_criticalsection);
        Audiosess.SendPacket((void *)m_groupdata.reg,strlen(m_groupdata.reg),PT_SIPCALL_RTP,false,160);
        CJSON_Socket_Send(m_groupdata.signal);
	}
	else
	{
		hw_enter_critical(m_criticalsection);
		Audiosess.SendPacket((void *)&(m_activedata),sizeof(AudioActiveData),PT_SIPCALL_RTP,false,160);   //Audio heartbeat packet
		Audiosess.SendPacket((void *)&(m_activedata),sizeof(AudioActiveData),PT_SIPCALL_RTP,false,160);   //Audio heartbeat packet
	}
    hw_leave_critical(m_criticalsection);
	
    return 0;
}

int AudioRtpSend::CJSON_Socket_Init(unsigned int ip,unsigned short port)
{
    unsigned int G_ip = 0;
    cJSON_socket = socket(AF_INET,SOCK_DGRAM,0);
    if( cJSON_socket < 0)
    {
        debugstring("Create cJSON Socket Failed!");
        return PN_ERR_FAILED;
    }
    memset(&cJSON_addr,0,sizeof(cJSON_addr));
    cJSON_addr.sin_family = AF_INET;
    cJSON_addr.sin_port = 5000;
    cJSON_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(cJSON_socket,(struct sockaddr *)&cJSON_addr,sizeof(cJSON_addr)))
    {
        printf("client Bind Port : Failed!\n");
        return PN_ERR_FAILED;
    }
    G_ip = htonl(ip);
    memset(&CServer_addr,0,sizeof(CServer_addr));
    CServer_addr.sin_family = AF_INET;
    CServer_addr.sin_port = htons(port);
    CServer_addr.sin_addr.s_addr = G_ip;//inet_addr("192.168.188.101");//htonl(INADDR_ANY);
    return PN_ERR_OK;
}

void AudioRtpSend::DevJsonData(char * id, char * lev)
{
    cJSON * pJsonRoot = NULL;
	pJsonRoot = cJSON_CreateObject();
	cJSON_AddStringToObject(pJsonRoot, "devid",id);
	char *p = cJSON_Print(pJsonRoot);
	strcpy(m_groupdata.reg, p);

	pJsonRoot = cJSON_CreateObject();
	cJSON_AddStringToObject(pJsonRoot, "logout",id);
    p = cJSON_Print(pJsonRoot);
	strcpy(m_groupdata.lout, p);

	pJsonRoot = cJSON_CreateObject();
	cJSON_AddStringToObject(pJsonRoot, "start",id);
	cJSON_AddStringToObject(pJsonRoot, "level",lev);
	p = cJSON_Print(pJsonRoot);
	strcpy(m_groupdata.begin, p);
    
	pJsonRoot = cJSON_CreateObject();
	cJSON_AddStringToObject(pJsonRoot, "signal",id);
	p = cJSON_Print(pJsonRoot);
	strcpy(m_groupdata.signal, p);

	pJsonRoot = cJSON_CreateObject();
	cJSON_AddStringToObject(pJsonRoot, "end",id);
	p = cJSON_Print(pJsonRoot);
	strcpy(m_groupdata.end, p);

	pJsonRoot = cJSON_CreateObject();
	cJSON_AddStringToObject(pJsonRoot, "timeout","200");
	p = cJSON_Print(pJsonRoot);
	strcpy(m_groupdata.timeout, p);

	pJsonRoot = cJSON_CreateObject();
	cJSON_AddStringToObject(pJsonRoot, "terminate","200");
	p = cJSON_Print(pJsonRoot);
	strcpy(m_groupdata.terminate, p);

	cJSON_Delete(pJsonRoot);
	free(p);
}

int AudioRtpSend::CJSON_Socket_Send(const char *mess)
{
    char *buf;
    int len = strlen(mess);
    buf = (char*)malloc(len+50);
    strcpy(buf,mess);

    printf("send buf = %s\n",buf);
    sendto(cJSON_socket,buf,strlen(buf),0,(struct sockaddr *)&CServer_addr,sizeof(CServer_addr));

    free(buf);
    return PN_ERR_OK;
}

void AudioRtpSend::runLoop()
{
    if(GroupBusy == true)
    {
        if(p2(0) == 0)
        {
            if(g_status.start_flag == false)
            {
                CJSON_Socket_Send(m_groupdata.begin);
                g_status.start_flag = true;
                g_status.end_flag = true;
            }
            if(flag == true)
            {
               int AudioStatus;
               short *pTmp = NULL;
               AudioStatus = mUpAudioShm->ReadShM(mUpAudioTemp,UP_AUDIO_SHARE_SIZE);
               v2(0);
               pTmp = (short *)mUpAudioTemp;
               hw_enter_critical(m_criticalsection);
               Audiosess.SendPacket((void *)pTmp,AudioStatus,PT_SIPCALL_RTP,false,160);
               hw_leave_critical(m_criticalsection);
            }
            else{
               v2(0);
            }
        }
        else{
            g_status.start_flag = false;
            if(g_status.end_flag == true)
            {
                CJSON_Socket_Send(m_groupdata.end);
                g_status.end_flag = false;
            }
        }
    }
    else{
        if(p2(0) == 0)
        {
            int AudioStatus;
            short *pTmp = NULL;
            AudioStatus = mUpAudioShm->ReadShM(mUpAudioTemp,UP_AUDIO_SHARE_SIZE);
            v2(0);
            pTmp = (short *)mUpAudioTemp;
            hw_enter_critical(m_criticalsection);
            Audiosess.SendPacket((void *)pTmp,AudioStatus,PT_SIPCALL_RTP,false,160);
            hw_leave_critical(m_criticalsection);
        }
    }
}

int AudioRtpSend::CJSON_Socket_Listening()
{
    int err;

    err = pthread_create (&CJSON_listening_thread, NULL , (void*(*)(void*))CJSON_Socket_Thread_Listening, (void *)this );
    if ( err!= 0)
    {
        printf ( "can't create socket listening thread: %s\n" , strerror(err)) ;
        return PN_ERR_FAILED;
    }
    // err = pthread_create (&QTPTT_listening_thread, NULL , (void*(*)(void*))QTPTT_Socket_Thread_Listening, (void *)this );
    // if ( err!= 0)
    // {
        // printf ( "can't create socket listening thread: %s\n" , strerror(err)) ;
        // return PN_ERR_FAILED;
    // }
    return PN_ERR_OK;
}

void AudioRtpSend::CJSON_Socket_Thread_Listening(void* params)
{
    AudioRtpSend *pRtpSend = (AudioRtpSend *) params;
    int err;
    char recv_buf[500];
    cJSON *pJson=NULL;
    unsigned int addr_len = sizeof(struct sockaddr_in);

    while(1)
    {
        memset(recv_buf,0,sizeof(recv_buf));
        err=recvfrom(pRtpSend->cJSON_socket,recv_buf,sizeof(recv_buf),0,(struct sockaddr *)&(pRtpSend->CServer_addr),&addr_len);
        if(err<0)
        {
            debugstring("cJSON socket recv error!\n");
        }
        printf("********************recv_buf=%s\n",recv_buf);
        pJson = cJSON_Parse(recv_buf);
        if(NULL == pJson)
        {
            debugstring("_______________________________");
        }
        pRtpSend->ParseJson(pJson,0);
    }
    cJSON_Delete(pJson);
}

void AudioRtpSend::ParseJson(cJSON * pJson,int iCnt)
{
    if(NULL == pJson)
    {
        // return -1;
    }
    switch(pJson->type)
    {
        case cJSON_False :
            {
                printf("%s : %d\n", pJson->string, pJson->valueint);
            }
            break;
        case cJSON_True :
            {
                printf("%s : %d\n", pJson->string, pJson->valueint);
            }
            break;
        case cJSON_NULL :
            {
                printf("%s : NULL\n", pJson->string);
            }
            break;
        case cJSON_Number :
            {
                printf("%s : %d | %lf\n", pJson->string, pJson->valueint, pJson->valuedouble);
            }
            break;
        case cJSON_String :
            {
                msg_task msg;
                memset(&msg,0,sizeof(msg));
                msg.msg_type = MSG2SIPCALL4QT;
                if((strcmp(pJson->string,"signal"))== 0)
                    printf("signal result:%s\n",pJson->valuestring);
 
                if((strcmp(pJson->string,"start")) == 0)
                {
                    printf("start result:%s\n",pJson->valuestring);
                    if(strcmp(pJson->valuestring,"200") == 0)
                    {
                        flag = true;
                        msg.m_type = 'b';
                        strcpy(msg.text,Group_num);
                        debugstring("start to send begin talk to QT");
                        C_msq.Msq_Send_Task(msg);
                    }
                    else
                    {
                        flag = false;
                    }
                 }

                if((strcmp(pJson->string,"level")) == 0)
                    printf("start result:%s\n",pJson->valuestring);

                if((strcmp(pJson->string,"end"))== 0)
                {
                    flag = false;
                    printf("end result:%s\n",pJson->valuestring);
                    msg.m_type = 'c';
                    strcpy(msg.text,Group_num);
                    debugstring("start to send end talk to QT");
                    C_msq.Msq_Send_Task(msg);
                }

                if((strcmp(pJson->string,"timeout")) == 0)
                {
                    printf("timeout result:%s\n",pJson->valuestring);
                    flag = false;
                    msg.m_type = 'd';
                    strcpy(msg.text,Group_num);
                    debugstring("start to send timeout to QT");
                    C_msq.Msq_Send_Task(msg);
                    CJSON_Socket_Send(m_groupdata.timeout);
                }

                if((strcmp(pJson->string,"terminate")) == 0)
                {
                    printf("terminate result:%s\n",pJson->valuestring);
                    flag = false;
                    msg.m_type = 'e';
                    strcpy(msg.text,Group_num);
                    debugstring("start to send terminate talk to QT");
                    C_msq.Msq_Send_Task(msg);
                    CJSON_Socket_Send(m_groupdata.terminate);
                }
             }
            break;
        case cJSON_Array  :
        case cJSON_Object :
            {
                int iSize = cJSON_GetArraySize(pJson);
                int i = 0;
                iCnt++;
                for(i = 0; i < iSize; i++)
                {
                    cJSON * pSub = cJSON_GetArrayItem(pJson, i);
                    ParseJson(pSub, iCnt);
                }
            }
            break;
        default :
            break;
       }
}

void AudioRtpSend::ActivePacketSender(void *params)
{
    AudioRtpSend *pRtpSend = (AudioRtpSend *) params;

    while(1)
    {
        unsigned int tick = hw_get_sys_tick();
		if(GroupBusy == true)
		{		
			do
       		{
       		     if(pRtpSend->activethread_quit)
       		         goto END ;
       		     hw_delay(10);
       		}while ( (int)(hw_get_sys_tick()-tick) < 10000);//10s
			hw_enter_critical(pRtpSend->m_criticalsection);
            pRtpSend->Audiosess.SendPacket((void *)(pRtpSend->m_groupdata.reg),strlen(pRtpSend->m_groupdata.reg),PT_SIPCALL_RTP,false,160);
            pRtpSend->CJSON_Socket_Send(pRtpSend->m_groupdata.signal);
		}
		else
		{
			do
       		{
       		     if(pRtpSend->activethread_quit)
       		         goto END ;
       		     hw_delay(1);
       		}while ( (int)(hw_get_sys_tick()-tick) < 1000);//1s
			hw_enter_critical(pRtpSend->m_criticalsection);
			pRtpSend->Audiosess.SendPacket((void *)&(m_activedata),sizeof(AudioActiveData),PT_SIPCALL_RTP,false,160);
			pRtpSend->Audiosess.SendPacket((void *)&(m_activedata),sizeof(AudioActiveData),PT_SIPCALL_RTP,false,160);
		}
        hw_leave_critical(pRtpSend->m_criticalsection);
        hw_delay(5);
    }
END:
    hw_set_event(pRtpSend->activethread_quit_event);
    return;
}

int AudioRtpSend::PnAudioUploadControl()
{
    union semun arg;

    key_t key;
    key = ftok("/shm/qtControl",0);
    if((qtsemid = semget(key , 1, IPC_CREAT|IPC_EXCL|0666)) == -1)
    {
        if(errno==EEXIST)
        {
            errno = 0;
            qtsemid = semget(key , 1, IPC_CREAT|0666);
            return qtsemid;
        }
        else
        {
            cerr<<"semget semid failed!\n"<<endl;
            return qtsemid;
        }
    }

    arg.val = 0;
    if ((semctl(qtsemid, 0, SETVAL, arg)) == -1)
    {
        cerr<<"init semaphore error"<<endl;
        semctl(qtsemid, 0, IPC_RMID);
        return -1;
    }

    return qtsemid;
}

int AudioRtpSend::p2(unsigned short semnum)
{
    struct sembuf sops = {semnum, -1, IPC_NOWAIT};
    return semop(qtsemid, &sops, 1);
}

int AudioRtpSend::v2(unsigned short semnum)
{
    struct sembuf sops = {semnum, 1, IPC_NOWAIT};
    return semop(qtsemid, &sops, 1);
}
