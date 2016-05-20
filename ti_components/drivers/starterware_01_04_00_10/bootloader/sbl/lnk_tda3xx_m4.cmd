/**
 *  \file     lnk_tda3xx_m4.cmd
 *
 *  \brief    This file contains the System Memory Map used by Secondary
 *            Bootloader (SBL) for QSPI and QSPI_SD boot mode for tda3xx SoC.
 *
 *  \copyright Copyright (C) 2014 Texas Instruments Incorporated -
 *             http://www.ti.com/
 */

/* ======================================================================
 *   Copyright (C) 2014 Texas Instruments Incorporated
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

-stack  0x1000                            /* SOFTWARE STACK SIZE */
-heap   0x1000                            /* HEAP AREA SIZE */

/* SPECIFY THE SYSTEM MEMORY MAP */

MEMORY
{
    IRAM_MEM:     org = 0x00000000 len = 0x4000      /* IRAM */
    OCMCRAM1_0:   org = 0x00300000 len = 0x00000100  /* OCMC RAM mapped
                                                        to 0x40300000 */
    OCMCRAM1_1:   org = 0x00300100 len = 0x00000100  /* OCMC RAM mapped
                                                        to 0x40300100 */
    OCMCRAM1_2:   org = 0x40300200 len = 0x00000100  /* OCMC RAM */
    OCMCRAM1_3:   org = 0x00300300 len = 0x0007FD00  /* OCMC RAM mapped
                                                        to 0x40300300 */
}

/* SPECIFY THE SECTIONS ALLOCATION INTO MEMORY */

SECTIONS
{
    .intvecs     : load > OCMCRAM1_3
    .intc_text   : load > OCMCRAM1_3
    .sbl_init    : load > OCMCRAM1_0
    .ipu1_1_init : load > OCMCRAM1_1
    .init        : load > OCMCRAM1_3

    .text        : load > OCMCRAM1_3      /* CODE */
    .data        : load > OCMCRAM1_3      /* INITIALIZED GLOBAL AND STATIC VARIABLES */
    .bss         : load > OCMCRAM1_3      /* UNINITIALIZED OR ZERO INITIALIZED */
                                          /* GLOBAL & STATIC VARIABLES */
                    RUN_START(bssStart)
                    RUN_END(bssEnd)
    .const       : load > OCMCRAM1_3      /* GLOBAL CONSTANTS */
    .cinit       : load > OCMCRAM1_3
    .stack       : load > OCMCRAM1_3      /* SOFTWARE SYSTEM STACK */
                    RUN_START(stackStart)
                    RUN_END(stackEnd)
    .plt         : load > OCMCRAM1_3
    .sysmem      : load > OCMCRAM1_3
    .img_hdr     : load > OCMCRAM1_2

}

