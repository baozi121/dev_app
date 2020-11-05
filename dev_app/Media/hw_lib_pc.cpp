#ifdef _MSC_VER
#include <windows.h>
#include <pthread.h>
#else
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <inttypes.h>
#include <time.h>
#include <sys/time.h>
typedef uint32_t u32;
#endif
#include "hw_lib.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

void* hw_alloc_physmem( int size , u32*phy )
{
    phy = (u32*)malloc( size );
    return phy;
}

void  hw_free_physmem( void*vir)
{
    free( vir );
}

void* hw_map_io( u32 phy , int size , int cacheenable )
{
    return 0;
}

void hw_unmap_io( void*vir , int size )
{
    return;
}
/*
typedef struct  
{
    u32     sysintr_id;
    HANDLE  event;
}interrupt_info;*/

void* hw_init_interrupt( int id )
{
    return 0;
}
int hw_wait_interrupt( void*handle , unsigned int timeout )
{
    return -1;
}
void hw_complete_interrupt( void*handle )
{
    return;
}
void hw_release_interrupt( void*handle )
{
    return ;
}

void    hw_delay( int ms )
{
#ifdef _MSC_VER
    Sleep( ms );
#else
    usleep(ms*1000);
#endif
}

void* hw_init_critical(void)
{
#ifdef _MSC_VER
    LPCRITICAL_SECTION c = (LPCRITICAL_SECTION)malloc(sizeof(CRITICAL_SECTION));
    if( c )
    {
        InitializeCriticalSection( c );
        return c;
    }
    return 0;
#else
    pthread_mutex_t *cs = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    if( cs )
    {
        pthread_mutex_init(cs,NULL);
        return cs;
    }
    return NULL;
#endif
}

int hw_enter_critical(void *h)
{
#ifdef _MSC_VER
    if( h )
    {
        EnterCriticalSection( (LPCRITICAL_SECTION)h );
        return 0;
    }
    return -1;
#else    
    if( h )
    {
        pthread_mutex_lock( (pthread_mutex_t *)h );
        return 0;
    }
    return -1;
#endif
}

int hw_leave_critical(void* h)
{
#ifdef _MSC_VER
    if( h )
    {
        LeaveCriticalSection( (LPCRITICAL_SECTION)h );
        return 0;
    }
    return -1;
#else    
    if( h )
    {
        pthread_mutex_unlock( (pthread_mutex_t *)h );
        return 0;
    }
    return -1;
#endif
}

void hw_release_critical(void*h)
{
#ifdef _MSC_VER
    if( h )
    {
        DeleteCriticalSection( (LPCRITICAL_SECTION)h);
        free(h);
    }
#else
    if( h )
    {
        pthread_mutex_destroy( (pthread_mutex_t *)h);
        free(h);
    }
#endif
}

int hw_flush_cache( void* vir , int size )
{
    return 0;
}

void* hw_create_thread( void*func , void*param )
{
#ifdef _MSC_VER
    HANDLE thread = 0;
    thread = CreateThread( NULL , 0 , (LPTHREAD_START_ROUTINE)func , param , 0 , 0 );
    return (void*)thread;
#else
    pthread_t *thread = (pthread_t *)malloc(sizeof(pthread_t));
       
    if (thread)
    {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_create( thread, &attr, (void *(*)(void *))func, param);
    }
    return thread;
#endif
}
void hw_release_thread( void*thread )
{
#ifdef _MSC_VER
    if( thread )
    {
        if( WaitForSingleObject( (HANDLE)thread , 1000 ) == WAIT_TIMEOUT )
            TerminateThread( (HANDLE)thread , -1 );
        CloseHandle( (HANDLE)thread );
    }
#else
    if (thread)
    {
        usleep(1);
        pthread_kill(*(pthread_t*)thread,9);
        free(thread);
    }
#endif
}

#ifndef _MSC_VER
typedef struct _tag_Event_t
{
    pthread_mutex_t mutex;
    pthread_cond_t  condition;
    int             sem_count;
} Event_t;
#endif
void* hw_create_event()
{
#ifdef _MSC_VER
    return CreateEvent( NULL , 0 , 0 , NULL );
#else
    Event_t *token;
    token=(Event_t *)malloc(sizeof(Event_t));
    token->sem_count=0;
    pthread_mutex_init(&token->mutex, NULL);
    pthread_cond_init(&token->condition, NULL);
    return token;
#endif
}

int hw_set_event( void*handle )
{
#ifdef _MSC_VER
    if( handle )
        SetEvent( (HANDLE)handle );
    return 0;
#else
    Event_t *token = (Event_t *)handle;

    pthread_mutex_lock(&token->mutex);
    token->sem_count = 1;
    pthread_cond_broadcast(&token->condition);
    pthread_mutex_unlock(&token->mutex);
#endif
    return 0;
}

int hw_wait_event( void*handle , int timeout )
{
#ifdef _MSC_VER
    if( handle )
    {
        if( WaitForSingleObject( (HANDLE)handle , timeout ) == WAIT_TIMEOUT )
            return 1;
        else
            return 0;
    }
    else 
        return -1;
#else
    Event_t *token = (Event_t *)handle;

    pthread_mutex_lock(&token->mutex);
    if (!token->sem_count)
    {
            pthread_cond_wait(&token->condition, &token->mutex);
    }
        token->sem_count = 0;
    pthread_mutex_unlock(&token->mutex);
#endif
    return 0;
}

int   hw_release_event( void*handle )
{
#ifdef _MSC_VER
    if( handle )
    {
        CloseHandle( (HANDLE)handle );
        return 0;
    }
    else
        return -1;
#else
    Event_t *token = (Event_t *)handle;

    pthread_mutex_destroy(&token->mutex);
    pthread_cond_destroy(&token->condition);
    free(token);
#endif
    return 0;
}

unsigned long     hw_get_sys_tick()
{
#ifdef _MSC_VER
    return (u32)GetTickCount();
#else
    uint64_t system_clock;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    system_clock = tv.tv_sec*1000+(tv.tv_usec+500)/1000;
        return (u32)system_clock;
#endif
}

int   hw_get_framebuffer( u32*framebuffer , u32*width , u32*height)
{
#ifdef _MSC_VER
    if( framebuffer )
        *framebuffer = 0x03f00000;
    if( width )
        *width = GetSystemMetrics(SM_CXSCREEN);
    if( height )
        *height = GetSystemMetrics(SM_CYSCREEN);
    return 0;
#else
    return -1;
#endif
}

static struct  
{
    int status;
    int type;
    int priority;
    FILE* tracefp;
    void* critical;
}trace = { 0 , 0 , 0 , 0 , 0 };

int   hw_trace_init( int type , int prio )
{
    if( trace.status == 0 )
    {
        trace.status = 1;
        trace.type = type;
        trace.priority = prio;
        if( type == 0 )
            trace.tracefp = fopen("trace.txt" , "w" );
        trace.critical = hw_init_critical();
        if( ((type == 0&&trace.tracefp)||type) && trace.critical )
            return 0;
        else
        {
            hw_trace_end();
            return -1;
        }
    }
    else
        return -1;
}

int   hw_trace_msg( int prio , char* fmt, ...)
{
    if( trace.status && (prio&trace.priority) )
    {
        va_list args;       
        hw_enter_critical( trace.critical );
        va_start(args, fmt);
        if( trace.type == 0 && trace.tracefp )
            vfprintf( trace.tracefp , fmt , args );         
        else
        {
            vprintf( fmt , args);
        }
        hw_leave_critical( trace.critical );
        return 0;
    }   
    else
        return -1;
}

int   hw_trace_end()
{
    if( trace.status )
    {
        if( trace.type == 0 )
            fclose( trace.tracefp );
        if( trace.critical )
            hw_release_critical( trace.critical );
        memset( &trace , 0 , sizeof(trace) );
        return 0;
    }
    else
        return -1;
}

