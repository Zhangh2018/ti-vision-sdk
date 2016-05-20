/*==========================================================================*/
/*      Copyright (C) 2009-2013 Texas Instruments Incorporated.             */
/*                      All Rights Reserved                                 */
/*==========================================================================*/

/**
 *  @file       bam_median_filter_exec_funcs.c
 *
 *  @brief      This file defines interfaces for integrating fast9 kernel
 *              into BAM
 */

#include <stdio.h>
#include <stdlib.h>

#include "evealgframework.h"

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


/* CHECK_MISRA("-19.7")  -> Disable rule 19.7  */
/* These are simple non-harmful macros.   */
#define ALIGN_4SIMD(a)  (((a) + 4U*VCOP_SIMD_WIDTH-1U) & ~(4U*VCOP_SIMD_WIDTH-1U))
/* RESET_MISRA("19.7")  -> Reset rule 19.7     */


/* Function Prototypes */
static BAM_Status BAM_Remap_getMemRecFunc(const void *kernelArgs, BAM_MemRec internalBlock[],
        BAM_MemRec outBlock[], uint8_t *numInternalBlocks, uint8_t *numOutBlocks);


static BAM_Status BAM_Remap_getMemRecFunc(const void *kernelArgs, BAM_MemRec internalBlock[],
        BAM_MemRec outBlock[], uint8_t *numInternalBlocks, uint8_t *numOutBlocks)
{
#if (VCOP_HOST_EMULATION)
    const BAM_Remap_Args *args = (const BAM_Remap_Args *)kernelArgs;
#else
    /* Typecasting by (const BAM_Remap_Args *) will lead to MISRA C 11.5. */
    const BAM_Remap_Args *args = kernelArgs;
#endif
    const sConvertMap    *maps = &(args->maps);
    Format          srcFormat;
    uint16_t        width, height;
    uint8_t         numIntBlocks = 0, enableTileApproach;
    uint16_t   tileWidth, tileHeight, inputTileSize, maxNumMappedPixels, maxNumEvenMappedPixels, maxNumOddMappedPixels;
    uint16_t   srcPixelSize;

    width     = maps->outputBlockDim.width;
    height    = maps->outputBlockDim.height;
    srcFormat = maps->srcFormat;
    srcPixelSize  = sizeofFormat(srcFormat);

    if(args->maps.maxInputBlockSize != 0U)
    {
        enableTileApproach = 0U;
    }
    else
    {
        enableTileApproach = 1U;
    }

    if(enableTileApproach)
    {
        tileWidth     = maps->inputTileDim.width;
        tileHeight    = maps->inputTileDim.height;
        inputTileSize = (tileWidth + HORZ_TILE_PADDING) * (tileHeight + VERT_TILE_PADDING);

        maxNumMappedPixels     = maps->maxNumPixelsinTile;
        maxNumEvenMappedPixels = maps->maxNumEvenPixelsinTile;
        maxNumOddMappedPixels  = maps->maxNumOddPixelsinTile;
    }
    else
    {
        inputTileSize = maps->maxInputBlockSize;
        maxNumMappedPixels = width * height;
    }

    /* Among the different interpolation techniques, Bilinear interpolation requires more memory */
    /* Allocating for worst case, that is assuming bilinear interpolation.                                        */

    /* Bilinear Interpolation needs scratch buffers in WBUF and IBUFH. One is used to store the              */
    /* regenerated Integral luma indexes and the other is used to store the table looked up pixels.          */
    /* The functions of these buffers in WBUF and IBUFH are swapped between Tile and BB Approaches. */

    /* Bilinear Interpolation needs 2 scratch buffers in WBUF.                                                      */
    /* In Tile Approach, they are used to temporarily store Integral Luma Indexes.                      */
    /* In BB Approach, they are used to temporarily store the table looked up pixels.                  */
    /* This memory will be reused for Chroma interpolation as scratch buffers.                           */
    /* The sizes of these buffers are format dependent, hence assigned below per format.         */
    internalBlock[REMAP_INTERPOLATION_BUF0_PTR_IDX].alignment         = 32U;
    internalBlock[REMAP_INTERPOLATION_BUF0_PTR_IDX].attrs.memAttrs    = BAM_MEMATTRS_SCRATCH;
    internalBlock[REMAP_INTERPOLATION_BUF0_PTR_IDX].space             = BAM_MEMSPACE_WBUF;
    numIntBlocks++;


    /* In BB, the output is filled with interpolated data in raster scan order. we can use npt() to fill
            the output. But to use the same kernel across BB and Tile approach, we are forced to use s_scatter().
            Hence, below we create the scatter offsets (1,2,3 ....) to be used by s_scatter.
            Since the kernel is compute intensive, the usage of s_scatter() will not led to performance degradation. */
    if(enableTileApproach == 0)
    {
        internalBlock[REMAP_INTERPOLATION_BUF0_PTR_IDX].size = (sizeof(uint16_t) * ALIGN_2SIMD(maxNumMappedPixels))
                                                             + (sizeof(uint16_t) * ALIGN_2SIMD(maxNumMappedPixels/2U));
    }
    /* In Tile, the input stride was passed as unsigned short as it is a constant whereas in BB, the
            pointer to the stride is passed. To use the same kernel across BB and Tile, we need to pass stride
            to the kernel as a pointer. Hence we allocate the below memory to store stride for luma and also for Chroma. */
    else
    {
        internalBlock[REMAP_INTERPOLATION_BUF0_PTR_IDX].size = 2U * sizeof(uint16_t *);
    }

    /* Bilinear Interpolation needs 2 scratch buffers in IBUFH.                                                     */
    /* In BB Approach, they are used to temporarily store Integral Luma Indexes.                      */
    /* In Tile Approach, they are used to temporarily store the table looked up Input image Luma pixels. */
    /* This memory will be reused for Chroma interpolation as scratch buffers.                           */
    /* The sizes of these buffers are format dependent, hence assigned below per format.         */
    internalBlock[REMAP_INTERPOLATION_BUF1_PTR_IDX].alignment         = 32U;
    internalBlock[REMAP_INTERPOLATION_BUF1_PTR_IDX].attrs.memAttrs    = BAM_MEMATTRS_SCRATCH;
    internalBlock[REMAP_INTERPOLATION_BUF1_PTR_IDX].space             = BAM_MEMSPACE_IBUFHA;
    numIntBlocks++;

    /* pBlocks are the scratch kernels  memory requirements for their params.        */
#if (VCOP_HOST_EMULATION)
    internalBlock[REMAP_PBLOCK_PTR_IDX].size = 4U;
#endif
    internalBlock[REMAP_PBLOCK_PTR_IDX].attrs.memAttrs    = BAM_MEMATTRS_SCRATCH;
    internalBlock[REMAP_PBLOCK_PTR_IDX].space             = BAM_MEMSPACE_WBUF;
    numIntBlocks++;



    if ((srcFormat== U8BIT) || (srcFormat== S8BIT)) {
        if(enableTileApproach)
        {
            internalBlock[REMAP_INTERPOLATION_BUF0_PTR_IDX].size += ((uint16_t)2 * sizeof(uint16_t) * ALIGN_2SIMD(maxNumMappedPixels));
            internalBlock[REMAP_INTERPOLATION_BUF1_PTR_IDX].size  = ((uint16_t)2 * (uint16_t)2 * sizeof(uint8_t) * ALIGN_2SIMD(maxNumMappedPixels));
        } else
        {
            internalBlock[REMAP_INTERPOLATION_BUF0_PTR_IDX].size += ((uint16_t)2 * (uint16_t)2 * sizeof(uint8_t) * ALIGN_2SIMD(maxNumMappedPixels));
            internalBlock[REMAP_INTERPOLATION_BUF1_PTR_IDX].size  = ((uint16_t)2 * sizeof(uint16_t) * ALIGN_2SIMD(maxNumMappedPixels));
        }
        /* P Block memory requirements */
#if (!VCOP_HOST_EMULATION)
        internalBlock[REMAP_PBLOCK_PTR_IDX].size = (vcop_bilinearInterpolate8b_param_count() * sizeof(__pblock_vcop_bilinearInterpolate8b[0]))
                                                 + (vcop_bilinearInterpolateLumaPblockUpdate_param_count() * sizeof(__pblock_vcop_bilinearInterpolateLumaPblockUpdate[0]));
#endif
    }

    else if ((srcFormat== U16BIT) || (srcFormat== S16BIT)) {
        if(enableTileApproach)
        {
            internalBlock[REMAP_INTERPOLATION_BUF0_PTR_IDX].size += ((uint16_t)2 * sizeof(uint16_t) * ALIGN_2SIMD(maxNumMappedPixels));
            internalBlock[REMAP_INTERPOLATION_BUF1_PTR_IDX].size  = (uint16_t)2 * (uint16_t)2 * sizeof(uint16_t) * ALIGN_2SIMD(maxNumMappedPixels);
        } else
        {
            internalBlock[REMAP_INTERPOLATION_BUF0_PTR_IDX].size += ((uint16_t)2 * (uint16_t)2 * sizeof(uint16_t) * ALIGN_2SIMD(maxNumMappedPixels));
            internalBlock[REMAP_INTERPOLATION_BUF1_PTR_IDX].size  = (uint16_t)2 * sizeof(uint16_t) * ALIGN_2SIMD(maxNumMappedPixels);
        }

        /* P Block memory requirements */
#if (!VCOP_HOST_EMULATION)
        internalBlock[REMAP_PBLOCK_PTR_IDX].size = (vcop_bilinearInterpolate16b_param_count() * sizeof(__pblock_vcop_bilinearInterpolate16b[0]))
                                                 + (vcop_bilinearInterpolateLumaPblockUpdate_param_count() * sizeof(__pblock_vcop_bilinearInterpolateLumaPblockUpdate[0]));
#endif
    }

    else if ((srcFormat== YUV_422ILE) || (srcFormat== YUV_422IBE)) {
        if(enableTileApproach)
        {
            internalBlock[REMAP_INTERPOLATION_BUF0_PTR_IDX].size += ((uint16_t)2 * sizeof(uint16_t) * ALIGN_2SIMD(maxNumMappedPixels));
            internalBlock[REMAP_INTERPOLATION_BUF1_PTR_IDX].size  = (uint16_t)2 * (uint16_t)2 * sizeof(uint8_t) * ALIGN_2SIMD(maxNumMappedPixels);
        } else
        {
            internalBlock[REMAP_INTERPOLATION_BUF0_PTR_IDX].size += ((uint16_t)2 * (uint16_t)2 * sizeof(uint8_t) * ALIGN_2SIMD(maxNumMappedPixels));
            internalBlock[REMAP_INTERPOLATION_BUF1_PTR_IDX].size  = (uint16_t)2 * sizeof(uint16_t) * ALIGN_2SIMD(maxNumMappedPixels);
        }

        /* P Block memory requirements */
#if (!VCOP_HOST_EMULATION)
        internalBlock[REMAP_PBLOCK_PTR_IDX].size = (vcop_deInterleaveYUV422ILE_param_count() * sizeof(__pblock_vcop_deInterleaveYUV422ILE[0]))
                                                 + (vcop_bilinearInterpolateYUV422Iluma_param_count() * sizeof(__pblock_vcop_bilinearInterpolateYUV422Iluma[0]))
                                                 + (vcop_dsTLUindexAndFracBilInterpolate_param_count() * sizeof(__pblock_vcop_dsTLUindexAndFracBilInterpolate[0]))
                                                 + (vcop_bilinearInterpolateYUV422Ichroma_param_count() * sizeof(__pblock_vcop_bilinearInterpolateYUV422Ichroma[0]))
                                                 + (vcop_bilinearInterpolateYUV422Ichroma_param_count() * sizeof(__pblock_vcop_bilinearInterpolateYUV422Ichroma[0]))
                                                 + (vcop_bilinearInterpolateLumaPblockUpdate_param_count() * sizeof(__pblock_vcop_bilinearInterpolateLumaPblockUpdate[0]))
                                                 + (vcop_dsTLUindexAndFracBilInterpolatePblockUpdate_param_count() * sizeof(__pblock_vcop_dsTLUindexAndFracBilInterpolatePblockUpdate[0]))
                                                 + (vcop_bilinearInterpolateYUV422IchromaUPblockUpdate_param_count() * sizeof(__pblock_vcop_bilinearInterpolateYUV422IchromaUPblockUpdate[0]))
                                                 + (vcop_bilinearInterpolateYUV422IchromaVPblockUpdate_param_count() * sizeof(__pblock_vcop_bilinearInterpolateYUV422IchromaVPblockUpdate[0]));
#endif

        /* Memory to separately store Integral and Fractional TLU Indexes for Chroma components populated by Down Sampling TLU Index kernel */
#if (!NATC)
        if(enableTileApproach)
        {
            if (args->interpolationChroma == BILINEAR)
            {
                internalBlock[REMAP_CHROMA_TLU_INDEX_PTR_IDX].size = (uint16_t)4*((ALIGN_2SIMD(maxNumEvenMappedPixels)) + (ALIGN_2SIMD(maxNumOddMappedPixels)));
            } else
            {
                internalBlock[REMAP_CHROMA_TLU_INDEX_PTR_IDX].size = (uint16_t)2* ( (ALIGN_2SIMD(maxNumEvenMappedPixels)) + (ALIGN_2SIMD(maxNumOddMappedPixels)) );
            }
        } else
        {
            internalBlock[REMAP_CHROMA_TLU_INDEX_PTR_IDX].size = (uint16_t)4 * ALIGN_2SIMD(maxNumMappedPixels);
        }
#else
        internalBlock[REMAP_CHROMA_TLU_INDEX_PTR_IDX].size = (uint16_t)10 * ALIGN_2SIMD(maxNumMappedPixels);
#endif
        internalBlock[REMAP_CHROMA_TLU_INDEX_PTR_IDX].alignment         = 32U;
        internalBlock[REMAP_CHROMA_TLU_INDEX_PTR_IDX].attrs.memAttrs    = BAM_MEMATTRS_SCRATCH;
        internalBlock[REMAP_CHROMA_TLU_INDEX_PTR_IDX].space             = BAM_MEMSPACE_WBUF;
        numIntBlocks++;

        /* Needed for BB Approach */
        /* 8 bits to store Scattered destination Offset values for U and V */
        internalBlock[REMAP_SCATTERED_DEST_OFFSET_VALUES_PTR_IDX].size = 8U;
        internalBlock[REMAP_SCATTERED_DEST_OFFSET_VALUES_PTR_IDX].alignment         = 32U;
        internalBlock[REMAP_SCATTERED_DEST_OFFSET_VALUES_PTR_IDX].attrs.memAttrs    = BAM_MEMATTRS_SCRATCH;
        internalBlock[REMAP_SCATTERED_DEST_OFFSET_VALUES_PTR_IDX].space             = BAM_MEMSPACE_WBUF;
        numIntBlocks++;

        /* Memory to store deInterleaved input U and V components.     */
        /* Memory for these individual components should be 32 b aligned. */
        internalBlock[REMAP_DEINTERLEAVED_CHROMA_U_V_PTR_IDX].size =  ALIGN_4SIMD((inputTileSize/2U) + 7U) + ALIGN_4SIMD((inputTileSize/2U) + 7U);
        internalBlock[REMAP_DEINTERLEAVED_CHROMA_U_V_PTR_IDX].alignment         = 32U;
        internalBlock[REMAP_DEINTERLEAVED_CHROMA_U_V_PTR_IDX].attrs.memAttrs    = BAM_MEMATTRS_SCRATCH;
        internalBlock[REMAP_DEINTERLEAVED_CHROMA_U_V_PTR_IDX].space             = BAM_MEMSPACE_IBUFHA;
        numIntBlocks++;

    }

    /* CHECK_MISRA("-14.1")  -> Disable rule 14.1  */
    /* Currently srcFormat will not be grater than YUV_420SP, hence code is Always true  */
    /* as the implementation of the function changes, it will can be false.   */
    /* INVARIANT_CONDITION.GEN */
    else if (srcFormat== YUV_420SP) {
  /* RESET_MISRA("14.1")  -> Reset rule 14.1      */
        if(enableTileApproach)
        {
            internalBlock[REMAP_INTERPOLATION_BUF0_PTR_IDX].size += ((uint16_t)2 * sizeof(uint16_t) * ALIGN_2SIMD(maxNumMappedPixels));
            internalBlock[REMAP_INTERPOLATION_BUF1_PTR_IDX].size  = (uint16_t)2 * (uint16_t)2 * sizeof(uint8_t) * ALIGN_2SIMD(maxNumMappedPixels);
        } else
        {
            internalBlock[REMAP_INTERPOLATION_BUF0_PTR_IDX].size += ((uint16_t)2 * (uint16_t)2 * sizeof(uint8_t) * ALIGN_2SIMD(maxNumMappedPixels));
            internalBlock[REMAP_INTERPOLATION_BUF1_PTR_IDX].size  = (uint16_t)2 * sizeof(uint16_t) * ALIGN_2SIMD(maxNumMappedPixels);
        }

#if (!VCOP_HOST_EMULATION)
        internalBlock[REMAP_PBLOCK_PTR_IDX].size =( vcop_bilinearInterpolate8b_param_count() * sizeof(__pblock_vcop_bilinearInterpolate8b[0]))
                                + (vcop_chromaTLUIndexCalcBilInterpolate_param_count() * sizeof(__pblock_vcop_chromaTLUIndexCalcBilInterpolate[0]))
                                + (vcop_bilinearInterpolateYUV420SPchroma_param_count() * sizeof(__pblock_vcop_bilinearInterpolateYUV420SPchroma[0]))
                                + (vcop_bilinearInterpolateLumaPblockUpdate_param_count() * sizeof(__pblock_vcop_bilinearInterpolateLumaPblockUpdate[0]))
                                + (vcop_chromaTLUIndexCalcBilInterpolatePblockUpdate_param_count() * sizeof(__pblock_vcop_chromaTLUIndexCalcBilInterpolatePblockUpdate[0]))
                                + (vcop_bilinearInterpolateYUV420SPchromaPblockUpdate_param_count() * sizeof(__pblock_vcop_bilinearInterpolateYUV420SPchromaPblockUpdate[0]));
#endif

        /* Memory to store TLU Index for Chroma component populated by Chroma TLU Index Calculation kernel */
        if(enableTileApproach)
        {
            /* 32 is added to enable Offset writing into end of Scatter Store buffer */
            if (args->interpolationChroma == BILINEAR)
            {
                internalBlock[REMAP_CHROMA_TLU_INDEX_PTR_IDX].size = ((uint16_t)6*ALIGN_2SIMD(maxNumEvenMappedPixels)) + 32U;
            } else
            {
                internalBlock[REMAP_CHROMA_TLU_INDEX_PTR_IDX].size = ((uint16_t)4*ALIGN_2SIMD(maxNumEvenMappedPixels)) + 32U;
            }
        } else
        {
#if (!NATC)
            internalBlock[REMAP_CHROMA_TLU_INDEX_PTR_IDX].size = ALIGN_2SIMD(maxNumMappedPixels);
#else
            internalBlock[REMAP_CHROMA_TLU_INDEX_PTR_IDX].size = 4U*ALIGN_2SIMD(maxNumMappedPixels);
#endif
        }
        internalBlock[REMAP_CHROMA_TLU_INDEX_PTR_IDX].alignment         = 32U;
        internalBlock[REMAP_CHROMA_TLU_INDEX_PTR_IDX].attrs.memAttrs    = BAM_MEMATTRS_SCRATCH;
        internalBlock[REMAP_CHROMA_TLU_INDEX_PTR_IDX].space             = BAM_MEMSPACE_WBUF;
        numIntBlocks++;

        /* Needed for Tile Approach */
        /* 8 bits to store Scattered destination Offset values for U and V */
        internalBlock[REMAP_SCATTERED_DEST_OFFSET_VALUES_PTR_IDX].size = 8U;
        internalBlock[REMAP_SCATTERED_DEST_OFFSET_VALUES_PTR_IDX].alignment         = 32U;
        internalBlock[REMAP_SCATTERED_DEST_OFFSET_VALUES_PTR_IDX].attrs.memAttrs    = BAM_MEMATTRS_SCRATCH;
        internalBlock[REMAP_SCATTERED_DEST_OFFSET_VALUES_PTR_IDX].space             = BAM_MEMSPACE_WBUF;
        numIntBlocks++;

    }
    else
    {
        /* The else is added to avoid MISRA RULE 14.10 MISRA.IF.NO_ELSE. */
    }

    /* Filling remaining fields of internalBlock Array */
    for(; numIntBlocks < REMAP_NUM_INTERNAL_BLOCKS; numIntBlocks++)
    {
        internalBlock[numIntBlocks].size           = 4U;
        internalBlock[numIntBlocks].alignment      = 4U;
        internalBlock[numIntBlocks].attrs.memAttrs = BAM_MEMATTRS_SCRATCH;
        internalBlock[numIntBlocks].space          = BAM_MEMSPACE_WBUF;
    }

    /* Memory for remapped Output */
    outBlock[REMAP_OUT_LUMA_IDX].size = srcPixelSize * ALIGN_4SIMD(width * height);
    outBlock[REMAP_OUT_LUMA_IDX].alignment         = 32U;
    outBlock[REMAP_OUT_LUMA_IDX].attrs.memAttrs    = BAM_MEMATTRS_SCRATCH;
    outBlock[REMAP_OUT_LUMA_IDX].space             = BAM_MEMSPACE_IBUFHA;
    if (srcFormat== YUV_420SP)
    {
        outBlock[REMAP_OUT_LUMA_IDX].size += (ALIGN_4SIMD(width * height)/2U);
    }
    if(enableTileApproach)
    {
        /* Extra memory to Scatter store invalid data */
        outBlock[REMAP_OUT_LUMA_IDX].size += 32U;
        outBlock[REMAP_OUT_LUMA_IDX].space             = BAM_MEMSPACE_WBUF;
    }

    return BAM_S_SUCCESS;
}


BAM_KernelInfo gBAM_TI_remapKernel =
{
    0,                                      /* kernelId */
    sizeof(BAM_Remap_Context),
    sizeof(BAM_Remap_Args),
    BAM_EVE,                                /* coreType */
    BAM_NODE_COMPUTE,                       /* nodeType */
    REMAP_NUM_IN_BLOCKS,                    /* 2 */
    REMAP_NUM_OUT_BLOCKS,                   /* 1 */
    REMAP_NUM_INTERNAL_BLOCKS               /* 6 */
};

BAM_KernelHelperFuncDef gBAM_TI_remapHelperFunc =
{ &BAM_Remap_getMemRecFunc, NULL};

