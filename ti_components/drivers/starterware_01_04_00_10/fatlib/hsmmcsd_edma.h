/* ======================================================================
 *   Copyright (C) 2013 Texas Instruments Incorporated
 *
 *   All rights reserved. Property of Texas Instruments Incorporated.
 *   Restricted rights to use, duplicate or disclose this code are
 *   granted through contract.
 *
 *   The program may not be used without the written permission
 *   of Texas Instruments Incorporated or against the terms and conditions
 *   stipulated in the agreement under which this program has been
 *   supplied.
 * ==================================================================== */
/**
 *   Component:     fatlib_edma
 *
 *   Filename:      hsmmcsd_edma.h
 *
 *   Description:  This file contain SD API to configure the edma channel
 *
 */

#ifndef HSMMCSD_EDMA_H_
#define HSMMCSD_EDMA_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/**
 *  \brief Enumerates the emda transfer mode write & read.
 */
enum hsmmcsdRWFlag
{
    FLAG_WRITE,
    FLAG_READ
};

/**
 *  \brief hsmmcsdEdmaParam_t defines the edma transfer parameters.
 *
 * */
typedef struct
{
    /*base address of edma*/
    uint32_t baseAddr;

    /*sd data bfr address */
    uint32_t srcBfr;

    /*application destination bfr */
    uint32_t dstBfr;

    /*size of transfer bfr*/
    uint32_t length;

    /*read or write transfer mode*/
    uint32_t rwflag;

    /*DMA & QDMA channel type*/
    uint32_t channelType;

    /*event queue to be mapped with channel*/
    uint32_t eventQueue;

    /*edma logical channel number*/
    uint32_t chan;

    /*event src number like MMCSD TX DREQ & MMCSD RC DREQ*/
    uint32_t dreq;

    /*size of single block or frame*/
    uint32_t blockSize;
} hsmmcsdEdmaParam_t;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief  This function performs a memcpy using EDMA.
 *             Map the sd event source to a logical emda channel using XBAR.
 *             Derive the PaRAM set values & configure the PaRAM registers.
 *             Configure the channel to event manual mode & trigger the channel.
 *
 * \param   pEdmaParam   Parameter for edma channel & PaRAM set configuration
 *
 * \returns error status.If error has occured it returns a non zero value.
 *          If no error has occured then return status will be zero..
 *
 **/
uint32_t HSMMCSDEdmaCopy(const hsmmcsdEdmaParam_t *pEdmaParam);

/**
 * \brief  This function configure & enable the MMCSD edma channel.
 *             Map the sd event source to a logical emda channel using XBAR.
 *             Derive the PaRAM set values & configure the PaRAM registers.
 *             Configure the channel to event trigger mode & enable the channel.
 *
 * \param   pEdmaParam   Parameter for edma channel & PaRAM set configuration
 *
 * \returns error status.If error has occured it returns a non zero value.
 *          If no error has occured then return status will be zero..
 *
 **/
uint32_t HSMMCSDEdmaTransfer(const hsmmcsdEdmaParam_t *pEdmaParam);

/**
 * \brief  This function free & disable the MMCSD edma channel.
 *         Disconnect XBAR mapping between the sd event source & logical
 *         emda channel.
 *
 * \param   pEdmaParam   Parameter for edma channel & PaRAM set configuration
 *
 * \returns error status.If error has occured it returns a non zero value.
 *          If no error has occured then return status will be zero..
 *
 **/
uint32_t HSMMCSDEdmaClose(const hsmmcsdEdmaParam_t *pEdmaParam);

/**
 * \brief  This function waits for the particular EDMA channel transfer to
 *         complete
 *
 * \param   edmaChan Edma channel
 */
void HSMMCSDEdmaWaitTransfer(uint32_t edmaChan);

/**
 * \brief  Debug trace string print function
 *
 * \param  string String to print
 */
void HSMMCSDUartPuts(const char *string);

/**
 * \brief  Debug trace integer print function
 *
 * \param  value Integer to print
 */
void HSMMCSDUARTPutNum(int32_t value);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef HSMMCSD_EDMA_H_*/
