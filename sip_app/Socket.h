#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define CLIENT_MAX              10
#define CLIENT_NAME_MAX         15
#define SERVER_PORT             9090
#define MSG_LEN_MAX             1500

typedef struct
{
    struct sockaddr_in client_addr;
    char client_name[CLIENT_NAME_MAX];
}pn_client_info;

typedef struct
{
    char client_name[CLIENT_NAME_MAX];
    int port;
    int method;
    char msg[MSG_LEN_MAX];
}pn_msg_info;

const char msg_type[][10]={
    "name=",
    "port=",
    "method=",
    "msg=",
};

int socket_init();
int socket_close();
int socket_listening();
int socket_send(const char* msg);
static void *thread_listening(void* arg);
static int client_parse(pn_msg_info msg_info);
static int msg_parse(char* msg, pn_msg_info*msg_info);
static int is_num(const char* buf);
#endif

