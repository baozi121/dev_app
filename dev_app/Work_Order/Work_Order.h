#ifndef _WORK_ORDER_H_
#define _WORK_ORDER_H_

#include "IniFile.h"
#include "Pnmsq.h"
#include "comm.h"

class CWork_Order
{
    public:
        CWork_Order();
        ~CWork_Order();

        int PN_Work_Order_Init();
        int PN_Work_Order_Release();
        int PN_Work_Order_Setconfig(int config_id ,void* config_data);
        int PN_Work_Order_Electricity_Information_XML_Send(char* text);
        int PN_Work_Order_Wifi_Info_XML_Send();
        int PN_Work_Order_SipCall_Query_XML_Send(char* text);
        int PN_Work_Order_GroupNum_Query_XML_Send(char* text);
        void SetCallback(SendCallback callback,void* inst);

    private:
        int PN_Work_Order_Setconfig_Wifi_Info_Response(void* config_data);
        int PN_Work_Order_Setconfig_Dev_Call_Info_Response(void* config_data);
        int PN_Work_Order_Setconfig_GroupNum_Info_Response(void* config_data);                                                              
        int PN_Work_Order_Listening();
        static void PN_Work_Order_Thread_Listening(void* params);
        static void PN_Work_Order_Usb_Thread_Listening(void* params);

        pthread_t listening_thread;
        pthread_t usb_thread;
        CIniFile m_ini;
        CMSQ m_msq;
        SendCallback    callback_send;
        void* m_inst;
};

#endif
