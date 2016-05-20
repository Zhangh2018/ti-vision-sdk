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
 *  \file   mcspiSample_main.c
 *
 *  \brief   McSPI sample application
 *
 *          This file demonstrates the use of Mcspi by Connecting
 *          SPI pins D0 and D1 externally to demonstrate data loopback.
 *
 *  \author     PDP team
 *
 */

/* ========================================================================== */
/*                          INCLUDE FILES                                     */
/* ========================================================================== */

#include <xdc/std.h>
#include <stdio.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/io/GIO.h>
#include <ti/sysbios/io/IOM.h>
#include <xdc/runtime/Log.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/heaps/HeapMem.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>

#ifndef TI814X_FAMILY_BUILD

#include <ti/sdo/edma3/drv/edma3_drv.h>

#include <common/bsp_types.h>
#include <common/trace.h>
#include <common/bsp_config.h>
#include <common/bsp_utils.h>
#include <common/bsp_common.h>
#include "mcspi/bsp_mcspi.h"
#include "platforms/bsp_platform.h"
#include <boards/bsp_board.h>
#include <devices/bsp_device.h>
#include <i2c/bsp_i2c.h>
#include <fvid2/fvid2.h>
#include <utility/bsputils_mem.h>
#include <utility/bsputils_prf.h>
#include <utility/bsputils_app.h>

/* ========================================================================== */
/*                       IMPORTED FUNCTIONS                                   */
/* ========================================================================== */

extern void start_spi_sample(void);
extern void spi_initParams(void);

void spiSampleTask(UArg arg1, UArg arg2);

/* ========================================================================== */
/*                          LOCAL FUNCTION PROTOTYPE                          */
/* ========================================================================== */

static void tskHeartBeat(void);
static Int32 App_mcspiDefaultInit(UInt32 isI2cInitReq);
static Int32 App_mcspiDefaultDeInit(UInt32 isI2cDeInitReq);
#ifdef TDA3XX_FAMILY_BUILD
static void PlatformMCSPI1PrcmEnable(void);
static void PlatformMCSPI2PrcmEnable(void);
#endif

/* ========================================================================== */
/*                             GLOBAL VARIABLES                               */
/* ========================================================================== */

/**
 *  \brief I2C Device Init Params used for storing init Params handle,
 *  Semaphore handle, etc.
 */
static I2c_DevInitParams gI2cDevInitParams[BSP_DEVICE_I2C_INST_ID_MAX];

UInt32       gIsI2cInitReq;

Mcspi_Params mcspiPrms;
Int32        inst_num;

/* Global function prototypes */
void user_mcspi_init(void);

/* ========================================================================== */
/*                           FUNCTION DEFINITIONS                             */
/* ========================================================================== */

/**
 * \brief      Entry point for the main task. This creates the required channels
 *             and calls the required functions for demonstrating the SPI
 *             driver
 *
 * \param      arg1  [IN]   unused
 * \param      arg2  [IN]   unused
 *
 * \return     None
 */
void spiSampleTask(UArg arg1, UArg arg2)
{
    Int32 retVal = FVID2_SOK;
    Bsp_PlatformInitParams platInitPrms;

    /* isPinMuxSettingReq parameter will be initialised to TRUE */
    BspPlatformInitParams_init(&platInitPrms);

    platInitPrms.isAllMcSPIInitReq  = TRUE;
    platInitPrms.isPinMuxSettingReq = TRUE;

    /* Initialize pinmux and evm related configurations */
    Bsp_platformInit(&platInitPrms);
    /* System init */
    gIsI2cInitReq = TRUE;
    retVal       = App_mcspiDefaultInit(gIsI2cInitReq);
    if (retVal != FVID2_SOK)
    {
        System_printf("Error: : System Init Failed!!!\n");
    }

    /* Set the pin Mux */
    Bsp_boardSetPinMux(BSP_DRV_ID_MCSPI, BSP_DEVICE_MCSPI_INST_ID_0,
                       BSP_BOARD_MODE_DEFAULT);
    Bsp_boardSetPinMux(BSP_DRV_ID_MCSPI, BSP_DEVICE_MCSPI_INST_ID_1,
                       BSP_BOARD_MODE_DEFAULT);

    /* Set the board muxes */
    Bsp_boardSelectDevice(BSP_DRV_ID_MCSPI, BSP_DEVICE_MCSPI_INST_ID_0);
    Bsp_boardSelectDevice(BSP_DRV_ID_MCSPI, BSP_DEVICE_MCSPI_INST_ID_1);
#ifdef TDA3XX_FAMILY_BUILD
    /* Enable PRCM for the McSPI */
    PlatformMCSPI1PrcmEnable();
    PlatformMCSPI2PrcmEnable();
#endif

    /* call the function for the SPI application test                     */

    start_spi_sample();

    App_mcspiDefaultDeInit(gIsI2cInitReq);

    /* Start the Heart Beat Print                                         */
    tskHeartBeat();

    return;
}

/**
 * \brief     Main entry function for the spi sample application. it creates
 *            the spi sample task(used to demonstrate the spi functionality).
 *
 * \param     None
 *
 * \return    None
 *
 */
extern const IOM_Fxns Mcspi_IOMFXNS;
Int32 main(void)
{
    System_printf("Running McSPI Loopback test.\n");
#ifndef TI814X_FAMILY_BUILD
    System_printf(
        "Enter McSPI instance on which test to be run\
        \n0 - McSPI1 instance\n1 - McSPI2 instance\n$>");
    scanf("%d", &inst_num);
    if (inst_num != 0 && inst_num != 1)
    {
        inst_num = 0;
    }
#else
    /* Instance number is 1 for TI814x */
    inst_num = 1;
#endif
    GIO_addDevice("/mcspi0", (xdc_Ptr) & Mcspi_IOMFXNS, &user_mcspi_init,
                  inst_num, (xdc_Ptr) & mcspiPrms);
    Task_create((Task_FuncPtr) spiSampleTask, NULL, NULL);
    BIOS_start();

    return (0);
}

#ifdef TDA3XX_FAMILY_BUILD
static void PlatformMCSPI1PrcmEnable(void)
{
    *((volatile UInt32 *) (0x4a009700 + 0xf0)) = 0x02;
    while ((*((volatile UInt32 *) (0x4a009700 + 0xf0))) != 0x02) ;
}

static void PlatformMCSPI2PrcmEnable(void)
{
    *((volatile UInt32 *) (0x4a009700 + 0xf8)) = 0x02;
    while ((*((volatile UInt32 *) (0x4a009700 + 0xf8))) != 0x02) ;
}

#endif
/*
 * McSPI0 init function called when creating the driver.
 */
void user_mcspi_init(void)
{
    Int32 opMode;
    Mcspi_init();

    mcspiPrms = Mcspi_PARAMS;
    System_printf("Please Connect the McSPI%d MISO (DAT0) to MOSI (DAT1)\n",
                  (inst_num + 1U));
    System_printf(
        "And then enter the Operating mode\n0 - Polled Mode\
            \n1 - Interrupt Mode\
            \n2 - DMA Interrupt mode\n$>");
    scanf("%d", &opMode);
    if (opMode == 0)
    {
        mcspiPrms.opMode = MCSPI_OPMODE_POLLED;
    }
    else if (opMode == 1)
    {
        mcspiPrms.opMode = MCSPI_OPMODE_INTERRUPT;
    }
    else if (opMode == 2)
    {
        mcspiPrms.opMode = MCSPI_OPMODE_DMAINTERRUPT;
    }
    else
    {
        mcspiPrms.opMode = MCSPI_OPMODE_POLLED;
    }

    mcspiPrms.instNum         = inst_num;
    mcspiPrms.hwiNumber       = 7;
    mcspiPrms.enableCache     = (UInt32) TRUE;
    mcspiPrms.edma3EventQueue = 0;
    mcspiPrms.enableErrIntr   = (UInt32) FALSE;

    mcspiPrms.spiHWCfgData.masterOrSlave         = MCSPI_COMMMODE_MASTER;
    mcspiPrms.spiHWCfgData.singleOrMultiChEnable = MCSPI_SINGLE_CHANNEL;
    mcspiPrms.spiHWCfgData.pinOpModes    = MCSPI_PINOPMODE_4PIN;
    mcspiPrms.spiHWCfgData.fifoRxTrigLvl = 32;
    mcspiPrms.spiHWCfgData.fifoTxTrigLvl = 32;
    mcspiPrms.spiHWCfgData.configChfmt[0].charLength = MCSPI_LEN_8BIT;
    mcspiPrms.spiHWCfgData.configChfmt[0].multiWordAccessEnable =
        (UInt32) FALSE;
    mcspiPrms.spiHWCfgData.configChfmt[0].spiChipSelectEnablePol =
        (UInt32) FALSE;
    mcspiPrms.spiHWCfgData.configChfmt[0].clockMode           = MCSPI_MODE0;
    mcspiPrms.spiHWCfgData.configChfmt[0].clockRatioExtension = 0;
    mcspiPrms.spiHWCfgData.configChfmt[0].spiWordInitDelay    = MCSPI_NO_DELAY;
    mcspiPrms.spiHWCfgData.configChfmt[0].trasmitReceiveMode  = MCSPI_BOTH_RXTX;
    mcspiPrms.spiHWCfgData.configChfmt[0].granularityEnable   = (UInt32) TRUE;
    mcspiPrms.spiHWCfgData.configChfmt[0].busFreq             = 1000000;
    mcspiPrms.spiHWCfgData.configChfmt[0].spienHighPolarity   = (UInt32) FALSE;
    mcspiPrms.spiHWCfgData.configChfmt[0].slaveModeChipSelect = MCSPI_SPIEN_0;
    mcspiPrms.spiHWCfgData.configChfmt[0].spiDat0Dir          = MCSPI_IN;
    mcspiPrms.spiHWCfgData.configChfmt[0].spiDat1Dir          = MCSPI_OUT;

    if (MCSPI_OPMODE_POLLED == mcspiPrms.opMode)
    {
        System_printf("\r\nMcspi is configured in polled mode\r\n");
    }
    else if (MCSPI_OPMODE_INTERRUPT == mcspiPrms.opMode)
    {
        System_printf("\r\nMcspi is configured in interrupt mode\r\n");
    }
    else if (MCSPI_OPMODE_DMAINTERRUPT == mcspiPrms.opMode)
    {
        System_printf("\r\nMcspi is configured in dma mode\r\n");
    }
    else
    {
        System_printf("\r\nError: unknown mode of operation!!!!!!!!!!\r\n");
    }
}

Ptr NULLIOM_Params = NULL;

void user_prcm_init(void)
{
    /* Do Nothing */
}

/*
 * Function to print an log to the console every one second(Approx)
 */
static void tskHeartBeat(void)
{
    static Uint32 counter = 0;

    while (counter < 0xFFFFFFFF)
    {
        Task_sleep(1000U);

        System_printf("\r\n\r\n!!! PDP HrtBt %d", counter);
        counter++;
    }
}

/**
 *  App_mcspiDefaultInit
 *  \brief Application utils function to call all the default init functions.
 */
static Int32 App_mcspiDefaultInit(UInt32 isI2cInitReq)
{
    Int32  retVal = BSP_SOK;
    UInt32 instCnt;
    Bsp_CommonInitParams    commonInitPrms;
    Bsp_BoardInitParams     boardInitPrms;
    Bsp_PlatformInitParams  platInitPrms;
    Bsp_DeviceInitParams    deviceInitPrms;
    lld_hsi2c_initParam_t   i2cInitParams[BSP_DEVICE_I2C_INST_ID_MAX];
    const Bsp_BoardI2cData *i2cData;
    Bsp_BoardI2cInstData   *i2cInstData;
    BspCommonInitParams_init(&commonInitPrms);
    retVal += Bsp_commonInit(&commonInitPrms);
    if (BSP_SOK != retVal)
    {
        GT_0trace(BspAppTrace, GT_ERR, "Error: BSP Common Init failed!!\n");
    }

    BspBoardInitParams_init(&boardInitPrms);
    /* Override board detection if I2C is disabled */
    if (((Bool) TRUE) != ((Bool) isI2cInitReq))
    {
        boardInitPrms.boardId      = BSP_BOARD_UNKNOWN;
        boardInitPrms.baseBoardRev = BSP_BOARD_REV_UNKNOWN;
        boardInitPrms.dcBoardRev   = BSP_BOARD_REV_UNKNOWN;
    }
    retVal += Bsp_boardInit(&boardInitPrms);
    if (BSP_SOK != retVal)
    {
        GT_0trace(BspAppTrace, GT_ERR, "Error: Board Init failed!!\n");
    }

    BspPlatformInitParams_init(&platInitPrms);
    retVal += Bsp_platformInit(&platInitPrms);
    if (BSP_SOK != retVal)
    {
        GT_0trace(BspAppTrace, GT_ERR, "Error: Platform Init failed!!\n");
    }

    retVal += Fvid2_init(NULL);
    if (BSP_SOK != retVal)
    {
        GT_0trace(BspAppTrace, GT_ERR, "Error: FVID2 Init failed!!\n");
    }

    /* Override I2C init for non-EVM builds */
    if (BSP_PLATFORM_ID_EVM != Bsp_platformGetId())
    {
        isI2cInitReq = FALSE;
    }

    if (((Bool) TRUE) == ((Bool) isI2cInitReq))
    {
        i2cData = Bsp_boardGetI2cData();
        GT_assert(BspAppTrace, (NULL != i2cData));
        GT_assert(BspAppTrace,
                  (i2cData->numInst <= BSP_DEVICE_I2C_INST_ID_MAX));
        GT_assert(BspAppTrace, (NULL != i2cData->instData));

        for (instCnt = 0U; instCnt < i2cData->numInst; instCnt++)
        {
            i2cInstData = &i2cData->instData[instCnt];
            GT_assert(BspAppTrace,
                      (i2cInstData->instId < BSP_DEVICE_I2C_INST_ID_MAX));
            i2cInitParams[instCnt].opMode       = HSI2C_OPMODE_INTERRUPT;
            i2cInitParams[instCnt].isMasterMode = TRUE;
            i2cInitParams[instCnt].is10BitAddr  = FALSE;
            i2cInitParams[instCnt].i2cBusFreq   =
                (lld_i2c_busspeed) i2cInstData->busClkKHz;
            i2cInitParams[instCnt].i2cIntNum      = i2cInstData->intNum;
            i2cInitParams[instCnt].i2cOwnAddr     = 0xCC;
            gI2cDevInitParams[instCnt].initParams = &i2cInitParams[instCnt];
            gI2cDevInitParams[instCnt].hsi2c_sem  =
                BspOsal_semCreate((Int32) 1, (Bool) TRUE);
            gI2cDevInitParams[instCnt].instId = i2cInstData->instId;
        }

        if (i2cData->numInst > 0)
        {
            retVal = I2c_GlobalInit(i2cData->numInst, &gI2cDevInitParams[0U]);
            if (BSP_SOK != retVal)
            {
                GT_0trace(BspAppTrace, GT_ERR, "Error: I2C Init failed!!\n");
            }
        }

        BspDeviceInitParams_init(&deviceInitPrms);
        deviceInitPrms.isI2cProbingReq = FALSE;
        retVal += Bsp_deviceInit(&deviceInitPrms);
        if (BSP_SOK != retVal)
        {
            GT_0trace(BspAppTrace, GT_ERR, "Error: Device Init failed!!\n");
        }
    }

    retVal += BspUtils_memInit();
    if (BSP_SOK != retVal)
    {
        GT_0trace(BspAppTrace, GT_ERR, "Error: App MEM Utils Init failed!!\n");
    }

    retVal += BspUtils_prfInit();
    if (BSP_SOK != retVal)
    {
        GT_0trace(BspAppTrace, GT_ERR, "Error: App PRF Utils Init failed!!\n");
    }

    retVal += BspUtils_appInit();
    if (BSP_SOK != retVal)
    {
        GT_0trace(BspAppTrace, GT_ERR, "Error: App Utils Init failed!!\n");
    }

    /* Print BSP version string and platform info*/
    GT_0trace(BspAppTrace, GT_INFO, " \r\n");
    GT_2trace(BspAppTrace, GT_INFO,
              " Build time            : [%s %s]\r\n", __TIME__, __DATE__);
    GT_1trace(BspAppTrace, GT_INFO,
              " BSP Version           : [%s]\r\n", Bsp_getVersionString());
    Bsp_platformPrintInfo();
    Bsp_boardPrintInfo();
    GT_0trace(BspAppTrace, GT_INFO, " \r\n");

    return (retVal);
}

/**
 *  App_mcspiDefaultDeInit
 *  \brief Application utils function to call all the default de-init
 *  functions.
 */
static Int32 App_mcspiDefaultDeInit(UInt32 isI2cDeInitReq)
{
    Int32  retVal = BSP_SOK;
    const Bsp_BoardI2cData *i2cData;
    UInt32 instCnt;

    retVal += BspUtils_appDeInit();
    if (BSP_SOK != retVal)
    {
        GT_0trace(BspAppTrace, GT_ERR, "Error: App Utils De-Init failed!!\n");
    }

    retVal += BspUtils_prfDeInit();
    if (BSP_SOK != retVal)
    {
        GT_0trace(BspAppTrace, GT_ERR,
                  "Error: App PRF Utils De-Init failed!!\n");
    }

    retVal += BspUtils_memDeInit();
    if (BSP_SOK != retVal)
    {
        GT_0trace(BspAppTrace, GT_ERR,
                  "Error: App MEM Utils De-Init failed!!\n");
    }

    /* Override I2C de-init for non-EVM builds */
    if (BSP_PLATFORM_ID_EVM != Bsp_platformGetId())
    {
        isI2cDeInitReq = FALSE;
    }

    if (((Bool) TRUE) == ((Bool) isI2cDeInitReq))
    {
        i2cData = Bsp_boardGetI2cData();
        GT_assert(BspAppTrace, (NULL != i2cData));
        GT_assert(BspAppTrace,
                  (i2cData->numInst <= BSP_DEVICE_I2C_INST_ID_MAX));

        retVal += Bsp_deviceDeInit(NULL);
        if (BSP_SOK != retVal)
        {
            GT_0trace(BspAppTrace, GT_ERR, "Error: Device De-Init failed!!\n");
        }
        if (i2cData->numInst > 0)
        {
            for (instCnt = 0U; instCnt < i2cData->numInst; instCnt++)
            {
                BspOsal_semDelete(&(gI2cDevInitParams[instCnt].hsi2c_sem));
            }
            retVal += I2c_GlobalDeInit(NULL);
            if (BSP_SOK != retVal)
            {
                GT_0trace(BspAppTrace, GT_ERR, "Error: I2C De-Init failed!!\n");
            }
        }
    }

    retVal += Fvid2_deInit(NULL);
    if (BSP_SOK != retVal)
    {
        GT_0trace(BspAppTrace, GT_ERR, "Error: FVID2 De-Init failed!!\n");
    }

    retVal += Bsp_platformDeInit(NULL);
    if (BSP_SOK != retVal)
    {
        GT_0trace(BspAppTrace, GT_ERR,
                  "Error: BSP Platform De-Init failed!!\n");
    }

    retVal += Bsp_boardDeInit(NULL);
    if (BSP_SOK != retVal)
    {
        GT_0trace(BspAppTrace, GT_ERR, "Error: BSP Board De-Init failed!!\n");
    }

    retVal += Bsp_commonDeInit(NULL);
    if (BSP_SOK != retVal)
    {
        GT_0trace(BspAppTrace, GT_ERR, "Error: BSP Common De-Init failed!!\n");
    }

    return (retVal);
}

#else
/*
 * Else Section for Build Compatibility of  previous silicon version TI814x
 */

/**
 * \brief     Main entry function for the spi sample application. it creates
 *            the spi sample task(used to demonstrate the spi functionality).
 *
 * \param     None
 *
 * \return    None
 *
 */
Int32 main(void)
{
    BIOS_start();

    return (0);
}

#endif

/* ========================================================================== */
/*                                END OF FILE                                 */
/* ========================================================================== */

