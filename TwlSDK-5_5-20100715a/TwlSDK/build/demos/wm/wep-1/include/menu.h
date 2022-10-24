/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - demos - wep-1
  File:     menu.h

  Copyright 2006-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef __MENU_H__
#define __MENU_H__

#define ITEM_LENGTH_MAX (30)
#define ITEM_NUM_MAX    (20)

enum
{
    WIN_STATE_OPENING,
    WIN_STATE_OPENED,
    WIN_STATE_CLOSING,
    WIN_STATE_CLOSED,
    WIN_STATE_NUM
};

enum
{
    WIN_FLAG_NONE = 0,
    WIN_FLAG_SELECTABLE = (1 << 0),
    WIN_FLAG_NOCONTROL = (1 << 1),
    WIN_FLAG_LAST = (1 << 8)
};

typedef struct Window_
{
    int     x, y;
    int     width, height;
    int     count;
    int     itemnum;
    int     lineheight;
    int     leftmargin;
    int     rightmargin;
    int     selected;
    int     state;
    int     flag;
    char    item[ITEM_NUM_MAX][ITEM_LENGTH_MAX];
    struct Window_ *next;
}
Window;

extern void initWindow(Window * win);
extern void setupWindow(Window * win,
                        int x, int y, int flag, int lineheight, int leftmargin, int rightmargin);
extern void addItemToWindow(Window * win, const char *item);
extern void drawWindow(Window * win);
extern int updateWindow(Window * win);
extern void openWindow(Window * win);
extern void closeWindow(Window * win);
extern int getWindowSelected(Window * win);
extern void drawAllWindow(void);
extern void updateAllWindow(void);
extern void closeAllWindow(void);

#endif
