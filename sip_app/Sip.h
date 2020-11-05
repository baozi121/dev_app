#ifndef __SIP_H__
#define __SIP_H__

#include <pjsua-lib/pjsua.h>

const char register_msg[]="<?xml version=\"1.0\"?>\r\n<login_request><password>%s</password></login_request>\r\n";
const pj_str_t INVITE_METHOD = pj_str("INVITE");
const pj_str_t BYE_METHOD = pj_str("BYE");

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

typedef struct
{
    char devname[20];
    char devid[20];
    char devpwd[20];
    char servname[20];
    char servid[20];
    char servip[20];
    char servport[10];
    char servdn[40];
}sip_config_t;

typedef struct
{
    int valide;
    char user_id[20];
    char user_name[20];
}user_info_t;

int sip_read_config();
int sip_init();
int sip_destroy();
int sip_register();
int sip_send_msg(int msg_type, char* msg);

static void on_message(pjsua_call_id call_id, const pj_str_t *from,
                        const pj_str_t *to, const pj_str_t *contact,
                        const pj_str_t *mime_type, const pj_str_t *body,
            pjsip_rx_data *rdata, pjsua_acc_id acc_id);

static void on_response(pjsua_call_id call_id, const pj_str_t *to,
                                const pj_str_t *body, void *user_data,
                                pjsip_status_code status,
                                const pj_str_t *reason,
                pjsip_tx_data *tdata,
                pjsip_rx_data *rdata,
                pjsua_acc_id acc_id);

static int sip_keepalive();
static void *thread_keepalive(void* arg);
static void hex2str(const unsigned char digest[], char *output);
static char* Md5_Digest(char* msg);
static int sip_register_2(char* seed);
static void Get_User_Info(char* from,int len);
#endif
