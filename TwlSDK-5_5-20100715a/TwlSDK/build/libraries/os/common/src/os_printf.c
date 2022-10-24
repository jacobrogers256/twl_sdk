/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraries - OS
  File:     os_printf.c

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-09-17#$
  $Rev: 11062 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#include        <nitro.h>
#include        <nitro/dbghost.h>

//---------------- For IS-NITRO-DEBUGGER
#ifdef          SDK_LINK_ISD
# pragma warn_extracomma off
# include       <isdbglib.h>           // Has extra comma in enum
# pragma warn_extracomma reset
#else
void    ISDPrint(const char *);
#endif

//---------------- For IS-TWL-DEBUGGER
#ifdef          SDK_LINK_ISTD
# pragma warn_extracomma off
# include       <istdbglib.h>          // Has extra comma in enum
# pragma warn_extracomma reset
#else
void    ISTDPrintEx(int console, const char *);
#endif
//---- Print output
#ifdef SDK_ARM9
#define OSi_PRINT_OUTPUT        (*(u8*)HW_PRINT_OUTPUT_ARM9)
#define OSi_PRINT_ERR_OUTPUT    (*(u8*)HW_PRINT_OUTPUT_ARM9ERR)
#else
#define OSi_PRINT_OUTPUT        (*(u8*)HW_PRINT_OUTPUT_ARM7)
#define OSi_PRINT_ERR_OUTPUT    (*(u8*)HW_PRINT_OUTPUT_ARM7ERR)
#endif

#ifndef SDK_FINALROM
static char common_buffer[256];        // Thread unsafe, but less use of stack
#endif

#if !defined(SDK_FINALROM) && defined(SDK_NO_MESSAGE)
#undef OSi_Warning
#undef OSi_TWarning
#undef OSi_Panic
#undef OSi_TPanic
void    OSi_Warning(const char *file, int line, const char *fmt, ...);
void    OSi_TWarning(const char *file, int line, const char *fmt, ...);
void    OSi_Panic(const char *file, int line, const char *fmt, ...);
void    OSi_TPanic(const char *file, int line, const char *fmt, ...);
#endif /* SDK_NO_MESSAGE */


/*---------------------------------------------------------------------------*
  Name:         OS_PutChar
                OS_FPutChar

  Description:  Puts a letter for debug console.

  Arguments:    console: Output window
                c: Char code, should be 0x01-0xff

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifndef SDK_FINALROM
SDK_WEAK_SYMBOL void OS_PutChar(char c)
{
	OS_FPutChar((int)OSi_PRINT_OUTPUT, c);
}

SDK_WEAK_SYMBOL void OS_FPutChar(int console, char c)
{
    char    str[2];

    str[0] = c;
    str[1] = '\0';

    // Because of no putchar-type function on ISDebugger library,
    // use 'putstring' for a single letter outputting
    OS_FPutString(console, str);
}
#endif

/*---------------------------------------------------------------------------*
  Name:         OS_FPutString

  Description:  put a string to debug console.
                console port are automatically switched depends on emu/hw

  Arguments:    console : output window
                str: String

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void OS_PutStringDefault(const char* str);
static void OS_FPutStringInit(int console, const char *str);

#ifndef SDK_FINALROM
static void OS_FPutStringDummy(int console, const char *str);
static void OS_FPutStringAris(int console, const char *str);
static void OS_FPutStringISD(int console, const char *str);
static void OS_FPutStringISTD(int console, const char *str);
#ifdef  SDK_ARM7
static void OS_FPutStringPrnSrv(int console, const char *str);
#endif
#endif

#ifndef SDK_FINALROM
void    (*OS_PutString)  (const char *str) = OS_PutStringDefault;
void    (*OS_FPutString) (int console, const char *str) = OS_FPutStringInit;
void    (*OSi_FuncTerminate) (void) = OS_Terminate;
//----  Hook function
static OSFPutStringHookType  OSi_FPutStringHook = NULL;
#endif


/*---------------------------------------------------------------------------*
  Name:         OS_PutStringDefault
  Description:  OS_PutString initializer
 *---------------------------------------------------------------------------*/
static void OS_PutStringDefault(const char* str)
{
#pragma unused(str)
#ifndef SDK_FINALROM
    OS_FPutString(OS_PRINT_OUTPUT_CURRENT, str);
#endif
}

/*---------------------------------------------------------------------------*
  Name:         OS_FPutStringInit
  Description:  OS_FPutString initializer
 *---------------------------------------------------------------------------*/
static void OS_FPutStringInit(int console, const char *str)
{
#ifndef SDK_FINALROM
    OSIntrMode intr = OS_DisableInterrupts();
	u32 consoleType = OS_GetConsoleType();

    OS_FPutString = OS_FPutStringDummy;

#ifdef  SDK_ARM9
	//---- ARM9
    if ( consoleType & OS_CONSOLE_ENSATA )
    {
        OS_FPutString = OS_FPutStringAris;
    }
    else if ( consoleType & OS_CONSOLE_TWLDEBUGGER )
	{
		OS_FPutString = OS_FPutStringISTD;
	}
	else if ( consoleType & OS_CONSOLE_ISDEBUGGER )
	{
#ifndef SDK_TWLLTD
		OS_FPutString = OS_FPutStringISD;
#endif
	}
#else
	//---- ARM7
    if ( (OSPrintServerBuffer *)PXI_GetComponentParam() != NULL )
    {
        OS_FPutString = OS_FPutStringPrnSrv;
    }
    else
    {
        if ( consoleType & OS_CONSOLE_TWLDEBUGGER )
        {
            OS_FPutString = OS_FPutStringISTD;
        }
    }
#endif

    OS_FPutString(console, str);

    (void)OS_RestoreInterrupts(intr);

#else
#pragma unused(console, str)
#endif
}

/*---------------------------------------------------------------------------*
  Name:         OS_FPutStringDummy
  Description:  dummy OS_FPutString. do nothing.
 *---------------------------------------------------------------------------*/
static void OS_FPutStringDummy(int console, const char *str)
{
#pragma unused(console, str)
#ifndef SDK_FINALROM
	if ( OSi_FPutStringHook )
	{
		OSi_FPutStringHook(OS_FPUTSTRING_DUMMY, console, str);
	}
#endif
}

/*---------------------------------------------------------------------------*
  Name:         OS_FPutStringAris
  Description:  OS_FPutString for software simulator called ARIS
 *---------------------------------------------------------------------------*/
#ifndef SDK_FINALROM
#ifdef  SDK_CW_WARNOFF_SAFESTRB
#include <nitro/code32.h>
#endif
static void OS_FPutStringAris(int console, const char *str)
{
    char    c;

	if ( console == OS_PRINT_OUTPUT_CURRENT )
	{
		console = (int)OSi_PRINT_OUTPUT;
	}
	else if ( console == OS_PRINT_OUTPUT_ERROR )
	{
		console = (int)OSi_PRINT_ERR_OUTPUT;
	}

	if ( console != OS_PRINT_OUTPUT_NONE )
	{
		while ('\0' != (c = *str))
		{
			reg_OS_EMU_CONSOLE_OUT = (u8)c; // Console out
			str++;
		}
		if ( OSi_FPutStringHook )
		{
			OSi_FPutStringHook(OS_FPUTSTRING_ARIS, console, str);
		}
	}
}
#ifdef  SDK_CW_WARNOFF_SAFESTRB
#include <nitro/codereset.h>
#endif
#endif

/*---------------------------------------------------------------------------*
  Name:         OS_FPutStringISD
  Description:  OS_FPutString for hardware emulator called IS Debugger
 *---------------------------------------------------------------------------*/
#ifndef SDK_FINALROM
static void OS_FPutStringISD(int console, const char *str)
{
    // ISDPrint initialize process
    OS_InitLock();

	if ( console == OS_PRINT_OUTPUT_CURRENT )
	{
		console = (int)OSi_PRINT_OUTPUT;
	}
	else if ( console == OS_PRINT_OUTPUT_ERROR )
	{
		console = (int)OSi_PRINT_ERR_OUTPUT;
	}

	if ( console != OS_PRINT_OUTPUT_NONE )
	{
		ISDPrint(str);
		if ( OSi_FPutStringHook )
		{
			OSi_FPutStringHook(OS_FPUTSTRING_ISD, console, str);
		}
	}
}
#endif

/*---------------------------------------------------------------------------*
  Name:         OS_FPutStringISTD
  Description:  OS_FPutString for hardware emulator called IS TWL Debugger
 *---------------------------------------------------------------------------*/
#ifndef SDK_FINALROM
static void OS_FPutStringISTD(int console, const char *str)
{
    // ISTDPrintEx initialize process
    OS_InitLock();

	if ( console == OS_PRINT_OUTPUT_CURRENT )
	{
		console = (int)OSi_PRINT_OUTPUT;
	}
	else if ( console == OS_PRINT_OUTPUT_ERROR )
	{
		console = (int)OSi_PRINT_ERR_OUTPUT;
	}

	if ( console != OS_PRINT_OUTPUT_NONE )
	{
		ISTDPrintEx(console, str);
		if ( OSi_FPutStringHook )
		{
			OSi_FPutStringHook(OS_FPUTSTRING_ISTD, console, str);
		}
	}
}
#endif


/*---------------------------------------------------------------------------*
  Name:         OS_VPrintf/OS_TVPrintf

  Description:  Prints formatted strings (vprintf version).

  Arguments:    fmt: Format string
                vlist: Parameters

  Returns:      None.
 *---------------------------------------------------------------------------*/
// If you want to use "vsnprintf()" in ARM7, define SDK_USE_VSNPRINTF.
// "vsnprintf()" is able to print float format but the code size become hugely bigger.
// #define SDK_USE_VSNPRINTF

#ifndef SDK_FINALROM
#if     defined(SDK_ARM9) || defined(SDK_USE_VSNPRINTF)
SDK_WEAK_SYMBOL void OS_VPrintf(const char *fmt, va_list vlist)
{
	OS_VFPrintf(OS_PRINT_OUTPUT_CURRENT, fmt, vlist);
}
SDK_WEAK_SYMBOL void OS_VFPrintf(int console, const char *fmt, va_list vlist)
{
    (void)vsnprintf(common_buffer, sizeof(common_buffer), fmt, vlist);
    OS_FPutString(console, common_buffer);
}
#else
SDK_WEAK_SYMBOL void OS_VPrintf(const char *fmt, va_list vlist)
{
    OS_TVFPrintf(OS_PRINT_OUTPUT_CURRENT, fmt, vlist);
}
SDK_WEAK_SYMBOL void OS_VFPrintf(int console, const char *fmt, va_list vlist)
{
    OS_TVFPrintf(console, fmt, vlist);
}
#endif
SDK_WEAK_SYMBOL void OS_TVPrintf(const char *fmt, va_list vlist)
{
	OS_TVFPrintf(OS_PRINT_OUTPUT_CURRENT, fmt, vlist);
}
SDK_WEAK_SYMBOL void OS_TVFPrintf(int console, const char *fmt, va_list vlist)
{
    (void)OS_VSNPrintf(common_buffer, sizeof(common_buffer), fmt, vlist);
    OS_FPutString(console, common_buffer);
}
SDK_WEAK_SYMBOL void OS_TVPrintfEx(const char *fmt, va_list vlist)
{
	OS_TVFPrintfEx(OS_PRINT_OUTPUT_CURRENT, fmt, vlist);
}
SDK_WEAK_SYMBOL void OS_TVFPrintfEx(int console, const char *fmt, va_list vlist)
{
    (void)OS_VSNPrintfEx(common_buffer, sizeof(common_buffer), fmt, vlist);
    OS_FPutString(console, common_buffer);
}
#endif

/*---------------------------------------------------------------------------*
  Name:         OS_Printf/OS_TPrintf/OS_TPrintfEx
                OS_FPrintf/OS_TFPrintf/OS_TFPrintfEx

  Description:  Prints formatted strings.

  Arguments:    console: Output window
                fmt: Format string
                ...   and parameters

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifndef SDK_FINALROM
SDK_WEAK_SYMBOL void OS_Printf(const char *fmt, ...)
{
    va_list vlist;

    va_start(vlist, fmt);
    OS_VFPrintf(OS_PRINT_OUTPUT_CURRENT, fmt, vlist);
    va_end(vlist);
}

SDK_WEAK_SYMBOL void OS_FPrintf(int console, const char *fmt, ...)
{
    va_list vlist;

    va_start(vlist, fmt);
    OS_VFPrintf(console, fmt, vlist);
    va_end(vlist);
}

SDK_WEAK_SYMBOL void OS_TPrintf(const char *fmt, ...)
{
    va_list vlist;

    va_start(vlist, fmt);
    OS_TVFPrintf(OS_PRINT_OUTPUT_CURRENT, fmt, vlist);
    va_end(vlist);
}

SDK_WEAK_SYMBOL void OS_TFPrintf(int console, const char *fmt, ...)
{
    va_list vlist;

    va_start(vlist, fmt);
    OS_TVFPrintf(console, fmt, vlist);
    va_end(vlist);
}

SDK_WEAK_SYMBOL void OS_TPrintfEx(const char *fmt, ...)
{
    va_list vlist;

    va_start(vlist, fmt);
    OS_TVFPrintfEx(OS_PRINT_OUTPUT_CURRENT, fmt, vlist);
    va_end(vlist);
}

SDK_WEAK_SYMBOL void OS_TFPrintfEx(int console, const char *fmt, ...)
{
    va_list vlist;

    va_start(vlist, fmt);
    OS_TVFPrintfEx(console, fmt, vlist);
    va_end(vlist);
}
#endif


/*---------------------------------------------------------------------------*
  Name:         OSi_Warning / OSi_TWarning

  Description:  Prints warning message.

  Arguments:    file: Filename of warning location
                line: Line number of warning location
                fmt: Format string
                ...   and parameters

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifndef SDK_FINALROM
SDK_WEAK_SYMBOL void OSi_Warning(const char *file, int line, const char *fmt, ...)
{
    va_list vlist;

    va_start(vlist, fmt);
    OS_FPrintf(OS_PRINT_OUTPUT_ERROR, "%s:%d Warning:", file, line);
    OS_VFPrintf(OS_PRINT_OUTPUT_ERROR, fmt, vlist);
    OS_FPrintf(OS_PRINT_OUTPUT_ERROR, "\n");
    va_end(vlist);
}


SDK_WEAK_SYMBOL void OSi_TWarning(const char *file, int line, const char *fmt, ...)
{
    va_list vlist;

    va_start(vlist, fmt);
    OS_TPrintf("%s:%d Warning:", file, line);
    OS_TVPrintf(fmt, vlist);
    OS_TPrintf("\n");
    va_end(vlist);
}
#endif

/*---------------------------------------------------------------------------*
  Name:         OSi_Panic/OSi_TPanic

  Description:  Prints panic message and halt CPU.

  Arguments:    file: Filename of panic location
                line: Line number of panic location
                fmt: Format string
                ...   and parameters

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifndef SDK_FINALROM
SDK_WEAK_SYMBOL void OSi_Panic(const char *file, int line, const char *fmt, ...)
{
    va_list vlist;

    va_start(vlist, fmt);
    (void)OS_DisableInterrupts();
    OS_FPrintf(OS_PRINT_OUTPUT_ERROR, "%s:%d Panic:", file, line);
    OS_VFPrintf(OS_PRINT_OUTPUT_ERROR, fmt, vlist);
    OS_FPrintf(OS_PRINT_OUTPUT_ERROR, "\n");
    OSi_FuncTerminate();               // Never Returns
}

SDK_WEAK_SYMBOL void OSi_TPanic(const char *file, int line, const char *fmt, ...)
{
    va_list vlist;

    va_start(vlist, fmt);
    (void)OS_DisableInterrupts();
    OS_TPrintf("%s:%d Panic:", file, line);
    OS_TVPrintf(fmt, vlist);
    OS_TPrintf("\n");
    OSi_FuncTerminate();               // Never Returns
}
#endif

/*---------------------------------------------------------------------------*
  Name:         OSi_FWarning / OSi_TFWarning

  Description:  Prints warning message.

  Arguments:    console: Output window
                file: Filename of warning location
                line: Line number of warning location
                fmt: Format string
                ...   and parameters

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifndef SDK_FINALROM
SDK_WEAK_SYMBOL void OSi_FWarning(int console, const char *file, int line, const char *fmt, ...)
{
    va_list vlist;

    va_start(vlist, fmt);
    OS_FPrintf(console, "%s:%d Warning:", file, line);
    OS_VFPrintf(console, fmt, vlist);
    OS_FPrintf(console, "\n");
    va_end(vlist);
}


SDK_WEAK_SYMBOL void OSi_TFWarning(int console, const char *file, int line, const char *fmt, ...)
{
    va_list vlist;

    va_start(vlist, fmt);
    OS_TFPrintf(console, "%s:%d Warning:", file, line);
    OS_TVFPrintf(console, fmt, vlist);
    OS_TFPrintf(console, "\n");
    va_end(vlist);
}
#endif

/*---------------------------------------------------------------------------*
  Name:         OSi_FPanic/OSi_TFPanic

  Description:  Prints panic message and halts CPU.

  Arguments:    file: Filename of panic location
                line: Line number of panic location
                fmt: Format string
                ...   and parameters

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifndef SDK_FINALROM
SDK_WEAK_SYMBOL void OSi_FPanic(int console, const char *file, int line, const char *fmt, ...)
{
    va_list vlist;

    va_start(vlist, fmt);
    (void)OS_DisableInterrupts();
    OS_FPrintf(console, "%s:%d Panic:", file, line);
    OS_VFPrintf(console, fmt, vlist);
    OS_FPrintf(console, "\n");
    OSi_FuncTerminate();               // Never Returns
}

SDK_WEAK_SYMBOL void OSi_TFPanic(int console, const char *file, int line, const char *fmt, ...)
{
    va_list vlist;

    va_start(vlist, fmt);
    (void)OS_DisableInterrupts();
    OS_TFPrintf(console, "%s:%d Panic:", file, line);
    OS_TVFPrintf(console, fmt, vlist);
    OS_TFPrintf(console, "\n");
    OSi_FuncTerminate();               // Never Returns
}
#endif

/*---------------------------------------------------------------------------*
  Name:         OS_InitPrintServer

  Description:  Initializes print server.
                - This code should be called before OS_InitLock()
                - OS_InitLock() are also called from OS_Init()

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifdef   SDK_ARM9
#ifndef  SDK_FINALROM
void OS_InitPrintServer(void)
{
    OSPrintServerBuffer *p;

    // First, allocate buffer and register it.
    OS_InitArena();
    p = OS_AllocFromSharedArenaHi(sizeof(OSPrintServerBuffer), 4);
    p->in = p->out = 0UL;
    PXI_SetComponentParam((u32)p);
}
#endif //SDK_FINALROM
#endif //SDK_ARM9


/*---------------------------------------------------------------------------*
  Name:         OS_PrintServer

  Description:  Prints a string comes from ARM7 via shared memory.

  Arguments:    str: String

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifdef   SDK_ARM9
#ifndef  SDK_FINALROM
#include <nitro/code32.h>              // To access 'common_buffer' via cache
void OS_PrintServer(void)
{
    OSPrintServerBuffer *p;
    register OSPrintWChar word;

    u32     in, out;
    int     i;

    p = (OSPrintServerBuffer *)PXI_GetComponentParam();

    //---- If print buffer isn't set up, do nothing.
    if (!p)
    {
        return;
    }

    out = p->out;
    in = p->in;

    while (in != out)
    {
        i = 0;

        while (in != out && i < sizeof(common_buffer) - 3)
        {
            word.s = p->buffer[out].s;
            if (word.c[0])
            {
                common_buffer[i++] = word.c[0]; // store via cache

                if (word.c[1])
                {
                    common_buffer[i++] = word.c[1];     // store via cache
                }
            }
            out++;
            if (out >= OS_PRINTSRV_WCHARSIZE)
                out = 0;
        }
        common_buffer[i] = '\0';       // store via cache
        OS_FPutString(OS_PRINT_OUTPUT_CURRENT, common_buffer);
    }

    // tell finished
    p->out = out;
}

#include <nitro/codereset.h>
#endif //SDK_FINALROM
#endif //SDK_ARM9


/*---------------------------------------------------------------------------*
  Name:         OS_FPutStringPrnSrv

  Description:  Sends string to ARM9 via shared memory.

  Arguments:    console: Output window
                str: String to be sent

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifdef  SDK_ARM7
static void OS_FPutStringPrnSrv(int console, const char *str)
{
#pragma unused(console, str)
#ifndef  SDK_FINALROM
    OSPrintServerBuffer *p;
    register OSPrintWChar word;

    u32     in, in_tmp, out;
    u32     isOdd;

	if ( console == OS_PRINT_OUTPUT_NONE )
	{
		return;
	}

    p = (OSPrintServerBuffer *)PXI_GetComponentParam();
    in = p->in;
    out = p->out;
    isOdd = ((u32)str) & 1;

    while (1)
    {
        in_tmp = in + 1;
        if (in_tmp >= OS_PRINTSRV_WCHARSIZE)
            in_tmp = 0;
        if (out == in_tmp)
            break;                     // Buffer full, then exit

        if (isOdd)
        {
            p->buffer[in].s = word.s = (u16)((*(u16 *)(str - 1)) & 0xff00);
            str++;
            isOdd = 0UL;
            if (!word.c[1])
                break;
            in = in_tmp;
        }
        else
        {
            p->buffer[in].s = word.s = *(u16 *)str;
            str += 2;
            if (!word.c[0])
                break;
            in = in_tmp;
            if (!word.c[1])
                break;
        }
    }
    p->in = in;
#endif //SDK_FINALROM
}
#endif //SDK_ARM7


#if defined(SDK_CW_WARNOFF_SAFESTRB)
#include <nitro/code32.h>
#endif

/* inner function for sized-buffer output */
typedef struct dst_string_tag
{
    size_t  len;
    char   *cur;
    char   *base;
}
dst_string;

static void string_put_char(dst_string * p, char c)
{
    if (p->len > 0)
        *p->cur = c, --p->len;
    ++p->cur;
}

static void string_fill_char(dst_string * p, char c, int n)
{
    if (n > 0)
    {
        size_t  i, k = p->len;
        if (k > (size_t) n)
            k = (size_t) n;
        for (i = 0; i < k; ++i)
            p->cur[i] = c;
        p->len -= k;
        p->cur += n;
    }
}

static void string_put_string(dst_string * p, const char *s, int n)
{
    if (n > 0)
    {
        size_t  i, k = p->len;
        if (k > (size_t) n)
            k = (size_t) n;
        for (i = 0; i < k; ++i)
            p->cur[i] = s[i];
        p->len -= k;
        p->cur += n;
    }
}

/*---------------------------------------------------------------------------*
  Name:         OS_SPrintf

  Description:  Equal to 'OS_VSPrintf' except argument style.

  Arguments:    dst: Destination buffer
                fmt: Format string

  Returns:      Length of the generated string.
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL int OS_SPrintf(char *dst, const char *fmt, ...)
{
    int     ret;
    va_list va;
    va_start(va, fmt);
    ret = OS_VSPrintf(dst, fmt, va);
    va_end(va);
    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         OS_VSPrintf

  Description:  Equal to 'OS_VSNPrintf' except buffer size argument.

  Arguments:    dst: Destination buffer
                fmt: Format string
                vlist: Parameters

  Returns:      Length of the generated string.
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL int OS_VSPrintf(char *dst, const char *fmt, va_list vlist)
{
    return OS_VSNPrintf(dst, 0x7FFFFFFF, fmt, vlist);
}

/*---------------------------------------------------------------------------*
  Name:         OS_SNPrintf

  Description:  Equal to 'OS_VSNPrintf' except argument style.

  Arguments:    dst: Destination buffer
                len: Destination buffer size
                fmt: Format string

  Returns:      Length of the generated string (except '\0').
                if(result < len),
                  put NUL in dst[result].
                else if(len > 0),
                  put NUL in dst[len - 1].
                else,
                  do nothing.
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL int OS_SNPrintf(char *dst, size_t len, const char *fmt, ...)
{
    int     ret;
    va_list va;
    va_start(va, fmt);
    ret = OS_VSNPrintf(dst, len, fmt, va);
    va_end(va);
    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         OS_VSNPrintf

  Description:  small-size vsnprintf which is similar to 'vsnprintf'
                without following supports.
                * CodeWarrior Extensions (#s)
                * MSL AltiVec Extensions (v, vh, vl, hv, lv, @)
                * indexed argments (%m$, *m$)
                * floating-point
                * wchar_t

  Note:         '+' and '#' do not work, MSL's sprintf().
                to keep same result, they are no implement.
                { // example
                  char buf[5];
                  sprintf(buf, "%-i\n", 45);  // "45"  (OK)
                  sprintf(buf, "%0i\n", 45);  // "45"  (OK)
                  sprintf(buf, "% i\n", 45);  // " 45" (OK)
                  sprintf(buf, "%+i\n", 45);  // "%+i" ("+45" expected)
                  sprintf(buf, "%#x\n", 45);  // "%#x" ("0x2d" expected)
                  // but, this works correctly!
                  sprintf(buf, "% +i\n", 45); // "+45" (OK)
                }

  Arguments:    dst: Destination buffer
                len: Destination buffer size
                fmt: Format string
                vlist: Parameters

  Returns:      Length of the generated string (except '\0').
                if(result < len),
                  put NUL in dst[result].
                else if(len > 0),
                  put NUL in dst[len - 1].
                else,
                  do nothing.
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL int OS_VSNPrintf(char *dst, size_t len, const char *fmt, va_list vlist)
{
    return STD_TVSNPrintf(dst, len, fmt, vlist);
}

/*---------------------------------------------------------------------------*
  Name:         OS_VSNPrintfEx

  Description:  OS_VSNPrintfEx is an addition of binary output to OS_VSNPrintfEx.

  Arguments:    dst: Destination buffer
                len: Destination buffer size
                fmt: Format string
                vlist: Parameters

  Returns:      Length of the generated string (except '\0').
                if(result < len),
                  put NUL in dst[result].
                else if(len > 0),
                  put NUL in dst[len - 1].
                else,
                  do nothing.
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL int OS_VSNPrintfEx(char *dst, size_t len, const char *fmt, va_list vlist)
{
    char    buf[24];
    int     n_buf;
    char    prefix[2];
    int     n_prefix;

    const char *s = fmt;

    dst_string str;
    str.len = len, str.cur = str.base = dst;

    while (*s)
    {
        if ((unsigned int)(((unsigned char)*s ^ 0x20) - 0xA1) < 0x3C)
        {
            /* Shift JIS character */
            string_put_char(&str, *s++);
            if (*s)
                string_put_char(&str, *s++);
        }
        else if (*s != '%')
        {
            /* normal ASCII character */
            string_put_char(&str, *s++);
        }
        else
        {
            /* output with format */
            enum
            {
                flag_blank = 000001,   /* ' ' */
                flag_plus = 000002,    /* '+' */
                flag_sharp = 000004,   /* '#' */
                flag_minus = 000010,   /* '-' */
                flag_zero = 000020,    /* '0' */
                flag_l1 = 000040,      /* "l" */
                flag_h1 = 000100,      /* "h" */
                flag_l2 = 000200,      /* "ll" */
                flag_h2 = 000400,      /* "hh" */
                flag_unsigned = 010000, /* 'o', 'u', ... */
                flag_binary = 020000,  /* 'b' */
                flag_colon = 040000,   /* ':' */
                flag_end
            };
            int     flag = 0, width = 0, precision = -1, radix = 10;
            char    hex_char = 'a' - 10;
            const char *p_start = s;
            /* flags */
            for (;;)
            {
                switch (*++s)
                {
                case '+':
                    if (s[-1] != ' ')
                        break;
                    flag |= flag_plus;
                    continue;
                case ' ':
                    flag |= flag_blank;
                    continue;
                case '-':
                    flag |= flag_minus;
                    continue;
                case '0':
                    flag |= flag_zero;
                    continue;
                case ':':
                    flag |= flag_colon;
                    continue;
                }
                break;
            }
            /* Width */
            if (*s == '*')
            {
                ++s, width = va_arg(vlist, int);
                if (width < 0)
                    width = -width, flag |= flag_minus;
            }
            else
            {
                while ((*s >= '0') && (*s <= '9'))
                    width = (width * 10) + *s++ - '0';
            }
            /* precision */
            if (*s == '.')
            {
                ++s, precision = 0;
                if (*s == '*')
                {
                    ++s, precision = va_arg(vlist, int);
                    if (precision < 0)
                        precision = -1;
                }
                else
                {
                    while ((*s >= '0') && (*s <= '9'))
                        precision = (precision * 10) + *s++ - '0';
                }
            }
            /* Options */
            switch (*s)
            {
            case 'h':
                if (*++s != 'h')
                    flag |= flag_h1;
                else
                    ++s, flag |= flag_h2;
                break;
            case 'l':
                if (*++s != 'l')
                    flag |= flag_l1;
                else
                    ++s, flag |= flag_l2;
                break;
            }

            /* type */
            switch (*s)
            {
            case 'd':                 /* signed decimal */
            case 'i':                 /* signed decimal */
                goto put_integer;
            case 'o':                 /* unsigned octal */
                radix = 8;
                flag |= flag_unsigned;
                goto put_integer;
            case 'u':                 /* unsigned decimal */
                flag |= flag_unsigned;
                goto put_integer;
            case 'X':                 /* unsigned hexadecimal */
                hex_char = 'A' - 10;
                goto put_hexadecimal;
            case 'x':                 /* unsigned hexadecimal */
                goto put_hexadecimal;
            case 'p':                 /* pointer */
                /* equal to code warrior */
                flag |= flag_sharp;
                precision = 8;
                goto put_hexadecimal;

            case 'C':                 /* unicode-character */
                flag |= flag_l1;
            case 'c':                 /* character */
                if (precision >= 0)
                    goto put_invalid;
                {
                    int     c = va_arg(vlist, int);
                    width -= 1;
                    if (flag & flag_minus)
                    {
                        if (flag & flag_l1)
                        {
                            char    dst[2];

                            if (STD_ConvertCharUnicodeToSjis(dst, (u16)c) == 1)
                            {
                                string_put_char(&str, dst[0]);
                            }
                            else
                            {
                                string_put_string(&str, dst, 2);
                            }
                        }
                        else
                        {
                            string_put_char(&str, (char)c);
                        }
                        string_fill_char(&str, ' ', width);
                    }
                    else
                    {
                        char    pad = (char)((flag & flag_zero) ? '0' : ' ');
                        string_fill_char(&str, pad, width);

                        if (flag & flag_l1)
                        {
                            char    dst[2];

                            if (STD_ConvertCharUnicodeToSjis(dst, (u16)c) == 1)
                            {
                                string_put_char(&str, dst[0]);
                            }
                            else
                            {
                                string_put_string(&str, dst, 2);
                            }
                        }
                        else
                        {
                            string_put_char(&str, (char)c);
                        }
                    }
                    ++s;
                }
                break;

            case 'S':                 /* unicode-string */
                flag |= flag_l1;
            case 's':                 /* string */
                {
                    int     n_buf = 0;
                    const char *p_buf = va_arg(vlist, const char *);

                    if (precision < 0)
                    {
                        while (p_buf[n_buf])
                        {
                            if (flag & flag_l1)
                            {
                                n_buf += 2;
                            }
                            else
                            {
                                ++n_buf;
                            }
                        }
                    }
                    else
                    {
                        while ((n_buf < precision) && p_buf[n_buf])
                        {
                            if (flag & flag_l1)
                            {
                                n_buf += 2;
                            }
                            else
                            {
                                ++n_buf;
                            }
                        }
                    }
                    if (flag & flag_l1)
                    {
                        n_buf /= 2;
                    }
                    width -= n_buf;
                    if (flag & flag_minus)
                    {
                        if (flag & flag_l1)
                        {
                            u16    *w_buf = (u16 *)p_buf;
                            char    dst[2];

                            while (n_buf--)
                            {
                                if (STD_ConvertCharUnicodeToSjis(dst, *w_buf) == 1)
                                {
                                    string_put_char(&str, dst[0]);
                                }
                                else
                                {
                                    string_put_string(&str, dst, 2);
                                }
                                w_buf++;
                            }
                        }
                        else
                        {
                            string_put_string(&str, p_buf, n_buf);
                        }
                        string_fill_char(&str, ' ', width);
                    }
                    else
                    {
                        char    pad = (char)((flag & flag_zero) ? '0' : ' ');
                        string_fill_char(&str, pad, width);

                        if (flag & flag_l1)
                        {
                            u16    *w_buf = (u16 *)p_buf;
                            char    dst[2];

                            while (n_buf--)
                            {
                                if (STD_ConvertCharUnicodeToSjis(dst, *w_buf) == 1)
                                {
                                    string_put_char(&str, dst[0]);
                                }
                                else
                                {
                                    string_put_string(&str, dst, 2);
                                }
                                w_buf++;
                            }
                        }
                        else
                        {
                            string_put_string(&str, p_buf, n_buf);
                        }
                    }
                    ++s;
                }
                break;

            case 'B':                 /* binary */
                hex_char = 'A' - 10;
            case 'b':                 /* binary */
                {
                    int     n_buf = width;
                    int     count = 0;
                    char   *p_buf = va_arg(vlist, char *);
                    int     column_count;

                    if (n_buf <= 0)
                    {
                        n_buf = 1;
                    }
                    if (precision == 0)
                    {
                        precision = -1;
                    }
                    column_count = precision;

                    while (1)
                    {
                        char    left_char, right_char;
                        int     left = ((p_buf[count] & 0xF0) >> 4);
                        int     right = (p_buf[count] & 0x0F);

                        left_char = (char)((left < 10) ? (left + '0') : (left + hex_char));
                        right_char = (char)((right < 10) ? (right + '0') : (right + hex_char));

                        string_put_char(&str, left_char);
                        string_put_char(&str, right_char);

                        count++;

                        if (count >= n_buf)
                            break;

                        if (column_count > 0)
                        {
                            --column_count;
                        }

                        if (column_count == 0)
                        {
                            string_put_char(&str, '\n');
                            if ((flag & flag_blank) && !(flag & flag_minus))
                            {
                                string_put_char(&str, ' ');
                            }
                            column_count = precision;
                        }
                        else if (flag & flag_colon)
                        {
                            string_put_char(&str, ':');
                        }
                        else if (flag & flag_blank)
                        {
                            string_put_char(&str, ' ');
                        }
                    }
                    ++s;
                }
                break;

            case 'n':                 /* store the number of output */
                {
                    int     pos = str.cur - str.base;
                    if (flag & flag_h2)
                        ;
                    else if (flag & flag_h1)
                        *va_arg(vlist, signed short *) = (signed short)pos;
                    else if (flag & flag_l2)
                        *va_arg(vlist, u64 *) = (u64)pos;
                    else
                        *va_arg(vlist, signed int *) = (signed int)pos;
                }
                ++s;
                break;

            case '%':                 /* output '%' */
                if (p_start + 1 != s)
                    goto put_invalid;
                string_put_char(&str, *s++);
                break;

            default:                  /* invalid type */
                goto put_invalid;

              put_invalid:
                string_put_string(&str, p_start, s - p_start);
                break;

              put_hexadecimal:
                radix = 16;
                flag |= flag_unsigned;
              put_integer:
                {
                    u64     val = 0;
                    n_prefix = 0;

                    if (flag & flag_minus)
                        flag &= ~flag_zero;
                    if (precision < 0)
                        precision = 1;
                    else
                        flag &= ~flag_zero;

                    if (flag & flag_unsigned)
                    {
                        if (flag & flag_h2)
                            val = va_arg(vlist, unsigned char);
                        else if (flag & flag_h1)
                            val = va_arg(vlist, unsigned short);
                        else if (flag & flag_l2)
                            val = va_arg(vlist, u64);
                        else
                            val = va_arg(vlist, unsigned long);
                        flag &= ~(flag_plus | flag_blank);
                        if (flag & flag_sharp)
                        {
                            if (radix == 16)
                            {
                                if (val != 0)
                                {
                                    prefix[0] = (char)(hex_char + (10 + 'x' - 'a'));
                                    prefix[1] = '0';
                                    n_prefix = 2;
                                }
                            }
                            else if (radix == 8)
                            {
                                prefix[0] = '0';
                                n_prefix = 1;
                            }
                        }
                    }
                    else
                    {
                        if (flag & flag_h2)
                            val = va_arg(vlist, char);
                        else if (flag & flag_h1)
                            val = va_arg(vlist, short);
                        else if (flag & flag_l2)
                            val = va_arg(vlist, u64);
                        else
                            val = va_arg(vlist, long);
                        if ((val >> 32) & 0x80000000)
                        {
                            val = ~val + 1;
                            prefix[0] = '-';
                            n_prefix = 1;
                        }
                        else
                        {
                            if (val || precision)
                            {
                                if (flag & flag_plus)
                                {
                                    prefix[0] = '+';
                                    n_prefix = 1;
                                }
                                else if (flag & flag_blank)
                                {
                                    prefix[0] = ' ';
                                    n_prefix = 1;
                                }
                            }
                        }
                    }
                    n_buf = 0;
                    switch (radix)
                    {
                    case 8:
                        while (val != 0)
                        {
                            int     d = (int)(val & 0x07);
                            val >>= 3;
                            buf[n_buf++] = (char)(d + '0');
                        }
                        break;
                    case 10:
                        if ((val >> 32) == 0)
                        {
#if defined(SDK_CW) || defined(SDK_RX) || defined(__MWERKS__)
#pragma optimize_for_size off
#endif
                            u32     v = (u32)val;
                            while (v != 0)
                            {
                                // If the operation is division of a u32 and a constant, the compiler automatically converts it to addition using magic numbers
                                // 
                                u32     r = v / 10;
                                int     d = (int)(v - (r * 10));
                                v = r;
                                buf[n_buf++] = (char)(d + '0');
                            }
                        }
                        else
                        {
                            while (val != 0)
                            {
                                u64     r = val / 10;
                                int     d = (int)(val - (r * 10));
                                val = r;
                                buf[n_buf++] = (char)(d + '0');
                            }
                        }
                        break;
                    case 16:
                        while (val != 0)
                        {
                            int     d = (int)(val & 0x0f);
                            val >>= 4;
                            buf[n_buf++] = (char)((d < 10) ? (d + '0') : (d + hex_char));
                        }
                        break;
                    }
                    if ((n_prefix > 0) && (prefix[0] == '0'))
                    {
                        n_prefix = 0;
                        buf[n_buf++] = '0';
                    }
                }
                goto put_to_stream;

              put_to_stream:
                {
                    int     n_pad = precision - n_buf;
                    if (flag & flag_zero)
                    {
                        if (n_pad < width - n_buf - n_prefix)
                            n_pad = width - n_buf - n_prefix;
                    }
                    if (n_pad > 0)
                        width -= n_pad;
                    width -= n_prefix + n_buf;
                    if (!(flag & flag_minus))
                        string_fill_char(&str, ' ', width);
                    while (n_prefix > 0)
                        string_put_char(&str, prefix[--n_prefix]);
                    string_fill_char(&str, '0', n_pad);
                    while (n_buf > 0)
                        string_put_char(&str, buf[--n_buf]);
                    if (flag & flag_minus)
                        string_fill_char(&str, ' ', width);
                    ++s;
                }
                break;
            }
        }
    }

    if (str.len > 0)
        *str.cur = '\0';
    else if (len > 0)
        str.base[len - 1] = '\0';
    return str.cur - str.base;
}


#if defined(SDK_CW_WARNOFF_SAFESTRB)
#include <nitro/codereset.h>
#endif

//================================================================================
//              Set output of print (for IS-TWL-DEBUGGER)
//================================================================================
#ifndef SDK_FINALROM
#ifdef  SDK_LINK_ISTD
/*---------------------------------------------------------------------------*
  Name:         OS_SetPrintOutput

  Description:  Specifies the output window OS_Printf uses.
                This is only for IS-TWL-DEBUGGER.

  Arguments:    proc : processor
                         OS_PRINT_OUTPUT_PROC_ARM9     : ARM9
                         OS_PRINT_OUTPUT_PROC_ARM7     : ARM7
                         OS_PRINT_OUTPUT_PROC_ARM9ERR  : ARM9 for error
                         OS_PRINT_OUTPUT_PROC_ARM7ERR  : ARM7 for error
                num: Window number (0-3)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OS_SetPrintOutput( OSPrintOutputProc proc, int num )
{
    SDK_ASSERT( (0<=num && num<=OS_PRINT_OUTPUT_WINDOW_MAX) || num==OS_PRINT_OUTPUT_NONE );
    SDK_ASSERT( 0<=(int)proc && (int)proc<=OS_PRINT_OUTPUT_PROC_ARM7ERR );
	*(u8*)(HW_PRINT_OUTPUT_ARM9 + proc) = (u8)num;
}
#endif
#endif

//================================================================================
//              FPutString hook
//================================================================================
#ifndef SDK_FINALROM
/*---------------------------------------------------------------------------*
  Name:         OS_SetFPutStringHook

  Description:  Sets hook function in FPutString.

  Arguments:    func : hook function

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OS_SetFPutStringHook( OSFPutStringHookType func )
{
	OSi_FPutStringHook = func;
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetFPutStringHook

  Description:  get hook function in FPutString

  Arguments:    None.

  Returns:      hook function
 *---------------------------------------------------------------------------*/
OSFPutStringHookType OS_GetFPutStringHook(void)
{
	return OSi_FPutStringHook;
}
#endif

//================================================================================
//              DUMMY PRINT (stub for FINALROM)
//================================================================================
#ifdef SDK_FINALROM
#ifdef OS_Printf
#undef OS_Printf
#endif
void    OS_Printf(const char *fmt, ...);
void OS_Printf(const char *fmt, ...)
{
#pragma unused( fmt )
}
#endif

//================================================================================
//              Blocking mode
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_SetPrintBlocking

  Description:  set blocking mode on debug print

  Arguments:    sw : blocking mode to set.
                     TRUE if want to block mode, or FALSE if not.

  Returns:      None
 *---------------------------------------------------------------------------*/
#ifndef SDK_FINALROM
void ISTDPrintSetBlockingMode(BOOL);
void ISDPrintSetBlockingMode(int);

void OS_SetPrintBlockingMode( BOOL sw )
{
    if ( OS_IsRunOnTwl() )
    {
        ISTDPrintSetBlockingMode( sw );
    }
    else
    {
        ISDPrintSetBlockingMode( (int)sw );
    }
}
#endif // ifndef SDK_FINALROM

/*---------------------------------------------------------------------------*
  Name:         OS_GetPrintBlockingMode

  Description:  get blocking mode on debug print

  Arguments:    None.

  Returns:      blocking mode.
                TRUE if blocking is available, FALSE if not.
 *---------------------------------------------------------------------------*/
#ifndef SDK_FINALROM
BOOL ISTDPrintGetBlockingMode(void);
int  ISDPrintGetBlockingMode(void);

BOOL OS_GetPrintBlockingMode( void )
{
    if ( OS_IsRunOnTwl() )
    {
        return ISTDPrintGetBlockingMode()? TRUE: FALSE;
    }
    else
    {
        return ISDPrintGetBlockingMode()? TRUE: FALSE;
    }
}
#endif // ifndef SDK_FINALROM
