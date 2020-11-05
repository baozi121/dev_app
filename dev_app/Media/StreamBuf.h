/*****************************************************************************
 * Filename: StreamBuf.h
 * Description: Declaration of the class RTPClient
 *
 * Modify History:
 * Date: 2009.08.07      Author: Tony
 *
 ****************************************************************************/
#ifndef _STREAM_BUF_H
#define _STREAM_BUF_H

class StreamBufNode
{
    friend class StreamBuf;
    public:
    StreamBufNode();
    StreamBufNode(char *pData, int nLen,  unsigned short nPts, int nKey);
    ~StreamBufNode();
    char* GetData();
    int   GetLen();
    unsigned short   GetPts();
    bool  IsKey(){return m_nKey == 1;}
    StreamBufNode *GetNext(){return m_pNext;}

    private:
    /*
     * Declaring these member functions explicitly to prevent compilers from
     * generating their own version, and making the functions private to keep
     * people from calling it.
     * Do not defining these member functions to prevent member and friend functions
     * calling them.
     */
    StreamBufNode(const StreamBufNode &pStreamNode);
    StreamBufNode& operator = (const StreamBufNode &pStreamNode);
    StreamBufNode* operator&();
    const StreamBufNode* operator&() const;
    private:
    char  *m_pData;
    int   m_nLen;
    int   m_nKey;
    unsigned short   m_nPts;
    StreamBufNode *m_pNext;
};
/***********************************************************
 * Buffer Structure:
 *              []->[]->[]->[]->[]->...->[]
 *              /                         \
 *            head                        tail
 *
 * '[]' is a node of the buffer. This is a linked queue,
 * which is FIFO(First In First Out).
 *
 *
 ***********************************************************/
class StreamBuf
{
    public:
        StreamBuf();
        StreamBuf(const int nLen);
        StreamBuf(const int nLen, const int nReadyLen);
        ~StreamBuf();
        bool AddToBuf(StreamBufNode *pNode);
        StreamBufNode* GetFromBuf();
        bool IsEmpty();
        int GetCurLen();
        void play();
        //  bool IsReady();
        bool IsReady(){return m_bReady == true;}

        void SetReadyLen(const int nDelay);
        void ClearBuf();
        bool AddToBufBySeq(StreamBufNode *pNode);
    private:
        StreamBuf(const StreamBuf &pStreamBuf);
        StreamBuf& operator = (const StreamBuf &pStreamBuf);
        StreamBuf* operator&();
        const StreamBuf* operator&() const;
    public:
        //HANDLE    m_hBufEnough;   // Buffer enough before run the filter graph
    private:
        int   m_nLen;
        int   m_nCurLen;
        int   m_nReadyLen;
        int   m_nMinLen;
        bool  m_bReady;
        StreamBufNode *m_pHead;
        StreamBufNode *m_pTail;
};
#endif
