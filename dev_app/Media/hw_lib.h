#ifndef HW_LIB_H
#define HW_LIB_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef unsigned int u32;   /* Unsigned 32 bit integer               */

/************************************************************************/
/*
    alloc a block of physical-linear memory

out
    ret :   virtual address of the memory
    *phy :  physical address                                                                */
/************************************************************************/
void* hw_alloc_physmem( int size , u32*phy );

/************************************************************************/
/* free memory allocated by hw_alloc_physmem
in
    vir : virtual address allocated
    size: size of the memory block
    phy : physical address allocated
                                                      */
/************************************************************************/
void  hw_free_physmem( void*vir);

/************************************************************************/
/* map physical space to virtual

in
    phy : address

out
    ret : mapped virtual address                                                                     */
/************************************************************************/
void* hw_map_io( u32 phy , int size , int cacheenable );
/************************************************************************/
/* free mapped space                                                                     */
/************************************************************************/
void hw_unmap_io( void*vir , int size );

#define InterruptID_ITUInput    1
#define InterruptID_ITUOutput   2
#define InterruptID_Mp4Decoder  3
#define InterruptID_Mp4Encoder  4
#define InterruptID_VIA     5
/************************************************************************/
/* initial interrupt

in
    id : interrupt id(irq or system_interrupt
out
    ret : handle
                                                                     */
/************************************************************************/
void* hw_init_interrupt( int id );
/************************************************************************/
/* wait until interrupt occure or time out
in
    handle  : handle initialized by hw_init_interrupt
    timeout : time out value for wait
out
    *status : store interrupt status
    ret     : 0 indicate success, -1 indicate fail.

                                                                */
/************************************************************************/
int hw_wait_interrupt( void*handle , unsigned int timeout );
/************************************************************************/
/* tell system interrupt has been handled
in
    handle  : handle    : handle initialized by hw_init_interrupt
*/
/************************************************************************/
void hw_complete_interrupt( void*handle );
/************************************************************************/
/* release interrupt handling
in
    handle  : handle    : handle initialized by hw_init_interrupt                                                                      */
/************************************************************************/
void hw_release_interrupt( void*handle );

/************************************************************************/
/* sleep
in
    ms  : m-seconds
*/
/************************************************************************/
void    hw_delay( int ms );

/************************************************************************/
/* initial a object for critical-section
out
    ret : handle of the object
*/
/************************************************************************/
void* hw_init_critical(void);

/************************************************************************/
/* enter a critical-section
in
    h   : handle of the object
*/
/************************************************************************/
int hw_enter_critical(void*h);
/************************************************************************/
/* leave a critical-section
in
    h   : handle of the object
*/
/************************************************************************/
int hw_leave_critical(void*h);

/************************************************************************/
/* release a critical-section
in
    h   : handle of the object
*/
/************************************************************************/
void hw_release_critical(void*h);

/************************************************************************/
/* flush cache of the range of memory
in
    vir : address of the memory
    size    : range of flushing
*/
/************************************************************************/
int hw_flush_cache( void* vir , int size );

/************************************************************************/
/* create a thread
in
    func    : function address for thread
    param   : param that pass to the function
out
    ret     : handle of the thread
*/
/************************************************************************/
void* hw_create_thread( void*func , void*param );
/************************************************************************/
/* release thread
in
    handle  : handle of the thread
*/
/************************************************************************/
void  hw_release_thread( void*handle );


void hw_setpriority( void *handle, int priority);
/************************************************************************/
/* create a event object
out
    ret     : handle of the object
*/
/************************************************************************/
void* hw_create_event();

/************************************************************************/
/* set the event object
in
    handle  : handle of the object
*/
/************************************************************************/
int   hw_set_event( void*handle );

/************************************************************************/
/* wait for event triggle
in
    handle  : handle of the object
    timeout : time to wait
out
    ret     : 0 indicate success, 1 indicate time out, <0 indicate error
*/
/************************************************************************/
int   hw_wait_event( void*handle , int timeout );

/************************************************************************/
/* release event object
in
    handle  : handle of the object
*/
/************************************************************************/
int   hw_release_event( void*handle );

int   hw_get_framebuffer( unsigned int * framebuffer , unsigned int * width , unsigned int * height);

unsigned long     hw_get_sys_tick();
#define HW_TRACE_ERROR      0x01
#define HW_TRACE_IMPORTANT  0x02
#define HW_TRACE_TRACE      0x04
int   hw_trace_init( int type , int prio );
int   hw_trace_msg( int prio , char* fmt, ...);
int   hw_trace_end();
#ifdef __cplusplus
}
#endif
#endif

