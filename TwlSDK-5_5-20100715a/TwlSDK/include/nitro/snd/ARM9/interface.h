/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - snd - ARM9
  File:     interface.h

  Copyright 2004-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-19#$
  $Rev: 9356 $
  $Author: ida $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_SND_INTERFACE_H_
#define NITRO_SND_INTERFACE_H_

#include <nitro/types.h>
#include <nitro/snd/common/capture.h>  // for SNDCapture and SNDCaptureFormat
#include <nitro/snd/common/channel.h>  // for SNDWaveFormat
#include <nitro/snd/common/global.h>   // for SNDChannelMix and SNDOutput
#include <nitro/snd/common/alarm.h>    // for SNDAlarmHandler
#include <nitro/snd/common/seq.h>      // for SNDSeqMute

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
    structure declaration
 ******************************************************************************/

struct SNDBankData;
struct SNDSharedWork;
struct SNDSpCallback;
struct SNDDriverInfo;

/******************************************************************************
    public function declaration
 ******************************************************************************/

//-----------------------------------------------------------------------------
// sequence

void    SND_StartSeq(int playerNo, const void *base, u32 offset, const struct SNDBankData *bank);
void    SND_PrepareSeq(int playerNo, const void *base, u32 offset, const struct SNDBankData *bank);
void    SND_StartPreparedSeq(int playerNo);
void    SND_StopSeq(int playerNo);

void    SND_StopSeq(int playerNo);
void    SND_PauseSeq(int playerNo, BOOL flag);

//-----------------------------------------------------------------------------
// player

void    SND_SetPlayerTempoRatio(int playerNo, int ratio);
void    SND_SetPlayerVolume(int playerNo, int volume);
void    SND_SetPlayerChannelPriority(int playerNo, int prio);
void    SND_SetPlayerLocalVariable(int playerNo, int varNo, s16 var);
void    SND_SetPlayerGlobalVariable(int varNo, s16 var);

//-----------------------------------------------------------------------------
// track

void    SND_SetTrackMute(int playerNo, u32 trackBitMask, BOOL flag);
void    SND_SetTrackMuteEx(int playerNo, u32 trackBitMask, SNDSeqMute mute);
void    SND_SetTrackVolume(int playerNo, u32 trackBitMask, int volume);
void    SND_SetTrackPitch(int playerNo, u32 trackBitMask, int pitch);
void    SND_SetTrackPan(int playerNo, u32 trackBitMask, int pan);
void    SND_SetTrackPanRange(int playerNo, u32 trackBitMask, int panRange);
void    SND_SetTrackModDepth(int playerNo, u32 trackBitMask, int depth);
void    SND_SetTrackModSpeed(int playerNo, u32 trackBitMask, int speed);
void    SND_SetTrackAllocatableChannel(int playerNo, u32 trackBitMask, u32 chBitMask);

//-----------------------------------------------------------------------------
// timer

void    SND_StartTimer(u32 chBitMask, u32 capBitMask, u32 alarmBitMask, u32 flags);
void    SND_StopTimer(u32 chBitMask, u32 capBitMask, u32 alarmBitMask, u32 flags);

//-----------------------------------------------------------------------------
// channel

void    SND_LockChannel(u32 chBitMask, u32 flags);
void    SND_UnlockChannel(u32 chBitMask, u32 flags);
void    SND_StopUnlockedChannel(u32 chBitMask, u32 flags);

void    SND_SetupChannelPcm(int chNo,
                            SNDWaveFormat format,
                            const void *dataAddr,
                            SNDChannelLoop loop,
                            int loopStart,
                            int loopLen, int volume, SNDChannelDataShift shift, int timer, int pan);
void    SND_SetupChannelPsg(int chNo,
                            SNDDuty duty,
                            int volume, SNDChannelDataShift shift, int timer, int pan);
void    SND_SetupChannelNoise(int chNo, int volume, SNDChannelDataShift shift, int timer, int pan);

void    SND_SetChannelVolume(u32 chBitMask, int volume, SNDChannelDataShift shift);
void    SND_SetChannelTimer(u32 chBitMask, int timer);
void    SND_SetChannelPan(u32 chBitMask, int pan);

//-----------------------------------------------------------------------------
// capture

void    SND_SetupCapture(SNDCapture capture,
                         SNDCaptureFormat format,
                         void *buffer_p,
                         u32 length, BOOL loopFlag, SNDCaptureIn in, SNDCaptureOut out);

//-----------------------------------------------------------------------------
// alarm

void    SND_SetupAlarm(int alarmNo, u32 tick, u32 period, SNDAlarmHandler handler, void *arg);

//-----------------------------------------------------------------------------
// global setting

void    SND_SetMasterVolume(int volume);

void    SND_SetOutputSelector(SNDOutput left,
                              SNDOutput right, SNDChannelOut channel1, SNDChannelOut channel3);

void    SND_SetMasterPan(int pan);
void    SND_ResetMasterPan(void);

//-----------------------------------------------------------------------------
// invalidate data

void    SND_InvalidateSeqData(const void *start, const void *end);
void    SND_InvalidateBankData(const void *start, const void *end);
void    SND_InvalidateWaveData(const void *start, const void *end);

//-----------------------------------------------------------------------------
// driver info

void    SND_ReadDriverInfo(struct SNDDriverInfo *info);

/******************************************************************************
    private function declaration
 ******************************************************************************/

void    SNDi_SetPlayerParam(int playerNo, u32 offset, u32 data, int size);
void    SNDi_SetTrackParam(int playerNo, u32 trackBitMask, u32 offset, u32 data, int size);
void    SNDi_SkipSeq(int playerNo, u32 tick);
void    SNDi_SetSurroundDecay(int decay);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* NITRO_SND_INTERFACE_H_ */
