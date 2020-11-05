#include "PnH264RtpSend.h"
#include "rtpsourcedata.h"
#include <queue>
#include <iostream>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include "comm.h"
#include<unistd.h>
#define DOWN_AUDIO_SHM_DIR "/shm/audio2"
#define DOWN_AUDIO_SHARE_SIZE 160

using namespace std;

PnH264RtpSend* PnH264RtpSend::m_this=NULL;

PnH264RtpSend::PnH264RtpSend(void)
{
	buffer = NULL;
	SetMTU(1400);
	count=0;
	m_this=this;
}

PnH264RtpSend::~PnH264RtpSend(void)
{
}

int PnH264RtpSend::SendH264Packet(const void * data, size_t len,size_t pt)
{
	unsigned char * point, *headpoint, *nalup, *cbuffer;
	size_t count;
	struct NALU nalu;
	queue<struct NALU> qnalu;

	//区分NALU放入QUEUE内
	headpoint = point = (unsigned char *)data;
	while(point < headpoint+len-3)
	{
		if((point[0] == 0x00 && point[1] == 0x00)
				&&( (point[2] == 0x00 && point[3] == 0x01)
					|| point[2] == 0x01))
		{
			if(point[3] == 0x01)
				point += 4;
			else if(point[2] == 0x01)
				point += 3;

			nalup= point;
			count = 0;
			while(point < headpoint+len)
			{
				if((point[0] == 0x00 && point[1] == 0x00)
						&&( (point[2] == 0x00 && point[3] == 0x01)
							|| point[2] == 0x01))
				{
					break;
				}
				count++;
				point++;
			}
			nalu.offset = nalup;
			nalu.len = count;
			qnalu.push(nalu);
		}
		point++;
	}

	while(qnalu.size()){
		nalu=qnalu.front();
		{
			if(nalu.len <= MaxPacketSize - 5)
			{
				//构建独立NAL包并发送
				SendPacket((void *)nalu.offset, nalu.len,pt,false,3600);
			}
			else
			{
				point = nalu.offset;
				//构建分片FU包并循环发送
				while(point < nalu.offset + nalu.len)
				{
					cbuffer = (unsigned char *)buffer;
					//清空缓存
					memset(buffer, 0, MaxPacketSize);
					*cbuffer = (*(nalu.offset) & 0xe0) | FU_A;
					cbuffer++;

					if (point == nalu.offset)
					{
						//分片第一个包
						*cbuffer = FU_START | (*(nalu.offset) & 0x1f);
						cbuffer++;
						memcpy(cbuffer, point, MaxPacketSize - FU_HEAD);
						point += (MaxPacketSize-FU_HEAD);
						//发送缓存
						SendPacket(buffer, MaxPacketSize,pt,false,0);
						usleep(4000);
					}
					else if((point + MaxPacketSize - FU_HEAD) < (nalu.offset + nalu.len))
					{
						//分片中间包
						*cbuffer = FU_MIDDLE | (*(nalu.offset) & 0x1f);
						cbuffer++;
						memcpy(cbuffer, point, MaxPacketSize - FU_HEAD);
						point += (MaxPacketSize-FU_HEAD);
						//发送缓存
						SendPacket(buffer, MaxPacketSize,pt,false,0);
						usleep(4000);
					}
					else
					{
						//分片结尾包
						*cbuffer = FU_END | (*(nalu.offset) & 0x1f);
						cbuffer++;
						memcpy(cbuffer, point, nalu.len+nalu.offset - point);
						SendPacket(buffer, nalu.len+nalu.offset-point+FU_HEAD,pt,false,3600);
						usleep(4000);
						point = nalu.len+nalu.offset;
					}
				}
			}
		}
		qnalu.pop();
	}
	return 0;
}

int PnH264RtpSend::SetMTU(size_t s)
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

