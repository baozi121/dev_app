#ifndef __MEDIA_H__
#define __MEDIA_H__

#include "CRtpSend.h"
#include "AudioRtpSend.h"
#include "Ring.h"
#include "IniFile.h"
#include "comm.h"
#include "Pnmsq.h"

typedef struct
{
    unsigned short  destport;
    unsigned int    destip;
    unsigned int    ssrc;
    char            guid[16];
}media_server_info;

typedef enum{
    SIPCALL_FREE = 0,
    SIPCALL_CALLING,
    SIPCALL_CALLED,
    SIPCALL_BUSY
}sipcall_st;

class CMedia
{
    public:
        CMedia();
        ~CMedia();

        int PN_Media_Init();
        int PN_Media_Release();
        int PN_Media_Setconfig(int config_id ,void* config_data);
        int PN_Media_Getconfig(int config_id ,void* config_data);
        int PN_Media_Control(int ctrl_id);
        void SetCallback(SendCallback callback,void* inst) ;//flh add
        int PN_Media_SipCall_Invite_XML_Send(char* from_num,char* to_num);
        int PN_Media_SipCall_Invite_XML_Ack(char* from_num,char* to_num,char* code);
        int PN_Media_SipCall_Cancel_XML_Send(char* from_num,char* to_num);
        int PN_Media_SipCall_Bye_XML_Send(char* from_num,char* to_num);
		int PN_Media_SipCall_GroupNum_Query_XML_Send();

    private:
        int PN_Media_Start();
        int PN_Media_Stop();
        int PN_Audio_Start();
        int PN_Audio_Stop();
        int PN_Ring_Start();
        int PN_Ring_Stop();

        int PN_Media_Setconfig_Video_Invite(void* config_data);
        int PN_Media_Setconfig_SipCall_Invite(void* config_data);
        int PN_Media_Setconfig_SipCall_Invited(void* config_data);
        int PN_Media_Setconfig_SipCall_Cancel(void* config_data);
        int PN_Media_Setconfig_SipCall_Bye(void* config_data);
        int PN_Media_Setconfig_Group_Busy_Invite(void* config_data);
		int PN_Media_Setconfig_Group_SipCall_Invite(void* config_data);
        int PN_Media_Setconfig_Video_BYE(void* config_data);
        int PN_Media_Setconfig_Audio(void* config_data);
        int PN_Media_Setconfig_Video(void* config_data);

        int PN_Media_Getconfig_Video(void* config_data);
        int PN_Media_Getconfig_Audio(void* config_data);
        int PN_Media_Getconfig_Label(void* config_data);

        int PN_Media_Save_Config(int config_id);
        int PN_Media_Read_Config(int config_id);
        int PN_Media_SipCall_Listening();
        static void PN_Media_Timer(int sig);
        void PN_Media_Timer_Proccess();
        static void PN_Media_Timer1(int sig);
        void PN_Media_Setitimer();
        void PN_Media_Timer_Proccess1();
        void PN_Login_Detect_Success();
        void PN_Login_Detect_Init();
        static void PN_Media_SipCall_Thread_Listening(void* params);
        void PN_Media_SipCall_Abnormal_Status(char* from_num, char* to_num);

        int GetIpPort(void* config_data,char*peer);
        int GetGuidSsrc(void* config_data,char* magic);
        unsigned hex_digit_to_val(unsigned char c);
        void str2hex(char hex[], char * output,unsigned int num);

        pthread_t listening_thread;
        CRtpSend    *m_prtpsend;
        AudioRtpSend *m_pAudiortpsend;
        CRing       *m_pRing;
        media_server_info m_media_server_info;
        media_server_info m_audio_server_info;
        pn_media_info m_media_info;
        volatile int sipcall_status;
        volatile int login_status;
        volatile int sipcall_status_detect;
        pthread_mutex_t mutex;
        CIniFile m_ini;
        CMSQ m_msq;
        SendCallback    callback_send;
        void* m_inst;
        static CMedia * m_this;
        bool Rec_flag;   //received response
};

#endif
