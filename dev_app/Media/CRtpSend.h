#ifndef __CRTPSEND_H__
#define __CRTPSEND_H__

#include "CThreadObject.h"
#include "PnShareMem.h"
#include "PnH264RtpSend.h"
#include "hw_lib.h"

#include "rtpsession.h"
#include "rtppacket.h"
#include "rtpudpv4transmitter.h"
#include "rtpipv4address.h"
#include "rtpsessionparams.h"
#include "rtperrors.h"

#define ACTIVE_TIME 20*1000
#define VIDEO_FORMAT_TIME 5*1000

typedef struct _tag_ActiveData
{
    unsigned short cmd;
    unsigned short len;
    char guid[16];
}ActiveData;

class CRtpSend :public CThreadObject
{
    public:
        CRtpSend();
        ~CRtpSend();
        void runLoop();

        int init(unsigned int destip, unsigned short destport, unsigned int  ssrc,char * guid);

    private:
        static void ActivePacketSender(void *params);

    public:
        volatile int video_format;
    private:
        PnShMem *mVideoShm;
        char *mVideoTemp;

        unsigned short portbase;

        void *h_activethread;
        int activethread_quit;
        void *activethread_quit_event;

        PnH264RtpSend sess;
        void *m_criticalsection;
        //static const int PT_ACTIVE_RTP = 122;
        static const int PT_VIDEO_RTP = 98;
};

#endif
