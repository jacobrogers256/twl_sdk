/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI
  File:     mi_dma_vblank.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-10-16#$
  $Rev: 8957 $
  $Author: yada $
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include "../include/mi_dma.h"

/*---------------------------------------------------------------------------*
  Name:         MI_VBlankDmaCopy32

  Description:  VBlank DMA copy.
                32bit, sync version.

  Arguments:    dmaNo   : DMA channel No.
                src     : source address
                dest    : destination address
                size    : transfer size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_VBlankDmaCopy32(u32 dmaNo, const void *src, void *dest, u32 size)
{
    vu32   *dmaCntp;

    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_SRC_ALIGN4(src);
    MIi_ASSERT_DEST_ALIGN4(dest);
    MIi_WARNING_ADDRINTCM(src, size);
    MIi_WARNING_ADDRINTCM(dest, size);

#ifdef SDK_ARM9
    MIi_CheckAnotherAutoDMA(dmaNo, MI_DMA_TIMING_V_BLANK);
#endif
    //---- check DMA0 source address
    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_INC);

    if (size == 0)
    {
        return;
    }

    MIi_Wait_BeforeDMA(dmaCntp, dmaNo);
	MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_VBCOPY32(size), 0);
    MIi_Wait_AfterDMA(dmaCntp);
}

/*---------------------------------------------------------------------------*
  Name:         MI_VBlankDmaCopy16

  Description:  VBlank DMA copy.
                16bit, sync version.

  Arguments:    dmaNo   : DMA channel No.
                src     : source address
                dest    : destination address
                size    : transfer size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_VBlankDmaCopy16(u32 dmaNo, const void *src, void *dest, u32 size)
{
    vu32   *dmaCntp;

    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_SRC_ALIGN2(src);
    MIi_ASSERT_DEST_ALIGN2(dest);
    MIi_WARNING_ADDRINTCM(src, size);
    MIi_WARNING_ADDRINTCM(dest, size);

#ifdef SDK_ARM9
    MIi_CheckAnotherAutoDMA(dmaNo, MI_DMA_TIMING_V_BLANK);
#endif
    //---- check DMA0 source address
    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_INC);

    if (size == 0)
    {
        return;
    }

    MIi_Wait_BeforeDMA(dmaCntp, dmaNo);
	MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_VBCOPY16(size), MIi_DMA_MODE_WAIT);
    MIi_Wait_AfterDMA(dmaCntp);
}

/*---------------------------------------------------------------------------*
  Name:         MI_VBlankDmaCopy32Async

  Description:  VBlank DMA copy.
                32bit, async version

  Arguments:    dmaNo   : DMA channel No.
                src     : source address
                dest    : destination address
                size    : transfer size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_VBlankDmaCopy32Async(u32 dmaNo, const void *src, void *dest, u32 size,
                             MIDmaCallback callback, void *arg)
{
    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_SRC_ALIGN4(src);
    MIi_ASSERT_DEST_ALIGN4(dest);
    MIi_WARNING_ADDRINTCM(src, size);
    MIi_WARNING_ADDRINTCM(dest, size);

#ifdef SDK_ARM9
    MIi_CheckAnotherAutoDMA(dmaNo, MI_DMA_TIMING_V_BLANK);
#endif
    //---- check DMA0 source address
    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_INC);

    if (size == 0)
    {
        MIi_CallCallback(callback, arg);
    }
    else
    {
        MI_WaitDma(dmaNo);
        if (callback)
        {
            OSi_EnterDmaCallback(dmaNo, callback, arg);
			MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_VBCOPY32_IF(size), 0 );
        }
        else
        {
			MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_VBCOPY32(size), 0 );
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         MI_VBlankDmaCopy16Async

  Description:  VBlank DMA copy.
                16bit, async version

  Arguments:    dmaNo   : DMA channel No.
                src     : source address
                dest    : destination address
                size    : transfer size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_VBlankDmaCopy16Async(u32 dmaNo, const void *src, void *dest, u32 size,
                             MIDmaCallback callback, void *arg)
{
    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_SRC_ALIGN2(src);
    MIi_ASSERT_DEST_ALIGN2(dest);
    MIi_WARNING_ADDRINTCM(src, size);
    MIi_WARNING_ADDRINTCM(dest, size);

#ifdef SDK_ARM9
    MIi_CheckAnotherAutoDMA(dmaNo, MI_DMA_TIMING_V_BLANK);
#endif
    //---- check DMA0 source address
    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_INC);

    if (size == 0)
    {
        MIi_CallCallback(callback, arg);
    }
    else
    {
        MI_WaitDma(dmaNo);
        if (callback)
        {
            OSi_EnterDmaCallback(dmaNo, callback, arg);
			MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_VBCOPY16_IF(size), 0);
        }
        else
        {
			MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_VBCOPY16(size), 0 );
        }
    }
}
