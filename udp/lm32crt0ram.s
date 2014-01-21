/****************************************************************************
**
**  Name: lm32crt0ram.S
**
**  Description:
**        Implements boot-code that calls LatticeDDInit (that calls main())
**        Implements exception handlers (actually, redirectors)
**
**  $Revision: 1 $
**
** Disclaimer:
**
**   This source code is intended as a design reference which
**   illustrates how these types of functions can be implemented.  It
**   is the user's responsibility to verify their design for
**   consistency and functionality through the use of formal
**   verification methods.  Lattice Semiconductor provides no warranty
**   regarding the use or functionality of this code.
**
** --------------------------------------------------------------------
**
**                     Lattice Semiconductor Corporation
**                     5555 NE Moore Court
**                     Hillsboro, OR 97214
**                     U.S.A
**
**                     TEL: 1-800-Lattice (USA and Canada)
**                          (503)268-8001 (other locations)
**
**                     web:   http://www.latticesemi.com
**                     email: techsupport@latticesemi.com
**
** --------------------------------------------------------------------------
**
**  Change History (Latest changes on top)
**
**  Ver    Date        Description
** --------------------------------------------------------------------------
**  3.1   Jun-18-2008  Added __MICO_NO_INTERRUPTS__ preprocessor
**                     option to exclude invoking MicoISRHandler
**                     to reduce code-size in apps that don't use
**                     interrupts
**
**  3.0   Mar-25-2008  Added Header
**
**---------------------------------------------------------------------------
*****************************************************************************/

/*
 * LatticeMico32 C startup code.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


/* FIXME:  This define is either incorrect or poorly named */
#define __MICO_NO_INTERRUPTS__

/* Exception handlers - Must be 32 bytes long. */
        .section    .boot, "ax", @progbits
        .global     _start
_start:
        .global _reset_handler
        .type _reset_handler, @function
_reset_handler:
    xor r0, r0, r0
    wcsr    IE, r0
    mvhi    r1, hi(_reset_handler)
    ori     r1, r1, lo(_reset_handler)
    wcsr    EBA, r1
    calli   _crt0
    nop
    nop
        .size     _reset_handler, .-_reset_handler
        .global _breakpoint_handler
        .type _breakpoint_handler, @function

_breakpoint_handler:
.ifdef _GDB
    sw      (sp+0), ra    /* Calli is going to clobber ra, so store it before executing the calli */
    calli   _save_all     /* Save machine state (all the registers) */
    mvi     r1, 1         /* Set the Exception ID value */
    sw      (sp+136), r1
    sw      (sp+132), ba  /* Set the PC location */
    addi    r1, sp, 4     /* Set the first parameter to the structure loaded by _save_all */
    bi      _call_handler
    bi      _e_restore_all_and_return
.else
    calli   breakpoint_handler
    rcsr    r7, DEBA
    addi    r7, r7, 32
    b       r7
    nop
    nop
    nop
    nop
.endif

        .size     _breakpoint_handler, .-_breakpoint_handler
        .global _instruction_bus_error_handler
        .type _instruction_bus_error_handler, @function
_instruction_bus_error_handler:
.ifdef _GDB
    sw      (sp+0), ra    /* Calli is going to clobber ra, so store it before executing the calli */
    calli   _save_all     /* Save machine state (all the registers) */
    mvi     r1, 2         /* Set the Exception ID value */
    sw      (sp+136), r1
    sw      (sp+132), ea  /* Set the PC location */
    addi    r1, sp, 4        /* Set the first parameter to the structure loaded by _save_all */
    bi      _call_handler
    bi      _e_restore_all_and_return
.else
    calli   instruction_bus_error_handler
    rcsr    r7, DEBA
    addi    r7, r7, 64
    b       r7
    nop
    nop
    nop
    nop
.endif

        .size     _instruction_bus_error_handler, .-_instruction_bus_error_handler
        .global _watchpoint_handler
        .type _watchpoint_handler, @function
_watchpoint_handler:
.ifdef _GDB
    sw      (sp+0), ra    /* Calli is going to clobber ra, so store it before executing the calli */
    calli   _save_all     /* Save machine state (all the registers) */
    mvi     r1, 3         /* Set the Exception ID value */
    sw      (sp+136), r1
    sw      (sp+132), ba  /* Set the PC location */
    addi    r1, sp, 4        /* Set the first parameter to the structure loaded by _save_all */
    bi      _call_handler
    bi      _e_restore_all_and_return
.else
    calli   watchpoint_handler
    rcsr    r7, DEBA
    addi    r7, r7, 96
    b       r7
    nop
    nop
    nop
    nop
.endif

        .size     _watchpoint_handler, .-_watchpoint_handler
        .global _data_bus_error_handler
        .type _data_bus_error_handler, @function
_data_bus_error_handler:
.ifdef _GDB
    sw      (sp+0), ra    /* Calli is going to clobber ra, so store it before executing the calli */
    calli   _save_all     /* Save machine state (all the registers) */
    mvi     r1, 4         /* Set the Exception ID value */
    sw      (sp+136), r1
    sw      (sp+132), ea  /* Set the PC location */
    addi    r1, sp, 4        /* Set the first parameter to the structure loaded by _save_all */
    bi      _call_handler
    bi      _e_restore_all_and_return
.else
    calli   data_bus_error_handler
    rcsr    r7, DEBA
    addi    r7, r7, 128
    b       r7
    nop
    nop
    nop
    nop
.endif

        .size     _data_bus_error_handler, .-_data_bus_error_handler
        .global _divide_by_zero_handler
        .type _divide_by_zero_handler, @function
_divide_by_zero_handler:
.ifdef _GDB
    sw      (sp+0), ra    /* Calli is going to clobber ra, so store it before executing the calli */
    calli   _save_all     /* Save machine state (all the registers) */
    mvi     r1, 5         /* Set the Exception ID value */
    sw      (sp+136), r1
    sw      (sp+132), ea  /* Set the PC location */
    addi    r1, sp, 4        /* Set the first parameter to the structure loaded by _save_all */
    bi      _call_handler
    bi      _e_restore_all_and_return
.else
    calli   divide_by_zero_handler
    rcsr    r7, DEBA
    addi    r7, r7, 160
    b       r7
    nop
    nop
    nop
    nop
.endif

        .size     _divide_by_zero_handler, .-_divide_by_zero_handler
        .global _interrupt_handler
        .type _interrupt_handler, @function
_interrupt_handler:
.ifdef _GDB
    sw      (sp+0), ra    /* Calli is going to clobber ra, so store it before executing the calli */
    calli   _save_all     /* Save machine state (all the registers) */
    mvi     r1, 6         /* Set the Exception ID value */
    sw      (sp+136), r1
    sw      (sp+132), ea  /* Set the PC location */
    addi    r1, sp, 4        /* Set the first parameter to the structure loaded by _save_all */
    bi      _call_handler
    bi      _e_restore_all_and_return
.else
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
.endif

        .size     _interrupt_handler, .-_interrupt_handler
        .global _system_call_handler
        .type _system_call_handler, @function
_system_call_handler:
.ifdef _GDB
    sw      (sp+0), ra    /* Calli is going to clobber ra, so store it before executing the calli */
    calli   _save_all     /* Save machine state (all the registers) */
    mvi     r1, 7         /* Set the Exception ID value */
    sw      (sp+136), r1
    sw      (sp+132), ea  /* Set the PC location */
    addi    r1, sp, 4        /* Set the first parameter to the structure loaded by _save_all */
    bi      _call_handler
    bi      _e_restore_all_and_return
.else
    rcsr    r7, DEBA
    addi    r7, r7, 224
    b       r7
    nop
    nop
    nop
    nop
    nop
.endif

        .size     _system_call_handler, .-_system_call_handler
        .global _crt0
        .type _crt0, @function
_crt0:
    /* Clear r0 */
    xor     r0, r0, r0
    /* Setup stack and global pointer */
    mvhi    sp, hi(_fstack)
    ori     sp, sp, lo(_fstack)
    mvhi    gp, hi(_gp)
    ori     gp, gp, lo(_gp)
    /* Clear BSS */
    mvhi    r1, hi(_fbss)
    ori     r1, r1, lo(_fbss)
    mvhi    r3, hi(_ebss)
    ori     r3, r3, lo(_ebss)
        .size     _crt0, .-_crt0
        .global .ClearBSS
        .type .ClearBSS, @function
.ClearBSS:
    be      r1, r3, .CallConstructor
    sw      (r1+0), r0
    addi    r1, r1, 4
    bi      .ClearBSS
        .size     .ClearBSS, .-.ClearBSS
        .global .CallConstructor
        .type .CallConstructor, @function
.CallConstructor:
    /* Call C++ constructors */
    calli   _init
    /* Call main program */
    mvi     r1, 0
    mvi     r2, 0
    mvi     r3, 0
    calli   main
    /* Call exit, which doesn't return, to perform any clean up */
    /* In all CCSW projects, main() doesn't return. */
    /*    Don't bother linking in _fini() and exit(). */
    /* calli   _fini */
    /* calli   exit */


/* Save all registers onto the stack */
        .size     .CallConstructor, .-.CallConstructor
        .global _save_all
        .type _save_all, @function
_save_all:
	/* By convention we move the top of stack before writing to it */
    addi    sp, sp,   -160
	/* By convention callees can write to top of stack  */
	/* so we start at +4 with our structure load        */
    sw      (sp+4),   r0
    sw      (sp+8),   r1
    sw      (sp+12),  r2
    sw      (sp+16),  r3
    sw      (sp+20),  r4
    sw      (sp+24),  r5
    sw      (sp+28),  r6
    sw      (sp+32),  r7
    sw      (sp+36),  r8
    sw      (sp+40),  r9
    sw      (sp+44),  r10
    sw      (sp+48),  r11
    sw      (sp+52),  r12
    sw      (sp+56),  r13
    sw      (sp+60),  r14
    sw      (sp+64),  r15
    sw      (sp+68),  r16
    sw      (sp+72),  r17
    sw      (sp+76),  r18
    sw      (sp+80),  r19
    sw      (sp+84),  r20
    sw      (sp+88),  r21
    sw      (sp+92),  r22
    sw      (sp+96),  r23
    sw      (sp+100), r24
    sw      (sp+104), r25
    sw      (sp+108), r26  /* GP */
    sw      (sp+112), r27  /* FP */
    /* the saved sp value needs to be reduced by the amount we added at the beginning of this sub */
    mv      r1, sp
    addi    r1, r1, 160
    sw      (sp+116), r1   /* SP or r28 */
    /* ra was put on the top of the stack just before this call */
    lw      r1, (sp+160)
    sw      (sp+120), r1   /* RA or r29 */
    sw      (sp+124), ea   /* r30 */
    sw      (sp+128), ba   /* r31 */
    /* The next two locations are "synthetic" -- they are not real registers in the LM32 */
    /* but loaded for the benefit of the GDB stub.  They are named PC and EID.           */
    /* Location 132 is the PC and needs special handling and is best done in the caller  */
    /* Location 136 is the exception number that was fired and must be set by caller     */
    rcsr    r1, EBA
    sw      (sp+140), r1
    rcsr    r1, DEBA
    sw	    (sp+144), r1
    rcsr    r1, IE
    sw      (sp+148), r1
    rcsr    r1, IM
    sw      (sp+152), r1
    rcsr    r1, IP
    sw      (sp+156), r1
    ret
        .size     _save_all, .-_save_all
        .global _e_restore_all_and_return
        .type _e_restore_all_and_return, @function
/* Restore all registers and return from exception */
_e_restore_all_and_return:
/*  lw      r0, (sp+4)   We don't touch r0.  The compiler creates code that requires it to be 0 */
/*  lw      r1, (sp+8)   restore r1 later so we can use it for scratch now  */
    lw      r2, (sp+12)
    lw      r3, (sp+16)
    lw      r4, (sp+20)
    lw      r5, (sp+24)
    lw      r6, (sp+28)
    lw      r7, (sp+32)
    lw      r8, (sp+36)
    lw      r9, (sp+40)
    lw      r10, (sp+44)
    lw      r11, (sp+48)
    lw      r12, (sp+52)
    lw      r13, (sp+56)
    lw      r14, (sp+60)
    lw      r15, (sp+64)
    lw      r16, (sp+68)
    lw      r17, (sp+72)
    lw      r18, (sp+76)
    lw      r19, (sp+80)
    lw      r20, (sp+84)
    lw      r21, (sp+88)
    lw      r22, (sp+92)
    lw      r23, (sp+96)
    lw      r24, (sp+100)
    lw      r25, (sp+104)
    lw      r26, (sp+108)
    lw      r27, (sp+112)
    /* Don't retore SP here! The following code is SP-relative and will break... */
    lw      ra, (sp+120)
    lw      ea, (sp+124)
    lw      ba, (sp+128)
    /* +132 and +136 are synthetic so they don't need to be restored */
    /* CSR's can't be loaded directly from memory; they have to be first loaded into a gpr */
    lw      r1, (sp+140)
    wcsr    EBA, r1
    lw      r1, (sp+144)
    wcsr    DEBA, r1
    lw      r1, (sp+148)
    wcsr    IE, r1
    lw      r1, (sp+152)
    wcsr    IM, r1
    /*  +156 (IP) is read-only and so it cannot be restored */

    /*
     * Manage eret vs. bret

		load R1 with EID
		if (EID is breakpoint)
		    branch to bret
	        if (EID is not watchpoint)
			    branch to eret
			else
			    fall thru to bret
    */

    lw      r1, (sp+136)
    xori    r1, r1, 1
    be      r1, r0, _debug_return     /* 1 == breakpoint exception */
    lw      r1, (sp+136)
    xori    r1, r1, 3                 /* 3 == watchpoint exception */
    bne     r1, r0, _non_debug_return /* if watchpoint exception, fall through */

_debug_return:
    lw      r1, (sp+8)   /* now we are done with r1 we will set it back */
    lw      sp, (sp+116) /* SP has to be restored very last because all of the code is SP relative */
    bret

_non_debug_return:
    lw      r1, (sp+8)   /* now we are done with r1 we will set it back */
    lw      sp, (sp+116) /* SP has to be restored very last because all of the code is SP relative */
    eret
        .size     _e_restore_all_and_return, .-_e_restore_all_and_return

.ifdef _GDB
        .global _call_handler
        .type _call_handler @function
_call_handler:
    mvhi    r2,hi(_handle_exception)
    ori     r2,r2,lo(_handle_exception)
    call    r2
    bi      _e_restore_all_and_return
.endif
