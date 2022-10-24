/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - exceptionDisplay-4
  File:     screen.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: main.c,v $
  Revision 1.23  2006/01/18 02:11:29  kitase_hirotake
  do-indent

  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include  <nitro.h>
#include  "screen.h"

u16     gScreen[32 * 32];

/*---------------------------------------------------------------------------*
  Name:         ClearScreen

  Description:  clear screen buffer

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ClearScreen(void)
{
    MI_CpuClearFast((void *)gScreen, sizeof(gScreen));
}

/*---------------------------------------------------------------------------*
  Name:         PrintString

  Description:  enter string into screen buffer
                string must be within 32 chars

  Arguments:    x       : x
                y       : y
                palette : color (0-15)
                text    : string. end mark is NULL

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PrintString(s16 x, s16 y, u8 palette, char *text, ...)
{
    va_list vlist;
    char    temp[32 + 2], *tempPtr;
    s32     i;
    u16    *p, *pLimit;

    va_start(vlist, text);
    (void)vsnprintf(temp, 33, text, vlist);
    va_end(vlist);

    *(u16 *)(&temp[32]) = 0;
    p = &gScreen[((y * 32) + x) % (32 * 32)];
    pLimit = &gScreen[32 * 32];
    tempPtr = &temp[0];

    for (i = 0; *tempPtr; i++, tempPtr++)
    {
        *p = (u16)((palette << 12) | *tempPtr);
        if (++p >= pLimit)
        {
            p = &gScreen[0];
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         ColorString

  Description:  change string color which is put in screen buffer

  Arguments:    x       : x
                y       : y
                length  : number of characters to change color
                palette : color (0-15)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ColorString(s16 x, s16 y, s16 length, u8 palette)
{
    s32     i;
    u16    *p, *pLimit;

    if (length < 0)
        return;

    p = &gScreen[((y * 32) + x) % (32 * 32)];
    pLimit = &gScreen[32 * 32];

    for (i = 0; i < length; i++)
    {
        u16     temp = *p;
        temp &= 0x0fff;
        temp |= (palette << 12);
        *p = temp;

        if (++p >= pLimit)
        {
            p = &gScreen[0];
        }
    }
}
