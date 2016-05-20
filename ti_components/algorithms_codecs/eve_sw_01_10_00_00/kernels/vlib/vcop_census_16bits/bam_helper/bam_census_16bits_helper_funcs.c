/*==========================================================================*/
/*      Copyright (C) 2009-2014 Texas Instruments Incorporated.             */
/*                      All Rights Reserved                                 */
/*==========================================================================*/

/**
 *  @file       bam_census_16bits_exec_funcs.c
 *
 *  @brief      This file defines interfaces for integrating census 8-bits transform kernel
 *              into BAM
 */

#include <stdio.h>
#include <stdlib.h>

#include "evealgframework.h"
#include "bam_census_16bits_int.h"
#include "vcop_census_16bits_kernel.h"
#include "vcop_census_16bits_cn.h"

/* CHECK_MISRA("-15.1")  -> Disable rule 15.1 */
/* Goto statement is used */
/* Deviation is allowed because we use GOTO several times throughout the code but they are all forward GOTO to a single exit point */

/* Function Prototypes */
static BAM_Status BAM_Census_16bits_getMemRecFunc(const void *kernelArgs, BAM_MemRec internalBlock[],
        BAM_MemRec outBlock[], uint8_t *numInternalBlocks, uint8_t *numOutBlocks);

/* CHECK_MISRA("-16.7")  -> Disable rule 16.7 */
/* Pointer parameter kernelArgs is not used to modify the addressed object but is not declared as a pointer to const */
/* Must be a false positive because it is declared as pointer to const void here */
static BAM_Status BAM_Census_16bits_getMemRecFunc(const void *kernelArgs, BAM_MemRec internalBlock[],
        BAM_MemRec outBlock[], uint8_t *numInternalBlocks, uint8_t *numOutBlocks)
/*RESET_MISRA("16.7")  -> Reset rule 16.7 */
{
    int32_t numWinHiter, numWinWiter;
    uint16_t numCensusOrientations;
    uint16_t numBytesPerCensus;
    uint16_t scratchStride;
    BAM_Status status= BAM_S_SUCCESS;

#if (!VCOP_HOST_EMULATION)
    const BAM_Census_16bits_Args *args= kernelArgs;
#else
    const BAM_Census_16bits_Args *args= (const BAM_Census_16bits_Args *)kernelArgs;
#endif

    numCensusOrientations= _CENSUS_WIN_SIZE((uint16_t)args->winWidth, (uint16_t)args->winHeight, (uint16_t)args->winHorzStep, (uint16_t)args->winVertStep);
    numBytesPerCensus= (numCensusOrientations + 7u) / 8u;

    /* Make sure that out_pitch is multiple of 4 but not multiple of 32 */
    if ((args->out_pitch < (numBytesPerCensus*args->in_w)) || (((uint32_t)args->out_pitch & 3U) != 0) || (((uint32_t)args->out_pitch & 31U)==0)) {
        status= BAM_E_FAIL;
        goto Exit;
    }

    numWinHiter= (args->winHeight + (args->winVertStep -1) )/ args->winVertStep;
    numWinWiter= (args->winWidth + (args->winHorzStep -1) )/ args->winHorzStep;

#if (!VCOP_HOST_EMULATION)
    internalBlock[CENSUS_16BITS_PARAMS_IDX].size = 2U*vcop_census_16bits_param_count();
#else
    /* This field could be any non-zero number for host emulation */
    internalBlock[CENSUS_16BITS_PARAMS_IDX].size = 4U;
#endif
    /*
     * (2*NUM_WIN_W_ITER*NUM_WIN_H_ITER + 16)*((computeWidth+15)/16)*computeHeight bytes
     */
    internalBlock[CENSUS_16BITS_SCRATCH_BITMASK_IDX].size = (((2*numWinWiter*numWinHiter) + (2*(int32_t)VCOP_SIMD_WIDTH))*((args->in_w+((2*(int32_t)VCOP_SIMD_WIDTH)-1))/(2*(int32_t)VCOP_SIMD_WIDTH))*args->in_h);
    internalBlock[CENSUS_16BITS_SCRATCH_BITMASK_IDX].attrs.memAttrs    = BAM_MEMATTRS_SCRATCH;
    internalBlock[CENSUS_16BITS_SCRATCH_BITMASK_IDX].space             = BAM_MEMSPACE_IBUFHA;

    /* Size of scratch should be :
     * MAX(computeWidth*scratchStride*(NUM_WIN_W_ITER*NUM_WIN_H_ITER+7)/8, inStride * (computeHeight + winHeight -1 ) + 15) bytes
     */
    scratchStride= ((args->in_h + 31U) & (uint16_t)(~31U)) + 4U; /* This line is taken from init_census_16bits_params(), line #93. It makes sure that the stride is such that there is no bank conflict when using p-scatter */
    internalBlock[CENSUS_16BITS_SCRATCH_IDX].size = (MAX(((int32_t)args->in_w*(int32_t)scratchStride*((numWinWiter*numWinHiter)+ ((int32_t)VCOP_SIMD_WIDTH-1)))/(int32_t)VCOP_SIMD_WIDTH, (2*(int32_t)args->in_pitch * ((int32_t)args->in_h + ((int32_t)args->winHeight -1) )) + ((2*(int32_t)VCOP_SIMD_WIDTH)-1)));
    internalBlock[CENSUS_16BITS_SCRATCH_IDX].attrs.memAttrs    = BAM_MEMATTRS_SCRATCH;
    internalBlock[CENSUS_16BITS_SCRATCH_IDX].space             = BAM_MEMSPACE_WBUF;
    internalBlock[CENSUS_16BITS_SCRATCH_IDX].alignment         = 32;

    /* Size should be 32 bytes */
    internalBlock[CENSUS_16BITS_OFFSET_IDX].size = 32U;
    internalBlock[CENSUS_16BITS_OFFSET_IDX].attrs.memAttrs    = BAM_MEMATTRS_CONST;
    internalBlock[CENSUS_16BITS_OFFSET_IDX].space             = BAM_MEMSPACE_WBUF;

    /*size should be (NUM_WIN_W_ITER*NUM_WIN_H_ITER+7)/8 bytes. */
    internalBlock[CENSUS_16BITS_CODEWORDMASK_IDX].size = ((numWinWiter*numWinHiter)+((int32_t)VCOP_SIMD_WIDTH-1))/(int32_t)VCOP_SIMD_WIDTH;
    internalBlock[CENSUS_16BITS_CODEWORDMASK_IDX].attrs.memAttrs    = BAM_MEMATTRS_CONST;
    internalBlock[CENSUS_16BITS_CODEWORDMASK_IDX].space             = BAM_MEMSPACE_WBUF;

    /*size should be (computeHeight+7)/8 bytes bytes. */
    internalBlock[CENSUS_16BITS_ROWMASK_IDX].size = (args->in_h + ((int32_t)VCOP_SIMD_WIDTH - 1))/(int32_t)VCOP_SIMD_WIDTH;
    internalBlock[CENSUS_16BITS_ROWMASK_IDX].attrs.memAttrs    = BAM_MEMATTRS_CONST;
    internalBlock[CENSUS_16BITS_ROWMASK_IDX].space             = BAM_MEMSPACE_WBUF;

    outBlock[CENSUS_16BITS_OUT_IDX].size = args->out_pitch*args->in_h;

Exit:
    return status;
}


BAM_KernelInfo gBAM_TI_census_16bits_Kernel =
{
    0,                                      /* kernelId */
    sizeof(BAM_Census_16bits_Context),
    sizeof(BAM_Census_16bits_Args),
    BAM_EVE,                                /* coreType */
    BAM_NODE_COMPUTE,                       /* nodeType */
    NUM_IN_BLOCKS,                          /* 1 */
    NUM_OUT_BLOCKS,                         /* 1 */
    NUM_INTERNAL_BLOCKS                     /* 6 */
};

BAM_KernelHelperFuncDef gBAM_TI_census_16bits_HelperFunc =
{ &BAM_Census_16bits_getMemRecFunc, NULL};

/*RESET_MISRA("15.1")  -> Reset rule 15.1 for GOTO */

