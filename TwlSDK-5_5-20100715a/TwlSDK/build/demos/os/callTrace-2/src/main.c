/*---------------------------------------------------------------------------*
  Project:  NitroSDK - OS - demos - callTrace-2
  File:     main.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <nitro.h>

int     test1(int a);
int     test2(int a);
int     test3(int a);
void    test4(int a);
void    test5(int a);

//---- thread
#define STACK_SIZE      1024

void    proc1(void *arg);
void    proc2(void *arg);

OSThread thread1;
OSThread thread2;

u64     stack1[STACK_SIZE / sizeof(u64)];
u64     stack2[STACK_SIZE / sizeof(u64)];

#define         THREAD1_PRIO    10
#define         THREAD2_PRIO    11

#define MYTRACEBUFFER_SIZE 0x400
u32     myTraceBuffer0[MYTRACEBUFFER_SIZE / sizeof(u32)];
u32     myTraceBuffer1[MYTRACEBUFFER_SIZE / sizeof(u32)];
u32     myTraceBuffer2[MYTRACEBUFFER_SIZE / sizeof(u32)];

SDK_WEAK_SYMBOL asm void __PROFILE_ENTRY( void ){ bx lr }
SDK_WEAK_SYMBOL asm void __PROFILE_EXIT( void ){ bx lr }

//================================================================

//----------------------------------------------------------------
// test1
//
int test1(int a)
{
    return test2(a + 1);
}

//----------------------------------------------------------------
// test2
//
int test2(int a)
{
    return test3(a + 2);
}

//----------------------------------------------------------------
// test3
//
int test3(int a)
{
    OS_DumpCallTrace();
    return a + 4;
}

//----------------------------------------------------------------
// test4
//
void test4(int a)
{
    test5(a + 3);
    return;
}

//----------------------------------------------------------------
// test5
//
void test5(int a)
{
#pragma unused( a )
    return;
}

//================================================================================
/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
#pragma profile off
void NitroMain(void)
{
    s32     n;

    OS_Init();
    OS_CreateThread(&thread1, proc1, NULL, stack1 + STACK_SIZE / sizeof(u64), STACK_SIZE,
                    THREAD1_PRIO);
    OS_CreateThread(&thread2, proc2, NULL, stack2 + STACK_SIZE / sizeof(u64), STACK_SIZE,
                    THREAD2_PRIO);
    OS_WakeupThreadDirect(&thread1);
    OS_WakeupThreadDirect(&thread2);

    OS_InitCallTrace(&myTraceBuffer0, MYTRACEBUFFER_SIZE, OS_CALLTRACE_LOG);

    for (n = 0; n < 5; n++)
    {
        OS_Printf("Idle\n");
        OS_WakeupThreadDirect(&thread2);

        test4(n);
    }

    //---- display all thread trace
    {
        OS_Printf("----------------\n");
        OS_Printf("---- thread 1: trace\n");
        OS_DumpThreadCallTrace(&thread1);
        OS_Printf("---- thread 2: trace\n");
        OS_DumpThreadCallTrace(&thread2);
        OS_Printf("---- idle thread: trace\n");
        OS_DumpThreadCallTrace(NULL);
    }

    OS_Printf("==== Finish sample.\n");
    OS_Terminate();
}

/*---------------------------------------------------------------------------*
  Name:         proc1

  Description:  procedure of thread1

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void proc1(void *arg)
{
#pragma unused(arg)

    static int cnt = 0;

    OS_InitCallTrace(&myTraceBuffer1, MYTRACEBUFFER_SIZE, OS_CALLTRACE_STACK);
    while (1)
    {
        OS_Printf("thread1\n");
        if (cnt++ == 3)
        {
            (void)test1(0x100);
        }
        OS_SleepThread(NULL);
    }
}

/*---------------------------------------------------------------------------*
  Name:         proc2

  Description:  procedure of thread2

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void proc2(void *arg)
{
#pragma unused(arg)

    static int cnt = 0;

    OS_InitCallTrace(&myTraceBuffer2, MYTRACEBUFFER_SIZE, OS_CALLTRACE_STACK);
    while (1)
    {
        OS_Printf("thread2\n");
        if (cnt++ == 3)
        {
            (void)test1(0x200);
        }
        OS_SleepThread(NULL);
        OS_WakeupThreadDirect(&thread1);
    }
}

/*====== End of main.c ======*/
