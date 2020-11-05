#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <pjsua-lib/pjsua.h>
#include "Socket.h"
#include "Sip.h"

#define THIS_FILE   "Socket.cpp"

int server_socket;  
pn_client_info m_client_info[CLIENT_MAX]; 
int client_num;   
extern pj_pool_t*   pool;
extern int reg_state;

int socket_init()
{
    struct sockaddr_in server_addr;
    int i;

    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;  
    server_addr.sin_port = htons(SERVER_PORT); 
    server_addr.sin_addr.s_addr =htonl(INADDR_ANY);

    server_socket = socket(AF_INET,SOCK_DGRAM,0);
    if( server_socket < 0)
    {
        printf("Create Server Socket Failed!");
        return -1;
    }
 
    if(bind(server_socket,(struct sockaddr *)&server_addr,sizeof(server_addr)))
    {
        printf("Server Bind Port : %d Failed!\n", SERVER_PORT); 
        return -1;
    }

    for(i=0;i<CLIENT_MAX;i++)
        memset(&m_client_info[i],0,sizeof(pn_client_info));
    client_num=0;
    return 0;
}

int socket_close()
{
    close(server_socket);
    return 0;
}

int socket_listening()
{
    pj_thread_t*    listening_thread;
    pj_status_t     status;

    if (pool==NULL) 
    {
        pjsua_perror(THIS_FILE, "Error in socket listening pool ", status);
        return -1;
    }
    
    status = pj_thread_create(pool, NULL, (pj_thread_proc*)thread_listening, 0, 0, 0, &listening_thread);   
    if (status != PJ_SUCCESS) 
        pjsua_perror(THIS_FILE, "Error in listening thread create", status);

    return status;
}

static void *thread_listening(void* arg)
{
    char recv_buf[MSG_LEN_MAX];
    char process_buf[MSG_LEN_MAX];
    pn_msg_info m_msg_info;
    const char *end="END";
    int err;
    char buf[20]="callbye";
        char* p=NULL;
        char* tmp=NULL;
    
    memset(recv_buf,0,sizeof(recv_buf));
    memset(process_buf,0,sizeof(process_buf));
    
    while(1)
    {
//baozi        char* p=NULL;
//baozi        char* tmp=NULL;

        memset(recv_buf,0,sizeof(recv_buf));
        err=recvfrom(server_socket,recv_buf,sizeof(recv_buf),0,NULL,NULL);  
       printf("sip_socket recv = %s\n",recv_buf);
	 if(err<0)
        {
            printf("server socket recv error!\n"); 
        }
        
//      printf("server socket recv:%s----------------------------------------------------\n",recv_buf);
//      printf("len=%d\n",strlen(recv_buf));
        p=recv_buf;             
       // while(1)
       // {
            tmp = strstr(p,end);
            if(tmp==NULL)
                break;

            *tmp='\0';
            strcat(process_buf,p);     
            memset(&m_msg_info,0,sizeof(pn_msg_info));
            err=msg_parse(process_buf, &m_msg_info);
            if(err==0)   
            {
                client_parse(m_msg_info);
                if(reg_state!=1)
                {
			printf("++++****reg_state ==%d****++++\n",reg_state);
                    err=sip_register();
                    socket_send(buf);
                    if(err!=0)
                    {
                        printf("sip register err----------------------------\n");
                        exit(0);
                    }
                    sleep(1);
                }
                if(strcmp(m_msg_info.msg,"heartbeat")!=0)
                    sip_send_msg(m_msg_info.method,m_msg_info.msg);
            }
            p=tmp+strlen(end);      
            memset(process_buf,0,sizeof(process_buf));
        }
        tmp = strstr(p,msg_type[0]);
        if(tmp!=NULL)
            strcpy(process_buf,tmp);
   // }
         p=NULL;
        tmp=NULL;
}

int socket_send(const char* msg)
{
    int i;
    char* buf;
    int len=strlen(msg);
    buf=(char*)malloc(len+10);
    sprintf(buf,"msg=%s;END",msg);

    for(i=0;i<client_num;i++){
        printf("client name=%s\n",m_client_info[i].client_name);
        sendto(server_socket, buf,strlen(buf),0,(struct sockaddr *)&m_client_info[i].client_addr,sizeof(m_client_info[i].client_addr));
    }
    free(buf);
    return 0;
}

static int client_parse(pn_msg_info msg_info)
{
    int i;
    for(i=0;i<client_num;i++)
    {
        if(strcmp(m_client_info[i].client_name,msg_info.client_name)==0)
            break;
    }

    if(i==client_num)
    {
        printf("new client add:%s------------------------------------------------------\n",msg_info.client_name);
        if(client_num==CLIENT_MAX)
        {
            printf("err:client is max\n");
            return -1;
        }
        else
        {
            client_num++;
            memset(&m_client_info[client_num-1],0,sizeof(pn_client_info));
            m_client_info[client_num-1].client_addr.sin_family = AF_INET;
            m_client_info[client_num-1].client_addr.sin_port = htons(msg_info.port);
            m_client_info[client_num-1].client_addr.sin_addr.s_addr =htonl(INADDR_ANY);

            strcpy(m_client_info[client_num-1].client_name,msg_info.client_name);
        }
    }
    else
    {
        m_client_info[i].client_addr.sin_port = htons(msg_info.port);
    }

    return 0;
}

static int msg_parse(char* msg, pn_msg_info*msg_info)
{
    char* p=NULL;
    char* tmp=NULL;
    char c=';';
    char buf[10];
    int i;
    int err=0;

    p = strstr(msg,msg_type[0]);
    if(p==NULL)
        return -1;

    for(i=0;i<sizeof(msg_type)/sizeof(msg_type[0]);i++)
    {
        if(strncmp(p,msg_type[i],strlen(msg_type[i]))!=0)
        {
            err=-1;
            break;
        }

        tmp=strchr(p,c);
        if(tmp==NULL)
        {
            err=-2;
            break;
        }
        else
        {
            switch(i)
            {
                case 0:
                    *tmp='\0';
                    if(strlen(p+strlen(msg_type[i]))>CLIENT_NAME_MAX-1)
                    {
                        printf("name too long\n");
                        return -3;
                    }
                    strcpy(msg_info->client_name,p+strlen(msg_type[i]));
                    break;
                case 1:
                    *tmp='\0';
                    if(strlen(p+strlen(msg_type[i]))>10-1)
                    {
                        printf("port too long\n");
                        return -3;
                    }
                    memset(buf,0,sizeof(buf));
                    strcpy(buf,p+strlen(msg_type[i]));
                    if(is_num(buf)!=0)
                    {
                        printf("port is not a num\n");
                        return -3;
                    }
                    
                    msg_info->port=atoi(buf);
                    break;
                case 2:
                    *tmp='\0';
                    if(strlen(p+strlen(msg_type[i]))>10-1)
                    {
                        printf("method too long\n");
                        return -3;
                    }
                    memset(buf,0,sizeof(buf));
                    strcpy(buf,p+strlen(msg_type[i]));
                    if(is_num(buf)!=0)
                    {
                        printf("method is not a num\n");
                        return -3;
                    }
                    
                    msg_info->method=atoi(buf);
                    break;
                case 3:
                    *tmp='\0';
                    if(strlen(p+strlen(msg_type[i]))>MSG_LEN_MAX-1)
                    {
                        printf("msg too long\n");
                        return -3;
                    }
                    strcpy(msg_info->msg,p+strlen(msg_type[i]));
                    break;
                default:
                    break;
            }
            p=tmp+1;
        }
    }
        
    return err;
}

static int is_num(const char* buf)
{
    int i;
    for(i=0;i<strlen(buf);i++)
    {
        if(buf[i]<'0'||buf[i]>'9')
            break;
    }

    if(i==strlen(buf))
        return 0;
    else
        return -1;
}
