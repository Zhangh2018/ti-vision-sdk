/*==========================================================================*/
/*      Copyright (C) 2009-2013 Texas Instruments Incorporated.             */
/*                      All Rights Reserved                                 */
/*==========================================================================*/

/**
 *  @file       bam_census_16bits_exec_funcs.c
 *
 *  @brief      This file defines interfaces for integrating census 8 bits kernel
 *              function into BAM.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vcop.h>

#include "bam_census_16bits_int.h"
#include "bam_census_16bits.h"
#include "vcop_census_16bits_kernel.h"
#include "vcop_census_16bits_cn.h"
#include "init_census_16bits_params.h"

/* CHECK_MISRA("-15.1")  -> Disable rule 15.1 */
/* Goto statement is used */
/* Deviation is allowed because we use GOTO several times throughout the code but they are all forward GOTO to a single exit point */

/* Function Prototypes */
static BAM_Status Bam_Census_16bits_initFrame(void *kernelContext);

static BAM_Status Bam_Census_16bits_initFrame(void *kernelContext)
{
    int32_t numWinHiter, numWinWiter;
    BAM_Census_16bits_Context *context = (BAM_Census_16bits_Context *) kernelContext;
    BAM_Status status = 0;
    
    numWinHiter= ((int32_t)context->kernelArgs.winHeight + ((int32_t)context->kernelArgs.winVertStep -1) )/ (int32_t)context->kernelArgs.winVertStep;
    numWinWiter= ((int32_t)context->kernelArgs.winWidth + ((int32_t)context->kernelArgs.winHorzStep -1) )/ (int32_t)context->kernelArgs.winHorzStep;
    
    status= init_census_16bits_params (
            context->kernelArgs.winWidth, /* windWidth * winHeight msut be > 8 */
            context->kernelArgs.winHeight,
            context->kernelArgs.winHorzStep,
            context->kernelArgs.winVertStep,
            context->kernelArgs.in_w,
            context->kernelArgs.in_h,
            context->kernelArgs.out_pitch, /* in bytes, must multiple of 4 but not multiple of 32 bytes*/
            (uint16_t*)context->pInternalBlock[CENSUS_16BITS_OFFSET_IDX], /* Point to an array of 16 bytes. Call init_census_16bits_params() to initialize content pointed by pOffset*/
            32U, /* Should be 32 */
            (uint8_t*)context->pInternalBlock[CENSUS_16BITS_CODEWORDMASK_IDX], /* Point to an array of (winWidth*winHeight+7)/8 bytes. Call init_census_16bits_params() to initialize content pointed by pCodeWordMask */
            ((numWinWiter*numWinHiter) + ((int32_t)VCOP_SIMD_WIDTH - 1))/(int32_t)VCOP_SIMD_WIDTH, /* Should be (winWidth*winHeight+7)/8 */
            (uint8_t*)context->pInternalBlock[CENSUS_16BITS_ROWMASK_IDX],/* Point to an array of (computeHeight+7)/8 bytes. Call init_census_16bits_params() to initialize content pointed by pRowMask */
            ((int32_t)context->kernelArgs.in_h + ((int32_t)VCOP_SIMD_WIDTH - 1))/(int32_t)VCOP_SIMD_WIDTH,
            &context->scratchStride);

    if (status !=0) {
        goto Exit;
    }
        
    if (context->scratchStride != (((context->kernelArgs.in_h + 31U) & (uint16_t)(~31U)) + 4U)) {
        status= 1;
        goto Exit;
    }
            
#if (!VCOP_HOST_EMULATION)
    vcop_census_16bits_init(
            (uint16_t*)context->pInBlock[CENSUS_16BITS_IN_IDX],
            (uint8_t*)context->pOutBlock[CENSUS_16BITS_OUT_IDX],
            (uint8_t*)context->pInternalBlock[CENSUS_16BITS_SCRATCH_BITMASK_IDX],
            (uint16_t*)context->pInternalBlock[CENSUS_16BITS_SCRATCH_IDX],
            (uint8_t*)context->pInternalBlock[CENSUS_16BITS_SCRATCH_IDX],
            (uint16_t*)context->pInternalBlock[CENSUS_16BITS_OFFSET_IDX],
            (uint8_t*)context->pInternalBlock[CENSUS_16BITS_CODEWORDMASK_IDX],
            (uint8_t*)context->pInternalBlock[CENSUS_16BITS_ROWMASK_IDX],
            context->kernelArgs.winWidth, 
            context->kernelArgs.winHeight,
            context->kernelArgs.winHorzStep,
            context->kernelArgs.winVertStep,
            context->kernelArgs.in_w,
            context->kernelArgs.in_h,
            context->kernelArgs.in_pitch,
            context->kernelArgs.out_pitch,
            context->scratchStride,
            context->pInternalBlock[CENSUS_16BITS_PARAMS_IDX]
    );
#endif

Exit:
    return status;
}

#if (VCOP_HOST_EMULATION)
static BAM_Status BAM_Census_16bits_compute(void *kernelContext);

static BAM_Status BAM_Census_16bits_compute(void *kernelContext)
{
    BAM_Census_16bits_Context *context = (BAM_Census_16bits_Context *) kernelContext;
    BAM_Status status = 0;

    vcop_census_16bits(
                (uint16_t*)context->pInBlock[CENSUS_16BITS_IN_IDX],
                (uint8_t*)context->pOutBlock[CENSUS_16BITS_OUT_IDX],
                (uint8_t*)context->pInternalBlock[CENSUS_16BITS_SCRATCH_BITMASK_IDX],
                (uint16_t*)context->pInternalBlock[CENSUS_16BITS_SCRATCH_IDX],
                (uint8_t*)context->pInternalBlock[CENSUS_16BITS_SCRATCH_IDX],
                (uint16_t*)context->pInternalBlock[CENSUS_16BITS_OFFSET_IDX],
                (uint8_t*)context->pInternalBlock[CENSUS_16BITS_CODEWORDMASK_IDX],
                (uint8_t*)context->pInternalBlock[CENSUS_16BITS_ROWMASK_IDX],
                context->kernelArgs.winWidth, 
                context->kernelArgs.winHeight,
                context->kernelArgs.winHorzStep,
                context->kernelArgs.winVertStep,
                context->kernelArgs.in_w,
                context->kernelArgs.in_h,
                context->kernelArgs.in_pitch,
                context->kernelArgs.out_pitch,
                context->scratchStride
        );

    return status;
}
#endif

BAM_KernelExecFuncDef gBAM_TI_census_16bits_ExecFunc =
#if (!VCOP_HOST_EMULATION)
    { NULL, &Bam_Census_16bits_initFrame, NULL, NULL, NULL, &vcop_census_16bits_vloops };
#else
    { NULL, &Bam_Census_16bits_initFrame, &BAM_Census_16bits_compute, NULL, NULL, NULL };
#endif

/*RESET_MISRA("15.1")  -> Reset rule 15.1 for GOTO */

