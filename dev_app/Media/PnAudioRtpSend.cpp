#include<unistd.h>
#include "PnAudioRtpSend.h"
#include <iostream>
#include <fcntl.h>
#include <signal.h>
#include "comm.h"
#define DOWN_AUDIO_SHM_DIR "/shm/audio2"
#define DOWN_AUDIO_SHARE_SIZE 160

using namespace std;
bool audio_state = true;   //the flag for heartbeat 
StreamBuf *pBuf;
PnAudioRtpSend* PnAudioRtpSend::m_this=NULL;

PnAudioRtpSend::PnAudioRtpSend(void)
{
    buffer = NULL;
    SetAudioMTU(1400);
    m_audio_buf= new StreamBuf(100,3);
    mDownAudioShm = NULL;
    mDownAudioTemp = NULL;
    mDownAudioTemp = (char *)malloc(DOWN_AUDIO_SHARE_SIZE);
    mDownAudioShm = new PnShMem(DOWN_AUDIO_SHM_DIR,DOWN_AUDIO_SHARE_SIZE);
    audio_quit=0;
    audio_pid=0;
    heartbeat_pid =0;
    cnt =0 ;    
    count=0;
	CurSsrc=0;
    m_this=this;
    temp.Clear();
    destemp.Clear();
}

PnAudioRtpSend::~PnAudioRtpSend(void)
{
    audio_quit=1;
    audio_state = true;
    if(buffer != NULL)
        free(buffer);
    if(0!=audio_pid)
        while(0==pthread_kill(audio_pid,0))
        {
            printf("start to cancel audio thread");
            pthread_cancel(audio_pid);
            usleep(100*1000);
        }
    if(0!= heartbeat_pid)
        while(0==pthread_kill(heartbeat_pid,0))
        {
            printf("start to cancel heartbeat thread\n");
            pthread_cancel(heartbeat_pid);
            usleep(100*1000);
        }
    debugstring("audio _released");

    delete mDownAudioShm;
    mDownAudioShm = NULL;
    free(mDownAudioTemp);

    if(m_audio_buf){
        delete m_audio_buf;
        m_audio_buf = NULL;
    }
}

int PnAudioRtpSend::SetAudioMTU(size_t s)
{
    if(s < 32)
        return ERR_RTP_SESSION_NOTCREATED;

    MaxPacketSize = s - 16;
    if(buffer != NULL)
    {
        free(buffer);
    }
    buffer = malloc(MaxPacketSize);
    if (buffer == NULL)
    {
        debugstring("can not malloc enough mm for rtp buffer");
        return ERR_RTP_SESSION_NOTCREATED;
    }

    SetMaximumPacketSize(s);
    return 0;
}

void PnAudioRtpSend::OnPollThreadStep()
{
    int status = 0;
    BeginDataAccess();
    count++;
    if(count>3)
    {
        count=0;
        audio_quit=1;
    }
    if (GotoFirstSourceWithData())
    {
        do
        {
            RTPPacket *pack;
            RTPSourceData *srcdat;
            srcdat = GetCurrentSourceInfo();
            while ((pack = GetNextPacket()) != NULL)
            {
                status = AnalyRtpData(pack,srcdat);
                if(count>0)
                    count-=2;
                if(status)
                    DeletePacket(pack);
                pack = NULL;
            }
        } while (GotoNextSourceWithData());
    }
    EndDataAccess();
}

int PnAudioRtpSend::AnalyRtpData(RTPPacket *pack,RTPSourceData *srcdat)
{
    int PayLoadType = 0;
    int PacketLength = 0;
    int ret = 0;
    PayLoadType = (int)pack->GetPayloadType();
    PacketLength = pack->GetPacketLength();
    if(PayLoadType == 69||(PayLoadType == 8 && PacketLength == 172))
    {
        if(0==audio_pid)
        {
            audio_quit=0;
            count=0;
            ret=pthread_create(&audio_pid,NULL,audiowrite,(void*)this);
            if(ret)
            {
                debugstring("pthread_create error");
                ret=pthread_create(&audio_pid,NULL,audiowrite,(void*)this);
                // debugstring("pthread_create error");
            }
            usleep(2000);
			CurSsrc = pack->GetSSRC();
        }
		// debugint(CurSsrc);
		// debugint(pack->GetSSRC());
		if(pack->GetSSRC() != CurSsrc)
		{
			debugstring("clear buf!\n");
			m_audio_buf->ClearBuf();
			CurSsrc = pack->GetSSRC();
		}
		pRtpFrameNode = new StreamBufNode(( char *)pack->GetPayloadData(),pack->GetPayloadLength(), pack->GetSequenceNumber(),0 );
		m_audio_buf->AddToBufBySeq(pRtpFrameNode);
		DeletePacket(pack);
        return 0;
    }
    else
        if(PayLoadType == 8 && PacketLength == 32)  //count the heartbeat packets
        {     
            cnt++;
            if(cnt >= 60)
                cnt = 0 ;
            if(0 == heartbeat_pid)
            {
                ret=pthread_create(&heartbeat_pid,NULL,heartbeatthread,(void*)this);
                if(ret)
                debugstring("pthread_create error");
                usleep(2000);
            }
            return 1;
         }
    return 1;
}

void * PnAudioRtpSend::audiowrite(void * arg)
{
    PnAudioRtpSend* evn;
    evn=(PnAudioRtpSend *)(arg);
    StreamBufNode* temp;
    int downsize;
    int length;
    while(1){
        length =m_this->m_audio_buf->GetCurLen();
        // debugint(length);
        if(m_this->m_audio_buf->GetCurLen()>0)
        {
            temp=m_this->m_audio_buf->GetFromBuf();
            if(temp->GetData()&& temp->GetLen()> 0)
            {
                downsize = m_this->mDownAudioShm->WriteShM(temp->GetData(),temp->GetLen());
                downsize = 0;
                delete temp;
            }
        }
        if(m_this->m_audio_buf->GetCurLen()>10)
        {
            temp=m_this->m_audio_buf->GetFromBuf();
            if(temp->GetData()&& temp->GetLen()> 0)
            {
                downsize = m_this->mDownAudioShm->WriteShM(temp->GetData(),temp->GetLen());
                downsize = 0;
                delete temp;
            }
        }
        usleep(2000);
        pthread_testcancel();
        // if(evn->audio_quit==1)
            // break;
    }
    // debugstring("audio_quit");
    // evn->audio_quit=0;
    // evn->audio_pid=0;
    return NULL;
}

void * PnAudioRtpSend::heartbeatthread(void * arg)
{
    PnAudioRtpSend* rtp;
    rtp =(PnAudioRtpSend *)(arg);
    while(1)
    {
        int temp = 0;
        temp = rtp->cnt;
        sleep(3);              
        if(temp == rtp->cnt)
        {
            debugstring("audio out!!!!!!!!!!!\n\n");
            audio_state = false;
            break;
        }
    }
    return NULL;
}
