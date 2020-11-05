#ifndef __PNMSQ_H__
#define __PNMSQ_H__

#define PATH "/etc/profile"
// msg_type
#define MSG2GROUP4QT    2   //QTPTT 
#define MSGENCCONTROL   3   //Control message
#define MSG2SIPCALL4DEV 4   //SIPCALL
#define MSG2SIPCALL4QT  5   //SIPCALL
#define QTWORKORDER     6   //work order
#define DEVWORKORDER    7   //work order
#define QTFTPUP         8   //ftp upload
#define QTFTPDOWN       9   //ftp download

//control---dev
#define DEVBITRATE      1   //bitrate control

//QT PTT ---dev
// #define QTGROUPDOWN          1  //PTT DOWN
// #define QTGROUPUP          2  //PTT UP
// login---qt
#define QTMDLOGIN       1   //md login
#define QTCDLOGIN       2   //cd login
#define QTMDLOGINCONF   3   //md login confirm
#define QTCDLOGINCONF   4   //cd login confirm
#define QTLOGOUT        5   //logout
// login---dev
#define DEVMDLOGINRESP      1   //md login response
#define DEVCDLOGINRESP      2   //cd login response
#define DEVLOGINCONFRESP    3   //md login confirm response
#define DEVLOGOUTRESP       4   //cd login confirm response

// work order---qt
#define QTTASKREAD      11  //task read
#define QTTIMEINFO      12  //time information
#define QTFEEINFO       13  //fee information
#define QTSATISFACTION  14  //satisfaction information
#define QTLOADINFO      15  //load or not
#define QTTASKFINISH    16  //task finish
// work order---dev
#define DEVTASKINFO     11  //get new task

// maintain---qt
#define QTMAINTSTART    21  //maint start
#define QTMAINTNOTE     22  //maint report remind
#define QTMAINTINFO     23  //maint report information
#define QTMAINTEND      24  //maint end
// maintain---dev
#define DEVMAINTSTARTRESP   21  //maint start response
#define DEVMAINTNOTERESP    22  //maint report remind response

// other configuration---qt
#define QTWIFICFG           31  //wifi configure
#define QTELECINFO          32  //electric information
#define QTSTAKEINFO         33  //stake query
#define QTCALLINFO          34  //call query
#define QTGROUPINFO          35  //call query
// other configuration---dev
#define DEVWIFICFGRESP      31  //wifi configure response
#define DEVSTAKEINFO        32  //stake query response
#define USBDETECT           36  //usb detect

// ftp upload---qt
#define FILEBACKUP        1   //file upload
#define QTUPLOAD          2   //file upload
#define PICUPLOAD         3   //file upload

// ftp download---qt
#define QTSOFTWAREUPDATEDET 1   //software update detect
#define QTSOFTWAREUPDATE    2   //software update
// ftp upload download --dev
#define DEVUPLOADRESP   41      //file upload response
#define DEVUPDATEDET    42      //software update detect response
#define DEVUPDATERESP   43      //software update response
#define DEVBACKUPRESP   44      //file backup
#define DEVLOGINSTATUS  53      //login status

#define SIPCALLLEN 250
#define TEXTLEN 50

// typedef struct
// {
    // int msg_type;
    // unsigned int flag;
    // char caller[250];
// }msg_sipcall;

typedef struct
{
    int msg_type;
    unsigned int m_type;
    char text[250];
}msg_task;

typedef struct
{
    int msg_type;
    unsigned int value;
    unsigned int m_type;
}msg_control;

class CMSQ
{
    public:
        CMSQ(void);
        ~CMSQ(void);
        int Msq_Create();
        // int Msq_Delete();
        // int Msq_Send_Sipcall(msg_sipcall msg);
        // int Msq_Read_Sipcall(msg_sipcall* msg,long int type);
        int Msq_Send_Task(msg_task msg);
        int Msq_Read_Task(msg_task* msg,long int type);
        int Msq_Send_Control(msg_control msg);
        int Msq_Read_Control(msg_control* msg,long int type);
    private:
        int msqid;
};

#endif
