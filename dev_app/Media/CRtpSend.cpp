#include "CRtpSend.h"
#include <iostream>
#include "comm.h"

using namespace std;
#define VIDEO_SHM_DIR "/shm/video/v2"
#define VIDEO_SHARE_SIZE  352*288*3/2
#define MEDIA_INVITE    0
#define SIPCALL_INVITE  1

static ActiveData m_activedata;

static void checkerror(int rtperr)
{
    if (rtperr < 0)
    {
        std::cout << "ERROR: " << RTPGetErrorString(rtperr) << std::endl;
        exit(-1);
    }
}

CRtpSend::CRtpSend()
{
    mVideoShm = NULL;
    mVideoTemp =  NULL;
    portbase = 0;
    m_criticalsection = hw_init_critical();

    activethread_quit = 0;
    activethread_quit_event =  NULL;
    h_activethread = NULL;
}

CRtpSend::~CRtpSend()
{
    activethread_quit = 1;
    hw_wait_event(activethread_quit_event, -1);
    debugstring("quit the thread of active");

    if(activethread_quit_event)
    {
        hw_release_event(activethread_quit_event);
        activethread_quit_event = NULL;
    }

    if(h_activethread)
    {
        hw_release_thread(h_activethread);
        debugstring("release the thread of active");
        h_activethread = NULL;
    }

    hw_release_critical(m_criticalsection);

    if(mVideoShm){
        delete mVideoShm;
        mVideoShm = NULL;
        free(mVideoTemp);
    }
}

void CRtpSend::runLoop()
{
    int Videostatus;
    Videostatus = mVideoShm->ReadShM(mVideoTemp,VIDEO_SHARE_SIZE);
    hw_enter_critical(m_criticalsection);
    sess.SendH264Packet(mVideoTemp, Videostatus,PT_VIDEO_RTP);
    hw_leave_critical(m_criticalsection);
}

int CRtpSend::init(unsigned int destip, unsigned short destport, unsigned int  ssrc, char * guid)
{
    int status;

    mVideoTemp = (char *)malloc(VIDEO_SHARE_SIZE);
    if(mVideoTemp == NULL)
    {
        cerr<<"create temp buffer error"<<endl;
        return -1;
    }
    mVideoShm = new PnShMem(VIDEO_SHM_DIR, VIDEO_SHARE_SIZE);
    if(mVideoShm == NULL)
    {
        cerr<<"create shm error"<<endl;
        free(mVideoTemp);
        return -1;
    }

    portbase = 5000;
    RTPUDPv4TransmissionParams transparams;
    RTPSessionParams sessparams;

    sessparams.SetOwnTimestampUnit(1.0/90000.0);

    sessparams.SetAcceptOwnPackets(true);
    sessparams.SetUsePollThread(true);

    transparams.SetPortbase(portbase);
    status = sess.Create(sessparams,&transparams);
    checkerror(status);

    RTPIPv4Address addr(destip,destport);

    status = sess.AddDestination(addr);

    sess.SetMTU(1000);
    sess.SetDefaultPayloadType(PT_VIDEO_RTP);
    sess.SetDefaultMark(false);
    sess.SetDefaultTimestampIncrement(3600);
    if(ssrc != 0)
    {
        sess.SetSSRC(ssrc);
    }

    m_activedata.cmd = htons(1);
    m_activedata.len = htons(16);
    memcpy(m_activedata.guid,guid,16);

    // open the active thread
    activethread_quit = 0;
    activethread_quit_event = hw_create_event();
    h_activethread = hw_create_thread((void *)CRtpSend::ActivePacketSender, (void *)this);
    if(h_activethread == NULL)
    {
        return -1;
    }

    hw_enter_critical(m_criticalsection);
    sess.SendPacket((void *)&(m_activedata),sizeof(ActiveData),PT_ACTIVE_RTP,false,10);
    sess.SendPacket((void *)&(m_activedata),sizeof(ActiveData),PT_ACTIVE_RTP,false,10);
    hw_leave_critical(m_criticalsection);
    return 0;
}

void CRtpSend::ActivePacketSender(void *params)
{
    CRtpSend *pRtpSend = (CRtpSend *) params;

    while(1)
    {
        unsigned int tick = hw_get_sys_tick();
        do
        {
            if(pRtpSend->activethread_quit)
                goto END ;
            hw_delay(20);
        }while ( (int)(hw_get_sys_tick()-tick) < ACTIVE_TIME);//20s

        hw_enter_critical(pRtpSend->m_criticalsection);
        pRtpSend->sess.SendPacket((void *)&(m_activedata),sizeof(ActiveData),PT_ACTIVE_RTP,false,10);
        pRtpSend->sess.SendPacket((void *)&(m_activedata),sizeof(ActiveData),PT_ACTIVE_RTP,false,10);
        pRtpSend->sess.SendPacket((void *)&(m_activedata),sizeof(ActiveData),PT_ACTIVE_RTP,false,10);
        hw_leave_critical(pRtpSend->m_criticalsection);
        hw_delay(20);
    }
END:
    hw_set_event(pRtpSend->activethread_quit_event);
    return;
}

