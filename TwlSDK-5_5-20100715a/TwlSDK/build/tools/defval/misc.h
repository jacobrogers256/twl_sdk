/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tool - defval
  File:     misc.h

  Copyright 2005-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef MISC_H__
#define MISC_H__

#include <stdio.h>
#include <malloc.h>                    // calloc()
#include <stdlib.h>                    // free(), exit()
#include <string.h>                    // strlen/strdup/strcpy/memcpy
#include <stdarg.h>                    // va_start(),va_end()
#include <assert.h>                    // assert()

typedef enum
{
    TRUE = 1,
    FALSE = 0
}
BOOL;

#define DEFAULT_LINE_BUFFER_SIZE        1024

char   *StrDup(const char *str);
char   *StrCatDup(const char *str1, const char *str2);
char   *StrNDup(const char *str, int len);
void   *Calloc(int size);
void   *Realloc(void *buffer, int size);
void    Free(void **ptr);
FILE   *Fopen(const char *filename, const char *mode);
void    Fclose(FILE * fp);
char   *Fgets(char **pbuffer, int *pbuffer_size, FILE * fp);
void    UnpackFileName(const char *path, char **dir, char **base, char **ext);
void    SetDebugMode(BOOL mode);
void    DebugPrintf(const char *fmt, ...);

#endif //MISC_H__
