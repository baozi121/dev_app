#ifndef COMMON_H
#define COMMON_H
#include <stdio.h>
#define CFG_FILE "/opt/baozi/Config.dat"
/*Config param ID list */
#define PN_CFGID_RTC_TIME       0x00000001

//#define   PN_CFGID_MOTO_INFO  0x00001001
//#define   PN_CFGID_MOTO_CTRL  0x00001002

#define PN_CFGID_MEDIA_VIDEO_BITRATE        0x00002001
#define PN_CFGID_MEDIA_VIDEO_RESOLUTION     0x00002001
#define PN_CFGID_MEDIA_VIDEO_FRAMERATE      0x00002003
#define PN_CFGID_MEDIA_VIDEO_RATECONTROL    0x00002004
#define PN_CFGID_MEDIA_VIDEO_QUANT          0x00002005
#define PN_CFGID_MEDIA_VIDEO_BRIGHT         0x00002006
#define PN_CFGID_MEDIA_VIDEO_CONTRAST       0x00002007
#define PN_CFGID_MEDIA_VIDEO_START          0x00002008
#define PN_CFGID_MEDIA_VIDEO_STOP           0x00002009

#define PN_CFGID_MEDIA_VIDEO_INVITE         0x00002010
#define PN_CFGID_MEDIA_VIDEO                0x00002011
#define PN_CFGID_MEDIA_AUDIO                0x00002012
#define PN_CFGID_MEDIA_LABEL                0x00002013
#define PN_CFGID_MEDIA_MOTION               0x00002014
//#define   PN_CFGID_MEDIA_MOTION_AREA      0x00002015
#define PN_CFGID_MEDIA_VIDEO_BYE            0x00002016
#define PN_CFGID_MEDIA_SIPCALL_STOP         0x00002017
#define PN_CFGID_MEDIA_SIPCALL_INVITE       0x00002018
#define PN_CFGID_MEDIA_SIPCALL_INVITED      0x00002019
#define PN_CFGID_MEDIA_SIPCALL_CANCEL       0x00002020
#define PN_CFGID_MEDIA_SIPCALL_START        0x00002021
#define PN_CFGID_MEDIA_SIPCALL_BYE          0x00002022
#define PN_CFGID_MEDIA_GROUP_BUSY_INVITE    0x00002023
#define PN_CFGID_TASK_REMIND                0x00002031
#define PN_CFGID_MEDIA_GROUP_SIPCALL_START  0x00002024
#define PN_CFGID_MEDIA_GROUP_SIPCALL_INVITE 0x00002025
#define PN_CFGID_MEDIA_BREAKDOWN_STOP       0x00002026
// #define PN_CFGID_MEDIA_GROUPPOLL_STOP       0x00002027

#define PN_CFGID_MEDIA_LOGIN_STATUS         0x00002032
#define PN_CFGID_MEDIA_DEV					0x00002033

#define PN_CFGID_MD_LOGIN_RESPONSE          0x00002100
#define PN_CFGID_CD_LOGIN_RESPONSE          0x00002101
#define PN_CFGID_LOGIN_ACK_RESPONSE         0x00002102
#define PN_CFGID_LOGOUT_RESPONSE            0x00002104
#define PN_CFGID_WORK_TASK                  0x00002105
#define PN_CFGID_MAINT_START_RESPONSE       0x00002106
#define PN_CFGID_MAINT_UPLOAD_RESPONSE      0x00002107
#define PN_CFGID_WIFI_INFO_RESPONSE         0x00002108
#define PN_CFGID_STAKE_QUERY_RESPONSE       0x00002109
#define PN_CFGID_SipCall_QUERY_RESPONSE     0x00002110
#define PN_CFGID_DEV_CALL_INFO_RESPONSE     0x00002111
#define PN_CFGID_GROUP_CALL_INFO_RESPONSE   0x00002112
#define PN_CFGID_WORK_TASK_DELETE           0x00002113
#define PN_CFGID_WORK_TASK_ACCIDENT_CAR     0x00002114
#define PN_CFGID_WORK_TASK_CHECK            0x00002115

#define PN_CFGID_FTP_USER_INFO              0x00002200
#define PN_CFGID_FTP_UPLOAD_FILE            0x00002201
#define PN_CFGID_FTP_SOFTWARE_UPDATE        0x00002202

#define PN_CFGID_SYSINFO_DEV_ALL            0x00003001
#define PN_CFGID_SYSINFO_DEV_NAME           0x00003002
#define PN_CFGID_SYSINFO_DEV_ID             0x00003003
#define PN_CFGID_SYSINFO_DEV_PWD            0x00003004
#define PN_CFGID_SYSINFO_SERVERS_ALL        0x00003005
#define PN_CFGID_SYSINFO_SERVERS_NAME       0x00003006
#define PN_CFGID_SYSINFO_SERVERS_ID         0x00003007
#define PN_CFGID_SYSINFO_SERVERS_IP         0x00003008
#define PN_CFGID_SYSINFO_SERVERS_PORT       0x00003009
#define PN_CFGID_SYSINFO_NET_ALL            0x0000300A
#define PN_CFGID_SYSINFO_NET_IP             0x0000300B
#define PN_CFGID_SYSINFO_NET_MASK           0x0000300C
#define PN_CFGID_SYSINFO_NET_GATEWAY        0x0000300D
#define PN_CFGID_SYSINFO_NET_DNS            0x0000300E
#define PN_CFGID_SYSINFO_NET_TYPE           0x0000300F
#define PN_CFGID_SYSINFO_DEV                0x00003010
#define PN_CFGID_SYSINFO_SERVERS            0x00003011
#define PN_CFGID_SYSINFO_NET                0x00003012

#define PN_CFGID_RECORD_MEMORY              0x00004001
#define PN_CFGID_RECORD_CONFIG              0x00004002
#define PN_CFGID_RECORD_GETYEARS            0x00004003
#define PN_CFGID_RECORD_GETMONTHS           0x00004004
#define PN_CFGID_RECORD_GETDAYS             0x00004005
#define PN_CFGID_RECORD_GETHOURS            0x00004006
#define PN_CFGID_RECORD_GETFILES            0x00004007
#define PN_CFGID_RECORD_UPDATEFILES         0x00004008
#define PN_CFGID_RECORD_ON                  0x00004009
#define PN_CFGID_RECORD_OFF                 0x00004010

#define PN_CFGID_RECORD_FTP                 0x00004011

//#define   PN_CFGID_ALARM_NOTE_INFO        0x00005000

//#define   PN_CFGID_ALARM_VIDEO_LOST           0x00005001
//#define   PN_CFGID_ALARM_VIDEO_ENVELOP        0x00005002
//#define   PN_CFGID_ALARM_VIDEO_RECORD         0x00005003
//#define   PN_CFGID_ALARM_VIDEO_PICTURE        0x00005004
//#define   PN_CFGID_ALARM_INPUT_SMOKE          0x00005005
//#define   PN_CFGID_ALARM_INPUT_IR             0x00005006
//#define   PN_CFGID_ALARM_INPUT_TEMPERATURE    0x00005007
//#define   PN_CFGID_ALARM_INPUT_GAS            0x00005008
//#define   PN_CFGID_ALARM_INPUT_MANU           0x00005009
//#define   PN_CFGID_ALARM_INPUT_NOISE          0x0000500A
//#define   PN_CFGID_ALARM_INPUT_FLOW           0x0000500B
//#define   PN_CFGID_ALARM_INPUT_LIQUID         0x0000500C
//#define   PN_CFGID_ALARM_INPUT_PH             0x0000500D
//#define   PN_CFGID_ALARM_INPUT_OTHER          0x0000500E
//#define   PN_CFGID_ALARM_VIRTUALDEV_SPACE     0x0000500F
//#define   PN_CFGID_ALARM_VIRTUALDEV_DISKERROR 0x00005011
//#define   PN_CFGID_ALARM_VIRTUALDEV_OHTER     0x00005012
//#define   PN_CFGID_ALARM_THRESHOLD            0x00005013

//#define   PN_CFGID_ALARM_CLEAR_DOOR           0x00005014
//#define   PN_CFGID_ALARM_CLEAR_SALER          0x00005015
//#define   PN_CFGID_ALARM_CLEAR_DRIVER         0x00005016
//#define   PN_CFGID_ALARM_CLEAR_FIRE           0x00005017
//#define   PN_CFGID_ALARM_CLEAR_ALL            0x00005018

//#define   PN_CFGID_ALARM_STATE_ON             0x00005020
//#define   PN_CFGID_ALARM_STATE_OFF            0x00005021

#define DIRLEN 100

typedef int (*SendCallback)(int method, const char *msg, void* inst);

enum ERR
{
    PN_ERR_OK=0,
    PN_ERR_FAILED,
    PN_ERR_OPEN,
    PN_ERR_MEMORY,
    PN_ERR_INVALID_MSG,
    PN_ERR_TIMEOUT,
    PN_ERR_NOTCONN,
    PN_ERR_INVALID_DATA,
};

#ifdef  DEBUG
#define debugint(expr) printf("%s.%d: %s = %d\n",strrchr (__FILE__, '/') == 0 ? __FILE__ : strrchr (__FILE__, '/') + 1,__LINE__,#expr,(expr))
#define debugstr(expr) printf("%s.%d: %s = %s\n",strrchr (__FILE__, '/') == 0 ? __FILE__ : strrchr (__FILE__, '/') + 1,__LINE__,#expr,(expr))
#define debugstring(expr) printf("%s.%d: %s\n",strrchr (__FILE__, '/') == 0 ? __FILE__ : strrchr (__FILE__, '/') + 1,__LINE__,expr)

#else
#define debugint(expr)
#define debugstr(expr)
#define debugstring(expr)
#endif

//**************************
//                      SOCKET
//**************************
enum MSG_METHOD
{
    REGISTER= 0,
    SUBSCRIBE,
    NOTIFY,
    INVITE,
    BYE,
    ACK,
    OK,
    OPTIONS,
    MYOPTIONS,
};

//**************************
//               module_type
//**************************
enum MODULE_ID
{
    MODULE_RTC =0,
    MODULE_MOTO,
    MODULE_MEDIA,
    MODULE_WORK_ORDER,
    MODULE_FTP,
    MODULE_SYSINFO,
    MODULE_RECORD,
    MODULE_ALARM,
};

//**************************
//                      RTC
//**************************
typedef struct
{
    char year[5];
    char month[3];
    char day[3];
    char week[3];
    char hour[3];
    char min[3];
    char sec[3];
    char TimeBuf[21];
}pn_rtc_time_info;

//**************************
//                      SYSINFO
//**************************

typedef struct
{
    char dev_manufacturer[30];
    char dev_model[30];
    char dev_firmware[30];
    int  dev_maxcamera;
    char dev_name[20];
    char dev_id[30];
    char dev_pwd[20];
    char dev_online[2];
    char dev_status[2];
    char dev_record[2];
    char dev_sincestartup[20];
    char dev_sipnum[15];
}pn_sysinfo_dev;

typedef struct
{
    char serv_name[20];
    char serv_id[30];
    char serv_ip[20];
    char serv_port[8];
}pn_sysinfo_servers;//ƽ̨

typedef struct
{
    char ip[20];
    char mask[20];
    char gateway[20];
    char DNS[20];
    char type[10];
}pn_sysinfo_net;

//**************************
//                      MEDIA
//**************************
typedef struct                                       
{                             
    int  count;           
    char dev_id[30];
    char name[20];        
    char phone_num[10]; 
}pn_dev_call_info;

typedef struct                                       
{                             
    int  count;           
    char group_name[20];        
    char group_num[10]; 
}pn_group_call_info;

typedef struct
{
    char resolution_value[10];
    char video_value[10];
    char audio_value[10];
    char kbps_value[10];
    char connect_value[40];
    char mode_value[10];
    char magic_value[40];
}pn_media_invite_info;

typedef struct
{
    char sipcall_code[5];
    char cancel_code[5];
    char audio_value[10];
    char kbps_value[10];
    char connect_value[40];
    char channelid[10];
    char magic_value[40];
    char from_number[20];//sip call phone number
    char to_number[20];
    char level[10];    //Group sipcall level
}pn_audio_invite_info;

typedef struct
{
    char group_number[20];
    char phone_number[20];
    char dev_id[40];
}pn_group_busy_invite_info;

typedef struct
{
    int type; // 0: h.264 ;1 : mpeg4
    int format;  // 0: cif , 1:qcif
    //int voprc;        // 1 for cbr, 0 for vbr
    //int qphdr;    // enable if vbr
    int maxbitrate;
    int bitrate;    // enable if cbr
    int framerate;
    int bright;
    int contrast;
    int saturation;
} pn_media_video_info;

typedef struct
{
    int type;   //0: g.722
    int nchannel;
    int bitrate;
    int samplerate;
    int samplebits;
} pn_media_audio_info;

typedef struct
{
    int labeltime;  // 0: on ;  1 : off
    int labelgps;   // 0: on ;  1 : off
}pn_media_label_info;    //?ļ????ӿ???

typedef struct
{
	char name[10];
	char id[20];
    char level[10];    //Group sipcall level
    char group_num[10]; //group_num
}pn_media_dev_info;

typedef struct
{
    pn_media_video_info video_info;
    pn_media_audio_info audio_info;
    pn_media_label_info label_info;
	pn_media_dev_info   dev_info; //for dev
} pn_media_info;

//**************************
//                     Record
//**************************
typedef struct
{
    int total_size;
    int used_size;
    int left_size;
}pn_record_memory_info;

typedef struct
{
    int state;   //0;on   1:off
    int file_type;//0:cif   1:d1
    int file_length;
}pn_record_config_info;

typedef enum
{
    REC_YEAR_SEARCH=0,
    REC_MONTH_SEARCH,
    REC_DAY_SEARCH,
    REC_HOUR_SEARCH,
    REC_MIN_SEARCH,
}pn_rec_search;

typedef struct
{
    //int cmd;
    char year[5];
    char month[3];
    char day[3];
    char hour[3];
}pn_record_time;

typedef struct
{
    int num;
    char list[1024];
    char type;
}pn_record_files_info;//???Һ??ķ?????Ϣ

typedef struct
{
    char md_name[20];
    char cd_name[20];
    char result[5];
    char task_id[64];
    char stake_id[64];
    char maint_seq[64];
    char wifi_ssid[100];
    char wifi_passwd[200];
}pn_work_order_info;

typedef struct
{
    char result[5];
    char server_ip[20];
    char user_name[20];
    char user_passwd[20];
    char file_name[50];
    char dm365_app[20];
}pn_ftp_info;

typedef struct
{
    char file_type[10];
    char date[20];
    char stake_id[20];
    char dev_id[30];
    char task_type[64];
    char task_id[64];
    char maint_event_seq[100];
    char remotename[100];
    char remotedir[100];
    char localfile[200];
    pn_record_time time;
}pn_file_info;

typedef struct
{
    char user_id[20];
    char dev_id[30];
    char receiver[20];
    char receive_time[20];
    char info_source[20];
    char task_type[64];
    char task_id[64];
    char task_name[64];
    char order[64];
    char location[64];
    char event_id[64];
    char event_time[20];
    char pic_path_down[500];
    char vehicle_fault_num[20];
    char vehicle_fault_type[20];
    char parking_position[40];
    char cargo[40];
    char wrecker_num[20];
    char wrecker_type[20];
    char driver[20];
    char co_driver[20];
    char road_occupancy[64];
    char description[256];
    char task_code[10]; 
}pn_work_task_info;

typedef struct
{
    char task_id[64];
    char accar_count[4];
    char acc_car[500];
}pn_work_task_acc_car;

//hint audio play type
enum PLAYFILE
{
    PN_TASK_HINT = 0, //task hint
};

#endif

