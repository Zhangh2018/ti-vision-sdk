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
 *  \file bspdrv_adv7611Api.c
 *
 *  \brief ADV7611 video decoder FVID2 driver API file.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <adv7611/src/bspdrv_adv7611Priv.h>

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
static Fdrv_Handle Bsp_adv7611Create(UInt32                   drvId,
                                     UInt32                   instId,
                                     Ptr                      createPrms,
                                     Ptr                      createStatusArgs,
                                     const Fvid2_DrvCbParams *fdmCbPrms);
static Int32 Bsp_adv7611Delete(Fdrv_Handle handle, Ptr deleteArgs);
static Int32 Bsp_adv7611Control(Fdrv_Handle handle,
                                UInt32      cmd,
                                Ptr         cmdArgs,
                                Ptr         cmdStatusArgs);
#ifdef __cplusplus
}
#endif
static Bsp_Adv7611HandleObj *Bsp_adv7611AllocObj(void);
static Int32 Bsp_adv7611FreeObj(Bsp_Adv7611HandleObj *pObj);

static Int32 Bsp_adv7611LockObj(Bsp_Adv7611HandleObj *pObj);
static Int32 Bsp_adv7611UnlockObj(Bsp_Adv7611HandleObj *pObj);
static Int32 Bsp_adv7611Lock(void);
static Int32 Bsp_adv7611Unlock(void);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/** \brief ADV7611 driver function pointer. */
static const Fvid2_DrvOps gBspAdv7611DrvOps =
{
    FVID2_VID_DEC_ADV7611_DRV,          /* Driver ID */
    &Bsp_adv7611Create,                 /* Create */
    &Bsp_adv7611Delete,                 /* Delete */
    &Bsp_adv7611Control,                /* Control */
    NULL,                               /* Queue */
    NULL,                               /* Dequeue */
    NULL,                               /* ProcessFrames */
    NULL,                               /* GetProcessedFrames */
    NULL,                               /* ProcessRequest */
    NULL                                /* GetProcessedRequest */
};

/**
 *  \brief Global object storing all information related to all ADV7611 driver
 *  handles.
 */
static Bsp_Adv7611Obj     gBspAdv7611Obj;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/**
 *  \brief System init for ADV7611 driver
 *
 *  This API
 *      - create semaphore locks needed
 *      - registers driver to FVID2 sub-system
 *      - gets called as part of Bsp_deviceInit()
 *
 *  \return                 Returns FVID2_SOK on success else returns
 *                          appropriate error code.
 */
Int32 Bsp_adv7611Init(void)
{
    Int32 retVal = FVID2_SOK;
    Int32 initValue;

    /* Memset global object */
    BspUtils_memset(&gBspAdv7611Obj, 0, sizeof (gBspAdv7611Obj));

    /* Create global ADV7611 lock */
    initValue           = (Int32) 1;
    gBspAdv7611Obj.lock = BspOsal_semCreate(initValue, (Bool) TRUE);
    if (NULL == gBspAdv7611Obj.lock)
    {
        GT_0trace(BspDeviceTrace, GT_ERR, "Global semaphore create failed\n");
        retVal = FVID2_EALLOC;
    }

    if (FVID2_SOK == retVal)
    {
        /* Register ADV7611 driver with FVID2 sub-system */
        retVal = Fvid2_registerDriver(&gBspAdv7611DrvOps);
        if (FVID2_SOK != retVal)
        {
            GT_0trace(BspDeviceTrace, GT_ERR,
                      "Registering to FVID2 driver manager failed\n");
            /* Error - free acquired resources */
            GT_assert(BspDeviceTrace, (NULL != gBspAdv7611Obj.lock));
            BspOsal_semDelete(&gBspAdv7611Obj.lock);
        }
    }

    return (retVal);
}

/**
 *  \brief System de-init for ADV7611 driver
 *
 *  This API
 *      - de-registers driver from FVID2 sub-system
 *      - delete's allocated semaphore locks
 *      - gets called as part of Bsp_deviceDeInit()
 *
 *  \return                 Returns FVID2_SOK on success else returns
 *                          appropriate error code.
 */
Int32 Bsp_adv7611DeInit(void)
{
    /* Unregister FVID2 driver */
    Fvid2_unRegisterDriver(&gBspAdv7611DrvOps);

    /* Delete semaphore's. */
    GT_assert(BspDeviceTrace, (NULL != gBspAdv7611Obj.lock));
    BspOsal_semDelete(&gBspAdv7611Obj.lock);

    return (FVID2_SOK);
}

/**
 *  \brief ADV7611 create API that gets called when Fvid2_create is called.
 *
 *  This API does not configure the ADV7611 is any way.
 *
 *  This API
 *      - validates parameters
 *      - allocates driver handle
 *      - stores create arguments in its internal data structure.
 *
 *  Later the create arguments will be used when doing I2C communcation with
 *  ADV7611.
 *
 *  \param drvId            [IN] Driver ID, must be
 *                          FVID2_VID_DEC_ADV7611_DRV.
 *  \param instId           [IN] Must be 0.
 *  \param createPrms       [IN] Create arguments - pointer to valid
 *                          Bsp_VidDecCreateParams. This parameter should
 *                          be non-NULL.
 *  \param createStatusArgs [OUT] ADV7611 driver return parameter -
 *                          pointer to Bsp_VidDecCreateStatus.
 *                          This parameter could be NULL and the driver fills
 *                          the retVal information only if this is not NULL.
 *  \param fdmCbPrms        [IN] Not used. Set to NULL
 *
 *  \return                 Returns NULL in case of any error. Otherwise
 *                          returns a valid handle.
 */
static Fdrv_Handle Bsp_adv7611Create(UInt32                   drvId,
                                     UInt32                   instId,
                                     Ptr                      createPrms,
                                     Ptr                      createStatusArgs,
                                     const Fvid2_DrvCbParams *fdmCbPrms)
{
    Int32 retVal = FVID2_SOK;
    Bsp_Adv7611HandleObj   *pObj = NULL;
    Bsp_VidDecCreateParams *vidDecCreatePrms;
    Bsp_VidDecCreateStatus *vidDecCreateStatus;

    /* Check parameters */
    if ((NULL == createPrms) ||
        (drvId != FVID2_VID_DEC_ADV7611_DRV) ||
        (instId != 0U))
    {
        GT_0trace(BspDeviceTrace, GT_ERR, "Null pointer/Invalid parameters\n");
        retVal = FVID2_EBADARGS;
    }

    if (FVID2_SOK == retVal)
    {
        vidDecCreatePrms = (Bsp_VidDecCreateParams *) createPrms;
        if (vidDecCreatePrms->deviceI2cInstId >= BSP_DEVICE_I2C_INST_ID_MAX)
        {
            GT_0trace(BspDeviceTrace, GT_ERR, "Invalid I2C instance ID\n");
            retVal = FVID2_EINVALID_PARAMS;
        }
    }

    if (FVID2_SOK == retVal)
    {
        /* Allocate driver handle */
        pObj = Bsp_adv7611AllocObj();
        if (NULL == pObj)
        {
            GT_0trace(BspDeviceTrace, GT_ERR, "Alloc object failed\n");
            retVal = FVID2_EALLOC;
        }
    }

    if (FVID2_SOK == retVal)
    {
        /* Copy parameters to allocate driver handle */
        BspUtils_memcpy(
            &pObj->createPrms,
            vidDecCreatePrms,
            sizeof (*vidDecCreatePrms));
    }

    /* Fill the retVal if possible */
    if (NULL != createStatusArgs)
    {
        vidDecCreateStatus =
            (Bsp_VidDecCreateStatus *) createStatusArgs;
        vidDecCreateStatus->retVal = retVal;
    }

    return (pObj);
}

/**
 *  \brief Delete function that is called when Fvid2_delete is called.
 *
 *  This API
 *      - free's driver handle object
 *
 *  \param handle           [IN] Driver handle.
 *  \param deleteArgs       [IN] Not used currently. Meant for future purpose.
 *                          Set this to NULL.
 *
 *  \return                 Returns FVID2_SOK on success else returns
 *                          appropriate error code. *
 */
static Int32 Bsp_adv7611Delete(Fdrv_Handle handle, Ptr deleteArgs)
{
    Int32 retVal = FVID2_SOK;
    Bsp_Adv7611HandleObj *pObj;

    /* Check parameters */
    if (NULL == handle)
    {
        GT_0trace(BspDeviceTrace, GT_ERR, "Null pointer\n");
        retVal = FVID2_EBADARGS;
    }

    if (FVID2_SOK == retVal)
    {
        /* Free driver handle object */
        pObj   = (Bsp_Adv7611HandleObj *) handle;
        retVal = Bsp_adv7611FreeObj(pObj);
        if (FVID2_SOK != retVal)
        {
            GT_0trace(BspDeviceTrace, GT_ERR, "Free object failed\n");
        }
    }

    return (retVal);
}

/**
 *  \brief Control API that gets called when Fvid2_control is called.
 *
 *  This API does handle level semaphore locking
 *
 *  \param handle           [IN] ADV7611 driver handle returned by
 *                          create function.
 *  \param cmd              [IN] Supports the standard video decoder interface
 *                          commands.
 *  \param cmdArgs          [IN] Depending on the command this will vary.
 *  \param cmdStatusArgs    [OUT] Depending on the command this will vary.
 *
 *  \return                 Returns FVID2_SOK on success else returns
 *                          appropriate error codes for illegal parameters and
 *                          I2C command RX/TX error.
 */
static Int32 Bsp_adv7611Control(Fdrv_Handle handle,
                                UInt32      cmd,
                                Ptr         cmdArgs,
                                Ptr         cmdStatusArgs)
{
    Int32 retVal = FVID2_SOK;
    Bsp_Adv7611HandleObj *pObj = NULL;

    /* Check parameters */
    if (NULL == handle)
    {
        GT_0trace(BspDeviceTrace, GT_ERR, "Null pointer\n");
        retVal = FVID2_EBADARGS;
    }

    if (FVID2_SOK == retVal)
    {
        pObj = (Bsp_Adv7611HandleObj *) handle;

        /* lock handle */
        Bsp_adv7611LockObj(pObj);

        switch (cmd)
        {
            case IOCTL_BSP_VID_DEC_RESET:
                retVal = Bsp_adv7611Reset(pObj);
                break;

            case IOCTL_BSP_VID_DEC_GET_CHIP_ID:
                retVal = Bsp_adv7611GetChipId(
                    pObj,
                    (Bsp_VidDecChipIdParams *) cmdArgs,
                    (Bsp_VidDecChipIdStatus *) cmdStatusArgs);
                break;

            case IOCTL_BSP_VID_DEC_SET_VIDEO_MODE:
                retVal = Bsp_adv7611SetVideoMode(
                    pObj,
                    (Bsp_VidDecVideoModeParams *) cmdArgs);
                break;

            case IOCTL_BSP_VID_DEC_GET_VIDEO_STATUS:
                retVal = Bsp_adv7611GetVideoStatus(
                    pObj,
                    (Bsp_VidDecVideoStatusParams *) cmdArgs,
                    (Bsp_VidDecVideoStatus *) cmdStatusArgs);
                break;

            case FVID2_START:
                retVal = Bsp_adv7611Start(pObj);
                break;

            case FVID2_STOP:
                retVal = Bsp_adv7611Stop(pObj);
                break;

            case IOCTL_BSP_VID_DEC_REG_WRITE:
                retVal = Bsp_adv7611RegWrite(
                    pObj,
                    (Bsp_VidDecRegRdWrParams *) cmdArgs);
                break;

            case IOCTL_BSP_VID_DEC_REG_READ:
                retVal = Bsp_adv7611RegRead(
                    pObj,
                    (Bsp_VidDecRegRdWrParams *) cmdArgs);
                break;

            default:
                GT_0trace(BspDeviceTrace, GT_ERR, "Unsupported command\n");
                retVal = FVID2_EUNSUPPORTED_CMD;
                break;
        }

        /* Unlock handle */
        Bsp_adv7611UnlockObj(pObj);
    }

    return (retVal);
}

/**
 *  \brief Allocates driver object.
 *
 *  Searches in list of driver handles and allocate's a 'NOT IN USE' handle
 *  Also create's handle level semaphore lock.
 *
 *  Returns NULL in case handle could not be allocated.
 */
static Bsp_Adv7611HandleObj *Bsp_adv7611AllocObj(void)
{
    UInt32 handleId;
    Bsp_Adv7611HandleObj *pObj = NULL;
    Int32  initValue;

    /* Take global lock to avoid race condition */
    Bsp_adv7611Lock();

    /* Find a unallocated object in pool */
    for (handleId = 0U; handleId < BSP_DEVICE_MAX_HANDLES; handleId++)
    {
        if (BSP_ADV7611_OBJ_STATE_UNUSED ==
            gBspAdv7611Obj.handlePool[handleId].state)
        {
            /* Free object found */
            pObj = &gBspAdv7611Obj.handlePool[handleId];

            /* Init state and handle ID */
            BspUtils_memset(pObj, 0, sizeof (*pObj));
            pObj->state    = BSP_ADV7611_OBJ_STATE_IDLE;
            pObj->handleId = handleId;

            /* Create driver object specific semaphore lock */
            initValue  = 1;
            pObj->lock = BspOsal_semCreate(initValue, (Bool) TRUE);
            if (NULL == pObj->lock)
            {
                GT_0trace(BspDeviceTrace, GT_ERR,
                          "Handle semaphore create failed\n");
                /* Error - release object */
                pObj->state = BSP_ADV7611_OBJ_STATE_UNUSED;
                pObj        = NULL;
            }
            break;
        }
    }

    /* Release global lock */
    Bsp_adv7611Unlock();

    return (pObj);
}

/**
 *  \brief De-Allocate driver object.
 *
 *  Marks handle as 'NOT IN USE'.
 *  Also delete's handle level semaphore lock.
 */
static Int32 Bsp_adv7611FreeObj(Bsp_Adv7611HandleObj *pObj)
{
    /* Check for NULL pointers */
    GT_assert(BspDeviceTrace, (NULL != pObj));

    /* Take global lock to avoid race condition */
    Bsp_adv7611Lock();

    if (pObj->state != BSP_ADV7611_OBJ_STATE_UNUSED)
    {
        /* Mark state as unused */
        pObj->state = BSP_ADV7611_OBJ_STATE_UNUSED;

        /* Delete object locking semaphore */
        GT_assert(BspDeviceTrace, (NULL != pObj->lock));
        BspOsal_semDelete(&pObj->lock);
    }

    /* Release global lock */
    Bsp_adv7611Unlock();

    return (FVID2_SOK);
}

/**
 *  \brief Handle level lock.
 */
static Int32 Bsp_adv7611LockObj(Bsp_Adv7611HandleObj *pObj)
{
    /* Check for NULL pointers */
    GT_assert(BspDeviceTrace, (NULL != pObj));

    BspOsal_semWait(pObj->lock, BSP_OSAL_WAIT_FOREVER);

    return (FVID2_SOK);
}

/**
 *  \brief Handle level unlock
 */
static Int32 Bsp_adv7611UnlockObj(Bsp_Adv7611HandleObj *pObj)
{
    /* Check for NULL pointers */
    GT_assert(BspDeviceTrace, (NULL != pObj));

    BspOsal_semPost(pObj->lock);

    return (FVID2_SOK);
}

/**
 *  \brief Global driver level lock.
 */
static Int32 Bsp_adv7611Lock(void)
{
    BspOsal_semWait(gBspAdv7611Obj.lock, BSP_OSAL_WAIT_FOREVER);

    return (FVID2_SOK);
}

/**
 *  \brief Global driver level unlock.
 */
static Int32 Bsp_adv7611Unlock(void)
{
    BspOsal_semPost(gBspAdv7611Obj.lock);

    return (FVID2_SOK);
}
