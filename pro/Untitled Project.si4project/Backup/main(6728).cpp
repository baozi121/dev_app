#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "comm.h"
#include "Socket.h"
#include "IniFile.h"
#include "httpget.h"
#include <sys/time.h>
#if 1
int PN_Cfg_Default()
{
    CIniFile m_ini;

    m_ini.ConfigSetKey(CFG_FILE, "DEV_INFO", "manufacturer", "");
    m_ini.ConfigSetKey(CFG_FILE, "DEV_INFO", "model", "");
    m_ini.ConfigSetKey(CFG_FILE, "DEV_INFO", "firmware", "");
    m_ini.ConfigSetKey(CFG_FILE, "DEV_INFO", "name", "dev00001");
    m_ini.ConfigSetKey(CFG_FILE, "DEV_INFO", "id", "310023003100230001");
    m_ini.ConfigSetKey(CFG_FILE, "DEV_INFO", "phone_num", "7000");
    m_ini.ConfigSetKey(CFG_FILE, "DEV_INFO", "server_num", "6864");
    m_ini.ConfigSetKey(CFG_FILE, "DEV_INFO", "pwd", "123");
    m_ini.ConfigSetKey(CFG_FILE, "DEV_INFO", "maxcamera", "");

    m_ini.ConfigSetKey(CFG_FILE, "SERV_INFO", "name", "rvsup");
    m_ini.ConfigSetKey(CFG_FILE, "SERV_INFO", "id", "330100000010000090");
    m_ini.ConfigSetKey(CFG_FILE, "SERV_INFO", "ip", "192.168.1.220");
    m_ini.ConfigSetKey(CFG_FILE, "SERV_INFO", "port", "6060");
    m_ini.ConfigSetKey(CFG_FILE, "SERV_INFO", "dn", "");

    m_ini.ConfigSetKey(CFG_FILE, "NET_INFO", "ip", "");
    m_ini.ConfigSetKey(CFG_FILE, "NET_INFO", "mask", "");
    m_ini.ConfigSetKey(CFG_FILE, "NET_INFO", "gateway", "");
    m_ini.ConfigSetKey(CFG_FILE, "NET_INFO", "DNS", "");
    m_ini.ConfigSetKey(CFG_FILE, "NET_INFO", "type", "3G");

    m_ini.ConfigSetKey(CFG_FILE, "MOTO_INFO", "protocol", "PELCO_D");
    m_ini.ConfigSetKey(CFG_FILE, "MOTO_INFO", "baudrate", "B4800");
    m_ini.ConfigSetKey(CFG_FILE, "MOTO_INFO", "address", "0x00");

    m_ini.ConfigSetKey(CFG_FILE, "VIDEO_INFO", "type", "0");
    m_ini.ConfigSetKey(CFG_FILE, "VIDEO_INFO", "format", "1");
    m_ini.ConfigSetKey(CFG_FILE, "VIDEO_INFO", "maxbitrate", "700");
    m_ini.ConfigSetKey(CFG_FILE, "VIDEO_INFO", "bitrate", "200");
    m_ini.ConfigSetKey(CFG_FILE, "VIDEO_INFO", "framerate", "10");
    m_ini.ConfigSetKey(CFG_FILE, "VIDEO_INFO", "bright", "50");
    m_ini.ConfigSetKey(CFG_FILE, "VIDEO_INFO", "contrast", "50");
    m_ini.ConfigSetKey(CFG_FILE, "VIDEO_INFO", "saturation", "50");

    m_ini.ConfigSetKey(CFG_FILE, "AUDIO_INFO", "type", "");
    m_ini.ConfigSetKey(CFG_FILE, "AUDIO_INFO", "nchannel", "");
    m_ini.ConfigSetKey(CFG_FILE, "AUDIO_INFO", "bitrate", "");
    m_ini.ConfigSetKey(CFG_FILE, "AUDIO_INFO", "samplerate", "");
    m_ini.ConfigSetKey(CFG_FILE, "AUDIO_INFO", "samplebits", "");

    m_ini.ConfigSetKey(CFG_FILE, "LABEL_INFO", "labeltime", "1");
    m_ini.ConfigSetKey(CFG_FILE, "LABEL_INFO", "labelgps", "1");

    m_ini.ConfigSetKey(CFG_FILE, "MOTION_INFO", "state", "1");
    m_ini.ConfigSetKey(CFG_FILE, "MOTION_INFO", "sensitivity", "30");
    m_ini.ConfigSetKey(CFG_FILE, "MOTION_INFO", "len", "2");

    m_ini.ConfigSetKey(CFG_FILE, "RECORD_INFO", "state", "1");
    m_ini.ConfigSetKey(CFG_FILE, "RECORD_INFO", "file_type", "0");
    m_ini.ConfigSetKey(CFG_FILE, "RECORD_INFO", "file_length", "5");

    m_ini.ConfigSetKey(CFG_FILE, "FTP_INFO", "serv_ip", "101.69.255.130");
    m_ini.ConfigSetKey(CFG_FILE, "FTP_INFO", "user_id", "punuoftp");
    m_ini.ConfigSetKey(CFG_FILE, "FTP_INFO", "user_pwd", "punuoftp");

    m_ini.ConfigSetKey(CFG_FILE, "SOFTWARE_VERSION", "dm365_app", "v1.0");

    return PN_ERR_OK;
}
#endif

extern char devid_get[20];
extern char current_pwd[100];
extern char last_pwd[100];
extern pthread_t listening_thread;

int heart_beat_flag;


FILE *fd_logtext;
int main(int argc, char *argv[])
{

    int err;
    CSocket m_socket;

    if(access(CFG_FILE, F_OK))
        PN_Cfg_Default();

    err=m_socket.PN_Socket_Init();//devid已经保存在devid——get数组中
    if(err!=0)
    {
        debugstring("socket init err");
        exit(0);
    }
	fd_logtext=fopen("/opt/baozi/log.txt","r+");
    err=m_socket.PN_Socket_Listening();
    if(err!=0)
    {
        debugstring("socket listening err\n");
        exit(0);
    }
	if(getcwd(current_pwd,sizeof(current_pwd))==NULL)
		{
		perror("get current pwd is err !");
	}else{
		printf("current pwd is %s\n",current_pwd);
		memset(last_pwd,0,sizeof(last_pwd));
	}
	
    heart_beat_flag =0;;
    while(1)//just send heartbeat
    {
    	if(heart_beat_flag ==0)
        {
        	m_socket.PN_Socket_reg();
        	sleep(5);
    	}
		else{
			break;
		}
		
    }
	pthread_join (listening_thread, NULL);
	fclose(fd_logtext);
    m_socket.PN_Socket_Close();
}











