/*---------------------------------------------------------------------------*
  Project:  TwlSDK - wireless_shared - demos - wfs
  File:     wfs_common.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include <nitro.h>

#include "wfs.h"


/*---------------------------------------------------------------------------*/
/* Constants */

/* Wireless event */
typedef enum WirelessEvent
{
    WIRELESS_EVENT_END,             /* Quit */
    WIRELESS_EVENT_LINKUP,          /* Start MP communication */
    WIRELESS_EVENT_CONNECTED,       /* Connection notification */
    WIRELESS_EVENT_DISCONNECTED,    /* Disconnect notification */
    WIRELESS_EVENT_PACKET_SEND,     /* Send packet */
    WIRELESS_EVENT_PACKET_RECV      /* Receive packet */
}
WirelessEvent;


/*---------------------------------------------------------------------------*/
/* Declarations */

typedef void  (*WirelessEventCallback)(void *userdata, WirelessEvent event, void *argument);
typedef struct StaticWork
{
    /* Internal state management */
    WFSState                state;
    WFSStateCallback        state_func;
    u32                     available:1;
    u32                     mp_availale:1;
    u32                     initialized:1;
    u32                     linkup_done:1;
    u32                     mp_operating:1;
    u32                     debug_enable:1;
    u32                     flags:26;
    /* Allocator format change information */
    MIAllocator             allocator[1];
    WFSAllocator            user_alloc;
    void                   *user_alloc_arg;
    /* Dynamic memory for separate parent/child WFS processing */
    WirelessEventCallback   callback;
    void                   *context;
    WFSServerContext       *server;
    WFSClientContext       *client;
    u8                      padding[12];
    /* MP communication settings */
    int                     port;
    u8                      packet_buf[1024] ATTRIBUTE_ALIGN(32);
    WMStatus                status_buf[1] ATTRIBUTE_ALIGN(32);
}
StaticWork;


/*---------------------------------------------------------------------------*/
/* Variables */

static StaticWork           static_work[1] ATTRIBUTE_ALIGN(32);


/*---------------------------------------------------------------------------*/
/* Functions */

static void WFSi_PortSendCallback(void *callback);

/*---------------------------------------------------------------------------*
  Name:         InternalAlloc

  Description:  An internal function that converts the demo's old specification allocator.

  Arguments:    userdata:         StaticWork structure
                length:           The size to be allocated
                alignment:        The required byte-alignment (must be a power of 2)

  Returns:      Either the allocated memory or NULL.
 *---------------------------------------------------------------------------*/
static void *InternalAlloc(void *userdata, u32 length, u32 alignment)
{
    StaticWork *work = (StaticWork*)userdata;
    u32     newlen;
    void   *newptr;
    void   *aligned_ptr;
    alignment = MATH_ROUNDUP(alignment, sizeof(u32));
    newlen = MATH_ROUNDUP(length, alignment) + alignment;
    newptr = (*work->user_alloc)(work->user_alloc_arg, newlen, NULL);
    if (newptr)
    {
        aligned_ptr = (void*)MATH_ROUNDUP((u32)newptr + 1, alignment);
        ((u32*)aligned_ptr)[-1] = (u32)aligned_ptr - (u32)newptr;
    }
    return aligned_ptr;
}

/*---------------------------------------------------------------------------*
  Name:         InternalFree

  Description:  An internal function that converts the demo's old specification allocator.

  Arguments:    userdata:         StaticWork structure
                buffer:           The memory to deallocate

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InternalFree(void *userdata, void *buffer)
{
    StaticWork *work = (StaticWork*)userdata;
    if (buffer)
    {
        void   *rare_ptr = (u8*)buffer - ((u32*)buffer)[-1];
        (void)(*work->user_alloc)(work->user_alloc_arg, 0, rare_ptr);
    }
}

/*---------------------------------------------------------------------------*
  Name:         NotifyWirelessEvent

  Description:  Wireless event notification.

  Arguments:    work:             StaticWork structure
                event:         Notified event
                argument:         Event argument

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE
void NotifyWirelessEvent(StaticWork *work, WirelessEvent event, void *argument)
{
    (*work->callback)(work->context, event, argument);
}

/*---------------------------------------------------------------------------*
  Name:         WFSi_NotifySend

  Description:  Send packet notification.

  Arguments:    work:             StaticWork structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WFSi_NotifySend(StaticWork *work)
{
    if (work->mp_availale && work->linkup_done && !work->mp_operating)
    {
        /* Notify of a send packet event at the maximum MP setting currently possible. */
        WFSPacketBuffer packet[1];
        packet->buffer = work->packet_buf;
        packet->bitmap = (work->status_buf->aid == 0) ? 0xFFFE : 0x0001;
        packet->length = (work->status_buf->aid == 0) ?
            work->status_buf->mp_parentMaxSize :
            work->status_buf->mp_childMaxSize;
        NotifyWirelessEvent(work, WIRELESS_EVENT_PACKET_SEND, packet);
        /* If the packets that should be sent are specified, send them. */
        if (packet->length > 0)
        {
            int     ret = WM_SetMPDataToPortEx(WFSi_PortSendCallback,
                                               work,
                                               (const u16 *)packet->buffer,
                                               (u16)packet->length,
                                               (u16)packet->bitmap,
                                               (u16)work->port,
                                               WM_PRIORITY_LOW);
            work->mp_operating = (ret == WM_ERRCODE_OPERATING);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WFSi_NotifyLinkup

  Description:  Linkup notification.

  Arguments:    wireless:         StaticWork structure.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WFSi_NotifyLinkup(StaticWork *work)
{
    if (work->mp_availale && work->initialized && !work->linkup_done)
    {
        WFSPeerInfo peer[1];
        peer->aid = work->status_buf->aid;
        MI_CpuCopy8(work->status_buf->MacAddress, peer->mac, 6);
        NotifyWirelessEvent(work, WIRELESS_EVENT_LINKUP, peer);
        work->linkup_done = TRUE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         WFSi_PortSendCallback

  Description:  Process when SetMPData completes.

  Arguments:    arg:              WMPortSendCallback structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WFSi_PortSendCallback(void *arg)
{
    const WMPortSendCallback * const callback = (const WMPortSendCallback *)arg;
    StaticWork * const work = callback->arg;
    if (work->mp_availale)
    {
        work->mp_operating = FALSE;
        WFSi_NotifySend(work);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WFSi_PortCallback

  Description:  Port reception callback.

  Arguments:    arg:              WMPortRecvCallback structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WFSi_PortCallback(void *arg)
{
    const WMPortRecvCallback *const callback = (const WMPortRecvCallback *)arg;
    StaticWork * const              work = callback->arg;
    if (work->mp_availale && work->linkup_done)
    {
        /* Convert to a callback format that matches WFS */
        WFSPeerInfo     peer[1];
        WFSPacketBuffer packet[1];
        peer->aid = callback->aid;
        MI_CpuCopy8(callback->macAddress, peer->mac, 6);
        packet->buffer = (u8 *)callback->data;
        packet->length = callback->length;
        packet->bitmap = (1 << peer->aid);
        switch (callback->state)
        {
        case WM_STATECODE_CONNECTED:
            NotifyWirelessEvent(work, WIRELESS_EVENT_CONNECTED, peer);
            break;

        case WM_STATECODE_DISCONNECTED:
        case WM_STATECODE_DISCONNECTED_FROM_MYSELF:
            NotifyWirelessEvent(work, WIRELESS_EVENT_DISCONNECTED, peer);
            break;

        case WM_STATECODE_PORT_RECV:
            NotifyWirelessEvent(work, WIRELESS_EVENT_PACKET_RECV, packet);
            /* If MP was stopped, start it up here */
            WFSi_NotifySend(work);
            break;

        default:
            break;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WFSi_InitCommon

  Description:  Common WFS parent/child initialization processing.

  Arguments:    callback:         Status notification callback
                allocator:        Memory allocator
                allocator_arg:    Allocator arguments
                port:             Port number to use
                worksize:         Separate parent/child work memory size
                wireless_proc:    Separate parent/child wireless event handler

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WFSi_InitCommon(WFSStateCallback callback,
                            WFSAllocator allocator, void *allocator_arg,
                            int port, u32 worksize,
                            WirelessEventCallback wireless_proc)
{
    /* FS library is necessary for both the parent and child */
    if (!FS_IsAvailable())
    {
        OS_TWarning("FS is initialized by WFS (with FS_DMA_NOT_USE)");
        FS_Init(FS_DMA_NOT_USE);
    }
    {
        OSIntrMode  prev_irq = OS_DisableInterrupts();
        if (!static_work->available)
        {
            /* Initialize the allocator */
            static_work->user_alloc = allocator;
            static_work->user_alloc_arg = allocator_arg;
            MI_InitAllocator(static_work->allocator, static_work,
                             InternalAlloc, InternalFree);
            /* Allocate dynamic memory for the parent and child separately */
            static_work->context = MI_CallAlloc(static_work->allocator, worksize, 32);
            if (!static_work->context)
            {
                OS_TPanic("cannot allocate enough memory for WFS");
            }
            /* Initialize the internal status */
            static_work->available = TRUE;
            static_work->state = WFS_STATE_STOP;
            static_work->state_func = callback;
            static_work->debug_enable = FALSE;
            static_work->mp_availale = FALSE;
            static_work->initialized = FALSE;
            static_work->linkup_done = FALSE;
            static_work->mp_operating = FALSE;
            static_work->callback = wireless_proc;
            static_work->port = port;
            static_work->status_buf->aid = 0;
            static_work->status_buf->mp_parentMaxSize = 0;
            static_work->status_buf->mp_childMaxSize = 0;
        }
        (void)OS_RestoreInterrupts(prev_irq);
    }
}


/*---------------------------------------------------------------------------*
  Name:         ParentWFSEvent

  Description:  WFS server event notification.

  Arguments:    userdata: User-defined argument
                event:             Event type
                argument:         Arguments unique to event

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ParentWFSEvent(void *userdata, WFSEventType event, void *argument)
{
    /* Because we are using the WFS_ExecuteRomServerThread function, there is nothing in particular that needs to be done here */
    (void)userdata;
    (void)event;
    (void)argument;
}

/*---------------------------------------------------------------------------*
  Name:         ParentWirelessEvent

  Description:  Event notification from the wireless driver.

  Arguments:    userdata: User-defined argument
                event:             Event type
                argument:         Arguments unique to event

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ParentWirelessEvent(void *userdata, WirelessEvent event, void *argument)
{
    WFSServerContext * const context = (WFSServerContext *)userdata;
    switch (event)
    {
    case WIRELESS_EVENT_END:
        WFS_EndServer(context);
        break;

    case WIRELESS_EVENT_LINKUP:
        /* Immediately move to the READY state */
        static_work->state = WFS_STATE_READY;
        if (static_work->state_func)
        {
            (*static_work->state_func) (NULL);
        }
        break;

    case WIRELESS_EVENT_CONNECTED:
        WFS_CallServerConnectHook(context, (const WFSPeerInfo*)argument);
        break;

    case WIRELESS_EVENT_DISCONNECTED:
        WFS_CallServerDisconnectHook(context, (const WFSPeerInfo*)argument);
        break;

    case WIRELESS_EVENT_PACKET_SEND:
        WFS_CallServerPacketSendHook(context, (WFSPacketBuffer *)argument);
        break;

    case WIRELESS_EVENT_PACKET_RECV:
        WFS_CallServerPacketRecvHook(context, (const WFSPacketBuffer *)argument);
        break;

    default:
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         ChildWFSEvent

  Description:  WFS client event notification.

  Arguments:    userdata: User-defined argument
                event:             Event type
                argument:         Arguments unique to event

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ChildWFSEvent(void *userdata, WFSEventType event, void *argument)
{
    switch (event)
    {
    case WFS_EVENT_CLIENT_READY:
        /* Completion of ROM file table recepetion */
        {
            /* Replace the current "rom" archive with WFS */
            WFSClientContext * const context = (WFSClientContext*)userdata;
            WFS_ReplaceRomArchive(context);
            /* Notify the application that WFS has become available for use */
            static_work->state = WFS_STATE_READY;
            if (static_work->state_func)
            {
                (*static_work->state_func) (NULL);
            }
        }
        break;
    default:
        break;
    }
    (void)argument;
}

/*---------------------------------------------------------------------------*
  Name:         ChildWirelessEvent

  Description:  Event notification from the wireless driver.

  Arguments:    userdata: User-defined argument
                event:             Event type
                argument:         Arguments unique to event

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ChildWirelessEvent(void *userdata, WirelessEvent event, void *argument)
{
    WFSClientContext * const context = (WFSClientContext *)userdata;
    switch (event)
    {
    case WIRELESS_EVENT_END:
        WFS_EndClient(context);
        break;

    case WIRELESS_EVENT_LINKUP:
        WFS_StartClient(context, (const WFSPeerInfo*)argument);
        break;

    case WIRELESS_EVENT_CONNECTED:
        WFS_CallClientConnectHook(context, (const WFSPeerInfo*)argument);
        break;

    case WIRELESS_EVENT_DISCONNECTED:
        WFS_CallClientDisconnectHook(context, (const WFSPeerInfo*)argument);
        break;

    case WIRELESS_EVENT_PACKET_SEND:
        WFS_CallClientPacketSendHook(context, (WFSPacketBuffer*)argument);
        break;

    case WIRELESS_EVENT_PACKET_RECV:
        WFS_CallClientPacketRecvHook(context, (const WFSPacketBuffer*)argument);
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         WFS_InitParent

  Description:  Initializes Wireless File System parent parameters.

  Arguments:    port             Available WM port for WFS
                callback:         Callback of various state-notifications
                allocator        Dynamic memory allocator for WFS
                allocator_arg    Argument of 'allocator'
                parent_packet    Available WM packet for WFS
                p_rom:            Target file that directs SRL-program
                                 to export FAT/FNT and OVT.
                                 If specified NULL, use own.
                use_parent_fs:    If TRUE, ignore p_rom's FAT/FNT and use own.
                                 This setting enables both independent
                                 program to share FAT/FNT (except OVT).
                                 If p_rom is NULL, always as TRUE.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WFS_InitParent(int port, WFSStateCallback callback,
                    WFSAllocator allocator, void *allocator_arg, int parent_packet,
                    FSFile *p_rom, BOOL use_parent_fs)
{
    /* Common initialization */
    WFSi_InitCommon(callback, allocator, allocator_arg, port,
                    sizeof(WFSServerContext), ParentWirelessEvent);
    /* WFS server initialization */
    {
        WFSServerContext   *server = (WFSServerContext*)static_work->context;
        static_work->server = server;
        WFS_InitServer(server, server, ParentWFSEvent, static_work->allocator, parent_packet);
        if (!WFS_ExecuteRomServerThread(server, p_rom, use_parent_fs))
        {
            OS_TPanic("WFS_ExecuteRomServerThread() failed! (specified SRL file seems to be wrong)");
        }
    }
    static_work->state = WFS_STATE_IDLE;
    static_work->initialized = TRUE;
    /* Start sending packets if already in the communication-enabled state during initialization */
    WFSi_NotifyLinkup(static_work);
}

/*---------------------------------------------------------------------------*
  Name:         WFS_InitChild

  Description:  Initialization of a child device to use WFS.

  Arguments:    port:             WM port number assigned for WFS
                callback:         Status notification callback
                allocator:        Allocator for WFS
                allocator_arg:    Allocator arguments

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WFS_InitChild(int port, WFSStateCallback callback, WFSAllocator allocator, void *allocator_arg)
{
    /* Common initialization */
    WFSi_InitCommon(callback, allocator, allocator_arg, port,
                    sizeof(WFSClientContext), ChildWirelessEvent);
    /* WFS client initialization */
    {
        WFSClientContext   *client = (WFSClientContext*)static_work->context;
        static_work->client = client;
        WFS_InitClient(client, client, ChildWFSEvent, static_work->allocator);
    }
    static_work->state = WFS_STATE_IDLE;
    static_work->initialized = TRUE;
    /* Start sending packets if already in the communication-enabled state during initialization */
    WFSi_NotifyLinkup(static_work);
}

/*---------------------------------------------------------------------------*
  Name:         WFS_Start

  Description:  Notify that WFS is able to use MP communication.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WFS_Start(void)
{
    StaticWork *work = static_work;
    OSIntrMode bak_cpsr = OS_DisableInterrupts();
    if (work->available && !work->mp_availale)
    {
        /* Set the port callback for WM */
        int     result;
        result = WM_SetPortCallback((u16)work->port, WFSi_PortCallback, work);
        if (result != WM_ERRCODE_SUCCESS)
        {
            OS_TPanic("WM_SetPortCallback failed! (result=%d)", result);
        }
        /* Get the WM internal state */
        result = WM_ReadStatus(work->status_buf);
        if (result != WM_ERRCODE_SUCCESS)
        {
            OS_TPanic("WM_ReadStatus failed! (result=%d)", result);
        }
        work->mp_availale = TRUE;
        WFSi_NotifyLinkup(work);
        WFSi_NotifySend(work);
    }
    (void)OS_RestoreInterrupts(bak_cpsr);
}

/*---------------------------------------------------------------------------*
  Name:         WFS_End

  Description:  End WFS.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WFS_End(void)
{
    OSIntrMode bak_cpsr = OS_DisableInterrupts();
    if (static_work->available)
    {
        StaticWork *const work = static_work;
        work->state = WFS_STATE_STOP;
        work->state_func = NULL;
        work->available = FALSE;
        /* Release the WM port callback */
        if (work->mp_availale)
        {
            (void)WM_SetPortCallback((u16)work->port, NULL, NULL);
            work->mp_availale = FALSE;
            work->status_buf->aid = 0;
        }
        /* Separate parent/child shutdown processing */
        if (work->initialized)
        {
            NotifyWirelessEvent(work, WIRELESS_EVENT_END, NULL);
            static_work->server = NULL;
            static_work->client = NULL;
            work->initialized = FALSE;
        }
        /* Release the allocated dynamic memory */
        MI_CallFree(work->allocator, work->context);
    }
    (void)OS_RestoreInterrupts(bak_cpsr);
}

/*---------------------------------------------------------------------------*
  Name:         WFS_GetStatus

  Description:  Gets the current WFS status.

  Arguments:    None.

  Returns:      The current WFS status.
 *---------------------------------------------------------------------------*/
WFSState WFS_GetStatus(void)
{
    return static_work->state;
}

/*---------------------------------------------------------------------------*
  Name:         WFS_SetDebugMode

  Description:  Enables/Disables WFS internal debug output.

  Arguments:    enable:            TRUE if enabling debug output

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WFS_SetDebugMode(BOOL enable)
{
    static_work->debug_enable = enable;
}

/*---------------------------------------------------------------------------*
  Name:         WFS_GetCurrentBitmap

  Description:  Gets the current child device group bitmap that WBT is aware of.
                This function can only be called by the parent.

  Arguments:    None.

  Returns:      Currently recognized child devices.
 *---------------------------------------------------------------------------*/
int WFS_GetCurrentBitmap(void)
{
    return static_work->server ? WFS_GetServerConnectedBitmap(static_work->server) : 0;
}

/*---------------------------------------------------------------------------*
  Name:         WFS_GetSyncBitmap

  Description:  Gets bitmap of child group with access sync specified.
                This function can only be called by the parent.

  Arguments:    None.

  Returns:      Child devices that are specified to synchronize access
 *---------------------------------------------------------------------------*/
int WFS_GetSyncBitmap(void)
{
    return static_work->server ? WFS_GetServerSyncBitmap(static_work->server) : 0;
}

/*---------------------------------------------------------------------------*
  Name:         WFS_GetBusyBitmap

  Description:  Gets bitmap of child devices that are currently accessing parent.
                This function can only be called by the parent.

  Arguments:    None.

  Returns:      Child devices that are currently accessing parent.
 *---------------------------------------------------------------------------*/
int WFS_GetBusyBitmap(void)
{
    return static_work->server ? WFS_GetServerBusyBitmap(static_work->server) : 0;
}

/*---------------------------------------------------------------------------*
  Name:         WFS_IsBusy

  Description:  Determines whether or not the child device of a designated aid is being accessed.
                This function can only be called by the parent.

  Arguments:    aid:              AID of child to check

  Returns:      If the designated child device is being accessed, TRUE. Otherwise, FALSE.
 *---------------------------------------------------------------------------*/
BOOL WFS_IsBusy(int aid)
{
    return ((1 << aid) & WFS_GetBusyBitmap()) != 0;
}

/*---------------------------------------------------------------------------*
  Name:         WFS_GetPacketSize

  Description:  Gets MP communication packet size of parent set in WFS.

  Arguments:    None.

  Returns:      MP communication packet size of parent set in WFS.
 *---------------------------------------------------------------------------*/
int WFS_GetPacketSize(void)
{
    return static_work->server ? WFS_GetServerPacketLength(static_work->server) : 0;
}

/*---------------------------------------------------------------------------*
  Name:         WFS_SetPacketSize

  Description:  Sets the packet size to send from the parent.
                This function can only be called before calling WFS_Start() or after all child devices have properly finished connecting.
                
                
                This function is not guaranteed to work if called at any other time.
                This function can only be called by the parent.

  Arguments:    size:             The specified packet size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WFS_SetPacketSize(int size)
{
    if(static_work->server)
    {
        WFS_SetServerPacketLength(static_work->server, size);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WFS_EnableSync

  Description:  Configures the settings of the child device group that takes access synchronization on the parent device side.
                This function achieves efficient transmission rates that utilities unique characteristics of the WBT library; this is done by synchronizing responses to child devices that are all guaranteed to access the same files in precisely the same order.
                
                
                However, be cautious of the fact that if the synchronization start timing is not logically safe, responses to child devices will become out-of-sync, causing deadlocks to occur.
                
                This function can only be called by the parent.

  Arguments:    sync_bitmap:     The AID bitmap of the child device group that takes the access synchronization.
                                 The lowest bit 1 indicating the parent device itself is ignored.
                                 By assigning 0 for this value, synchronicity does not occur.
                                 This is the default state.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WFS_EnableSync(int sync_bitmap)
{
    if(static_work->server)
    {
        WFS_SetServerSync(static_work->server, sync_bitmap);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WFS_GetCurrentDownloadProgress

  Description:  Gets the progress status of the file on which ReadFile is currently being executed.

  Arguments:    current:          The number of packets that have been received
                total:            The total number of packets expected to receive

  Returns:      TRUE if ReadFile is currently executing.
 *---------------------------------------------------------------------------*/
BOOL WFS_GetCurrentDownloadProgress(int *current, int *total)
{
    return static_work->client && (WFS_GetClientReadProgress(static_work->client, current, total), TRUE);
}
