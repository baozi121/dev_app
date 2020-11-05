#include <stdio.h>
#include "Manage.h"
#include "tinyxml.h"
#include "comm.h"

#define XMLMAXLEN 2048
class CPnxml
{
    public:
        CPnxml();
        ~CPnxml();
        int PN_Xml_Init();
        int PN_Xml_Release();
        int PN_Xml_Parse(char *msg);
        void SetCallback(SendCallback callback,void* inst);

    private:

        int XmlDoQuery(TiXmlNode* pNode_root);
        int XmlDoControl(TiXmlNode* pNode_root);
        int XmlDoConfig(TiXmlNode* pNode_root);

        int XmlDevSpeechNotifyAck(char * msg);
        int XmlGroupNumNotifyAck(char * msg);
        int XmlGroupBusyInviteAck(char * msg);
        int XmlMediaInviteAck(pn_media_invite_info *media_invite_info,char * msg);
        int XmlMediaByeAck(pn_media_invite_info *media_invite_info,char * msg);
        int XmlAudioInviteAck(pn_audio_invite_info *Audio_invite_info,char * msg);
        int XmlAudioCancelAck(pn_audio_invite_info *Audio_invite_info,char * msg);
        int XmlAudioByeAck(pn_audio_invite_info *audio_invite_info,char * msg);
        int XmlMediaVideoQueryAck(pn_media_video_info *video_info,int success,char * msg);
        int XmlMediaAudioQueryAck(pn_media_audio_info *audio_info,int success,char * msg);
        int XmlMediaLabelQueryAck(pn_media_label_info *label_info,int success,char * msg);
        int XmlSysTimeQueryAck(pn_rtc_time_info *time_info,int success,char * msg);
        int XmlSysDevQueryAck(pn_sysinfo_dev *sysinfo_dev,int success,char * msg);
        int XmlSysServerQueryAck(pn_sysinfo_servers*sysinfo_servers,int success,char * msg);
        int XmlSysNetQueryAck(pn_sysinfo_net*sysinfo_net,int success,char * msg);
        int XmlRecMemoryQueryAck(pn_record_memory_info*record_memory_info,int success,char * msg);
        int XmlRecConfigQueryAck(pn_record_config_info*record_config_info,int success,char * msg);
        int XmlRecFilesQueryAck(pn_record_files_info*record_files_info,int success,char * msg);
        int XmlConfigAck(const char*variable,int success,char * msg);
        int XmlControlAck(const char*variable,int command, int success,char* msg);
        int XmlWorkTaskAck(const char* result,char* msg);
        int XmlWorkTaskAccidentAck(const char* result,char* msg);
        int XmlWorkTaskDeleteAck(const char* result,char* msg);
        int TimeChangeFormt(const char * time ,pn_rtc_time_info *time_info);
        int SipCall_Bye_Clean_XML_Send(char* from_num,char* to_num);
		int SipCall_GroupNum_Query_XML_Send();
        int XmlGroupChangeAck();

    private:

        CManage m_Manage;
        char * m_pxmlbuffer;
        void* m_inst;
        SendCallback    callback_send;
};

