/*==========================================================================*/
/*      Copyright (C) 2009-2013 Texas Instruments Incorporated.             */
/*                      All Rights Reserved                                 */
/*==========================================================================*/

/**
 *  @file       bam_rBrief_exec_funcs.c
 *
 *  @brief      This file defines interfaces for integrating rBrief kernel into BAM
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vcop.h>

#include "bam_rBrief_int.h"
#include "bam_rBrief.h"
#include "vcop_compute_rBrief_kernel.h"
#include "edma_utils_memcpy.h"

/* CHECK_MISRA("-8.7")  -> Disable rule 8.7  */
/*Objects shall be defined at block scope if they are only accessed from within a single function.*/
/* Variables which need longer life need to be declared as static. At the same time it is good to have all static variables at one place to increase readability and maintenance of code.
Given above reasoning, if one has 2 functions in a file and one of them need some data to be static -
he/she will keep it at top of file. As result of this the static data is also visible to another function though the function does not need to see it.
So the rule is voilated. OMAP software cosniders such voilations as waivers and don't plan to fix */
static const uint8_t BAM_RBRIEF_ZERO_TABLE[2U * PATCH_SIZE_P5 * RBRIEF_NUM_ZERO_ROWS] = {0};
/* RESET_MISRA("8.7")  -> Reset rule 8.7  */

/* Function Prototypes */
static BAM_Status BAM_rBrief_initFrame(void *kernelContext);

static BAM_Status BAM_rBrief_initFrame(void *kernelContext)
{
    BAM_rBrief_Context *context = (BAM_rBrief_Context *) kernelContext;
    BAM_Status status = 0;

    RBRIEF_INTERNAL_MEMORY_WBUF           * internalWbuf =
                    (RBRIEF_INTERNAL_MEMORY_WBUF *)context->pInternalBlock[RBRIEF_INTERNAL_WBUF_IDX];
    RBRIEF_INTERNAL_MEMORY_WBUF_tables    * internalWbufTables =
                    (RBRIEF_INTERNAL_MEMORY_WBUF_tables*)context->pInternalBlock[RBRIEF_INTERNAL_WBUF_TABLES_IDX];
    RBRIEF_INTERNAL_MEMORY_IBUFLA         * internalIbufla =
                    (RBRIEF_INTERNAL_MEMORY_IBUFLA *)context->pInternalBlock[RBRIEF_INTERNAL_IBUFLA_IDX];
    RBRIEF_INTERNAL_MEMORY_IBUFLA_tables  * internalIbuflaTables =
                    (RBRIEF_INTERNAL_MEMORY_IBUFLA_tables *)context->pInternalBlock[RBRIEF_INTERNAL_IBUFLA_TABLES_IDX];
    RBRIEF_INTERNAL_MEMORY_IBUFHA         * internalIbufha =
                    (RBRIEF_INTERNAL_MEMORY_IBUFHA *) context->pInternalBlock[RBRIEF_INTERNAL_IBUFHA_IDX];
    RBRIEF_INTERNAL_MEMORY_IBUFHA_tables  * internalIbufhaTables =
                    (RBRIEF_INTERNAL_MEMORY_IBUFHA_tables *) context->pInternalBlock[RBRIEF_INTERNAL_IBUFHA_TABLES_IDX];

    EDMA_UTILS_memcpy2D((void *)internalWbufTables->moments_col_mask,
                       (void *)BAM_RBRIEF_TI_MOMENTS_COL_MASK,
                       sizeof(BAM_RBRIEF_TI_MOMENTS_COL_MASK),
                       1U,
                       0,
                       0);

    EDMA_UTILS_memcpy2D((void *)internalWbufTables->moments_row_mask,
                       (void *)BAM_RBRIEF_TI_MOMENTS_ROW_MASK,
                       sizeof(BAM_RBRIEF_TI_MOMENTS_ROW_MASK),
                       1U,
                       0,
                       0);

    EDMA_UTILS_memcpy2D((void *)internalWbufTables->sinQ14_2,
                       (void *)BAM_RBRIEF_TI_SINQ14_2,
                       sizeof(BAM_RBRIEF_TI_SINQ14_2),
                       1U,
                       0,
                       0);

    EDMA_UTILS_memcpy2D((void *)internalWbufTables->tran_offset,
                       (void *)BAM_RBRIEF_TI_TRAN_OFFSET,
                       sizeof(BAM_RBRIEF_TI_TRAN_OFFSET),
                       1U,
                       0,
                       0);

    EDMA_UTILS_memcpy2D((void *)internalIbuflaTables->cosQ14_2,
                       (void *)BAM_RBRIEF_TI_COSQ14_2,
                       sizeof(BAM_RBRIEF_TI_COSQ14_2),
                       1U,
                       0,
                       0);

    EDMA_UTILS_memcpy2D((void *)internalIbufhaTables->arctan_xthr,
                       (void *)BAM_RBRIEF_TI_ARCTAN_XTHR,
                       sizeof(BAM_RBRIEF_TI_ARCTAN_XTHR),
                       1U,
                       0,
                       0);


    EDMA_UTILS_memcpy2D((void *)internalIbufhaTables->zero_src_dst_x,
                       (const void *)context->kernelArgs.orbPattern,
                       512U,
                       1U,
                       0,
                       0);

    EDMA_UTILS_memcpy2D((void *)internalIbufhaTables->zero_src_dst_y,
                       (const void *)&((*context->kernelArgs.orbPattern)[512U]),
                       512U,
                       1U,
                       0,
                       0);

    /* Make First 5 rows zero : Needed by kernel */
    EDMA_UTILS_memcpy2D(context->pInBlock[RBRIEF_IN_IMG_IDX],
                        (void *)BAM_RBRIEF_ZERO_TABLE,
                        PATCH_SIZE * RBRIEF_NUM_ZERO_ROWS,
                        1U,
                        0,
                        0);

    /* Make first 5 rows of col sum array to zero : Needed by kernel */
    EDMA_UTILS_memcpy2D(internalIbufha->col_sum,
                        (void *)BAM_RBRIEF_ZERO_TABLE,
                        PATCH_SIZE_P5 * sizeof(int16_t) * RBRIEF_NUM_ZERO_ROWS,
                        1U,
                        0,
                        0);


/* Only do ping pong copy and vcop_init call for non host build */
#if (!VCOP_HOST_EMULATION)

    /* Switch to full view for DMA transfer in PONG buffer */
      VCOP_SET_MEM_VIEW((uint32_t)FLAT_MEM_VIEW);

    /* Copy all tables which are in IBUFLA to IBUFHA  */

    /* CHECK_MISRA("-17.4")  -> Disable rule 17.4  */
    /* MISRA.PTR.ARITH :   Array indexing shall be the only allowed form of pointer arithmetic */
    /* This is acceptable because of the hardware supported alias memory view and hence the same buffer is available at two different memory location   */
    EDMA_UTILS_memcpy2D((void *)((uint8_t *)internalIbuflaTables->cosQ14_2 + RBRIEF_PING_PING_BUFFER_OFFSET),
                       (void *)BAM_RBRIEF_TI_COSQ14_2,
                       sizeof(BAM_RBRIEF_TI_COSQ14_2),
                       1U,
                       0,
                       0);

    EDMA_UTILS_memcpy2D((void *)((uint8_t *)internalIbufhaTables->arctan_xthr + RBRIEF_PING_PING_BUFFER_OFFSET),
                       (void *)BAM_RBRIEF_TI_ARCTAN_XTHR,
                       sizeof(BAM_RBRIEF_TI_ARCTAN_XTHR),
                       1U,
                       0,
                       0);


    EDMA_UTILS_memcpy2D((void *)((uint8_t *)internalIbufhaTables->zero_src_dst_x + RBRIEF_PING_PING_BUFFER_OFFSET),
                       (const void *)context->kernelArgs.orbPattern,
                       512U,
                       1U,
                       0,
                       0);

    EDMA_UTILS_memcpy2D((void *)((uint8_t *)internalIbufhaTables->zero_src_dst_y + RBRIEF_PING_PING_BUFFER_OFFSET),
                       (const void *)&((*context->kernelArgs.orbPattern)[512U]),
                       512U,
                       1U,
                       0,
                       0);

    /* Make First 5 rows zero : Needed by kernel */
    EDMA_UTILS_memcpy2D(((uint8_t *)context->pInBlock[RBRIEF_IN_IMG_IDX] + RBRIEF_PING_PING_BUFFER_OFFSET),
                        (void *)BAM_RBRIEF_ZERO_TABLE,
                        PATCH_SIZE * RBRIEF_NUM_ZERO_ROWS,
                        1U,
                        0,
                        0);
    /* Make first two rows of col sum array to zero : Needed by kernel */
    EDMA_UTILS_memcpy2D((void *)((uint8_t *)internalIbufha->col_sum + RBRIEF_PING_PING_BUFFER_OFFSET),
                        (void *)BAM_RBRIEF_ZERO_TABLE,
                        PATCH_SIZE_P5 * sizeof(int16_t) * RBRIEF_NUM_ZERO_ROWS,
                        1U,
                        0,
                        0);
    /* RESET_MISRA("17.4")  -> Reset rule 17.4     */
    /* Switch back to alias view */
    VCOP_SET_MEM_VIEW((uint32_t)ALIAS_128K_VIEW);

    vcop_compute_rBrief_init((char*)internalWbufTables->moments_col_mask,
            (int16_t *)internalIbufha->moments_col_sum,
            (char *)internalWbufTables->moments_row_mask,
            (int16_t *)internalIbufla->moments_row_sum,
            (int16_t *)internalIbufla->moments_m10,
            (int16_t *)internalWbuf->moments_m01,
            (uint16_t *)internalIbufhaTables->arctan_xthr,
            (uint8_t *)internalIbufha->arctan_pack_decision,
            (int16_t *)internalIbuflaTables->cosQ14_2,
            (int16_t *)internalWbufTables->sinQ14_2,
            (uint16_t *)internalWbufTables->tran_offset,
            (int16_t *)internalWbuf->cos_pt,
            (int16_t *)internalIbufla->sin_pt,
            /* input block */
            (uint8_t *)context->pInBlock[RBRIEF_IN_IMG_IDX],

            (int16_t *)internalIbufha->col_sum,
            (int16_t *)internalWbuf->row_col_sum,
            (char *)internalIbufhaTables->zero_src_dst_x,
            (char *)internalIbufhaTables->zero_src_dst_y,
            (char *)internalIbufla->rot_src_dst_x,
            (char *)internalWbuf->rot_src_dst_y,
            (uint16_t *)internalIbufla->rot_src_lin,
            (uint16_t *)internalIbufha->rot_dst_lin,
            (int16_t *)internalIbufha->tlu_src,
            (int16_t *)internalIbufla->tlu_dst,
            /*Output descriptor */
            (uint8_t *)context->pOutBlock[RBRIEF_OUT_BRIEF_DESCRIPTOR_IDX],
            /* Param Block */
            (uint16_t *)context->pInternalBlock[RBRIEF_INTERNAL_PARAMS_IDX]);
#endif


    return status;
}

#if (VCOP_HOST_EMULATION)
static BAM_Status BAM_rBrief_compute(void *kernelContext)
{
    BAM_rBrief_Context *context = (BAM_rBrief_Context *) kernelContext;
    BAM_Status status = 0;
    RBRIEF_INTERNAL_MEMORY_WBUF           * internalWbuf =
                    (RBRIEF_INTERNAL_MEMORY_WBUF *)context->pInternalBlock[RBRIEF_INTERNAL_WBUF_IDX];
    RBRIEF_INTERNAL_MEMORY_WBUF_tables    * internalWbufTables =
                    (RBRIEF_INTERNAL_MEMORY_WBUF_tables*)context->pInternalBlock[RBRIEF_INTERNAL_WBUF_TABLES_IDX];
    RBRIEF_INTERNAL_MEMORY_IBUFLA         * internalIbufla =
                    (RBRIEF_INTERNAL_MEMORY_IBUFLA *)context->pInternalBlock[RBRIEF_INTERNAL_IBUFLA_IDX];
    RBRIEF_INTERNAL_MEMORY_IBUFLA_tables  * internalIbuflaTables =
                    (RBRIEF_INTERNAL_MEMORY_IBUFLA_tables *)context->pInternalBlock[RBRIEF_INTERNAL_IBUFLA_TABLES_IDX];
    RBRIEF_INTERNAL_MEMORY_IBUFHA         * internalIbufha =
                    (RBRIEF_INTERNAL_MEMORY_IBUFHA *) context->pInternalBlock[RBRIEF_INTERNAL_IBUFHA_IDX];
    RBRIEF_INTERNAL_MEMORY_IBUFHA_tables  * internalIbufhaTables =
                    (RBRIEF_INTERNAL_MEMORY_IBUFHA_tables *) context->pInternalBlock[RBRIEF_INTERNAL_IBUFHA_TABLES_IDX];

    vcop_compute_rBrief((char*)internalWbufTables->moments_col_mask,
                (int16_t *)internalIbufha->moments_col_sum,
                (char *)internalWbufTables->moments_row_mask,
                (int16_t *)internalIbufla->moments_row_sum,
                (int16_t *)internalIbufla->moments_m10,
                (int16_t *)internalWbuf->moments_m01,
                (uint16_t *)internalIbufhaTables->arctan_xthr,
                (uint8_t *)internalIbufha->arctan_pack_decision,
                (int16_t *)internalIbuflaTables->cosQ14_2,
                (int16_t *)internalWbufTables->sinQ14_2,
                (uint16_t *)internalWbufTables->tran_offset,
                (int16_t *)internalWbuf->cos_pt,
                (int16_t *)internalIbufla->sin_pt,
                /* input block */
                (uint8_t *)context->pInBlock[RBRIEF_IN_IMG_IDX],

                (int16_t *)internalIbufha->col_sum,
                (int16_t *)internalWbuf->row_col_sum,
                (char *)internalIbufhaTables->zero_src_dst_x,
                (char *)internalIbufhaTables->zero_src_dst_y,
                (char *)internalIbufla->rot_src_dst_x,
                (char *)internalWbuf->rot_src_dst_y,
                (uint16_t *)internalIbufla->rot_src_lin,
                (uint16_t *)internalIbufha->rot_dst_lin,
                (int16_t *)internalIbufha->tlu_src,
                (int16_t *)internalIbufla->tlu_dst,
                /*Output descriptor */
                (uint8_t *)context->pOutBlock[RBRIEF_OUT_BRIEF_DESCRIPTOR_IDX]);
    return status;
}
#endif


BAM_KernelExecFuncDef gBAM_TI_rBriefExecFunc =
#if (!VCOP_HOST_EMULATION)
{
    NULL,
    &BAM_rBrief_initFrame,
    NULL,
    NULL,
    NULL,
    &vcop_compute_rBrief_vloops };
#else
{
    NULL,
    &BAM_rBrief_initFrame,
    &BAM_rBrief_compute,
    NULL,
    NULL,
    NULL};
#endif

