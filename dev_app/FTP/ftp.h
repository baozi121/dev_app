#ifndef _FTP_H_
#define _FTP_H_

#include <time.h>
#include "IniFile.h"
#include "Pnmsq.h"

typedef struct local_file_s {
    long size;
    time_t date;
    char *path;
    char type;

    struct local_file_s* next;
    struct local_file_s* child;
} local_file_t;

typedef struct file_dir_s {
    local_file_t *parent;
    local_file_t *root;
} file_dir;

typedef struct file_upload_num_s {
    unsigned int index;
    unsigned int total;
} file_upload_num;


class CFTP
{
    public:
        CFTP(void);
        ~CFTP(void);
        int ftp_connect(const char* ip);
        int ftp_login(char* user,char* pass);
        int ftp_pwd(char* buff);
        int ftp_cd(char* dir);
        int ftp_cdup();
        int ftp_mkdir(char* dir);
        int ftp_rmdir(char* dir);
        int ftp_setpasv();
        int ftp_upload(char* localfile,char* remotepath,char* remotefilename);
        int ftp_download(char* localfile,char* remotefile);
        int ftp_quit();
        int PN_Ftp_Init();
        int PN_Ftp_Release();
        int PN_Ftp_Setconfig(int config_id ,void* config_data);
        int PN_Ftp_Query_XML_Send();
        int PN_Ftp_File_Upload_XML_Send(void* config_data);
        int PN_Ftp_Software_Update_XML_Send();
        void SetCallback(SendCallback callback,void* inst);
    private:
        int ftp_sendcmd();
        int ftp_checkresp(char expresp);
        int ftp_mkdirSingle(char* dir);
        int PN_Ftp_Software_Version_Read_Config();
        int PN_Ftp_Download_Software();
        int PN_Ftp_Setconfig_User_Info(void* config_data);
        int PN_Ftp_Setconfig_Upload_File(void* config_data);
        int PN_Ftp_Setconfig_Software_Update_Response(void* config_data);
        int PN_Ftp_Listening();
        static void PN_Ftp_Thread_Upload_Listening(void* params);
        void PN_Ftp_Thread_Upload();
        static void PN_Ftp_Thread_Download_Listening(void* params);
        int Pn_File_Backup(char *path);
        int PN_Ftp_Filename_Parse(void* config_data);
        int PN_Ftp_Filename_Parse1(char* path,void* config_data);
        void Pn_Remove_File(char* path);
        int Pn_File_Operate(char* path);
        int Pn_Dir_Operate(file_dir* ctx);
        void Pn_Dir_List(file_dir* ctx);
        void Pn_Dir_Free(file_dir* ctx);
        local_file_t* Pn_New_File_Info(file_dir* ctx, const char* path, char type);

        int m_sockctrl;
        int m_sockdata;
        char m_cmd[256];
        char m_resp[256];
        char m_ip[64];

        char dev_id[20];
        char server_ip[20];
        char user_name[20];
        char user_passwd[20];
        char oldversion[20];
        char sw_type[20];
        char newversion[20];
        file_upload_num backup_file;

        int ftp_status;
        int upload_status;
        pthread_t upload_thread;
        pthread_t download_thread;
        SendCallback    callback_send;
        CIniFile m_ini;
        CMSQ m_msq;
        void* m_inst;
};

#endif
