/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - demos - simple-2
  File:     dispfunc.h

  Copyright 2005-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-15#$
  $Rev: 2414 $
  $Author: hatamoto_minoru $
 *---------------------------------------------------------------------------*/

#ifndef _DISPFUNC_H_
#define _DISPFUNC_H_

#include <nitro.h>

/* Palette color for text */
enum
{
    WHITE,
    RED,
    GREEN,
    BLUE,
    YELLOW,
    CYAN,
    PURPLE,
    LIGHT_GREEN,
    HI_YELLOW,
    HI_CYAN,
    HI_WHITE,
    HI_BLUE,
    HI_PURPLE,
    HI_LIGHT_GREEN,
    BROWN,
    HI_BROWN,
    COLOR_NUM
};

/* Constant array for each type of rendering */
extern const u32 sampleCharData[8 * 0xe0];
extern const u16 samplePlttData[16][16];

/*
	=================================================================================

	OBJ character rendering

	=================================================================================
*/

// --- Global value
extern GXOamAttr oamBak[128];

/*
	=================================================================================

	Background character rendering (fixed to BG0)

	=================================================================================
*/

// --- Definition

// --- Initialize
void BgInitForPrintStr(void);

// --- For main LCD
void BgPrintf(s16 x, s16 y, u8 palette, char *text, ...);
void BgPutString(s16 x, s16 y, u8 palette, char *text);
void BgPutStringN(s16 x, s16 y, u8 palette, char *text, u32 num);
void BgPutChar(s16 x, s16 y, u8 palette, char c);
void BgClear(void);

// --- V-Blank process
void BgScrSetVblank(void);

#endif /*       _DISPFUNC_H_    */
