/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - snd
  File:     capture.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#include <nnsys/snd/capture.h>

#include <nitro/mi.h>
#include <nitro/snd.h>
#include <nnsys/misc.h>
#include <nnsys/snd/resource_mgr.h>
#include <nnsys/snd/fader.h>

/******************************************************************************
	Macro Definitions
 ******************************************************************************/

#define CAPTURE_0 ( 1 << 0 )
#define CAPTURE_1 ( 1 << 1 )

#define THREAD_STACK_SIZE      (1024*4)
#define THREAD_MESSAGE_BUF_SIZE   8

/******************************************************************************
	Structure Definitions
 ******************************************************************************/

typedef struct CaptureParam
{
    BOOL activeFlag;
    NNSSndCaptureType type;
    NNSSndCaptureFormat format;
    void* bufferL;
    void* bufferR;
    u32 bufLen;
    u32 blockSize;  // size of 1 block
    int curBuffer;
    
    u32 chBitMask;
    u32 playChBitMask;
    u32 capBitMask;
    int alarmNo;
    
    int interval;
    NNSSndCaptureCallback callback;
    void* callbackArg;
    
    NNSSndFader fader;
    BOOL fadeOutFlag;
    int volume;
} CaptureParam;

typedef struct EffectInfo
{
    CaptureParam* cap;
    u32 blockSize;
    u32 offset;
    void* bufL_p;
    void* bufR_p;
} EffectInfo;

/******************************************************************************
	Static Variables
 ******************************************************************************/

static CaptureParam sCapParam;

static OSThread sThread;
static u64 sThreadStack[ THREAD_STACK_SIZE / sizeof(u64) ];
static OSMessageQueue sMesgQ;
static OSMessage sMesgBuf[ THREAD_MESSAGE_BUF_SIZE ];

static volatile BOOL sIsThreadCreated = FALSE;
static EffectInfo sEffectInfoBuffer[ THREAD_MESSAGE_BUF_SIZE ];
static int sCurEffectInfo;

/******************************************************************************
	Static Function Declarations
 ******************************************************************************/
static void AlarmCallback( void* arg );
static void CaptureThread( void* arg );

/******************************************************************************
	public functions
 ******************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         NNS_SndCaptureStartReverb

  Description:  start reverb

  Arguments:    buffer_p   - reverb buffer top address
                bufSize    - reverb buffer size
                format   - capture format
                sampleRate - sampling rate
                volume     - reverb component volume

  Returns:      Whether it was successful or not.
 *---------------------------------------------------------------------------*/
BOOL NNS_SndCaptureStartReverb(    
    void* buffer_p,
    u32 bufSize,
    NNSSndCaptureFormat format,
    int sampleRate,
    int volume
)
{
    void* bufL_p;
    void* bufR_p;

    NNS_NULL_ASSERT( buffer_p );
    NNS_ASSERTMSG( ((u32)buffer_p & 31) == 0, "buffer address must be aligned to 32 bytes boundary." );
    NNS_ASSERTMSG( ((u32)bufSize & 63) == 0, "buffer size must be aligned to 64 bytes boundary." );
    NNS_MINMAX_ASSERT( bufSize, 0, 0x7ffc0 );
    NNS_MINMAX_ASSERT( volume, 0, 63 );
    
	NNS_SndCaptureStopReverb( 0 );
    
    if ( sCapParam.activeFlag ) return FALSE;
    
    bufL_p = buffer_p;
    bufR_p = (u8*)buffer_p + bufSize / 2;
    
    MI_CpuClear32( buffer_p, bufSize );
    DC_FlushRange( buffer_p, bufSize );   
    
    return NNSi_SndCaptureStart(
        NNS_SND_CAPTURE_TYPE_REVERB,
        bufL_p,
        bufR_p,
        bufSize / 2,
        format,
        SND_CAPTURE_IN_MIXER,
        SND_CAPTURE_OUT_NORMAL,
        TRUE,  // loop
        sampleRate,
        volume,
        0,     // pan-L
        127,   // pan-R
        0,     // interval
        NULL,  // callback
        0      // arg
    );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndCaptureSetReverbVolume

  Description:  volume setting of reverb component

  Arguments:    volume - target volume
                frames - number of frames to change

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndCaptureSetReverbVolume( int volume, int frames )
{
    CaptureParam* cap = & sCapParam;
    
    NNS_MINMAX_ASSERT( volume, 0, 63 );
    
    if ( ! cap->activeFlag ) return;
    if ( cap->type != NNS_SND_CAPTURE_TYPE_REVERB ) return;
    
    NNSi_SndFaderSet( & cap->fader, volume << 8, frames );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndCaptureStopReverb

  Description:  stops reverb

  Arguments:    frames - fade out frame

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndCaptureStopReverb( int frames )
{
    CaptureParam* cap = & sCapParam;
    
    if ( ! cap->activeFlag ) return;
    if ( cap->type != NNS_SND_CAPTURE_TYPE_REVERB ) return;    
    
    if ( frames == 0 ) {
        NNSi_SndCaptureStop();
        return;
    }
    
    NNSi_SndFaderSet( & cap->fader, 0, frames );
    
    cap->fadeOutFlag = TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndCaptureStartEffect

  Description:  Starts an effect.

  Arguments:    buffer_p - effect buffer start address
                bufSize  - effect buffer size
                format   - capture format
                sampleRate - sampling rate
                interval - callback interval
                callback - callback function
                arg      - callback argument

  Returns:      Whether it was successful or not.
 *---------------------------------------------------------------------------*/
BOOL NNS_SndCaptureStartEffect(    
    void* buffer_p,
    u32 bufSize,
    NNSSndCaptureFormat format,
    int sampleRate,
    int interval,
    NNSSndCaptureEffectCallback callback,
    void* arg
)
{
    void* bufL_p;
    void* bufR_p;
    
    NNS_SndCaptureStopEffect();
    
    if ( sCapParam.activeFlag ) return FALSE;
    
    bufL_p = buffer_p;
    bufR_p = (u8*)buffer_p + bufSize / 2;
    
    MI_CpuClear32( buffer_p, bufSize );
    DC_FlushRange( buffer_p, bufSize );
    
    return NNSi_SndCaptureStart(
        NNS_SND_CAPTURE_TYPE_EFFECT,
        bufL_p,
        bufR_p,
        bufSize / 2,
        format,
        SND_CAPTURE_IN_MIXER,
        SND_CAPTURE_OUT_NORMAL,
        TRUE,  // loop
        sampleRate,
        127,
        0,     // pan-L
        127,   // pan-R
        interval,
        callback,
        arg
    );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndCaptureStopEffect

  Description:  Stops effect.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndCaptureStopEffect( void )
{
    if ( ! sCapParam.activeFlag ) return;
    if ( sCapParam.type != NNS_SND_CAPTURE_TYPE_EFFECT ) return;
    
    NNSi_SndCaptureStop();
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndCaptureStartSampling

  Description:  Starts sampling.

  Arguments:    buffer_p - sampling buffer start address
                bufSize  - sampling buffer size
                format   - capture format
                sampleRate - sampling rate
                interval - callback interval
                callback - callback function
                arg      - callback argument

  Returns:      Whether it was successful or not.
 *---------------------------------------------------------------------------*/
BOOL NNS_SndCaptureStartSampling(    
    void* buffer_p,
    u32 bufSize,
    NNSSndCaptureFormat format,
    int sampleRate,
    int interval,
    NNSSndCaptureCallback callback,
    void* arg
)
{
    void* bufL_p;
    void* bufR_p;
    
    NNS_SndCaptureStopSampling();
    
    if ( sCapParam.activeFlag ) return FALSE;
    
    bufL_p = buffer_p;
    bufR_p = (u8*)buffer_p + bufSize / 2;
    
    MI_CpuClear32( buffer_p, bufSize );
    DC_FlushRange( buffer_p, bufSize );
    
    return NNSi_SndCaptureStart(
        NNS_SND_CAPTURE_TYPE_SAMPLING,
        bufL_p,
        bufR_p,
        bufSize / 2,
        format,
        SND_CAPTURE_IN_MIXER,
        SND_CAPTURE_OUT_NORMAL,
        TRUE,  // loop
        sampleRate,
        127,
        0,     // pan-L
        127,   // pan-R
        interval,
        callback,
        arg
    );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndCaptureStopSampling

  Description:  Stops sampling.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndCaptureStopSampling( void )
{
    if ( ! sCapParam.activeFlag ) return;
    if ( sCapParam.type != NNS_SND_CAPTURE_TYPE_SAMPLING ) return;
    
    NNSi_SndCaptureStop();
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndCaptureCreateThread

  Description:  Creates capture thread.

  Arguments:    threadPrio - thread priority

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndCaptureCreateThread( u32 threadPrio )
{
    NNS_ASSERT( OS_IsThreadAvailable() );
    
    if ( sIsThreadCreated ) return;
    
    sCurEffectInfo = 0;
    
    OS_InitMessageQueue( & sMesgQ, sMesgBuf, THREAD_MESSAGE_BUF_SIZE );
    
    OS_CreateThread(
        &sThread,
        CaptureThread,
        NULL,
        sThreadStack + THREAD_STACK_SIZE / sizeof(u64),
        THREAD_STACK_SIZE,
        threadPrio
    );
    
    sIsThreadCreated = TRUE;
    
    OS_WakeupThreadDirect( &sThread );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndCaptureDestroyThread

  Description:  Gets rid of the capture thread.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndCaptureDestroyThread()
{
    BOOL result;
    
    if ( ! sIsThreadCreated ) return;
    
    sIsThreadCreated = FALSE;
    
    result = OS_SendMessage( & sMesgQ, (OSMessage)NULL, OS_MESSAGE_BLOCK );
    NNS_ASSERT( result );
    OS_JoinThread( &sThread );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndCaptureIsActive

  Description:  Gets whether capture is in use.

  Arguments:    None.

  Returns:      In use?
 *---------------------------------------------------------------------------*/
BOOL NNS_SndCaptureIsActive( void )
{
    return sCapParam.activeFlag;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndCaptureGetCaptureType

  Description:  Gets type of capture being used currently.

  Arguments:    None.

  Returns:      The type of capture being used currently.
 *---------------------------------------------------------------------------*/
NNSSndCaptureType NNS_SndCaptureGetCaptureType( void )
{
    NNS_ASSERT( NNS_SndCaptureIsActive() );
    
    return sCapParam.type;
}

/******************************************************************************
	Private function
 ******************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         NNSi_SndCaptureInit

  Description:  Initializes the capture library.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNSi_SndCaptureInit( void )
{
    sIsThreadCreated = FALSE;
    
    sCapParam.activeFlag = FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_SndCaptureMain

  Description:  capture library framework

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNSi_SndCaptureMain( void )
{
    CaptureParam* cap;
    NNSSndFader* fader;
    int volume;
    
    cap = & sCapParam;
    
    if ( cap->activeFlag && cap->type == NNS_SND_CAPTURE_TYPE_REVERB )
    {
        fader = & cap->fader;
        
        // fader process
        NNSi_SndFaderUpdate( fader );
        
        // Fadeout completion check.
        if ( cap->fadeOutFlag )
        {
            if ( NNSi_SndFaderIsFinished( fader ) )
            {
                NNSi_SndCaptureStop();
                return;
            }
        }
        
        volume = ( NNSi_SndFaderGet( fader ) >> 8 );
        
        if ( volume != cap->volume )
        {
            SND_SetChannelVolume(
                cap->playChBitMask,
                volume,
                SND_CHANNEL_DATASHIFT_NONE
            );
            
            cap->volume = volume;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_SndCaptureStart

  Description:  Starts capture.

  Arguments:    type - capture type
                buffer0 - capture buffer 0 start address
                buffer1 - capture buffer 1 start address
                bufLen  - capture buffer size
                format   - capture format
                input   - capture input settings
                output  - capture output settings
                loopFlag - loop flag
                sampleRate - sampling rate
                volume   - output volume
                pan0     - capture 0 output pan
                pan1     - capture 1 output pan
                interval - callback interval
                callback - callback function
                arg      - callback argument

  Returns:      Whether it was successful or not.
 *---------------------------------------------------------------------------*/
BOOL NNSi_SndCaptureStart(
    NNSSndCaptureType type,
    void* buffer0,
    void* buffer1,
    u32 bufLen,
    NNSSndCaptureFormat format,
    SNDCaptureIn input,
	SNDCaptureOut output,
    BOOL loopFlag,
    int sampleRate,
    int volume,
    int pan0,
    int pan1,
    int interval,
    NNSSndCaptureCallback callback,
    void* arg
)
{
    SNDWaveFormat wave_format;
    SNDCaptureFormat capture_format;
    u32 chBitMask = 0;
    u32 playChBitMask = 0;
    u32 capBitMask = 0;
    int alarmNo = -1;
    unsigned int samples;
    int timer;
    u32 alarmTimer;
    u32 alarmOffset;
    BOOL is8bit;
    CaptureParam* cap = & sCapParam;
    
    NNS_ASSERTMSG( ( (u32)buffer0 & 0x1f ) == 0, "buffer address must be aligned to 32 bytes boundary." );
    NNS_ASSERTMSG( ( (u32)buffer1 & 0x1f ) == 0, "buffer address must be aligned to 32 bytes boundary." );
    NNS_ASSERTMSG( ( bufLen & 0x1f ) == 0, "buffer size must be aligned to 32 bytes boundary." );
    NNS_MINMAX_ASSERT( pan0, SND_CHANNEL_PAN_MIN, SND_CHANNEL_PAN_MAX );
    NNS_MINMAX_ASSERT( pan1, SND_CHANNEL_PAN_MIN, SND_CHANNEL_PAN_MAX );
    NNS_MAX_ASSERT( bufLen >> 2, SND_CAPTURE_LEN_MAX );
    
    DC_FlushRange( buffer0, bufLen );
    DC_FlushRange( buffer1, bufLen );
    
    /* parameter calculations */
    is8bit = format == NNS_SND_CAPTURE_FORMAT_PCM8 ? TRUE: FALSE;
    
    timer = SND_TIMER_CLOCK / sampleRate;
    NNS_MINMAX_ASSERT( timer, SND_CHANNEL_TIMER_MIN, SND_CHANNEL_TIMER_MAX );
    
    if ( callback != NULL )
    {        
        NNS_ASSERTMSG( ( ( bufLen / interval ) & 0x1f ) == 0, "buffer size divided by inverval must be aligned to 32 bytes boundary." );
        
        samples = bufLen;
        if ( ! is8bit ) samples >>= 1;
        NNS_ASSERTMSG( ( samples % interval ) == 0, "interval must be a multiple of sample count." );
        
        timer = ( ( timer + 16 ) & ~0x1f );
        alarmTimer = ( timer >> 5 ) * ( samples / interval );
        
        alarmOffset = 32; // FIFO size
        if ( ! is8bit ) alarmOffset >>= 1; // byte to sample
        alarmOffset *= ( timer >> 5 );
    }
    
    wave_format    = is8bit ? SND_WAVE_FORMAT_PCM8    : SND_WAVE_FORMAT_PCM16;
    capture_format = is8bit ? SND_CAPTURE_FORMAT_PCM8 : SND_CAPTURE_FORMAT_PCM16;
    
    chBitMask  |= ( 1 << SND_CAPTURE_0_OUT_CHANNEL ) | ( 1 << SND_CAPTURE_1_OUT_CHANNEL );
    capBitMask |= ( 1 << SND_CAPTURE_0 ) | ( 1 << SND_CAPTURE_1 );

    if ( type != NNS_SND_CAPTURE_TYPE_SAMPLING ) {
        playChBitMask = chBitMask;
    }
    
    /* resource lock */    
    if ( callback != NULL )
    {
        alarmNo = NNS_SndAllocAlarm();
        if ( alarmNo < 0 ) return FALSE;
    }
    if ( ! NNS_SndLockCapture( capBitMask ) ) {
        if ( alarmNo >= 0 ) NNS_SndFreeAlarm( alarmNo );
        return FALSE;
    }
    if ( ! NNS_SndLockChannel( chBitMask ) ) {
        if ( alarmNo >= 0 ) NNS_SndFreeAlarm( alarmNo );
        NNS_SndUnlockCapture( capBitMask );
        return FALSE;
    }
    
    /* channel and capture settings */
    SND_SetupChannelPcm(
        SND_CAPTURE_0_OUT_CHANNEL,
        wave_format,
        buffer0,
        loopFlag ? SND_CHANNEL_LOOP_REPEAT : SND_CHANNEL_LOOP_1SHOT,
        0,
        (int)( bufLen >> 2 ),
        volume,
        SND_CHANNEL_DATASHIFT_NONE,
        timer,
        pan0
    );
    SND_SetupCapture(
        SND_CAPTURE_0,
        capture_format,
        buffer0,
        bufLen >> 2,
        loopFlag,
        input,
        output
    );
    SND_SetupChannelPcm(
        SND_CAPTURE_1_OUT_CHANNEL,
        wave_format,
        buffer1,
        loopFlag ? SND_CHANNEL_LOOP_REPEAT : SND_CHANNEL_LOOP_1SHOT,
        0,
        (int)( bufLen >> 2 ),
        volume,
        SND_CHANNEL_DATASHIFT_NONE,
        timer,
        pan1
    );
    SND_SetupCapture(
        SND_CAPTURE_1,
        capture_format,
        buffer1,
        bufLen >> 2,
        loopFlag,
        input,
        output
    );
    
    if ( alarmNo >= 0 )
    {
        SND_SetupAlarm(
            alarmNo,
            alarmTimer + alarmOffset,
            alarmTimer,
            AlarmCallback,
            cap
        );
    }
    
    /* output selector settings */
    if ( type == NNS_SND_CAPTURE_TYPE_EFFECT )
    {
        SND_SetOutputSelector(
            SND_OUTPUT_CHANNEL1,
            SND_OUTPUT_CHANNEL3,
            SND_CHANNEL_OUT_BYPASS,
            SND_CHANNEL_OUT_BYPASS
        );
    }
    
    /* start timer */
    SND_StartTimer(
        playChBitMask,
        capBitMask,
        alarmNo >= 0 ? (u32)( 1 << alarmNo ) : 0,
        0
    );
    
    /* store parameters */
    cap->activeFlag = TRUE;
    cap->type = type;
    
    cap->chBitMask = chBitMask;
    cap->playChBitMask = playChBitMask;
    cap->capBitMask = capBitMask;
    cap->alarmNo = alarmNo;
    
    cap->format = format;
    cap->bufferL = buffer0;
    cap->bufferR = buffer1;
    cap->bufLen = bufLen;
    cap->blockSize = bufLen / interval;
    
    cap->curBuffer = 0;
    
    cap->interval = interval;
    cap->callback = callback;
    cap->callbackArg = arg;
    
    cap->volume = volume;
    
    NNSi_SndFaderInit( & cap->fader );
    NNSi_SndFaderSet( & cap->fader, volume << 8, 1 );
    
    cap->fadeOutFlag = FALSE;
    
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_SndCaptureStop

  Description:  Stops capture.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNSi_SndCaptureStop( void )
{
    CaptureParam* cap = & sCapParam;
    u32 commandTag;
    BOOL useAlarm;

    if ( ! cap->activeFlag ) return;
    
    useAlarm = cap->alarmNo >= 0 ? TRUE : FALSE;
    
    /* stop timer */
    SND_StopTimer(
        cap->playChBitMask,
        cap->capBitMask,
        useAlarm ? (u32)( 1 << cap->alarmNo ) : 0,
        0
    );
    
    if ( useAlarm ) {
        // When an alarm is used, wait for the ARM7 process to avoid triggering a callback.
        // 
        commandTag = SND_GetCurrentCommandTag();
        (void)SND_FlushCommand( SND_COMMAND_BLOCK );
        SND_WaitForCommandProc( commandTag );
        
        // Destroy accumulated messages.
        while( OS_ReceiveMessage( &sMesgQ, NULL, OS_MESSAGE_NOBLOCK ) ) {
        }
    }
    
    /* deallocate resource */
    if ( cap->capBitMask ) NNS_SndUnlockCapture( cap->capBitMask );
    if ( cap->chBitMask ) NNS_SndUnlockChannel( cap->chBitMask );
    if ( useAlarm ) NNS_SndFreeAlarm( cap->alarmNo );
    
    /* output selector return */
    if ( cap->type == NNS_SND_CAPTURE_TYPE_EFFECT )
    {
        SND_SetOutputSelector(
            SND_OUTPUT_MIXER,
            SND_OUTPUT_MIXER,
            SND_CHANNEL_OUT_MIXER,
            SND_CHANNEL_OUT_MIXER
        );
    }
    
    /* change status */
    cap->activeFlag = FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_SndCaptureBeginSleep

  Description:  Sleep mode preprocessing.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNSi_SndCaptureBeginSleep( void )
{
    CaptureParam* cap;
    u32 commandTag;
    
    cap = & sCapParam;
    
    if ( ! cap->activeFlag ) return;
    
    SND_StopTimer(
        cap->playChBitMask,
        cap->capBitMask,
        cap->alarmNo >= 0 ? (u32)( 1 << cap->alarmNo ) : 0,
        0
    );
        
    commandTag = SND_GetCurrentCommandTag();
    (void)SND_FlushCommand( SND_COMMAND_BLOCK );
    SND_WaitForCommandProc( commandTag );
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_SndCaptureEndSleep

  Description:  Sleep mode post-processing.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNSi_SndCaptureEndSleep( void )
{
    CaptureParam* cap;
    
    cap = & sCapParam;
        
    if ( ! cap->activeFlag ) return;
    
    cap->curBuffer = 0;
    
    MI_CpuClear32( cap->bufferL, cap->bufLen );
    MI_CpuClear32( cap->bufferR, cap->bufLen );
    DC_FlushRange( cap->bufferL, cap->bufLen );
    DC_FlushRange( cap->bufferR, cap->bufLen );
    
    SND_StartTimer(
        cap->playChBitMask,
        cap->capBitMask,
        cap->alarmNo >= 0 ? (u32)( 1 << cap->alarmNo ) : 0,
        0
    );
}

/******************************************************************************
	Static function
 ******************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         AlarmCallback

  Description:  alarm callback function

  Arguments:    arg - callback argument  (capture parameter)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void AlarmCallback( void* arg )
{
    CaptureParam* cap = (CaptureParam*)arg;
    const u32 blockSize = cap->blockSize; // size of 1 block
    const u32 offset = blockSize * cap->curBuffer;
    void* bufL_p = (u8*)cap->bufferL + offset;
    void* bufR_p = (u8*)cap->bufferR + offset;
    BOOL result;
    
    if ( sIsThreadCreated ) {
        EffectInfo* info = & sEffectInfoBuffer[ sCurEffectInfo ];
        
        info->cap = cap;
        info->blockSize = blockSize;
        info->offset = offset;
        info->bufL_p = bufL_p; 
        info->bufR_p = bufR_p;
        
        result = OS_SendMessage( & sMesgQ, (OSMessage)info, OS_MESSAGE_NOBLOCK );
        
        // result will be FALSE unless the capture thread process is performed regularly.
        // 
        NNS_WARNING( result, "Capture thread is too busy." );
        
        sCurEffectInfo++;
        if ( sCurEffectInfo >= THREAD_MESSAGE_BUF_SIZE ) sCurEffectInfo = 0;
    }
    else {
        // invalidate cache
        DC_InvalidateRange( bufL_p, blockSize );
        DC_InvalidateRange( bufR_p, blockSize );
        
        cap->callback(
            bufL_p,
            bufR_p,
            blockSize,
            cap->format,
            cap->callbackArg
        );
    }
    
    // switch buffer
    cap->curBuffer++;
    if ( cap->curBuffer >= cap->interval ) cap->curBuffer = 0;
}

/*---------------------------------------------------------------------------*
  Name:         CaptureThread

  Description:  Captures threads.

  Arguments:    arg - user data (unused)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CaptureThread( void* /*arg*/ )
{
    OSMessage msg;
    EffectInfo* info;
    
    while( 1 ) {
        (void)OS_ReceiveMessage( & sMesgQ, &msg, OS_MESSAGE_BLOCK );
        
        info = (EffectInfo*)msg;
        if ( info == NULL ) break;
        
        // invalidate cache
        DC_InvalidateRange( info->bufL_p, info->blockSize );
        DC_InvalidateRange( info->bufR_p, info->blockSize );
        
        info->cap->callback(
            info->bufL_p,
            info->bufR_p,
            info->blockSize,
            info->cap->format,
            info->cap->callbackArg
        );
    }
}

/*====== End of capture.c ======*/

