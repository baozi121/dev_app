#ifndef __AUDIORTPSEND_H__
#define __AUDIORTPSEND_H__

#include "CThreadObject.h"
#include "PnShareMem.h"
#include "PnAudioRtpSend.h"
#include "hw_lib.h"

#include "rtpsession.h"
#include "rtppacket.h"
#include "rtpudpv4transmitter.h"
#include "rtpipv4address.h"
#include "rtpsessionparams.h"
#include "rtperrors.h"
#include "cJSON.h"
#include "Pnmsq.h"
#include "comm.h"

#define ACTIVE_TIME 20*1000

typedef struct _tag_AudioActiveData
{
    unsigned short cmd;
    unsigned short len;
    char guid[16];
}AudioActiveData;

struct AudioGroupData
{
	char reg[50];
	char lout[50]; 
    char begin[50];
    char signal[50];
    char end[50];
    char timeout[50];
    char terminate[50];
};

struct AudioStatus
{
    // static bool flag ;
    bool start_flag ;
    bool end_flag ;
};

class AudioRtpSend :public CThreadObject
{
    public:
        AudioRtpSend();
        ~AudioRtpSend();
        void runLoop();

        int init(unsigned int destip, unsigned short destport, unsigned int  ssrc,char * guid, char * devid,char * level,char* group_num);
        int CJSON_Socket_Listening();
        int CJSON_Socket_Send(const char *mess);
        int CJSON_Socket_Init(unsigned int ip,unsigned short port);
        struct AudioGroupData m_groupdata;
        struct AudioStatus g_status;

    private:
        static void ActivePacketSender(void *params);
        static void CJSON_Socket_Thread_Listening(void* params);
        void DevJsonData(char * id, char * lev);
        void ParseJson(cJSON * pJson,int iCnt);
        int PnAudioUploadControl();
        int p2(unsigned short semnum);
        int v2(unsigned short semnum);
        PnShMem *mUpAudioShm;
        char *mUpAudioTemp;

        unsigned short portbase;

        void *h_activethread;
        int activethread_quit;
        void *activethread_quit_event;

        PnAudioRtpSend Audiosess;
        void *m_criticalsection;
        static const int PT_SIPCALL_RTP = 8;
        int qtsemid;
        int cJSON_socket;
        struct sockaddr_in CServer_addr;
        struct sockaddr_in cJSON_addr;
        pthread_t CJSON_listening_thread;
        CMSQ C_msq;
        char Group_num[10];
};

#endif
