#ifndef __NETSDKAPP_H_asdasds_
#define __NETSDKAPP_H_asdasds_


#include "/opt/jthread_jrtplib/include/SDk/INetSdk.h"
#include <list>

//CNetSdkApp *pApp=NULL ;
typedef struct _NS_DataBuf__
{
    char *pData;
    int  iLen;
}NS_DataBuf;

void DumpJson(char *pData, char *pCgiCmd = NULL);


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
void OnMonitor(int channel, int stream,VF_FRAME_INFO*pInfo,char *pData,int iLen,unsigned int userdata);

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
void OnRecord(int channel, int stream,VF_FRAME_INFO*pInfo,char *pData,int iLen,unsigned int userdata);

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
void OnSpeech(int channel, int stream,VF_FRAME_INFO*pInfo,char *pData,int iLen,unsigned int userdata);

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
void OnAlarm(int channel, unsigned int type, int state,unsigned int userdata);

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
void OnDefMsg(enum DefEventCode code, unsigned int userdata);

class CNetSdkApp
{
public:
    typedef enum
    {
        SUCESS = 0,
        LISTENER_ERROR,
        CONNECT_FAILED,
        Login_HaveLogined,    // !用户已登录
        DiffStream_HaveStarted,    // !不同类型的码流已经开启,所以会开启失败
        Dsiconnect_No_Relogin,    // !断开, 不自动重新登陆
        Disconnect_AutoReLogin,    // !断开,自动重新登陆
        r_error_monitor_not_login,
    }Result_E;

	typedef std::list<NS_DataBuf> PACKET_LIST;

    static CNetSdkApp* instance();

public:
    CNetSdkApp();        
    virtual ~CNetSdkApp();


    int Start();
    int Stop();

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
    void Test();



//private:
    
    
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
    int ProcRecord();

    
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
    int GetRecordList(ILocalNetSDK *pNS,int iCh,NS_FILE_INFO*pFile,int &iNum);

    
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
    void TestMonitor();
	void TestMonitor1();
	void TestMonitor2();
    
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
    void TestConfig();
    
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
    void TestRecord();
    
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
    void TestAlarm();
    void TestAlarmEx();
	void TestAlarmOut();
	void TestAlarmIn();

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
    void TestSpeech();
	void TestSpeech1();
	void TestSpeech2();
    
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
    void TestPtz();
	void TestPtzUp();
	void TestPtzDown();
	void TestPtzLeft();
	void TestPtzRight();
    void Help();

	int TestFeederWeigh(); //喂食器称重
	void TestFeederPTZ(int position);  //喂食器控制
	void TestSetWIFI();    //设置wifi
	void TestGetWIFI();	//获取wifi
	void TestSetFeeder();	 //喂食器参数设置
	void TestGetFeeder(); //喂食器参数获取
	void TestGetRtmp();	//获取rtmp协议
	void TestSetRtmp(const char *rtmp_addr);	//设置rtmp协议

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
    void PtzCtrl(int iCh,int iCmd,int iSpeed,int iValue);

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
    void GetPtzPreset(int iCh);

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
    void SetPtzPreset(int iCh,int iPresent);

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
    void ClearPtzPreset(int iCh,int iPresent);

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
    void GetPtzTour(int iCh);

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
    void AddPtzTourPreset(int iCh,int iTourIndex,int iPresent);
    
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
    void DelPtzTourPreset(int iCh,int iTourIndex,int iPresent);

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
    void ClearPtzTourPreset(int iCh,int iTourIndex);

    int TestConfigEncode();

    ILocalNetSDK *m_pNS;

	PACKET_LIST m_packet_list;//录像数据缓冲列表
	unsigned int m_packet_count; //!主要实时更新m_packet_list的数量，这样判断数量可以不用进入锁里去
	unsigned int m_total_len; //!缓存pakcet list总大小
	void *m_hMutex;//录像数据列表锁
	unsigned int m_packet_size;//录像数据缓冲列表大小

    int m_iPause;//录像回放下载暂停状态
    int m_iRecordState;//录像下载状态
    int m_iFileTotalLen;//下载的录像的大小
    int m_iFileRecvlen;//已经下载的数据大小

	void *m_hThread;

};

#endif


