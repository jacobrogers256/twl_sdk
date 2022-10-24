/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS
  File:     os_exception.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-10-07#$
  $Rev: 11084 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#include <nitro/hw/common/armArch.h>
#include <nitro/memorymap.h>
#include <nitro/os.h>

//#define OSiDEBUG

//---- Displaying function OSi_ExPrintf
#ifdef SDK_ARM9
#define OSi_ExPrintf( ... )   OS_FPrintf(OS_PRINT_OUTPUT_ERROR, __VA_ARGS__)
#else /* SDK_ARM7 */
#define OSi_ExPrintf( ... )   OS_TPrintf(__VA_ARGS__)
#endif

static asm void OSi_ExceptionHandler( void );
static asm void OSi_GetAndDisplayContext( void );
static asm void OSi_SetExContext( void );
static     void OSi_DisplayExContext( void );

//---- for system use (using at OS_ResetSystem())
u32  OSi_GetOriginalExceptionHandler(void);

//---- Context for exception display
typedef struct
{
    OSContext   context;
    u32         cp15;
    u32         spsr;
    u32         exinfo;
#ifdef OSiDEBUG
    u32         debug[4];
#endif
} OSiExContext;

static OSiExContext OSi_ExContext;

//---- User's exception handler
static OSExceptionHandler OSi_UserExceptionHandler;
static void *OSi_UserExceptionHandlerArg;

//---- Debugger's exception handler
static void *OSi_DebuggerHandler = NULL;

//---- exception handler
static u32 OSi_OriginalHandler;

//================================================================================
//  Exception vector
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_SetExceptionVectorUpper

  Description:  Sets exception vector to high address.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#include <nitro/code32.h>
asm void OS_SetExceptionVectorUpper( void )
{
    mrc     p15, 0, r0, c1, c0, 0
    orr     r0, r0, #HW_C1_EXCEPT_VEC_UPPER
    mcr     p15, 0, r0, c1, c0, 0
    bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         OS_SetExceptionVectorLower

  Description:  Sets exception vector to low address.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void OS_SetExceptionVectorLower( void )
{
    mrc     p15, 0, r0, c1, c0, 0
    bic     r0, r0, #HW_C1_EXCEPT_VEC_UPPER
    mcr     p15, 0, r0, c1, c0, 0
    bx      lr
}
#include <nitro/codereset.h>


//================================================================================
//  Exception handling
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_InitException

  Description:  Initializes exception-handling system.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OS_InitException(void)
{
#ifdef SDK_ARM9
	u32* excpPtr = (u32*)HW_EXCP_VECTOR_MAIN;
#else
	u32* excpPtr = (u32*)HW_EXCP_VECTOR_BUF;
#endif
	OSi_OriginalHandler = *(u32*)HW_EXCP_VECTOR_MAIN;

	//---- Consider for IS-NITRO-DEBUGGER exception handler
	OSi_DebuggerHandler = (void*)( (0x2600000 <= OSi_OriginalHandler && OSi_OriginalHandler < 0x2800000)? OSi_OriginalHandler: NULL );

	if ( OSi_DebuggerHandler == NULL )
	{
		*excpPtr = (u32)OSi_ExceptionHandler;
#ifdef SDK_ARM9
		if ( ! OS_IsRunOnTwl() )
		{
			*(u32*)((u32)excpPtr & ~0x800000) = (u32)OSi_ExceptionHandler;
		}
#endif
	}

    //---- User's handler
    OSi_UserExceptionHandler = NULL;
}

/*---------------------------------------------------------------------------*
  Name:         OSi_PutbackExceptionHandler

  Description:  Put back exception handler
                (for system use)

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
u32  OSi_GetOriginalExceptionHandler(void)
{
    return OSi_OriginalHandler;
}

#if defined(SDK_ARM9)
/*
 * Make a self-determination and call a user handler before passing a control to the debugger.
 * However, the following undefined commands are breakpoints and will be ignored.
 * +-------------------+------------+--------+
 * |     debugger      |    ARM     | THUMB  |
 * +-------------------+------------+--------+
 * | CodeWarrior -1.1  | 0xE6000010 | 0xBE00 |
 * | CodeWarrior  1.2- | 0xE7FDDEFE | 0xDEFE |
 * | IS-NITRO-DEBUGGER | 0xE7FFFFFF | 0xEFFF |
 * +-------------------+------------+--------+
 */
/*---------------------------------------------------------------------------*
  Name:         OSi_DebuggerExceptionHook

  Description:  Exception hook that calls user handler before debugger handler.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#include <nitro/code32.h>
static u32 OSi_ExceptionHookStack[8];
asm static void OSi_DebuggerExceptionHook(void)
{
    /* Carefully save the work register such that sp is not changed */
    ldr       r12, =OSi_ExceptionHookStack
    stmia     r12, {r0-r4,sp,lr}
    mrs       r4, CPSR
    /*
     * Determine whether there is an exception that the user should be notified of.
     * - Exceptions the user should be notified of are either ABT (abort) or UND (undefined command).
     * - UND may occur as a result of a breakpoint.
     * - No notification should be made to the user for UND and breakpoint commands.
     */
    mrs       r0, CPSR
    and       r0, r0, #0x1f
    teq       r0, #0x17
    beq       user_exception
    teq       r0, #0x1b
    bne       user_exception_end
is_und:
    /* Determinations for both ARM/THUMB and IS-NITRO-DEBUGGER/CodeWarrior */
    bic       r0, sp, #1
    ldr       r1, [r0, #4]
    ldr       r0, [r0, #12]
    tst       r1, #0x20
    beq       is_und_arm
is_und_thumb:
    bic       r0, r0, #1
    ldrh      r0, [r0, #-2]
    ldr       r1, =0x0000EFFF
    cmp       r0, r1
    beq       user_exception_end
    ldr       r1, =0x0000DEFE
    cmp       r0, r1
    beq       user_exception_end
    ldr       r1, =0x0000BE00
    cmp       r0, r1
    beq       user_exception_end
    b         user_exception
is_und_arm:
    bic       r0, r0, #3
    ldr       r0, [r0, #-4]
    ldr       r1, =0xE7FFFFFF
    cmp       r0, r1
    beq       user_exception_end
    ldr       r1, =0xE7FDDEFE
    cmp       r0, r1
    beq       user_exception_end
    ldr       r1, =0xE6000010
    cmp       r0, r1
    beq       user_exception_end
    b         user_exception
user_exception:
    /* Notification to the user's exception handler */
    ldmia     r12, {r0-r1}
    ldr       r12, =HW_ITCM_END
    stmfd     r12!, {r0-r3,sp,lr}
    and       r0, sp, #1
    mov       sp, r12
    bl        OSi_GetAndDisplayContext
    ldmfd     sp!, {r0-r3,r12,lr}
    mov       sp, r12
user_exception_end:
    /* Restore the saved work register and pass the original control to the debugger */
    msr       CPSR_cxsf, r4
    ldr       r12, =OSi_ExceptionHookStack
    ldmia     r12, {r0-r4,sp,lr}
    ldr       r12, =OSi_DebuggerHandler
    ldr       r12, [r12]
    cmp       r12, #0
    bxne      r12
    bx        lr
}
#include <nitro/codereset.h>

/*---------------------------------------------------------------------------*
  Name:         OS_EnableUserExceptionHandlerOnDebugger

  Description:  Enables user exception handler even if running on the debugger.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OS_EnableUserExceptionHandlerOnDebugger(void)
{
    if (OSi_DebuggerHandler)
    {
        *(u32*)(HW_EXCP_VECTOR_MAIN) = (u32)OSi_DebuggerExceptionHook;
    }
}
#endif /* defined(SDK_ARM9) */

/*---------------------------------------------------------------------------*
  Name:         OS_SetUserExceptionHandler

  Description:  Sets user exception handler and its argument.

  Arguments:    handler:    Exception hander
                arg:        Its argument

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OS_SetUserExceptionHandler(OSExceptionHandler handler, void *arg)
{
    OSi_UserExceptionHandler = handler;
    OSi_UserExceptionHandlerArg = arg;
}

/*---------------------------------------------------------------------------*
  Name:         OSi_ExceptionHandler

  Description:  System exception handler.
                User handler is called from here.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
//
//      Status of stack is {cp15,spsr,r12,lr}
//      LSB = 1 means coming by reset
//
#include <nitro/code32.h>
asm void OSi_ExceptionHandler( void )
{
    //---- Call debugger monitor handler (if exists)
    ldr       r12, =OSi_DebuggerHandler // Only r12 can be destructed
    ldr       r12, [r12]
    cmp       r12, #0
    movne     lr, pc
    bxne      r12

    //---- Setting stack pointer <------------------------- consider later
#ifdef SDK_ARM9
    //---- ARM9 stack
    ldr       r12, =HW_ITCM_END
#else
    //---- ARM7 stack
    ldr       r12, =0x3806000
#endif
    stmfd     r12!, {r0-r3,sp,lr}

    and       r0, sp, #1
    mov       sp, r12
    
    mrs       r1, CPSR
    and       r1, r1, #0x1f

    //---- If ABORT exception, stop
    teq       r1, #0x17
    bne       @10
    bl        OSi_GetAndDisplayContext
    b         usr_return

@10:
    //---- If UNDEF exception, stop
    teq       r1, #0x1b
    bne       usr_return
    bl        OSi_GetAndDisplayContext

usr_return:
    ldr       r12, =OSi_DebuggerHandler
    ldr       r12, [r12]
    cmp       r12, #0
@1: beq       @1

//(Stop now even if debugger)
@2: 
    mov       r0,r0 // Nop
    b         @2

    ldmfd     sp!, {r0-r3, r12, lr}
    mov       sp, r12
    bx        lr
}

/*---------------------------------------------------------------------------*
  Name:         OSi_GetAndDisplayContext

  Description:  Stops after displaying registers.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static asm void OSi_GetAndDisplayContext( void )
{
    stmfd     sp!, {r0, lr} /* 8-byte alignment of the call stack */

    //---- Set exception context 
    bl        OSi_SetExContext
    //---- Display exception context (and call user callback)
    bl        OSi_DisplayExContext

    ldmfd     sp!, {r0, lr} /* 8-byte alignment of the call stack */
    bx        lr
}

/*---------------------------------------------------------------------------*
  Name:         OSi_SetExContext

  Description:  Sets context when exception occurred.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
//       Explanation registers at the top of this function.
//
//       In r12, {r0-r3,sp} (sp is exception sp) stored.
//       In this sp, {cp15,spsr,r12,lr} (cp15,spsr,r12,lr is registers when exception occurred) stored.
//       r4-r11 is registers then exception occurred.
//       If you want to know spsr,sp,lr, please switch bank and read.
//
static asm void OSi_SetExContext( void )
{
    //---- Pointer ExContext structure
    ldr    r1, =OSi_ExContext;

#ifdef OSiDEBUG
    //---- (For debug)
    mrs    r2, CPSR
    str    r2,  [r1, #OSiExContext.debug[1] ]
#endif

    //---- Store bit which means which is the reason, reset or exception
    str    r0, [r1, #OSiExContext.exinfo ]

    //---- Store r0 - r3
    ldr    r0, [r12,#0]
    str    r0, [r1, #OS_CONTEXT_R0]
    ldr    r0, [r12,#4]
    str    r0, [r1, #OS_CONTEXT_R1]
    ldr    r0, [r12,#8]
    str    r0, [r1, #OS_CONTEXT_R2]
    ldr    r0, [r12, #12]
    str    r0, [r1, #OS_CONTEXT_R3]
    ldr    r2, [r12, #16]
    bic    r2, r2, #1

    //---- Store r4 - r11
    add    r0, r1, #OS_CONTEXT_R4
    stmia  r0, {r4-r11}

#ifdef OSiDEBUG
    //---- (For debug)
    str    r12, [r1, #OSiExContext.debug[0] ]
#endif

#ifdef SDK_ARM9
    //---- Get {cp15,cpsr,r12,pc} from stack
    ldr    r0, [r2, #0]
    str    r0, [r1, #OSiExContext.cp15 ]
    ldr    r3, [r2, #4]
    str    r3, [r1, #OS_CONTEXT_CPSR]
    ldr    r0, [r2, #8]
    str    r0, [r1, #OS_CONTEXT_R12]
    ldr    r0, [r2, #12]
    str    r0, [r1, #OS_CONTEXT_PC_PLUS4]
#else // ifdef SDK_ARM9
    //---- Get {cpsr,r12,pc} from stack
    mov    r0, #0
    str    r0, [r1, #OSiExContext.cp15]
    ldr    r3, [r2, #0]
    str    r3, [r1, #OS_CONTEXT_CPSR]
    ldr    r0, [r2, #4]
    str    r0, [r1, #OS_CONTEXT_R12]
    ldr    r0, [r2, #8]
    str    r0, [r1, #OS_CONTEXT_PC_PLUS4]
#endif // ifdef SDK_ARM9

    //---- Set mode to one which exception occurred,
    //     but disable IRQ
    mrs    r0, CPSR
    orr    r3, r3, #0x80
    bic    r3, r3, #0x20
    msr    CPSR_cxsf, r3
   
    //---- Get sp, lr, spsr
    str    sp, [r1, #OS_CONTEXT_R13]
    str    lr, [r1, #OS_CONTEXT_R14]
    mrs    r2, SPSR

#ifdef OSiDEBUG
    //---- (For debug)
    str    r2, [r1, #OSiExContext.debug[3] ]
#endif

    //---- Restore mode
    msr    CPSR_cxsf, r0
    bx     lr
}

/*---------------------------------------------------------------------------*
  Name:         OSi_DisplayExContext

  Description:  Stops after display exception context.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void OSi_DisplayExContext()
{
#ifndef SDK_FINALROM
    int     i;

    OSi_ExPrintf("**** Exception Occurred ****\n");

    //---------------- Displaying registers
    //---- For R0-15 Registers
    for (i = 0; i < 13; i++)
    {
        OSi_ExPrintf("R%02d=%08X  %c", i, OSi_ExContext.context.r[i], ((i & 3) == 3) ? '\n' : ' ');
    }
    OSi_ExPrintf("SP =%08X   ", OSi_ExContext.context.sp);
    OSi_ExPrintf("LR =%08X   ", OSi_ExContext.context.lr);
    OSi_ExPrintf("PC =%08X\n", OSi_ExContext.context.pc_plus4);

    //---- For status Registers
#ifdef SDK_ARM9
    OSi_ExPrintf("  CPSR=%08X  SPSR=%08X  CP15=%08X\n",
                 OSi_ExContext.context.cpsr, OSi_ExContext.spsr, OSi_ExContext.cp15);
#else
    OSi_ExPrintf("  CPSR=%08X  SPSR=%08X\n", OSi_ExContext.context.cpsr, OSi_ExContext.spsr);
#endif

#ifdef OSiDEBUG
    //---- (For debug)
    for (i = 0; i < 4; i++)
    {
        OSi_ExPrintf("DEBUG%02d=%08X  ", i, OSi_ExContext.debug[i]);
    }
#endif
    OSi_ExPrintf("\n\n");
#endif

    //---------------- User's callback
    if (OSi_UserExceptionHandler)
    {
        //---- Force to become SYS mode,
        //     but use current stack not SYS mode stack
        asm
        {
          /* *INDENT-OFF* */

		  //---- Save cpsr and sp
          mrs      r2, CPSR
          mov      r0, sp
          ldr      r1, =0x9f
          msr      CPSR_cxsf, r1
          mov      r1, sp
          mov      sp, r0
          stmfd    sp!, {r1,r2}
#ifdef SDK_ARM9
          bl       OS_EnableProtectionUnit
#endif
		  //---- Context
          ldr      r0, =OSi_ExContext

		  //---- Arg
          ldr      r1, =OSi_UserExceptionHandlerArg
          ldr      r1, [r1]

		  //---- User handler
          ldr      r12, =OSi_UserExceptionHandler
          ldr      r12, [r12]
          ldr      lr, =@1
          bx       r12
@1:
#ifdef SDK_ARM9
          bl       OS_DisableProtectionUnit
#endif

		  //---- Restore cpsr and sp
          ldmfd    sp!, {r1,r2}
          mov      sp, r1
          msr      CPSR_cxsf, r2
          /* *INDENT-ON* */
        }
    }

}

#include <nitro/codereset.h>
