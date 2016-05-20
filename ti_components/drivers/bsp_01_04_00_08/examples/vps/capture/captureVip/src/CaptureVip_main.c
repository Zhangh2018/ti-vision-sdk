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
 *  \file CaptureVip_main.c
 *
 *  \brief VIP Capture sample application.
 */
/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "CaptureVip_main.h"

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

static void CaptApp_createTsk(void);
static void CaptApp_tskMain(UArg arg0, UArg arg1);

static void CaptApp_tskReset(UArg arg0, UArg arg1);

static void CaptApp_init(CaptApp_Obj *appObj);
static void CaptApp_deInit(CaptApp_Obj *appObj);

static void CaptApp_runTest(CaptApp_Obj              *appObj,
                            const CaptApp_TestParams *testPrms);

static Int32 CaptApp_create(CaptApp_Obj *appObj);
static Int32 CaptApp_delete(CaptApp_Obj *appObj);

static Int32 CaptApp_allocAndQueueFrames(const CaptApp_Obj *appObj,
                                         CaptApp_InstObj   *instObj,
                                         UInt32             fieldMerged);
static Int32 CaptApp_freeFrames(CaptApp_Obj     *appObj,
                                CaptApp_InstObj *instObj,
                                UInt32           fieldMerged);

static Int32 CaptApp_createVidDec(CaptApp_Obj     *appObj,
                                  CaptApp_InstObj *instObj);
static Int32 CaptApp_createVidSensor(const CaptApp_Obj *appObj,
                                     CaptApp_InstObj   *instObj);
static Int32 CaptApp_detectVideo(CaptApp_Obj     *appObj,
                                 CaptApp_InstObj *instObj);
static Int32 CaptApp_selectSimVideoSource(const CaptApp_Obj *appObj,
                                          CaptApp_InstObj   *instObj);

static void CaptApp_initParams(CaptApp_Obj *appObj);
static void CaptApp_printStatistics(CaptApp_Obj *appObj,
                                    UInt32       execTimeInMsecs);
static Int32 CaptApp_getTestId(CaptApp_Obj *appObj);
static Int32 CaptApp_overflowCbFxn(CaptApp_InstObj *appObj);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* Align stack memory to integer boundary. */
#pragma DATA_ALIGN(gCaptAppTskStackMain, 32)
/* Place the stack in stack section. */
#pragma DATA_SECTION(gCaptAppTskStackMain, ".bss:taskStackSection")
/* Test application stack */
static UInt8             gCaptAppTskStackMain[CAPT_APP_TSK_STACK_MAIN];

/* Align stack memory to integer boundary. */
#pragma DATA_ALIGN(gCaptAppTskStackReset, 32)
/* Place the stack in stack section. */
#pragma DATA_SECTION(gCaptAppTskStackReset, ".bss:taskStackSection")
/* Test application stack */
static UInt8             gCaptAppTskStackReset[CAPT_APP_TSK_STACK_RESET];

/* Maximum number of continuous interrupts*/
#define MAX_CONTI_OF_INTERRUPTS 2

/* Test application object */
CaptApp_Obj              gCaptAppObj;

/* Smaphore to sync between the reset and the callback function */
static BspOsal_SemHandle overflowSem;

static volatile UInt32   gExitApp;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/*
 * Application main
 */
Int32 main(void)
{
    static Char stdin_buf[BUFSIZ];

    /* Provide buffer so that STDIO library will use this memory instead of
     * allocating from system heap, which might lead to false system leak
     * warning */
    setvbuf(stdin, stdin_buf, _IOLBF, BUFSIZ);

    /* Create test task */
    CaptApp_createTsk();

    /* Start BIOS */
    BIOS_start();

    return (0);
}

/*
 * Create test task
 */
static void CaptApp_createTsk(void)
{
    Task_Params tskPrms;

    /* Create task to wait for overflow to occur.
     * When it occurs, this task will just reset VIP */
    gExitApp = (UInt32) FALSE;
    Task_Params_init(&tskPrms);
    tskPrms.priority           = CAPT_APP_TSK_PRI_MAIN;
    tskPrms.stack              = gCaptAppTskStackReset;
    tskPrms.stackSize          = sizeof (gCaptAppTskStackReset);
    tskPrms.arg0               = (UArg) & gCaptAppObj;
    gCaptAppObj.resetTskHandle = Task_create(CaptApp_tskReset, &tskPrms, NULL);
    GT_assert(BspAppTrace, (gCaptAppObj.resetTskHandle != NULL));

    /* Create test task */
    Task_Params_init(&tskPrms);
    tskPrms.priority      = CAPT_APP_TSK_PRI_MAIN;
    tskPrms.stack         = gCaptAppTskStackMain;
    tskPrms.stackSize     = sizeof (gCaptAppTskStackMain);
    tskPrms.arg0          = (UArg) & gCaptAppObj;
    gCaptAppObj.tskHandle = Task_create(CaptApp_tskMain, &tskPrms, NULL);
    GT_assert(BspAppTrace, (gCaptAppObj.tskHandle != NULL));

    /* Register the task to the load module for calculating the load */
    BspUtils_prfLoadRegister(gCaptAppObj.tskHandle, APP_NAME);

    return;
}

/*
 * Test task main
 */
static void CaptApp_tskMain(UArg arg0, UArg arg1)
{
    Int32       testId;
    Bsp_BoardId boardId;
    CaptApp_Obj           *appObj = (CaptApp_Obj *) arg0;
    CaptApp_TestParams    *testPrms;
    BspUtils_MemHeapStatus startHeapStat, startHeapStat1;

    GT_0trace(BspAppTrace, GT_INFO,
              APP_NAME ": Sample Application - STARTS !!!\n");
    BspUtils_memGetHeapStat(&startHeapStat);
    CaptApp_init(appObj);

    appObj->enableAutoRun = (UInt32) FALSE;
    boardId = Bsp_boardGetId();

    while (1U)
    {
        /* Get the TestId */
        testId = CaptApp_getTestId(appObj);
        if ((testId >= 0) && (testId < CAPT_APP_NUM_TESTS))
        {
            testPrms = &gCaptAppTestPrms[testId];
            if (testPrms->boardId & boardId)
            {
                BspUtils_memGetHeapStat(&startHeapStat1);

                testPrms->testId = testId;
                CaptApp_runTest(appObj, testPrms);

                BspUtils_memCheckHeapStat(&startHeapStat1);
            }
            else
            {
                GT_1trace(BspAppTrace, GT_INFO,
                          APP_NAME ": Skipping test case %d!!!\n", testId);
            }
        }
        else
        {
            /* Exit */
            break;
        }
    }

    CaptApp_deInit(appObj);

    gExitApp = (UInt32) TRUE;
    BspOsal_semPost(overflowSem);
    BspOsal_sleep(500);         /* Wait for reset task to exit */

    BspUtils_memCheckHeapStat(&startHeapStat);
    GT_1trace(BspAppTrace, GT_INFO,
              APP_NAME ": Max stack used for test task: %d bytes\n",
              BspOsal_getTaskStackUsage(NULL));
    GT_1trace(BspAppTrace, GT_INFO,
              APP_NAME ": Max system stack used (ISR): %d bytes\n",
              BspOsal_getSystemStackUsage());
    BspUtils_appPrintMemStatus();

    GT_0trace(BspAppTrace, GT_INFO,
              APP_NAME ": Sample Application - DONE !!!\n");

    return;
}

static void CaptApp_init(CaptApp_Obj *appObj)
{
    Int32  retVal;
    UInt32 isI2cInitReq, boardId;

    /* System init */
    isI2cInitReq = TRUE;
    retVal       = BspUtils_appDefaultInit(isI2cInitReq);
    if (retVal != FVID2_SOK)
    {
        GT_0trace(BspAppTrace, GT_ERR,
                  APP_NAME ": System Init Failed!!!\n");
    }

    if (FVID2_SOK == retVal)
    {
        /* Create global capture handle, used for common driver configuration */
        appObj->fvidHandleAll = Fvid2_create(
            FVID2_VPS_CAPT_VID_DRV,
            VPS_CAPT_INST_ALL,
            NULL,                       /* NULL for VPS_CAPT_INST_ALL */
            NULL,                       /* NULL for VPS_CAPT_INST_ALL */
            NULL);                      /* NULL for VPS_CAPT_INST_ALL */
        if (NULL == appObj->fvidHandleAll)
        {
            GT_0trace(BspAppTrace, GT_ERR,
                      APP_NAME ": Global Handle Create Failed!!!\n");
            retVal = FVID2_EFAIL;
        }
    }

    if (FVID2_SOK == retVal)
    {
        boardId = Bsp_boardGetId();

        if (BSP_BOARD_MULTIDES == boardId)
        {
            retVal = BspUtils_appInitSerDeSer(
                FVID2_VID_SENSOR_MULDES_OV1063X_DRV);
            if (retVal != FVID2_SOK)
            {
                GT_0trace(BspAppTrace, GT_ERR,
                          APP_NAME ": MultiDes Board Init failed!!!\n");
            }
        }
    }
    if (FVID2_SOK == retVal)
    {
        GT_0trace(BspAppTrace, GT_INFO,
                  APP_NAME ": CaptApp_init() - DONE !!!\n");
    }

    return;
}

static void CaptApp_deInit(CaptApp_Obj *appObj)
{
    Int32  retVal;
    UInt32 isI2cDeInitReq;

    /* Delete global VIP capture handle */
    retVal = Fvid2_delete(appObj->fvidHandleAll, NULL);
    if (retVal != FVID2_SOK)
    {
        GT_0trace(BspAppTrace, GT_ERR,
                  APP_NAME ": Global handle delete failed!!!\n");
    }

    if (FVID2_SOK == retVal)
    {
        if (BSP_BOARD_MULTIDES == Bsp_boardGetId())
        {
            retVal =
                BspUtils_appDeInitSerDeSer(
                    FVID2_VID_SENSOR_MULDES_OV1063X_DRV);
            if (retVal != FVID2_SOK)
            {
                GT_0trace(BspAppTrace, GT_ERR,
                          APP_NAME ": MultiDes Board DeInit failed!!!\n");
            }
        }
    }

    if (retVal == FVID2_SOK)
    {
        /* System de-init */
        isI2cDeInitReq = TRUE;
        retVal         = BspUtils_appDefaultDeInit(isI2cDeInitReq);
        if (retVal != FVID2_SOK)
        {
            GT_0trace(BspAppTrace, GT_ERR,
                      APP_NAME ": System De-Init Failed!!!\n");
        }
    }
    if (retVal == FVID2_SOK)
    {
        GT_0trace(BspAppTrace, GT_INFO,
                  APP_NAME ": CaptApp_deInit() - DONE !!!\n");
    }
    return;
}

/**
 *  CaptApp_runTest
 *  Capture test function.
 */
static void CaptApp_runTest(CaptApp_Obj              *appObj,
                            const CaptApp_TestParams *testPrms)
{
    Int32            retVal = FVID2_SOK;
    UInt32           instCnt;
    UInt32           captureTime, curTime, prevTime;
    UInt32           loopCount, prevFldCount, curFldCount, fldCount;
    CaptApp_InstObj *instObj;

    loopCount = 0;
    curTime   = prevTime = curFldCount = prevFldCount = 0;
    BspUtils_memcpy(&appObj->testPrms, testPrms, sizeof (appObj->testPrms));

    /* Print test case information */
    GT_6trace(BspAppTrace, GT_INFO,
              APP_NAME ": HANDLES %d: MODE %04x : CH %d: RUN COUNT %d: "
              "OUTPUT:%d:%d !!!\n",
              appObj->testPrms.numHandles,
              appObj->testPrms.videoIfMode,
              appObj->testPrms.numCh,
              appObj->testPrms.runCount,
              appObj->testPrms.outDataFmt[0],
              appObj->testPrms.outDataFmt[1]);

    /* Create driver */
    retVal = CaptApp_create(appObj);
    if (FVID2_SOK != retVal)
    {
        return;
    }

    /* Start decoder*/
    for (instCnt = 0U; instCnt < appObj->testPrms.numHandles; instCnt++)
    {
        instObj = &appObj->instObj[instCnt];
        if (NULL != instObj->decHandle)
        {
            Fvid2_start(instObj->decHandle, NULL);
        }
    }

    GT_0trace(BspAppTrace, GT_INFO,
              APP_NAME ": Starting capture ... !!!\r\n");
    GT_0trace(BspAppTrace, GT_INFO,
              APP_NAME ": Capture in progress ... DO NOT HALT !!!\r\n");

    appObj->startTime = BspOsal_getCurTimeInMsec();

    /* Start the load calculation */
    BspUtils_prfLoadCalcStart();

    /* Start driver */
    for (instCnt = 0U; instCnt < appObj->testPrms.numHandles; instCnt++)
    {
        instObj = &appObj->instObj[instCnt];

        retVal = Fvid2_start(instObj->drvHandle, NULL);
        if (retVal != FVID2_SOK)
        {
            GT_0trace(BspAppTrace, GT_ERR,
                      APP_NAME ": Capture Start Failed!!!\n");
            break;
        }
    }
    if (FVID2_SOK == retVal)
    {
        /* Capture frames, check status */
        while (loopCount < appObj->testPrms.runCount)
        {
            if (BSP_PLATFORM_ID_EVM == Bsp_platformGetId())
            {
                /* check status every sec */
                BspOsal_sleep(1000);
            }
            else
            {
                BspOsal_sleep(1);
            }

            /* Get CPU load */
            appObj->totalCpuLoad += Load_getCPULoad();
            appObj->cpuLoadCount++;

            /* Do runtime print every 5 sec */
            if ((loopCount != 0) && ((loopCount % 5) == 0))
            {
                curTime     = BspOsal_getCurTimeInMsec() - appObj->startTime;
                curFldCount = appObj->totalFldCount;

                captureTime = curTime - prevTime;
                fldCount    = curFldCount - prevFldCount;

                prevTime     = curTime;
                prevFldCount = curFldCount;

                Bsp_rprintf(APP_NAME ": %5d.%3ds: Fields = %5d (%3d fps)\r\n",
                            (curTime / 1000), (curTime % 1000),
                            fldCount, ((fldCount * 1000) / captureTime));
            }

            if (BSP_PLATFORM_ID_EVM == Bsp_platformGetId())
            {
                loopCount++;
            }
            else
            {
                if (loopCount != appObj->callBackCount)
                {
                    GT_1trace(BspAppTrace, GT_INFO,
                              APP_NAME ": Captured %d frames!!\n",
                              appObj->callBackCount);
                }
                loopCount = appObj->callBackCount;
            }
        }

        /* Stop driver */
        for (instCnt = 0U; instCnt < appObj->testPrms.numHandles; instCnt++)
        {
            instObj = &appObj->instObj[instCnt];
            retVal  = Fvid2_stop(instObj->drvHandle, NULL);
            if (retVal != FVID2_SOK)
            {
                GT_0trace(BspAppTrace, GT_ERR,
                          APP_NAME ": Capture Stop Failed!!!\n");
            }
        }
    }

    if (FVID2_SOK == retVal)
    {
        /* Stop the load calculation */
        BspUtils_prfLoadCalcStop();

        captureTime = (BspOsal_getCurTimeInMsec() - appObj->startTime);

        /* Stop decoder */
        for (instCnt = 0U; instCnt < appObj->testPrms.numHandles; instCnt++)
        {
            instObj = &appObj->instObj[instCnt];
            if (instObj->decHandle)
            {
                Fvid2_stop(instObj->decHandle, NULL);
            }
        }

        GT_0trace(BspAppTrace, GT_INFO,
                  APP_NAME ": Stopping capture ... DONE !!!\r\n");

        CaptApp_printStatistics(appObj, captureTime);

        /* Delete driver */
        CaptApp_delete(appObj);

        /* Print the load */
        BspUtils_prfLoadPrintAll(TRUE);
        /* Reset the load */
        BspUtils_prfLoadCalcReset();
    }

    return;
}

static Int32 CaptApp_create(CaptApp_Obj *appObj)
{
    Int32  retVal = FVID2_SOK;
    UInt32 instCnt;
    UInt32 streamId, chId;
    CaptApp_InstObj       *instObj;
    Vps_VpdmaMaxSizeParams vipMaxSizePrms;
    Vps_CaptFrameSkip      frmSkipPrms;

    CaptApp_initParams(appObj);

    for (instCnt = 0U; instCnt < appObj->testPrms.numHandles; instCnt++)
    {
        instObj = &appObj->instObj[instCnt];
        instObj->drvHandle = Fvid2_create(
            FVID2_VPS_CAPT_VID_DRV,
            instObj->instId,
            &instObj->createPrms,
            &instObj->createStatus,
            &instObj->cbPrms);
        if ((NULL == instObj->drvHandle) ||
            (instObj->createStatus.retVal != FVID2_SOK))
        {
            GT_0trace(BspAppTrace, GT_ERR,
                      APP_NAME ": Capture Create Failed!!!\n");
            retVal = instObj->createStatus.retVal;
        }

        if (FVID2_SOK == retVal)
        {
            retVal = Fvid2_control(
                instObj->drvHandle,
                IOCTL_VPS_CAPT_SET_VIP_PARAMS,
                &instObj->vipPrms,
                NULL);
            if (retVal != FVID2_SOK)
            {
                GT_0trace(BspAppTrace, GT_ERR,
                          APP_NAME ": VIP Set Params IOCTL Failed!!!\n");
                Fvid2_delete(instObj->drvHandle, NULL);
                return (retVal);
            }
        }
        if (FVID2_SOK == retVal)
        {
            /* Set frame skip for each channel */
            VpsCaptFrameSkip_init(&frmSkipPrms);
            for (streamId = 0U; streamId < instObj->createPrms.numStream;
                 streamId++)
            {
                for (chId = 0U; chId < instObj->createPrms.numCh; chId++)
                {
                    /* This IOCTL is supported only for frame drop mode */
                    if (instObj->createPrms.bufCaptMode ==
                        VPS_CAPT_BCM_FRM_DROP)
                    {
                        frmSkipPrms.chNum =
                            Vps_captMakeChNum(instObj->instId, streamId, chId);
                        frmSkipPrms.frmSkipMask = CAPT_APP_FRAME_SKIP_PATTERN;
                        retVal = Fvid2_control(
                            instObj->drvHandle,
                            IOCTL_VPS_CAPT_SET_FRAME_SKIP,
                            &frmSkipPrms,
                            NULL);
                        if (retVal != FVID2_SOK)
                        {
                            GT_0trace(
                                BspAppTrace, GT_ERR,
                                APP_NAME
                                ": VIP Set frame skip IOCTL Failed!!\n");
                            break;
                        }
                    }
                }
                if (FVID2_SOK != retVal)
                {
                    break;
                }
            }
        }
        if (FVID2_SOK == retVal)
        {
            if (!Bsp_platformIsTI814xFamilyBuild())
            {
                VpsVpdmaMaxSizeParams_init(&vipMaxSizePrms);
                vipMaxSizePrms.instId = Vps_captGetVipId(
                    instObj->instId);
                vipMaxSizePrms.maxOutWidth[0U]  = instObj->maxOutWidth[0U];
                vipMaxSizePrms.maxOutHeight[0U] = instObj->maxOutHeight[0U];
                vipMaxSizePrms.maxOutWidth[1U]  = instObj->maxOutWidth[1U];
                vipMaxSizePrms.maxOutHeight[1U] = instObj->maxOutHeight[1U];
                vipMaxSizePrms.maxOutWidth[2U]  = instObj->maxOutWidth[2U];
                vipMaxSizePrms.maxOutHeight[2U] = instObj->maxOutHeight[2U];

                retVal = Fvid2_control(
                    appObj->fvidHandleAll,
                    IOCTL_VPS_CAPT_SET_VIP_MAX_SIZE,
                    &vipMaxSizePrms,
                    NULL);
                if (retVal != FVID2_SOK)
                {
                    GT_0trace(
                        BspAppTrace, GT_ERR,
                        APP_NAME
                        ": VIP Set Max Frame Size Params IOCTL Failed!!!\n");
                }
            }
        }
        if (FVID2_SOK == retVal)
        {
            retVal = CaptApp_allocAndQueueFrames(appObj, instObj, FALSE);
            if (retVal != FVID2_SOK)
            {
                GT_0trace(BspAppTrace, GT_ERR,
                          APP_NAME ": Capture Alloc and Queue Failed!!!\n");
            }
        }

        if (retVal == FVID2_SOK)
        {
            Vps_CaptOverflowParams overflowParams;
            /* Fill the overflowParams with the callback function in case of
             * overflow and the application object corrosponding to that
             * instance of VIP
             */
            overflowParams.overflowCbFxn = (Vps_CaptVipOverflowFxn) &
                                           CaptApp_overflowCbFxn;
            overflowParams.retParams = (void *) instObj;

            /* Register for overflow intrrupt
             * User can deregister from the interrupt using
             * IOCTL_VPS_CAPT_UNREGISTER_OVF_INTR
             */
            retVal = Fvid2_control(
                instObj->drvHandle,
                IOCTL_VPS_CAPT_REGISTER_OVF_INTR,
                &overflowParams,
                NULL);
            if (retVal != FVID2_SOK)
            {
                GT_0trace(BspAppTrace, GT_ERR,
                          APP_NAME ": VIP Set Params IOCTL Failed!!!\n");
            }
        }

        if (FVID2_SOK == retVal)
        {
            instObj->decHandle = NULL;
            if (BSP_PLATFORM_ID_EVM == Bsp_platformGetId())
            {
                if ((FVID2_VID_SENSOR_MT9V022_DRV == instObj->decDrvId) ||
                    (FVID2_VID_SENSOR_OV1063X_DRV == instObj->decDrvId) ||
                    (FVID2_VID_SENSOR_LI_OV1063X_DRV == instObj->decDrvId) ||
                    (FVID2_VID_SENSOR_APT_AR0132_DRV == instObj->decDrvId) ||
                    (FVID2_VID_SENSOR_MULDES_OV1063X_DRV == instObj->decDrvId))
                {
                    CaptApp_createVidSensor(appObj, instObj);
                }
                else if (0U != instObj->decDrvId)
                {
                    CaptApp_createVidDec(appObj, instObj);
                }
                else
                {
                    GT_0trace(BspAppTrace, GT_INFO,
                              APP_NAME ": Skip decoder config!!\n");
                }
            }
            else if ((BSP_PLATFORM_ID_SIM == Bsp_platformGetId()) ||
                     (BSP_PLATFORM_ID_VIRTIO == Bsp_platformGetId()))
            {
                CaptApp_selectSimVideoSource(appObj, instObj);
            }
        }

        if (FVID2_SOK == retVal)
        {
            retVal = Bsp_boardSetPinMux(FVID2_VPS_CAPT_VID_DRV,
                                        instObj->instId,
                                        appObj->testPrms.boardMode);
            if (FVID2_SOK != retVal)
            {
                GT_0trace(BspAppTrace, GT_ERR, "Pin Muxing Failed \n");
            }
        }

        if (FVID2_SOK != retVal)
        {
            break;
        }
    }

    GT_0trace(BspAppTrace, GT_INFO, APP_NAME ": Capture create complete!!\n");

    return (retVal);
}

/*
 *  Callback function, recieved when overflow occues in VIP
 */
static Int32 CaptApp_overflowCbFxn(CaptApp_InstObj *instObj)
{
    Int32 retVal = FVID2_SOK;

    GT_assert(BspAppTrace, (instObj != NULL));

    instObj->overflowCount++;

    /* Most of the time VIP recovers automatically. This is to give
     * some time to VIP to recover automatically
     */
    if (instObj->overflowCount > MAX_CONTI_OF_INTERRUPTS)
    {
        /* This is to stop the VIP. This will set the S/W reset bit in the
         * FIQ registers of VIP parser. This is not a blocking IOCTL.
         */
        retVal = Fvid2_control(
            instObj->drvHandle,
            IOCTL_VPS_CAPT_RESET_VIP_PORT,
            NULL,
            NULL);
        if (retVal != FVID2_SOK)
        {
            GT_0trace(BspAppTrace, GT_ERR,
                      APP_NAME ": Capture Stop Failed!!!\n");
        }

        BspOsal_semPost(overflowSem);
    }
    return retVal;
}

/*
 *  Reset task
 * When overflow occurs, this task will run.
 * It will stop the driver, reset it and again start it.
 */

static void CaptApp_tskReset(UArg arg0, UArg arg1)

{
    CaptApp_Obj     *appObj = (CaptApp_Obj *) arg0;
    CaptApp_InstObj *instObj;
    UInt32           captInstCnt = 0;
    Int32 retVal;

    GT_assert(BspAppTrace, (appObj != NULL));

    overflowSem = BspOsal_semCreate(0, FALSE);
    GT_assert(BspAppTrace, (overflowSem != NULL));

    while (1U)
    {
        /* Wait for overflow to occur */
        BspOsal_semWait(overflowSem, BSP_OSAL_WAIT_FOREVER);
        if ((UInt32) TRUE == gExitApp)
        {
            BspOsal_semDelete(&overflowSem);
            break;
        }
        for (captInstCnt = 0; captInstCnt < appObj->testPrms.numHandles;
             captInstCnt++)
        {
            Vps_CaptOverflowCheckParams overflowCheckParams;
            /* For all instances, check if the overflow has occured on that
             *instance. */
            instObj = &appObj->instObj[captInstCnt];
            retVal  = Fvid2_control(
                instObj->drvHandle,
                IOCTL_VPS_CAPT_CHECK_OVERFLOW,
                &overflowCheckParams,
                NULL);
            /* If overflow has occured, initiate the reset sequence */
            if (TRUE == overflowCheckParams.isOverflowOccured)
            {
                /* Count is reset befor the reset. If overflow
                 * occurs in reset, then it will be lost otherwise*/
                instObj->overflowCount = 0;
                /*Stop the driver*/
                retVal = Fvid2_stop(instObj->drvHandle, NULL);
                if (retVal != FVID2_SOK)
                {
                    GT_0trace(BspAppTrace, GT_ERR,
                              APP_NAME ": Capture Stop Failed!!!\n");
                }
                if (FVID2_SOK == retVal)
                {
                    /* Reset the VIP */
                    retVal = Fvid2_control(
                        instObj->drvHandle,
                        IOCTL_VPS_CAPT_RESET_VIP,
                        NULL,
                        NULL);
                    if (retVal != FVID2_SOK)
                    {
                        GT_0trace(BspAppTrace, GT_ERR,
                                  APP_NAME ": Capture Reset Failed!!!\n");
                    }
                }
                if (FVID2_SOK == retVal)
                {
                    /* Restart the driver */
                    retVal = Fvid2_start(instObj->drvHandle, NULL);
                    if (retVal != FVID2_SOK)
                    {
                        GT_0trace(BspAppTrace, GT_ERR,
                                  APP_NAME ": Capture Start Failed!!!\n");
                    }
                }

                if (FVID2_SOK != retVal)
                {
                    break;
                }
            }
        }
    }
    return;
}

static Int32 CaptApp_delete(CaptApp_Obj *appObj)
{
    Int32            retVal = FVID2_SOK;
    UInt32           instCnt, streamId, sensorInstId, boardId;
    CaptApp_InstObj *instObj;
    Fvid2_FrameList  frmList;

    for (instCnt = 0U; instCnt < appObj->testPrms.numHandles; instCnt++)
    {
        instObj = &appObj->instObj[instCnt];

        for (streamId = 0U;
             streamId < instObj->createPrms.numStream;
             streamId++)
        {
            /* Dequeue all the request from the driver */
            while (1U)
            {
                retVal = Fvid2_dequeue(
                    instObj->drvHandle,
                    &frmList,
                    streamId,
                    FVID2_TIMEOUT_NONE);
                if (FVID2_SOK != retVal)
                {
                    break;
                }
            }
        }
        retVal = Fvid2_control(
            instObj->drvHandle,
            IOCTL_VPS_CAPT_UNREGISTER_OVF_INTR,
            NULL,
            NULL);
        if (retVal != FVID2_SOK)
        {
            GT_0trace(
                BspAppTrace, GT_ERR,
                APP_NAME
                ": Unregister From Overflow Interrupt Failed!!!\n");
        }

        retVal = Fvid2_delete(instObj->drvHandle, NULL);
        if (FVID2_SOK != retVal)
        {
            GT_0trace(BspAppTrace, GT_ERR,
                      APP_NAME ": Capture Delete Failed!!!\n");
        }

        /* free frame memory */
        retVal = CaptApp_freeFrames(appObj, instObj, FALSE);
        if (retVal != FVID2_SOK)
        {
            GT_0trace(BspAppTrace, GT_ERR,
                      APP_NAME ": Capture Free Frames Failed!!!\n");
        }

        if (NULL != instObj->decHandle)
        {
            retVal = Fvid2_delete(instObj->decHandle, NULL);
            if (FVID2_SOK != retVal)
            {
                GT_0trace(BspAppTrace, GT_ERR,
                          APP_NAME ": Decoder Delete Failed!!!\n");
            }
            instObj->decHandle = NULL;
        }

        /* DeConfiguring a particular instance of SerDeSer */
        boardId = Bsp_boardGetId();

        if ((BSP_BOARD_MULTIDES == boardId) &&
            (FVID2_VID_SENSOR_MULDES_OV1063X_DRV == instObj->decDrvId))
        {
            sensorInstId = Bsp_boardGetVideoDeviceInstId(
                instObj->decDrvId,
                FVID2_VPS_CAPT_VID_DRV,
                instObj->instId);

            retVal =
                BspUtils_appDeConfSerDeSer(instObj->decDrvId, sensorInstId);
            if (retVal != FVID2_SOK)
            {
                GT_1trace(
                    BspAppTrace, GT_ERR,
                    APP_NAME
                    ": DeConfiguring MultiDes instance %d failed!!!\n",
                    sensorInstId);
            }
        }
    }

    GT_0trace(BspAppTrace, GT_INFO, APP_NAME ": Capture delete complete!!\n");

    return (retVal);
}

/**
 *  CaptApp_cbFxn
 *  \brief Driver callback function.
 */
static Int32 CaptApp_cbFxn(Fvid2_Handle handle, Ptr appData, Ptr reserved)
{
    Int32            retVal = FVID2_SOK;
    UInt32           streamId;
    CaptApp_InstObj *instObj = (CaptApp_InstObj *) appData;
    Fvid2_FrameList  frmList;

    for (streamId = 0U; streamId < instObj->createPrms.numStream; streamId++)
    {
        retVal = Fvid2_dequeue(
            instObj->drvHandle,
            &frmList,
            streamId,
            FVID2_TIMEOUT_NONE);
        if (FVID2_SOK == retVal)
        {
            retVal = Fvid2_queue(instObj->drvHandle, &frmList, streamId);
            if (FVID2_SOK != retVal)
            {
                GT_0trace(BspAppTrace, GT_ERR,
                          APP_NAME ": Capture Queue Failed!!!\n");
            }
        }
    }

    instObj->appObj->callBackCount++;

    return (retVal);
}

/**
 *  \brief Allocate and queue frames to driver
 */
static Int32 CaptApp_allocAndQueueFrames(const CaptApp_Obj *appObj,
                                         CaptApp_InstObj   *instObj,
                                         UInt32             fieldMerged)
{
    Int32               retVal = FVID2_SOK;
    UInt32              streamId, chId, frmId, idx;
    UInt32              bufSize;
    UInt32              yFld1Offset, cbCrFld0Offset, cbCrFld1Offset;
    Fvid2_Format       *fmt;
    Fvid2_Frame        *frm;
    Fvid2_FrameList     frmList;
    Vps_CaptVipOutInfo *outInfo;
    Char fileStr[50U];

    /* for every stream and channel in a capture handle */
    Fvid2FrameList_init(&frmList);
    for (streamId = 0U; streamId < instObj->createPrms.numStream; streamId++)
    {
        for (chId = 0U; chId < instObj->createPrms.numCh; chId++)
        {
            outInfo = &instObj->vipPrms.outStreamInfo[streamId];
            fmt     = &instObj->allocFmt[streamId];
            Fvid2Format_init(fmt);

            /* base index for instObj->frames[] */
            idx =
                (VPS_CAPT_CH_PER_PORT_MAX * CAPT_APP_FRAMES_PER_CH
                 * streamId) + (CAPT_APP_FRAMES_PER_CH * chId);

            if (idx >= CAPT_APP_MAX_FRAMES_PER_HANDLE)
            {
                idx = 0U;
            }
            frm = &instObj->frames[idx];

            /* fill format with channel specific values  */
            fmt->chNum  = Vps_captMakeChNum(instObj->instId, streamId, chId);
            fmt->width  = appObj->maxWidth;
            fmt->height = appObj->maxHeight + CAPT_APP_PADDING_LINES;
            if (fieldMerged)
            {
                fmt->height *= 2;
            }
            fmt->pitch[0]       = outInfo->outFmt.pitch[0];
            fmt->pitch[1]       = outInfo->outFmt.pitch[1];
            fmt->pitch[2]       = outInfo->outFmt.pitch[2];
            fmt->fieldMerged[0] = FALSE;
            fmt->fieldMerged[1] = FALSE;
            fmt->fieldMerged[2] = FALSE;
            fmt->dataFormat     = outInfo->outFmt.dataFormat;
            fmt->scanFormat     = FVID2_SF_PROGRESSIVE;
            fmt->bpp = FVID2_BPP_BITS8; /* ignored */

            if (BSP_PLATFORM_ID_EVM == Bsp_platformGetId())
            {
                BspUtils_memClearOnAlloc(TRUE);
            }

            /*
             * alloc memory based on 'format'
             * Allocated frame info is put in frames[]
             * CAPT_APP_FRAMES_PER_CH is the number of buffers per channel to
             * allocate
             */
            retVal = BspUtils_memFrameAlloc(fmt, frm, CAPT_APP_FRAMES_PER_CH);
            if (FVID2_SOK != retVal)
            {
                GT_0trace(BspAppTrace, GT_ERR,
                          APP_NAME ": Capture Frame Alloc Failed!!!\n");
                break;
            }

            snprintf(fileStr, sizeof (fileStr),
                     "captureOption%dInst%dCh%dStr%d",
                     appObj->testPrms.testId, instObj->instId, chId, streamId);
            BspUtils_memFrameGetSize(fmt, &bufSize, NULL);
            BspUtils_appPrintSaveRawCmd(
                fileStr,
                frm[0].addr[0][0],
                outInfo->outFmt.dataFormat,
                fmt->width,
                fmt->height,
                (CAPT_APP_FRAMES_PER_CH * bufSize));

            for (frmId = 0U; frmId < CAPT_APP_FRAMES_PER_CH; frmId++)
            {
                frm[frmId].fid        = FVID2_FID_FRAME;
                frm[frmId].appData    = instObj;
                frmList.frames[frmId] = &frm[frmId];

                if (TRUE == fieldMerged)
                {
                    /* Since BspUtils_memFrameAlloc is setting the address
                     * for only top field, set addresses for bottom fields. */
                    if (Fvid2_isDataFmtYuv422I(fmt->dataFormat))
                    {
                        yFld1Offset = (UInt32) frm[frmId].addr[0U][0U] +
                                      fmt->pitch[0U];
                        frm[frmId].addr[1U][0U] = (Ptr) yFld1Offset;
                    }
                    if (Fvid2_isDataFmtSemiPlanar(fmt->dataFormat))
                    {
                        yFld1Offset =
                            (UInt32) frm[frmId].addr[0U][0U] + fmt->pitch[0U];
                        cbCrFld0Offset =
                            (UInt32) ((UInt32) frm[frmId].addr[0U][0U] +
                                      (fmt->pitch[0U] * appObj->maxHeight * 2U));
                        cbCrFld1Offset =
                            (UInt32) (cbCrFld0Offset + fmt->pitch[0U]);
                        frm[frmId].addr[0U][1U] = (Ptr) cbCrFld0Offset;
                        frm[frmId].addr[1U][0U] = (Ptr) yFld1Offset;
                        frm[frmId].addr[1U][1U] = (Ptr) cbCrFld1Offset;
                    }
                }
            }

            /* Set number of frame in frame list */
            frmList.numFrames = CAPT_APP_FRAMES_PER_CH;

            /*
             * queue the frames in frmList
             * All allocate frames are queued here as an example.
             * In general atleast 2 frames per channel need to queued
             * before starting capture,
             * else frame will get dropped until frames are queued
             */
            retVal = Fvid2_queue(instObj->drvHandle, &frmList, streamId);
            if (FVID2_SOK != retVal)
            {
                GT_0trace(BspAppTrace, GT_ERR,
                          APP_NAME ": Capture Queue Failed!!!\n");
                break;
            }
        }
        if (FVID2_SOK != retVal)
        {
            break;
        }
    }

    return (retVal);
}

/**
 *  \brief Free frames
 */
static Int32 CaptApp_freeFrames(CaptApp_Obj     *appObj,
                                CaptApp_InstObj *instObj,
                                UInt32           fieldMerged)
{
    Int32         retVal = FVID2_SOK;
    UInt32        streamId, chId, idx;
    Fvid2_Format *fmt;
    Fvid2_Frame  *frm;

    GT_assert(BspAppTrace,
              (instObj->createPrms.numStream <= VPS_CAPT_STREAM_ID_MAX));

    /* for every stream and channel in a capture handle */
    for (streamId = 0U; streamId < instObj->createPrms.numStream; streamId++)
    {
        for (chId = 0U; chId < instObj->createPrms.numCh; chId++)
        {
            fmt = &instObj->allocFmt[streamId];

            /* base index for instObj->frames[] */
            idx =
                (VPS_CAPT_CH_PER_PORT_MAX * CAPT_APP_FRAMES_PER_CH
                 * streamId) + (CAPT_APP_FRAMES_PER_CH * chId);

            if (idx >= CAPT_APP_MAX_FRAMES_PER_HANDLE)
            {
                idx = 0U;
            }
            frm = &instObj->frames[idx];

            retVal |= BspUtils_memFrameFree(fmt, frm, CAPT_APP_FRAMES_PER_CH);
            if (FVID2_SOK != retVal)
            {
                GT_0trace(BspAppTrace, GT_ERR,
                          APP_NAME ": Capture Frame Free Failed!!!\n");
            }
        }
    }

    return (retVal);
}

/**
 *  \brief Create and configure video decoder.
 */
static Int32 CaptApp_createVidDec(CaptApp_Obj     *appObj,
                                  CaptApp_InstObj *instObj)
{
    Int32  retVal;
    UInt32 decInstId, decI2cInstId, decI2cAddr;
    Vps_CaptCreateParams  *pVipCreatePrms;
    Bsp_VidDecChipIdParams decChipIdPrms;
    Bsp_VidDecChipIdStatus decChipIdStatus;
    Bsp_VidDecCreateStatus decCreateStatus;

    pVipCreatePrms = &instObj->createPrms;

    decInstId = Bsp_boardGetVideoDeviceInstId(
        instObj->decDrvId,
        FVID2_VPS_CAPT_VID_DRV,
        instObj->instId);
    decI2cInstId = Bsp_boardGetVideoDeviceI2cInstId(
        instObj->decDrvId,
        FVID2_VPS_CAPT_VID_DRV,
        instObj->instId);
    decI2cAddr = Bsp_boardGetVideoDeviceI2cAddr(
        instObj->decDrvId,
        FVID2_VPS_CAPT_VID_DRV,
        instObj->instId);

    /* Power on video decoder */
    retVal = Bsp_boardPowerOnDevice(instObj->decDrvId, decInstId, TRUE);
    if (FVID2_SOK != retVal)
    {
        GT_0trace(BspAppTrace, GT_ERR, "Device power-on failed\n");
    }
    if (FVID2_SOK == retVal)
    {
        /* select video decoder at board level mux */
        retVal = Bsp_boardSelectDevice(instObj->decDrvId, decInstId);
        if (FVID2_SOK != retVal)
        {
            GT_0trace(BspAppTrace, GT_ERR, "Device select failed\n");
        }
    }
    if (FVID2_SOK == retVal)
    {
        /* Perform any reset needed at board level */
        retVal = Bsp_boardResetDevice(instObj->decDrvId, decInstId);
        if (FVID2_SOK != retVal)
        {
            GT_0trace(BspAppTrace, GT_ERR, "Device reset failed\n");
        }
    }

    if (FVID2_SOK == retVal)
    {
        /* Select specific mode */
        retVal = Bsp_boardSelectMode(
            instObj->decDrvId,
            decInstId,
            appObj->testPrms.boardMode);
        if (FVID2_SOK != retVal)
        {
            GT_0trace(BspAppTrace, GT_ERR, "Device select mode failed\n");
        }
    }

    if ((FVID2_VID_DEC_TVP7002_DRV == instObj->decDrvId) &&
        (FVID2_SOK == retVal))
    {
        /* Enable THS filter */
        retVal = Bsp_boardEnableTvp7002Filter(
            decInstId,
            appObj->testPrms.standard);
        if (FVID2_SOK != retVal)
        {
            GT_0trace(BspAppTrace, GT_ERR, "THS filter programming failed\n");
        }
    }

    if (FVID2_SOK == retVal)
    {
        instObj->decCreatePrms.deviceI2cInstId    = decI2cInstId;
        instObj->decCreatePrms.numDevicesAtPort   = 1U;
        instObj->decCreatePrms.deviceI2cAddr[0]   = decI2cAddr;
        instObj->decCreatePrms.deviceResetGpio[0] = BSP_VID_DEC_GPIO_NONE;
        instObj->decHandle = Fvid2_create(
            instObj->decDrvId,
            decInstId,
            &instObj->decCreatePrms,
            &decCreateStatus,
            NULL);
        GT_assert(BspAppTrace, instObj->decHandle != NULL);

        decChipIdPrms.deviceNum = 0;
        retVal = Fvid2_control(
            instObj->decHandle,
            IOCTL_BSP_VID_DEC_GET_CHIP_ID,
            &decChipIdPrms,
            &decChipIdStatus);
        GT_assert(BspAppTrace, (retVal == FVID2_SOK));

        instObj->decVideoModePrms.videoIfWidth =
            pVipCreatePrms->videoIfWidth;
        instObj->decVideoModePrms.videoDataFormat = appObj->testPrms.inDataFmt;
        instObj->decVideoModePrms.standard        = appObj->testPrms.standard;
        instObj->decVideoModePrms.videoIfMode     = pVipCreatePrms->videoIfMode;
        /* TVP7002 supports only ACTVID/VSYNC mode of operation in single
         * channel
         * discrete sync mode */
        if ((instObj->decDrvId == FVID2_VID_DEC_TVP7002_DRV) &&
            (pVipCreatePrms->videoIfMode != FVID2_VIFM_SCH_ES))
        {
            instObj->decVideoModePrms.videoIfMode =
                FVID2_VIFM_SCH_DS_AVID_VSYNC;
        }
        instObj->decVideoModePrms.videoSystem =
            BSP_VID_DEC_VIDEO_SYSTEM_AUTO_DETECT;
        instObj->decVideoModePrms.videoAutoDetectTimeout =
            FVID2_TIMEOUT_FOREVER;
        instObj->decVideoModePrms.videoCropEnable = FALSE;

        GT_6trace(BspAppTrace, GT_INFO,
                  APP_NAME ": VIP %d: VID DEC %04x (0x%02x): %04x:%04x:%04x\n",
                  instObj->instId,
                  instObj->decDrvId,
                  instObj->decCreatePrms.deviceI2cAddr[0],
                  decChipIdStatus.chipId,
                  decChipIdStatus.chipRevision,
                  decChipIdStatus.firmwareVersion);

        CaptApp_detectVideo(appObj, instObj);
    }
    return (retVal);
}

/**
 *  \brief Create and configure video sensor.
 */
static Int32 CaptApp_createVidSensor(const CaptApp_Obj *appObj,
                                     CaptApp_InstObj   *instObj)
{
    Int32  retVal;
    UInt32 sensorInstId, sensorI2cInstId, sensorI2cAddr, boardId;
    Bsp_VidSensorChipIdParams sensorChipIdPrms;
    Bsp_VidSensorChipIdStatus sensorChipIdStatus;
    Bsp_VidSensorCreateStatus sensorCreateStatus;

    sensorInstId = Bsp_boardGetVideoDeviceInstId(
        instObj->decDrvId,
        FVID2_VPS_CAPT_VID_DRV,
        instObj->instId);
    sensorI2cInstId = Bsp_boardGetVideoDeviceI2cInstId(
        instObj->decDrvId,
        FVID2_VPS_CAPT_VID_DRV,
        instObj->instId);
    sensorI2cAddr = Bsp_boardGetVideoDeviceI2cAddr(
        instObj->decDrvId,
        FVID2_VPS_CAPT_VID_DRV,
        instObj->instId);

    GT_3trace(BspAppTrace, GT_DEBUG,
              " Sensor Inst ID-%d-Sensor I2C Inst ID %d,sensorI2cAddr: 0x%x\n",
              sensorInstId,
              sensorI2cInstId, sensorI2cAddr);

    /* Power on video sensor at board level mux */
    retVal = Bsp_boardPowerOnDevice(instObj->decDrvId, sensorInstId, TRUE);
    if (FVID2_SOK != retVal)
    {
        GT_0trace(BspAppTrace, GT_ERR, "Device power-on failed\n");
    }
    if (FVID2_SOK == retVal)
    {
        /* select video sensor at board level mux */
        retVal = Bsp_boardSelectDevice(instObj->decDrvId, sensorInstId);
        if (FVID2_SOK != retVal)
        {
            GT_0trace(BspAppTrace, GT_ERR, "Device select failed\n");
        }
    }

    if (FVID2_SOK == retVal)
    {
        /* Select specific mode */
        retVal = Bsp_boardSelectMode(
            instObj->decDrvId,
            sensorInstId,
            appObj->testPrms.boardMode);
        if (FVID2_SOK != retVal)
        {
            GT_0trace(BspAppTrace, GT_ERR, "Device select mode failed\n");
        }
    }
    if (FVID2_SOK == retVal)
    {
        boardId = Bsp_boardGetId();

        if ((BSP_BOARD_MULTIDES == boardId) &&
            (FVID2_VID_SENSOR_MULDES_OV1063X_DRV == instObj->decDrvId))
        {
            retVal = BspUtils_appConfSerDeSer(instObj->decDrvId, sensorInstId);
            if (retVal != FVID2_SOK)
            {
                GT_1trace(
                    BspAppTrace, GT_ERR,
                    APP_NAME
                    ": Configuring MultiDes instance %d failed!!!\n",
                    sensorInstId);
            }
        }

        instObj->sensorCreatePrms.deviceI2cInstId    = sensorI2cInstId;
        instObj->sensorCreatePrms.numDevicesAtPort   = 1U;
        instObj->sensorCreatePrms.deviceI2cAddr[0]   = sensorI2cAddr;
        instObj->sensorCreatePrms.deviceResetGpio[0] = BSP_VID_SENSOR_GPIO_NONE;
        instObj->sensorCreatePrms.sensorCfg          = NULL;
        instObj->sensorCreatePrms.numSensorCfg       = 0U;
        instObj->decHandle = Fvid2_create(
            instObj->decDrvId,
            sensorInstId,
            &instObj->sensorCreatePrms,
            &sensorCreateStatus,
            NULL);
        GT_assert(BspAppTrace, instObj->decHandle != NULL);

        sensorChipIdPrms.deviceNum = 0;
        retVal = Fvid2_control(
            instObj->decHandle,
            IOCTL_BSP_VID_SENSOR_GET_CHIP_ID,
            &sensorChipIdPrms,
            &sensorChipIdStatus);
        GT_assert(BspAppTrace, (retVal == FVID2_SOK));

        GT_6trace(
            BspAppTrace, GT_INFO,
            APP_NAME
            ": VIP %d: VID SENSOR %04x (0x%02x): %04x:%04x:%04x\n",
            instObj->instId,
            instObj->decDrvId,
            instObj->sensorCreatePrms.deviceI2cAddr[0],
            sensorChipIdStatus.chipId,
            sensorChipIdStatus.chipRevision,
            sensorChipIdStatus.firmwareVersion);

        /* Configure sensor in UYVY 8-bit 1280x720 @ 30FPS, OV only supports
         * 1280x720 @ 30FPS and VIP expects data in UYVY .
         * FOR MULDES_OV_DRV this IOCTL does nothing and returns SUCCESS. */
        if (((FVID2_VID_SENSOR_OV1063X_DRV == instObj->decDrvId) ||
             (FVID2_VID_SENSOR_LI_OV1063X_DRV == instObj->decDrvId) ||
             (FVID2_VID_SENSOR_MULDES_OV1063X_DRV == instObj->decDrvId)) &&
            (FVID2_SOK == retVal))
        {
            Bsp_VidSensorConfigParams configParams;
            configParams.videoIfWidth = appObj->testPrms.videoIfWidth;
            configParams.dataformat   = FVID2_DF_YUV422I_UYVY;
            configParams.standard     = appObj->testPrms.standard;
            configParams.fps          = FVID2_FPS_30;
            retVal =
                Fvid2_control(instObj->decHandle,
                              IOCTL_BSP_VID_SENSOR_SET_CONFIG,
                              &configParams,
                              NULL);
            GT_assert(BspAppTrace, (retVal == FVID2_SOK));
        }

        /* start sensor, right now only OV sensor driver is implemented */
        if (((FVID2_VID_SENSOR_OV1063X_DRV == instObj->decDrvId) ||
             (FVID2_VID_SENSOR_LI_OV1063X_DRV == instObj->decDrvId) ||
             (FVID2_VID_SENSOR_APT_AR0132_DRV == instObj->decDrvId) ||
             (FVID2_VID_SENSOR_MULDES_OV1063X_DRV == instObj->decDrvId)) &&
            (FVID2_SOK == retVal))
        {
            retVal = Fvid2_control(instObj->decHandle, FVID2_START, NULL, NULL);
            GT_assert(BspAppTrace, (retVal == FVID2_SOK));
        }
    }

    return (retVal);
}

static Int32 CaptApp_detectVideo(CaptApp_Obj     *appObj,
                                 CaptApp_InstObj *instObj)
{
    Int32  retVal;
    UInt32 chId, repeatCnt;
    Bsp_VidDecVideoStatusParams videoStatusPrms;
    Bsp_VidDecVideoStatus       videoStatus;

    GT_1trace(BspAppTrace, GT_INFO,
              APP_NAME ": Detect video in progress for inst %d !!!\n",
              instObj->instId);

    retVal = Fvid2_control(
        instObj->decHandle,
        IOCTL_BSP_VID_DEC_SET_VIDEO_MODE,
        &instObj->decVideoModePrms,
        NULL);
    GT_assert(BspAppTrace, (retVal == FVID2_SOK));

    for (chId = 0U; chId < instObj->createPrms.numCh; chId++)
    {
        repeatCnt = 5;
        while (--repeatCnt != 0)
        {
            videoStatusPrms.channelNum = chId;
            retVal = Fvid2_control(
                instObj->decHandle,
                IOCTL_BSP_VID_DEC_GET_VIDEO_STATUS,
                &videoStatusPrms,
                &videoStatus);
            GT_assert(BspAppTrace, (retVal == FVID2_SOK));

            if (videoStatus.isVideoDetect)
            {
                GT_5trace(
                    BspAppTrace, GT_INFO,
                    APP_NAME
                    ": Detected video at CH%d (%dx%d@%dHz, %d)!!!\n",
                    chId,
                    videoStatus.frameWidth,
                    videoStatus.frameHeight,
                    (1000000U / videoStatus.frameInterval),
                    videoStatus.isInterlaced);
                break;
            }
            BspOsal_sleep(100);
        }
    }

    GT_0trace(BspAppTrace, GT_INFO, APP_NAME ": Detect video Done !!!\n");

    return (FVID2_SOK);
}

static Int32 CaptApp_selectSimVideoSource(const CaptApp_Obj *appObj,
                                          CaptApp_InstObj   *instObj)
{
    UInt32 instId, fileId, pixelClk;

    /*
     * Select input source file,
     *
     * Assumes that the simulator VIP super file contents are like below
     *
     * 1 <user path>\output_bt656_QCIF.bin - 8 -bit YUV422 single CH input
     * 2 <user path>\output_bt1120_QCIF.bin - 16-bit YUV422 single CH input
     * 3 <user path>\output_bt1120_QCIF_RGB.bin - 24-bit RGB888 single CH input
     * 4 <user path>\output_tvp5158_8CH_bt656_QCIF.bin - 8-bit YUV422 multi
     * CH input
     */
    pixelClk = 1300 * 1000;

    switch (appObj->testPrms.videoIfMode)
    {
        case FVID2_VIFM_SCH_ES:
            switch (appObj->testPrms.videoIfWidth)
            {
                case FVID2_VIFW_8BIT:
                    fileId = 1;
                    break;

                case FVID2_VIFW_16BIT:
                    fileId = 2;
                    break;

                case FVID2_VIFW_24BIT:
                    fileId = 3;
                    break;

                default:
                    /* Default to 8-bit */
                    fileId = 1;
                    break;
            }
            break;

        case FVID2_VIFM_MCH_LINE_MUX_ES:
            fileId    = 4;
            pixelClk *= 8; /* assumes eight CHs in input file */
            break;

        default:
            /* Default to 8-bit */
            fileId = 1;
            break;
    }

    for (instId = 0U; instId < VPS_CAPT_INST_MAX; instId++)
    {
        Bsp_platformSimVideoInputSelect(instId, fileId, pixelClk);
    }

    return (FVID2_SOK);
}

/**
 *  CaptApp_initParams
 *  Initialize the global variables and frame pointers.
 */
static void CaptApp_initParams(CaptApp_Obj *appObj)
{
    UInt32 streamId, chId, instCnt, plId;
    Vps_CaptCreateParams *createPrms;
    Vps_CaptVipParams    *vipPrms;
    Vps_CaptVipScParams  *scPrms;
    Vps_CaptVipOutInfo   *outInfo;
    CaptApp_InstObj      *instObj;

    appObj->maxWidth      = 1920U;
    appObj->maxHeight     = 1080U;
    appObj->totalFldCount = 0U;
    appObj->totalCpuLoad  = 0U;
    appObj->cpuLoadCount  = 0U;
    appObj->callBackCount = 0U;

    for (instCnt = 0U; instCnt < appObj->testPrms.numHandles; instCnt++)
    {
        instObj            = &appObj->instObj[instCnt];
        instObj->instId    = appObj->testPrms.instId[instCnt];
        instObj->decDrvId  = appObj->testPrms.decDrvId[instCnt];
        instObj->drvHandle = NULL;
        Fvid2CbParams_init(&instObj->cbPrms);
        instObj->cbPrms.cbFxn   = &CaptApp_cbFxn;
        instObj->cbPrms.appData = instObj;
        instObj->appObj         = appObj;

        createPrms = &instObj->createPrms;
        VpsCaptCreateParams_init(&instObj->createPrms);
        createPrms->videoIfMode  = appObj->testPrms.videoIfMode;
        createPrms->videoIfWidth = appObj->testPrms.videoIfWidth;
        createPrms->bufCaptMode  = VPS_CAPT_BCM_LAST_FRM_REPEAT;
        createPrms->numCh        = appObj->testPrms.numCh;
        createPrms->numStream    = appObj->testPrms.numStream;

        vipPrms = &instObj->vipPrms;
        VpsCaptVipParams_init(&instObj->vipPrms);
        vipPrms->inFmt.chNum           = 0U;
        vipPrms->inFmt.width           = appObj->testPrms.inWidth;
        vipPrms->inFmt.height          = appObj->testPrms.inHeight;
        vipPrms->inFmt.pitch[0U]       = 0U;
        vipPrms->inFmt.pitch[1U]       = 0U;
        vipPrms->inFmt.pitch[2U]       = 0U;
        vipPrms->inFmt.fieldMerged[0U] = FALSE;
        vipPrms->inFmt.fieldMerged[1U] = FALSE;
        vipPrms->inFmt.fieldMerged[2U] = FALSE;
        vipPrms->inFmt.dataFormat      = appObj->testPrms.inDataFmt;
        vipPrms->inFmt.scanFormat      = FVID2_SF_PROGRESSIVE;
        vipPrms->inFmt.bpp      = FVID2_BPP_BITS16;
        vipPrms->inFmt.reserved = NULL;
        vipPrms->scPrms         = &instObj->scPrms;
        VpsCaptVipScParams_init(&instObj->scPrms);
        scPrms = &instObj->scPrms;
        scPrms->inCropCfg.cropStartX   = 0U;
        scPrms->inCropCfg.cropStartY   = 0U;
        scPrms->inCropCfg.cropWidth    = vipPrms->inFmt.width;
        scPrms->inCropCfg.cropHeight   = vipPrms->inFmt.height;
        scPrms->scCfg.bypass           = FALSE;
        scPrms->scCfg.nonLinear        = FALSE;
        scPrms->scCfg.stripSize        = 0U;
        scPrms->scCfg.enableEdgeDetect = TRUE;
        scPrms->scCfg.enablePeaking    = TRUE;
        scPrms->scCfg.advCfg           = NULL;
        scPrms->scCoeffCfg      = NULL;
        scPrms->enableCoeffLoad = FALSE;
        vipPrms->vipPortCfg     = NULL;

        Bsp_platformSetVipClkInversion(instObj->instId, FALSE);
        if (FVID2_VID_SENSOR_MT9V022_DRV == instObj->decDrvId)
        {
            vipPrms->vipPortCfg = &instObj->vipPortCfg;
            VpsVipPortConfig_init(&instObj->vipPortCfg);
            instObj->vipPortCfg.disCfg.actvidPol = FVID2_POL_LOW;
            instObj->vipPortCfg.disCfg.vsyncPol  = FVID2_POL_LOW;
            instObj->vipPortCfg.disCfg.hsyncPol  = FVID2_POL_HIGH;
        }
        else if ((FVID2_VID_SENSOR_OV1063X_DRV == instObj->decDrvId) ||
                 (FVID2_VID_SENSOR_LI_OV1063X_DRV == instObj->decDrvId) ||
                 (FVID2_VID_SENSOR_MULDES_OV1063X_DRV == instObj->decDrvId))
        {
            vipPrms->vipPortCfg = &instObj->vipPortCfg;
            VpsVipPortConfig_init(&instObj->vipPortCfg);
            instObj->vipPortCfg.disCfg.actvidPol = FVID2_POL_HIGH;
            instObj->vipPortCfg.disCfg.vsyncPol  = FVID2_POL_HIGH;
            instObj->vipPortCfg.disCfg.hsyncPol  = FVID2_POL_HIGH;

            /* Enable VIP cropping */
            instObj->vipPortCfg.actCropEnable     = TRUE;
            instObj->vipPortCfg.actCropCfg.srcNum = 0U;
            instObj->vipPortCfg.actCropCfg.cropCfg.cropStartX = 0U;
            instObj->vipPortCfg.actCropCfg.cropCfg.cropStartY = 0U;
            instObj->vipPortCfg.actCropCfg.cropCfg.cropWidth  = 1280U;
            instObj->vipPortCfg.actCropCfg.cropCfg.cropHeight = 720U;
        }
        else if (FVID2_VID_SENSOR_APT_AR0132_DRV == instObj->decDrvId)
        {
            vipPrms->vipPortCfg = &instObj->vipPortCfg;
            VpsVipPortConfig_init(&instObj->vipPortCfg);
            instObj->vipPortCfg.disCfg.actvidPol = FVID2_POL_LOW;
            instObj->vipPortCfg.disCfg.vsyncPol  = FVID2_POL_LOW;
            instObj->vipPortCfg.disCfg.hsyncPol  = FVID2_POL_HIGH;

            /* Enable VIP cropping */
            instObj->vipPortCfg.actCropEnable     = TRUE;
            instObj->vipPortCfg.actCropCfg.srcNum = 0U;
            instObj->vipPortCfg.actCropCfg.cropCfg.cropStartX = 0U;
            instObj->vipPortCfg.actCropCfg.cropCfg.cropStartY = 0U;
            instObj->vipPortCfg.actCropCfg.cropCfg.cropWidth  = 1280U;
            instObj->vipPortCfg.actCropCfg.cropCfg.cropHeight = 720U;
        }
        else if (FVID2_VID_DEC_SII9127_DRV == instObj->decDrvId)
        {
            vipPrms->vipPortCfg = &instObj->vipPortCfg;
            VpsVipPortConfig_init(&instObj->vipPortCfg);
            instObj->vipPortCfg.disCfg.actvidPol = FVID2_POL_HIGH;
            instObj->vipPortCfg.disCfg.vsyncPol  = FVID2_POL_HIGH;
            instObj->vipPortCfg.disCfg.hsyncPol  = FVID2_POL_HIGH;

            /* Disable VIP cropping */
            instObj->vipPortCfg.actCropEnable     = FALSE;
            instObj->vipPortCfg.actCropCfg.srcNum = 0U;
            instObj->vipPortCfg.actCropCfg.cropCfg.cropStartX = 0U;
            instObj->vipPortCfg.actCropCfg.cropCfg.cropStartY = 0U;
            instObj->vipPortCfg.actCropCfg.cropCfg.cropWidth  = 1920U;
            instObj->vipPortCfg.actCropCfg.cropCfg.cropHeight = 1080U;
        }
        else if (FVID2_VID_DEC_ADV7611_DRV == instObj->decDrvId)
        {
            vipPrms->vipPortCfg = &instObj->vipPortCfg;
            VpsVipPortConfig_init(&instObj->vipPortCfg);
            instObj->vipPortCfg.disCfg.actvidPol = FVID2_POL_HIGH;
            instObj->vipPortCfg.disCfg.vsyncPol  = FVID2_POL_LOW;
            instObj->vipPortCfg.disCfg.hsyncPol  = FVID2_POL_LOW;

            /* Disable VIP cropping */
            instObj->vipPortCfg.actCropEnable     = FALSE;
            instObj->vipPortCfg.actCropCfg.srcNum = 0U;
            instObj->vipPortCfg.actCropCfg.cropCfg.cropStartX = 0U;
            instObj->vipPortCfg.actCropCfg.cropCfg.cropStartY = 0U;
            instObj->vipPortCfg.actCropCfg.cropCfg.cropWidth  = 1920U;
            instObj->vipPortCfg.actCropCfg.cropCfg.cropHeight = 1080U;
        }
        else if (FVID2_VID_DEC_TVP5158_DRV == instObj->decDrvId)
        {
            vipPrms->vipPortCfg = &instObj->vipPortCfg;
            VpsVipPortConfig_init(&instObj->vipPortCfg);
            instObj->vipPortCfg.comCfg.pixClkEdgePol = FVID2_EDGE_POL_FALLING;
            Bsp_platformSetVipClkInversion(instObj->instId, TRUE);
        }
        else if (FVID2_VID_DEC_DS90UH926_DRV == instObj->decDrvId)
        {
            vipPrms->vipPortCfg = &instObj->vipPortCfg;
            VpsVipPortConfig_init(&instObj->vipPortCfg);
            instObj->vipPortCfg.disCfg.actvidPol     = FVID2_POL_HIGH;
            instObj->vipPortCfg.disCfg.vsyncPol      = FVID2_POL_LOW;
            instObj->vipPortCfg.disCfg.hsyncPol      = FVID2_POL_LOW;
            instObj->vipPortCfg.comCfg.pixClkEdgePol = FVID2_EDGE_POL_FALLING;
            Bsp_platformSetVipClkInversion(instObj->instId, TRUE);
        }
        vipPrms->cscCfg = NULL;

        for (streamId = 0U; streamId < appObj->testPrms.numStream; streamId++)
        {
            for (chId = 0U; chId < VPS_CAPT_CH_PER_PORT_MAX; chId++)
            {
                createPrms->chNumMap[streamId][chId] =
                    Vps_captMakeChNum(instObj->instId, streamId, chId);
            }
            outInfo = &vipPrms->outStreamInfo[streamId];
            outInfo->outFmt.chNum = 0U;
            outInfo->outFmt.width =
                appObj->testPrms.outWidth[streamId];
            outInfo->outFmt.height =
                appObj->testPrms.outHeight[streamId];
            outInfo->outFmt.fieldMerged[0U] = FALSE;
            outInfo->outFmt.fieldMerged[1U] = FALSE;
            outInfo->outFmt.fieldMerged[2U] = FALSE;
            outInfo->outFmt.dataFormat      =
                appObj->testPrms.outDataFmt[streamId];
            outInfo->outFmt.scanFormat = FVID2_SF_PROGRESSIVE;
            outInfo->outFmt.bpp        = FVID2_BPP_BITS16;
            outInfo->outFmt.pitch[0U]  = 0U;
            outInfo->outFmt.pitch[1U]  = 0U;
            outInfo->outFmt.pitch[2U]  = 0U;
            if (Fvid2_isDataFmtSemiPlanar(outInfo->outFmt.dataFormat))
            {
                outInfo->outFmt.pitch[FVID2_YUV_SP_Y_ADDR_IDX] =
                    VpsUtils_align(appObj->maxWidth, VPS_BUFFER_ALIGNMENT);
                outInfo->outFmt.pitch[FVID2_YUV_SP_CBCR_ADDR_IDX] =
                    outInfo->outFmt.pitch[FVID2_YUV_SP_Y_ADDR_IDX];
            }
            else if (Fvid2_isDataFmtYuv422I(outInfo->outFmt.dataFormat) ||
                     (FVID2_DF_RAW16 == outInfo->outFmt.dataFormat))
            {
                outInfo->outFmt.pitch[FVID2_YUV_INT_ADDR_IDX] =
                    VpsUtils_align(appObj->maxWidth * 2U, VPS_BUFFER_ALIGNMENT);
            }
            else if (FVID2_DF_RAW08 == outInfo->outFmt.dataFormat)
            {
                outInfo->outFmt.pitch[FVID2_YUV_INT_ADDR_IDX] =
                    VpsUtils_align(appObj->maxWidth, VPS_BUFFER_ALIGNMENT);
            }
            else
            {
                /* Align the pitch to BPP boundary as well since the pitch
                 * aligined to VPS_BUFFER_ALIGNMENT may not be multiple of 3
                 * bytes (1 pixel) */
                outInfo->outFmt.pitch[FVID2_RGB_ADDR_IDX] =
                    VpsUtils_align(
                        appObj->maxWidth * 3U, (VPS_BUFFER_ALIGNMENT * 3U));
            }
            outInfo->bufFmt  = FVID2_BUF_FMT_FRAME;
            outInfo->memType = VPS_VPDMA_MT_NONTILEDMEM;
            if (!Bsp_platformIsTI814xFamilyBuild())
            {
                outInfo->maxOutWidth[0U]  = VPS_VPDMA_MAX_OUT_WIDTH_REG1;
                outInfo->maxOutHeight[0U] = VPS_VPDMA_MAX_OUT_HEIGHT_REG1;
                for (plId = 1U; plId < FVID2_MAX_PLANES; plId++)
                {
                    outInfo->maxOutWidth[plId]  = VPS_VPDMA_MAX_OUT_WIDTH_REG2;
                    outInfo->maxOutHeight[plId] =
                        VPS_VPDMA_MAX_OUT_HEIGHT_REG2;
                }
            }
            else
            {
                for (plId = 0U; plId < FVID2_MAX_PLANES; plId++)
                {
                    outInfo->maxOutWidth[plId] =
                        VPS_VPDMA_MAX_OUT_WIDTH_1920_PIXELS;
                    outInfo->maxOutHeight[plId] =
                        VPS_VPDMA_MAX_OUT_HEIGHT_1080_LINES;
                }
            }

            outInfo->scEnable = FALSE;
            if (CAPT_APP_SC_ENABLE_ALL == appObj->testPrms.scEnable)
            {
                outInfo->scEnable = TRUE;
            }
            if ((0U == streamId) &&
                (CAPT_APP_SC_ENABLE_0 == appObj->testPrms.scEnable))
            {
                outInfo->scEnable = TRUE;
            }
            if ((1U == streamId) &&
                (CAPT_APP_SC_ENABLE_1 == appObj->testPrms.scEnable))
            {
                outInfo->scEnable = TRUE;
            }
            outInfo->subFrmPrms.subFrameEnable      = FALSE;
            outInfo->subFrmPrms.numLinesPerSubFrame = 0U;
            outInfo->subFrmPrms.subFrameCb          = NULL;
        }

        instObj->maxOutWidth[0U]  = CAPT_APP_MAXSIZE_1_WIDTH;
        instObj->maxOutHeight[0U] = CAPT_APP_MAXSIZE_1_HEIGHT;
        instObj->maxOutWidth[1U]  = CAPT_APP_MAXSIZE_2_WIDTH;
        instObj->maxOutHeight[1U] = CAPT_APP_MAXSIZE_2_HEIGHT;
        instObj->maxOutWidth[2U]  = CAPT_APP_MAXSIZE_3_WIDTH;
        instObj->maxOutHeight[2U] = CAPT_APP_MAXSIZE_3_HEIGHT;
    }

    return;
}

static void CaptApp_printStatistics(CaptApp_Obj *appObj,
                                    UInt32       execTimeInMsecs)
{
    UInt32           instCnt;
    CaptApp_InstObj *instObj;

    GT_0trace(BspAppTrace, GT_INFO, " \r\n");
    GT_0trace(BspAppTrace, GT_INFO, " Execution Statistics \r\n");
    GT_0trace(BspAppTrace, GT_INFO, " ==================== \r\n");
    GT_0trace(BspAppTrace, GT_INFO, " \r\n");
    GT_2trace(BspAppTrace, GT_INFO,
              " Execution time    : %d.%d s \r\n",
              (execTimeInMsecs / 1000), (execTimeInMsecs % 1000));
    GT_2trace(BspAppTrace, GT_INFO,
              " Total field Count : %d (%d fields/sec) \r\n",
              appObj->totalFldCount,
              (appObj->totalFldCount * 1000) / execTimeInMsecs);
    GT_1trace(BspAppTrace, GT_INFO,
              " Avg CPU Load      : %d %% \r\n",
              appObj->totalCpuLoad / appObj->cpuLoadCount);
    GT_0trace(BspAppTrace, GT_INFO, " \r\n");

    for (instCnt = 0U; instCnt < appObj->testPrms.numHandles; instCnt++)
    {
        instObj = &appObj->instObj[instCnt];

        /* Print the driver status */
        BspUtils_appPrintCaptStatus(
            instObj->drvHandle,
            instObj->instId,
            instObj->createPrms.numStream,
            instObj->createPrms.numCh,
            execTimeInMsecs);
    }

    GT_0trace(BspAppTrace, GT_INFO, " \r\n");

    return;
}

/**
 *  CaptApp_getTestId
 *  Return the test ID to run.
 */
static Int32 CaptApp_getTestId(CaptApp_Obj *appObj)
{
    UInt32       testCnt;
    static Int32 testId = 0;

    GT_0trace(BspAppTrace, GT_INFO, " \n");
    GT_0trace(BspAppTrace, GT_INFO,
              "--------------------------------------\n");
    GT_0trace(BspAppTrace, GT_INFO,
              "Select test to run as per below table:\n");
    GT_0trace(BspAppTrace, GT_INFO,
              "--------------------------------------\n");
    GT_0trace(BspAppTrace, GT_INFO, " \n");
    for (testCnt = 0; testCnt < CAPT_APP_NUM_TESTS; testCnt++)
    {
        GT_2trace(BspAppTrace, GT_INFO,
                  "%3d: %s\n", testCnt, gCaptAppTestPrms[testCnt].testDescStr);
    }
    GT_2trace(BspAppTrace, GT_INFO,
              "%3d: %s\n", CAPT_APP_NUM_TESTS, "Auto Run\n");
    GT_1trace(BspAppTrace, GT_INFO, "%3d: EXIT\n", 100);
    GT_0trace(BspAppTrace, GT_INFO, " \n");
    GT_0trace(BspAppTrace, GT_INFO, "Enter Test to Run: \n");

    if (TRUE == appObj->enableAutoRun)
    {
        testId++;
        if (testId < CAPT_APP_NUM_TESTS)
        {
            GT_1trace(BspAppTrace, GT_INFO, "%d\n", testId);
        }
    }
    else
    {
        while (1U)
        {
            BspOsal_scanf("%d", &testId);
            Bsp_printf("%d\n", testId);
            if ((testId >= 0) && (testId < CAPT_APP_NUM_TESTS))
            {
                break;
            }
            else if (testId == CAPT_APP_NUM_TESTS)
            {
                appObj->enableAutoRun = (UInt32) TRUE;
                testId = 0;
                break;
            }
            else if (100 == testId)
            {
                break;
            }
            GT_0trace(BspAppTrace, GT_INFO, "Invalid Test ID. Enter Agian!!\n");
        }
    }

    return (testId);
}

