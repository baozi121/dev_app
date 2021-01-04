#include "Pnxml.h"
#include "unistd.h"
#include "../Ini/IniFile.h"
#include "comm.h"
#include <signal.h>
#include "httpget.h"
#include <string>
#include <stdio.h>
#include <string.h>
#include <iostream>  
#include <ctime>   
#include <sys/types.h>    
#include <sys/stat.h> 




#define DOWNLOAD_PATH "/opt/baozi/download"
// static int recieved=0;
extern bool GroupBusy;
extern bool BreakDown;
extern pthread_mutex_t lock;

using namespace std;
static char xml_buffer[XMLMAXLEN];
extern FILE *fd_logtext;
char *result;
char devid_get[20]={0};
char logtxt[256]={0};
char webreturn[1024]={0};
char *final_gets;
int post_kmsg_flag;

char current_pwd[100]={0};
char last_pwd[100]={0};
int ping_pid;
FILE * ping_fp;
extern int kmsg_count;
extern int kmsg_cont_local;
FILE * tcp_fp;
pthread_t tcp_result_get;
int tcp_start_flag=0;
int tcp_stop_flag=0;
FILE* tcplog_fd;
void* tcp_re_get(void *arg)
{
		char tcplogtxt[256]={0};
		char *tcp_final_gets = (char *)malloc(1024);
		memset(tcp_final_gets,0,sizeof(tcp_final_gets));
		int count=0;
		#if 0
	//	memset(tcp_final_gets,0,sizeof(tcp_final_gets));
							 while(1)
								{  
								   printf("while 11\n");
									  while(1)//读取文件结束判断
									   {
									   	printf("while 22\n");
										memset(tcplogtxt,0,sizeof(tcplogtxt));  
										if(tcplog_fd!=NULL)
										{
										printf("33\n");
										fgets(tcplogtxt,sizeof(tcplogtxt),tcplog_fd);
											}
										else{
												printf("this is tcplog_fd == NULL\n");
												printf(" 11111111before post tcp_final_gets = %s\n ",tcp_final_gets);
													post(tcp_final_gets);
													tcp_start_flag=0;
													fclose(tcplog_fd);
													pclose(tcp_fp);
													tcp_fp==NULL;
													tcplog_fd==NULL;
													return (void*)0;
										}
										printf("44\n");
										if(strlen(tcplogtxt)>0){
											int count1=0;
											count1=strlen(tcp_final_gets)+strlen(tcplogtxt);

											if(count1 < 1024)
												{
												printf("tcp_final_gets 有空 \n");
												strcat(tcp_final_gets,tcplogtxt);
												memset(tcplogtxt,0,sizeof(tcplogtxt));
												}
											else{
												printf("tcp_final_gets 满了 提前\n");
		
												printf("22222222tcp_final_gets lens ==%d  ,tcplogtxt lens ==%d\n tcp_final_gets = %s\n  tcplogtxt =%s\n",strlen(tcp_final_gets),strlen(tcplogtxt),tcp_final_gets,tcplogtxt);
												post(tcp_final_gets);
												memset(tcp_final_gets,0,sizeof(tcp_final_gets));
												strcat(tcp_final_gets,tcplogtxt);
										    }


										}
										

												if(strstr(tcplogtxt,"#QUIT#")!=NULL)//说明收到消息是结束通讯消息停止读取文件。
												{
													printf("this is QUIT\n");
													post(tcp_final_gets);
													tcp_start_flag=0;
													fclose(tcplog_fd);
													printf(" 33333333before post tcp_final_gets = %s\n ",tcp_final_gets);
													pclose(tcp_fp);
													return (void*)0;
												}

											printf("66\n");

										 if(feof(tcplog_fd)) 
										  {
											 usleep(1000000);//0.5s

											 if(fgets(tcplogtxt,sizeof(tcplogtxt),tcplog_fd)<=0) {
													if(strlen(tcp_final_gets)>0)
													{
														printf(" 44444444before post tcp_final_gets = %s\n ",tcp_final_gets);
														post(tcp_final_gets);
														memset(tcp_final_gets,0,sizeof(tcp_final_gets));
													}
													break;

											 }
											
											
										  }
									   }
									  usleep(5*1000*100);//0.5s
								}
		#endif
		
	 while(1)
		  {
	//		fflush(fp2);
	
		  if(fgets(tcplogtxt,sizeof(tcplogtxt),tcplog_fd)!=NULL){		
///			  	printf("#### ping pid ===%s####\n ",tcplogtxt);
					/*********退出操作*******/
					if(strstr(tcplogtxt,"QUIT")!=NULL)//无论主动退出还是被动退出都在这里处理
					{
						strcat(tcp_final_gets,tcplogtxt);//发送到网页的最后一个tcp通信的消息
						memset(tcplogtxt,0,sizeof(tcplogtxt));
						if(strlen(tcp_final_gets )> 0)
						{
							post(tcp_final_gets);
							memset(tcp_final_gets,0,sizeof(tcp_final_gets));
						}
						
						tcp_start_flag=0;
						fclose(tcplog_fd);
						pclose(tcp_fp);
						system("ps -ef | grep \"tcps\" | grep -v \"grep\"|awk \'{print $1}\'| xargs kill -9");
						system("ps -ef | grep \"tcpc\" | grep -v \"grep\"|awk \'{print $1}\'| xargs kill -9");
						return (void*)0;
						
					}else{
					strcat(tcp_final_gets,tcplogtxt);
					memset(tcplogtxt,0,sizeof(tcplogtxt));
					}
			
		  }else{  
///		  		printf("wait count ==%d\n",count);
				usleep(333333);//500 000
		  		count++;
		  		if(count == 3)
		  		{
		  			count=0;
					if(strlen(tcp_final_gets )> 0){
						post(tcp_final_gets);
						memset(tcp_final_gets,0,sizeof(tcp_final_gets));
					}
					
		  		}
		  	
		 }
	}
        return (void*)0;
}


void ping_sig_handler(int num)
{
	kill(ping_pid,SIGINT);
	//pclose(ping_fp);
	printf("receive the signal %d.\n", num);
}

#if 0
void executeCMD(const char *cmd, char *result)
{
    char buf_ps[1024];
    char ps[1024]={0};
    FILE *ptr;
    strcpy(ps, cmd);
    if((ptr=popen(ps, "r"))!=NULL)
    {
        while(fgets(buf_ps, 1024, ptr)!=NULL)
        {
//	       可以通过这行来获取shell命令行中的每一行的输出
//	   	   printf("%s", buf_ps);
           strcat(result, buf_ps);
           if(strlen(result)>102400)
               break;
        }
        pclose(ptr);
        ptr = NULL;
    }
    else
    {
        printf("popen %s error\n", ps);
    }
}
#endif
CPnxml::CPnxml()
{
    memset(xml_buffer, '\0', sizeof(xml_buffer));
    m_pxmlbuffer = (char*)xml_buffer;

    callback_send =  NULL;
}


CPnxml::~CPnxml()
{
}

int CPnxml::PN_Xml_Init()
{
    m_Manage.PN_Manage_Init();
    m_Manage.SetCallback(callback_send,m_inst);
	final_gets=(char *)malloc(10240);
	memset(final_gets,0,sizeof(final_gets));
	char onlin_url[100]={0};
	 //Url_http = (char *)malloc(128);
        if(strlen(devid_get)==0)
	{
	CIniFile m_ini;
	m_ini.ConfigGetKey(CFG_FILE, "DEV_INFO", "id",devid_get );
	printf("*******************devid == %s*************************\n",devid_get);
	sprintf(onlin_url,"192.168.1.220:8080/Dev/devOnline?devId=%s",devid_get);
	printf("***onlin_url == %s****\n",onlin_url);
	httpget_GET_noreturn(onlin_url);
	}
//result = (char *)malloc(102400);
//memset(result,0,102400);
    return PN_ERR_OK;
}

int CPnxml::PN_Xml_Release()
{
    m_Manage.PN_Manage_Release();
    return PN_ERR_OK;
}

int CPnxml::
PN_Xml_Parse(char *msg)
{
    int err,err1;
    TiXmlDocument XmlDoc;
    TiXmlNode* pNode_root = NULL;
    TiXmlNode* pNode_child = NULL;
    TiXmlElement *spElement = NULL;
    TiXmlElement *RootElement = NULL;
    TiXmlElement *countElement = NULL;
    TiXmlElement *devsElement = NULL;
    TiXmlElement *devElement = NULL;
    TiXmlElement *devidElement = NULL;
    TiXmlElement *nameElement = NULL;
    TiXmlElement *phonenumElement = NULL;
   const  char * szXmlVaule = NULL;
    char* tmp=NULL;
    pn_media_invite_info media_invite_info;
    pn_audio_invite_info audio_invite_info;
    pn_work_order_info work_order_info;
    // pn_work_task_info work_task_info;
    // pn_work_task_acc_car work_task_acc_car;
    // pn_group_busy_invite_info group_busy_invite_info;
    pn_ftp_info ftp_info;
    // char type_info[2];
    // char timeInfo[20];
    char temp_cnt[2];
    int count=0;
printf("------parse-----------\n");
    //set time
  if(strncmp(msg,"Time",4) == 0)
    {
    //    What we should do is decode: Time20120229T181141 -> [MMDDhhmm[[CC]YY][.ss]]
        char tmp[21];
        memset(tmp,'\0',sizeof(tmp));
        strncpy(tmp, "date ",4);
        strncat(tmp,msg+8,4);
        strncat(tmp,msg+13,4);
        strncat(tmp,msg+4,4);
        strcat(tmp,".");
        strncat(tmp,msg+17,2);
        system(tmp);
        system("hwclock -w");
        printf("\nHardware clock and system clock have changed to %s.\n", msg+4);
        return PN_ERR_OK;
//perror("+++++parse time++++++");

    }
printf("$$$$$$%s  \n",msg);
    XmlDoc.Parse(msg);
printf("+++++XmlDoc.Parse(msg);++++++\n");

    // receive sip msg,if sip_app restart,judge OnBusy
    if(GroupBusy == false)
    {
//perror("+++++// receive sip msg,if sip_app restart,judge OnBusy++++++");
        //parse the msg from sip_app:msg=callbye,then hang up
		if(strncmp(msg,"callbye",8) == 0)return err;
       /* {
            debugstring("In the callbye Xml~~~~~~~~~~~~~~~~~~`````\n\n");
            err=m_Manage.PN_Manage_Setconfig(MODULE_MEDIA,PN_CFGID_MEDIA_SIPCALL_BYE, (void*)&audio_invite_info);
            m_Manage.PN_Manage_Control(MODULE_MEDIA, PN_CFGID_MEDIA_SIPCALL_STOP,NULL);
            return err;
        }*/
        //parse the msg=callclean,then send message to sip_app
        if(strncmp(msg,"callclean",8) == 0)
        {
			//printf("++++++++++B1++++++++++++++\n");
            debugstring("In the callclean Xml~~~~~~~~~~~~~~~~~~`````\n\n");
            err=m_Manage.PN_Manage_Setconfig(MODULE_MEDIA,PN_CFGID_MEDIA_SIPCALL_BYE, (void*)&audio_invite_info);
            m_Manage.PN_Manage_Control(MODULE_MEDIA, PN_CFGID_MEDIA_SIPCALL_STOP,NULL);
            SipCall_Bye_Clean_XML_Send(audio_invite_info.from_number,audio_invite_info.to_number);

            return err;
        }

printf("+++++++++if(strncmp(msg,is over++++++++++++++++++\n");
    }

//perror("++++++++++xxxxxxxxxxxx++++++++++++++++");
    // receive the login in response
    if ((pNode_root = XmlDoc.FirstChild("login_response")) != NULL)
    {
//perror("+++++// receive the login in response++++++");
        printf("login response!!!!!!!!!!!!!!\n");
        pNode_child =pNode_root->FirstChildElement("time");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule!=NULL)
           // err = m_Manage.PN_Manage_Control(MODULE_MEDIA,PN_CFGID_MEDIA_LOGIN_STATUS,NULL);
        return err;
		//perror("+++++parse login_response++++++");
    }


/*************
2020-11-16.将所有调试信息关闭，如果要使用那么就开启 ///的注释信息


**************/


//perror("+++++++++++exec_options+++++++++++++++");
	if ((pNode_root = XmlDoc.FirstChild("syxexec_options")) != NULL)
	   {
		 char *cmd_tmp=NULL ;
		 char cmd[128];
		 int ret;
		 int ret1;
		 char *pwd_tmp;
		 char *p;
		 char *cd_symble;
		 memset(cmd,0,128);
		 kmsg_cont_local=0;
		 kmsg_count=0;
		 pNode_child = pNode_root->FirstChildElement("syxopts");
		   if (NULL == pNode_child)
		   {
			   return PN_ERR_INVALID_DATA;
		   }
		   spElement = pNode_child->ToElement();
		   szXmlVaule = spElement->GetText();

		   
		   cmd_tmp= const_cast<char*>(szXmlVaule);
		    memset(logtxt,0,sizeof(logtxt));
			memset(final_gets,0,sizeof(final_gets));
/*********普通字符串*************/			
			if(tcp_start_flag == 1)//当tcp_start_flag==1表示所有收到的数据均当作字符串处理
			{	
				char tmp_1[512]={0};
				strncpy(tmp_1,cmd_tmp,strlen(cmd_tmp));
	///			printf("tmp1 lens ==%d\n",strlen(tmp_1));
				tmp_1[strlen(tmp_1)]='\n';
	///			printf("tmp——1 ===%s----",tmp_1);
				if(strncmp(tmp_1,"QUIT",4)==0)
				{
					tcp_start_flag=0;//这是用户输入的修后一条通信的字符串
					if(fputs("QUIT\n",tcp_fp)>0)//主动退出
					{
						fflush(tcp_fp);//这个很重要一定要有,在fputs之后
		///				printf("send QUIT is ok\n");
						
					}
				}
				else{
		///			  printf("this is tcp_start_flag == 1\n");
					  if(tcp_fp!=NULL)
				 		{
				 			if(fputs(tmp_1,tcp_fp)>0)
				 			//ret1=fwrite(cmd_tmp, 1, strlen(cmd_tmp),tcp_fp);
							fflush(tcp_fp);//这个很重要一定要有
		///					printf("fwrite is ok string == %s  lens is %d  ~\n",cmd_tmp,strlen(cmd_tmp));
							post("send msg ok");
						
						}else{
							post("send msg err\n");
						}
			   }
			
			
				
			}
/*********tcp*************/
           else if(strncmp(cmd_tmp,"./tcpc",5)==0 || strncmp(cmd_tmp,"tcpc",4)==0||strncmp(cmd_tmp,"./tcps",5)==0 || strncmp(cmd_tmp,"tcps",4)==0){
		   		if(tcp_start_flag == 0) //表示还没有启动tcp进程，
				{
			   		sprintf(cmd,"%s &> /opt/baozi/tcplog1.txt ",cmd_tmp);//标准输入/错误重定向到一个文件
		//		    printf("cmd is %s\n",cmd);
		          	
					system("rm /opt/baozi/tcplog1.txt");	
					system("touch /opt/baozi/tcplog1.txt");
						tcp_fp=popen(cmd,"w");
					
						if(tcp_fp==NULL)
							printf("tcp_popen is err\n");
						else{
				  			tcp_start_flag=1;
                        	tcplog_fd=fopen("/opt/baozi/tcplog1.txt","r+");
								if(tcplog_fd==NULL)
								{
								perror("tcplog_fd is NULL");
								} 
				//			fseek( tcplog_fd, 0, SEEK_SET );
				//			fgetc(tcplog_fd);
						int ret = pthread_create(&tcp_result_get, NULL, tcp_re_get,NULL);
				
				  		pthread_detach(tcp_result_get);//线程分离
			
							}
			   }
				else{                   //此时tcp进程已经启动，如果再次输入。/tcp上报进程已经启动
					
					post("process tcp is already running\n");
				}
			}
/*********cd*************/
		   else if(strncmp(cmd_tmp,"cd ",3)==0){   
				pwd_tmp=cmd_tmp;
				cd_symble=pwd_tmp+3;
		///		printf("pwd_tmp is =%s\n",(pwd_tmp+3));
				/*********执行cd ../xx   *************/
			   // if( (strncmp(cmd_tmp,"cd ..",5)==0) || (strncmp(cmd_tmp,"cd -",4)==0) )
				if( (strncmp(cmd_tmp,"cd ..",5)==0))
				{				
				///		printf("this is cd ..\n");
						FILE * fp;
						char buf[256] = {0};
						sprintf(cmd,"%s && pwd ",cmd_tmp);
				///		printf("cmd cd..==== %s\n",cmd);
						fp = popen(cmd, "r");
						if(NULL == fp)
							{
								perror("popen error.\n");
								return -1;
							}
						while(1)
						{
							if(fgets(buf, sizeof(buf), fp)!=NULL)
							{
								//printf("i get buf sizeis %d##bufis =%s##",strlen(buf),buf);
								buf[strlen(buf)-1] = '\0';
								p=buf;
							}
							else{
							break;
							}
						}
						pclose(fp);
					
						if(chdir(p)!=0)
						{  //路径不存在
						 char postdata[]={"dir is not exit !\n"};
				///		 printf("post data ===%s\n",postdata);
						 post(postdata);
						}
						else{
						 	char pwd_post[256] = {0};				
				///		 	printf("current pwd is %s\n",p);
				/*****cd正确后更新当前和上一次所在目录位置******/
							memcpy(last_pwd ,current_pwd,strlen(current_pwd));
							memcpy(current_pwd,p,strlen(p));
							sprintf(pwd_post,"current pwd == %s\n",p);
							post(pwd_post);
						}
			    }
				/*********cd -*************/
				else if( (strncmp(cmd_tmp,"cd -",4)==0))
						{
								char tmp_pwd[256]={0};
								/***切换到上一次的位置***/
				///				printf("####cd - is %s#####\n",last_pwd);
								if(chdir(last_pwd)==0) //切换路径成功为0 不成功为1
								{
									/******当前变为上一次，上一次变为当前位置*******/
									memcpy(tmp_pwd,current_pwd,strlen(current_pwd));
									memcpy(current_pwd,last_pwd,strlen(last_pwd));
									memcpy(last_pwd,tmp_pwd,strlen(tmp_pwd));
								}

							char pwd_post[256] = {0};				/*cd正确执行后应该回复当前路径*/
				///			printf("current pwd is %s\n",current_pwd);
				///			printf("last pwd is %s\n",last_pwd);
				///			sprintf(pwd_post,"current pwd == %s\n",current_pwd);	
							post(pwd_post);
									
						}
				
				/*********else中只执行带有路径的地址         *************/
				else{    
				///	  printf("this is cd pwd\n");
						if(chdir((pwd_tmp+3))!=0)
						{  //路径不存在
						char postdata[]={"dir is not exit !\n"};
				///		printf("post data ===%s\n",postdata);
						post(postdata);
						}
						else{	/* 保存上一次的位置信息并且更新新的地址信息 */
								//memcpy(last_pwd ,current_pwd,strlen(current_pwd));
								//memset(current_pwd,0,sizeof(current_pwd));  
								strncpy(last_pwd,current_pwd,strlen(current_pwd));
				///				printf("****last_pwd is ===%s\n",last_pwd);
								if(getcwd(current_pwd,sizeof(current_pwd))==NULL)
								{
									perror("get current pwd is err !");
								}else{
									printf("current pwd is %s\n",current_pwd);
									//memset(last_pwd,0,sizeof(last_pwd));
								}
								/*cd正确执行后应该回复当前路径*/
				///			printf("dir is ok !\n");
							char pwd_post[100] = {0};				/*cd正确执行后应该回复当前路径*/
				///			printf("current pwd is %s\n",current_pwd);
				///			printf("last pwd is %s\n",last_pwd);
							sprintf(pwd_post,"current pwd == %s\n",current_pwd);	
						post(pwd_post);
						}
			    	}
			}
/*********ping*************/
			else if(strncmp(cmd_tmp,"ping ",5)==0)
			{
					char ping_pid_c[10]={0};
					int i=0;
					//FILE * fp;
					FILE * fp2;
					char buf[256] = {0};
				///	printf("ping cmd_tmp is %s\n",cmd_tmp);
					ping_fp = popen(cmd_tmp, "r");					
					if(NULL == ping_fp)
					{
							perror("popen fp1 error.\n");
							return -1;
					}
	//get ping pid
					sprintf(buf,"ps -ef|grep \"%s\"|grep -v \"grep\"|awk '{print $1}'",cmd_tmp);
					//fp2 = popen("ps -ef|grep \"ping 192.168.1.110\"|grep -v \"grep\"|awk '{print $1}'", "r");
					fp2 = popen(buf, "r");
								if(NULL == fp2)
								{
										perror("popen fp2 error.\n");
										return -1;
								}
								memset(buf,0,sizeof(buf));
									if(fgets(ping_pid_c, sizeof(ping_pid_c), fp2)!=NULL)
									{
						//					printf("#### ping pid ===%s####\n ",ping_pid_c);
											//strcat(final_gets,buf);
											ping_pid=atoi(ping_pid_c);
						//					printf("#### ping pid ===%d####\n ",ping_pid);
									}
									pclose(fp2);
	//	get ping return data
					memset(buf,0,sizeof(buf));
					signal(SIGALRM, ping_sig_handler); 
					while(1)
					{	
						alarm(2);		
						if(fgets(buf, sizeof(buf), ping_fp)!=NULL)
						{
								alarm(0);
								printf("i get buf ==%s\n ",buf);
								strcat(final_gets,buf);
								memset(buf,0,sizeof(buf));
								i++;
								if(i==6)
									{
									pclose(ping_fp);
									kill(ping_pid,SIGINT);
									break;
									}
						}
						else{
						alarm(0);
						printf("ping fgets erro\n");
						pclose(ping_fp);
						break;
						}
					}
					printf("start post ping return\n");
					post(final_gets);
			}
							
#if 1

/*********insmod*************/
			else if(strncmp(cmd_tmp,"insmod ",7)==0 || strncmp(cmd_tmp,"rmmod ",6)==0)
			{
			///	printf("##----insmod \n");
				sprintf(cmd,"%s &> /opt/baozi/log.txt ",cmd_tmp);//标准输入/错误重定向到一个文件
			///	printf("cmd is %s\n",cmd);
				system(cmd);
				fseek(fd_logtext,0,SEEK_SET);
				fflush(fd_logtext);
				memset(logtxt,0,sizeof(logtxt));
				while(1)//读取文件结束判断
				{
						   
						   fgets(logtxt,sizeof(logtxt),fd_logtext);
				///		   printf("insmod logtxt ===%s====logtxt lens ==%d\n",logtxt,strlen(logtxt));
						   //strcat(logtxt,"#");
						   pthread_mutex_lock(&lock);
						   strcat(final_gets,logtxt);
						   pthread_mutex_unlock(&lock); 
						   memset(logtxt,0,sizeof(logtxt));
						   if(feof(fd_logtext)) 
						   {
						     break;
						   }
						   
				}
					kmsg_cont_local=kmsg_count;
					if(strlen(logtxt)==0){  //第一次加载
						while(1){
							usleep(50000);
							if(kmsg_cont_local!=kmsg_count){   //说明还有数据要插入
								kmsg_cont_local=kmsg_count;
							}
							else{								//说明内核数据全部插入
									break;
							}
						}
					 post(final_gets);
						kmsg_cont_local=0;
						kmsg_count=0;
					}else{      //重复加载
					post(final_gets);
					}
	
			}


#endif
/*********vi *************/
			else if(strncmp(cmd_tmp,"vi",2)==0 || strncmp(cmd_tmp,"vim",3)==0)
			{
				post("vi or vim is not allow to use\n");
			}
/*********history *************/
			else if(strncmp(cmd_tmp,"history",7)==0 )
				post(" 操作界面右侧展示了输入命令的历史数据\n");

/*********cat||wrong cmd||*************/
			else{
					   sprintf(cmd,"%s &> /opt/baozi/log.txt ",cmd_tmp);//标准输入/错误重定向到一个文件
				///	   printf("cmd is %s\n",cmd);
					   system(cmd);
				///	   printf("!!!!!cmd recieve!!!!!!!!!!!!\n"); 
				///	   printf("this is txt from log.txt\n");
					
					   fseek(fd_logtext,0,SEEK_SET);
					   fflush(fd_logtext);
					   while(1)//读取文件结束判断
						{
						   
						   fgets(logtxt,sizeof(logtxt),fd_logtext);
						   //strcat(logtxt,"#");
						   pthread_mutex_lock(&lock);
						   strcat(final_gets,logtxt);
						   pthread_mutex_unlock(&lock); 
						   memset(logtxt,0,sizeof(logtxt));
						   if(feof(fd_logtext)) 
						   {
						     break;
						   }
						}
					   memset(webreturn,0,sizeof(webreturn));
				///	   printf("size is %d  final_gets is\n %s\n ",strlen(final_gets),final_gets);
					   //sleep(30); 
					   kmsg_cont_local=kmsg_count;
					 	while(1){
							usleep(50000);
							if(kmsg_cont_local!=kmsg_count){   //说明还有数据要插入
								kmsg_cont_local=kmsg_count;
							}
							else{								//说明内核数据全部插入
									break;
							}
						}
					 //	post(final_gets);
						kmsg_cont_local=0;
						kmsg_count=0;
					   	post(final_gets);//上传反馈数据
		   		}	   
#if 0			
		   //httpget_uploadlogtxt(webreturn,final_gets);		 
		   //memset(webreturn,0,sizeof(webreturn));
		   example to show how to use cjson
		   if(httpget_GetTime(webreturn)!=0)
		   	{
			printf("http gets fail\n");
		   }else{
			printf("web return is %s\n",webreturn);
		   }
		   //post(接口的参数devid);
		   cJSON * root = NULL;
    	   cJSON * item = NULL;
		   root = cJSON_Parse(webreturn);
		    if (!root) 
		    {
		        printf("Error before: [%s]\n",cJSON_GetErrorPtr());
		    }
		    else
		    {
		        printf("%s\n", "有格式的方式打印Json:");           
		        printf("%s\n\n", cJSON_Print(root));
		        printf("%s\n", "无格式方式打印json：");
		        printf("%s\n\n", cJSON_PrintUnformatted(root));
		        printf("%s\n", "一步一步的获取time 键值对:");
		        printf("%s\n", "获取semantic下的cjson对象:");
		        item = cJSON_GetObjectItem(root, "time");//
		        printf("time is %s\n", cJSON_Print(item));

		    }
			
		   pNode_child =pNode_child->NextSibling("to");
		   if (NULL == pNode_child)
		   {
			   return PN_ERR_INVALID_DATA;
		   }
		   spElement = pNode_child->ToElement();
		   szXmlVaule = spElement->GetText();

		   pNode_child =pNode_child->NextSibling("feed_count");
		   if (NULL == pNode_child)
		   {
			   return PN_ERR_INVALID_DATA;
		   }
		   spElement = pNode_child->ToElement();
		   szXmlVaule = spElement->GetText();
		   printf("feed means ls\n");
		   executeCMD("ls -ls", result);
			printf("%s", result);	
		
#endif
// XmlGroupChangeAck();
	}


	//下载文件
 if ((pNode_root = XmlDoc.FirstChild("syxdownload_file")) != NULL)
    {
		char *downloadfilename=NULL ;
		struct stat buf1; 
		char local_file_name[100]={0};
		char downreturn_msg[60]={0};
    	int result;
        pNode_child = pNode_root->FirstChildElement("file_name");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
		downloadfilename= const_cast<char*>(szXmlVaule);
	
		#if 1
		printf("start download\n");
		
		HttpDownloadfile(downloadfilename,devid_get,local_file_name);
		printf("local_file_name i======%s\n",local_file_name);
		result = stat (local_file_name, &buf1);   
		printf("stop download\n");
		/*修改文件权限*/
		system("chmod -R 777 /opt/baozi/download");
		printf("chmod is ok\n");
		sprintf(downreturn_msg,"down file ok size is %d bytes !\n",buf1.st_size);
		post(downreturn_msg);
		#endif
		
    }
 #if 0
	
if ((pNode_root = XmlDoc.FirstChild("feed_plan_response")) != NULL)
    {
	printf("feed plam means\n");		
    }

// play music     --cp
if ((pNode_root = XmlDoc.FirstChild("play_music_response")) != NULL)
    {

        pNode_child = pNode_root->FirstChildElement("music_url");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();//szXmlVaule is url is name 
	printf("this is  play_music_response  ---cp\n");    
}

//stop is mv
if ((pNode_root = XmlDoc.FirstChild("stop_video")) != NULL)
    {

        pNode_child = pNode_root->FirstChildElement("status");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
                printf("close rtmp is mv\n");
     }
#endif
    return PN_ERR_OK;
	printf("-----parse is ok----\n");
}

int CPnxml::XmlDoQuery(TiXmlNode* pNode_root)
{
    int err;
    TiXmlNode* pNode_child = NULL;
    TiXmlElement *spElement = NULL;
    const char * szXmlVaule = NULL;

    pn_media_info media_info;
    pn_rtc_time_info rtc_time_info;
    pn_sysinfo_dev sysinfo_dev;
    pn_sysinfo_servers sysinfo_servers;
    pn_sysinfo_net  sysinfo_net;
    pn_record_memory_info record_memory_info;
    pn_record_config_info record_config_info;
    pn_record_files_info record_files_info;
    pn_record_time record_time;
    // pn_record_ftp_info  record_ftp_info;

    memset(&media_info,'\0',sizeof(pn_media_info));
    memset(&rtc_time_info,'\0',sizeof(pn_rtc_time_info));
    memset(&sysinfo_dev,'\0',sizeof(pn_sysinfo_dev));
    memset(&sysinfo_servers,'\0',sizeof(pn_sysinfo_servers));
    memset(&sysinfo_net,'\0',sizeof(pn_sysinfo_net));
    memset(&record_memory_info,'\0',sizeof(pn_record_memory_info));
    memset(&record_config_info,'\0',sizeof(pn_record_config_info));
    memset(&record_files_info,'\0',sizeof(pn_record_files_info));
    memset(&record_time,'\0',sizeof(pn_record_time));
    // memset(&record_ftp_info,'\0',sizeof(pn_record_ftp_info));

    pNode_child = pNode_root->FirstChildElement("variable");

    if (NULL == pNode_child)
    {
        return PN_ERR_INVALID_DATA;
    }
    spElement = pNode_child->ToElement();
    szXmlVaule = spElement->GetText();

    if(szXmlVaule==NULL)
        return PN_ERR_INVALID_DATA;

    if(strncmp(szXmlVaule,"SysTime",7) == 0)
    {
        err=m_Manage.PN_Manage_Getconfig(MODULE_RTC,PN_CFGID_RTC_TIME,&rtc_time_info,NULL);
        XmlSysTimeQueryAck(&rtc_time_info,err,m_pxmlbuffer);

        if(callback_send)
        {
            callback_send(OK,m_pxmlbuffer,m_inst);
        }
    }

    if(strncmp(szXmlVaule,"MediaInfo_Video",15) == 0)
    {
        err=m_Manage.PN_Manage_Getconfig(MODULE_MEDIA,PN_CFGID_MEDIA_VIDEO,&media_info.video_info,NULL);
        XmlMediaVideoQueryAck(&media_info.video_info,err,m_pxmlbuffer);

        if(callback_send)
        {
            callback_send(OK,m_pxmlbuffer,m_inst);
        }
    }

    if(strncmp(szXmlVaule,"MediaInfo_Audio",15) == 0)
    {
        err=m_Manage.PN_Manage_Getconfig(MODULE_MEDIA,PN_CFGID_MEDIA_AUDIO,&media_info.audio_info,NULL);
        XmlMediaAudioQueryAck(&media_info.audio_info,err,m_pxmlbuffer);

        if(callback_send)
        {
            callback_send(OK,m_pxmlbuffer,m_inst);
        }
    }

    if(strncmp(szXmlVaule,"MediaInfo_Label",15) == 0)
    {
        err=m_Manage.PN_Manage_Getconfig(MODULE_MEDIA,PN_CFGID_MEDIA_LABEL,&media_info.label_info,NULL);
        XmlMediaLabelQueryAck(&media_info.label_info,err,m_pxmlbuffer);

        if(callback_send)
        {
            callback_send(OK,m_pxmlbuffer,m_inst);
        }
    }

    if(strncmp(szXmlVaule,"DeviceInfo",10) == 0)
    {
        err=m_Manage.PN_Manage_Getconfig(MODULE_SYSINFO,PN_CFGID_SYSINFO_DEV,&sysinfo_dev,NULL);
        XmlSysDevQueryAck(&sysinfo_dev,err,m_pxmlbuffer);

        if(callback_send)
        {
            callback_send(OK,m_pxmlbuffer,m_inst);

        }
    }

    if(strncmp(szXmlVaule,"ServerInfo",10) == 0)
    {

        err=m_Manage.PN_Manage_Getconfig(MODULE_SYSINFO,PN_CFGID_SYSINFO_SERVERS,&sysinfo_servers,NULL);
        XmlSysServerQueryAck(&sysinfo_servers,err,m_pxmlbuffer);

        if(callback_send)
        {
            callback_send(OK,m_pxmlbuffer,m_inst);
        }
    }

    if(strncmp(szXmlVaule,"NetInfo",7) == 0)
    {
        err=m_Manage.PN_Manage_Getconfig(MODULE_SYSINFO,PN_CFGID_SYSINFO_NET,&sysinfo_net,NULL);
        XmlSysNetQueryAck(&sysinfo_net,err,m_pxmlbuffer);

        if(callback_send)
        {
            callback_send(OK,m_pxmlbuffer,m_inst);
        }
    }

    if(strncmp(szXmlVaule,"MemoryInfo",10) == 0)
    {
        err=m_Manage.PN_Manage_Getconfig(MODULE_RECORD,PN_CFGID_RECORD_MEMORY,&record_memory_info,NULL);
        XmlRecMemoryQueryAck(&record_memory_info,err,m_pxmlbuffer);

        if(callback_send)
        {
            callback_send(OK,m_pxmlbuffer,m_inst);
        }
    }

    if(strncmp(szXmlVaule,"RecordConfig",11) == 0)
    {
        err=m_Manage.PN_Manage_Getconfig(MODULE_RECORD,PN_CFGID_RECORD_CONFIG,&record_config_info,NULL);
        XmlRecConfigQueryAck(&record_config_info,err,m_pxmlbuffer);

        if(callback_send)
        {
            callback_send(OK,m_pxmlbuffer,m_inst);
        }
    }

    if(strncmp(szXmlVaule,"RecordInfo",10) == 0)
    {
        pNode_child =pNode_child->NextSibling("command");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();

        if(szXmlVaule==NULL)
            return PN_ERR_INVALID_DATA;

        record_files_info.type=*szXmlVaule;

        switch(*szXmlVaule)
        {
            case 'y':
                err=m_Manage.PN_Manage_Getconfig(MODULE_RECORD,PN_CFGID_RECORD_GETYEARS,&record_files_info,NULL);
                break;
            case 'm':

                pNode_child =pNode_child->NextSibling("year");
                if (NULL == pNode_child)
                {
                    return PN_ERR_INVALID_DATA;
                }
                spElement = pNode_child->ToElement();
                szXmlVaule = spElement->GetText();
                if(szXmlVaule!=NULL)
                    strncpy(record_time.year,szXmlVaule,sizeof(szXmlVaule));

                err=m_Manage.PN_Manage_Getconfig(MODULE_RECORD,PN_CFGID_RECORD_GETMONTHS,&record_files_info,&record_time);
                break;
            case 'd':
                pNode_child =pNode_child->NextSibling("year");
                if (NULL == pNode_child)
                {
                    return PN_ERR_INVALID_DATA;
                }
                spElement = pNode_child->ToElement();
                szXmlVaule = spElement->GetText();
                if(szXmlVaule!=NULL)
                    strncpy(record_time.year,szXmlVaule,sizeof(szXmlVaule));

                pNode_child =pNode_child->NextSibling("month");
                if (NULL == pNode_child)
                {
                    return PN_ERR_INVALID_DATA;
                }
                spElement = pNode_child->ToElement();
                szXmlVaule = spElement->GetText();
                if(szXmlVaule!=NULL)
                    strncpy(record_time.month,szXmlVaule,sizeof(szXmlVaule));

                err=m_Manage.PN_Manage_Getconfig(MODULE_RECORD,PN_CFGID_RECORD_GETDAYS,&record_files_info,&record_time);
                break;
            case 'h':

                pNode_child =pNode_child->NextSibling("year");
                if (NULL == pNode_child)
                {
                    return PN_ERR_INVALID_DATA;
                }
                spElement = pNode_child->ToElement();
                szXmlVaule = spElement->GetText();
                if(szXmlVaule!=NULL)
                    strncpy(record_time.year,szXmlVaule,sizeof(szXmlVaule));

                pNode_child =pNode_child->NextSibling("month");
                if (NULL == pNode_child)
                {
                    return PN_ERR_INVALID_DATA;
                }
                spElement = pNode_child->ToElement();
                szXmlVaule = spElement->GetText();
                if(szXmlVaule!=NULL)
                    strncpy(record_time.month,szXmlVaule,sizeof(szXmlVaule));

                pNode_child =pNode_child->NextSibling("day");
                if (NULL == pNode_child)
                {
                    return PN_ERR_INVALID_DATA;
                }
                spElement = pNode_child->ToElement();
                szXmlVaule = spElement->GetText();
                if(szXmlVaule!=NULL)
                    strncpy(record_time.day,szXmlVaule,sizeof(szXmlVaule));

                err=m_Manage.PN_Manage_Getconfig(MODULE_RECORD,PN_CFGID_RECORD_GETHOURS,&record_files_info,&record_time);
                break;
            case 'f':

                pNode_child =pNode_child->NextSibling("year");
                if (NULL == pNode_child)
                {
                    return PN_ERR_INVALID_DATA;
                }
                spElement = pNode_child->ToElement();
                szXmlVaule = spElement->GetText();
                if(szXmlVaule!=NULL)
                    strncpy(record_time.year,szXmlVaule,sizeof(szXmlVaule));

                pNode_child =pNode_child->NextSibling("month");
                if (NULL == pNode_child)
                {
                    return PN_ERR_INVALID_DATA;
                }
                spElement = pNode_child->ToElement();
                szXmlVaule = spElement->GetText();
                if(szXmlVaule!=NULL)
                    strncpy(record_time.month,szXmlVaule,sizeof(szXmlVaule));

                pNode_child =pNode_child->NextSibling("day");
                if (NULL == pNode_child)
                {
                    return PN_ERR_INVALID_DATA;
                }
                spElement = pNode_child->ToElement();
                szXmlVaule = spElement->GetText();
                if(szXmlVaule!=NULL)
                    strncpy(record_time.day,szXmlVaule,sizeof(szXmlVaule));

                pNode_child =pNode_child->NextSibling("hour");
                if (NULL == pNode_child)
                {
                    return PN_ERR_INVALID_DATA;
                }
                spElement = pNode_child->ToElement();
                szXmlVaule = spElement->GetText();
                if(szXmlVaule!=NULL)
                    strcpy(record_time.hour,szXmlVaule);

                err=m_Manage.PN_Manage_Getconfig(MODULE_RECORD,PN_CFGID_RECORD_GETFILES,&record_files_info,&record_time);
                break;
            default:
                err=PN_ERR_INVALID_DATA;
                break;
        }

        XmlRecFilesQueryAck(&record_files_info,err,m_pxmlbuffer);
        if(callback_send)
        {
            callback_send(OK,m_pxmlbuffer,m_inst);
        }

        return err;
    }

    return PN_ERR_OK;
}

int CPnxml::XmlDoConfig(TiXmlNode* pNode_root)
{
    int err=PN_ERR_OK;
    TiXmlNode* pNode_child = NULL;
    TiXmlElement *spElement = NULL;
    const char * szXmlVaule = NULL;

    pn_media_info media_info;
    pn_rtc_time_info rtc_time_info;
    pn_sysinfo_dev sysinfo_dev;
    pn_sysinfo_servers sysinfo_servers;
    pn_sysinfo_net  sysinfo_net;
    pn_record_memory_info record_memory_info;
    pn_record_config_info record_config_info;

    memset(&media_info,'\0',sizeof(pn_media_info));
    memset(&rtc_time_info,'\0',sizeof(pn_rtc_time_info));
    memset(&sysinfo_dev,'\0',sizeof(pn_sysinfo_dev));
    memset(&sysinfo_servers,'\0',sizeof(pn_sysinfo_servers));
    memset(&sysinfo_net,'\0',sizeof(pn_sysinfo_net));
    memset(&record_memory_info,'\0',sizeof(pn_record_memory_info));
    memset(&record_config_info,'\0',sizeof(pn_record_config_info));

    pNode_child = pNode_root->FirstChildElement("variable");
    if (NULL == pNode_child)
    {
        return PN_ERR_INVALID_DATA;
    }
    spElement = pNode_child->ToElement();
    szXmlVaule = spElement->GetText();

    if(szXmlVaule==NULL)
        return PN_ERR_INVALID_DATA;

    if(strncmp(szXmlVaule,"SysTime",7) == 0)
    {
        pNode_child =pNode_child->NextSibling("time");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        //"20100706-12:10:00"

        if(szXmlVaule!=NULL)
            err=TimeChangeFormt(szXmlVaule,&rtc_time_info);

        if(err== PN_ERR_OK)
            err=m_Manage.PN_Manage_Setconfig(MODULE_RTC,0,&rtc_time_info);

        XmlConfigAck("SysTime",err,m_pxmlbuffer);
        if(callback_send)
        {
            callback_send(OK,m_pxmlbuffer,m_inst);
        }

        return err;
    }

    if(strncmp(szXmlVaule,"MediaInfo_Video",15) == 0)
    {
        // part 1  : video info
        pNode_child =pNode_child->NextSibling("video");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule==NULL)
            goto media_video_back;

        if(strcmp(szXmlVaule,"H.264") == 0)
        {
            media_info.video_info.type = 0;
        }
        else if(strcmp(szXmlVaule,"MPEG4") == 0)
        {
            media_info.video_info.type = 1;
        }
        else
        {
            debugstring("MediaInfo video error!");
            err=PN_ERR_INVALID_DATA;
            goto media_video_back;
        }

        pNode_child =pNode_child->NextSibling("resolution");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule==NULL)
            goto media_video_back;

        if(strcmp(szXmlVaule,"CIF") == 0)
        {
            media_info.video_info.format = 0;
        }
        else if(strcmp(szXmlVaule,"D1") == 0)
        {
            media_info.video_info.format= 1;
        }
        else
        {
            debugstring("MediaInfo resolution error!");
            err=PN_ERR_INVALID_DATA;
            goto media_video_back;
        }

        pNode_child =pNode_child->NextSibling("framerate");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule!=NULL)
            media_info.video_info.framerate= atoi(szXmlVaule);

        pNode_child =pNode_child->NextSibling("bitrate");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule!=NULL)
            media_info.video_info.bitrate= atoi(szXmlVaule);

        pNode_child =pNode_child->NextSibling("bright");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule!=NULL)
            media_info.video_info.bright= atoi(szXmlVaule);

        pNode_child =pNode_child->NextSibling("contrast");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule!=NULL)
            media_info.video_info.contrast= atoi(szXmlVaule);

        pNode_child =pNode_child->NextSibling("saturation");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule!=NULL)
            media_info.video_info.saturation= atoi(szXmlVaule);

        err=m_Manage.PN_Manage_Setconfig(MODULE_MEDIA,PN_CFGID_MEDIA_VIDEO,&media_info.video_info);

media_video_back:

        XmlConfigAck("MediaInfo_Video",err,m_pxmlbuffer);
        if(callback_send)
        {
            callback_send(OK,m_pxmlbuffer,m_inst);
        }

        return err;
    }

    if(strcmp(szXmlVaule,"MediaInfo_Audio") == 0)
    {
        // part 2  : audio info
        pNode_child =pNode_child->NextSibling("audio");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule==NULL)
            goto media_audio_back;

        if(strcmp(szXmlVaule,"G.722") == 0)
        {
            media_info.audio_info.type= 0;

        }
        else
        {
            debugstring("MediaInfo audio error!");
            err=PN_ERR_INVALID_DATA;
            goto media_audio_back;
        }

        pNode_child =pNode_child->NextSibling("channel");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule!=NULL)
            media_info.audio_info.nchannel= atoi(szXmlVaule);

        pNode_child =pNode_child->NextSibling("bitrate");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule!=NULL)
            media_info.audio_info.bitrate= atoi(szXmlVaule);

        pNode_child =pNode_child->NextSibling("samplerate");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule!=NULL)
            media_info.audio_info.samplerate= atoi(szXmlVaule);

        pNode_child =pNode_child->NextSibling("samplebits");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule!=NULL)
            media_info.audio_info.samplebits= atoi(szXmlVaule);

        err=m_Manage.PN_Manage_Setconfig(MODULE_MEDIA,PN_CFGID_MEDIA_AUDIO,&media_info.audio_info);

media_audio_back:
        XmlConfigAck("MediaInfo_Audio",err,m_pxmlbuffer);

        if(callback_send)
        {
            callback_send(OK,m_pxmlbuffer,m_inst);
        }
        return err;
    }

    if(strcmp(szXmlVaule,"MediaInfo_Label") == 0)
    {
        // part3  : label info
        pNode_child =pNode_child->NextSibling("labeltime");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule==NULL)
            goto media_label_back;

        if(strcmp(szXmlVaule,"on") == 0)
        {
            media_info.label_info.labeltime = 0;
        }
        else if(strcmp(szXmlVaule,"off") == 0)
        {
            media_info.label_info.labeltime = 1;
        }
        else
        {
            debugstring("MediaInfo_Label  error");
            err=PN_ERR_INVALID_DATA;
            goto media_label_back;
        }

        pNode_child =pNode_child->NextSibling("labelgps");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule==NULL)
            goto media_label_back;

        if(strcmp(szXmlVaule,"on") == 0)
        {
            media_info.label_info.labelgps= 0;
        }
        else if(strcmp(szXmlVaule,"off") == 0)
        {
            media_info.label_info.labelgps = 1;
        }
        else
        {
            debugstring("MediaInfo_Label  error!");
            err=PN_ERR_INVALID_DATA;
            goto media_label_back;
        }

        err=m_Manage.PN_Manage_Setconfig(MODULE_MEDIA,PN_CFGID_MEDIA_LABEL,&media_info.label_info);
media_label_back:
        XmlConfigAck("MediaInfo_Label",err,m_pxmlbuffer);

        if(callback_send)
        {
            callback_send(OK,m_pxmlbuffer,m_inst);
        }
        return err;
    }

    if(strcmp(szXmlVaule,"DeviceInfo") == 0)
    {
        pNode_child =pNode_child->NextSibling("manufacturer");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule!=NULL)
            strcpy(sysinfo_dev.dev_manufacturer,szXmlVaule);

        pNode_child =pNode_child->NextSibling("model");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule!=NULL)
            strcpy(sysinfo_dev.dev_model,szXmlVaule);

        pNode_child =pNode_child->NextSibling("firmware");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule!=NULL)
            strcpy(sysinfo_dev.dev_firmware,szXmlVaule);

        pNode_child =pNode_child->NextSibling("maxcamera");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule!=NULL)
            sysinfo_dev.dev_maxcamera = atoi(szXmlVaule);

        pNode_child =pNode_child->NextSibling("name");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule!=NULL)
            strcpy(sysinfo_dev.dev_name,szXmlVaule);

        pNode_child =pNode_child->NextSibling("id");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule!=NULL)
            strcpy(sysinfo_dev.dev_id,szXmlVaule);

        pNode_child =pNode_child->NextSibling("password");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule!=NULL)
            strcpy(sysinfo_dev.dev_pwd,szXmlVaule);

        err=m_Manage.PN_Manage_Setconfig(MODULE_SYSINFO,PN_CFGID_SYSINFO_DEV,&sysinfo_dev);

        XmlConfigAck("DeviceInfo",err,m_pxmlbuffer);

        if(callback_send)
        {
            callback_send(OK,m_pxmlbuffer,m_inst);
        }
        return err;
    }

    if(strcmp(szXmlVaule,"ServerInfo") == 0)
    {
        pNode_child =pNode_child->NextSibling("servername");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule!=NULL)
            strcpy(sysinfo_servers.serv_name,szXmlVaule);

        pNode_child =pNode_child->NextSibling("serverid");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule!=NULL)
            strcpy(sysinfo_servers.serv_id,szXmlVaule);

        pNode_child =pNode_child->NextSibling("serverip");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule!=NULL)
            strcpy(sysinfo_servers.serv_ip,szXmlVaule);

        pNode_child =pNode_child->NextSibling("serverport");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule!=NULL)
            strcpy(sysinfo_servers.serv_port,szXmlVaule);

        err=m_Manage.PN_Manage_Setconfig(MODULE_SYSINFO,PN_CFGID_SYSINFO_SERVERS,&sysinfo_servers);

        XmlConfigAck("ServerInfo",err,m_pxmlbuffer);

        if(callback_send)
        {
            callback_send(OK,m_pxmlbuffer,m_inst);
        }
        return err;
    }

    if(strcmp(szXmlVaule,"NetInfo") == 0)
    {
        pNode_child =pNode_child->NextSibling("ip");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule!=NULL)
            strcpy(sysinfo_net.ip,szXmlVaule);

        pNode_child =pNode_child->NextSibling("mask");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule!=NULL)
            strcpy(sysinfo_net.mask,szXmlVaule);

        pNode_child =pNode_child->NextSibling("gateway");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule!=NULL)
            strcpy(sysinfo_net.gateway,szXmlVaule);

        pNode_child =pNode_child->NextSibling("dns");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule!=NULL)
            strcpy(sysinfo_net.DNS,szXmlVaule);

        pNode_child =pNode_child->NextSibling("type");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule!=NULL)
            strcpy(sysinfo_net.type,szXmlVaule);

        err=m_Manage.PN_Manage_Setconfig(MODULE_SYSINFO,PN_CFGID_SYSINFO_NET,&sysinfo_net);

        XmlConfigAck("NetInfo",err,m_pxmlbuffer);

        if(callback_send)
        {
            callback_send(OK,m_pxmlbuffer,m_inst);
        }
        return err;
    }

    if(strcmp(szXmlVaule,"RecordConfig") == 0)
    {
        pNode_child =pNode_child->NextSibling("state");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule==NULL)
            goto record_config_back;

        if(strcmp(szXmlVaule,"on")==0)
            record_config_info.state=0;
        else if(strcmp(szXmlVaule,"off")==0)
            record_config_info.state=1;
        else
            goto record_config_back;

        pNode_child =pNode_child->NextSibling("recformat");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule==NULL)
            goto record_config_back;

        if(strcmp(szXmlVaule,"CIF")==0)
            record_config_info.file_type=0;
        else if(strcmp(szXmlVaule,"D1")==0)
            record_config_info.file_type=1;
        else
            goto record_config_back;

        pNode_child =pNode_child->NextSibling("recsize");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule!=NULL)
            record_config_info.file_length=atoi(szXmlVaule);

        err=m_Manage.PN_Manage_Setconfig(MODULE_RECORD,PN_CFGID_RECORD_CONFIG,&record_config_info);
record_config_back  :
        XmlConfigAck("RecordConfig",err,m_pxmlbuffer);
        if(callback_send)
        {
            callback_send(OK,m_pxmlbuffer,m_inst);
        }
        return err;
    }

    return PN_ERR_OK;
}

int CPnxml::XmlDoControl(TiXmlNode* pNode_root)
{
    int err;
    TiXmlNode* pNode_child = NULL;
    TiXmlElement *spElement = NULL;
    const char * szXmlVaule = NULL;

    pNode_child = pNode_root->FirstChildElement("variable");
    if (NULL == pNode_child)
    {
        return PN_ERR_INVALID_DATA;
    }
    spElement = pNode_child->ToElement();
    szXmlVaule = spElement->GetText();

    if(strcmp(szXmlVaule,"RecordCommand") == 0)
    {
        int recordcommand = -1;
        char file[1024];
        memset(file,0,sizeof(file));

        pNode_child =pNode_child->NextSibling("command");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();
        if(szXmlVaule!=NULL)
            recordcommand = atoi(szXmlVaule);

        pNode_child =pNode_child->NextSibling("filename");
        if (NULL == pNode_child)
        {
            return PN_ERR_INVALID_DATA;
        }
        spElement = pNode_child->ToElement();
        szXmlVaule = spElement->GetText();

        if(szXmlVaule!=NULL)
        {
            if(strlen(szXmlVaule)>=sizeof(file))
                memcpy(file,szXmlVaule,sizeof(file));
            else
                strcpy(file,szXmlVaule);

            if(file[strlen(file)-1]!=' ')
                file[strlen(file)]=' ';
        }

        XmlControlAck("RecordCommand",recordcommand, PN_ERR_OK,m_pxmlbuffer);

        if(callback_send)
        {
            callback_send(OK,m_pxmlbuffer,m_inst);
        }
        memset(m_pxmlbuffer,'\0',sizeof(m_pxmlbuffer));

        err=m_Manage.PN_Manage_Control(MODULE_RECORD, PN_CFGID_RECORD_UPDATEFILES, file);
        //  switch(recordcommand)
        //  {
        //      case 0:
        //          //err=m_Manage.PN_Manage_Control(MODULE_RECORD, PN_CFGID_RECORD_ON, NULL);
        //          break;
        //      case 1:
        //          err=m_Manage.PN_Manage_Control(MODULE_RECORD, PN_CFGID_RECORD_OFF, NULL);
        //          break;
        //      case 2:
        //
        //          break;
        //      case 3:
        //
        //          break;
        //      default:
        //          debugstring("recordcommand error!");
        //          err=PN_ERR_INVALID_DATA;
        //  }

        return err;
    }

    return err;
}

int CPnxml::XmlDevSpeechNotifyAck(char * msg)                                                                                                   
{                                                                                                                                               
    char buffer[XMLMAXLEN];                                                                                                                     
    memset(buffer,'\0',sizeof(buffer));                                                                                                         

    sprintf(buffer,"<?xml version=\"1.0\"?>\n"                                                                                                  
            "<dev_speechpn_notify_response>\n"                                                                                                    
            "<code>200</code>\n"                                                                                                                
            "</dev_speechpn_notify_response>"                                                                                                     
           );                                                                                                                                  

    memset(msg,'\0',XMLMAXLEN);                                                                                                                 
    memcpy(msg,buffer,strlen(buffer));                                                                                                          
    return PN_ERR_OK;                                                                                                                           
}          

#if 0
int CPnxml::XmlGroupBusyInviteAck(char * msg)                                                                                                   
{                                                                                                                                               
    char buffer[XMLMAXLEN];                                                                                                                     
    memset(buffer,'\0',sizeof(buffer));                                                                                                         

    sprintf(buffer,"<?xml version=\"1.0\"?>\n"                                                                                                  
            "<group_busy_response>\n"                                                                                                    
            "<code>200</code>\n"                                                                                                                
            "</group_busy_response>"                                                                                                     
           );                                                                                                                                  

    memset(msg,'\0',XMLMAXLEN);                                                                                                                 
    memcpy(msg,buffer,strlen(buffer));                                                                                                          
    return PN_ERR_OK;                                                                                                                           
}          

#endif
int CPnxml::XmlMediaInviteAck(pn_media_invite_info *media_invite_info,char * msg)
{
    char buffer[XMLMAXLEN];
    memset(buffer,'\0',sizeof(buffer));

    sprintf(buffer,"<?xml version=\"1.0\"?>\n"
            "<media><resolution>%s</resolution>\n"
            "<video>%s</video>\n"
            "<audio>%s</audio>\n"
            "<kbps>%s</kbps>\n"
            "<self>%s</self>\n"
            "<mode>%s</mode>\n"
            "<magic>%s</magic></media>",
            media_invite_info->resolution_value,
            media_invite_info->video_value,
            media_invite_info->audio_value,
            media_invite_info->kbps_value,
            media_invite_info->connect_value,
            media_invite_info->mode_value,
            media_invite_info->magic_value);

    memset(msg,'\0',XMLMAXLEN);
    memcpy(msg,buffer,strlen(buffer));
    return PN_ERR_OK;
}

int CPnxml::XmlMediaByeAck(pn_media_invite_info *media_invite_info,char * msg)
{
    char buffer[XMLMAXLEN];
    memset(buffer,'\0',sizeof(buffer));

    sprintf(buffer,"<?xml version=\"1.0\"?>\n"
            "<media_bye_response>\n"
            "<resolution>%s</resolution>\n"
            "<video>%s</video>\n"
            "</media_bye_response>",
            media_invite_info->resolution_value,
            media_invite_info->video_value);

    memset(msg,'\0',XMLMAXLEN);
    memcpy(msg,buffer,strlen(buffer));
    return PN_ERR_OK;
}

int CPnxml::XmlAudioInviteAck(pn_audio_invite_info *audio_invite_info,char * msg)
{
    char buffer[XMLMAXLEN];
    memset(buffer,'\0',sizeof(buffer));

    sprintf(buffer,"<?xml version=\"1.0\"?>\n"
            "<speech_response>\n"
            "<code>%s</code>\n"
            "<from_number>%s</from_number>\n"
            "<to_number>%s</to_number>\n"
            "</speech_response>",
            audio_invite_info->sipcall_code,
            audio_invite_info->from_number,
            audio_invite_info->to_number);
    memset(msg,'\0',XMLMAXLEN);
    memcpy(msg,buffer,strlen(buffer));
    return PN_ERR_OK;
}

int CPnxml::XmlAudioCancelAck(pn_audio_invite_info *audio_invite_info,char * msg)
{
    char buffer[XMLMAXLEN];
    memset(buffer,'\0',sizeof(buffer));

    sprintf(buffer,"<?xml version=\"1.0\"?>\n"
            "<speech_cancel_response>\n"
            "<code>%s</code>\n"
            "<from_number>%s</from_number>\n"
            "<to_number>%s</to_number>\n"
            "</speech_cancel_response>",
            audio_invite_info->cancel_code,
            audio_invite_info->from_number,
            audio_invite_info->to_number);
    memset(msg,'\0',XMLMAXLEN);
    memcpy(msg,buffer,strlen(buffer));
    return PN_ERR_OK;
}

int CPnxml::XmlAudioByeAck(pn_audio_invite_info *audio_invite_info,char * msg)
{
    char buffer[XMLMAXLEN];
    memset(buffer,'\0',sizeof(buffer));

    sprintf(buffer,"<?xml version=\"1.0\"?>\n"
            "<speech_bye_response>\n"
            "<code>200</code>\n"
            "<from_number>%s</from_number>\n"
            "<to_number>%s</to_number>\n"
            "</speech_bye_response>",
            audio_invite_info->from_number,
            audio_invite_info->to_number);

    memset(msg,'\0',XMLMAXLEN);
    memcpy(msg,buffer,strlen(buffer));
    return PN_ERR_OK;
}

int CPnxml::XmlWorkTaskAck(const char* result,char * msg)
{
    char buffer[XMLMAXLEN];
    memset(buffer,'\0',sizeof(buffer));

    sprintf(buffer,"<?xml version=\"1.0\"?>\n"
            "<task_response>\n"
            "<result>%s</result>\n"
            "</task_response>",
            result);

    memset(msg,'\0',XMLMAXLEN);
    memcpy(msg,buffer,strlen(buffer));
    return PN_ERR_OK;
}

int CPnxml::XmlMediaVideoQueryAck(pn_media_video_info *video_info,int success,char * msg)
{
    char buffer[XMLMAXLEN];
    memset(buffer,'\0',sizeof(buffer));

    char tmp_video[10]="";
    char tmp_resolution[10]="";

    if(video_info->type == 0)
        strcpy(tmp_video,"H.264");
    else
        strcpy(tmp_video,"MPEG4");

    if(video_info->format == 1)
        strcpy(tmp_resolution,"QCIF");
    else if(video_info->format == 0)
        strcpy(tmp_resolution,"CIF");

    sprintf(buffer,"<?xml version=\"1.0\"?>\n"
            "<query_response>\n"
            "<variable>MediaInfo_Video</variable>\n"
            "<result>%d</result>\n"
            "<video>%s</video>\n"
            "<resolution>%s</resolution>\n"
            "<framerate>%d</framerate>\n"
            "<bitrate>%d</bitrate>\n"
            "<bright>%d</bright>\n"
            "<contrast>%d</contrast>\n"
            "<saturation>%d</saturation>\n"
            "</query_response>",
            success,
            tmp_video,
            tmp_resolution,
            video_info->framerate,
            video_info->bitrate,
            video_info->bright,
            video_info->contrast,
            video_info->saturation);

    memset(msg,'\0',XMLMAXLEN);
    memcpy(msg,buffer,strlen(buffer));

    return PN_ERR_OK;
}

int CPnxml::XmlMediaAudioQueryAck(pn_media_audio_info *audio_info,int success,char * msg)
{
    char buffer[XMLMAXLEN];
    memset(buffer,'\0',sizeof(buffer));
    char tmp_audio[10]="";

    if(audio_info->type == 0)
    {
        strcpy(tmp_audio,"G.722");
    }

    sprintf(buffer,"<?xml version=\"1.0\"?>\n"
            "<query_response>\n"
            "<variable>MediaInfo_Audio</variable>\n"
            "<result>%d</result>\n"
            "<audio>%s</audio>\n"
            "<channel>%d</channel>\n"
            "<bitrate>%d</bitrate>\n"
            "<samplerate>%d</samplerate>\n"
            "<samplebits>%d</samplebits>\n"
            "</query_response>",
            success,
            tmp_audio,
            audio_info->nchannel,
            audio_info->bitrate,
            audio_info->samplerate,
            audio_info->samplebits);

    memset(msg,'\0',XMLMAXLEN);
    memcpy(msg,buffer,strlen(buffer));

    return PN_ERR_OK;
}

int CPnxml::XmlMediaLabelQueryAck(pn_media_label_info *label_info,int success,char * msg)
{
    char buffer[XMLMAXLEN];
    char tmp_time[5];
    char tmp_gps[5];
    memset(buffer,'\0',sizeof(buffer));
    memset(tmp_time,'\0',sizeof(tmp_time));
    memset(tmp_gps,'\0',sizeof(tmp_gps));

    if(label_info->labeltime==0)
        strcpy(tmp_time,"on");
    else if(label_info->labeltime==1)
        strcpy(tmp_time,"off");

    if(label_info->labelgps==0)
        strcpy(tmp_gps,"on");
    else if(label_info->labeltime==1)
        strcpy(tmp_gps,"off");

    sprintf(buffer,"<?xml version=\"1.0\"?>\n"
            "<query_response>\n"
            "<variable>MediaInfo_Label</variable>\n"
            "<result>%d</result>\n"
            "<labeltime>%s</labeltime>\n"
            "<labelgps>%s</labelgps>\n"
            "</query_response>",
            success,
            tmp_time,
            tmp_gps);

    memset(msg,'\0',XMLMAXLEN);
    memcpy(msg,buffer,strlen(buffer));

    return PN_ERR_OK;
}

int CPnxml::XmlConfigAck(const char*variable,int success,char * msg)
{
    char buffer[XMLMAXLEN];
    memset(buffer,'\0',sizeof(buffer));

    sprintf(buffer,"<?xml version=\"1.0\"?>\n"
            "<config_response>\n"
            "<variable>%s</variable>\n"
            "<result>%d</result>\n"
            "</config_response>",
            variable,success);

    memset(msg,'\0',XMLMAXLEN);
    memcpy(msg,buffer,strlen(buffer));

    return PN_ERR_OK;
}

int CPnxml::XmlControlAck(const char*variable,int command, int success,char * msg)
{
    char buffer[XMLMAXLEN];
    memset(buffer,'\0',sizeof(buffer));

    sprintf(buffer,"<?xml version=\"1.0\"?>\n"
            "<control_response>\n"
            "<variable>%s</variable>\n"
            "<command>%d</command>\n"
            "<result>%d</result>\n"
            "</control_response>",
            variable,command,success);

    memset(msg,'\0',XMLMAXLEN);
    memcpy(msg,buffer,strlen(buffer));

    return PN_ERR_OK;
}

int CPnxml::XmlSysTimeQueryAck(pn_rtc_time_info *time_info,int success,char * msg)
{
    char buffer[XMLMAXLEN];

    memset(buffer,'\0',sizeof(buffer));

    sprintf(buffer,"<?xml version=\"1.0\"?>\n"
            "<query_response>\n"
            "<variable>SysTime</variable>\n"
            "<result>%d</result>\n"
            "<time>%s</time>\n"
            "</query_response>",
            success,
            time_info->TimeBuf);

    memset(msg,'\0',XMLMAXLEN);
    memcpy(msg,buffer,strlen(buffer));

    return PN_ERR_OK;
}

int CPnxml::XmlSysDevQueryAck(pn_sysinfo_dev *sysinfo_dev,int success,char * msg)
{
    char buffer[XMLMAXLEN];
    memset(buffer,'\0',sizeof(buffer));
    sprintf(buffer,"<?xml version=\"1.0\"?>\n"
            "<query_response>\n"
            "<variable>DeviceInfo</variable>\n"
            "<result>%d</result>\n"
            "<manufacturer>%s</manufacturer>\n"
            "<model>%s</model>\n"
            "<firmware>%s</firmware>\n"
            "<maxcamera>%d</maxcamera>\n"
            "<name>%s</name>\n"
            "<id>%s</id>\n"
            "<password>%s</password>\n"
            "<online>%s</online>\n"
            "<status>%s</status>\n"
            "<record>%s</record>\n"
            "<sincestartup>%s</sincestartup>\n"
            "<sipnum>%s</sipnum>\n"
            "</query_response>",
            success,
            sysinfo_dev->dev_manufacturer,
            sysinfo_dev->dev_model,
            sysinfo_dev->dev_firmware,
            sysinfo_dev->dev_maxcamera,
            sysinfo_dev->dev_name,
            sysinfo_dev->dev_id,
            sysinfo_dev->dev_pwd,
            sysinfo_dev->dev_online,
            sysinfo_dev->dev_status,
            sysinfo_dev->dev_record,
            sysinfo_dev->dev_sincestartup,
            sysinfo_dev->dev_sipnum);

    memset(msg,'\0',XMLMAXLEN);
    memcpy(msg,buffer,strlen(buffer));
    return PN_ERR_OK;
}

int CPnxml::XmlSysServerQueryAck(pn_sysinfo_servers*sysinfo_servers,int success,char * msg)
{
    char buffer[XMLMAXLEN];
    memset(buffer,'\0',sizeof(buffer));
    sprintf(buffer,"<?xml version=\"1.0\"?>\n"
            "<query_response>\n"
            "<variable>ServerInfo</variable>\n"
            "<result>%d</result>\n"
            "<servername>%s</servername>\n"
            "<serverid>%s</serverid>\n"
            "<serverip>%s</serverip>\n"
            "<serverport>%s</serverport>\n"
            "</query_response>",
            success,
            sysinfo_servers->serv_name,
            sysinfo_servers->serv_id,
            sysinfo_servers->serv_ip,
            sysinfo_servers->serv_port);

    memset(msg,'\0',XMLMAXLEN);
    memcpy(msg,buffer,strlen(buffer));
    return PN_ERR_OK;
}

int CPnxml::XmlSysNetQueryAck(pn_sysinfo_net*sysinfo_net,int success,char * msg)
{
    char buffer[XMLMAXLEN];
    memset(buffer,'\0',sizeof(buffer));
    sprintf(buffer,"<?xml version=\"1.0\"?>\n"
            "<query_response>\n"
            "<variable>NetInfo</variable>\n"
            "<result>%d</result>\n"
            "<ip>%s</ip>\n"
            "<mask>%s</mask>\n"
            "<gateway>%s</gateway>\n"
            "<dns>%s</dns>\n"
            "<type>%s</type>\n"
            "</query_response>",
            success,
            sysinfo_net->ip,
            sysinfo_net->mask,
            sysinfo_net->gateway,
            sysinfo_net->DNS,
            sysinfo_net->type);
    memset(msg,'\0',XMLMAXLEN);
    memcpy(msg,buffer,strlen(buffer));
    return PN_ERR_OK;
}

int CPnxml::XmlRecMemoryQueryAck(pn_record_memory_info*record_memory_info,int success,char * msg)
{
    char buffer[XMLMAXLEN];
    memset(buffer,'\0',sizeof(buffer));
    sprintf(buffer,"<?xml version=\"1.0\"?>\n"
            "<query_response>\n"
            "<variable>MemoryInfo</variable>\n"
            "<result>%d</result>\n"
            "<size>%d</size>\n"
            "<usedsize>%d</usedsize>\n"
            "<leftsize>%d</leftsize>\n"
            "</query_response>",
            success,
            record_memory_info->total_size,
            record_memory_info->used_size,
            record_memory_info->left_size);

    memset(msg,'\0',XMLMAXLEN);
    memcpy(msg,buffer,strlen(buffer));
    return PN_ERR_OK;
}

int CPnxml::XmlRecConfigQueryAck(pn_record_config_info*record_config_info,int success,char * msg)
{
    char buffer[XMLMAXLEN];
    char tmp_state[5];
    char tmp_type[5];
    memset(buffer,'\0',sizeof(buffer));
    memset(tmp_state,'\0',sizeof(tmp_state));
    memset(tmp_type,'\0',sizeof(tmp_type));

    if(record_config_info->state==0)
        strcpy(tmp_state,"on");
    else if(record_config_info->state==1)
        strcpy(tmp_state,"off");

    if(record_config_info->file_type==0)
        strcpy(tmp_type,"CIF");
    else if(record_config_info->file_type==1)
        strcpy(tmp_type,"D1");

    sprintf(buffer,"<?xml version=\"1.0\"?>\n"
            "<query_response>\n"
            "<variable>RecordConfig</variable>\n"
            "<result>%d</result>\n"
            "<state>%s</state>\n"
            "<recformat>%s</recformat>\n"
            "<recsize>%d</recsize>\n"
            "</query_response>",
            success,
            tmp_state,
            tmp_type,
            record_config_info->file_length);

    memset(msg,'\0',XMLMAXLEN);
    memcpy(msg,buffer,strlen(buffer));
    return PN_ERR_OK;
}

int CPnxml::XmlRecFilesQueryAck(pn_record_files_info*record_files_info,int success,char * msg)
{
    char buffer[XMLMAXLEN];
    memset(buffer,'\0',sizeof(buffer));

    if(success==PN_ERR_OK)
        success=record_files_info->type;

    sprintf(buffer,"<?xml version=\"1.0\"?>\n"
            "<query_response>\n"
            "<variable>RecordInfo</variable>\n"
            "<result>%d</result>\n"
            "<num>%d</num>\n"
            "<list>%s</list>\n"
            "</query_response>",
            success,
            record_files_info->num,
            record_files_info->list);

    memset(msg,'\0',XMLMAXLEN);
    memcpy(msg,buffer,strlen(buffer));
    return PN_ERR_OK;
}

int CPnxml::TimeChangeFormt(const char * time ,pn_rtc_time_info *time_info)
{
    //"20100706-12:10:00"
    int err= PN_ERR_OK;

    if(strlen(time) != 17)
    {
        err = PN_ERR_INVALID_DATA;
        return err;
    }

    strncpy(time_info->year,time,4);
    strncpy(time_info->month,time+4,2);
    strncpy(time_info->day,time+6,2);
    strncpy(time_info->hour,time+9,2);
    strncpy(time_info->min,time+12,2);
    strncpy(time_info->sec,time+15,2);

    return err;
}

void CPnxml::SetCallback(SendCallback callback,void* inst)
{
    callback_send = callback;
    m_inst=inst;
}

int CPnxml::SipCall_Bye_Clean_XML_Send(char* from_num,char* to_num)
{
    char buffer[300];
    memset(buffer,'\0',sizeof(buffer));

    sprintf(buffer,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<bye_clean>\n"
            "<from_number>%s</from_number>\n"
            "<to_number>%s</to_number>\n"
            "</bye_clean>",from_num,to_num);
    if(callback_send!=NULL)
        callback_send(NOTIFY,buffer,m_inst);
    return 0;
}

int CPnxml::XmlGroupChangeAck()
{
    char buffer[300];
    memset(buffer,'\0',sizeof(buffer));

    sprintf(buffer,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<change_dev_group_response>\n"
            "<code>200</code>\n"
            "</change_dev_group_response>");
    // memset(msg,'\0',XMLMAXLEN);    
    // memcpy(msg,buffer,strlen(buffer));
    if(callback_send!=NULL)
        callback_send(OK,buffer,m_inst);
    return PN_ERR_OK;
}    
