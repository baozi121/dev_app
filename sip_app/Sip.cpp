#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include "IniFile.h"
#include "Sip.h"
#include "Socket.h"

#define THIS_FILE       "Sip.c"
#define CFG_FILE        "/opt/baozi/Config.dat"

static char md5_result[33];
static sip_config_t sip_config;
static user_info_t user_info;
static pjsua_acc_id acc_id = PJSUA_INVALID_ID;
static pthread_mutex_t mutex;
static pjsip_rx_data *rx_data=NULL;
static pjsip_rx_data *rx_speech_data=NULL;
static pjsip_rx_data *rx_speech_extra_data=NULL;

pj_pool_t*  pool;
int reg_state=-1;

int sip_read_config()
{
    CIniFile m_ini;
    struct sockaddr_in server_addr;
    struct hostent *h; 
    
    memset(&sip_config,0,sizeof(sip_config_t));
    memset(&user_info,0,sizeof(user_info_t));

    if(access (CFG_FILE, F_OK))
    {                           
        strcpy(sip_config.devname,"DM365");
        strcpy(sip_config.devid,"600522000200880001");
        strcpy(sip_config.devpwd,"123");
        strcpy(sip_config.servname,"rvsup");
        strcpy(sip_config.servid,"330100000010000090");
        strcpy(sip_config.servip,"122.224.250.36");
        strcpy(sip_config.servport,"6060");
        strcpy(sip_config.servdn,"sip.punuo.net");
    } 
    else                           
    {
        m_ini.ConfigGetKey(CFG_FILE, "DEV_INFO", "name", sip_config.devname);
        m_ini.ConfigGetKey(CFG_FILE, "DEV_INFO", "id", sip_config.devid);
        m_ini.ConfigGetKey(CFG_FILE, "DEV_INFO", "pwd", sip_config.devpwd);

        m_ini.ConfigGetKey(CFG_FILE, "SERV_INFO", "name", sip_config.servname);
        m_ini.ConfigGetKey(CFG_FILE, "SERV_INFO", "id", sip_config.servid);
        m_ini.ConfigGetKey(CFG_FILE, "SERV_INFO", "ip", sip_config.servip);
        m_ini.ConfigGetKey(CFG_FILE, "SERV_INFO", "port", sip_config.servport);
        m_ini.ConfigGetKey(CFG_FILE, "SERV_INFO", "dn", sip_config.servdn);
    }

    if(strlen(sip_config.servip)<5)
    {
        if ((h=gethostbyname(sip_config.servdn)) == NULL) 
        { 
            perror("gethostbyname"); 
            exit(1); 
        } 
        strcpy(sip_config.servip,inet_ntoa(*((struct in_addr *)h->h_addr)));
        printf("server IP Address : %s\n",sip_config.servip); 
    }
    
    return 0;
}

int sip_init()
{  
    pj_status_t         status;
    pjsua_config        cfg;
    pjsua_logging_config    log_cfg;    
    pjsua_transport_config  udp_cfg;
    pjsua_transport_id      transport_id;
    

    pthread_mutex_init (&mutex, NULL); 
    
    /* Create pjsua first! */
    status = pjsua_create();
    if (status != PJ_SUCCESS)
    {
        pjsua_perror(THIS_FILE, "Error in pjsua_create()", status);
        return status;
    }
    
    /* Init pjsua */
    pjsua_config_default(&cfg);
    cfg.cb.on_pager2 = &on_message;
    cfg.cb.on_pager_status2 = &on_response;
    
    pjsua_logging_config_default(&log_cfg);
    log_cfg.console_level = 4;

    status = pjsua_init(&cfg, &log_cfg, NULL);
    if (status != PJ_SUCCESS)
    {
        pjsua_perror(THIS_FILE, "Error in pjsua_init()", status);
        pjsua_destroy();
        return status;
    }
    
     /* Add UDP transport. */ 
    pjsua_transport_config_default(&udp_cfg);
    udp_cfg.port = htons(0);

    status = pjsua_transport_create(PJSIP_TRANSPORT_UDP, &udp_cfg, &transport_id);
    if (status != PJ_SUCCESS) 
    {
        pjsua_perror(THIS_FILE, "Error in creating transport", status);
        pjsua_destroy();
        return status;
    }

    pjsua_acc_add_local(transport_id, PJ_TRUE, &acc_id);
    printf("default acc_ud is %d-----------\n ",acc_id);

    //close sound device
    status = pjsua_set_null_snd_dev();
    pjsua_set_no_snd_dev();
    if (status != PJ_SUCCESS)
    {
        pjsua_perror(THIS_FILE, "Error in set null snd dev", status);
        pjsua_destroy();
        return status;
    }

    status = pjsua_start(); 
    if (status != PJ_SUCCESS) 
    {
        pjsua_perror(THIS_FILE, "Error starting pjsua", status);
    }

    pool = pjsua_pool_create("pjsua", 512, 512);
    if (pool==NULL) 
    {
        pjsua_perror(THIS_FILE, "Error in pool create", status);
        pjsua_destroy();
        return -1;
    }
    
    return status;
}

int sip_destroy()
{
    pj_status_t status;
    status = pjsua_destroy();

    if(pool!=NULL)
        pj_pool_release(pool);
    pool = NULL;
    
    return status;
}

int sip_send_msg(int msg_type, char* msg)
{   
    pj_status_t status;
    pj_str_t message;
    pj_str_t method;
    pj_str_t dest_uri;
    pj_info_t info;
    char uri[40];
    char* speech=NULL;
    char* ptr;

    sprintf(uri,"sip:%s:%s",sip_config.servip,sip_config.servport);
    dest_uri = pj_str(uri);

    strcpy(info.devname,sip_config.devname);
    strcpy(info.devid,sip_config.devid);
    strcpy(info.servip,sip_config.servip);
    
    if(msg!=NULL){
        speech=strstr(msg,"speech");
    }
    
    if(speech!=NULL){
        strcpy(info.destname,sip_config.devname);
        strcpy(info.destid,sip_config.devid);
    }

    else if(msg_type==MYOPTIONS&&user_info.valide==1)
    {
        strcpy(info.destname,user_info.user_name);
        strcpy(info.destid,user_info.user_id);
        user_info.valide=0;
    }
    
    else
    {
        strcpy(info.destname,sip_config.servname);
        strcpy(info.destid,sip_config.servid);
    }


    if(msg==NULL)
        message=pj_str("");
    else
        message=pj_str(msg);

    switch(msg_type)
    {
        case REGISTER:
            method=pj_str("REGISTER");
            break;
        case SUBSCRIBE:
            method=pj_str("SUBSCRIBE");
            break;
        case NOTIFY:
            method=pj_str("NOTIFY");
            break;
        case INVITE:
            method=pj_str("INVITE");
            break;
        case BYE:
            method=pj_str("BYE");
            break;
        case ACK:
            method=pj_str("ACK");
            break;
        case MYOPTIONS:
        case OPTIONS:
            method=pj_str("OPTIONS");
            break;
        case OK:
            break;
        default:
            return -1;
            break;
    }
    pthread_mutex_lock(&mutex); 
    if(msg_type==OK)
    {
        ptr=strstr(msg,"<speech_response>");
        if(ptr!=NULL){
            if(rx_speech_extra_data!=NULL){
                status = pjsua_send_200(rx_speech_extra_data,&message);
                rx_speech_extra_data=NULL;
            }
            else if(rx_speech_data!=NULL){
                status = pjsua_send_200(rx_speech_data,&message);
                rx_speech_data=NULL;
            }
        }

        else if (rx_data!=NULL){
            status = pjsua_send_200(rx_data,&message);
            rx_data=NULL;
        }
    }
    else
        status = pjsua_send_msg(acc_id, &dest_uri, NULL,&method,&info, &message);
    pthread_mutex_unlock(&mutex); 

    return status;
}

int sip_register()
{
    pj_status_t status;
    status=sip_send_msg(REGISTER,NULL);

    return status;
}

static int sip_register_2(char* seed)
{
    char tmp_md5[200];  
    char *pwd;
    pj_status_t status;


    sprintf(tmp_md5,"%s%s%s",sip_config.devid, seed, Md5_Digest(sip_config.devpwd));
    pwd=Md5_Digest(tmp_md5);

    memset(tmp_md5,0,sizeof(tmp_md5));
    sprintf(tmp_md5,register_msg,pwd);
    status=sip_send_msg(REGISTER,tmp_md5);
    
    return status;
}

static void *thread_keepalive(void* arg)
{
    char keepalive_msg[]="<heartbeat_request></heartbeat_request>";
    while(1)
    {
        sleep(10);
        printf("keep alive  msg-----------------------------------------\n");
        sip_send_msg(REGISTER,keepalive_msg);
        // sip_send_msg(REGISTER,keepalive_msg);
   //     sip_send_msg(REGISTER,keepalive_msg);
        // sleep(5);
    }
}

static int sip_keepalive()
{
    pj_thread_t*    alive_active_thread;
    pj_status_t     status;

    if (pool==NULL) 
    {
        pjsua_perror(THIS_FILE, "Error in sip keepalive pool ", status);
        return -1;
    }
    
    status = pj_thread_create(pool, NULL, (pj_thread_proc*)thread_keepalive, 0, 0, 0, &alive_active_thread);    
    if (status != PJ_SUCCESS) 
        pjsua_perror(THIS_FILE, "Error in thread create", status);
    
    return status;
}

static void on_message(pjsua_call_id call_id, const pj_str_t *from,
                        const pj_str_t *to, const pj_str_t *contact,
                        const pj_str_t *mime_type, const pj_str_t *body,
            pjsip_rx_data *rdata, pjsua_acc_id acc_id)
{
    char* msg_info;
    int i=0;
    pjsip_method method;
    pjsip_msg *msg;

    if(rdata == NULL)
        return;
    rx_data=rdata;
    msg = rdata->msg_info.msg;  
    if(msg == NULL)
        return;

    // pjsip_method_init_np(&method, (pj_str_t*)&BYE_METHOD);   
    // if (pjsip_method_cmp(&msg->line.req.method, &method) == 0) 
    // {
        // sip_send_msg(OK,NULL);
        // socket_send("BYE");
        // rx_data=NULL;
        // return;
    // }
    
    msg_info=strstr(rdata->msg_info.msg_buf,"<speech>");
    if(msg_info!=NULL){
        if(rx_speech_data==NULL)
            pjsip_rx_data_clone(rdata,0,&rx_speech_data);
        else
            pjsip_rx_data_clone(rdata,0,&rx_speech_extra_data);
    }

    msg_info=strstr(rdata->msg_info.msg_buf,"<?xml version=\"1.0\"");
    if(msg_info!=NULL)
    {
        if(strstr(rdata->msg_info.msg_buf,"RecordCommand")!=NULL)
            Get_User_Info(from->ptr,from->slen);
        
	printf("++this is messege+++socket_send msg_info is %s\n",msg_info);
        socket_send(msg_info);
    }   

    while(i++<10)
    {
        if(rx_data==NULL)
            break;
        else
            usleep(50*1000);
    }
    rx_data=NULL;
    return;
}

static void on_response(pjsua_call_id call_id, const pj_str_t *to,
                                const pj_str_t *body, void *user_data,
                                pjsip_status_code status,
                                const pj_str_t *reason,
                pjsip_tx_data *tdata,
                pjsip_rx_data *rdata,
                pjsua_acc_id acc_id)
{
    char* msg;
    
    if(status==PJSIP_SC_BAD_REQUEST||status==PJSIP_SC_INTERNAL_SERVER_ERROR||status==PJSIP_SC_TSX_TIMEOUT)
    {
        pjsua_perror(THIS_FILE, "Error in on_response", status);
        return;
    }

    msg=strstr(rdata->msg_info.msg_buf,"<?xml version=\"1.0\"?>");
    if(msg!=NULL)
    {
        if(strstr(rdata->msg_info.msg_buf,"<negotiate_response>")!=NULL)
        {
            char *tmp, *tmp_end;
            tmp=strstr(rdata->msg_info.msg_buf,"<seed>");
            tmp_end=strstr(rdata->msg_info.msg_buf,"</seed>");
            if(tmp!=NULL&&tmp_end!=NULL)
            {
                *tmp_end='\0';
                sip_register_2(tmp);
                if(reg_state==-1)
                    reg_state=0;
            }
            return;
        }

        if(strstr(rdata->msg_info.msg_buf,"<login_response>")!=NULL)
        {
            pjsua_perror(THIS_FILE, "register success", status);
            if(reg_state==0)
            {
                sip_keepalive();
                reg_state=1;
            }
            // return;
        }

        // if(strstr(rdata->msg_info.msg_buf,"<speech_bye_response>")!=NULL)
        // {
            // socket_send("BYE");
            // return;
        // }
	printf("+++++++++++++++++this is response+++++++++++++++++\n");
        socket_send(msg);
    }
    return;
}

static void hex2str(const unsigned char digest[], char *output)
{
    int i;
    for (i = 0; i<16; ++i) 
    {
        pj_val_to_hex_digit(digest[i], output);
        output += 2;
    }
}

static char* Md5_Digest(char* msg)
{
    unsigned char digest[16];
    
    memset(md5_result,'\0',sizeof(md5_result));
    pj_md5_context pms; 
    
    pj_md5_init(&pms);
    pj_md5_update( &pms, (const pj_uint8_t*)msg, strlen(msg));
    pj_md5_final(&pms, digest);

    hex2str(digest, md5_result);
    return md5_result;
}

static void Get_User_Info(char* from,int len)
{
    char* tmp1=NULL;
    char*tmp2=NULL;
    memset(&user_info,0,sizeof(user_info_t));

    if(user_info.valide!=0)
        return;
    
    tmp1=strstr(from,"\"");
    tmp2=strstr(tmp1+1,"\"");
    if(tmp1==NULL||tmp2==NULL)
        return;
    memcpy(user_info.user_name,tmp1+1,tmp2-tmp1-1);
    
    tmp1=strstr(from,"sip:");
    tmp2=strstr(tmp1+1,"@");
    if(tmp1==NULL||tmp2==NULL)
        return;
    memcpy(user_info.user_id,tmp1+4,tmp2-tmp1-4);   

    user_info.valide=1;

    printf("user_name=%s\n",user_info.user_name);
    printf("user_id=%s\n",user_info.user_id);
    return;
}

