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
 *  \file   dma_xbar.c
 *
 *  \brief  DMA XBAR APIs.
 *
 *   This file contains the device abstraction layer APIs for DMA XBAR.
 */

/****************************************************************
 *  INCLUDE FILES
 ****************************************************************/
#include "stdint.h"
#include "hw_types.h"
#include "dma_xbar.h"
#include "chip_config.h"
#include "hw_ctrl_core_irq_dma.h"

/**
 * \brief   This function validates the DMA XBar arguemnts: DMA argument, DMA
 *          Xbar Instance Number, Event Source Number.\n
 *
 * \param   dma                  DMA Id.\n
 *
 * \param   xbarInst             DMA XBar Instance Number. \n
 *
 * \param   eventSource          Event Source Id.  \n
 *
 * \return  dma_xbar_ret_type_t  Possible Return values from DMA XBar. \n
 *
 */
dma_xbar_ret_type_t DMAXBARValidateArgs(dma_id_t dma, uint32_t xbarInst,
                                        xbar_evt_t eventSource)
{
    dma_xbar_ret_type_t retVal = dma_xbar_success;
    /* Validate DMA argument.*/
    if (dma >= 3)
    {
        retVal = invalid_dma_id;
    }

    /* Validate the dma xbar instance number */

    if ((dma_xbar_success == retVal) && (dma == EDMA) &&
        (xbarInst > (uint32_t) EDMA_EVT_XBAR_COUNT))
    {
        retVal = invalid_dma_xbar_inst_id;
    }

    if ((dma_xbar_success == retVal) && (dma == EDMA_DSP1) &&
        (xbarInst > (uint32_t) DSP1_EDMA_EVT_XBAR_COUNT))
    {
        retVal = invalid_dma_xbar_inst_id;
    }
    if ((dma_xbar_success == retVal) && (dma == EDMA_DSP2) &&
        (xbarInst > (uint32_t) DSP2_EDMA_EVT_XBAR_COUNT))
    {
        retVal = invalid_dma_xbar_inst_id;
    }

    /* Validate that the event source is legal. */

    if ((dma_xbar_success == retVal) && (eventSource >= DMA_XBAR_MAX_EVT_COUNT))
    {
        retVal = invalid_event_source_id;
    }

    return retVal;
}

/**
 * \brief   This function computes the register address for a valid DMA XBar
 *          instance.\n
 *
 * \param   dma          DMA Id.\n
 *
 * \param   xbarInst     DMA XBar Instance Number. \n
 *
 * \return  register address. \n
 *
 */
uint32_t DMAXBARGetOffset(dma_id_t dma, uint32_t xbarInst)
{
    uint32_t offset;
    uint32_t regOffset;

    if (dma == EDMA)
    {
        offset = CTRL_CORE_DMA_EDMA_DREQ_0_1;
    }

    if (dma == EDMA_DSP1)
    {
        offset = CTRL_CORE_DMA_DSP1_DREQ_0_1;
    }
    if (dma == EDMA_DSP2)
    {
        offset = CTRL_CORE_DMA_DSP2_DREQ_0_1;
    }
    /* Now we add an offset for the xbarInst.
     * This assumes that we are packing two xbar config bitfields per 32-bit
     * reg.
     * i.e. if xbar_inst is n, that means that the bitfield for it is at the
     * register n/2 32-bit offset.*/

    regOffset  = (xbarInst - 1U) >> 1;
    regOffset *= 4U; /* 32-bit MMRs */

    return (offset + regOffset);
}

/**
 * \brief   API to set up a connection between an event source and a DMA engine
 *          via the DMA XBAR.\n
 *
 * \param   baseAddr             Memory address of DMA XBAR.\n
 *
 * \param   dma                  DMA ID whose xbar instance is to be setup .\n
 *
 * \param   xbarInst             DMA XBAR instance number to be programmed .\n
 *
 * \param   eventSource          Event source to be selected via the DMA XBAR
 *                               instance .\n
 *
 * \return  dma_xbar_ret_type_t  Possible Return values from DMA XBar. \n
 *
 */
dma_xbar_ret_type_t DMAXBARConnect(uint32_t baseAddr, dma_id_t dma,
                                   uint32_t xbarInst,
                                   xbar_evt_t eventSource)
{
    dma_xbar_ret_type_t ret = dma_xbar_success;
    uint32_t offset;
    uint32_t pos;
    uint32_t val;
    uint32_t tempValue;
    ret = DMAXBARValidateArgs(dma, xbarInst, eventSource);
    if (dma_xbar_success == ret)
    {
        offset = DMAXBARGetOffset(dma, xbarInst);

        /* Write to either the high or the low bitfield depending on whether
         * xbarInst is odd or even.
         * This assumes that the MMR contains two bitfields - one per one XBAR
         * instance. */

        val       = HW_RD_REG32(baseAddr + offset);
        tempValue = (uint32_t) (xbarInst & 0x1U);
        if ((uint32_t) 0 != tempValue)
        {
            /* Odd - means, lower bitfield - starting at bit-offset 0 */
            pos = 0U;
            val = val & 0xffff0000U; /* zero out the lower bitfield */
        }
        else
        {
            /* Even - means, higher bitfield - starting at bit-offset 16 */
            pos = 16U;
            val = val & (uint32_t) 0xffff; /* zero out the higher bitfield */
        }

        val = val | (eventSource << pos);
        HW_WR_REG32(baseAddr + offset, val);
    }
    return ret;
}

/**
 * \brief   API to disconnect any connection between an event source and a dma
 *          engine in the DMA XBAR.\n
 *
 * \param   baseAddr             Memory address of DMA XBAR.\n
 *
 * \param   dma                  DMA ID whose xbar instance is to be
 *                               disconnected.\n
 *
 * \param   xbarInst             DMA XBAR instance number to be disconnected.\n
 *
 * \return  dma_xbar_ret_type_t  Possible Return values from DMA XBar. \n
 *
 */
dma_xbar_ret_type_t DMAXBARDisconnect(uint32_t baseAddr, dma_id_t dma,
                                      uint32_t xbarInst)
{
    dma_xbar_ret_type_t ret = dma_xbar_success;
    uint32_t offset;
    uint32_t pos;
    uint32_t val;
    uint32_t tempValue;

    ret = DMAXBARValidateArgs(dma, xbarInst, DMA_XBAR_RSVD);
    if (dma_xbar_success == ret)
    {
        offset = DMAXBARGetOffset(dma, xbarInst);

        /* Write to either the high or the low bitfield depending on whether
         * xbar_inst is odd or even.
         * This assumes that the MMR contains two bitfields - one per one XBAR
         * instance.*/

        val       = HW_RD_REG32(baseAddr + offset);
        tempValue = (uint32_t) (xbarInst & 0x1U);
        if ((uint32_t) 0 != tempValue)
        {
            /* Odd - means, lower bitfield - starting at bit-offset 0 */
            pos = 0U;
            val = val & 0xffff0000U; /* zero out the lower bitfield */
        }
        else
        {
            /* Even - means, higher bitfield - starting at bit-offset 16 */
            pos = 16U;
            val = val & (uint32_t) 0xffff; /* zero out the higher bitfield */
        }

        val = val | (DMA_XBAR_RSVD << pos);
        HW_WR_REG32(baseAddr + offset, val);
    }
    return ret;
}

/**
 * \brief   API to read the current connection between a dma engine and an event
 *          source via a dma xbar.\n
 *
 * \param   baseAddr             Memory address of DMA XBAR.\n
 *
 * \param   dma                  DMA ID whose xbar instance configuration is to
 *                               be read.\n
 *
 * \param   xbarInst             DMA XBAR instance number whose configuration is
 *                               to be read.\n
 *
 * \param   eventSource          Event Source that is currently selected via
 *                               this dma xbar.\n
 *
 * \return  dma_xbar_ret_type_t  Possible Return values from DMA XBar. \n
 *
 */
dma_xbar_ret_type_t DMAXBARGetConnection(uint32_t baseAddr, dma_id_t dma,
                                         uint32_t xbarInst,
                                         xbar_evt_t *eventSource)
{
    dma_xbar_ret_type_t ret = dma_xbar_success;
    uint32_t offset;
    uint32_t pos;
    uint32_t val;
    uint32_t tempValue;

    ret = DMAXBARValidateArgs(dma, xbarInst, DMA_XBAR_RSVD);
    if (dma_xbar_success == ret)
    {
        offset = DMAXBARGetOffset(dma, xbarInst);

        /* Write to either the high or the low bitfield depending on whether
         * xbar_inst is odd or even.
         * This assumes that the MMR contains two bitfields - one per one XBAR
         * instance.*/

        val       = HW_RD_REG32(baseAddr + offset);
        tempValue = (uint32_t) (xbarInst & 0x1U);
        if ((uint32_t) 0 != tempValue)
        {
            /* Odd - means, lower bitfield - starting at bit-offset 0 */
            pos = 0U;
        }
        else
        {
            /* Even - means, higher bitfield - starting at bit-offset 16 */
            pos = 16U;
        }

        *eventSource = (xbar_evt_t) ((val >> pos) & (uint32_t) 0xffff);
    }
    return ret;
}

/**
 * \brief   API to check if a DMA XBAR instance is indeed connected to the event
 *          source.\n
 *
 * \param   baseAddr             Memory address of DMA XBAR.\n
 *
 * \param   dma                  DMA ID whose interrupt xbar instance
 *                               configuration is to be verified.\n
 *
 * \param   xbarInst             DMA XBAR instance number whose configuration is
 *                               to be verified.\n
 *
 * \param   eventSource          Event source whose selection is to be
 *                               verified.\n
 *
 * \param   status               Returns 1 if the dma xbar instance has indeed
 *                               selected this source, 0 otherwise.\n
 *
 * \return  dma_xbar_ret_type_t  Possible Return values from DMA XBar. \n
 *
 */
dma_xbar_ret_type_t DMAXBARIsConnected(uint32_t baseAddr, dma_id_t dma,
                                       uint32_t xbarInst,
                                       xbar_evt_t eventSource,
                                       uint8_t *status)
{
    xbar_evt_t          currEventSource;
    dma_xbar_ret_type_t ret = dma_xbar_success;
    ret = DMAXBARGetConnection(baseAddr, dma, xbarInst, &currEventSource);
    if (dma_xbar_success == ret)
    {
        if (currEventSource == eventSource)
        {
            *status = 1U;
        }
        else
        {
            *status = 0U;
        }
    }
    return ret;
}

