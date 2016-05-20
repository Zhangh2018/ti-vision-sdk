/*==========================================================================*/
/*      Copyright (C) 2009-2013 Texas Instruments Incorporated.             */
/*                      All Rights Reserved                                 */
/*==========================================================================*/

/**
 *  @file       bam_nms_s16_to_list_helper_funcs.c
 *
 *  @brief      This file defines interfaces for integrating for integrating the combination of
 *                NMS 16 bit signed kernel and bin to image list kernel  into BAM.
 */

#include <stdio.h>
#include <stdlib.h>

#include "evealgframework.h"

#include "bam_nms_s16_to_list.h"
#include "bam_nms_s16_to_list_int.h"

#include "bam_nonMaxSuppress_mxn_S16bitPack.h"
#include "bam_nonMaxSuppress_mxn_S16bitPack_int.h"

#include "bam_bin_image_to_list.h"
#include "bam_bin_image_to_list_int.h"
#include "vcop_nonMaxSuppress_mxn_16s_kernel.h"

#define NMS_IMAGE_TO_LIST_MAX_BLK_SIZE_INBYTES (7000U)

/* Function Prototypes */
static BAM_Status BAM_Nms16sToList_getMemRecFunc(const void *kernelArgs, BAM_MemRec internalBlock[],
            BAM_MemRec outBlock[], uint8_t *numInternalBlocks, uint8_t *numOutBlocks);
static BAM_Status BAM_Nms16sToList_setMemRecFunc(const BAM_MemRec *memRecInternal[],
            const BAM_MemRec *memRecInputDataBlock[], const BAM_MemRec *memRecOutputDataBlock[],
            uint8_t numRecInternal, uint8_t numRecInputDataBlock, uint8_t numRecOutputDataBlock,
            void *kernelContext, const void *kernelArgs);


static BAM_Status BAM_Nms16sToList_getMemRecFunc(const void *kernelArgs, BAM_MemRec internalBlock[],
            BAM_MemRec outBlock[], uint8_t *numInternalBlocks, uint8_t *numOutBlocks)
{
  BAM_MemRec nms16sBitPackInternalBlock[BAM_NONMAXSUPPRESS_MXN_S16BITPACK_INTERNAL_NUM_BLOCK];
  BAM_MemRec nms16sBitPackOutBlock[BAM_NONMAXSUPPRESS_MXN_S16BITPACK_NUM_OUTPUT_BLOCKS];

  BAM_MemRec binImgToListInternalBlock[BAM_BIN_IMAGE_TO_LIST_NUM_INTERNAL_BLOCKS];
  BAM_MemRec binImgToListOutBlock[BAM_BIN_IMAGE_TO_LIST_NUM_OUT_BLOCKS];

  int32_t status = BAM_S_SUCCESS;

  gBAM_TI_nonMaxSuppress_mxn_S16bitPack_HelperFunc.getKernelMemRec(
                NULL,
                nms16sBitPackInternalBlock,
                nms16sBitPackOutBlock,
                NULL,
                NULL
                );

  gBAM_TI_binImageToListHelperFunc.getKernelMemRec(
                NULL,
                binImgToListInternalBlock,
                binImgToListOutBlock,
                NULL,
                NULL
                );

  internalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_NMSBINPACK_PARAM_IDX].size =
                nms16sBitPackInternalBlock[BAM_NONMAXSUPPRESS_MXN_S16BITPACK_INTERNAL_PARAMS_IDX].size;
  internalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_NMSBINPACK_PARAM_IDX].space = BAM_MEMSPACE_WBUF;
  internalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_NMSBINPACK_PARAM_IDX].attrs.memAttrs = BAM_MEMATTRS_CONST;

  internalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_BINTOLIST_PARAM_IDX].size =
                binImgToListInternalBlock[BAM_BIN_IMAGE_TO_LIST_TEMP_PARAM].size;
  internalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_BINTOLIST_PARAM_IDX].space = BAM_MEMSPACE_WBUF;
  internalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_BINTOLIST_PARAM_IDX].attrs.memAttrs = BAM_MEMATTRS_CONST;

#if !VCOP_HOST_EMULATION
  internalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_UPDATE_DST_PARAM_IDX].size =
                vcop_nonMaxSuppress_mxn_16sUpdateDstPtr_param_count();
#else
  internalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_UPDATE_DST_PARAM_IDX].size = 4;
#endif
  internalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_UPDATE_DST_PARAM_IDX].space = BAM_MEMSPACE_WBUF;
  internalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_UPDATE_DST_PARAM_IDX].attrs.memAttrs = BAM_MEMATTRS_CONST;


  internalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_DSTPTR_OFFSET_IDX].size = sizeof(uint16_t);
  internalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_DSTPTR_OFFSET_IDX].space = BAM_MEMSPACE_IBUFLA;
  internalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_DSTPTR_OFFSET_IDX].attrs.memAttrs = BAM_MEMATTRS_CONST;


  internalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_SCRATCHBUF1_IDX].size =
                nms16sBitPackInternalBlock[BAM_NONMAXSUPPRESS_MXN_S16BITPACK_INTERNAL_SCRATCHBUF1].size;
  internalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_SCRATCHBUF1_IDX].space = BAM_MEMSPACE_WBUF;
  internalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_SCRATCHBUF1_IDX].attrs.memAttrs = BAM_MEMATTRS_SCRATCH;

  internalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_BINPACK_BUF_IDX].size =
                NMS_IMAGE_TO_LIST_MAX_BLK_SIZE_INBYTES;
  internalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_BINPACK_BUF_IDX].space = BAM_MEMSPACE_WBUF;
  internalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_BINPACK_BUF_IDX].attrs.memAttrs = BAM_MEMATTRS_CONST;


  internalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_XSEQ_IDX].size =
                binImgToListInternalBlock[BAM_BIN_IMAGE_TO_LIST_TEMP_XSEQ].size;
  internalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_XSEQ_IDX].space = BAM_MEMSPACE_WBUF;
  internalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_XSEQ_IDX].attrs.memAttrs = BAM_MEMATTRS_SCRATCH;


  internalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_STARTXY_IDX].size =
                binImgToListInternalBlock[BAM_BIN_IMAGE_TO_LIST_TEMP_PORT_STARTXY].size;
  internalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_STARTXY_IDX].space = BAM_MEMSPACE_WBUF;
  internalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_STARTXY_IDX].attrs.memAttrs = BAM_MEMATTRS_SCRATCH;

  outBlock[BAM_NMS_MXN_S16_TO_LIST_OUTPUT_XY_LIST_PORT].size =
                binImgToListOutBlock[BAM_BIN_IMAGE_TO_LIST_OUTPUT_PORT_XY].size;
  outBlock[BAM_NMS_MXN_S16_TO_LIST_OUTPUT_XY_LIST_PORT].space = BAM_MEMSPACE_IBUFHA;
  outBlock[BAM_NMS_MXN_S16_TO_LIST_OUTPUT_XY_LIST_PORT].attrs.memAttrs = BAM_MEMATTRS_SCRATCH;

  outBlock[BAM_NMS_MXN_S16_TO_LIST_OUTPUT_LIST_SIZE_PORT].size =
                binImgToListOutBlock[BAM_BIN_IMAGE_TO_LIST_OUTPUT_PORT_SIZE].size;
  outBlock[BAM_NMS_MXN_S16_TO_LIST_OUTPUT_LIST_SIZE_PORT].space = BAM_MEMSPACE_IBUFLA;
  outBlock[BAM_NMS_MXN_S16_TO_LIST_OUTPUT_LIST_SIZE_PORT].attrs.memAttrs = BAM_MEMATTRS_SCRATCH;

  return status;
}


static BAM_Status BAM_Nms16sToList_setMemRecFunc(const BAM_MemRec *memRecInternal[],
            const BAM_MemRec *memRecInputDataBlock[], const BAM_MemRec *memRecOutputDataBlock[],
            uint8_t numRecInternal, uint8_t numRecInputDataBlock, uint8_t numRecOutputDataBlock,
            void *kernelContext, const void *kernelArgs)
{
  BAM_Nms_16s_to_list_Context            *context =  (BAM_Nms_16s_to_list_Context *)kernelContext;
  BAM_Bin_image_to_list_Context  *binImgToListContext = &context->binImgToListContext;
  BAM_NonMaxSuppress_mxn_S16bitPack_Context  *nms16sbitPackContext= &context->nms16sbitPackContext;
  int32_t i;
  int32_t status = BAM_S_SUCCESS;

  for ( i = 0; i < (int32_t)numRecInternal; i++ )
  {
    if ( memRecInternal[i]->base == NULL )
    {
      status = BAM_E_FAIL;
      /* CHECK_MISRA("-14.4")  -> Disable rule 14.4  */
      /* GOTO is used at error check to jump to end of function, to exit.   */
      goto EXIT;
      /* RESET_MISRA("14.4")  -> Reset rule 14.4  */
    }
    context->pInternalBlock[i] = memRecInternal[i]->base;
  }

  for ( i = 0; i < (int32_t)numRecInputDataBlock; i++ )
  {
    if ( memRecInputDataBlock[i]->base == NULL )
    {
      status = BAM_E_FAIL;
      /* CHECK_MISRA("-14.4")  -> Disable rule 14.4  */
      /* GOTO is used at error check to jump to end of function, to exit.   */
      goto EXIT;
      /* RESET_MISRA("14.4")  -> Reset rule 14.4  */
    }
    context->pInBlock[i] = memRecInputDataBlock[i]->base;
  }

  for ( i = 0; i < (int32_t)numRecOutputDataBlock; i++ )
  {
    if ( memRecOutputDataBlock[i]->base == NULL )
    {
      status = BAM_E_FAIL;
      /* CHECK_MISRA("-14.4")  -> Disable rule 14.4  */
      /* GOTO is used at error check to jump to end of function, to exit.   */
      goto EXIT;
      /* RESET_MISRA("14.4")  -> Reset rule 14.4  */

    }
    context->pOutBlock[i] = memRecOutputDataBlock[i]->base;
  }

  context->numNms16sbitPackParamCnt = memRecInternal[NMS_MXN_S16_TO_LIST_INTERNAL_NMSBINPACK_PARAM_IDX]->size / sizeof(uint16_t) ;


  nms16sbitPackContext->pInternalBlock[BAM_NONMAXSUPPRESS_MXN_S16BITPACK_INTERNAL_PARAMS_IDX] =
            context->pInternalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_NMSBINPACK_PARAM_IDX];

  nms16sbitPackContext->pInternalBlock[BAM_NONMAXSUPPRESS_MXN_S16BITPACK_INTERNAL_SCRATCHBUF1] =
            context->pInternalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_SCRATCHBUF1_IDX];

  /* We are resuing the output buffer for scratch buf 2 thats why here we are using pOutBlock instead of
  pInternalBlock */
  nms16sbitPackContext->pInternalBlock[BAM_NONMAXSUPPRESS_MXN_S16BITPACK_INTERNAL_SCRATCHBUF2] =
            context->pOutBlock[BAM_NMS_MXN_S16_TO_LIST_OUTPUT_XY_LIST_PORT];

  nms16sbitPackContext->pInBlock[BAM_NONMAXSUPPRESS_MXN_S16BITPACK_INPUT_PORT] =
            context->pInBlock[BAM_NMS_MXN_S16_TO_LIST_INPUT_BLOCK_PORT];

  /* Output of nms kernel is stored in intermediate buffer */
  nms16sbitPackContext->pOutBlock[BAM_NONMAXSUPPRESS_MXN_S16BITPACK_OUTPUT_PORT] =
            context->pInternalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_BINPACK_BUF_IDX];

  binImgToListContext->pInternalBlock[BAM_BIN_IMAGE_TO_LIST_TEMP_PARAM] =
        context->pInternalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_BINTOLIST_PARAM_IDX];

  binImgToListContext->pInternalBlock[BAM_BIN_IMAGE_TO_LIST_TEMP_PORT_STARTXY] =
        context->pInternalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_STARTXY_IDX];

  binImgToListContext->pInternalBlock[BAM_BIN_IMAGE_TO_LIST_TEMP_XSEQ] =
        context->pInternalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_XSEQ_IDX];

  binImgToListContext->pInBlock[BAM_BIN_IMAGE_TO_LIST_INPUT_PORT_IMG] =
        context->pInternalBlock[NMS_MXN_S16_TO_LIST_INTERNAL_BINPACK_BUF_IDX];

  binImgToListContext->pOutBlock[BAM_BIN_IMAGE_TO_LIST_OUTPUT_PORT_XY] =
        context->pOutBlock[BAM_NMS_MXN_S16_TO_LIST_OUTPUT_XY_LIST_PORT];

  binImgToListContext->pOutBlock[BAM_BIN_IMAGE_TO_LIST_OUTPUT_PORT_SIZE] =
        context->pOutBlock[BAM_NMS_MXN_S16_TO_LIST_OUTPUT_LIST_SIZE_PORT];

EXIT:
  return status;
}


BAM_KernelInfo gBAM_TI_nms16sToListKernel =
{
  0,                                      /* kernelId */
  sizeof(BAM_Nms_16s_to_list_Context),
  0,
  BAM_EVE,                                /* coreType */
  BAM_NODE_COMPUTE,                       /* nodeType */
  BAM_NMS_MXN_S16_TO_LIST_NUM_INPUT_BLOCKS,/* Num Input Blocks*/
  BAM_NMS_MXN_S16_TO_LIST_NUM_OUTPUT_BLOCKS,/* Num Output Blocks*/
  NMS_MXN_S16_TO_LIST_INTERNAL_NUM_BLOCK/* Num Internal Blocks*/
};

/* NMS 16s to list  Node kernel Info, helper, and exec functions */
BAM_KernelHelperFuncDef gBAM_TI_nms16sToListHelperFunc =
{
  &BAM_Nms16sToList_getMemRecFunc, /* function to get kernels memory records BAM will call it when BAM_getNodeMemRec() is invoked by the applet */
  &BAM_Nms16sToList_setMemRecFunc  /* function to get kernels memory records BAM will call it when BAM_getNodeMemRec() is invoked by the applet */
};


