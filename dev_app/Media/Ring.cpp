#include <iostream>
#include "Ring.h"
#include "comm.h"

#define SHM_DIR "/shm/audio2"
#define SHARE_SIZE  160
#define RING_FILE   "/mnt/ring/ring.g711"

using namespace std;

CRing::CRing()
{
    mShm = NULL;
    mTemp = NULL;

    thread_quit = 0;
    thread_quit_event = NULL;
}

CRing::~CRing()
{
    thread_quit = 1;
    hw_wait_event(thread_quit_event, -1);
    debugstring("quit the thread of ring");

    if(thread_quit_event)
    {
        hw_release_event(thread_quit_event);
        thread_quit_event = NULL;
    }

    if(h_thread)
    {
        hw_release_thread(h_thread);
        debugstring("release the thread of ring");
        h_thread = NULL;
    }

    if(mShm){
        delete mShm;
        mShm = NULL;
        free(mTemp);
    }
}

int CRing::RingRun()
{
    debugstring("start the Ringrun");
    mTemp = (char *)malloc(SHARE_SIZE);
    if(mTemp == NULL){
        cerr<<"create Up Ring temp buffer error"<<endl;
        return -1;
    }

    mShm = new PnShMem(SHM_DIR,SHARE_SIZE);
    if(mShm == NULL){
        cerr<<"create Up audio shm error"<<endl;
        free(mTemp);
        return -1;
    }

    //open the ring thread
    thread_quit = 0;
    thread_quit_event = hw_create_event();
    h_thread = hw_create_thread((void *)CRing::WriteRing, (void *)this);
    if(h_thread == NULL)
    {
        return -1;
    }
    return 0;
}

void CRing::WriteRing(void *params)
{
    debugstring("In writeRing thread");
    CRing *pRing = (CRing *) params;
    FILE* fd = NULL;
    char buffer[160];
    int readsize;
    int count =0;

    if((fd = fopen(RING_FILE,"r"))==NULL)
        debugstring("Open the sound file failed");

    while(count <10)
    {
        if(pRing->thread_quit)
            break;
        if(feof(fd)){
            fseek(fd,0,SEEK_SET);
            count ++;
        }
        readsize = fread(buffer,sizeof(char),160,fd);
        pRing->mShm->WriteShM(buffer,readsize);

        hw_delay(10);
    }
    hw_set_event(pRing->thread_quit_event);
    return;
}

