/******************************************************************************
 *                                                                            *
 * Copyright (c) 2012 Texas Instruments Incorporated - http://www.ti.com/     *
 *                                                                            *
 * All rights reserved. Property of Texas Instruments Incorporated.           *
 * Restricted rights to use, duplicate or disclose this code are              *
 * granted through contract.                                                  *
 *                                                                            *
 * The program may not be used without the written permission                 *
 * of Texas Instruments Incorporated or against the terms and conditions      *
 * stipulated in the agreement under which this program has been              *
 * supplied.                                                                  *
 *                                                                            *
 *****************************************************************************/

/**
 *  \file vpsdrv_m2mSlice.c
 *
 *  \brief VPS M2M SubFrame level processing functions used in all m2m drivers
 *           which supports sub-frame level processing .
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <xdc/std.h>
#include <common/bsp_types.h>
#include <osal/bsp_osal.h>
#include <common/trace.h>
#include <common/bsp_config.h>
#include <common/bsp_utils.h>

#include <fvid2/fvid2.h>
#include <vps/vps.h>
#include <vpedrv/vpsdrv_m2mSlice.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/** \brief Scaler vertical filter RAV max init value: used for subframe height
 *calc */
#define VPSMDRV_SC_VERT_RAV_INIT_MAX                  (1024U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/**
 * \brief Initializes subframe instance structure of the channel.
 * Called when first subframe of a frame is processed.
 * Initializes buffer offsets and calculates subframe height and scaler
 * phase information based on chosen configuration.
 * Source/target frame height, filter type and dataformat should
 * have been initialised before calling this function which is done at Create
 * time
 * TODO: Also takes care of
 * updating subframe instance based on frame runtime parameters.
 */
void vpsMdrvScSubFrameInitInfo(VpsMdrv_SubFrameChInst *slcData)
{
    slcData->subFrameNum  = 0;
    slcData->srcBufOffset = 0;
    slcData->tarBufOffset = 0;
    slcData->rowOffset    = 0;

    /* Since Scaler hardware use multi tap filter, it needs few lines of image
     * from
     *     previous and next subFrame to make scaled output equal to frame level
     * processing.
     *     This Line memory varies depending on Scaler type (polyPhase/ RAV) and
     *     input type (420/422) as CHR_US line memory comes into picture */
    if (slcData->dataFmt == FVID2_DF_YUV420SP_UV)
    {
        if (slcData->filtType == VPS_SC_VST_POLYPHASE)
        {
            slcData->chrUsLnMemSlcTop = 0;
            /*< 0 for first subframe and all other subFrame 2: 2 for chr_us*/
            slcData->chrUsLnMemSlcBtm = 4;
            /*< 0 for Last subframe and all other subFrame 4: 4 for chr_us */
            slcData->scCropLnsTop = 0;
            /*< 0 for first subframe and all other subFrame 2*/
            slcData->scCropLnsBtm = 4;
            /*< 0 for Last subframe and all other subFrame 4*/
        }
        else if (slcData->filtType == VPS_SC_VST_RAV)
        {
            slcData->chrUsLnMemSlcTop = 0;
            /* 0 for first subframe and all other subFrame 2+1: 2 for chr_us and
             *+1 for Sc*/
            slcData->chrUsLnMemSlcBtm = 4;
            /*0 for Last subframe and all other subframes 4:0 for Sc  and 4 for
             *chr_us */
            slcData->scCropLnsTop = 0;
            /*< 0 for first subframe and all other subframes 3 :(2+1)*/
            slcData->scCropLnsBtm = 4;
            /*< 0 for Last subframe and all other subframes 4*/
        }
        else
        {
            /* Do nothing for AUTO mode */
        }
    }
    else /* 422 input case */
    {
        slcData->chrUsLnMemSlcTop = 0;
        slcData->chrUsLnMemSlcBtm = 0;
        slcData->scCropLnsTop     = 0;
        slcData->scCropLnsBtm     = 0;
    }

    /*Scaler Phase info calc for PolyPh and RAV filter  */
    /* TODO: assumes progressinv in/out. Need update to support interlaced
     *in/out */
    if (slcData->filtType == VPS_SC_VST_POLYPHASE)
    {
        slcData->rowAccIncr = ((slcData->srcFrmH - 1U) << 16) /
                              (slcData->tarFrmH - 1U);
        slcData->rowAccOffset  = 0;
        slcData->rowAccOffsetB = 0;
        slcData->rowAcc        = 0;
    }
    else if (slcData->filtType == VPS_SC_VST_RAV)
    {
        /*Scaler Phase info calc for RAV filter  */
        slcData->ravMaxSrcHtPerSlc = (slcData->slcSz);
        slcData->ravScFactor       = (UInt16) ((slcData->tarFrmH << 10) /
                                               slcData->srcFrmH);
        slcData->ravRowAccInit = slcData->ravScFactor +
                                 ((1U + slcData->ravScFactor) >> 1U);
        if (slcData->ravRowAccInit >= (Int32) VPSMDRV_SC_VERT_RAV_INIT_MAX)
        {
            slcData->ravRowAccInit -= VPSMDRV_SC_VERT_RAV_INIT_MAX;
        }
        slcData->ravRowAccInitB =
            slcData->ravRowAccInit +
            (((UInt32) 1U + (slcData->ravRowAccInit >> (UInt32) 1U)) -
             (VPSMDRV_SC_VERT_RAV_INIT_MAX >> (UInt32) 1U));
        if (slcData->ravRowAccInitB < 0)
        {
            slcData->ravRowAccInitB += slcData->ravRowAccInit;
            slcData->ravRowAccInit  += slcData->ravRowAccInit;
        }
    }
    else
    {
        /* Do nothing for AUTO mode */
    }

    return;
}

void vpsMdrvScSubFrameCalcInfo(VpsMdrv_SubFrameChInst *slcData)

{
    Float tempData1;
    Float tempData2, tempData3, tempData4, result;
    if (slcData->filtType == VPS_SC_VST_POLYPHASE)
    {
        /* source height*/
        slcData->srcH = slcData->slcSz + 4;
        /* make this as first subframe '0' for INT DRV */
        if (slcData->subFrameNum == (slcData->noOfSlcs - 1))
        {
            slcData->srcH = slcData->srcH - 2;
        }

        /* target height*/
        if (slcData->subFrameNum == 0)    /* the first subframe */
        {
            tempData1 = (Float) slcData->srcH - (Float) 4.5;
            tempData2 = ((Float) slcData->tarFrmH - (Float) 1U);
            tempData3 = ((Float) slcData->srcFrmH - (Float) 1U);
            tempData4 = ((Float) slcData->rowAccOffset / (Float) 65536.0);
            result    =
                ((tempData1 -
                  tempData4) * (tempData2 / tempData3)) + (Float) 1U;
            slcData->tarH = (UInt32) result;
        }
        /* last subframe */
        else if (slcData->subFrameNum == (slcData->noOfSlcs - 1))
        {
            tempData1 = (Float) slcData->srcH - (Float) 0.5;
            tempData2 = ((Float) slcData->tarFrmH - (Float) 1U);
            tempData3 = ((Float) slcData->srcFrmH - (Float) 1U);
            tempData4 = ((Float) slcData->rowAccOffset / (Float) 65536.0);
            result    =
                ((tempData1 -
                  tempData4) * (tempData2 / tempData3)) + (Float) 1U;
            slcData->tarH = (UInt32) result;
        }
        else
        {
            tempData1 = (Float) slcData->srcH - (Float) 2.5;
            tempData2 = ((Float) slcData->tarFrmH - (Float) 1U);
            tempData3 = ((Float) slcData->srcFrmH - (Float) 1U);
            tempData4 = ((Float) slcData->rowAccOffset / (Float) 65536.0);
            result    =
                ((tempData1 -
                  tempData4) * (tempData2 / tempData3)) + (Float) 1U;
            slcData->tarH = (UInt32) result;
        }
    }
    else if (slcData->filtType == VPS_SC_VST_RAV)
    {
        /* Calculate the last RAV filter reset line with in the given max Source
         * height lines
         *         Also calculate the target height */
        UInt16 tarRow, srcRow, slcSrcH = 0;
        UInt32 rowAcc = slcData->ravRowAccInit;
        UInt32 rowAccEndOfLstSlc = slcData->ravRowAccInit;
        tarRow = 0;
        for (srcRow = 0; srcRow < slcData->ravMaxSrcHtPerSlc; srcRow++)
        {
            if (rowAcc >= VPSMDRV_SC_VERT_RAV_INIT_MAX)
            {
                rowAcc -= VPSMDRV_SC_VERT_RAV_INIT_MAX;
                tarRow++;
                slcSrcH           = srcRow;
                rowAccEndOfLstSlc = rowAcc;
            }
            rowAcc += slcData->ravScFactor;
        }

        slcData->srcH          = slcSrcH + 1U;
        slcData->tarH          = tarRow;
        slcData->ravRowAccInit = rowAccEndOfLstSlc;
    }
    else
    {
        /* Do nothing for AUTO mode */
    }

    return;
}

/* update post processing subframe data */
void vpsMdrvScSubFrameCalcPostProcInfo(VpsMdrv_SubFrameChInst *slcData)
{
    UInt32 i;

    if (slcData->dataFmt == FVID2_DF_YUV420SP_UV)
    {
        if (slcData->filtType == VPS_SC_VST_POLYPHASE)
        {
            slcData->chrUsLnMemSlcTop = 2;
            /*< 0 for first subframe and all other subframes 2: for chr_us*/
            slcData->scCropLnsTop = 2;
            /*< 0 for first subframe and all other subframes 2*/
            if (slcData->subFrameNum == (slcData->noOfSlcs - 1))
            {
                slcData->chrUsLnMemSlcBtm = 0;
                /*< 0 for Last subframe and all other subframes 4: 4 for
                 *  chr_us */
                slcData->scCropLnsBtm = 0;
                /*< 0 for Last subframe and all other subframes 4*/
            }
            else
            {
                slcData->chrUsLnMemSlcBtm = 4;
                /*< 0 for Last subframe and all other subframes 4: 4 for
                 *  chr_us */
                slcData->scCropLnsBtm = 4;
                /*< 0 for Last subframe and all other subframes 4*/
            }

            for (i = 0; i < slcData->tarH; i++) {
                slcData->rowAcc += slcData->rowAccIncr;      /* phase
                                                              *accumulation*/
            }
            /* at subframe boundary, need to read two more lines for overlapped
             * processing */
            slcData->rowAccOffset = (slcData->rowAcc -
                                     ((((UInt32) slcData->srcH - 4U) *
                                       ((UInt32) slcData->subFrameNum + 1U)) <<
                                      16U)) + ((UInt32) 2U << 16U);

            /* update row_offset which is to determine slice_row_offset for each
             * subframe input
             *           vertical index.*/
            slcData->rowOffset    += (slcData->srcH - 4);
            slcData->tarBufOffset += (slcData->tarH);

            /* source buf offset. -2 for scaler top line memory */
            slcData->srcBufOffset =
                (slcData->rowOffset - 2 - slcData->chrUsLnMemSlcTop);
        }
        else if (slcData->filtType == VPS_SC_VST_RAV)
        {
            slcData->chrUsLnMemSlcTop = 2 + 1;
            /* 0 for first subframe and all other subframes 2+2: 2 for Sc and 2
             *for chr_us*/
            slcData->scCropLnsTop = 3;
            /*< 0 for first subframe and all other subframes 3 :(2+1)*/

            if (slcData->subFrameNum == (slcData->noOfSlcs - 1))
            {
                slcData->chrUsLnMemSlcBtm = 0;
                /*0 for Last subframe and all other subframes 4:0 for Sc  and 4
                 *for chr_us */
                slcData->scCropLnsBtm = 0;
                /*< 0 for Last subframe and all other subframes 4*/
            }
            else
            {
                slcData->chrUsLnMemSlcBtm = 4;
                /*0 for Last subframe and all other subframes 4:0 for Sc  and 4
                 *for chr_us */
                slcData->scCropLnsBtm = 4;
                /*< 0 for Last subframe and all other subframes 4*/
            }
            /* one line of memory for RAV from prev subframe */
            slcData->rowOffset    += (slcData->srcH - 1);
            slcData->tarBufOffset += slcData->tarH;

            /* max process remaining lines from previous subframe and the lines
             *in next subframe */
            slcData->ravMaxSrcHtPerSlc = ((slcData->ravMaxSrcHtPerSlc
                                           - (slcData->srcH - 1))
                                          + slcData->slcSz);
            /* Process all remaining lines with Last subframe */
            if ((slcData->srcBufOffset + slcData->ravMaxSrcHtPerSlc) >
                slcData->srcFrmH)
            {
                slcData->ravMaxSrcHtPerSlc =
                    (slcData->srcFrmH - slcData->srcBufOffset);
            }

            slcData->srcBufOffset = (slcData->rowOffset - 1
                                     - slcData->scCropLnsTop);
            /* -1 for RAV Ln Mem on top*/
        }
        else
        {
            /* Do nothing for AUTO mode */
        }
    }
    else
    {
        slcData->chrUsLnMemSlcTop = 0;
        slcData->chrUsLnMemSlcBtm = 0;
        slcData->scCropLnsTop     = 0;
        slcData->scCropLnsBtm     = 0;

        if (slcData->filtType == VPS_SC_VST_POLYPHASE)
        {
            for (i = 0; i < slcData->tarH; i++)
            {
                slcData->rowAcc += slcData->rowAccIncr;
            }
            /* at subframe boundary, need to read two more lines for overlapped
             *processing */
            slcData->rowAccOffset = (slcData->rowAcc -
                                     ((((UInt32) slcData->srcH - 4U) *
                                       ((UInt32) slcData->subFrameNum + 1U))
                                      << 16U)) + ((UInt32) 2U << 16U);

            /* update row_offset which is to determine slice_row_offset for each
             * subframe
             *            input vertical index.*/
            slcData->rowOffset    += (slcData->srcH - 4);
            slcData->tarBufOffset += (slcData->tarH);

            /* source buf offset : at subframe boundary, need to read
             * two more */
            slcData->srcBufOffset = slcData->rowOffset - 2;
        }
        else if (slcData->filtType == VPS_SC_VST_RAV)
        {
            slcData->srcBufOffset += (slcData->srcH - 1);
            /* one line of memory for RAV from prev subframe */
            slcData->tarBufOffset += slcData->tarH;

            /* max process remaining lines from previous subframe and the lines
             *in next subframe */
            slcData->ravMaxSrcHtPerSlc = ((slcData->ravMaxSrcHtPerSlc -
                                           (slcData->srcH - 1)) +
                                          slcData->slcSz);
            /* Process all remaining lines with Last subframe */
            if ((slcData->srcBufOffset + slcData->ravMaxSrcHtPerSlc) >
                slcData->srcFrmH)
            {
                slcData->ravMaxSrcHtPerSlc =
                    (slcData->srcFrmH - slcData->srcBufOffset);
            }
        }
        else
        {
            /* Do nothing for AUTO mode */
        }
    }

    return;
}

