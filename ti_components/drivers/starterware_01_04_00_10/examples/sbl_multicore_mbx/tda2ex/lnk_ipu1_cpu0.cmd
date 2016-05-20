/**
 *  \file     lnk_ipu1_cpu0.cmd
 *
 *  \brief    This file contains the System Memory Map used by IPU1 CPU0 binary
 *            for multicore mailbox app image.
 *
 *  \copyright Copyright (C) 2015 Texas Instruments Incorporated -
 *             http://www.ti.com/
 */

/* ======================================================================
 *   Copyright (C) 2015 Texas Instruments Incorporated
 *
 *   All rights reserved. Property of Texas Instruments Incorporated.
 *   Restricted rights to use, duplicate or disclose this code are
 *   granted through contract.
 *
 *   The program may not be used without the written permission
 *   of Texas Instruments Incorporated or against the terms and conditions
 *   stipulated in the agreement under which this program has been
 *   supplied.
 * ==================================================================== */

-stack  0x00500000                            /* Software Stack Size */
-heap   0x00500000                            /* Heap Area Size */

/* Specify the System Memory Map */
MEMORY
{
    /* Code Section - 20MB */
    /* Data Section - 10MB */
    /* Stack - 5MB */
    /* Heap - 5MB */
    IRAM_MEM:     org = 0x00000000           len = 0x1000
    MMU_TLB :     org = 0x4031C000,          len = 0x000004000

    DDR3_MPU_CPU0_CODE  : org = 0x83200000,  len = 0x01400000
    DDR3_MPU_CPU0_DATA  : org = 0x84600000,  len = 0x00A00000
    DDR3_MPU_CPU0_STACK : org = 0x85000000,  len = 0x00500000
    DDR3_MPU_CPU0_HEAP  : org = 0x85500000,  len = 0x00500000

    DDR3_IPU1_CPU0_CODE  : org = 0x86400000, len = 0x01400000
    DDR3_IPU1_CPU0_DATA  : org = 0x87800000, len = 0x00A00000
    DDR3_IPU1_CPU0_STACK : org = 0x88200000, len = 0x00500000
    DDR3_IPU1_CPU0_HEAP  : org = 0x88700000, len = 0x00500000

    DDR3_IPU1_CPU1_CODE  : org = 0x89600000, len = 0x01400000
    DDR3_IPU1_CPU1_DATA  : org = 0x8AA00000, len = 0x00A00000
    DDR3_IPU1_CPU1_STACK : org = 0x8B400000, len = 0x00500000
    DDR3_IPU1_CPU1_HEAP  : org = 0x8B900000, len = 0x00500000

    DDR3_DSP1_CODE  : org = 0x8C800000,      len = 0x01400000
    DDR3_DSP1_DATA  : org = 0x8DC00000,      len = 0x00A00000
    DDR3_DSP1_STACK : org = 0x8E600000,      len = 0x00500000
    DDR3_DSP1_HEAP  : org = 0x8EB00000,      len = 0x00500000
}

/* Specify the Section Allocation into Memory */
SECTIONS
{
    .intvecs     : load > IRAM_MEM
    .init        : load > DDR3_IPU1_CPU0_CODE

    .text        : load > DDR3_IPU1_CPU0_CODE /* Code */
    .data        : load > DDR3_IPU1_CPU0_DATA /* Initialized Global and Static variables */
    .bss         : load > DDR3_IPU1_CPU0_DATA /* Uninitialized or Zero initialized */
                                              /* Global and Static Variables       */
                    RUN_START(bss_start)
                    RUN_END(bss_end)
    .const       : load > DDR3_IPU1_CPU0_DATA  /* Global Constants */
    .cinit       : load > DDR3_IPU1_CPU0_DATA
    .stack       : load > DDR3_IPU1_CPU0_STACK /* Software System Stack */
    .plt         : load > DDR3_IPU1_CPU0_CODE
    .sysmem      : load > DDR3_IPU1_CPU0_DATA
    .my_sect_ddr : load > DDR3_IPU1_CPU0_CODE
}

