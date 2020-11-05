#include <unistd.h>
#include <pthread.h>
#include <inttypes.h>
#include <time.h>
#include <sys/time.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "Sip.h"
#include "Socket.h"

int main()
{   
    int err;
    
    sip_read_config();
    
    err=sip_init();     
    if(err!=0)
    {
        printf("sip init err--------------------------------\n");
        exit(0);
    }
    
    err=socket_init();
    if(err!=0)
    {
        printf("socket init err------------------------------\n");
        exit(0);
    }

    err=socket_listening();
    if(err!=0)
    {
        printf("socket listening err-------------------------\n");
        exit(0);
    }

    while(1)
    {
        sleep(10);
    }

    socket_close();
    sip_destroy();
}

