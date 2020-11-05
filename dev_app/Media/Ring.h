#pragma once
#include "PnShareMem.h"
#include <pthread.h>
#include "hw_lib.h"

class CRing
{
    public:
        CRing();
        ~CRing();
        int RingRun();

    private:
        static void WriteRing(void *params);

        PnShMem *mShm;
        char *mTemp;

        void *h_thread;
        int thread_quit;
        void* thread_quit_event;
};

