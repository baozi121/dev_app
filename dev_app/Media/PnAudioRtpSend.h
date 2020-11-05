#pragma once
#include <rtpsession.h>
#include "rtpconfig.h"
#include "rtptransmitter.h"
#include "rtpipv4destination.h"
#include "rtpsourcedata.h"
#include "rtphashtable.h"
#include "rtpkeyhashtable.h"
#include "rtpudpv4transmitter.h"
#include "rtpipv4address.h"
#include "StreamBuf.h"
#include "PnShareMem.h"
#include <pthread.h>
#include <list>
#define PT_ACTIVE_RTP  122
// #define PT_AUDIO_ACTIVE_RTP  0

class PnAudioRtpSend :public RTPSession
{
    public:
        PnAudioRtpSend(void);
        ~PnAudioRtpSend(void);
        int SetAudioMTU(size_t s);
        void OnPollThreadStep();
        int AnalyRtpData(RTPPacket *pack,RTPSourceData *srcdat);
        static void * heartbeatthread(void * arg);
        static void * audiowrite(void * arg);

        RTPHashTable<const RTPIPv4Destination,RTPUDPv4Trans_GetHashIndex_IPv4Dest,RTPUDPV4TRANS_HASHSIZE> temp;
        RTPHashTable<const RTPIPv4Destination,RTPUDPv4Trans_GetHashIndex_IPv4Dest,RTPUDPV4TRANS_HASHSIZE> destemp;

    private:
        size_t MaxPacketSize;
        void *buffer;
        StreamBuf* m_audio_buf;
        StreamBufNode *pRtpFrameNode;
        PnShMem *mDownAudioShm;
        char *mDownAudioTemp;
        pthread_t audio_pid;
        pthread_t heartbeat_pid;
        bool audio_quit;
        int count;
        int cnt ;
		unsigned int CurSsrc;
        static PnAudioRtpSend * m_this;
};

