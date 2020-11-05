#pragma once
#include <rtpsession.h>
#include "rtpconfig.h"
#include "rtptransmitter.h"
#include "rtpipv4destination.h"
#include "rtphashtable.h"
#include "rtpkeyhashtable.h"
#include "rtpudpv4transmitter.h"
#include "rtpipv4address.h"
#include "StreamBuf.h"
#include "PnShareMem.h"
#include <pthread.h>
#include <list>
#define PT_ACTIVE_RTP  122

struct NALU
{
	unsigned char * offset;
	size_t len;
};

class PnH264RtpSend :public RTPSession
{
	public:
		PnH264RtpSend(void);
		~PnH264RtpSend(void);
		int SendH264Packet(const void * data, size_t len,size_t pt);
		int SetMTU(size_t s);
		int heartbeat();
		static void * heartbeatthread(void * arg);

		static const unsigned short UNKNOW = 0;
		static const unsigned short NALU = 23;
		static const unsigned short STAP_A = 24;
		static const unsigned short STAP_B = 25;
		static const unsigned short MTAP16 = 26;
		static const unsigned short MTAP24 = 27;
		static const unsigned short FU_A = 28;
		static const unsigned short FU_B = 29;

		static const unsigned char FU_START = 0x80;
		static const unsigned char FU_MIDDLE = 0x00;
		static const unsigned char FU_END = 0x40;
		static const unsigned char FU_HEAD = 2;

		//  virtual void OnGetNalu(PnNaluBuilder *naluBuilder)  {}
	private:
		size_t MaxPacketSize;
		void *buffer;
		int count;
		PnShMem *mDownAudioShm;
		char *mDownAudioTemp;
		static PnH264RtpSend * m_this;
};

