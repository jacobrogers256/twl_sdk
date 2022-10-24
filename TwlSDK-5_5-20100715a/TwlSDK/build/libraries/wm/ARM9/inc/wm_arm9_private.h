/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - libraries
  File:     wm_arm9_private.h

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-19#$
  $Rev: 9342 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/

#ifndef LIBRARIES_WM_ARM9_WM_ARM9_PRIVATE_H__
#define LIBRARIES_WM_ARM9_WM_ARM9_PRIVATE_H__

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include    "wm_private.h"


/*---------------------------------------------------------------------------*
    Constant Definitions
 *---------------------------------------------------------------------------*/
#define     WM_GET_ALLOWED_CHANNEL_BEFORE_INIT      0x8000


/*---------------------------------------------------------------------------*
    Macro Definitions
 *---------------------------------------------------------------------------*/
#define     WM_CHECK_RESULT( res )      \
    if( ( res ) != WM_ERRCODE_SUCCESS ) \
    {                                   \
        return ( res );                 \
    }


#ifdef WM_DEBUG_DATASHARING
#define WM_DPRINTF_DATASHARING         WMi_Printf
#else
#define WM_DPRINTF_DATASHARING(...)    ((void) 0)
#endif
#ifdef WM_DEBUG_CALLBACK
#define WM_DPRINTF_CALLBACK            WMi_Printf
#else
#define WM_DPRINTF_CALLBACK(...)       ((void) 0)
#endif
#ifdef WM_DEBUG_AIDBITMAP
#define WM_DPRINTF_AIDBITMAP           WMi_Printf
#else
#define WM_DPRINTF_AIDBITMAP(...)      ((void) 0)
#endif


#define WMi_LOG(cat, msg) \
    do { s32 _SDK_WM_vcount = GX_GetVCount(); (void)WMi_Printf( "%02x:%03d| " cat msg "\n", ((OS_GetVBlankCount()+(_SDK_WM_vcount<192))&0xff), _SDK_WM_vcount );} while ( FALSE )

#define WMi_LOGF(cat, msg, ...) \
    do { s32 _SDK_WM_vcount = GX_GetVCount(); (void)WMi_Printf( "%02x:%03d| " cat msg "\n", ((OS_GetVBlankCount()+(_SDK_WM_vcount<192))&0xff), _SDK_WM_vcount, __VA_ARGS__ );} while ( FALSE )

#define WM_LOG(msg) WMi_LOG("", msg)
#define WM_LOGF(msg, ...) WMi_LOGF("", msg, __VA_ARGS__)

#ifdef WM_DEBUG
#define WM_DLOG  WM_LOG
#define WM_DLOGF WM_LOGF
#else
#define WM_DLOG(...)  ((void) 0)
#define WM_DLOGF(...) ((void) 0)
#endif
#ifdef WM_DEBUG_DATASHARING
#define WM_DLOG_DATASHARING(msg)  WMi_LOG("[DS]", msg);
#define WM_DLOGF_DATASHARING(msg, ...) WMi_LOGF("[DS]", msg, __VA_ARGS__);
#else
#define WM_DLOG_DATASHARING(...)  ((void) 0)
#define WM_DLOGF_DATASHARING(...) ((void) 0)
#endif
#ifdef WM_DEBUG_CALLBACK
#define WM_DLOG_CALLBACK  WM_LOG
#define WM_DLOGF_CALLBACK WM_LOGF
#define WM_DLOGF2_CALLBACK(from, msg, ...) \
    do { s32 _SDK_WM_vcount = GX_GetVCount(); (void)WMi_Printf( "%02x:%03d-%03d| " msg "\n", ((OS_GetVBlankCount()+(_SDK_WM_vcount<192))&0xff), from, _SDK_WM_vcount, __VA_ARGS__ );} while ( FALSE )
//    ((void) ( WMi_Printf( "%02x:%03d-%03d| " msg "\n", ((OS_GetVBlankCount()+(GX_GetVCount()<192))&0xff), from, GX_GetVCount(), __VA_ARGS__) ))
#else
#define WM_DLOG_CALLBACK(...)  ((void) 0)
#define WM_DLOGF_CALLBACK(...) ((void) 0)
#define WM_DLOGF2_CALLBACK(...) ((void) 0)
#endif
#ifdef WM_DEBUG_AIDBITMAP
#define WM_DLOG_AIDBITMAP(msg)  WMi_LOG("[AID]", msg);
#define WM_DLOGF_AIDBITMAP(msg, ...) WMi_LOGF("[AID]", msg, __VA_ARGS__);
#else
#define WM_DLOG_AIDBITMAP(...)  ((void) 0)
#define WM_DLOGF_AIDBITMAP(...) ((void) 0)
#endif


/*---------------------------------------------------------------------------*
    Private Function Prototypes
 *---------------------------------------------------------------------------*/
void    WMi_SetCallbackTable(WMApiid id, WMCallbackFunc callback);
WMErrCode WMi_SendCommand(WMApiid id, u16 paramNum, ...);
WMErrCode WMi_SendCommandDirect(void *data, u32 length);
WMArm9Buf *WMi_GetSystemWork(void);
WMErrCode WMi_CheckStateEx(s32 paramNum, ...);
WMErrCode WMi_CheckInitialized(void);
WMErrCode WMi_CheckIdle(void);
WMErrCode WMi_StartMP(WMCallbackFunc callback,
                      u16 *recvBuf,
                      u16 recvBufSize, u16 *sendBuf, u16 sendBufSize, WMMPTmpParam* tmpParam);

void WMi_RegisterSleepCallback(void);
void WMi_DeleteSleepCallback(void);

#ifdef SDK_TWL
BOOL WMi_CheckEnableFlag(void);
#endif

/*---------------------------------------------------------------------------*
    Inline Function Definitions
 *---------------------------------------------------------------------------*/
static inline WMErrCode WMi_CheckState(u32 state)
{
    return WMi_CheckStateEx(1, state);
}


/*===========================================================================*/

#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif /* LIBRARIES_WM_ARM9_WM_ARM9_PRIVATE_H__ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
