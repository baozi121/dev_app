#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include "Pnmsq.h"
#include "comm.h"

CMSQ::CMSQ(void)
{
    msqid = -1;
}

CMSQ::~CMSQ(void)
{
    // msgctl(msqid, IPC_RMID, 0);
}

int CMSQ::Msq_Create()
{
    key_t lKey;
    if((lKey = ftok(PATH,1)) == -1)
    {
        perror("ftok");
        exit(1);
    }

    if((msqid = msgget(lKey,IPC_CREAT|0666)) < 0)
    {
        fprintf(stderr,"Problema al crear la cola de mensages IPC\n");
        return -1;
    }
    // for(;;)
    // {
        // msg_task msg;
        // if(msgrcv(msqid,&msg,sizeof(msg),0,0)==-1)
            // break;
    // }
    return PN_ERR_OK;
}
#if 0
int CMSQ::Msq_Delete()
{
    debugint(msqid);
    if(msqid!=-1)
    {
        if(msgctl(msqid, IPC_RMID, 0) != 0)
        {
            printf("msgctl(IPC_RMID) failed\n");
            return PN_ERR_FAILED;
        }
    }
    return PN_ERR_OK;
}

int CMSQ::Msq_Send_Sipcall(msg_sipcall msg)
{
    int err;

    if(msqid==-1)
    {
        if(Msq_Create()==-1)
            return PN_ERR_FAILED;
    }
    printf("msg_type=%d\n",msg.msg_type);
    printf("flag = %d\n",msg.flag);
    printf("caller = %s\n",msg.caller);

    err=msgsnd(msqid,&msg,sizeof(msg_sipcall),0);
    if(err==-1)
    {
        printf("Send Msq failed!\n");
        perror("msgsnd");
        return PN_ERR_FAILED;
    }

    return PN_ERR_OK;
}

int CMSQ::Msq_Read_Sipcall(msg_sipcall* msg, long int type)
{
    int err;
    memset(msg, 0x00, sizeof(msg_sipcall));

    if(msqid==-1)
    {
        if(Msq_Create()!=PN_ERR_OK)
            return PN_ERR_OPEN;
    }

    err = msgrcv(msqid, msg, sizeof(msg_sipcall), type, 0);
    if(err < 0)
    {
        perror("msgrcv error !!!!!!!!!!!\n");
        return PN_ERR_FAILED;
    }

    return PN_ERR_OK;
}
#endif

int CMSQ::Msq_Send_Task(msg_task msg)
{
    int err;
    if(msqid==-1)
    {
        if(Msq_Create()==-1)
            return PN_ERR_FAILED;
    }

    err=msgsnd(msqid,&msg,sizeof(msg_task),0);
    if(err==-1)
    {
        printf("Send Msq failed!\n");
        perror("msgsnd");
        return PN_ERR_FAILED;
    }

    return PN_ERR_OK;
}

int CMSQ::Msq_Read_Task(msg_task* msg, long int type)
{
    int err;

    memset(msg, 0x00, sizeof(msg_task));

    if(msqid==-1)
    {
        if(Msq_Create()!=PN_ERR_OK)
            return PN_ERR_OPEN;
    }

    err = msgrcv(msqid, msg, sizeof(msg_task), type, 0);
    if(err < 0)
    {
        perror("msgrcv read error!!!!!!!!!!!!!!!!!!!\n");
        return PN_ERR_FAILED;
    }

    return PN_ERR_OK;
}

int CMSQ::Msq_Send_Control(msg_control msg)
{
    int err;

    if(msqid==-1)
    {
        if(Msq_Create()==-1)
            return PN_ERR_FAILED;
    }

    err=msgsnd(msqid,&msg,sizeof(msg_control),0);
    if(err==-1)
    {
        printf("Send Msq failed!\n");
        perror("msgsnd");
        return PN_ERR_FAILED;
    }

    return PN_ERR_OK;
}

int CMSQ::Msq_Read_Control(msg_control* msg, long int type)
{
    int err;

    memset(msg, 0x00, sizeof(msg_control));

    if(msqid==-1)
    {
        if(Msq_Create()!=PN_ERR_OK)
            return PN_ERR_OPEN;
    }

    err = msgrcv(msqid, msg, sizeof(msg_control), type, 0);
    if(err < 0)
    {
        perror("msgrcv");
        return PN_ERR_FAILED;
    }

    return PN_ERR_OK;
}
