/* =================================================================
 *   Copyright (C) 2013 Texas Instruments Incorporated
 *
 *   All rights reserved. Property of Texas Instruments Incorporated.
 *   Restricted rights to use, duplicate or disclose this code are
 *   granted through contract.
 *
 *   The program may not be used without the written permission
 *   of Texas Instruments Incorporated or against the terms and
 *   conditionsstipulated in the agreement under which this program
 *   has been supplied.
 * =================================================================*/

/*
 * \file bl_hsmmcsd.c
 *
 * \brief  HSMMCSD Initialization functions.
 *         And a funciton to copy data from card to the given address.
 *
 */

#include <stdint.h>
#include <string.h>
#include "hw_types.h"
#include "ff.h"
#include "soc.h"
#include "hs_mmcsd.h"
#include "mmcsd_proto.h"
#include "hs_mmcsdlib.h"
#include "soc_defines.h"
#include "platform.h"
#include "hsmmcsd_API.h"
#include "hsmmcsd_edma.h"

/******************************************************************************
**                      TYPE DEFINITIONS
*******************************************************************************/

/* Fat devices registered */
#ifndef fatDevice
typedef struct _fatDevice
{
    /* Pointer to underlying device/controller */
    void        *dev;

    /* File system pointer */
    FATFS       *fs;

    /* state */
    uint32_t initDone;
}fatDevice;
#endif
extern fatDevice        fat_devices[2];
extern uint32_t     entryPoint;
/******************************************************************************
**                      INTERNAL MACRO DEFINITIONS
*******************************************************************************/

/* Global flags for interrupt handling */
volatile uint32_t   xferPend    = 0;
volatile uint32_t   cmdCompFlag = 0;
volatile uint32_t   cmdTimeout  = 0;
volatile uint32_t   errFlag     = 0;
volatile uint32_t   sdBlkSize   = MMCSD_BLK_SIZE;

/* SD card info structure */
#ifdef __IAR_SYSTEMS_ICC__

#pragma data_alignment=32
mmcsdCardInfo           sdCard;
#pragma data_alignment=32
mmcsdCtrlInfo           ctrlInfo;
#pragma data_alignment=32
mmcsdAddrConv           addrInfo;

#elif defined (__TMS470__)

#ifdef __cplusplus
#pragma DATA_ALIGN(32);
#else
#pragma DATA_ALIGN(sdCard, 32);
#endif  /* #ifdef __cplusplus */
mmcsdCardInfo           sdCard;

#ifdef __cplusplus
#pragma DATA_ALIGN(32);
#else
#pragma DATA_ALIGN(ctrlInfo, 32);
#endif  /* #ifdef __cplusplus */
mmcsdCtrlInfo           ctrlInfo;

#ifdef __cplusplus
#pragma DATA_ALIGN(32);
#else
#pragma DATA_ALIGN(addrInfo, 32);
#endif  /* #ifdef __cplusplus */
mmcsdAddrConv           addrInfo;

#else

mmcsdCardInfo           sdCard __attribute__ ((aligned(32)));
mmcsdCtrlInfo           ctrlInfo __attribute__ ((aligned(32)));
mmcsdAddrConv           addrInfo __attribute__ ((aligned(32)));

#endif

/******************************************************************************
**                      INTERNAL FUNCTION PROTOTYPES
*******************************************************************************/
static uint32_t HSMMCSDStatusGet(void);

/******************************************************************************
**                      INTERNAL VARIABLE DEFINITIONS
*******************************************************************************/
uint32_t            hsmmcsd_dataLen = 0;
volatile unsigned char *hsmmcsd_buffer  = NULL;

/*****************************************************************************
 * Defines the size of the buffer that holds the command line.
 ******************************************************************************/
#define CMD_BUF_SIZE    512

/*****************************************************************************
 * Current FAT fs state.
 ******************************************************************************/
static FATFS            g_sFatFs;
// static FIL g_sFileObject;

/*****************************************************************************
 * A structure that holds a mapping between an FRESULT numerical code,
 * and a string representation.  FRESULT codes are returned from the FatFs
 * FAT file system driver.
 *
 ******************************************************************************/
typedef struct
{
    FRESULT fresult;
    char   *pcResultStr;
}
tFresultString;
/*****************************************************************************
 * A macro to make it easy to add result codes to the table.
 ******************************************************************************/
#define FRESULT_ENTRY(f)        {(f), (# f)}

/*****************************************************************************
 * A table that holds a mapping between the numerical FRESULT code and
 * it's name as a string.  This is used for looking up error codes for
 * printing to the console.
 ******************************************************************************/
tFresultString          g_sFresultStrings[] =
{
    FRESULT_ENTRY(FR_OK),
    FRESULT_ENTRY(FR_DISK_ERR),
    FRESULT_ENTRY(FR_INT_ERR),
    FRESULT_ENTRY(FR_NOT_READY),
    FRESULT_ENTRY(FR_NO_FILE),
    FRESULT_ENTRY(FR_NO_PATH),
    FRESULT_ENTRY(FR_INVALID_NAME),
    FRESULT_ENTRY(FR_DENIED),
    FRESULT_ENTRY(FR_EXIST),
    FRESULT_ENTRY(FR_INVALID_OBJECT),
    FRESULT_ENTRY(FR_WRITE_PROTECTED),
    FRESULT_ENTRY(FR_INVALID_DRIVE),
    FRESULT_ENTRY(FR_NOT_ENABLED),
    FRESULT_ENTRY(FR_NO_FILESYSTEM),
    FRESULT_ENTRY(FR_MKFS_ABORTED),
    FRESULT_ENTRY(FR_TIMEOUT),
    FRESULT_ENTRY(FR_LOCKED),
    FRESULT_ENTRY(FR_NOT_ENOUGH_CORE),
    FRESULT_ENTRY(FR_TOO_MANY_OPEN_FILES),
    FRESULT_ENTRY(FR_INVALID_PARAMETER)
};

/*****************************************************************************
 * A macro that holds the number of result codes.
 ******************************************************************************/
#define NUM_FRESULT_CODES (sizeof (g_sFresultStrings) / sizeof (tFresultString))

/*****************************************************************************
 * The buffer that holds the command line.
 ******************************************************************************/

volatile uint32_t   g_sPState = 0;
volatile uint32_t   g_sCState = 0;

/******************************************************************************
**                          FUNCTION DEFINITIONS
*******************************************************************************/

/*
 * Check command status
 */

static uint32_t HSMMCSDCmdStatusGet(mmcsdCtrlInfo *ctrl)
{
    uint32_t status = 0;

    while ((cmdCompFlag == 0) && (cmdTimeout == 0))
    {
        status = HSMMCSDStatusGet();
    }

    if (cmdCompFlag)
    {
        HSMMCSDIntrStatusClear(ctrlInfo.memBase,
                               HS_MMCSD_STAT_CMDCOMP);
        status      = 1;
        cmdCompFlag = 0;
    }

    if (cmdTimeout)
    {
        HSMMCSDIntrStatusClear(ctrlInfo.memBase,
                               HS_MMCSD_STAT_CMDTIMEOUT);
        status     = 0;
        cmdTimeout = 0;
    }

    return status;
}

static uint32_t HSMMCSDXferStatusGet(mmcsdCtrlInfo *ctrl)
{
    volatile uint32_t   status = 0;
    volatile uint32_t   temp;
    volatile uint32_t   i       = 0, i_max = 0;
    volatile unsigned char *dst_bfr = hsmmcsd_buffer;

    while (1)
    {
        status = HSMMCSDStatusGet();

        if (status & HS_MMCSD_STAT_BUFRDRDY)
        {
            HSMMCSDIntrStatusClear(ctrlInfo.memBase,
                                   HS_MMCSD_STAT_BUFRDRDY);

            if (dst_bfr != NULL)
            {
                if (hsmmcsd_dataLen < ctrl->blocksize)
                {
                    i_max = hsmmcsd_dataLen;
                }
                else
                {
                    i_max = ctrl->blocksize;
                }
                /*Input data bfr will not be 4-byte aligned*/
                for (i = 0; i < i_max; i += 4)
                {
                    temp           = HW_RD_REG32(ctrlInfo.memBase + MMC_DATA);
                    dst_bfr[i]     = *((char *) &temp);
                    dst_bfr[i + 1] = *((char *) &temp + 1);
                    dst_bfr[i + 2] = *((char *) &temp + 2);
                    dst_bfr[i + 3] = *((char *) &temp + 3);
                }
                dst_bfr += i_max;
            }
        }

        if (status & HS_MMCSD_STAT_BUFWRRDY)
        {
            HSMMCSDIntrStatusClear(ctrlInfo.memBase,
                                   HS_MMCSD_STAT_BUFWRRDY);

            if (hsmmcsd_buffer != NULL)
            {
                for (i = 0; i < hsmmcsd_dataLen; i += 4)
                {
                    *((char *) &temp)     = hsmmcsd_buffer[i];
                    *((char *) &temp + 1) = hsmmcsd_buffer[i + 1];
                    *((char *) &temp + 2) = hsmmcsd_buffer[i + 2];
                    *((char *) &temp + 3) = hsmmcsd_buffer[i + 3];
                    HW_WR_REG32(ctrlInfo.memBase + MMC_DATA, temp);
                }
            }
        }

        if (status & HS_MMCSD_STAT_DATATIMEOUT)
        {
            HSMMCSDIntrStatusClear(ctrlInfo.memBase,
                                   HS_MMCSD_STAT_DATATIMEOUT);
            status   = 0;
            xferPend = 0;
            break;
        }

        if (status & HS_MMCSD_STAT_TRNFCOMP)
        {
            HSMMCSDIntrStatusClear(ctrlInfo.memBase,
                                   HS_MMCSD_STAT_TRNFCOMP);
            status   = 1;
            xferPend = 0;
            break;
        }
    }

    return status;
}

static void HSMMCSDXferSetup(mmcsdCtrlInfo *ctrl, unsigned char rwFlag,
                             void *ptr,
                             uint32_t blkSize,
                             uint32_t nBlks)
{
    HSMMCSDIntrStatusClear(ctrl->memBase, HS_MMCSD_INTR_TRNFCOMP);

    if (rwFlag == 1)
    {
        HSMMCSDIntrStatusClear(ctrl->memBase, HS_MMCSD_INTR_BUFRDRDY);
        HSMMCSDIntrStatusEnable(ctrl->memBase, HS_MMCSD_INTR_BUFRDRDY);
        HSMMCSDIntrStatusDisable(ctrl->memBase, HS_MMCSD_INTR_BUFWRRDY);
    }
    else
    {
        HSMMCSDIntrStatusClear(ctrl->memBase, HS_MMCSD_INTR_BUFWRRDY);
        HSMMCSDIntrStatusEnable(ctrl->memBase, HS_MMCSD_INTR_BUFWRRDY);
        HSMMCSDIntrStatusDisable(ctrl->memBase, HS_MMCSD_INTR_BUFRDRDY);
    }

    HSMMCSDBlkLenSet(ctrl->memBase, blkSize);
    hsmmcsd_dataLen = (nBlks * blkSize);
    hsmmcsd_buffer  = (volatile unsigned char *) ptr;
    xferPend        = 1;
}

static uint32_t HSMMCSDStatusGet(void)
{
    volatile uint32_t status = 0;

    status = HSMMCSDIntrStatusGet(ctrlInfo.memBase, 0xFFFFFFFF);

    if (status & HS_MMCSD_STAT_CMDCOMP)
    {
        cmdCompFlag = 1;
    }

    if (status & HS_MMCSD_STAT_ERR)
    {
        errFlag = status & 0xFFFF0000;

        if (status & HS_MMCSD_STAT_CMDTIMEOUT)
        {
            cmdTimeout = 1;
        }
    }

    return status;
}

/*
** Initialize the MMCSD controller structure for use
*/
static int32_t HSMMCSDControllerSetup(MMC_INST_t num)
{
    switch (num)
    {
#ifndef TDA3XX_FAMILY_BUILD
        case MMC1_INST:
            ctrlInfo.memBase    = SOC_MMC1_BASE;
            ctrlInfo.edmaTxDreq = EDMA_DREQ_MMC1_TX;
            ctrlInfo.edmaRxDreq = EDMA_DREQ_MMC1_RX;
            ctrlInfo.edmaChan   = EDMA_MMCSD_CH;
            break;
#endif
#ifdef TDA2XX_FAMILY_BUILD
        case MMC2_INST:
            ctrlInfo.memBase    = SOC_MMC2_BASE;
            ctrlInfo.edmaTxDreq = EDMA_DREQ_MMC2_TX;
            ctrlInfo.edmaRxDreq = EDMA_DREQ_MMC2_RX;
            ctrlInfo.edmaChan   = EDMA_MMCSD_CH;
            break;

        case MMC3_INST:
            ctrlInfo.memBase    = SOC_MMC3_BASE;
            ctrlInfo.edmaTxDreq = EDMA_DREQ_MMC3_TX;
            ctrlInfo.edmaRxDreq = EDMA_DREQ_MMC3_RX;
            ctrlInfo.edmaChan   = EDMA_MMCSD_CH;
            break;
#endif
#if (defined (TDA2XX_FAMILY_BUILD)) || (defined (TDA3XX_FAMILY_BUILD))
        case MMC4_INST:
            ctrlInfo.memBase    = SOC_MMC4_BASE;
            ctrlInfo.edmaTxDreq = EDMA_DREQ_MMC4_TX;
            ctrlInfo.edmaRxDreq = EDMA_DREQ_MMC4_RX;
            ctrlInfo.edmaChan   = EDMA_MMCSD_CH;
#if defined (TDA3XX_FAMILY_BUILD)
            sdCard.numConv             = 1U;
            addrInfo.virtualAddr       = 0x00300000U;
            addrInfo.phyAddr           = 0x40300000U;
            addrInfo.addrSize          = 0x00080000U;
            addrInfo.phyAddrOffset     = 0x40000000;
            addrInfo.virtualAddrOffset = -0x40000000;
#endif
            break;
#endif
        default:
            return -1;
    }

    ctrlInfo.ctrlInit       = HSMMCSDControllerInit;
    ctrlInfo.xferSetup      = (_mmcsdxferSetup) HSMMCSDXferSetup;
    ctrlInfo.cmdStatusGet   = (_mmcsdcmdStatusGet) HSMMCSDCmdStatusGet;
    ctrlInfo.xferStatusGet  = (_mmcsdxferStatusGet) HSMMCSDXferStatusGet;
    ctrlInfo.cardPresent    = HSMMCSDCardPresent;
    ctrlInfo.cmdSend        = HSMMCSDCmdSend;
    ctrlInfo.busWidthConfig = HSMMCSDBusWidthConfig;
    ctrlInfo.busFreqConfig  = HSMMCSDBusFreqConfig;
    ctrlInfo.intrMask       =
        (HS_MMCSD_INTR_CMDCOMP | HS_MMCSD_INTR_CMDTIMEOUT |
         HS_MMCSD_INTR_DATATIMEOUT |
         HS_MMCSD_INTR_TRNFCOMP);
    ctrlInfo.intrEnable = HSMMCSDIntEnable;
    ctrlInfo.busWidth   = (SD_BUS_WIDTH_1BIT | SD_BUS_WIDTH_4BIT);
    ctrlInfo.highspeed  = 1;
    ctrlInfo.ocr        = MMCSD_OCR;
    ctrlInfo.card       = &sdCard;
    ctrlInfo.ipClk      = MMCSD_IN_FREQ;
    ctrlInfo.opClk      = MMCSD_INIT_FREQ;
#if defined (TDA3XX_FAMILY_BUILD)
    sdCard.addrConv = &addrInfo;
#endif
    sdCard.ctrl = &ctrlInfo;

    cmdCompFlag = 0;
    cmdTimeout  = 0;

    return 0;
}

void HSMMCSDFsMount(uint32_t driveNum, void *ptr)
{
    TCHAR* path = (TCHAR*)&driveNum;

    f_mount(&g_sFatFs, path, 0);
    fat_devices[0].dev = ptr;
    fat_devices[0].fs  = &g_sFatFs;
}

int32_t HSMMCSDInit(MMC_INST_t num)
{
    int32_t ret_val = 0;

    memset(&sdCard, 0, sizeof (mmcsdCardInfo));
    memset(&ctrlInfo, 0, sizeof (mmcsdCtrlInfo));
    memset(&addrInfo, 0, sizeof (mmcsdAddrConv));

    /* Basic controller initializations */
    ret_val = HSMMCSDControllerSetup(num);

    if (ret_val)
    {
        return ret_val;
    }

    /* Initialize the MMCSD controller */
    MMCSDCtrlInit(&ctrlInfo);

    MMCSDIntEnable(&ctrlInfo);

    HSMMCSDFsMount(0, &sdCard);

    return 0;
}

