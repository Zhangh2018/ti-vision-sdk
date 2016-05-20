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
 *  \file M2mVpeScale_utils.h
 *
 *  \brief VPS VPE memory to memory driver example header file containing
 *  appplication object and utility function desclaration.
 *
 */

#ifndef M2MVPESCALE_UTILS_H_
#define M2MVPESCALE_UTILS_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdio.h>
#include <string.h>
#include <xdc/std.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/utils/Load.h>

#include <common/bsp_types.h>
#include <common/trace.h>
#include <common/bsp_config.h>
#include <common/bsp_utils.h>
#include <platforms/bsp_platform.h>
#include <fvid2/fvid2.h>
#include <vps/vps.h>
#include <vps/vps_control.h>
#include <vps/vps_m2m.h>
#include <utility/bsputils_mem.h>
#include <utility/bsputils_tiler.h>
#include <utility/bsputils_app.h>
#include <utility/bsputils_prf.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Number of input buffers to allocate. */
#define VPE_MAX_IN_BUFFERS              (20U)

/* Number of output buffers to allocate. */
#define VPE_MAX_OUT_BUFFERS             (VPS_M2M_MAX_CH_PER_HANDLE * \
                                         VPE_MAX_IN_BUFFERS)

/* Number of FVID2 frame list to allocate per process list. */
#define VPE_NUM_FRAMELIST_PER_PLIST     (2U)

/* Number of FVID2 frame object to allocate per framelist. */
#define VPE_NUM_FRAMES_PER_LIST         (VPS_M2M_MAX_CH_PER_HANDLE)

/* Application name string used in print statements. */
#define APP_NAME                        "AppVpeScale"

/* Maximum number of buffer that would required to be submitted before the
 * first buffer is returned back */
#define VPE_NO_OF_BUF_REQ_BEFORE_RES    (3U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 *  \brief M2M VPE application object.
 */
typedef struct
{
    UInt32              drvInst;
    /**< M2M VPE driver instance. */
    UInt32              numCh;
    /**< Number of channels per handle. */

    UInt32              inMemType;
    /**< VPDMA Memory type for the VPE input frame. */
    UInt32              outMemType;
    /**< VPDMA Memory type for the VPE output frame. */

    UInt32              numInBuffers;
    /**< Number of input buffers. */
    UInt32              numOutBuffers;
    /**< Number of output buffers. */

    UInt32              inWidth;
    /**< Input width in pixels. */
    UInt32              inHeight;
    /**< Input height. */
    UInt32              inDrvWidth;
    /**< Input width in pixels to be programmed to driver - used in case
     *   of tiler rotation. */
    UInt32              inDrvHeight;
    /**< Input height to be programmed to driver - used in case
     *   of tiler rotation. */
    UInt32              inDataFmt;
    /**< VPE input data format. */

    UInt32              outWidth;
    /**< VPE output width in pixels. */
    UInt32              outHeight;
    /**< VPE output height. */
    UInt32              outDrvWidth;
    /**< Output width in pixels to be programmed to driver - used in case
     *   of tiler rotation. */
    UInt32              outDrvHeight;
    /**< Output height to be programmed to driver - used in case
     *   of tiler rotation. */
    UInt32              outDataFmt;
    /**< VPE output data format. */

    UInt32              inScanFmt;
    /**< VPE input scan format. */
    UInt32              outScanFmt;
    /**< VPE output scan format. */

    UInt32              inPitchY;
    /**< Luma input buffer pitch in bytes. */
    UInt32              inPitchCbCr;
    /**< Chroma input buffer pitch in bytes - used in semiplanar format. */
    UInt32              outPitchY;
    /**< Luma output buffer pitch in bytes. */
    UInt32              outPitchCbCr;
    /**< Chroma output buffer pitch in bytes - used in semiplanar format. */

    UInt32              inOriMode;
    /**< Input orientation flag. */
    UInt32              outOriMode;
    /**< VPE output orientation flag. */
    Bool                isInDimSwapped;
    /**< Flag to indicate whether rotation is enabled for input. */
    Bool                isOutDimSwapped;
    /**< Flag to indicate whether rotation is enabled for VPE output. */

    /* Below variables are defined so that the buffers are contiguous in memory
     * and so the user could load the buffer using CCS using one command.
     * This buffer will then be spilt in to multiple single buffers and assigned
     * to buffer pointers below. In case of tiler memory, the buffer pointers
     * are allocated separately one buffer at a time and then data is copied
     * from the load/save buffers using CPU so that tiler conversion is taken
     * care automatically. */
    UInt8              *inLoadBuf;
    /**< Input buffer used to load the input video frames. */
    UInt8              *outSaveBuf;
    /**< VPE writeback out buffer used to store the output video frames. */

    UInt8              *inBufY[VPE_MAX_IN_BUFFERS];
    /**< Input luma buffer. */
    UInt8              *inBufCbCr[VPE_MAX_IN_BUFFERS];
    /**< Input chroma buffer. */
    UInt8              *outBufY[VPE_MAX_OUT_BUFFERS];
    /**< Output luma buffer. */
    UInt8              *outBufCbCr[VPE_MAX_OUT_BUFFERS];
    /**< Output chroma buffer. */

    Fvid2_Handle        fvidHandle;
    /**< M2M VPE driver handle. */
    UInt32              inBufIdx;
    /**< Current input buffer index. Used to wrap around input buffer. */
    UInt32              outBufIdx;
    /**< Current output buffer index. Used to wrap around output buffer. */
    UInt32              fid;
    /**< Software FID. */
    UInt32              completedLoopCnt;
    /**< Total number of completed frames. */
    Semaphore_Handle    reqCompleteSem;
    /**< Request complete semaphore used to sync between callback and task. */
    UInt32              deiBypassFlag;
    /**< DEI bypass flag. */

    Fvid2_ProcessList   processList;
    /**< Process list used for queue/dequeue operations. */
    Fvid2_FrameList     frameList[VPE_NUM_FRAMELIST_PER_PLIST];
    /**< Frame list used for queue/dequeue operations. */
    Fvid2_Frame         frames[VPE_NUM_FRAMELIST_PER_PLIST]
    [VPE_NUM_FRAMES_PER_LIST];
    /**< Frames used for queue/dequeue operations. */

    Fvid2_ProcessList   errProcessList;
    /**< Process list used for error operations. */

    Vps_M2mCreateParams createPrms;
    /**< M2M VPE driver create parameters. */
    Vps_M2mVpeParams    vpePrms[VPS_M2M_MAX_CH_PER_HANDLE];
    /**< M2M VPE driver channel parameters for each of the channels. */
    Fvid2_Frame         cntxFrames[VPS_M2M_MAX_CH_PER_HANDLE]
    [VPE_NO_OF_BUF_REQ_BEFORE_RES];
    /**< Place holder for FVID2_frame containers */
    UInt32              freeCntxFrameIndex[VPS_M2M_MAX_CH_PER_HANDLE];
    /**< Counter to indicate used FRAME containers */

    Fvid2_SubFrameInfo  subFrameInfoIn[VPS_M2M_MAX_CH_PER_HANDLE];
    Fvid2_SubFrameInfo  subFrameInfoOut[VPS_M2M_MAX_CH_PER_HANDLE];
    /**< Memory for SubFrame level processing information exchange between
     *   application and driver.*/
} App_M2mVpeObj;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

Int32 App_m2mVpeAllocBuf(App_M2mVpeObj *appObj);
Int32 App_m2mVpeFreeBuf(App_M2mVpeObj *appObj);
void App_m2mVpeCopyToTilerMem(const App_M2mVpeObj *appObj);
void App_m2mVpeCopyFromTilerMem(const App_M2mVpeObj *appObj);
Int32 App_m2mVpeProgramScCoeff(App_M2mVpeObj *appObj);
Int32 App_m2mVpeSetFmdCfg(App_M2mVpeObj *appObj, UInt16 fmdEnable);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef M2MVPESCALE_UTILS_H_ */
