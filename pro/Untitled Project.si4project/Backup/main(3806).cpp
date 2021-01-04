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
#include <pthread.h>
#include <signal.h>

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

#define BUF_SIZE 256
extern char devid_get[20];
extern char current_pwd[100];
extern char last_pwd[100];
extern pthread_t listening_thread;
extern char *final_gets;
extern int post_kmsg_flag;


int heart_beat_flag;
FILE *fd_logtext;
pthread_mutex_t lock;
int kmsg_count=0;
int kmsg_cont_local=0;

/**
void post_handler(int num)
{
	if(post_kmsg_flag == 1)
	{
		post(final_gets);
		post_kmsg_flag=0;
		memset(final_gets,0,sizeof(final_gets));
	}
	printf("receive the signal %d.\n", num);
}
**/

void* threadFunc_get_kmsg(void *)
{
	printf("this is threadFunc_get_kmsg\n");
	FILE * pf_get_kmsg;
	char buf[BUF_SIZE]={0};
    pf_get_kmsg = popen("cat /proc/kmsg", "r");
    if (!pf_get_kmsg) {
        fprintf(stderr, "Erro to popen");
    }
	//signal(SIGALRM, post_handler); 
	/***
	当超过1秒没有读到数据可以认为内核信息打印完全，那么就会出发超时处理函数
   	超时处理函数中判断是否是命令insmod==1，如果是1，那么就可以将现有的final_get
   	post发送出去。然后处理函数返回后继续卡在fgets函数处，等待下一次由内核打印数据可读
   	。如果insmod！=1说明不是由命令insmod rmmod触发的内核打印信息，另作考虑。
	**/
    while(1){ 
		//alarm(1);
		
		if(fgets(buf, BUF_SIZE, pf_get_kmsg)!= NULL) {
		//	signal(SIGALRM, post_handler); 
		//	alarm(0);
			
			printf("this is threadFunc after fgets\n");
        	fprintf(stdout, "%s", buf);//获取到kernal消息
		      //  if(post_kmsg_flag == 1)//由
		       // {
					pthread_mutex_lock(&lock);
					strcat(final_gets,buf);
					pthread_mutex_unlock(&lock); 
					if(strlen(final_gets)>10000)
						{
						post("strlen(final_gets) is to large!");
						memset(final_gets,0,sizeof(final_gets));
					}
					
			//	}
   		}
    }
    pclose(pf_get_kmsg);

}


int main(int argc, char *argv[])
{
	FILE *fp_init;
	pthread_t read_kmsg_thread;
    int err;
    CSocket m_socket;
	
	//setvbuf(stdin, NULL, _IOLBF, 0);
	//setvbuf(stdout, NULL, _IOLBF, 0);
	system("rm tcplog1.txt ");
	system("touch tcplog1.txt");
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
	
    heart_beat_flag =0;
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
	/***清除第一次cat /proc/kmsg产生的内核打印数据***/
	#if 0
	fp_init=popen("cat /proc/kmsg","r");
	if(NULL == fp_init)
	{
			perror("popen fp_init error.\n");
			return -1;
	}
	system("ps -ef | grep \"cat /proc/kmsg\"| grep -v grep | awk \'{print $1}\' | xargs kill -9");
	pclose(fp_init);
	#endif
	pthread_mutex_init(&lock, NULL);
	int ret = pthread_create(&read_kmsg_thread, NULL,threadFunc_get_kmsg, NULL);
	    if(ret != 0){
        printf("thread create fail, ret = %d\n",ret);
        return ret;
    }
	
	
	pthread_join (listening_thread, NULL);
	pthread_join (read_kmsg_thread, NULL);
	fclose(fd_logtext);
    m_socket.PN_Socket_Close();
}











