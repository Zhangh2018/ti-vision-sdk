/*==========================================================================*/
/*      Copyright (C) 2009-2013 Texas Instruments Incorporated.             */
/*                      All Rights Reserved                                 */
/*==========================================================================*/

/**
 *  @file       bam_median_filter_exec_funcs.c
 *
 *  @brief      This file defines interfaces for integrating fast9 kernel
 *              function into BAM.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vcop.h>

#include "bam_remap_int.h"
#include "bam_remap.h"

#include "remap.h"
#include "remap_common.h"

#include "vcop_deInterleaveYUV422IBE.h"
#include "vcop_deInterleaveYUV422ILE.h"
#include "vcop_bilinearInterpolate8b.h"
#include "vcop_bilinearInterpolate16b.h"
#include "vcop_bilinearInterpolateYUV422Ichroma.h"
#include "vcop_bilinearInterpolateYUV420SPchroma.h"
#include "vcop_bilinearInterpolateYUV422Iluma.h"
#include "vcop_dsTLUindexAndFrac.h"
#include "vcop_chromaTLUindexCalc.h"
#include "vcop_nnInterpolate8b.h"
#include "vcop_nnInterpolate420SPchroma.h"
#include "vcop_nnInterpolate422Ichroma.h"
#include "vcop_memcpy.h"
#include "vcop_bilinearInterpolateLumaPblockUpdate.h"
#include "vcop_chromaTLUindexCalcPblockUpdate.h"
#include "vcop_nnInterpolate420SPchromaPblockUpdate.h"
#include "vcop_bilinearInterpolateYUV420SPchromaPblockUpdate.h"
#include "vcop_dsTLUindexAndFracPblockUpdate.h"
#include "vcop_nnInterpolate422IchromaPblockUpdate.h"
#include "vcop_bilinearInterpolateYUV422IchromaPblockUpdate.h"


/* Function Prototypes */
static BAM_Status Bam_Remap_initFrame(void *kernelContext);
static BAM_Status Bam_Remap_compute(void *kernelContext);


static BAM_Status Bam_Remap_initFrame(void *kernelContext)
{
    BAM_Remap_Context  *context     = (BAM_Remap_Context *) kernelContext;
    VCOP_Remap_Context *vcopContext = &(context->vcopContext);
    BAM_Remap_Args     *params      = &(context->kernelArgs);
    Format   srcFormat;
    BAM_Status status = 0;
    uint32_t enableTileApproach;

    if(params->maps.maxInputBlockSize != 0U)
    {
        enableTileApproach = 0U;
    }
    else
    {
        enableTileApproach = 1U;
    }

    srcFormat       = params->maps.srcFormat;
    vcopContext->pblockMem  = (uint16_t *)context->pInternalBlock[REMAP_PBLOCK_PTR_IDX];


    if ((srcFormat == U8BIT) || (srcFormat == S8BIT))
    {
        vcopContext->numPblocks = (uint16_t)1;

#if ((!VCOP_HOST_EMULATION) && (!NATC))
        vcopContext->pblockSize = (vcop_bilinearInterpolate8b_param_count() * sizeof(__pblock_vcop_bilinearInterpolate8b[0]))
                                + (vcop_bilinearInterpolateLumaPblockUpdate_param_count() * sizeof(__pblock_vcop_bilinearInterpolateLumaPblockUpdate[0]));
#endif
    }
    else if ((srcFormat == U16BIT) || (srcFormat == S16BIT))
    {
        vcopContext->numPblocks = (uint16_t)1;

#if (!VCOP_HOST_EMULATION && !NATC)
        vcopContext->pblockSize = (vcop_bilinearInterpolate16b_param_count() * sizeof(__pblock_vcop_bilinearInterpolate16b[0]))
                                + (vcop_bilinearInterpolateLumaPblockUpdate_param_count() * sizeof(__pblock_vcop_bilinearInterpolateLumaPblockUpdate[0]));
#endif

    }
    else if ((srcFormat == YUV_422ILE) || (srcFormat == YUV_422IBE))
    {
        vcopContext->numPblocks = (uint16_t)5;

#if (!VCOP_HOST_EMULATION && !NATC)
        vcopContext->pblockSize = (vcop_deInterleaveYUV422ILE_param_count() * sizeof(__pblock_vcop_deInterleaveYUV422ILE[0]))
                                + (vcop_bilinearInterpolateYUV422Iluma_param_count() * sizeof(__pblock_vcop_bilinearInterpolateYUV422Iluma[0]))
                                + (vcop_dsTLUindexAndFracBilInterpolate_param_count() * sizeof(__pblock_vcop_dsTLUindexAndFracBilInterpolate[0]))
                                + (vcop_bilinearInterpolateYUV422Ichroma_param_count() * sizeof(__pblock_vcop_bilinearInterpolateYUV422Ichroma[0]))
                                + (vcop_bilinearInterpolateYUV422Ichroma_param_count() * sizeof(__pblock_vcop_bilinearInterpolateYUV422Ichroma[0]))
                                + (vcop_bilinearInterpolateLumaPblockUpdate_param_count() * sizeof(__pblock_vcop_bilinearInterpolateLumaPblockUpdate[0]))
                                + (vcop_dsTLUindexAndFracBilInterpolatePblockUpdate_param_count() * sizeof(__pblock_vcop_dsTLUindexAndFracBilInterpolatePblockUpdate[0]))
                                + (vcop_bilinearInterpolateYUV422IchromaUPblockUpdate_param_count() * sizeof(__pblock_vcop_bilinearInterpolateYUV422IchromaUPblockUpdate[0]))
                                + (vcop_bilinearInterpolateYUV422IchromaVPblockUpdate_param_count() * sizeof(__pblock_vcop_bilinearInterpolateYUV422IchromaVPblockUpdate[0]));
#endif
    }
    /* CHECK_MISRA("-14.1")  -> Disable rule 14.1  */
    /* Currently srcFormat will not be grater than YUV_420SP, hence code is Always true  */
    /* as the implementation of the function changes, it will can be false.   */
    /* INVARIANT_CONDITION.GEN */
    else if (srcFormat == YUV_420SP)
    {
        vcopContext->numPblocks = (uint16_t)3;

#if (!VCOP_HOST_EMULATION && !NATC)
        vcopContext->pblockSize = (vcop_bilinearInterpolate8b_param_count() * sizeof(__pblock_vcop_bilinearInterpolate8b[0]))
                                + (vcop_chromaTLUIndexCalcBilInterpolate_param_count() * sizeof(__pblock_vcop_chromaTLUIndexCalcBilInterpolate[0]))
                                + (vcop_bilinearInterpolateYUV420SPchroma_param_count() * sizeof(__pblock_vcop_bilinearInterpolateYUV420SPchroma[0]))
                                + (vcop_bilinearInterpolateLumaPblockUpdate_param_count() * sizeof(__pblock_vcop_bilinearInterpolateLumaPblockUpdate[0]))
                                + (vcop_chromaTLUIndexCalcBilInterpolatePblockUpdate_param_count() * sizeof(__pblock_vcop_chromaTLUIndexCalcBilInterpolatePblockUpdate[0]))
                                + (vcop_bilinearInterpolateYUV420SPchromaPblockUpdate_param_count() * sizeof(__pblock_vcop_bilinearInterpolateYUV420SPchromaPblockUpdate[0]));
#endif
    }
    else
    {
        /* The else is added to avoid MISRA RULE 14.10 MISRA.IF.NO_ELSE. */
    }
  /* RESET_MISRA("14.1")  -> Reset rule 14.1      */

#if (!VCOP_HOST_EMULATION && !NATC)
    if(enableTileApproach)
    {
        status= vcop_remap_tile_approach_init((uint8_t *)context->pInBlock[REMAP_LUT_PTR_IDX],
                        (uint8_t *)context->pInBlock[REMAP_INPUT_IMAGE_BLOCK_PTR_IDX],
                        (uint8_t *)context->pInternalBlock[REMAP_INTERPOLATION_BUF0_PTR_IDX],
                        (uint8_t *)context->pInternalBlock[REMAP_INTERPOLATION_BUF1_PTR_IDX],
                        (uint8_t *)context->pInternalBlock[REMAP_CHROMA_TLU_INDEX_PTR_IDX],
                        (uint8_t *)context->pInternalBlock[REMAP_SCATTERED_DEST_OFFSET_VALUES_PTR_IDX],
                        (uint8_t *)context->pInternalBlock[REMAP_DEINTERLEAVED_CHROMA_U_V_PTR_IDX],
                        (uint8_t *)context->pOutBlock[REMAP_OUT_LUMA_IDX],
                        (const BAM_Remap_Args *)&context->kernelArgs,
                         vcopContext);

        context->vloopFunc = &vcop_remap_tile_approach;
    } else
    {
        status= vcop_remap_bb_approach_init((uint8_t *)context->pInBlock[REMAP_LUT_PTR_IDX],
                        (uint8_t *)context->pInBlock[REMAP_INPUT_IMAGE_BLOCK_PTR_IDX],
                        (uint8_t *)context->pInternalBlock[REMAP_INTERPOLATION_BUF0_PTR_IDX],
                        (uint8_t *)context->pInternalBlock[REMAP_INTERPOLATION_BUF1_PTR_IDX],
                        (uint8_t *)context->pInternalBlock[REMAP_CHROMA_TLU_INDEX_PTR_IDX],
                        (uint8_t *)context->pInternalBlock[REMAP_SCATTERED_DEST_OFFSET_VALUES_PTR_IDX],
                        (uint8_t *)context->pInternalBlock[REMAP_DEINTERLEAVED_CHROMA_U_V_PTR_IDX],
                        (uint8_t *)context->pOutBlock[REMAP_OUT_LUMA_IDX],
                        (const BAM_Remap_Args *)&context->kernelArgs,
                         vcopContext);

        context->vloopFunc = &vcop_remap_bb_approach;
    }
#endif

    return status;
}



static BAM_Status Bam_Remap_compute(void *kernelContext)
{
    BAM_Remap_Context  *context     = (BAM_Remap_Context *) kernelContext;
    VCOP_Remap_Context *vcopContext = &(context->vcopContext);
    BAM_Status            status = 0;

#if (NATC)
        status = vcop_remap_c((uint8_t *)context->pInBlock[REMAP_LUT_PTR_IDX],
                              (uint8_t *)context->pInBlock[REMAP_INPUT_IMAGE_BLOCK_PTR_IDX],
                              (uint8_t *)context->pInternalBlock[REMAP_CHROMA_TLU_INDEX_PTR_IDX],
                              (uint8_t *)context->pOutBlock[REMAP_OUT_LUMA_IDX],
                              (BAM_Remap_Args *)&context->kernelArgs);

#elif (VCOP_HOST_EMULATION)
        if(context->kernelArgs.maps.maxInputBlockSize != 0U)
        {
            status= vcop_remap_bb_approach_init((uint8_t *)context->pInBlock[REMAP_LUT_PTR_IDX],
                                (uint8_t *)context->pInBlock[REMAP_INPUT_IMAGE_BLOCK_PTR_IDX],
                                (uint8_t *)context->pInternalBlock[REMAP_INTERPOLATION_BUF0_PTR_IDX],
                                (uint8_t *)context->pInternalBlock[REMAP_INTERPOLATION_BUF1_PTR_IDX],
                                (uint8_t *)context->pInternalBlock[REMAP_CHROMA_TLU_INDEX_PTR_IDX],
                                (uint8_t *)context->pInternalBlock[REMAP_SCATTERED_DEST_OFFSET_VALUES_PTR_IDX],
                                (uint8_t *)context->pInternalBlock[REMAP_DEINTERLEAVED_CHROMA_U_V_PTR_IDX],
                                (uint8_t *)context->pOutBlock[REMAP_OUT_LUMA_IDX],
                                (const BAM_Remap_Args *)&context->kernelArgs,
                                 vcopContext);
        }
        else
        {
            status= vcop_remap_tile_approach_init((uint8_t *)context->pInBlock[REMAP_LUT_PTR_IDX],
                                (uint8_t *)context->pInBlock[REMAP_INPUT_IMAGE_BLOCK_PTR_IDX],
                                (uint8_t *)context->pInternalBlock[REMAP_INTERPOLATION_BUF0_PTR_IDX],
                                (uint8_t *)context->pInternalBlock[REMAP_INTERPOLATION_BUF1_PTR_IDX],
                                (uint8_t *)context->pInternalBlock[REMAP_CHROMA_TLU_INDEX_PTR_IDX],
                                (uint8_t *)context->pInternalBlock[REMAP_SCATTERED_DEST_OFFSET_VALUES_PTR_IDX],
                                (uint8_t *)context->pInternalBlock[REMAP_DEINTERLEAVED_CHROMA_U_V_PTR_IDX],
                                (uint8_t *)context->pOutBlock[REMAP_OUT_LUMA_IDX],
                                (const BAM_Remap_Args *)&context->kernelArgs,
                                 vcopContext);
        }
#else
        context->vloopFunc(vcopContext);

#endif

    return status;
}

BAM_KernelExecFuncDef gBAM_TI_remapExecFunc =
    { NULL, &Bam_Remap_initFrame, &Bam_Remap_compute, NULL, NULL, NULL };


