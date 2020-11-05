/*****************************************************************************
 * Filename: StreamBuf.cpp
 * Description: Declaration of the class RTPClient
 *
 * Modify History:
 * Date: 2009.08.07      Author: Tony
 *
 ****************************************************************************/
//this file is added  by cl for audio postplay

#include <iostream>
#include "StreamBuf.h"
#include "hw_lib.h"
//#include <streams.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#define readylen  (12000/1000)

StreamBufNode::StreamBufNode()
: m_pData(NULL),
    m_nLen(0),
    m_nKey(-1),
    m_nPts(0),
    m_pNext(NULL)
{
}

StreamBufNode::StreamBufNode(char *pData, int nLen, unsigned short nPts, int nKey)
: m_pData(NULL),
    m_nLen(0),
    m_nKey(-1),
    m_nPts(0),
    m_pNext(NULL)
{
    if(pData && nLen > 0)
    {
        try
        {
            // malloc length is nLen+8
            m_pData = new char[nLen];
            if(m_pData)
            {
                memcpy(m_pData, pData, nLen);
                m_nLen = nLen;
                m_nPts = nPts;
                m_nKey = nKey;
                m_pNext = NULL;
            }
        }
        catch (...)
        {
            delete []m_pData;
            throw;
        }

    }
}

StreamBufNode::~StreamBufNode()
{
    if(m_pData)
    {
        delete []m_pData;
        m_pData = NULL;
    }
    m_nLen = 0;
    m_nPts = 0;
    m_nKey = -1;
    m_pNext = NULL;
}

char* StreamBufNode::GetData()
{
    return m_pData;
}

int StreamBufNode::GetLen()
{
    return m_nLen;
}

unsigned short StreamBufNode::GetPts()
{
    return m_nPts;
}

StreamBuf::StreamBuf()
: m_nLen(0),
    m_nCurLen(0),
    m_nReadyLen(0),
    m_bReady(false),
    m_pHead(NULL),
    m_pTail(NULL)
{

    m_nLen =30;
    m_nCurLen = 0;
    m_nReadyLen =readylen;
    m_pHead = NULL;
    m_pTail = NULL;
    m_bReady = false;
}

StreamBuf::StreamBuf(const int nLen, const int nReadyLen)
: m_nLen(0),
    m_nCurLen(0),
    m_nReadyLen(0),
    m_bReady(false),
    m_pHead(NULL),
    m_pTail(NULL)
{
    m_nLen =nLen;
    m_nCurLen = 0;
    if(nReadyLen<readylen)
        m_nReadyLen =readylen;
    else
        m_nReadyLen =nReadyLen;
    m_pHead = NULL;
    m_pTail = NULL;
    m_bReady = false;
}

StreamBuf::StreamBuf(const int nLen)
: m_nLen(0),
    m_nCurLen(0),
    m_nReadyLen(0),
    m_bReady(false),
    m_pHead(NULL),
    m_pTail(NULL)
{
    if(nLen > 0)
    {
        m_nLen = 100*nLen;
        m_nReadyLen = nLen;

    }
    else
    {
        m_nLen = 0;
    }
    m_nCurLen = 0;
    m_pHead = m_pTail = NULL;
    m_bReady = false;


}

StreamBuf::~StreamBuf()
{
    StreamBufNode *pNode = m_pHead;
    if(m_pHead)
        m_pHead = m_pHead->m_pNext;
    while(pNode)
    {
        delete pNode;
        pNode = m_pHead;
        if(m_pHead)
            m_pHead = m_pHead->m_pNext;
    }
    m_nLen = 0;
    printf("release!\n");
}

bool StreamBuf::AddToBuf(StreamBufNode *pNode)
{
    if(m_nCurLen  > m_nLen)//m_nLen是设置的buffer的长度
    {
        return false;
    }
    else if(IsEmpty())
    {
        m_pHead = m_pTail = pNode;
    }
    else
    {
        m_pTail->m_pNext = pNode;
        m_pTail = pNode;
    }
    m_nCurLen ++;
    if(m_nCurLen == m_nReadyLen)
    {
        m_bReady = true;
    }
    return true;
}

StreamBufNode* StreamBuf::GetFromBuf()
{
    if(IsEmpty())
    {
        return NULL;
    }
    StreamBufNode *pNode = m_pHead;
    m_pHead = m_pHead->m_pNext;
    m_nCurLen --;
    if(m_nCurLen < m_nReadyLen)
    {
        m_bReady = false;
    }
    return pNode;
}

bool StreamBuf::IsEmpty()
{
    return m_nCurLen == 0;
}

int StreamBuf::GetCurLen()
{
    return m_nCurLen;
}

void StreamBuf::SetReadyLen(const int nDelay)
{
    if(nDelay > 0)
    {
        if(nDelay > m_nReadyLen && m_bReady)
            m_bReady = false;
        m_nReadyLen = nDelay;
        while(m_nCurLen >= m_nReadyLen)
        {
            StreamBufNode *pNode = m_pHead;
            if(m_pHead)
                m_pHead = m_pHead->GetNext();
            if(pNode)
                delete pNode;
            m_nCurLen --;
        }
    }
}

void StreamBuf::ClearBuf()
{
    StreamBufNode *pNode = m_pHead;
    if(m_pHead)
        m_pHead = m_pHead->m_pNext;
    while(pNode)
    {
        delete pNode;
        m_nCurLen--;
        pNode = m_pHead;
        if(m_pHead)
            m_pHead = m_pHead->m_pNext;

    }
    if(m_nCurLen!=0)
    {
        m_nCurLen = 0;
    }

    m_nCurLen = 0;
    m_pHead = m_pTail = NULL;
    m_bReady = false;
    m_nReadyLen=readylen;
}

/*
   this function is ued to sort the sequence number of RTP packet,
   it maitains m_nReadyLen number of packet , if the number is less then
   m_nReadyLen ,do the same as the function of AddToBuf();
*/
bool StreamBuf::AddToBufBySeq(StreamBufNode *pNode)
{
    if(m_nCurLen  > m_nLen)
    {
        return false;
    }

    if(m_nCurLen == 0)
    {
        m_pHead = m_pTail = pNode;
        m_nCurLen ++;
        m_bReady = false;
        return true;
    }

    else if(m_nCurLen >= 0)
    {
        StreamBufNode *pTemp;
        if(m_nCurLen >= m_nReadyLen)
            m_bReady = true;

        if((short)(pNode->m_nPts-m_pTail->m_nPts)>0)        //arrived by sequence,insert after the tail ,
            {
                m_pTail->m_pNext = pNode;
                m_pTail = pNode;
                pNode->m_pNext = NULL;
                m_nCurLen ++;
                return true;
            }
        else                                                          //arrived not by sequence
        {
            if((short)(pNode->m_nPts-m_pHead->m_nPts)<0)
            {
                if((pNode->m_nPts-m_pHead->m_nPts)<0xfff0)  //add to delete to  early packet
                    return false;
                pNode->m_pNext = m_pHead ;   //insert before the head
                m_pHead = pNode;
                m_nCurLen ++;
                return true;

            }

            else
            {
                pTemp = m_pHead;
                while(pTemp->m_pNext !=     NULL)   //insert in the middle
                {
                    if((short)(pNode->m_nPts-pTemp->m_pNext->m_nPts)==0)
                        return false;
                    if((short)(pNode->m_nPts-pTemp->m_pNext->m_nPts)<0)
                    {
                        pNode->m_pNext = pTemp->m_pNext;
                        pTemp->m_pNext = pNode;
                        m_nCurLen ++;
                        return true;
                    }
                    else
                    {
                        pTemp = pTemp ->m_pNext;
                    }
                }

                if(pTemp == m_pTail)
                {
                    return false; //this should not happen
                }
            }
        }

    }
    return true;
}
