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
 *  \file vpsDctrl_displayApi.c
 *
 *  \brief File containing the VPS display driver APIs.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <xdc/std.h>
#include <common/bsp_types.h>
#include <common/trace.h>
#include <common/bsp_config.h>
#include <common/bsp_utils.h>
#include <fvid2/fvid2.h>
#include <fvid2_drvMgr.h>
#include <vps/vps.h>
#include <vps/vps_display.h>
#include <vps/vps_displayCtrl.h>
#include <vps/vps_cfgDss.h>
#include <common/vpscore.h>
#include <dispcore/vpscore_dss.h>
#include <dispcore/vpscore_dctrl.h>
#include <dispdrv/vpsdrv_dctrl.h>
#include <dispdrv/src/vpsdrv_dctrlPriv.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */
/**
 *  Below ifdef __cplusplus is added so that C++ build passes without
 *  typecasting. This is because the prototype is build as C type
 *  whereas this file is build as CPP file. Hence we get C++ build error.
 *  Also if tyecasting is used, then we get MisraC error Rule 11.1.
 */
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Display API's
 */
static Fdrv_Handle VpsDrv_dctrlCreate(UInt32                   drvId,
                                      UInt32                   instId,
                                      Ptr                      createArgs,
                                      Ptr                      createStatusArgs,
                                      const Fvid2_DrvCbParams *fdmCbPrms);
static Int32 VpsDrv_dctrlDelete(Fdrv_Handle handle, Ptr reserved);
static Int32 VpsDrv_dctrlControl(Fdrv_Handle handle,
                                 UInt32      cmd,
                                 Ptr         cmdArgs,
                                 Ptr         cmdStatusArgs);
#ifdef __cplusplus
}
#endif
static Int32 vpsDrvDctrlSetVencDivisorIoctl(
    VpsDrv_DctrlInstObj *instObj,
    Vps_DctrlVencDivisorInfo *
    vencDivisor);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

VpsDrv_DctrlCommonObj gVpsDctrlCommonObj;
VpsDrv_DctrlInstObj   gVpsDctrlInstObj;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

static Int32 vpsDrvDctrlSetConfigIoctl(VpsDrv_DctrlInstObj *instObj,
                                       Vps_DctrlConfig     *dctrlCfg);
static Int32 vpsDrvDctrlClearConfigIoctl(VpsDrv_DctrlInstObj *instObj,
                                         Vps_DctrlConfig     *dctrlCfg);
static Int32 vpsDrvDctrlSetVencOutputIoctl(VpsDrv_DctrlInstObj *instObj,
                                           Vps_DctrlOutputInfo *dctrl_info);
static Int32 vpsDrvDctrlGetVencOutputIoctl(VpsDrv_DctrlInstObj *instObj,
                                           Vps_DctrlOutputInfo *dctrl_info);
static Int32 vpsDrvDctrlSetOvlyParamsIoctl(
    VpsDrv_DctrlInstObj *instObj,
    Vps_DssDispcOvlyPanelConfig *
    panelconfig);
static Int32 vpsDrvDctrlSetPipelineParamsIoctl(
    VpsDrv_DctrlInstObj *instObj,
    Vps_DssDispcOvlyPipeConfig *
    ovlpipeconfig);

static Int32 vpsDrvDctrlSetAdvVencTdmParamsIoctl(
    VpsDrv_DctrlInstObj *instObj,
    Vps_DssDispcAdvLcdTdmConfig *
    advLcdTdmPrms);

static Int32 vpsDrvDctrlSetAdvVencDispParamsIoctl(
    VpsDrv_DctrlInstObj *instObj,
    Vps_DssDispcLcdAdvDisplayConfig *
    advLcdDispPrms);

static Int32 vpsDrvDctrlSetAdvVencSignalParamsIoctl(
    VpsDrv_DctrlInstObj *instObj,
    Vps_DssDispcLcdAdvSignalConfig *
    advVencSignalPrms);

static Int32 vpsDrvDctrlEnableSdVencIoctl(
    VpsDrv_DctrlInstObj *instObj,
    UInt32              *enable);

static Int32 vpsDrvDctrlSetSdVencModeIoctl(
    VpsDrv_DctrlInstObj          *instObj,
    Vps_DctrlSDVencVideoStandard *standard);

static Int32 vpsDrvDctrlSetSdVencAdvCfgIoctl(
    VpsDrv_DctrlInstObj *instObj,
    Vps_DssDispcSdVencAdvConfig *
    dispcSdVencConfig);

static Int32 vpsDrvDctrlGetDefSdVencAdvCfgIoctl(
    VpsDrv_DctrlInstObj *instObj,
    Vps_DssDispcSdVencAdvConfig *
    dispcSdVencConfig);

static Int32 vpsDrvDctrlGetErrorStats(VpsDrv_DctrlInstObj         *instObj,
                                      Vps_DctrlErrorSyncLostStats *errStats);

/**
 *  VpsDrv_dctrlInit
 *  \brief Display driver init function.
 *
 *  Initializes driver objects, allocates memory etc.
 *  This function should be called before calling any of display driver API.
 *  This should be called only once for all the instance together.
 *  This function could be called from task context only.
 *
 *  \param numInst          [IN] Number of instance objects to be initialized.
 *  \param initPrms         [IN] Pointer to the instance parameter containing
 *                          instance specific information. If the number of
 *                          instance is greater than 1, then this pointer
 *                          should point to an array of init parameter
 *                          structure of size numInst.
 *
 *  \return                 Returns FVID2_SOK on success else returns
 *                          appropriate error code.
 */
Int32 VpsDrv_dctrlInit(const VpsDrv_DctrlInitParams *initPrms)
{
    Int32 retVal = FVID2_SOK;
    VpsDrv_DctrlCommonObj *pObj;
    Int32 initValue;

    pObj = &gVpsDctrlCommonObj;
    BspUtils_memset(pObj, 0, sizeof (VpsDrv_DctrlCommonObj));

    /* Initialize instance object members */
    pObj->instObj = &gVpsDctrlInstObj;

    if (FVID2_SOK == retVal)
    {
        pObj->instObj->coreInstObj = initPrms->coreInstObj;
        pObj->instObj->coreOps     = initPrms->coreOps;

        /* Allocate instance semaphore */
        initValue = 1;
        pObj->instObj->lockSem = BspOsal_semCreate(initValue, FALSE);
        if (NULL == pObj->instObj->lockSem)
        {
            GT_0trace(VpsDrvDctrlTrace, GT_ERR,
                      "Instance semaphore create failed!!\n");
            retVal = FVID2_EALLOC;
        }

        pObj->instObj->state.isInit    = TRUE;
        pObj->instObj->state.isOpened  = FALSE;
        pObj->instObj->state.isStarted = FALSE;
    }

    if (FVID2_SOK == retVal)
    {
        /* Initialize Driver operations */
        Fvid2DrvOps_init(&pObj->fvidDrvOps);

        pObj->fvidDrvOps.drvId      = FVID2_VPS_DCTRL_DRV;
        pObj->fvidDrvOps.createFxn  = &VpsDrv_dctrlCreate;
        pObj->fvidDrvOps.deleteFxn  = &VpsDrv_dctrlDelete;
        pObj->fvidDrvOps.controlFxn = &VpsDrv_dctrlControl;

        retVal = Fvid2_registerDriver(&pObj->fvidDrvOps);
        if (FVID2_SOK != retVal)
        {
            GT_0trace(VpsDrvDctrlTrace, GT_ERR,
                      "Registering to FVID2 driver manager failed\n");
        }
        else
        {
            /* Init successful */
            pObj->isRegistered = (UInt32) TRUE;
        }
    }

    return (retVal);
}

/**
 *  VpsDrv_dctrlDeInit
 *  \brief Display driver exit function.
 *
 *  Deallocates memory allocated by init function. This should be called only
 *  once for all the instance together. This function could be called from
 *  task context only.
 *
 *  \param arg              [IN] Not used currently. Meant for future purpose.
 *                          Set this to NULL.
 *
 *  \return                 Returns FVID2_SOK on success else returns
 *                          appropriate error code.
 */
Int32 VpsDrv_dctrlDeInit(void)
{
    Int32 retVal = FVID2_SOK;
    VpsDrv_DctrlCommonObj *pObj;
    VpsDrv_DctrlInstObj   *instObj;
    pObj = &gVpsDctrlCommonObj;
    if (NULL != pObj->instObj)
    {
        instObj = pObj->instObj;
        if (instObj->state.isOpened == TRUE)
        {
            GT_0trace(VpsDrvDctrlTrace, GT_ERR,
                      "Can't deinit when an instance is active\n");
            retVal = FVID2_EFAIL;
        }
        /* Delete the instance semaphore */
        if (NULL != instObj->lockSem)
        {
            BspOsal_semDelete(&instObj->lockSem);
            instObj->lockSem = NULL;
        }

        instObj->state.isInit = FALSE;

        pObj->instObj = NULL;
    }

    if (TRUE == pObj->isRegistered)
    {
        /* Unregister from driver manager */
        retVal = Fvid2_unRegisterDriver(&pObj->fvidDrvOps);
        if (FVID2_SOK != retVal)
        {
            GT_0trace(VpsDrvDctrlTrace, GT_ERR,
                      "Unregistering from FVID2 driver manager failed\n");
        }
        pObj->isRegistered = (UInt32) FALSE;
    }

    return (retVal);
}

/**
 *  VpsDrv_dctrlCreate
 *  \brief Display driver create function. Returns the handle to the created
 *  driver instance.
 *
 *  This function could be called from task context only.
 *  And is reentrant - could be called from multiple tasks at the same time for
 *  creating different instances.
 *
 *  \param drvId            [IN] Driver ID - Should be FVID2_VPS_DISP_DRV.
 *  \param instId           [IN] Instance to open.
 *  \param createArgs       [IN] Display driver create parameter - pointer
 *                          to Vps_DispCreateParams. This parameter should be
 *                          non-NULL.
 *  \param createStatusArgs [OUT] Display driver return status parameter -
 *                          pointer to Vps_DispCreateStatus. This parameter
 *                          could be NULL and the driver fills the status
 *                          information only if this is not NULL.
 *  \param fdmCbParams      [IN] Call back function parameters. This parameter
 *                          should be non-NULL. But the callback function
 *                          pointer inside this structure could be NULL if the
 *                          application decides not to get display completion
 *                          callback. In this case application has to poll
 *                          the driver for knowing the status about display
 *                          completion.
 *
 *  \return                 Returns instance handle for the respective instance
 *                          ID else returns NULL on error. Appropriate error
 *                          code is returned via status argument.
 */
static Fdrv_Handle VpsDrv_dctrlCreate(UInt32                   drvId,
                                      UInt32                   instId,
                                      Ptr                      createArgs,
                                      Ptr                      createStatusArgs,
                                      const Fvid2_DrvCbParams *fdmCbPrms)
{
    Int32       retVal             = FVID2_SOK, tempRetVal;
    UInt32      coreCreateFlag     = FALSE;
    Fdrv_Handle drvHandle          = NULL;
    VpsDrv_DctrlInstObj   *instObj = NULL;
    VpsDrv_DctrlCommonObj *pObj;

    pObj = &gVpsDctrlCommonObj;

    GT_assert(VpsDrvDctrlTrace, (NULL != pObj));
    GT_assert(VpsDrvDctrlTrace, (NULL != pObj->instObj));

    instObj = pObj->instObj;

    if (NULL == instObj)
    {
        GT_0trace(VpsDrvDctrlTrace, GT_ERR, "Invalid instance ID\n");
        retVal = FVID2_EINVALID_PARAMS;
    }

    if (NULL != instObj)
    {
        /* Take the instance semaphore */
        BspOsal_semWait(instObj->lockSem, BSP_OSAL_WAIT_FOREVER);

        /* Check if the instance is already opened */
        if (TRUE == instObj->state.isOpened)
        {
            GT_0trace(VpsDrvDctrlTrace, GT_ERR,
                      "DEVICE_INUSE: Driver instance already created!!\n");
            retVal = FVID2_EDEVICE_INUSE;
        }
    }

    if (FVID2_SOK == retVal)
    {
        retVal = vpsDrvDctrlOpenCore(instObj);
        if (FVID2_SOK != retVal)
        {
            GT_0trace(VpsDrvDctrlTrace, GT_ERR, "Core open failed!!\n");
        }
        else
        {
            coreCreateFlag = (UInt32) TRUE;
        }
    }

    if (FVID2_SOK == retVal)
    {
        /* Initialize instance variables */
        instObj->state.isOpened = TRUE;
        drvHandle = instObj;
    }

    if (NULL != instObj)
    {
        /* Deallocate if error occurs */
        if (FVID2_SOK != retVal)
        {
            if ((UInt32) TRUE == coreCreateFlag)
            {
                tempRetVal = vpsDrvDctrlCloseCore(instObj);
                GT_assert(VpsDrvDctrlTrace, (FVID2_SOK == tempRetVal));
            }
        }

        /* Post the instance semaphore */
        BspOsal_semPost(instObj->lockSem);
    }

    return (drvHandle);
}

/**
 *  VpsDrv_dctrlDelete
 *  \brief Display driver delete function. Relinquish the resources allocated
 *  by create function.
 *
 *  This function could be called from task context only.
 *  And is reentrant - could be called from multiple tasks at the same time for
 *  deleting different instances.
 *
 *  \param handle           [IN] Display driver instance handle returned by
 *                          create function.
 *  \param deleteArgs       [IN] Not used currently. Meant for future purpose.
 *                          Set this to NULL.
 *
 *  \return                 Returns FVID2_SOK on success else returns
 *                          appropriate error code.
 */
static Int32 VpsDrv_dctrlDelete(Fdrv_Handle handle, Ptr reserved)
{
    Int32 retVal = FVID2_SOK;

    VpsDrv_DctrlInstObj *instObj = NULL;

    /* Check for NULL pointers and invalid arguments */
    if (NULL == handle)
    {
        GT_0trace(VpsDrvDctrlTrace, GT_ERR, "Invalid arguments\n");
        retVal = FVID2_EBADARGS;
    }
    else
    {
        instObj = (VpsDrv_DctrlInstObj *) handle;
    }

    if (NULL != instObj)
    {
        /* Take the instance semaphore */
        BspOsal_semWait(instObj->lockSem, BSP_OSAL_WAIT_FOREVER);

        /* Check if already opened. */
        if (TRUE != instObj->state.isOpened)
        {
            GT_0trace(VpsDrvDctrlTrace, GT_ERR, "ERROR: Driver not opened\n");
            retVal = FVID2_EFAIL;
        }
    }

    if (FVID2_SOK == retVal)
    {
        retVal = vpsDrvDctrlCloseCore(instObj);
        GT_assert(VpsDrvDctrlTrace, (FVID2_SOK == retVal));

        /* Reset other variables */
        instObj->state.isOpened  = FALSE;
        instObj->state.isStarted = FALSE;
    }

    if (NULL != instObj)
    {
        /* Post the instance semaphore */
        BspOsal_semPost(instObj->lockSem);
    }

    return (retVal);
}

/**
 *  VpsDrv_dispControl
 *  \brief Display driver control function. This function implements various
 *  IOCTLS.
 *
 *  All IOCTLS are blocking and could not be called from ISR context unless
 *  otherwise specified for a particular IOCTL.
 *
 *  cmd             : FVID2_START
 *  cmdArgs         : NULL
 *  cmdStatusArgs   : NULL
 *  Description     : Starts the display. At least minimum number of buffers
 *                    should be queued before calling start IOCTL.
 *
 *  cmd             : FVID2_STOP
 *  cmdArgs         : NULL
 *  cmdStatusArgs   : NULL
 *  Description     : Stops the display
 *
 *  cmd             : IOCTL_VPS_DISP_GET_STATUS
 *  cmdArgs         : Vps_DispStatus *
 *  cmdStatusArgs   : NULL
 *  Description     : Gets the current display status information.
 *
 *  \param handle           [IN] Display driver instance handle returned by
 *                          create function.
 *  \param cmd              [IN] Supports the above mentioned commands.
 *  \param cmdArgs          [IN] Depending on the command this will vary.
 *  \param cmdStatusArgs    [OUT] Depending on the command this will vary.
 *
 *  \return                 Returns FVID2_SOK on success else returns
 *                          appropriate error code.
 */
static Int32 VpsDrv_dctrlControl(Fdrv_Handle handle,
                                 UInt32      cmd,
                                 Ptr         cmdArgs,
                                 Ptr         cmdStatusArgs)
{
    Int32 retVal = FVID2_SOK;
    VpsDrv_DctrlInstObj *instObj;

    /* Check for NULL pointers */
    if (NULL == handle)
    {
        GT_0trace(VpsDrvDctrlTrace, GT_ERR, "Invalid argument!!\n");
        retVal = FVID2_EBADARGS;
    }
    else
    {
        instObj = (VpsDrv_DctrlInstObj *) handle;
    }

    if (FVID2_SOK == retVal)
    {
        switch (cmd)
        {
            case IOCTL_VPS_DCTRL_SET_CONFIG:
                retVal = vpsDrvDctrlSetConfigIoctl(
                    instObj,
                    (Vps_DctrlConfig *) cmdArgs);
                break;
            case IOCTL_VPS_DCTRL_CLEAR_CONFIG:
                retVal = vpsDrvDctrlClearConfigIoctl(
                    instObj,
                    (Vps_DctrlConfig *) cmdArgs);
                break;
            case IOCTL_VPS_DCTRL_SET_VENC_OUTPUT:
                retVal = vpsDrvDctrlSetVencOutputIoctl(
                    instObj,
                    (Vps_DctrlOutputInfo *) cmdArgs);
                break;
            case IOCTL_VPS_DCTRL_GET_VENC_OUTPUT:
                retVal = vpsDrvDctrlGetVencOutputIoctl(
                    instObj,
                    (Vps_DctrlOutputInfo *) cmdArgs);
                break;
            case IOCTL_VPS_DCTRL_SET_OVLY_PARAMS:
                retVal = vpsDrvDctrlSetOvlyParamsIoctl(
                    instObj,
                    (Vps_DssDispcOvlyPanelConfig *) cmdArgs);
                break;
            case IOCTL_VPS_DCTRL_SET_PIPELINE_PARAMS:
                retVal = vpsDrvDctrlSetPipelineParamsIoctl(
                    instObj,
                    (Vps_DssDispcOvlyPipeConfig *) cmdArgs);
                break;
            case IOCTL_VPS_DCTRL_SET_VENC_PCLK_DIVISORS:
                retVal = vpsDrvDctrlSetVencDivisorIoctl(
                    instObj,
                    (Vps_DctrlVencDivisorInfo *) cmdArgs);
                break;
            case IOCTL_VPS_DCTRL_DSS_SET_ADV_VENC_TDM_PARAMS:
                retVal = vpsDrvDctrlSetAdvVencTdmParamsIoctl(
                    instObj,
                    (Vps_DssDispcAdvLcdTdmConfig *) cmdArgs);
                break;
            case IOCTL_VPS_DCTRL_DSS_SET_ADV_VENC_DISP_PARAMS:
                retVal = vpsDrvDctrlSetAdvVencDispParamsIoctl(
                    instObj,
                    (Vps_DssDispcLcdAdvDisplayConfig *) cmdArgs);
                break;
            case IOCTL_VPS_DCTRL_DSS_SET_ADV_VENC_SIGNAL_PARAMS:
                retVal = vpsDrvDctrlSetAdvVencSignalParamsIoctl(
                    instObj,
                    (Vps_DssDispcLcdAdvSignalConfig *) cmdArgs);
                break;
            case IOCTL_VPS_DCTRL_ENABLE_SDVENC:
                retVal = vpsDrvDctrlEnableSdVencIoctl(
                    instObj,
                    (UInt32 *) cmdArgs);
                break;
            case IOCTL_VPS_DCTRL_SET_SDVENC_MODE:
                retVal = vpsDrvDctrlSetSdVencModeIoctl(
                    instObj,
                    (Vps_DctrlSDVencVideoStandard *) cmdArgs);
                break;
            case IOCTL_VPS_DCTRL_GET_ERROR_STATS:
                retVal = vpsDrvDctrlGetErrorStats(
                    instObj,
                    (Vps_DctrlErrorSyncLostStats *) cmdArgs);
                break;
            case IOCTL_VPS_DCTRL_SDVENC_GET_DEF_ADV_CONFIG:
                retVal = vpsDrvDctrlGetDefSdVencAdvCfgIoctl(
                    instObj,
                    (Vps_DssDispcSdVencAdvConfig *) cmdArgs);
                break;
            case IOCTL_VPS_DCTRL_SDVENC_SET_ADV_CONFIG:
                retVal = vpsDrvDctrlSetSdVencAdvCfgIoctl(
                    instObj,
                    (Vps_DssDispcSdVencAdvConfig *) cmdArgs);
                break;
            default:
                GT_0trace(VpsDrvDctrlTrace, GT_ERR,
                          "UNSUPPORTED_CMD: IOCTL not supported\n");
                retVal = FVID2_EUNSUPPORTED_CMD;
                break;
        }
    }

    return (retVal);
}

/**
 *  vpsDrvDctrlSetConfigIoctl
 *  \brief Gets the current display status information.
 *
 *  NOTE: This can't be be called from ISR context.
 *
 *  Returns FVID2_SOK on success else returns appropriate error code.
 */
static Int32 vpsDrvDctrlSetConfigIoctl(VpsDrv_DctrlInstObj *instObj,
                                       Vps_DctrlConfig     *dctrlCfg)

{
    Int32          retVal = FVID2_SOK;
    Fvid2_ModeInfo mInfo;
    UInt32         i;

    if (NULL == instObj)
    {
        GT_0trace(VpsDrvDctrlTrace, GT_ERR, "Invalid argument!!\n");
        retVal = FVID2_EBADARGS;
    }

    /* Check for NULL pointers */
    GT_assert(VpsDrvDctrlTrace, (NULL != dctrlCfg));

    for (i = 0; i < dctrlCfg->vencInfo.numVencs; i++)
    {
        if (FVID2_STD_CUSTOM != dctrlCfg->vencInfo.modeInfo[i].mInfo.standard)
        {
            /* Get the timing from FVID2 layer */
            mInfo.standard = dctrlCfg->vencInfo.modeInfo[i].mInfo.standard;
            retVal        += Fvid2_getModeInfo(&mInfo);

            if (retVal == FVID2_SOK)
            {
                BspUtils_memcpy(
                    &dctrlCfg->vencInfo.modeInfo[i].mInfo,
                    &mInfo,
                    sizeof (mInfo));
            }
        }
    }
    retVal = instObj->coreOps->control(
        instObj,
        IOCTL_VPSCORE_DCTRL_SET_CONFIG,
        dctrlCfg,
        (Ptr) instObj->coreInstObj);
    if (FVID2_SOK != retVal)
    {
        GT_0trace(VpsDrvCaptTrace, GT_ERR,
                  "Core control: Set Config failed!!\n");
    }

    return (retVal);
}

/**
 *  vpsDrvDctrlCleatConfigIoctl
 *  \brief Gets the current display status information.
 *
 *  NOTE: This can't be be called from ISR context.
 *
 *  Returns FVID2_SOK on success else returns appropriate error code.
 */
static Int32 vpsDrvDctrlClearConfigIoctl(VpsDrv_DctrlInstObj *instObj,
                                         Vps_DctrlConfig     *dctrlCfg)

{
    Int32 retVal = FVID2_SOK;

    /* Check for NULL pointers */
    GT_assert(VpsDrvDctrlTrace, (NULL != dctrlCfg));

    retVal = instObj->coreOps->control(
        instObj,
        IOCTL_VPSCORE_DCTRL_CLEAR_CONFIG,
        dctrlCfg,
        (Ptr) instObj->coreInstObj);
    if (FVID2_SOK != retVal)
    {
        GT_0trace(VpsDrvCaptTrace, GT_ERR,
                  "Core control: clear config failed!!\n");
    }

    return (retVal);
}

/**
 *  vpsDrvDctrlSetVencOutputIoctl
 *  \brief Gets the current display status information.
 *
 *  NOTE: This can't be be called from ISR context.
 *
 *  Returns FVID2_SOK on success else returns appropriate error code.
 */
static Int32 vpsDrvDctrlSetVencOutputIoctl(VpsDrv_DctrlInstObj *instObj,
                                           Vps_DctrlOutputInfo *dctrl_info)

{
    Int32 retVal = FVID2_SOK;

    /* Check for NULL pointers */
    GT_assert(VpsDrvDctrlTrace, (NULL != dctrl_info));

    retVal = instObj->coreOps->control(
        instObj,
        IOCTL_VPSCORE_DCTRL_SET_VENC_OUTPUT,
        dctrl_info,
        (Ptr) instObj->coreInstObj);
    if (FVID2_SOK != retVal)
    {
        GT_0trace(VpsDrvCaptTrace, GT_ERR,
                  "Core control: Set venc output failed!!\n");
    }

    return (retVal);
}

/**
 *  vpsDrvDctrlGetVencOutputIoctl
 *  \brief Gets the current display status information.
 *
 *  NOTE: This can't be be called from ISR context.
 *
 *  Returns FVID2_SOK on success else returns appropriate error code.
 */
static Int32 vpsDrvDctrlGetVencOutputIoctl(VpsDrv_DctrlInstObj *instObj,
                                           Vps_DctrlOutputInfo *dctrl_info)

{
    Int32 retVal = FVID2_SOK;

    /* Check for NULL pointers */
    GT_assert(VpsDrvDctrlTrace, (NULL != dctrl_info));

    retVal = instObj->coreOps->control(
        instObj,
        IOCTL_VPSCORE_DCTRL_GET_VENC_OUTPUT,
        dctrl_info,
        (Ptr) instObj->coreInstObj);
    if (FVID2_SOK != retVal)
    {
        GT_0trace(VpsDrvCaptTrace, GT_ERR,
                  "Core control: Get venc output failed!!\n");
    }

    return (retVal);
}

/**
 *  vpsDrvDctrlSetAdvVencTdmParamsIoctl
 *  \brief Sets the Venc TDM params.
 *
 *  NOTE: This can't be be called from ISR context.
 *
 *  Returns FVID2_SOK on success else returns appropriate error code.
 */
static Int32 vpsDrvDctrlSetAdvVencTdmParamsIoctl(
    VpsDrv_DctrlInstObj *instObj,
    Vps_DssDispcAdvLcdTdmConfig *
    advLcdTdmPrms)

{
    Int32 retVal = FVID2_SOK;

    /* Check for NULL pointers */
    GT_assert(VpsDrvDctrlTrace, (NULL != advLcdTdmPrms));

    retVal = instObj->coreOps->control(
        instObj,
        IOCTL_VPSCORE_DCTRL_SET_ADV_VENC_TDM_PARAMS,
        advLcdTdmPrms,
        (Ptr) instObj->coreInstObj);
    if (FVID2_SOK != retVal)
    {
        GT_0trace(VpsDrvCaptTrace, GT_ERR,
                  "Core control: Set panelconfig failed!!\n");
    }

    return (retVal);
}

/**
 *  vpsDrvDctrlSetAdvVencDispParamsIoctl
 *  \brief Sets the Venc Adv Display params.
 *
 *  NOTE: This can't be called from ISR context.
 *
 *  Returns FVID2_SOK on success else returns appropriate error code.
 */
static Int32 vpsDrvDctrlSetAdvVencDispParamsIoctl(
    VpsDrv_DctrlInstObj *instObj,
    Vps_DssDispcLcdAdvDisplayConfig *
    advLcdDispPrms)

{
    Int32 retVal = FVID2_SOK;

    /* Check for NULL pointers */
    GT_assert(VpsDrvDctrlTrace, (NULL != advLcdDispPrms));

    retVal = instObj->coreOps->control(
        instObj,
        IOCTL_VPSCORE_DCTRL_SET_ADV_VENC_DISP_PARAMS,
        advLcdDispPrms,
        (Ptr) instObj->coreInstObj);
    if (FVID2_SOK != retVal)
    {
        GT_0trace(VpsDrvCaptTrace, GT_ERR,
                  "Core control: Set panelconfig failed!!\n");
    }

    return (retVal);
}

/**
 *  vpsDrvDctrlSetAdvVencSignalParamsIoctl
 *  \brief Sets the Venc Adv Signal params.
 *
 *  NOTE: This can't be called from ISR context.
 *
 *  Returns FVID2_SOK on success else returns appropriate error code.
 */
static Int32 vpsDrvDctrlSetAdvVencSignalParamsIoctl(
    VpsDrv_DctrlInstObj *instObj,
    Vps_DssDispcLcdAdvSignalConfig *
    advVencSignalPrms)

{
    Int32 retVal = FVID2_SOK;

    /* Check for NULL pointers */
    GT_assert(VpsDrvDctrlTrace, (NULL != advVencSignalPrms));

    retVal = instObj->coreOps->control(
        instObj,
        IOCTL_VPSCORE_DCTRL_SET_ADV_VENC_SIGNAL_PARAMS,
        advVencSignalPrms,
        (Ptr) instObj->coreInstObj);
    if (FVID2_SOK != retVal)
    {
        GT_0trace(VpsDrvCaptTrace, GT_ERR,
                  "Core control: Set panelconfig failed!!\n");
    }

    return (retVal);
}

/**
 *  vpsDrvDctrlSetSdVencModeIoctl
 *  \brief Configures the SDVENC to output a particular video mode(NTSC/PAL).
 *
 *  NOTE: This can't be be called from ISR context.
 *
 *  Returns FVID2_SOK on success else returns appropriate error code.
 */
static Int32 vpsDrvDctrlSetSdVencModeIoctl(
    VpsDrv_DctrlInstObj *instObj,
    Vps_DctrlSDVencVideoStandard *
    standard)
{
    Int32 retVal = FVID2_SOK;

    /* Check for NULL pointers */
    GT_assert(VpsDrvDctrlTrace, (NULL != standard));

    retVal = instObj->coreOps->control(
        instObj,
        IOCTL_VPSCORE_DCTRL_SET_SDVENC_MODE,
        (Void *) standard,
        (Ptr) instObj->coreInstObj);
    if (FVID2_SOK != retVal)
    {
        GT_0trace(VpsDrvCaptTrace, GT_ERR,
                  "Core control: Set SDVENC Mode failed!!\n");
    }

    return (retVal);
}

/**
 *  vpsDrvDctrlGetSdVencDefAdvCfgIoctl
 *  \brief Get the default parameters for sd venc adv config object.
 *
 *  Returns FVID2_SOK on success else returns appropriate error code.
 */
static Int32 vpsDrvDctrlGetDefSdVencAdvCfgIoctl(
    VpsDrv_DctrlInstObj *instObj,
    Vps_DssDispcSdVencAdvConfig *
    dispcSdVencConfig)
{
    Int32 retVal = FVID2_SOK;

    /* Check for NULL pointers */
    GT_assert(VpsDrvDctrlTrace, (NULL != dispcSdVencConfig));

    retVal = instObj->coreOps->control(
        instObj,
        IOCTL_VPSCORE_DCTRL_GET_DEF_SDVENC_ADV_CONFIG,
        dispcSdVencConfig,
        NULL);
    if (FVID2_SOK != retVal)
    {
        GT_0trace(VpsDrvCaptTrace, GT_ERR,
                  "Core control: Set SDVENC Mode failed!!\n");
    }

    return (retVal);
}

/**
 *  vpsDrvDctrlSetSdVencModeIoctl
 *  \brief Configures the SDVENC using user passed configuration structure.
 *
 *  NOTE: This can't be be called from ISR context.
 *
 *  Returns FVID2_SOK on success else returns appropriate error code.
 */
static Int32 vpsDrvDctrlSetSdVencAdvCfgIoctl(
    VpsDrv_DctrlInstObj *instObj,
    Vps_DssDispcSdVencAdvConfig *
    dispcSdVencConfig)
{
    Int32 retVal = FVID2_SOK;

    /* Check for NULL pointers */
    GT_assert(VpsDrvDctrlTrace, (NULL != dispcSdVencConfig));

    retVal = instObj->coreOps->control(
        instObj,
        IOCTL_VPSCORE_DCTRL_SET_SDVENC_ADV_CONFIG,
        dispcSdVencConfig,
        NULL);
    if (FVID2_SOK != retVal)
    {
        GT_0trace(VpsDrvCaptTrace, GT_ERR,
                  "Core control: Set SDVENC Mode failed!!\n");
    }

    return (retVal);
}

/**
 *  vpsDrvDctrlGetErrorStats
 *  \brief Gets the sync lost and ocp error stats of applicable overlays
 *
 *  NOTE: This can't be be called from ISR context.
 *
 *  Returns FVID2_SOK on success else returns appropriate error code.
 */
static Int32 vpsDrvDctrlGetErrorStats(VpsDrv_DctrlInstObj         *instObj,
                                      Vps_DctrlErrorSyncLostStats *errStats)
{
    Int32 retVal = FVID2_SOK;

    /* Check for NULL pointers */
    GT_assert(VpsDrvDctrlTrace, (NULL != errStats));

    retVal = instObj->coreOps->control(
        instObj,
        IOCTL_VPSCORE_DCTRL_GET_ERROR_STATS,
        (Void *) errStats,
        (Ptr) instObj->coreInstObj);
    if (FVID2_SOK != retVal)
    {
        GT_0trace(VpsDrvCaptTrace, GT_ERR,
                  "Core control: Get Error Stats IOCTL failed!!\n");
    }

    return (retVal);
}

/**
 *  vpsDrvDctrlEnableSdVencIoctl
 *  \brief Enables/Disables the SDVENC.
 *
 *  NOTE: This can't be be called from ISR context.
 *
 *  Returns FVID2_SOK on success else returns appropriate error code.
 */
static Int32 vpsDrvDctrlEnableSdVencIoctl(VpsDrv_DctrlInstObj *instObj,
                                          UInt32              *enable)
{
    Int32 retVal = FVID2_SOK;

    /* Check for NULL pointers */
    GT_assert(VpsDrvDctrlTrace, (NULL != enable));

    retVal = instObj->coreOps->control(
        instObj,
        IOCTL_VPSCORE_DCTRL_ENABLE_SDVENC,
        enable,
        (Ptr) instObj->coreInstObj);
    if (FVID2_SOK != retVal)
    {
        GT_0trace(VpsDrvCaptTrace, GT_ERR,
                  "Core control: Enable/Disable SDVENC failed!!\n");
    }

    return (retVal);
}

/**
 *  vpsDrvDctrlSetVencDivisorIoctl
 *  \brief Sets the Venc divisor values to get desired pixel clock.
 *
 *  NOTE: This can't be be called from ISR context.
 *
 *  Returns FVID2_SOK on success else returns appropriate error code.
 */
static Int32 vpsDrvDctrlSetVencDivisorIoctl(
    VpsDrv_DctrlInstObj *instObj,
    Vps_DctrlVencDivisorInfo *
    vencDivisor)

{
    Int32 retVal = FVID2_SOK;

    /* Check for NULL pointers */
    GT_assert(VpsDrvDctrlTrace, (NULL != vencDivisor));

    retVal = instObj->coreOps->control(
        instObj,
        IOCTL_VPSCORE_DCTRL_SET_VENC_PCLK_DIVISORS,
        vencDivisor,
        (Ptr) instObj->coreInstObj);
    if (FVID2_SOK != retVal)
    {
        GT_0trace(VpsDrvCaptTrace, GT_ERR,
                  "Core control: Set panelconfig failed!!\n");
    }

    return (retVal);
}

/**
 *  vpsDrvDctrlSetOvlyParmsIoctl
 *  \brief Gets the current display status information.
 *
 *  NOTE: This can't be be called from ISR context.
 *
 *  Returns FVID2_SOK on success else returns appropriate error code.
 */
static Int32 vpsDrvDctrlSetOvlyParamsIoctl(
    VpsDrv_DctrlInstObj *instObj,
    Vps_DssDispcOvlyPanelConfig *
    panelconfig)

{
    Int32 retVal = FVID2_SOK;

    /* Check for NULL pointers */
    GT_assert(VpsDrvDctrlTrace, (NULL != panelconfig));

    retVal = instObj->coreOps->control(
        instObj,
        IOCTL_VPSCORE_DCTRL_SET_OVLY_PARAMS,
        panelconfig,
        (Ptr) instObj->coreInstObj);
    if (FVID2_SOK != retVal)
    {
        GT_0trace(VpsDrvCaptTrace, GT_ERR,
                  "Core control: Set panelconfig failed!!\n");
    }

    return (retVal);
}

/**
 *  vpsDrvDctrlSetPipelineParmsIoctl
 *  \brief Gets the current display status information.
 *
 *  NOTE: This can't be be called from ISR context.
 *
 *  Returns FVID2_SOK on success else returns appropriate error code.
 */
static Int32 vpsDrvDctrlSetPipelineParamsIoctl(
    VpsDrv_DctrlInstObj        *instObj,
    Vps_DssDispcOvlyPipeConfig *ovlpipeconfig)

{
    Int32 retVal = FVID2_SOK;

    /* Check for NULL pointers */
    GT_assert(VpsDrvDctrlTrace, (NULL != ovlpipeconfig));

    retVal = instObj->coreOps->control(
        instObj,
        IOCTL_VPSCORE_DCTRL_SET_PIPELINE_PARAMS,
        ovlpipeconfig,
        (Ptr) instObj->coreInstObj);

    if (FVID2_SOK != retVal)
    {
        GT_0trace(VpsDrvCaptTrace, GT_ERR,
                  "Core control: Set pipelineconfig failed!!\n");
    }

    return (retVal);
}

