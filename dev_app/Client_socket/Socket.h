#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "comm.h"
#include "Pnxml.h"

#define CLIENT_NAME_MAX 15
#define SERVER_PORT 9090
#define client_PORT 9091
#define MSG_LEN_MAX 1024

const char process_name[] ="dev_app";
const char msg_type[] ="msg=";

class CSocket
{
    public:
        CSocket();
        ~CSocket();
        int PN_Socket_Init();
        int PN_Socket_Close ();
        int PN_Socket_Listening();
        static int PN_Socket_Send(int method,const char* msg, void* inst);
        int PN_Socket_reg();
    private:
        static void PN_Socket_Thread_Listening(void* params);
        int PN_Socket_Msg_Parse(char* msg, char*msg_info);

        int client_socket;
        struct sockaddr_in server_addr;
        int port;
        CPnxml m_pnxml;
        pthread_mutex_t mutex;
};

#endif
