/******************************************************************************

          版权所有 (C), 2001-2020, 上海集光安防科技股份有限公司

 ******************************************************************************
  文 件 名   : NetSdkApp.cpp
  版 本 号   : 初稿
  作    者   : zdx
  生成日期   : 2016年9月30日
  最近修改   :
  功能描述   : 本文件用于实现local sdk的具体的功能的演示
  函数列表   :
              CNetSdkApp.AddPtzTourPreset
              CNetSdkApp.ClearPtzPreset
              CNetSdkApp.ClearPtzTourPreset
              CNetSdkApp.CNetSdkApp
              CNetSdkApp.DelPtzTourPreset
              CNetSdkApp.GetPtzPreset
              CNetSdkApp.GetPtzTour
              CNetSdkApp.GetRecordList
              CNetSdkApp.Help
              CNetSdkApp.OnAlarm
              CNetSdkApp.OnDefMsg
              CNetSdkApp.OnMonitor
              CNetSdkApp.OnRecord
              CNetSdkApp.OnSpeech
              CNetSdkApp.ProcRecord
              CNetSdkApp.PtzCtrl
              CNetSdkApp.SetPtzPreset
              CNetSdkApp.Start
              CNetSdkApp.Stop
              CNetSdkApp.Test
              CNetSdkApp.TestAlarm
              CNetSdkApp.OnAlarmEx
              CNetSdkApp.TestAlarmEx
              CNetSdkApp.TestConfig
              CNetSdkApp.TestMonitor
              CNetSdkApp.TestMonitor1
              CNetSdkApp.TestMonitor2
              CNetSdkApp.TestPtz
              CNetSdkApp.TestPtzUp
              CNetSdkApp.TestPtzDown
              CNetSdkApp.TestPtzLeft
              CNetSdkApp.TestPtzRight
              CNetSdkApp.TestRecord
              CNetSdkApp.TestSpeech
              CNetSdkApp.TestSpeech1
              CNetSdkApp.TestSpeech2
              CNetSdkApp.ThreadProc
              CNetSdkApp.~CNetSdkApp
              DumpJson
              GetFrameHeadLen
  修改历史   :
  1.日    期   : 2016年9月30日
    作    者   : zdx
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/



#include "NetSdkApp.h"
#include <stdio.h>
#include <unistd.h>
#include "/opt/jthread_jrtplib/include/SDk/AuxLibs/Json/json.h"
#include <string.h>
#include "/opt/jthread_jrtplib/include/SDk/AuxLibs/Json/value.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory.h>
#include <iostream>
//#ifdef WIN32
//    typedef Json::StyledWriter CConfigWriter;
//#else
    typedef Json::FastWriter CConfigWriter;
//#endif

typedef Json::Reader CConfigReader;



/*****************************************************************************
 函 数 名  : DumpJson
 功能描述  : 格式化输出json数据内容
 输入参数  : char *pData  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年9月30日
    作    者   : zdx
    修改内容   : 新生成函数

*****************************************************************************/
void DumpJson(char *pData, char *pCgiCmd)
{
    CConfigReader JsonReader;
    Json::Value table;
    bool ret = JsonReader.parse(pData,table);
    if (ret == false)
    {
        printf("DumpJson()>>>error:JsonReader.parse failed \r\n");
    }
        
    std::string stream = "";
    Json::StyledWriter JsonWtire(stream);
    JsonWtire.write(table);
    printf("\r\nDumpJson()>>>start!!!\r\n");
    if(pCgiCmd)
    {
        printf("[%s]:[%s]\r\n",pCgiCmd,stream.c_str());
    }
    else
    {
        printf("%s\r\n",stream.c_str());
    }
    printf("DumpJson()>>>end!!!\r\n\r\n");
}

/*****************************************************************************
 函 数 名  : ThreadBodyRecord
 功能描述  : 处理录像的线程体
 输入参数  : void *pdat  CNetSdkApp对象
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年10月8日
    作    者   : zdx
    修改内容   : 新生成函数

*****************************************************************************/
void ThreadBodyRecord(void *pdat)
{
    CNetSdkApp *pApp = (CNetSdkApp *)pdat;

    while(1)
    {
        pApp->ProcRecord(); 
        
        //演示用，这里控制回放数据的处理速度
        usleep(20*1000);
    }
        
}

/*****************************************************************************
 函 数 名  : CNetSdkApp.OnMonitor
 功能描述  : 视频回调函数
 输入参数  : int channel       视频通道号
     VF_FRAME_INFO*pInfo,码流信息
             int stream        码流类型
             char *pData            码流数据存放指针
             int iLen               码流数据长度
             unsigned int userdata               绑定时传入的用户数据，用于区分不同的对象
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年9月30日
    作    者   : zdx
    修改内容   : 新生成函数

*****************************************************************************/
void OnMonitor(int channel, int stream,VF_FRAME_INFO*pInfo,char *pData,int iLen,unsigned long userdata)
{
    if(pInfo->iFameType == VF_PACK_TYPE_FRAME_I)
    {
        printf("OnMonitor():ch[%d] stream[%d],iLen=%d iCodeType[%d] FrameType = %d,iFameLen[%d]"
            "iFamerate[%d] iWidth[%d] iHeight[%d]uiTime[%d]\r\n",
            channel,stream,iLen,pInfo->iCodeType,pInfo->iFameType,pInfo->iFameLen,
            pInfo->iFamerate,pInfo->iWidth,pInfo->iHeight,pInfo->uiTime);
    }
    else
    {
        printf("OnMonitor():ch[%d] stream[%d],iLen=%d iCodeType[%d]  FrameType = %d,iFameLen[%d]\r\n",
            channel,stream,iLen,pInfo->iCodeType,pInfo->iFameType,pInfo->iFameLen);
    }
}


/*****************************************************************************
 函 数 名  : CNetSdkApp.OnRecord
 功能描述  : 录像回放和下载的回调接口
 输入参数  : int channel       暂时无用
             int stream        暂时无用
             char *pData            码流数据存放指针
             int iLen               码流数据长度
             unsigned int userdata               绑定时传入的用户数据，用于区分不同的对象
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年9月30日
    作    者   : zdx
    修改内容   : 新生成函数

*****************************************************************************/
void OnRecord(int channel, int stream,VF_FRAME_INFO*pInfo,char *pData,int iLen,unsigned long userdata)
{
   //这里的channel和stream目前没有意义
    if( CNetSdkApp::instance()->m_iFileRecvlen/(2*1024*1024) != ((CNetSdkApp::instance()->m_iFileRecvlen + iLen)/(2*1024*1024)))
    {
        printf("CNetSdkApp::OnRecord():lenth = %d m_iFileRecvlen = %d m_iFileTotalLen = %d\r\n",
            iLen,CNetSdkApp::instance()->m_iFileRecvlen+iLen,CNetSdkApp::instance()->m_iFileTotalLen);
    }

    CNetSdkApp::instance()->m_iFileRecvlen += iLen;

    if( CNetSdkApp::instance()->m_packet_count > 100
        || CNetSdkApp::instance()->m_total_len > 1*1024*1024 )
    {
        pthread_mutex_lock((pthread_mutex_t *)CNetSdkApp::instance()->m_hMutex);  
        CNetSdkApp::PACKET_LIST::iterator it;
        for( it = CNetSdkApp::instance()->m_packet_list.begin(); it != CNetSdkApp::instance()->m_packet_list.end(); it++ )
        {
            if((*it).pData)
            {
                delete (*it).pData;
                (*it).pData = NULL;
                (*it).iLen= 0;
            }            
        }
        CNetSdkApp::instance()->m_packet_list.clear();
        CNetSdkApp::instance()->m_packet_count = 0;
        CNetSdkApp::instance()->m_total_len = 0;
        CNetSdkApp::instance()->m_packet_size = 0;
        pthread_mutex_unlock((pthread_mutex_t *)CNetSdkApp::instance()->m_hMutex);  

        return ;
    }

    NS_DataBuf data={0,0};

    data.iLen = iLen;
    if(iLen>0)
    {
        data.pData = new char[iLen];
        memcpy(data.pData,pData,iLen);
    }
    
    pthread_mutex_lock((pthread_mutex_t *)CNetSdkApp::instance()->m_hMutex);  
    CNetSdkApp::instance()->m_packet_list.push_back(data);
    CNetSdkApp::instance()->m_packet_count++;
    CNetSdkApp::instance()->m_total_len += iLen;
    CNetSdkApp::instance()->m_packet_size += iLen;
    pthread_mutex_unlock((pthread_mutex_t *)CNetSdkApp::instance()->m_hMutex);  

    //单个文件结束，会回调出来一个长度为0 的数据
    if (data.iLen == 0)
    {
        printf("CNetSdkApp::OnRecord():m_iFileRecvlen = %d m_iFileTotalLen = %d\r\n",
            CNetSdkApp::instance()->m_iFileRecvlen,CNetSdkApp::instance()->m_iFileTotalLen);
    }
}

/*****************************************************************************
 函 数 名  : CNetSdkApp.OnSpeech
 功能描述  : 对讲的回调接口
 输入参数  : int channel       暂时无用
             VF_FRAME_INFO*pInfo,码流信息
             int stream        暂时无用
             char *pData            码流数据存放指针
             int iLen               码流数据长度
             unsigned int userdata               绑定时传入的用户数据，用于区分不同的对象
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年9月30日
    作    者   : zdx
    修改内容   : 新生成函数

*****************************************************************************/
void OnSpeech(int channel, int stream,VF_FRAME_INFO*pInfo,char *pData,int iLen,unsigned long userdata)
{
//channel 和stream没有意义
//一个pPacket 帧为一帧数据，不带私有头
    printf("CNetSdkApp::OnSpeech():iLen = %d\r\n",iLen);

    //这里简单实现接收音频数据发送回去
    //具体实现时不建议在这个回调里面再调用sdk接口
    CNetSdkApp::instance()->m_pNS->SendSpeech((unsigned int)iLen,(unsigned char*)pData);
}

/*****************************************************************************
 函 数 名  : CNetSdkApp.OnAlarm
 功能描述  : 报警的回调接口
 输入参数  : int channel  报警的视频通道号或者编号
             uint type    报警类型
             int state    报警状态，1表示报警发生0表示报警恢复
    unsigned int userdata               绑定时传入的用户数据，用于区分不同的对象
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年9月30日
    作    者   : zdx
    修改内容   : 新生成函数

*****************************************************************************/
void OnAlarm(int channel, unsigned int type, int state,unsigned long userdata)
{
    printf("CNetSdkApp::OnAlarm():channel = %d type = %d state = %d\r\n",
        channel,type,state);

    //type ALARM_LOCAL

    switch(type)
    {
        case VF_ALARM_VIDEOLOSS://视频丢失
            break;
        case VF_ALARM_LOCAL://外部报警
            break;
        case VF_ALARM_MOTION://移动侦测
            break;
        case VF_ALARM_BLIND://遮挡告警
            break;
        case VF_STORAGE_LOWSPACE://无存储设备
            break;
        case VF_STORAGE_FAILURE://存储错误
            break;
        case VF_ALARM_DECODER://解码告警
            break;
        case VF_VIDEO_CROSSLINE://越线检测
            break;
        case VF_VIDEO_CROSSNET:
            break;
        case VF_STORAGE_NOT_EXIST:
            break;
        case VF_VIDEO_SCENE:
            break;
        case VF_VIDEO_FACE:
            break;
        case VF_VIDEO_ABANDON:
            break;
        case VF_VIDEO_FOCUS:
            break;
        case VF_VIDEO_COLORDVT:
            break;
        case VF_VIDEO_LEAVE:
            break;
        case VF_VIDEO_TRACE:
            break;
    }    
}

/*****************************************************************************
 函 数 名  : CNetSdkApp.OnDefMsg
 功能描述  : 连接状态的回调接口，主要用于异常处理，断线重连等逻辑
 输入参数  : enum DefEventCode code  
             unsigned int userdata               绑定时传入的用户数据，用于区分不同的对象
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年9月30日
    作    者   : zdx
    修改内容   : 新生成函数

*****************************************************************************/
void OnDefMsg(enum DefEventCode code, unsigned long userdata)
{
    printf("CNetSdkApp::OnDefMsg():code = %d userdata = %u\r\n",code,userdata);

    //录像链接断开
    if(code == DEC_Record_Sock_Disconnect)
    {
        CNetSdkApp::instance()->m_iRecordState = 2;//队列里面如果有数据还需要处理
        printf("CNetSdkApp::OnDefMsg():m_iRecordStop = 1 m_iFileRecvlen = %d m_iFileTotalLen = %d\r\n",
            CNetSdkApp::instance()->m_iFileRecvlen,CNetSdkApp::instance()->m_iFileTotalLen);
    }
}

/*****************************************************************************
 函 数 名  : CNetSdkApp.instance
 功能描述  : netsdkapp的单键实例化函数
 输入参数  : 无
 输出参数  : 无
 返 回 值  : CNetSdkApp*
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年10月8日
    作    者   : zdx
    修改内容   : 新生成函数

*****************************************************************************/
CNetSdkApp* CNetSdkApp::instance()
{
    static CNetSdkApp *pNetSdkApp = NULL;
    if (pNetSdkApp)
    {
        return pNetSdkApp;
    }
    
    pNetSdkApp = new CNetSdkApp;

    return pNetSdkApp;
}


/*****************************************************************************
 函 数 名  : CNetSdkApp.CNetSdkApp
 功能描述  : 应用构造函数
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年9月30日
    作    者   : zdx
    修改内容   : 新生成函数

*****************************************************************************/
CNetSdkApp::CNetSdkApp()
{
    m_iRecordState = 0;
    m_pNS = NULL;
    m_iPause = 0;
    m_packet_size = 0;
    m_total_len =0;
    m_packet_count =0;
    m_iFileTotalLen = 0;
    m_iFileRecvlen = 0;
    m_hThread= NULL;

    m_hMutex = 0;
    int ret;    
    pthread_mutex_t *lock;

    lock = new pthread_mutex_t;
    ret = pthread_mutex_init(lock, NULL);
	if( ret != 0 )
	{
		delete lock;
		m_hMutex = NULL;
	}
    else
    {
        m_hMutex = (void *)lock;
    }
    
}

/*****************************************************************************
 函 数 名  : CNetSdkApp.~CNetSdkApp
 功能描述  : 析构函数
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年9月30日
    作    者   : zdx
    修改内容   : 新生成函数

*****************************************************************************/
CNetSdkApp::~CNetSdkApp() 
{
    Stop();

    if(m_pNS)
    {
        delete m_pNS;
        m_pNS= NULL;
    }

    pthread_mutex_destroy((pthread_mutex_t *)m_hMutex);

    delete ((pthread_mutex_t*)m_hMutex);
    
}

/*****************************************************************************
 函 数 名  : CNetSdkApp.TestMonitor
 功能描述  : 视频测试函数
 输入参数  : 无
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年9月30日
    作    者   : zdx
    修改内容   : 新生成函数

*****************************************************************************/
void CNetSdkApp::TestMonitor1()
{
    printf("TestMonitor1():begin\r\n");

    if ( false == m_pNS->IsLogin())
    {
        if(0 != (m_pNS->Login()))
        {
            printf("CNetSdkApp::TestMonitor1()>>>login failed\r\n");
            return;
        }
    }

    m_pNS->StartMonitor(0,0);

    printf("---------[%s] ,[%d] \n",__FUNCTION__,__LINE__);
    usleep(10*1000*1000);
    
    m_pNS->StopMonitor(0,0);
    printf("TestMonitor1():end\r\n");
    return ;
}

void CNetSdkApp::TestMonitor2()
{
    printf("------22222222222---[%s] ,[%d] \n",__FUNCTION__,__LINE__);
    printf("TestConfig():begin\r\n");

    ILocalNetSDK *m_pNS=ILocalNetSDK::CreatNew();

    m_pNS->SetServerInfo((char *)"127.0.0.1",80);
    m_pNS->SetNamepasswd("admin","123456");

    m_pNS->AttachDefDelegate(OnDefMsg,(unsigned long)m_pNS);
    m_pNS->AttachDelegateMonitor(OnMonitor,(unsigned long)m_pNS);
    m_pNS->AttachDelegateAlarm(OnAlarm,(unsigned long)m_pNS);

    while(1)
    {
        if (0 == m_pNS->Login())
        {
            printf("-----OOOOOOO----[%s] ,[%d] \n",__FUNCTION__,__LINE__);
            m_pNS->StartMonitor(0,0);
            m_pNS->StartMonitor(2,1);
        }
        usleep(5*1000*1000);
        printf("-----CCCCCCCCCCC----[%s] ,[%d] \n",__FUNCTION__,__LINE__);
        
        m_pNS->StopMonitor(0,0);
        m_pNS->StopMonitor(2,1);
        usleep(2*1000*1000);
    }

    m_pNS->Logout();
    delete m_pNS;
    printf("TestConfig():end\r\n");
    
}



void CNetSdkApp::TestMonitor()
{
    printf("TestConfig():begin\r\n");

    ILocalNetSDK *m_pNS=ILocalNetSDK::CreatNew();

    m_pNS->SetServerInfo((char *)"127.0.0.1",80);
    m_pNS->SetNamepasswd("admin","123456");

    m_pNS->AttachDefDelegate(OnDefMsg,(unsigned long)m_pNS);
    m_pNS->AttachDelegateMonitor(OnMonitor,(unsigned long)m_pNS);
    m_pNS->AttachDelegateAlarm(OnAlarm,(unsigned long)m_pNS);

    if (0 == m_pNS->Login())
    {
        m_pNS->StartMonitor(0,0);
        m_pNS->StartMonitor(2,1);
    }

    usleep(10*1000*1000);
    m_pNS->StopMonitor(0,0);
    m_pNS->StopMonitor(2,1);
    
    m_pNS->Logout();
    delete m_pNS;
    printf("TestConfig():end\r\n");
    
}


/*****************************************************************************
 函 数 名  : CNetSdkApp.TestConfig
 功能描述  : 配置测试函数，这里主要测试了osd通道名称显示
 输入参数  : 无
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年9月30日
    作    者   : zdx
    修改内容   : 新生成函数

*****************************************************************************/
void CNetSdkApp::TestConfig()
{
    printf("TestConfig():begin\r\n");
    ILocalNetSDK *m_pNS=ILocalNetSDK::CreatNew();

    m_pNS->SetServerInfo((char *)"127.0.0.1",80);
    m_pNS->SetNamepasswd("admin","123456");

    //获取版本信息
    char aRecvBuf[2048]={0};
    int  iRecvLen = sizeof(aRecvBuf)-1;
    int  iResult = 0;
    m_pNS->ConfigOperation((char *)"merlin/GetDeviceInfo.cgi",NULL,aRecvBuf,iRecvLen,iResult);
    printf("TestConfig():GetDeviceInfo iResult = %d iRecvLen = %d aRecvBuf = \r\n",iResult,iRecvLen);
    DumpJson(aRecvBuf);

    if (iResult != 200 && iRecvLen <= 0)
    {
        printf("CNetSdkApp::TestConfig():iResult error");
        return;
    }
    
    //获取osd 配置参数
    memset(aRecvBuf,0,sizeof(aRecvBuf));
    iRecvLen = sizeof(aRecvBuf)-1;
    iResult = 0;
    m_pNS->ConfigOperation((char *)"merlin/GetOsdCfg.cgi?chstart=0&chnum=1",NULL,aRecvBuf,iRecvLen,iResult);
    printf("TestConfig():GetOsdCfg iResult = %d iRecvLen = %d aRecvBuf = \r\n",iResult,iRecvLen);
    DumpJson(aRecvBuf);

    if (iResult != 200 && iRecvLen <= 0)
    {
        printf("CNetSdkApp::TestConfig():iResult error");
        delete m_pNS;
        return;
    }
    
    CConfigReader JsonReader;
    Json::Value table;
    bool ret = JsonReader.parse(aRecvBuf,table);
    if (ret == false)
    {
        printf("TestConfig()::error:JsonReader.parse failed \r\n");
    }
    
//设置osd配置参数    
    //通道名称叠加使能
    int enable=0;
    GetCgiJSValue_Int(table["OSDCfg"][0u]["title"],"enable",enable);
    table["OSDCfg"][0u]["title"]["enable"] = 1-enable;
    
    std::string stream = "";
    CConfigWriter JsonWtire(stream);
    JsonWtire.write(table);

    iRecvLen = 0;

    printf("TestConfig():SetOsdCfg data:\r\n");
    DumpJson((char *)stream.c_str());

    m_pNS->ConfigOperation((char *)"merlin/SetOsdCfg.cgi",(char *)stream.c_str(),NULL,iRecvLen,iResult);
    printf("TestConfig(): SetOsdCfg iResult = %d\r\n",iResult);
    delete m_pNS;
    printf("TestConfig():end\r\n");    
}

/*****************************************************************************
 函 数 名  : CNetSdkApp.GetRecordList
 功能描述  : 获取录像列表
 输入参数  : ILocalNetSDK *m_pNS  
             int iCh              
             NS_FILE_INFO*pFile   
             int &iNum            
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年9月30日
    作    者   : zdx
    修改内容   : 新生成函数

*****************************************************************************/
int CNetSdkApp::GetRecordList(ILocalNetSDK *m_pNS,int iCh,NS_FILE_INFO*pFile,int &iNum)
{
    //查询录像列表
    char recvData[1000*2048]={0};
    int iMaxNum = iNum;
    iNum = 0;
    
    char URL[256];
    memset(URL,0,sizeof(URL));
    
    sprintf(URL,"merlin/QueryRecord.cgi?begintime=20160929-020000&endtime=20300929-235959"
        "&cameraid=0$%d&pic=0&type=1&stream=0",iCh);
    
    int  iRecvLen = sizeof(recvData)-1;
    int  iResult = 0;
    m_pNS->ConfigOperation((char *)URL,NULL,recvData,iRecvLen,iResult);
    printf("CNetSdkApp::GetRecordList():GetRecordList iResult = %d iRecvLen = %d aRecvBuf = \r\n",iResult,iRecvLen);
    DumpJson(recvData);

    if (iResult != 200 && iRecvLen <= 0)
    {
        printf("CNetSdkApp::GetRecordList():iResult error [%d]\r\n",iResult);
        return -1;
    }

    CConfigReader JsonReader;
    Json::Value m_table;
    bool ret = JsonReader.parse(recvData,m_table);
    if (ret == false)
    {
        printf("CNetSdkApp::GetRecordList():JsonReader.parse failed \r\n");
        return -1;
    }

    if (!m_table.isMember("RecordList"))
    {
        iNum = 0;
        return 0;
    }
            
    Json::Value *pTable=NULL;

// "Cl" : 122, "Di" : 0, "Pa" : 0, "ch" : 0, "et" : "20151109-154000", "l" : 155733, "num" : 1, "st" : "20151109-153500", "t" : 6         
    for (int i=0;i<(int)m_table["RecordList"].size();i++)
    {
        if (iNum>=iMaxNum)
        {
            printf("iNum>=iMaxNum[%d]\r\n",iMaxNum);
            return 0;
        }
        
        memset(&pFile[i],0,sizeof(NS_FILE_INFO));

        pFile[i].ucFileType = 0;
        pFile[i].ucSubType = 0;

        int ch=0,ty=0;
        
        pTable=&m_table["RecordList"][i];

        const char *st = GetCgiJSValue_CString2(*pTable,"st");
        if (st)
        {
            sscanf(st, "%04d%02d%02d-%02d%02d%02d",&(pFile[i].start_time.year),
            &(pFile[i].start_time.month),
            &(pFile[i].start_time.day),
            &(pFile[i].start_time.hour),
            &(pFile[i].start_time.minute),
            &(pFile[i].start_time.second));
        }

        const char *et = GetCgiJSValue_CString2(*pTable,"et");
        if (st)
        {
            sscanf(et, "%04d%02d%02d-%02d%02d%02d",&(pFile[i].end_time.year),
            &(pFile[i].end_time.month),
            &(pFile[i].end_time.day),
            &(pFile[i].end_time.hour),
            &(pFile[i].end_time.minute),
            &(pFile[i].end_time.second));
        }
        
        GetCgiJSValue_Int((*pTable),"t",ty);
        pFile[i].ucSubType = ty;//参考枚举CMS_RECRORD_TYPE
        
        GetCgiJSValue_UInt((*pTable),"Cl",pFile[i].first_clus_no);
        GetCgiJSValue_UInt((*pTable),"Di",pFile[i].disk_no);
        GetCgiJSValue_UInt((*pTable),"Pa",pFile[i].disk_part);

        ch = pFile->channel;
        GetCgiJSValue_Int((*pTable),"ch",ch);
        pFile->channel = ch;
        
        GetCgiJSValue_UInt((*pTable),"l",pFile[i].file_length);

        iNum++;            
    }    

    printf("CNetSdkApp::GetRecordList():iNum = %d\r\n",iNum);
    
    return 0;
}

/*****************************************************************************
 函 数 名  : CNetSdkApp.TestRecord
 功能描述  : 录像回放下载测试
 输入参数  : 无
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年9月30日
    作    者   : zdx
    修改内容   : 新生成函数

*****************************************************************************/
void CNetSdkApp::TestRecord()
{
    NS_FILE_INFO file[1024];
    int iNum = 1024;
    int iCh = 2;
    int ii=0;

    if (m_iRecordState >= 1)
    {
        printf("CNetSdkApp::TestRecord():test is running");
        return;
    }
    
    printf("CNetSdkApp::TestRecord():test begin");
    m_pNS->AttachDelegateRecord(OnRecord,(unsigned long)m_pNS);

    if ( 0 == m_pNS->Login())
    {
        printf("TestRecord():login sucess\r\n");
    }
    else
    {
        printf("TestRecord():login failed\r\n");
        return;
    }

    if(GetRecordList(m_pNS,iCh,file,iNum)<0)
    {
        printf("GetRecordList() failed\r\n");
        return;
    }
    
    if (iNum <= 0)
    {
        printf("TestRecord(): no record find\r\n");
        return;
    }

    while(ii<iNum)
    {
        printf("file[%d].file_length = %d\r\n",ii,file[ii].file_length);
        ii++;
    }
        
    m_iFileTotalLen = file[0].file_length*1024;
    m_iFileRecvlen = 0;

    m_iRecordState = 1;
    printf("TestRecord(): now StartRecord\r\n");
    usleep(1000*1000);

    if (m_pNS->StartRecord(iCh,&file[0]) < 0)
    {    
        m_iRecordState = 0;
        printf("TestRecord(): StartRecord failed\r\n");
        return;
    }

    while(m_iRecordState)
    {
        usleep(1000*1000);
    }

    printf("CNetSdkApp::TestRecord():test end\r\n");
    
}

/*****************************************************************************
 函 数 名  : CNetSdkApp.TestAlarm
 功能描述  : 报警测试，实际上登陆后设备如果有报警会自动上报，进入相应的回调
             接口，不需要单独订阅
 输入参数  : 无
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年9月30日
    作    者   : zdx
    修改内容   : 新生成函数

*****************************************************************************/
void CNetSdkApp::TestAlarm()
{
    if ( 0 == m_pNS->Login())
    {
        printf("TestRecord():login sucess\r\n");
    }
    else
    {
        printf("TestRecord():login failed\r\n");
        return;
    }

    printf("----Please wait alarm\n");
}


void CNetSdkApp::TestAlarmOut()
{
	printf("TestConfig():begin\r\n");
	ILocalNetSDK *m_pNS=ILocalNetSDK::CreatNew();

    m_pNS->SetServerInfo((char *)"127.0.0.1",80);
    m_pNS->SetNamepasswd("admin","123456");

	//获取版本信息
	char aRecvBuf[2048]={0};
	int  iRecvLen = sizeof(aRecvBuf)-1;
	int  iResult = 0;

	
    CConfigReader JsonReader;
    Json::Value table;
    //设置编码配置参数 
    //可以设置的值在GetEncodeAbility.cgi获取的能力集里
    table["AlarmOutCfg"][0u]["index"] = 0;
    table["AlarmOutCfg"][0u]["mode"] = 1;
        std::string stream = "";
    CConfigWriter JsonWtire(stream);
    JsonWtire.write(table);
    DumpJson((char *)stream.c_str(),(char *)"SetAlarmOutCfg.cgi");	
	
	m_pNS->ConfigOperation((char *)"merlin/SetAlarmOutCfg.cgi",(char *)stream.c_str(),aRecvBuf,iRecvLen,iResult);
	printf("TestConfig():TestAlarmOut iResult = %d iRecvLen = %d aRecvBuf = \r\n",iResult,iRecvLen);
	DumpJson(aRecvBuf);

	delete m_pNS;
	printf("TestConfig():end\r\n");    
}


void CNetSdkApp::TestAlarmIn()
{
	printf("TestConfig():begin\r\n");
	ILocalNetSDK *m_pNS=ILocalNetSDK::CreatNew();

    m_pNS->SetServerInfo((char *)"127.0.0.1",80);
    m_pNS->SetNamepasswd("admin","123456");

	//获取版本信息
	char aRecvBuf[2048]={0};
	int  iRecvLen = sizeof(aRecvBuf)-1;
	int  iResult = 0;

	
    CConfigReader JsonReader;
    Json::Value table;
    //设置编码配置参数 
    //可以设置的值在GetEncodeAbility.cgi获取的能力集里
    table["AlarmCfg"][0u]["Channel"] = 0;
    table["AlarmCfg"][0u]["Type"] = "Alarm";
    table["AlarmCfg"][0u]["Enable"] = 1;
	table["AlarmCfg"][0u]["SensorType"] = 1;

	table["AlarmCfg"][0u]["EventHandler"]["Iinkage"]=0;
	table["AlarmCfg"][0u]["EventHandler"]["RecordCh"][0u]=0;					
	table["AlarmCfg"][0u]["EventHandler"]["RecordEn"]=1;
	table["AlarmCfg"][0u]["EventHandler"]["RecordLatch"]=10;
	table["AlarmCfg"][0u]["EventHandler"]["EventLatch"][0u]=0;
	table["AlarmCfg"][0u]["EventHandler"]["AlarmOutEn"]=1;
	table["AlarmCfg"][0u]["EventHandler"]["AOLatch"]=10;
	table["AlarmCfg"][0u]["EventHandler"]["AlarmOutCh"][0u]=0;	
	
	table["AlarmCfg"][0u]["EventHandler"]["Mail"]=0;					
	table["AlarmCfg"][0u]["EventHandler"]["SnapEn"]=0;					
	table["AlarmCfg"][0u]["EventHandler"]["SnapCh"]=1;					
	table["AlarmCfg"][0u]["EventHandler"]["Tip"]=0;					

	table["AlarmCfg"][0u]["EventHandler"]["Beep"]=0;					
	table["AlarmCfg"][0u]["EventHandler"]["TourEn"]=0;					
	table["AlarmCfg"][0u]["EventHandler"]["TourCh"][0u]=1;					
	table["AlarmCfg"][0u]["EventHandler"]["MessageEn"]=0;					
	table["AlarmCfg"][0u]["Sector"][0u] = "1 00:00:00-24:00:00";
	
	table["AlarmCfg"][0u]["Sector"][0u] = "1 00:00:00-24:00:00";
	table["AlarmCfg"][0u]["Sector"][1u] = "1 00:00:00-24:00:00";
	table["AlarmCfg"][0u]["Sector"][2u] = "1 00:00:00-24:00:00";
	table["AlarmCfg"][0u]["Sector"][3u] = "1 00:00:00-24:00:00";
	table["AlarmCfg"][0u]["Sector"][4u] = "1 00:00:00-24:00:00";
	
	table["AlarmCfg"][0u]["Sector"][5u] = "1 00:00:00-24:00:00";

	std::string stream = "";
    CConfigWriter JsonWtire(stream);
    JsonWtire.write(table);
    DumpJson((char *)stream.c_str(),(char *)"SetAlarmCfg.cgi");	
	
	m_pNS->ConfigOperation((char *)"merlin/SetAlarmCfg.cgi",(char *)stream.c_str(),aRecvBuf,iRecvLen,iResult);
	printf("TestConfig():SetAlarmCfg iResult = %d iRecvLen = %d aRecvBuf = \r\n",iResult,iRecvLen);
	DumpJson(aRecvBuf);

	delete m_pNS;
	printf("TestConfig():end\r\n");    
}


void CNetSdkApp::TestSetWIFI()
{
    int ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == ServerSocket)                 
    {
		printf( " creat ServerSocket error\n"); 
		//return -1;
    }
    printf("靠靠靠靠靠!\n");
    
    //靠靠靠靠靠靠靠
    sockaddr_in ServerAddress;     //靠靠靠靠靠靠
    memset(&ServerAddress, 0, sizeof(sockaddr_in));  //靠靠靠靠靠靠
    ServerAddress.sin_family = AF_INET;    //靠靠靠,IPV4靠
    ServerAddress.sin_port = htons(1234);  //靠
   
	ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	printf("IP靠靠縗n");
	
  
   
    //靠靠靠靠靠靠靠靠靠
    int bindd = bind(ServerSocket, (sockaddr *)&ServerAddress, sizeof(sockaddr_in));
    if(bindd == -1)                 //靠靠靠靠靠靠靠靠靠
    {
		printf("bind error\n" );
		close(ServerSocket);
		//return -1;
    }
	printf("靠靠靠靠靠靠靠靠靠靠\n");
    //listen靠靠靠靠靠靠靠靠靠靠靠靠靠靠靠靠靠靠靠靠靠靠靠靠�
    //int lis = listen(ServerSocket, LengthOfQueueOfListen);  
	int lis = listen(ServerSocket, 10); 	
    if(lis == -1)             //靠靠靠靠靠靠靠靠靠
    {
		printf("listen error\n"); 
		close(ServerSocket);
		//return -1;
    }
    printf("靠靠\n");
    //靠靠靠靠靠靠靠靠靠靠靠靠靠靠靠靠靠縤p靠靠靠靠
    sockaddr_in ClientAddress;   //靠靠靠靠靠靠靠靠靠靠靠�
    socklen_t LengthOfClientAddress = sizeof(sockaddr_in);
	
    printf( "prepare to connect client.....\n") ;
    //靠靠靠靠靠靠靠靠靠靠,靠靠靠靠靠靠靠靠靠靠靠靠靠靠靠
    int ConnectedSocket = accept(ServerSocket, (sockaddr *)&ClientAddress, &LengthOfClientAddress);  
    if(-1 == ConnectedSocket)    //靠靠靠靠靠靠靠靠靠靠�
    {
		printf( "accept error\n");
		close(ServerSocket);
		//return -1;
    }
    else
    {
        printf("connect client success!\n");
    }
	
	char temp[50]={0},pwd[15]={0},userId[20]={0};//靠靠�
    //靠靠靠靠靠靠靠靠靠靠靠�
	int count;
	read(ConnectedSocket, temp, sizeof(temp));
	printf("from the client message is: %s\n",temp);
			
	for(int i=0;i<strlen(temp);i++)
	{
		if(temp[i]=='+')
		{
			count=i; 
		}
	}
	printf("count=%d\n",count);
	for(int i=0;i<count;i++)
	{
		userId[i]=temp[i];
	}
	printf("temp=%s\n",temp);
	const char* USERID=userId;
	printf("the userId is�%s\n",userId);
	printf("the USERID is�%s\n",USERID);
	int i=0;
	for(int j=count+1;j<strlen(temp);j++)
	{
		pwd[i]=temp[j];
		i++;
	}
	const char* PWD=pwd;
	printf("the pwd is�%s\n",pwd);
	printf("the PWD is�%s\n",PWD);
	
    //靠靠靠靠靠靠靠靠靠�
    close(ConnectedSocket);
    close(ServerSocket);
    printf( "server closed \r\n");
	printf("TestWIFI():begin\r\n");
	ILocalNetSDK *m_pNS=ILocalNetSDK::CreatNew();

    m_pNS->SetServerInfo((char *)"127.0.0.1",80);
    m_pNS->SetNamepasswd("admin","123456");

	char aRecvBuf[2048]={0};
	int  iRecvLen = sizeof(aRecvBuf)-1;
	int  iResult = 0;
	
    CConfigReader JsonReader;
    Json::Value table;

	//模式 "WIFI","AP"
    table["WifiCfg"]["WorkMode"] = "WIFI";
    //wifi开关，是否打开wifi
    table["WifiCfg"]["ENABLE"] = 1;
    
    table["WifiCfg"]["DHCP"] = 1;
    table["WifiCfg"]["SSID"] = USERID;
	//"NONE","WEP","WPA","WPA2","WPA/WPA2"
    table["WifiCfg"]["Encrypt"] = "WPA2";
    table["WifiCfg"]["Password"] = PWD;

	//DHCP 未打开，需要设置IP
    if(1)    
    {
        table["WifiCfg"]["IP"] = "192.168.1.108";        
        table["WifiCfg"]["NetMask"] = "255.255.0.0"; 
        table["WifiCfg"]["Gateway"] = "192.168.1.1"; 
    }
	
    std::string stream = "";
    CConfigWriter JsonWtire(stream);
    JsonWtire.write(table);
    DumpJson((char *)stream.c_str(),(char *)"SetWifiCfg.cgi");	
	
	m_pNS->ConfigOperation((char *)"merlin/SetWifiCfg.cgi",(char *)stream.c_str(),aRecvBuf,iRecvLen,iResult);
	printf("TestConfig():SetWifiCfg iResult = %d iRecvLen = %d aRecvBuf = \r\n",iResult,iRecvLen);
	DumpJson(aRecvBuf);

	delete m_pNS;
	printf("TestConfig():end\r\n");    
}


void CNetSdkApp::TestGetWIFI()
{
	printf("TestWIFI():begin\r\n");
	ILocalNetSDK *m_pNS=ILocalNetSDK::CreatNew();

    m_pNS->SetServerInfo((char *)"127.0.0.1",80);
    m_pNS->SetNamepasswd("admin","123456");

    //获取wifi信息
    char aRecvBuf[2048]={0};
    int  iRecvLen = sizeof(aRecvBuf)-1;
    int  iResult = 0;
    m_pNS->ConfigOperation((char *)"merlin/GetWifiCfg.cgi",NULL,aRecvBuf,iRecvLen,iResult);
    printf("TestConfig():GetWifiCfg iResult = %d iRecvLen = %d aRecvBuf = \r\n",iResult,iRecvLen);
    DumpJson(aRecvBuf);

    if (iResult != 200 && iRecvLen <= 0)
    {
        printf("CNetSdkApp::TestConfig():iResult error");
    }

	delete m_pNS;
		printf("--------------靠wifi靠靠--------------\n");
	CConfigReader JsonReader;
    Json::Value table;
	char tmpComp[64] = {0};
    bool ret = JsonReader.parse(aRecvBuf,table);
    if (ret == false)
    {
        printf("CNetSdkApp::TestGetWIFI()>>>error:JsonReader.parse failed \r\n");
    }
	GetCgiJSValue_CString(table["WifiCfg"],"WorkMode",tmpComp,sizeof(tmpComp));
    printf("CNetSdkApp::TestGetWIFI()>>>tmpComp=[%s]\r\n",tmpComp);
	printf("tmpComp=%s\n",tmpComp);
	printf("------------靠靠靠-------------\n");
	if(strstr(tmpComp,"AP"))
	{
		printf("------------Now is AP type-----------\n");
		printf("----------- begin set wifi ----------\n");
		TestSetWIFI();
	}
	else
	{
		printf("-----------Now is wifi type-----------\n");
	}
	printf("TestConfig():end\r\n");    
}

/*****************************************************************************
 函 数 名  : CNetSdkApp.TestFeederPTZ
 功能描述  : 喂食器方向控制
 输入参数  : 无
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
*****************************************************************************/
void CNetSdkApp::TestFeederPTZ(int position)
{
	printf("TestFeederPTZ():begin\r\n");
	ILocalNetSDK *m_pNS=ILocalNetSDK::CreatNew();

    m_pNS->SetServerInfo((char *)"127.0.0.1",80);
    m_pNS->SetNamepasswd("admin","123456");

    char URL[256];
    memset(URL,0,sizeof(URL));
	
    int direction = 1;
    int action = 4;
   // int position = 0;
	//方向 动作 目标位置
    sprintf(URL,"merlin/PTSPtzCtrl.cgi?direction=%d&action=%d&position=%d",direction,action,position);

    printf("CNetSdkApp::PTSPtzCtrl():URL = %s\r\n",URL);
    
    int iResult = 0;
    int  iRecvLen = 0;    
    m_pNS->ConfigOperation((char *)URL,NULL,NULL,iRecvLen,iResult);

	delete m_pNS;
	printf("TestConfig():end\r\n");    
}

/*****************************************************************************
 函 数 名  : CNetSdkApp.TestFeederWeigh
 功能描述  : 称重
 输入参数  : 无
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
*****************************************************************************/
int CNetSdkApp::TestFeederWeigh()
{
	printf("TestFeederPTZ():begin\r\n");
	ILocalNetSDK *m_pNS=ILocalNetSDK::CreatNew();

    m_pNS->SetServerInfo((char *)"127.0.0.1",80);
    m_pNS->SetNamepasswd("admin","123456");
    int Weigh; 
    char URL[256];
    memset(URL,0,sizeof(URL));
	
    int action = 1;
    sprintf(URL,"merlin/PTSWeigh.cgi?action=%d",action);

    printf("CNetSdkApp::PTSWeigh():URL = %s\r\n",URL);
    
    char aRecvBuf[2048]={0};
    int  iRecvLen = sizeof(aRecvBuf)-1;
    int  iResult = 0;   
    m_pNS->ConfigOperation((char *)URL,NULL,aRecvBuf,iRecvLen,iResult);
    printf("TestConfig():PTSWeigh iResult = %d iRecvLen = %d aRecvBuf = %s\r\n",iResult,iRecvLen,aRecvBuf);
    DumpJson(aRecvBuf);
//get weight from json
CConfigReader JsonReader;
    Json::Value table;
    bool ret = JsonReader.parse(aRecvBuf,table);
 if (ret == false)
    {
        printf("CNetSdkApp::TestConfigEncode()>>>error:JsonReader.parse failed \r\n");
    }

    if (iResult != 200 && iRecvLen <= 0)
    {
        printf("CNetSdkApp::TestConfig():iResult error");
    }
GetCgiJSValue_Int(table["FeederCfg"],"Weigh",Weigh);

	delete m_pNS;
	printf("TestConfig():end\r\n");   
	return Weigh; 
}

/*****************************************************************************
 函 数 名  : CNetSdkApp.TestGetFeeder
 功能描述  : 设置喂食器参数
 输入参数  : 无
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
*****************************************************************************/
void CNetSdkApp::TestSetFeeder()
{
	printf("TestSetFeeder():begin\r\n");
	ILocalNetSDK *m_pNS=ILocalNetSDK::CreatNew();
    char aRecvBuf[2048]={0};
    int  iRecvLen = sizeof(aRecvBuf)-1;
    int  iResult = 0;

    m_pNS->SetServerInfo((char *)"127.0.0.1",80);
    m_pNS->SetNamepasswd("admin","123456");
		
	m_pNS->ConfigOperation((char *)"/merlin/PTSSetFeeder.cgi?action=1&value=256",NULL,aRecvBuf,iRecvLen,iResult);
	printf("TestConfig():SetWifiCfg iResult = %d iRecvLen = %d aRecvBuf = \r\n",iResult,iRecvLen);
	DumpJson(aRecvBuf);

	delete m_pNS;
	printf("TestConfig():end\r\n");    
}

/*****************************************************************************
 函 数 名  : CNetSdkApp.TestGetFeeder
 功能描述  : 获取喂食器参数
 输入参数  : 无
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
*****************************************************************************/
void CNetSdkApp::TestGetFeeder()
{
	printf("TestGetFeeder():begin\r\n");
	ILocalNetSDK *m_pNS=ILocalNetSDK::CreatNew();

    m_pNS->SetServerInfo((char *)"127.0.0.1",80);
    m_pNS->SetNamepasswd("admin","123456");

    //获取wifi信息
    char aRecvBuf[2048]={0};
    int  iRecvLen = sizeof(aRecvBuf)-1;
    int  iResult = 0;
    m_pNS->ConfigOperation((char *)"merlin/PTSGetFeeder.cgi",NULL,aRecvBuf,iRecvLen,iResult);
    printf("TestConfig():PTSGetFeeder iResult = %d iRecvLen = %d aRecvBuf = \r\n",iResult,iRecvLen);
    DumpJson(aRecvBuf);

    if (iResult != 200 && iRecvLen <= 0)
    {
        printf("CNetSdkApp::TestConfig():iResult error");
    }

	delete m_pNS;
	printf("TestConfig():end\r\n");    
}


/*****************************************************************************
 函 数 名  : CNetSdkApp.TestGetRtmp
 功能描述  : RTMP配置
 输入参数  : 无
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
*****************************************************************************/
void CNetSdkApp::TestGetRtmp()
{
	printf("TestGetRtmp():begin\r\n");
	ILocalNetSDK *m_pNS=ILocalNetSDK::CreatNew();
    char aRecvBuf[2048]={0};
    int  iRecvLen = sizeof(aRecvBuf)-1;
    int  iResult = 0;

    m_pNS->SetServerInfo((char *)"127.0.0.1",80);
    m_pNS->SetNamepasswd("admin","123456");
		
	m_pNS->ConfigOperation((char *)"/merlin/GetXvrConfig2.cgi?name=RtmpPublish",NULL,aRecvBuf,iRecvLen,iResult);
	printf("TestConfig():GetRtmpCfg iResult = %d iRecvLen = %d aRecvBuf = \r\n",iResult,iRecvLen);
	DumpJson(aRecvBuf);

	delete m_pNS;
	printf("TestConfig():end\r\n");    
}

/*****************************************************************************
 函 数 名  : CNetSdkApp.TestGetFeeder
 功能描述  : RTMP设置
 输入参数  : 无
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
*****************************************************************************/
void CNetSdkApp::TestSetRtmp(const char *rtmp_addr)
{
	printf("TestSetRtmp():begin\r\n");
	ILocalNetSDK *m_pNS=ILocalNetSDK::CreatNew();

    m_pNS->SetServerInfo((char *)"127.0.0.1",80);
    m_pNS->SetNamepasswd("admin","123456");

    //获取wifi信息
    char aRecvBuf[2048]={0};
    int  iRecvLen = sizeof(aRecvBuf)-1;
    int  iResult = 0;
    CConfigReader JsonReader;
    Json::Value table;

    table["RtmpPublish"][0u]["Channel"] = 0;
    table["RtmpPublish"][0u]["Enable"] = 1;
    table["RtmpPublish"][0u]["Stream"] = 0;//main = 0 ;;fuzhu =1
    table["RtmpPublish"][0u]["Url"] = rtmp_addr;
	
    std::string stream = "";
    CConfigWriter JsonWtire(stream);
    JsonWtire.write(table);
    DumpJson((char *)stream.c_str(),(char *)"SetXvrConfig2.cgi");	
	
	m_pNS->ConfigOperation((char *)"/merlin/SetXvrConfig2.cgi",(char *)stream.c_str(),aRecvBuf,iRecvLen,iResult);
	printf("TestConfig():TestSetRtmp iResult = %d iRecvLen = %d aRecvBuf = \r\n",iResult,iRecvLen);
	DumpJson(aRecvBuf);

	delete m_pNS;
	printf("TestConfig():end\r\n");    
}

/*****************************************************************************
 函 数 名  : CNetSdkApp.TestAlarm
 功能描述  : 报警测试，实际上登陆后设备如果有报警会自动上报，进入相应的回调
             接口，不需要单独订阅
 输入参数  : 无
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年9月30日
    作    者   : zdx
    修改内容   : 新生成函数

*****************************************************************************/
void OnAlarmEx(std::string type, int index, LocalSdkEventAction action, const Json::Value & table, void* exparam, unsigned long userdata)
{
    std::string stream = "";
    CConfigWriter JsonWtire(stream);
    JsonWtire.write(table);
    DumpJson((char *)stream.c_str());

	if(NULL != exparam)
	{
		NS_ALARM_DATA *pdata = (NS_ALARM_DATA *)exparam;	
		int num = pdata->PicNum;

		for(int iIndex=0;iIndex < num;iIndex++)
		{
			int len = pdata->PicData[iIndex].DataLen;
			char *pname = pdata->PicData[iIndex].name;
			printf("wpl @@@@@@@@@@@@@@@@@@[%d][%s]\n",len,pname);
			printf("wpl @@@@@@@@@[0x%x][0x%x][0x%x][0x%x]\n",pdata->PicData[iIndex].pData[0],pdata->PicData[iIndex].pData[1]
				,pdata->PicData[iIndex].pData[len-2],pdata->PicData[iIndex].pData[len-1]);
		}
	}
	
	return ;
}

void CNetSdkApp::TestAlarmEx()
{
    printf("TestConfig():begin\r\n");
    ILocalNetSDK *m_pNS=ILocalNetSDK::CreatNew();

    m_pNS->SetServerInfo((char *)"192.168.0.252",80);
    m_pNS->SetNamepasswd("admin","123456");

    #if 0
    m_pNS->StartAlarmEx(OnAlarmEx,"faceCheck",1,(unsigned int)m_pNS);
    m_pNS->StartAlarmEx(OnAlarmEx,"faceIdentify",1,(unsigned int)m_pNS);
    m_pNS->StartAlarmEx(OnAlarmEx,"faceCompare",1,(unsigned int)m_pNS);
	m_pNS->StartAlarmEx(OnAlarmEx,"pedestrian",1,(unsigned int)m_pNS);
#else
	m_pNS->StartAlarmEx(OnAlarmEx,"faceCheck&faceIdentify&faceCompare&pedestrian",1,(unsigned long)m_pNS);
#endif


    printf("----Please wait alarm\n");
} 
/*****************************************************************************
 函 数 名  : CNetSdkApp.TestSpeech
 功能描述  : 对讲测试函数，这里实现的功能是把设备发送过来的音频数据直接发送
             回设备
 输入参数  : 无
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年9月30日
    作    者   : zdx
    修改内容   : 新生成函数

*****************************************************************************/
void CNetSdkApp::TestSpeech()
{
    printf("CNetSdkApp::TestSpeech():test end\r\n");
    
    if ( 0 == m_pNS->Login())
    {
        printf("TestSpeech():login sucess\r\n");
    }
    else
    {
        printf("TestSpeech():login failed\r\n");
        return;
    }

    m_pNS->AttachDelegateSpeech(OnSpeech,(unsigned long)m_pNS);

    if (m_pNS->StartSpeech() < 0)
    {    
        printf("TestSpeech(): TestSpeech failed\r\n");
        return;
    }

    usleep(20*1000*1000);
    m_pNS->StopSpeech();
    printf("CNetSdkApp::TestSpeech():test end\r\n");
}

void CNetSdkApp::TestSpeech1()
{
    printf("CNetSdkApp::TestSpeech1111111():test end\r\n");
    
    if ( 0 == m_pNS->Login())
    {
        printf("TestSpeech():login sucess\r\n");
    }
    else
    {
        printf("TestSpeech():login failed\r\n");
        return;
    }

    m_pNS->AttachDelegateSpeech(OnSpeech,(unsigned long)m_pNS);

    if (m_pNS->StartSpeech() < 0)
    {    
        printf("TestSpeech(): TestSpeech failed\r\n");
        return;
    }

    while(1)
    {
        printf("---------[%s] ,[%d] \n",__FUNCTION__,__LINE__);
        usleep(1*1000*1000);
    }

    usleep(20*1000*1000);
    m_pNS->StopSpeech();
    printf("CNetSdkApp::TestSpeech():test end\r\n");
}

void CNetSdkApp::TestSpeech2()
{
    printf("CNetSdkApp::TestSpeech2222222():test end\r\n");
    
    if ( 0 == m_pNS->Login())
    {
        printf("TestSpeech():login sucess\r\n");
    }
    else
    {
        printf("TestSpeech():login failed\r\n");
        return;
    }

    m_pNS->AttachDelegateSpeech(OnSpeech,(unsigned long)m_pNS);

    while(1)
    {
        printf("------OPEN---[%s] ,[%d] \n",__FUNCTION__,__LINE__);
        if (m_pNS->StartSpeech() < 0)
        {    
            printf("TestSpeech(): TestSpeech failed\r\n");
            return;
        }
        usleep(5*1000*1000);
        
        printf("------STOP---[%s] ,[%d] \n",__FUNCTION__,__LINE__);
        m_pNS->StopSpeech();
        usleep(2*1000*1000);
    }


    printf("CNetSdkApp::TestSpeech():test end\r\n");
}


/*****************************************************************************
 函 数 名  : CNetSdkApp.PtzCtrl
 功能描述  : 云台控制
 输入参数  : int iCh     通道
             int iCmd    参考VF_PTZ_TYPE
             int iSpeed  移动速度等
             int iValue  预置点或循迹等的编号
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年9月30日
    作    者   : 请求包信息不处理，默认成功
    修改内容   : 新生成函数

*****************************************************************************/
void CNetSdkApp::PtzCtrl(int iCh,int iCmd,int iSpeed,int iValue)
{
    char URL[256];
    memset(URL,0,sizeof(URL));
    sprintf(URL,"merlin/PtzCtrl.cgi?operation=%d&speed=%d&channelno=%d&value=%d",iCmd,iSpeed,iCh,iValue);

    printf("CNetSdkApp::PtzCtrl():URL = %s\r\n",URL);
    
    int iResult = 0;
    int  iRecvLen = 0;    
    m_pNS->ConfigOperation((char *)URL,NULL,NULL,iRecvLen,iResult);

    return;
}

/*****************************************************************************
 函 数 名  : CNetSdkApp.GetPtzPreset
 功能描述  : 获取预置点
 输入参数  : int iCh  通道号
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年9月30日
    作    者   : zdx
    修改内容   : 新生成函数

*****************************************************************************/
void CNetSdkApp::GetPtzPreset(int iCh)
{
    char URL[256];
    memset(URL,0,sizeof(URL));
    sprintf(URL,"merlin/GetPtzPresetCfg.cgi?Ch=%d",iCh);
    char recvData[2048]={0};
    int iRecvLen = sizeof(recvData)-1;

    printf("CNetSdkApp::GetPtzPreset():URL = %s\r\n",URL);
    
    int iResult = 0;
    m_pNS->ConfigOperation((char *)URL,NULL,recvData,iRecvLen,iResult);
    printf("CNetSdkApp::GetPtzPreset():recvData = %s\r\n",recvData);

    return;
}

/*****************************************************************************
 函 数 名  : CNetSdkApp.SetPtzPreset
 功能描述  : 设置预置点
 输入参数  : int iCh       通道号
             int iPresent  预置点编号
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年9月30日
    作    者   : zdx
    修改内容   : 新生成函数

*****************************************************************************/
void CNetSdkApp::SetPtzPreset(int iCh,int iPresent)
{
    char URL[256];
    memset(URL,0,sizeof(URL));
    sprintf(URL,"merlin/ModPTZPresetCfg.cgi");
    char recvData[2048]={0};
    int iRecvLen = sizeof(recvData)-1;

    Json::Value table;
    table["cmd"] = 0;
    table["Channel"] = iCh;
    table["PresetId"] = iPresent;
    table["Speed"] = 5;// 1-8
    table["DWellTime"] = 1;
    table["PresetName"] = "1";
    
    std::string stream = "";
    CConfigWriter JsonWtire(stream);
    JsonWtire.write(table);

    printf("CNetSdkApp::SetPtzPreset():URL = %s stream = %s\r\n",URL,stream.c_str());
    
    int iResult = 0;
    m_pNS->ConfigOperation((char *)URL,(char*)stream.c_str(),recvData,iRecvLen,iResult);

    return;
}

/*****************************************************************************
 函 数 名  : CNetSdkApp.ClearPtzPreset
 功能描述  : 清除预置点
 输入参数  : int iCh       通道号
             int iPresent  预置点编号
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年9月30日
    作    者   : zdx
    修改内容   : 新生成函数

*****************************************************************************/
void CNetSdkApp::ClearPtzPreset(int iCh,int iPresent)
{
    char URL[256];
    memset(URL,0,sizeof(URL));
    sprintf(URL,"merlin/ModPTZPresetCfg.cgi");
    char recvData[2048]={0};
    int iRecvLen = sizeof(recvData)-1;

    Json::Value table;
    table["cmd"] = 1;
    table["Channel"] = iCh;
    table["PresetId"] = iPresent;
    
    std::string stream = "";
    CConfigWriter JsonWtire(stream);
    JsonWtire.write(table);

    printf("CNetSdkApp::ClearPtzPreset():URL = %s stream = %s\r\n",URL,stream.c_str());
    
    int iResult = 0;
    m_pNS->ConfigOperation((char *)URL,(char*)stream.c_str(),recvData,iRecvLen,iResult);

    return;
}

/*****************************************************************************
 函 数 名  : CNetSdkApp.GetPtzTour
 功能描述  : 获取巡航组
 输入参数  : int iCh  通道
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年9月30日
    作    者   : zdx
    修改内容   : 新生成函数

*****************************************************************************/
void CNetSdkApp::GetPtzTour(int iCh)
{
    char URL[256];
    memset(URL,0,sizeof(URL));
    sprintf(URL,"merlin/GetPtzTourCfg.cgi?Ch=%d",iCh);
    char recvData[2048]={0};
    int iRecvLen = sizeof(recvData)-1;

    printf("CNetSdkApp::GetPtzPreset():URL = %s\r\n",URL);
    
    int iResult = 0;
    m_pNS->ConfigOperation((char *)URL,NULL,recvData,iRecvLen,iResult);
    printf("CNetSdkApp::GetPtzTour():recvData = %s\r\n",recvData);

    return;
}

/*****************************************************************************
 函 数 名  : CNetSdkApp.AddPtzTourPreset
 功能描述  : 增加预置点到巡航组
 输入参数  : int iCh         通道号
             int iTourIndex  巡航组号
             int iPresent    预置点编号
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年9月30日
    作    者   : zdx
    修改内容   : 新生成函数

*****************************************************************************/
void CNetSdkApp::AddPtzTourPreset(int iCh,int iTourIndex,int iPresent)
{
    char URL[256];
    memset(URL,0,sizeof(URL));
    sprintf(URL,"merlin/ModPtzTourCfg.cgi");
    char recvData[2048]={0};
    int iRecvLen = sizeof(recvData)-1;

    Json::Value table;
    table["cmd"] = 0;
    table["Channel"] = iCh;
    table["TourIndex"] = iTourIndex;
    table["PresetId"] = iPresent;
    table["DWellTime"] = 5;//秒
    table["Speed"] = 5;
    
    std::string stream = "";
    CConfigWriter JsonWtire(stream);
    JsonWtire.write(table);

    printf("CNetSdkApp::AddPtzTourPreset():URL = %s stream = %s\r\n",URL,stream.c_str());
    
    int iResult = 0;
    m_pNS->ConfigOperation((char *)URL,(char*)stream.c_str(),recvData,iRecvLen,iResult);

    return;
}


/*****************************************************************************
 函 数 名  : CNetSdkApp.DelPtzTourPreset
 功能描述  : 删除巡航组内的预置点
 输入参数  : int iCh         
             int iTourIndex  巡航组号
             int iPresent    预置点编号
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年9月30日
    作    者   : zdx
    修改内容   : 新生成函数

*****************************************************************************/
void CNetSdkApp::DelPtzTourPreset(int iCh,int iTourIndex,int iPresent)
{
    char URL[256];
    memset(URL,0,sizeof(URL));
    sprintf(URL,"merlin/ModPtzTourCfg.cgi");
    char recvData[2048]={0};
    int iRecvLen = sizeof(recvData)-1;

    Json::Value table;
    table["cmd"] = 1;
    table["Channel"] = iCh;
    table["TourIndex"] = iTourIndex;
    table["PresetId"] = iPresent;
    
    std::string stream = "";
    CConfigWriter JsonWtire(stream);
    JsonWtire.write(table);

    printf("CNetSdkApp::DelPtzTourPreset():URL = %s stream = %s\r\n",URL,stream.c_str());
    
    int iResult = 0;
    m_pNS->ConfigOperation((char *)URL,(char*)stream.c_str(),recvData,iRecvLen,iResult);

    return;
}

/*****************************************************************************
 函 数 名  : CNetSdkApp.ClearPtzTourPreset
 功能描述  : 清空巡航组内的预置点
 输入参数  : int iCh         通道号
             int iTourIndex  巡航组号
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年9月30日
    作    者   : zdx
    修改内容   : 新生成函数

*****************************************************************************/
void CNetSdkApp::ClearPtzTourPreset(int iCh,int iTourIndex)
{
    char URL[256];
    memset(URL,0,sizeof(URL));
    sprintf(URL,"merlin/ModPtzTourCfg.cgi");
    char recvData[2048]={0};
    int iRecvLen = sizeof(recvData)-1;

    Json::Value table;
    table["cmd"] = 2;
    table["Channel"] = iCh;
    table["TourIndex"] = iTourIndex;
    
    std::string stream = "";
    CConfigWriter JsonWtire(stream);
    JsonWtire.write(table);

    printf("CNetSdkApp::ClearPtzTourPreset():URL = %s stream = %s\r\n",URL,stream.c_str());
    
    int iResult = 0;
    m_pNS->ConfigOperation((char *)URL,(char*)stream.c_str(),recvData,iRecvLen,iResult);

    return;
}


/*****************************************************************************
 函 数 名  : CNetSdkApp.TestPtz
 功能描述  : 云台控制的测试函数
 输入参数  : 无
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年9月30日
    作    者   : zdx
    修改内容   : 新生成函数

*****************************************************************************/
void CNetSdkApp::TestPtz()
{
    printf("CNetSdkApp::TestSpeech():test start\r\n");
    int iCh = 0,iSpeed=5,iValue=1;

    /* 向左转 */
    PtzCtrl(iCh,VF_PTZ_OPT_LEFT,iSpeed,0);

    usleep(2*1000*1000);

    /* 停止后向右转 */
    PtzCtrl(iCh,VF_PTZ_OPT_STOP,iSpeed,0);
    PtzCtrl(iCh,VF_PTZ_OPT_RIGHT,iSpeed,0);
    usleep(2*1000*1000);

    /* 停止后向上转 */
    PtzCtrl(iCh,VF_PTZ_OPT_STOP,iSpeed,0);
    PtzCtrl(iCh,VF_PTZ_OPT_UP,iSpeed,0);
    usleep(2*1000*1000);

    /* 停止后向下转 */
    PtzCtrl(iCh,VF_PTZ_OPT_STOP,iSpeed,0);
    PtzCtrl(iCh,VF_PTZ_OPT_DOWN,iSpeed,0);

    /* 变倍测试 */
    PtzCtrl(iCh,VF_PTZ_OPT_STOP,iSpeed,0);
    PtzCtrl(iCh,VF_PTZ_OPT_ZOOM_TELE,iSpeed,0);
    usleep(2*1000*1000);

    PtzCtrl(iCh,VF_PTZ_OPT_STOP,iSpeed,0);
    PtzCtrl(iCh,VF_PTZ_OPT_ZOOM_WIDE,iSpeed,0);
    usleep(2*1000*1000);

    PtzCtrl(iCh,VF_PTZ_OPT_STOP,iSpeed,0);

    /* 转到预置点 */
    PtzCtrl(iCh,VF_PTZ_OPT_GOTOPRESET,iSpeed,iValue);

    /* 获取预置点 */
    GetPtzPreset(iCh);

    /* 预置点设置 */
    SetPtzPreset(iCh,1);
    
    /* 获取预置点 */
    GetPtzPreset(iCh);

    /* 删除预置点 */
    ClearPtzPreset(iCh,1);
    
    /* 获取预置点 */
    GetPtzPreset(iCh);

    /* 预置点设置 */
    SetPtzPreset(iCh,1);

    /* 获取巡航组 */
    GetPtzTour(iCh);

    AddPtzTourPreset(iCh,1,1);
    GetPtzTour(iCh);
    DelPtzTourPreset(iCh,1,1);
    ClearPtzTourPreset(iCh,1);
    GetPtzTour(iCh);
    
    printf("CNetSdkApp::TestSpeech():test end\r\n");
}

void CNetSdkApp::TestPtzUp()
{
    printf("---------[%s] ,[%d] \n",__FUNCTION__,__LINE__);
    printf("CNetSdkApp::TestPtzUp():test start\r\n");
    
    int iCh = 0,iSpeed=5;
    PtzCtrl(iCh,VF_PTZ_OPT_UP,iSpeed,0);
    usleep(2*1000*1000);
    
    PtzCtrl(iCh,VF_PTZ_OPT_STOP,iSpeed,0);
}

void CNetSdkApp::TestPtzDown()
{
    printf("---------[%s] ,[%d] \n",__FUNCTION__,__LINE__);
    printf("CNetSdkApp::TestPtzDown():test start\r\n");
    
    int iCh = 0,iSpeed=5;
    PtzCtrl(iCh,VF_PTZ_OPT_DOWN,iSpeed,0);
    usleep(2*1000*1000);
    
    PtzCtrl(iCh,VF_PTZ_OPT_STOP,iSpeed,0);
}

void CNetSdkApp::TestPtzLeft()
{
    printf("---------[%s] ,[%d] \n",__FUNCTION__,__LINE__);
    printf("CNetSdkApp::TestPtzLeft():test start\r\n");
    
    int iCh = 0,iSpeed=5;
    PtzCtrl(iCh,VF_PTZ_OPT_LEFT,iSpeed,0);
    usleep(2*1000*1000);
    
    PtzCtrl(iCh,VF_PTZ_OPT_STOP,iSpeed,0);
}

void CNetSdkApp::TestPtzRight()
{
    printf("---------[%s] ,[%d] \n",__FUNCTION__,__LINE__);
    printf("CNetSdkApp::TestPtzRight():test start\r\n");
    
    int iCh = 0,iSpeed=5;
    PtzCtrl(iCh,VF_PTZ_OPT_RIGHT,iSpeed,0);
    usleep(2*1000*1000);
    
    PtzCtrl(iCh,VF_PTZ_OPT_STOP,iSpeed,0);
}



/*****************************************************************************
 函 数 名  : CNetSdkApp.Help
 功能描述  : 帮助信息
 输入参数  : 无
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年9月30日
    作    者   : zdx
    修改内容   : 新生成函数

*****************************************************************************/
void CNetSdkApp::Help()
{
    printf("----Input test cmd\n");
    printf("a ---- TestAlarm\n");
    printf("m ---- TestMonitor\n");
    printf("m1 ---- TestMonitor_stable_test ctrl+c stop\n");
    printf("m2 ---- TestMonitor_open_close_stable_test ctrl+c stop\n");
    printf("r ---- TestRecord\n");
    printf("c ---- TestConfig\n");
    printf("c1 ---- TestConfigEncode\n");
    printf("s ---- TestSpeech\n");
    printf("s1 ---- TestSpeech_stable_test ctrl+c stop\n");
    printf("s2 ---- TestSpeech_open_close_stable_test ctrl+c stop\n");
    printf("p ---- TestPtz\n");
    printf("pu ---- TestPtzUp\n");
    printf("pd---- TestPtzDown\n");
    printf("pl---- TestPtzLeft\n");
    printf("pr---- TestPtzRight\n");
    printf("e ---- TestAlarmEx\n");		
    printf("f ---- TestFeeder\n");	
}

/*****************************************************************************
 函 数 名  : CNetSdkApp.Test
 功能描述  : 测试程序主入口
 输入参数  : 无
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年9月30日
    作    者   : zdx
    修改内容   : 新生成函数

*****************************************************************************/
void CNetSdkApp::Test()
{
    char buf[64]={0};
    int i = 0;

    m_pNS=ILocalNetSDK::CreatNew();

    m_pNS->SetServerInfo((char *)"127.0.0.1",80);
    m_pNS->SetNamepasswd("admin","123456");

    m_pNS->AttachDefDelegate(OnDefMsg,(unsigned long)m_pNS);
    m_pNS->AttachDelegateMonitor(OnMonitor,(unsigned long)m_pNS);
    m_pNS->AttachDelegateAlarm(OnAlarm,(unsigned long)m_pNS);

    Help();

    while(1)
    {
        fflush(stdout);
        memset(buf, 0, sizeof(buf));

        for (i = 0; i < 64; i++)
        { 
            buf[i]=getchar();
            if(buf[i] == '\n')
            {
                //printf("\n");
                break;
            }            
        }

        // 命令长度超出，丢弃
        if(i >= 64)
        {
            ///避免串口作为其他用途时，getchar总是立即返回构成死循环
            usleep(1000*1000);
            continue;
        }
    
        // 去掉多余的换行符
        if(i > 0 && buf[i - 1] == '\r')
        {
            i--;
        }
        buf[i] = '\0';

        switch(buf[0])
        {
            case 'a':
                TestAlarm();
                break;
            case  'm':

                    printf("------TestMonitor---[%s] ,[%d] [%d]\n",__FUNCTION__,__LINE__,buf[1]);
                    switch(buf[1])
                    {
                        case '1':
                            TestMonitor1();
                            break;
                        case '2':
                            TestMonitor2();
                            break;
                        default:
                            TestMonitor();
                            break;
                    }
                
                break;
            case  'r':
                TestRecord();
                break;
            case  'c':
                printf("CNetSdkApp::Test()>>>buf[0]=[%c],buf[1]=[%c]\r\n",buf[0],buf[1]);
                switch(buf[1])
                {
                    case '1':
                    {
                        TestConfigEncode();
                        break;
                    }
                    default:
                    {
                        TestConfig();
                        break;
                    }
                }
                break;
            case  's':
                    printf("-----TestSpeech----[%s] ,[%d] [%d]\n",__FUNCTION__,__LINE__,buf[1]);
                    switch(buf[1])
                    {
                        case '1':
                            TestSpeech1();
                            break;
                        case '2':
                            TestSpeech2();
                            break;
                        default:
                            TestSpeech();
                            break;
                    }
                
                break;
            case  'p':
                
                    printf("-----TestPtz----[%s] ,[%d] [%d]\n",__FUNCTION__,__LINE__,buf[1]);
                    switch(buf[1])
                    {
                        case 'u':
                            TestPtzUp();
                            break;
                        case 'd':
                           TestPtzDown();
                            break;
                        case 'l':
                           TestPtzLeft();
                            break;
                        case 'r':
                           TestPtzRight();
                            break;
                        default:
                            TestPtz();
                            break;
                    }
                break;
            case  'e':
                TestAlarmEx();
                break;
			case  'o':
                TestAlarmOut();
                break;	
			case  'i':		
				TestAlarmIn();
				break;
            case  'f':            
                    printf("-----TestFeeder----[%s] ,[%d] [%d]\n",__FUNCTION__,__LINE__,buf[1]);
                    switch(buf[1])
                    {
                        case '1':
                            TestGetWIFI();
                            break;
                        case '2':
                           TestSetWIFI();
                            break;
                        case '3':
                          // TestFeederPTZ();
                            break;
                        case '4':
                           TestFeederWeigh();
						   break;
						case '5':
                           TestSetFeeder();
                           break;
                        case '6':
                           TestGetFeeder();   
                            break;
						case '7':
						   TestGetRtmp();   
							break;
                        case '8':
                           //TestSetRtmp();   
                            break;						
                        default:
                           // TestFeederPTZ();
                            break;
                    }				
            default:
                Help();
                break;
        }
    }
}


/*****************************************************************************
 函 数 名  : CNetSdkApp.ProcRecord
 功能描述  : 录像回放时回调的数据先入队列再这里处理，模拟真实的处理速度
 输入参数  : 无
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年9月30日
    作    者   : zdx
    修改内容   : 新生成函数

*****************************************************************************/
int CNetSdkApp::ProcRecord()
{
    if( m_packet_count > 0 )
    {
//        printf("m_packet_count = %d\r\n",m_packet_count);

        pthread_mutex_lock((pthread_mutex_t *)CNetSdkApp::instance()->m_hMutex);  
        PACKET_LIST::iterator it;

        for( it = m_packet_list.begin(); it != m_packet_list.end();  )
        {
            m_total_len -= (*it).iLen;

            //todo 这里增加转发等处理
            if((*it).iLen == 0)
            {
                //单个文件结束
                printf("CNetSdkApp::ProcRecord():record file end\r\n");
                m_iRecordState = 0;
            }

            if((*it).pData)
            {
                delete (*it).pData;
                (*it).pData = NULL;
                (*it).iLen = 0;
            }

            it++;
            m_packet_list.pop_front();
            m_packet_count--;

            //模拟40ms处理一帧数据
            break;
        }
        
        pthread_mutex_unlock((pthread_mutex_t *)CNetSdkApp::instance()->m_hMutex);  
     }

    if (m_packet_count > 80)
    {
        if (m_iPause == 0)
        {
            printf("Pause record m_packet_count = %d\r\n",m_packet_count);
            m_pNS->PauseRecord(0,0);
            m_iPause = 1;
        }
    }
    else if(m_packet_count == 0)
    {
        if(m_iRecordState == 1 && m_iPause == 1)
        {        
            printf("resume record\r\n");
            m_pNS->PauseRecord(0,1);
            m_iPause = 0;
        }
    }

    return 0;
}

/*****************************************************************************
 函 数 名  : CNetSdkApp.Start
 功能描述  : 启动函数，用于创建线程
 输入参数  : 无
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年9月30日
    作    者   : zdx
    修改内容   : 新生成函数

*****************************************************************************/
int CNetSdkApp::Start()
{
    int m_nPriority = 60;
    int m_dwStackSize = 0;
	int			policy;
	int			priority_min;
	int			priority_max;
	unsigned int			min_stack_size = 16384;
	pthread_attr_t		attr;

	policy = SCHED_OTHER;

	if ((priority_min = sched_get_priority_min(policy)) == -1)
	{
		//return -1;
	}

	if ((priority_max = sched_get_priority_max(policy)) == -1)
	{
		//return -1;
	}

	if (m_nPriority > priority_max)	m_nPriority = priority_max;
	if (m_nPriority < priority_min)	m_nPriority = priority_min;

	if (policy != SCHED_OTHER)
	{
		m_nPriority = priority_max - m_nPriority;
	}
	else
	{
		m_nPriority = 50;
	}

	if( m_dwStackSize < (int)min_stack_size )
	{
		m_dwStackSize = min_stack_size;
	}

	if (pthread_attr_init(&attr) != 0)
	{
	    printf("pthread_attr_init error\r\n");
		return -1;
	}

	if (pthread_create((pthread_t *)&m_hThread, NULL,
		(void* (*)(void *))ThreadBodyRecord, this) != 0)
	{
	    printf("pthread_create error\r\n");
		return -1;
	}

	if (pthread_detach((pthread_t)(m_hThread)) != 0)
	{
	    printf("pthread_detach error\r\n");
		return -1;
	}
    
    return 0;
}

/*****************************************************************************
 函 数 名  : CNetSdkApp.Stop
 功能描述  : 停止线程
 输入参数  : 无
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年9月30日
    作    者   : zdx
    修改内容   : 新生成函数

*****************************************************************************/
int CNetSdkApp::Stop()
{
    return 0;
}

int CNetSdkApp::TestConfigEncode()
{
    printf("CNetSdkApp::TestConfigEncode()>>>begin\r\n");
    if(m_pNS == NULL)
    {
        printf("CNetSdkApp::TestConfigEncode()>>>error:create m_pNS failed\r\n");
        return -1;
    }

    if ( false == m_pNS->IsLogin())
    {
        if(0 != (m_pNS->Login()))
        {
            printf("CRangingNvr::TestConfigEncode()>>>login failed\r\n");
            return -1;
        }
    }

    char aRecvBuf[5*1024]={0};
    int  iRecvLen = sizeof(aRecvBuf)-1;
    int  iResult = 0;

    memset(aRecvBuf,0,sizeof(aRecvBuf));
    iRecvLen = sizeof(aRecvBuf)-1;
    iResult = 0;
    m_pNS->ConfigOperation((char *)"merlin/GetEncodeAbility.cgi?chstart=0&chnum=1",NULL,aRecvBuf,iRecvLen,iResult);
    printf("CNetSdkApp::TestConfigEncode()>>>iResult = [%d],iRecvLen = [%d]\r\n",iResult,iRecvLen);
    DumpJson(aRecvBuf,(char *)"GetEncodeAbility.cgi");

    if (iResult != 200 && iRecvLen <= 0)
    {
        printf("CNetSdkApp::TestConfigEncode()>>>iResult error");
        return -1;
    }
    
    //获取编码配置参数
    memset(aRecvBuf,0,sizeof(aRecvBuf));
    iRecvLen = sizeof(aRecvBuf)-1;
    iResult = 0;
    m_pNS->ConfigOperation((char *)"merlin/GetEncodeCfg.cgi?chstart=0&chnum=1",NULL,aRecvBuf,iRecvLen,iResult);
    printf("CNetSdkApp::TestConfigEncode()>>>iResult = %d iRecvLen = %d aRecvBuf = \r\n",iResult,iRecvLen);
    DumpJson(aRecvBuf,(char *)"GetEncodeCfg.cgi");

    if (iResult != 200 && iRecvLen <= 0)
    {
        printf("CNetSdkApp::TestConfigEncode()>>>iResult error");
        return -1;
    }
    
    CConfigReader JsonReader;
    Json::Value table;
    bool ret = JsonReader.parse(aRecvBuf,table);
    if (ret == false)
    {
        printf("CNetSdkApp::TestConfigEncode()>>>error:JsonReader.parse failed \r\n");
    }
    
       
    char tmpComp[64] = {0};
    GetCgiJSValue_CString(table["EncodeCfg"][0u]["mainstream"]["video"],
        "compression",tmpComp,sizeof(tmpComp));
    printf("CNetSdkApp::TestConfigEncode()>>>tmpComp=[%s]\r\n",tmpComp);
    int iWidth = 0,iHeight = 0;
    GetCgiJSValue_Int(table["EncodeCfg"][0u]["mainstream"]["video"]["resolution"],
        "h",iHeight);
    GetCgiJSValue_Int(table["EncodeCfg"][0u]["mainstream"]["video"]["resolution"],
        "w",iWidth);
    printf("CNetSdkApp::TestConfigEncode()>>>iWidth=[%d],iHeight=[%d]\r\n",iWidth,iHeight);

    //设置编码配置参数 
    //可以设置的值在GetEncodeAbility.cgi获取的能力集里
    table["EncodeCfg"][0u]["mainstream"]["video"]["compression"] = "H264";
    table["EncodeCfg"][0u]["mainstream"]["video"]["resolution"]["w"] = 1280;
    table["EncodeCfg"][0u]["mainstream"]["video"]["resolution"]["h"] = 720;
    
    std::string stream = "";
    CConfigWriter JsonWtire(stream);
    JsonWtire.write(table);

    iRecvLen = 0;
    DumpJson((char *)stream.c_str(),(char *)"SetEncodeCfg.cgi");

    m_pNS->ConfigOperation((char *)"merlin/SetEncodeCfg.cgi",(char *)stream.c_str(),NULL,iRecvLen,iResult);
    printf("CNetSdkApp::TestConfigEncode()>>>iResult = [%d] end\r\n",iResult); 
    return 0;
}

