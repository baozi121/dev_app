#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "hw_lib.h"
#include "Socket.h"

extern bool audio_state;
extern int heart_beat_flag;
pthread_t listening_thread;


CSocket::CSocket()
{
    pthread_mutex_init (&mutex, NULL);
}

CSocket::~CSocket()
{
    m_pnxml.PN_Xml_Release();
}

int CSocket::PN_Socket_Init()
{
    struct sockaddr_in client_addr;
    memset(&client_addr,0,sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(client_PORT);
    client_addr.sin_addr.s_addr =htonl(INADDR_ANY);

    client_socket = socket(AF_INET,SOCK_DGRAM,0);
    if( client_socket < 0)
    {
        debugstring("Create client Socket Failed!");
        return PN_ERR_FAILED;
    }

    if(bind(client_socket,(struct sockaddr *)&client_addr,sizeof(client_addr)))
    {
        printf("client Bind Port : %d Failed!\n", port);
        return PN_ERR_FAILED;
    }

    //get port
    port=ntohs(client_addr.sin_port);
    debugint(port);

    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr =htonl(INADDR_ANY);//inet_addr("192.168.188.101");//htonl(INADDR_ANY);

    m_pnxml.SetCallback(PN_Socket_Send,(void *)this);
    m_pnxml.PN_Xml_Init();

    return PN_ERR_OK;
}

int CSocket::PN_Socket_Close ()
{
    close(client_socket);
    return PN_ERR_OK;
}

int CSocket::PN_Socket_Listening()
{
    int err;

    err = pthread_create (&listening_thread, NULL , (void*(*)(void*))PN_Socket_Thread_Listening, (void *)this );
    if ( err!= 0)
    {
        printf ( "can't create socket listening thread: %s\n" , strerror(err)) ;
        return PN_ERR_FAILED;
    }

    return PN_ERR_OK;
}

void CSocket::PN_Socket_Thread_Listening(void* params)
{
    CSocket *pSocket = (CSocket *) params;

    char recv_buf[MSG_LEN_MAX];
    char process_buf[MSG_LEN_MAX];
    char msg_info[MSG_LEN_MAX];
    char audio_buf[20] = "msg=callclean;END";
    const char *end="END";
    int err;

        char* p=NULL;
        char* tmp=NULL;
    memset(recv_buf,0,sizeof(recv_buf));
    memset(process_buf,0,sizeof(process_buf));
    debugstring("socket listening.....");

    while(1)
    {
        //bao char* p=NULL;
        //bao char* tmp=NULL;

        memset(recv_buf,0,sizeof(recv_buf));
        if(audio_state == true)
        {
            err=recvfrom(pSocket->client_socket,recv_buf,sizeof(recv_buf),0,NULL,NULL);
			heart_beat_flag=1;
        	printf("dev_app recv socket == %s\n",recv_buf);    
	if(err<0)
            {
                debugstring("client socket recv error!");
            }
        }
        else
            strcpy(recv_buf,audio_buf);
        p=recv_buf;
//        while(1)
  //      {
            tmp = strstr(p,end);
            if(tmp==NULL)
                continue;

            *tmp='\0';
            strcat(process_buf,p);
            memset(msg_info,0,sizeof(msg_info));
            err=pSocket->PN_Socket_Msg_Parse(process_buf, msg_info);
//        printf("process_buf is %s\n",process_buf);    
//	printf("msg_info is %s\n",msg_info);
	    if(err==PN_ERR_OK)
            {
               pSocket->m_pnxml.PN_Xml_Parse(msg_info);
            }
printf("pSocket->m_pnxml.PN_Xml_Parse(msg_info) is ok\n");
            p=tmp+strlen(end);
            memset(process_buf,0,sizeof(process_buf));
        }

//printf("dev_app socket listen thread end\n");
        tmp = strstr(p,msg_type);
        if(tmp!=NULL)
            strcpy(process_buf,tmp);
   // }
	tmp = NULL;
	p=NULL;
}

int CSocket::PN_Socket_Send(int method,const char* msg,void* inst)
{
    CSocket *pSocket = (CSocket *) inst;
    pthread_mutex_lock(&(pSocket->mutex));
    char* buf;
    int len=strlen(msg);
    buf=(char*)malloc(len+50);
    sprintf(buf,"name=%s;port=%d;method=%d;msg=%s;END",process_name,pSocket->port,method,msg);
	printf("PN_Socket_Send msg = %s\n",msg);   
 sendto(pSocket->client_socket, buf,strlen(buf),0,(struct sockaddr *)&pSocket->server_addr,sizeof(pSocket->server_addr));

    free(buf);
    pthread_mutex_unlock(&(pSocket->mutex));
    return PN_ERR_OK;
}

int CSocket::PN_Socket_reg()
{
    char buf[100];
    sprintf(buf,"name=%s;port=%d;method=%d;msg=heartbeat;END",process_name,port,0);
    pthread_mutex_lock(&(mutex));
    sendto(client_socket, buf,strlen(buf),0,(struct sockaddr *)&server_addr,sizeof(server_addr));
	printf("PN_Socket_reg() send buf is %s\n",buf);
    pthread_mutex_unlock(&(mutex));
    return PN_ERR_OK;
}

int CSocket::PN_Socket_Msg_Parse(char* msg, char* msg_info)
{
    char* p=NULL;
    char* tmp=NULL;
    char c=';';

    p = strstr(msg,msg_type);
    if(p==NULL)
        return PN_ERR_INVALID_MSG;

    tmp=strchr(p,c);
    if(tmp==NULL)
        return PN_ERR_INVALID_MSG;

    *tmp='\0';
    if(strlen(p+strlen(msg_type))>MSG_LEN_MAX-1)
    {
        debugstring("msg too long!");
        return PN_ERR_INVALID_MSG;
    }
    strcpy(msg_info,p+strlen(msg_type));

    return PN_ERR_OK;
}

