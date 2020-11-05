#include <iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include "comm.h"
#include "ftp.h"

#define FILELENTH 23

#define ENABLE 0
#define UNABLE 1

using namespace std;

CFTP::CFTP(void)
{
    upload_thread=0;
    download_thread=0;
    ftp_status=0;
    upload_status=0;
}

CFTP::~CFTP(void)
{
    if(0!=upload_thread)
        while(0==pthread_kill(upload_thread,0))
            pthread_cancel(upload_thread);
    debugstring("ftp upload thread _released");

    if(0!=download_thread)
        while(0==pthread_kill(download_thread,0))
            pthread_cancel(download_thread);
    debugstring("ftp download thread _released");
}

int CFTP::ftp_checkresp(char expresp)
{
    int len;
    len=recv(m_sockctrl,m_resp,256,0);
    if(-1 == len)
        return -1;

    m_resp[len]=0;
    puts(m_resp);
    if(m_resp[0]!=expresp)
        return -1;

    return 0;
}

int CFTP::ftp_sendcmd()
{
    int ret = send(m_sockctrl,m_cmd,strlen(m_cmd),0);
    if(-1 == ret)
        return -1;

    return 0;
}

int CFTP::ftp_connect(const char* ip)
{
    int err;
    struct sockaddr_in addr;
    m_sockctrl = socket(AF_INET,SOCK_STREAM,0);
    if(0==m_sockctrl)
        return -1;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(21);
    addr.sin_addr.s_addr = inet_addr(ip);

    err=connect(m_sockctrl,(sockaddr*)&addr,sizeof(addr));
    if(err)
        return -1;

    err=ftp_checkresp('2');
    if(err)
        return -1;

    return 0;
}

int CFTP::ftp_login(char* user,char* pass)
{
    int err;
    sprintf(m_cmd,"USER %s\r\n",user);
    err = ftp_sendcmd();
    if(err)
        return -1;

    err = ftp_checkresp('3');
    if(err)
        return -1;
    sprintf(m_cmd,"PASS %s\r\n",pass);

    err = ftp_sendcmd();
    if(err)
        return -1;

    err = ftp_checkresp('2');
    if(err)
        return -1;

    return 0;
}

int CFTP::ftp_quit()
{
    int err;
    sprintf(m_cmd,"QUIT\r\n");
    err = ftp_sendcmd();
    if(err)
        return -1;

    err = ftp_checkresp('2');
    if(err)
        return -1;

    return 0;
}

int CFTP::ftp_cd(char* dir)
{
    int err;
    sprintf(m_cmd,"CWD %s\r\n",dir);
    err = ftp_sendcmd();
    if(err)
        return -1;

    err = ftp_checkresp('2');
    if(err)
        return -1;

    return 0;
}

int CFTP::ftp_cdup()
{
    int err;
    sprintf(m_cmd,"CDUP\r\n");
    err = ftp_sendcmd();
    if(err)
        return -1;

    err = ftp_checkresp('2');
    if(err)
        return -1;

    return 0;
}

int CFTP::ftp_pwd(char* buff)
{
    int err;
    sprintf(m_cmd,"PWD\r\n");
    err = ftp_sendcmd();
    if(err)
        return -1;

    err = ftp_checkresp('2');
    if(err)
        return -1;

    char* p=m_resp;
    while(*p)
    {
        if(*p++ == '"')
            while(*p!='"')
                *buff++=*p++;
    }
    *buff=0;
    printf("current work directory is : %s\n",buff);
    return 0;
}

int CFTP::ftp_mkdirSingle(char* dir)
{
    int err;
    sprintf(m_cmd,"MKD %s\r\n",dir);
    err = ftp_sendcmd();
    if(err)
        return -1;

    err = ftp_checkresp('2');
    if(err)
        return -1;

    return 0;
}

int CFTP::ftp_mkdir(char* dir)
{
    int err;
    char path[300];
    unsigned int i,j;
    for(i=0,j=0;i<strlen(dir);i++)
    {
        path[j++] = dir[i];
        if(dir[i]=='/')
        {
            path[j++]='\0';
            err=ftp_cd(path);
            if(err){
                printf("create :%s\n",path);
                err=ftp_mkdirSingle(path);
                err=ftp_cd(path);
                if(err)
                    return -1;
            }
            j=0;
        }
    }
    path[j++]='\0';
    err=ftp_cd(path);
    if(err){
        err=ftp_mkdirSingle(path);
        err=ftp_cd(path);
        if(err)
            return -1;
    }
    return 0;
}

int CFTP::ftp_rmdir(char* dir)
{
    int err;
    sprintf(m_cmd,"RMD %s\r\n",dir);
    err=ftp_sendcmd();
    if(err)
        return -1;

    err = ftp_checkresp('2');
    if(err)
        return -1;

    return 0;
}

int CFTP::ftp_upload(char* localfile,char* remotepath,char* remotefilename)
{
    int err;
    ftp_mkdir(remotepath);
    ftp_setpasv();
    sprintf(m_cmd,"STOR %s\r\n",remotefilename);

    err = ftp_sendcmd();
    if(err)
        return PN_ERR_FAILED;

    err = ftp_checkresp('1');
    if(err)
        return PN_ERR_FAILED;

    FILE* pf = fopen(localfile,"r");
    if(NULL==pf)
        return PN_ERR_FAILED;

    char sendbuf[256];
    size_t len = 0;
    while((len = fread(sendbuf,1,255,pf))>0)
    {
        err=send(m_sockdata,sendbuf,len,0);
        if(err<0)
            return PN_ERR_FAILED;
    }
    close(m_sockdata);
    fclose(pf);

    err = ftp_checkresp('2');
    if(err)
        return PN_ERR_FAILED;

    return PN_ERR_OK;
}

int CFTP::ftp_setpasv()
{
    int err;
    sprintf(m_cmd,"PASV\r\n");
    err = ftp_sendcmd();
    if(err)
        return -1;

    err = ftp_checkresp('2');
    if(err)
        return -1;

    m_sockdata = socket(AF_INET,SOCK_STREAM,0);
    unsigned int v[6];
    union {
        struct sockaddr sa;
        struct sockaddr_in in;
    } sin;

    sscanf(m_resp,"%*[^(](%u,%u,%u,%u,%u,%u",&v[2],&v[3],&v[4],&v[5],&v[0],&v[1]);
    sin.sa.sa_family = AF_INET;
    sin.sa.sa_data[2] = v[2];
    sin.sa.sa_data[3] = v[3];
    sin.sa.sa_data[4] = v[4];
    sin.sa.sa_data[5] = v[5];
    sin.sa.sa_data[0] = v[0];
    sin.sa.sa_data[1] = v[1];

    int on =1;
    if (setsockopt(m_sockdata,SOL_SOCKET,SO_REUSEADDR,(const char*) &on,sizeof(on)) == -1)
    {
        perror("setsockopt");
        close(m_sockdata);
        return -1;
    }
    struct linger lng = { 0, 0 };

    if (setsockopt(m_sockdata,SOL_SOCKET,SO_LINGER,(const char*) &lng,sizeof(lng)) == -1)
    {
        perror("setsockopt");
        close(m_sockdata);
        return -1;
    }

    err = connect(m_sockdata,(sockaddr*)&sin,sizeof(sin));
    if(err)
        return -1;

    return 0;
}

int CFTP::ftp_download(char* localfile,char* remotefile)
{
    int err;
    ftp_setpasv();
    sprintf(m_cmd,"RETR %s\r\n",remotefile);
    err = ftp_sendcmd();
    if(err)
        return -1;

    err = ftp_checkresp('1');
    if(err)
        return -1;

    FILE* pf = fopen(localfile,"w");
    if(NULL==pf)
        return -1;

    char recvbuf[256];
    int len = 0;
    while((len = recv(m_sockdata,recvbuf,256,0))>0)
    {
        err = fwrite(recvbuf,len,1,pf);
        if(len<0)
            return -1;
    }
    close(m_sockdata);
    fclose(pf);
    err = ftp_checkresp('2');
    if(err)
        return -1;

    return 0;
}

int CFTP::PN_Ftp_Init()
{
    m_msq.Msq_Create();
    PN_Ftp_Software_Version_Read_Config();
    PN_Ftp_Listening();
    return PN_ERR_OK;
}

int CFTP::PN_Ftp_Release()
{
    return PN_ERR_OK;
}

int CFTP::PN_Ftp_Listening()
{
    int err;

    err = pthread_create (&upload_thread, NULL , (void*(*)(void*))PN_Ftp_Thread_Upload_Listening, (void *)this );
    if ( err!= 0)
    {
        printf ( "can't create Ftp upload thread: %s\n" , strerror(err)) ;
        return PN_ERR_FAILED;
    }

    err = pthread_create (&download_thread, NULL , (void*(*)(void*))PN_Ftp_Thread_Download_Listening, (void *)this ); if ( err!= 0)
    {
        printf ( "can't create Ftp download thread: %s\n" , strerror(err)) ;
        return PN_ERR_FAILED;
    }
    return PN_ERR_OK;
}

int CFTP::PN_Ftp_Software_Version_Read_Config()
{
    m_ini.ConfigGetKey(CFG_FILE, "SOFTWARE_VERSION", "dm365_app", oldversion);
    m_ini.ConfigGetKey(CFG_FILE, "SOFTWARE_VERSION", "sw_type", sw_type);
    m_ini.ConfigGetKey(CFG_FILE, "FTP_INFO", "serv_ip", server_ip);
    m_ini.ConfigGetKey(CFG_FILE, "FTP_INFO", "user_id", user_name);
    m_ini.ConfigGetKey(CFG_FILE, "FTP_INFO", "user_pwd", user_passwd);
    m_ini.ConfigGetKey(CFG_FILE, "DEV_INFO", "id", dev_id);
    return PN_ERR_OK;
}

void CFTP::PN_Ftp_Thread_Upload_Listening(void* params)
{
    CFTP *pFtp = (CFTP *) params;
    pFtp->PN_Ftp_Thread_Upload();
}

void CFTP::PN_Ftp_Thread_Upload()
{
    int err;
    msg_task msg_recv;
    msg_task msg_send;
    pn_file_info file_info;
    memset(&msg_recv,0,sizeof(msg_task));
    memset(&msg_send,0,sizeof(msg_task));

    msg_send.msg_type = DEVWORKORDER;
    msg_send.m_type = DEVUPLOADRESP;

    while(1){
        debugstring("wait for the ftp upload");
        err=m_msq.Msq_Read_Task(&msg_recv,QTFTPUP);
        switch(msg_recv.m_type){
            case FILEBACKUP:
                debugstr(msg_recv.text);
                //since QT may change the FTP server,it need to read Config.dat before backup
                PN_Ftp_Software_Version_Read_Config();
                //connet to the server
                err = ftp_connect(server_ip);
                if(err){
                    debugstring("connect ftp server failed");
                    strcpy(msg_send.text,"1");
                    m_msq.Msq_Send_Task(msg_send);
                    continue;
                }
                debugstring("connect OK");

                //login in the ftp server
                err = ftp_login(user_name,user_passwd);
                if(err){
                    debugstring("login failed");
                    strcpy(msg_send.text,"2");
                    m_msq.Msq_Send_Task(msg_send);
                    continue;
                }
                debugstring("ftp login OK");

                //start to upload pic files
                err = Pn_File_Backup(msg_recv.text);
                if(err){
                    debugstring("file backup failed!");
                    strcpy(msg_send.text,"3");
                    m_msq.Msq_Send_Task(msg_send);
                    continue;
                }
                debugstring("file backup success");
                strcpy(msg_send.text,"0");
                m_msq.Msq_Send_Task(msg_send);

                err = ftp_quit();
                debugstring("ftp logout");
                break;
            case PICUPLOAD:
                debugstr(msg_recv.text);
                PN_Ftp_Query_XML_Send();

                strcpy(file_info.localfile,msg_recv.text);
                err = PN_Ftp_Filename_Parse((void*)&file_info);
                if(err)
                {
                    debugstring("picture upload info is Error!");
                    strcpy(msg_send.text,"8");
                    m_msq.Msq_Send_Task(msg_send);
                    continue;
                }

                PN_Ftp_File_Upload_XML_Send((void*)&file_info);
                sleep(1);

                if(upload_status == ENABLE)
                {
                    if(ftp_status == UNABLE)
                    {
                        PN_Ftp_Query_XML_Send();
                        sleep(1);
                    }

                    if(ftp_status==ENABLE)
                    {
                        err = ftp_connect(server_ip);
                        if(err){
                            debugstring("connect ftp server failed");
                            strcpy(msg_send.text,"5");
                            m_msq.Msq_Send_Task(msg_send);
                            ftp_status=UNABLE;
                            continue;
                        }
                        debugstring("connect OK");

                        //login in the ftp server
                        err = ftp_login(user_name,user_passwd);
                        if(err){
                            debugstring("login failed");
                            strcpy(msg_send.text,"6");
                            m_msq.Msq_Send_Task(msg_send);
                            ftp_status=UNABLE;
                            continue;
                        }
                        debugstring("ftp login OK");

                        //upload file
                        err = ftp_upload(file_info.localfile, file_info.remotedir,file_info.remotename);
                        if(err){
                            debugstring("picture upload failed!");
                            strcpy(msg_send.text,"7");
                            m_msq.Msq_Send_Task(msg_send);
                            continue;
                        }
                        debugstring("picture upload success");
                        strcpy(msg_send.text,"4");
                        m_msq.Msq_Send_Task(msg_send);

                        err = ftp_quit();
                        debugstring("ftp logout");                                                                                          
                    }
                }
                break;
            case QTUPLOAD:
                debugstring("In the way.....");
                debugstr(msg_recv.text);
                err = PN_Ftp_Filename_Parse1(msg_recv.text,(void*)&file_info);
                // err =PN_Ftp_Filename_Parse(msg_recv.text , (void*)&file_info);
                PN_Ftp_File_Upload_XML_Send((void*)&file_info);
                break;
            default:
                break;
        }
        usleep(50*1000);
    }
}

int CFTP::Pn_File_Backup(char *path)
{
    int err;
    file_dir ctx;
    char path1[500] = {0};

    // backup_file = {0,0};
    backup_file.total = 0;
    backup_file.index = 0;
    printf("start to upload dir %s",path1);
    sprintf(path1,"%s",path);
    ctx.root = (local_file_t*)calloc(1, sizeof(local_file_t));
    if(ctx.root == NULL)
        return PN_ERR_FAILED;
    ctx.root->path = path1;
    ctx.parent = ctx.root;
    debugstring("start to list dir");
    Pn_Dir_List(&ctx);
    err = Pn_Dir_Operate(&ctx);
    Pn_Dir_Free(&ctx);
    free(ctx.root);
    return err;
}

void CFTP::PN_Ftp_Thread_Download_Listening(void* params)
{
    int err;

    CFTP *pFtp = (CFTP *) params;
    msg_task msg;
    msg_task msg_send;
    memset(&msg,0,sizeof(msg_task));
    memset(&msg_send,0,sizeof(msg_task));

    while(1){
        err=pFtp->m_msq.Msq_Read_Task(&msg,QTFTPDOWN);
        switch(msg.m_type){
            case QTSOFTWAREUPDATEDET:
                pFtp->PN_Ftp_Software_Update_XML_Send();
                sleep(2);
                pFtp->PN_Ftp_Query_XML_Send();
                break;
            case QTSOFTWAREUPDATE:
                msg_send.msg_type=DEVWORKORDER;
                msg_send.m_type=DEVUPDATERESP;

                err = pFtp->ftp_connect(pFtp->server_ip);
                if(err){
                    debugstring("connect ftp server failed");
                    strcpy(msg_send.text,"1");
                    pFtp->m_msq.Msq_Send_Task(msg_send);
                    continue;
                }
                puts("connect OK");

                err = pFtp->ftp_login(pFtp->user_name,pFtp->user_passwd);
                if(err){
                    debugstring("login failed");
                    strcpy(msg_send.text,"1");
                    pFtp->m_msq.Msq_Send_Task(msg_send);
                    continue;
                }
                puts("login OK");

                if(pFtp->PN_Ftp_Download_Software())
                {
                    strcpy(msg_send.text,"1");
                    pFtp->m_msq.Msq_Send_Task(msg_send);
                    continue;
                }

                strcpy(msg_send.text,"0");
                pFtp->m_msq.Msq_Send_Task(msg_send);
                err = pFtp->ftp_quit();
                if(err)
                    continue;
                break;
            default:
                break;
        }
        usleep(50*1000);
    }
}

int CFTP::PN_Ftp_Download_Software()
{
    int err;
    char localfile[40]={0};
    char remotefile[40]={0};
    char pn_commod[100]={0};

    sprintf(remotefile,"dev_software/%s_%s.tar.gz",sw_type,newversion);
    sprintf(localfile,"/opt/dm365/%s_%s.tar.gz",sw_type,newversion);

    debugstr(localfile);
    debugstr(remotefile);

    err=ftp_download(localfile,remotefile);
    if (err)
        return PN_ERR_FAILED;
    sprintf(pn_commod,"tar -xzvf %s -C /opt/dm365",localfile);
    system(pn_commod);
    system("/opt/dm365/update.sh");
    strcpy(oldversion,newversion);
    m_ini.ConfigSetKey(CFG_FILE, "SOFTWARE_VERSION","dm365_app",newversion);
    debugstring(newversion);
    sprintf(pn_commod,"rm %s",localfile);
    system(pn_commod);
    return PN_ERR_OK;
}

int CFTP::PN_Ftp_Setconfig(int config_id ,void* config_data)
{
    int err;

    switch(config_id)
    {
        case PN_CFGID_FTP_USER_INFO:
            err=PN_Ftp_Setconfig_User_Info(config_data);
            break;
        case PN_CFGID_FTP_UPLOAD_FILE:
            err=PN_Ftp_Setconfig_Upload_File(config_data);
            break;
        case PN_CFGID_FTP_SOFTWARE_UPDATE:
            err=PN_Ftp_Setconfig_Software_Update_Response(config_data);
            break;
        default:
            err=PN_ERR_INVALID_MSG;
            break;
    }

    return err;
}

int CFTP::PN_Ftp_Setconfig_User_Info(void* config_data)
{
    pn_ftp_info *pftp_info;
    pftp_info = (pn_ftp_info *)config_data;

    strcpy(server_ip,pftp_info->server_ip);
    strcpy(user_name,pftp_info->user_name);
    strcpy(user_passwd,pftp_info->user_passwd);
    ftp_status=ENABLE;

    debugstr(server_ip);
    debugstr(user_name);
    debugstr(user_passwd);
    return PN_ERR_OK;
}

int CFTP::PN_Ftp_Setconfig_Upload_File(void* config_data)
{
    pn_ftp_info *pftp_info;
    pftp_info = (pn_ftp_info *)config_data;
    msg_task msg;
    msg.msg_type = DEVWORKORDER;
    msg.m_type = DEVUPLOADRESP;
    if(strcmp(pftp_info->result,"0")!=0)
    {   
        strcpy(msg.text,"9");
        int err = m_msq.Msq_Send_Task(msg);
        upload_status = UNABLE;
        return err;
    }       
    upload_status = ENABLE;
    return PN_ERR_OK;
}

int CFTP::PN_Ftp_Setconfig_Software_Update_Response(void* config_data)
{
    int err;
    msg_task msg;
    pn_ftp_info *pftp_info;
    pftp_info = (pn_ftp_info *)config_data;

    msg.msg_type = DEVWORKORDER;
    msg.m_type = DEVUPDATEDET;

    strcpy(newversion,pftp_info->dm365_app);
    err=strcmp(newversion,oldversion);
    if(err)
        strcpy(msg.text,"0");
    else
        strcpy(msg.text,"1");

    m_msq.Msq_Send_Task(msg);

    return PN_ERR_OK;
}

int CFTP::PN_Ftp_Query_XML_Send()
{
    char buffer[300]={0};

    sprintf(buffer,"<?xml version=\"1.0\"?>\n"
            "<ftpinfo_query>\n"
            "</ftpinfo_query>");
    if(callback_send!=NULL)
        callback_send(NOTIFY,buffer,m_inst);
    return PN_ERR_OK;
}

int CFTP::PN_Ftp_File_Upload_XML_Send(void* config_data)                                                                                   
{
    char buffer[500];
    pn_file_info* file_info;
    file_info = (pn_file_info*)config_data;

    debugstring("SEND +++++++++++++++++\n");
    sprintf(buffer,"<?xml version=\"1.0\"?>\n"
            "<file_query>\n"
            "<file_name>%s</file_name>\n"
            "<file_path>%s</file_path>\n"
            "</file_query>",file_info->remotename,file_info->remotedir);
    debugstr(buffer);
    if(callback_send!=NULL)
    {
        debugstring("hehhhhhhhhhhhhhhhhhhhhhhhh\n");
        callback_send(NOTIFY,buffer,m_inst);
    }
    debugstring("SEND +++++++++++++++++\n");
    return PN_ERR_OK;
}

int CFTP::PN_Ftp_Software_Update_XML_Send()
{
    char buffer[300]={0};
    debugstr(oldversion);

    sprintf(buffer,"<?xml version=\"1.0\"?>\n"
            "<sw_update>\n"
            "<dm365_app>%s</dm365_app>\n"
            "<sw_type>%s</sw_type>\n"
            "</sw_update>",oldversion,sw_type);
    if(callback_send!=NULL)
        callback_send(NOTIFY,buffer,m_inst);
    return 0;
}

void CFTP::SetCallback(SendCallback callback,void* inst)
{
    callback_send = callback;
    m_inst=inst;
}

local_file_t* CFTP::Pn_New_File_Info(file_dir* ctx, const char* path, char type)
{
    local_file_t* file = (local_file_t *)calloc(1, sizeof(local_file_t));
    size_t parent_path_len = ctx->parent == ctx->root ?
        0 : strlen(ctx->parent->path);

    file->path = (char *)calloc(1, strlen(path) + parent_path_len + 2);
    file->type = type;

    if(ctx->parent != ctx->root)
        sprintf(file->path, "%s/%s", ctx->parent->path, path);
    else
        sprintf(file->path, "%s", path);

    char path1[4096] = {0};
    sprintf(path1, "%s/%s", ctx->root->path, file->path);

    struct stat file_stat;
    stat(path1, &file_stat);
    file->size = file_stat.st_size;

    file->next = ctx->parent->child;
    ctx->parent->child = file;

    return file;
}

void CFTP::Pn_Dir_List(file_dir* ctx)
{
    DIR              *pDir;
    struct dirent    *ent;
    char path[512] = {0};

    if(ctx->parent == ctx->root)
        sprintf(path, "%s", ctx->root->path);
    else
        sprintf(path, "%s/%s", ctx->root->path, ctx->parent->path);

    pDir = opendir(path);
    if(pDir==NULL){
        printf("Open Directory %s Error：%s\n", path, strerror(errno));
        return;
    }

    while((ent=readdir(pDir))!=NULL)
    {
        if(ent->d_type & DT_DIR)
        {
            if(strcmp(ent->d_name,".")==0 || strcmp(ent->d_name,"..")==0)
                continue;

            local_file_t* new_file = Pn_New_File_Info(ctx, ent->d_name, 'D');
            local_file_t* old_parent = ctx->parent;
            ctx->parent = new_file;
            Pn_Dir_List(ctx);
            ctx->parent = old_parent;
        }
        else
        {
            Pn_New_File_Info(ctx, ent->d_name, 'F');
            backup_file.total++;
        }
    }
}

int CFTP::PN_Ftp_Filename_Parse(void* config_data)
{
    char* p1=NULL;
    pn_file_info *file_info;
    char text[200] = {0};
    file_info = (pn_file_info *)config_data;

    p1 = rindex(file_info->localfile, '/');
    if(p1 == NULL)
        return PN_ERR_FAILED;
    strcpy(text, p1+1);

    // if(strlen(text) != FILELENTH)
        // return PN_ERR_FAILED;
    if(strstr(text,".jpg"))
        strcpy(file_info->file_type,"Pic");
    else if(strstr(text,".264"))
        strcpy(file_info->file_type,"Video");
    else 
        return PN_ERR_FAILED;

    strcpy(file_info->remotename,text);
    debugstr(file_info->remotename);
    strcpy(file_info->dev_id, dev_id);
    strncpy(file_info->time.year,text,4);
    strncpy(file_info->time.month,text+5,2);
    strncpy(file_info->time.day,text+8,2);
    sprintf(file_info->date,"%s%s%s",file_info->time.year,file_info->time.month,file_info->time.day);
    debugstr(file_info->date);
    sprintf(file_info->remotedir,"/%s/%s/%s",file_info->dev_id,file_info->file_type,file_info->date);
    debugstr(file_info->remotedir);
    return PN_ERR_OK;
}

int CFTP::PN_Ftp_Filename_Parse1(char* path,void* config_data)
{
    debugstring("filenme parse++++++++++++++++++++++++++++++\n");
    debugstr(path);
    char* p1=NULL;
    pn_file_info *file_info;
    file_info = (pn_file_info *)config_data;

    p1 = rindex(path, '/');
    if(p1 == NULL)
        return PN_ERR_FAILED;
    strcpy(file_info->remotename, p1+1);
    debugstr(file_info->remotename);
    strncpy(file_info->remotedir,path+1,path-p1-1);
    debugstr(file_info->remotedir);
    return PN_ERR_OK;
}

int CFTP::Pn_File_Operate(char *path)
{
    // do the file operate 
    int err;
    msg_task msg_send;
    pn_file_info file_info;

    memset(&file_info,0,sizeof(pn_file_info));
    // send file upload progress to qt
    backup_file.index++;
    msg_send.msg_type = DEVWORKORDER;
    msg_send.m_type = DEVBACKUPRESP;
    sprintf(msg_send.text, "%d;%d", backup_file.index, backup_file.total);
    debugstr(msg_send.text);
    m_msq.Msq_Send_Task(msg_send);

    // get the remote file dir and remote file name
    strcpy(file_info.localfile,path);
    err = PN_Ftp_Filename_Parse((void *)&file_info);
    if(err)
    {
        return PN_ERR_FAILED;
    }

    //upload file
    err = ftp_upload(file_info.localfile, file_info.remotedir,file_info.remotename);
    return err;
}

int CFTP::Pn_Dir_Operate(file_dir* ctx)
{
    int err = PN_ERR_OK;
    int ret = PN_ERR_OK;
    char path1[512] = {0};
    local_file_t* file = ctx->parent->child;

    for(; file!=NULL; file=file->next)
    {
        if(file->type == 'F')
        {
            printf("file: %s, size: %ld\n", file->path, file->size);
            sprintf(path1,"%s/%s", ctx->root->path, file->path);
            err = Pn_File_Operate(path1);
            if(err)
            {
                backup_file.index--;
                err = Pn_File_Operate(path1);
                if (err)
                    ret = PN_ERR_FAILED;
            }
            else
                Pn_Remove_File(path1);
        }
        else if(file->type == 'D')
        {
            local_file_t* old_parent = ctx->parent;
            printf("dir: %s\n", file->path);

            ctx->parent = file;
            err = Pn_Dir_Operate(ctx);
            ctx->parent = old_parent;
            if (err)
                ret = err;
        }
    }
    return ret;
}

void CFTP::Pn_Dir_Free(file_dir* ctx)
{
    local_file_t* file = ctx->parent->child;
    local_file_t* filetmp = NULL;

    for(; file!=NULL; file=filetmp)
    {
        if(file->type == 'F')
        {
            filetmp=file->next;
            free(file->path);
            free(file);
        }
        else if(file->type == 'D')
        {
            local_file_t* old_parent = ctx->parent;
            ctx->parent = file;
            Pn_Dir_Free(ctx);
            filetmp=file->next;
            free(file->path);
            free(file);
            ctx->parent = old_parent;
        }
    }
    return;
}

void CFTP::Pn_Remove_File(char* path)
{
    char cmd[100];

    sprintf(cmd,"rm %s",path);
    system(cmd);

    return;
}
