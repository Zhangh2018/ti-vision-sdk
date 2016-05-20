/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \file algorithmLink_algPluginSupport.c
 *
 * \brief  This file has the implementation of Support functions for alg plug in
 *
 *         Functions here, will be called by the plug in functions OR skeletal
 *         implmentation. For plug in function writers, only reference of
 *         algorithmLink_algPluginSupport.h is enough.
 *
 * \version 0.0 (Aug 2013) : [PS] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <include/link_api/algorithmLink_algPluginSupport.h>
#include <src/links_common/algorithm/algorithmLink_cfg.h>
#include <src/links_common/algorithm/algorithmLink_priv.h>

/**
 *******************************************************************************
 *
 *   \brief Algorithm link register plugin
 *
 *          Each algorithm will have the corresponding plugin functions for
 *          create, process, control, stop, delete.
 *          These plugin functions will be registered in the algorithm link
 *          function table via this API.
 *
 *   \param algId               [IN] Algorithm Id
 *   \param pPluginFuncs        [IN] Pointer to struct of plugin functions
 *
 *   \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/

Int32 AlgorithmLink_registerPlugin(Int32                    algId,
                                   const AlgorithmLink_FuncTable *pPluginFuncs)
{

    UTILS_assert(algId < ALGORITHM_LINK_ALG_MAXNUM);

    gAlgorithmLinkFuncTable[algId].AlgorithmLink_AlgPluginCreate =
        pPluginFuncs->AlgorithmLink_AlgPluginCreate;

    gAlgorithmLinkFuncTable[algId].AlgorithmLink_AlgPluginProcess =
        pPluginFuncs->AlgorithmLink_AlgPluginProcess;

    gAlgorithmLinkFuncTable[algId].AlgorithmLink_AlgPluginControl =
        pPluginFuncs->AlgorithmLink_AlgPluginControl;

    gAlgorithmLinkFuncTable[algId].AlgorithmLink_AlgPluginStop =
        pPluginFuncs->AlgorithmLink_AlgPluginStop;

    gAlgorithmLinkFuncTable[algId].AlgorithmLink_AlgPluginDelete =
        pPluginFuncs->AlgorithmLink_AlgPluginDelete;

    return  SYSTEM_LINK_STATUS_SOK;
}

Int32 AlgorithmLink_registerPutEmptyBuffersPlugin(
                     void  *ptr,
                     const  AlgorithmLink_AlgPluginPutEmptyBuffers pPluginFunc)
{
    AlgorithmLink_Obj * pObj;
    pObj = (AlgorithmLink_Obj*) ptr;

    pObj->callbackPutEmptyBuffers = pPluginFunc;

    return SYSTEM_LINK_STATUS_SOK;
}


/**
 *******************************************************************************
 *
 *   \brief Algorithm link queue initializations, which need to be done at the
 *          algorithm creation time
 *
 *          Several book keeping operations are needed for buffer handling.
 *          Initializations needed for them are done in this function.
 *
 *   \param ptr                [IN] Alg link object
 *   \param numInputQUsed      [IN] Number of input queues used
 *   \param pInputQInfo        [IN] Pointer to base element of inputQInfo array
 *   \param numOutputQUsed     [IN] Number of output queues used
 *   \param pOutputQInfo       [IN] Pointer to base element of outputQInfo array
 *
 *   \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 AlgorithmLink_queueInfoInit(
                void                                *ptr,
                UInt32                               numInputQUsed,
                const AlgorithmLink_InputQueueInfo  *pInputQInfo,
                UInt32                               numOutputQUsed,
                const AlgorithmLink_OutputQueueInfo *pOutputQInfo)
{

    Int32  status = SYSTEM_LINK_STATUS_SOK;
    UInt32 queId;
    UInt32 chanId;
    UInt32 tempCntr = 0U;


    AlgorithmLink_Obj * pObj;
    pObj = (AlgorithmLink_Obj*) ptr;

    if((numInputQUsed > 0U) && (pInputQInfo == NULL))
    {
        status = SYSTEM_LINK_STATUS_EFAIL;
    }

    if((numOutputQUsed > 0U) && (pOutputQInfo == NULL))
    {
        status = SYSTEM_LINK_STATUS_EFAIL;
    }

    if(numInputQUsed > ALGORITHM_LINK_MAX_NUMQUEUES)
    {
        status = SYSTEM_LINK_STATUS_EFAIL;
    }

    if(numOutputQUsed > ALGORITHM_LINK_MAX_NUMQUEUES)
    {
        status = SYSTEM_LINK_STATUS_EFAIL;
    }

    pObj->numInputQUsed  = numInputQUsed;
    pObj->numOutputQUsed = numOutputQUsed;

    pObj->linkInfo.numQue = numOutputQUsed;

    for(queId = 0U;
        (queId < numOutputQUsed) && (status == SYSTEM_LINK_STATUS_SOK);
        queId++)
    {
        memcpy(&(pObj->outputQInfo[queId]),
               pOutputQInfo,
               sizeof(AlgorithmLink_OutputQueueInfo));

        /*
         * Copying output Q info into Link info. This will get used by the next
         * link.
         */
        memcpy(&(pObj->linkInfo.queInfo[queId]),
               &(pOutputQInfo->queInfo),
               sizeof(System_LinkQueInfo));

        /*
         * Creation of empty output Q. Seperate Q for each channel.
         * Empty Q holds all empty buffers (which do not have data in them)
         * So after creation all buffers are pushed into empty Q. Once data
         * is present in them, buffers will be moved into full Q. In case of
         * INPLACE computations, empty buffer Q will not get used.
         */
        for(chanId = 0U; chanId < (pOutputQInfo->queInfo.numCh); chanId++)
        {
            status = Utils_queCreate
                                (&(pObj->emptyOutputQ[queId][chanId].queHandle),
                                ALGORITHM_LINK_MAX_QUEUELENGTH,
                                &(pObj->emptyOutputQ[queId][chanId].queMem),
                                UTILS_QUE_FLAG_NO_BLOCK_QUE);

            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
        }

        /*
         * Creation of full output Q. Single Q holds buffers for all channels.
         * Full Q holds all full buffers (which have processed data in them)
         * Once processing is done, if a buffer needs to be passed to next link
         * then it will be queued in this que (For both cases of INPLACE and
         * NON-INPLACE computations)
         */
        status = Utils_queCreate
                            (&(pObj->fullOutputQ[queId].queHandle),
                            ALGORITHM_LINK_MAX_QUEUELENGTH,
                            &(pObj->fullOutputQ[queId].queMem),
                            UTILS_QUE_FLAG_NO_BLOCK_QUE);

        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

        /* MISRA.PTR.ARITH
         * MISRAC_2004 Rule_17.1 and MISRAC_2004 Rule_17.4
         * MISRAC_WAIVER:
         * Pointer is initialized with the address of array variable and
         * then accessed as an array
         * Null check is included. Access will not exceed numOutputQUsed
         * entries.
         */
        pOutputQInfo++;

        for(tempCntr = 0U;
            tempCntr < ALGORITHM_LINK_MAX_QUEUELENGTH;
            tempCntr++)
        {
            pObj->outputQFreeStatus[queId][tempCntr].pBuff = NULL;
        }

    }

    for(queId = 0U;
        (queId < numInputQUsed) && (status == SYSTEM_LINK_STATUS_SOK);
        queId++)
    {
        memcpy(&(pObj->inputQInfo[queId]),
               pInputQInfo,
               sizeof(AlgorithmLink_InputQueueInfo));

        /* MISRA.PTR.ARITH
         * MISRAC_2004 Rule_17.1 and MISRAC_2004 Rule_17.4
         * MISRAC_WAIVER:
         * Pointer is initialized with the address of array variable and
         * then accessed as an array
         * Null check is included. Access will not exceed numInputQUsed
         * entries.
         */
        pInputQInfo++;
    }

    return status;
}

/**
 *******************************************************************************
 *
 *   \brief Algorithm link returns pointer to algorithm specific parameters obj
 *
 *   \param ptr                [IN] Alg link object
 *
 *   \return  (void *) pointer to algorithm specific parameters obj
 *
 *******************************************************************************
*/
void * AlgorithmLink_getAlgorithmParamsObj(void * ptr)
{
    AlgorithmLink_Obj *pObj;
    pObj = (AlgorithmLink_Obj *)ptr;

    return (pObj->algorithmParams);
}

/**
 *******************************************************************************
 *
 *   \brief Algorithm link records the pointer of algo specific parameters obj
 *
 *   \param pObj                [IN] Alg link object
 *   \param pAlgObj             [IN] Pointer to algo specific parameters obj
 *
 *   \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 AlgorithmLink_setAlgorithmParamsObj(void * ptr, void * pAlgObj)
{
    AlgorithmLink_Obj *pObj;
    pObj = (AlgorithmLink_Obj *)ptr;

    pObj->algorithmParams = pAlgObj;

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 *   \brief Algorithm link get Link Id
 *
 *          This plugin function is used to return the link id
 *          associated with the algorithm. This is used by the link stats.
 *
 *   \param pObj               [IN] Current link object
 *
 *   \return Id of the Link
 *
 *******************************************************************************
*/
UInt32 AlgorithmLink_getLinkId(void *ptr)
{
    AlgorithmLink_Obj *pObj;
    pObj = (AlgorithmLink_Obj *)ptr;

    return (pObj->linkId);
}

/**
 *******************************************************************************
 *
 *   \brief Algorithm link function to put o.p buffer into empty local output Q
 *
 *          For each output Q, there will be empty and Full output Q.
 *          Empty Q holds all output buffers which are empty (No data yet)
 *          Full Q holds all output buffers which are full (Data present)
 *          Plugin functions, at the time of buffer creation, need to call this
 *          API to put all the created buffers into the empty.
 *
 *   \param ptr                 [IN] Current link object
 *   \param outputQId           [IN] Output Que ID of current link
 *   \param pSystemBuffer       [IN] POinter to system buffer to be put into Q
 *
 *   \return SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 AlgorithmLink_putEmptyOutputBuffer(void          *ptr,
                                         Int32          outputQId,
                                         System_Buffer *pSystemBuffer)
{
    AlgorithmLink_Obj *pObj;
    Int32              status;
    UInt32             chId;

    pObj = (AlgorithmLink_Obj *)ptr;

    UTILS_assert(outputQId < pObj->numOutputQUsed);

    chId = pSystemBuffer->chNum;

    status = Utils_quePut(&(pObj->emptyOutputQ[outputQId][chId].queHandle),
                          pSystemBuffer,
                          BSP_OSAL_NO_WAIT);

    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    return status;
}

/**
 *******************************************************************************
 *
 *   \brief Algorithm link function to get o.p buffer from empty local output Q
 *
 *          For each output Q, there will be an empty and Full output Q.
 *          Empty Q holds all output buffers which are empty (No data yet)
 *          Full Q holds all output buffers which are full (Data present)
 *          Plugin functions, at the time of process function, needs to call
 *          API to get an empty output buffer to put the processed data.
 *
 *   \param ptr                 [IN] Current link object
 *   \param outputQId           [IN] Output Que ID of current link
 *   \param channelId           [IN] Channel ID of the Que
 *   \param pSystemBuffer       [IN] Pointer to pointer of system buffer, which
 *                                   will get updated with System buffer
 *                                   address.
 *
 *   \return SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 AlgorithmLink_getEmptyOutputBuffer(void           * ptr,
                                         UInt32           outputQId,
                                         UInt32           channelId,
                                         System_Buffer ** pSystemBuffer)
{
    AlgorithmLink_Obj *pObj;
    Int32              status;

    pObj = (AlgorithmLink_Obj *)ptr;

    UTILS_assert(outputQId < pObj->numOutputQUsed);
    UTILS_assert(channelId < pObj->outputQInfo[outputQId].queInfo.numCh);

    /* MISRA.CAST.PTR
     * MISRAC_2004 Rule_11.4
     * MISRAC_WAIVER:
     * This is a generic queue implementation which supports all dataTypes.
     * This typecasting is necessary.
     */
    status = Utils_queGet(&(pObj->emptyOutputQ[outputQId][channelId].queHandle),
                          (Ptr *)pSystemBuffer,
                          1U,
                          BSP_OSAL_NO_WAIT);

    return status;
}

/**
 *******************************************************************************
 *
 *   \brief Algorithm link function to put o.p buffer into Full output Q
 *
 *          For each output Q, there will be empty and Full output Q.
 *          Empty Q holds all output buffers which are empty (No data yet)
 *          Full Q holds all output buffers which are full (Data present)
 *          Plugin functions, at the time of process function, needs to call
 *          this API to put a Full output buffer into output Q
 *
 *   \param ptr                 [IN] Current link object
 *   \param outputQId           [IN] Output Que ID of current link
 *   \param pSystemBuffer       [IN] POinter to system buffer to be put into Q
 *
 *   \return SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 AlgorithmLink_putFullOutputBuffer(void          *ptr,
                                        Int32          outputQId,
                                        System_Buffer *pSystemBuffer)
{
    AlgorithmLink_Obj *pObj;
    Int32              status;

    pObj = (AlgorithmLink_Obj *)ptr;

    UTILS_assert(outputQId < pObj->numOutputQUsed);

    status = Utils_quePut(&(pObj->fullOutputQ[outputQId].queHandle),
                          pSystemBuffer,
                          BSP_OSAL_NO_WAIT);

    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    return status;
}

/**
 *******************************************************************************
 *
 *   \brief Algorithm link function to return empty input buffers to prev link
 *
 *          In algorithm link, returning empty input buffer to previous link
 *          depends on the mode of operation as follows -
 *          1) Non-Inplace: In this case, once the algorithm returns (frees up)
 *          input buffer, it can be returned to previous link.
 *          2) Inplace: In this case,  input buffer can be returned to previous
 *          link only after both the current link algorithm and next link
 *          algorithm free up the buffer.
 *          This function does the necessary book keeping to facilitate
 *          emtpy buffer return, in both the modes.
 *
 *          Note that algorithm plug in functions will have to call this
 *          function in cases of both Inplace and Non-Inplace computations
 *
 *   \param ptr                [IN] Current link object
 *   \param inputQId           [IN] Input Que ID of current link
 *   \param prevLinkId         [IN] Link ID of previous link
 *   \param prevLinkQueId      [IN] Que ID of previous link
 *   \param pBufList           [IN] List of buffers released to previous link
 *   \param pBufDropFlag       [IN] Pointer to an array of flags. To indicate
 *                                  if this i.p buffer was used or dropped
 *                                  with out processing. Array of flags
 *                                  correspond to array of buffers in pBufList
 *
 *   \return SYSTEM_LINK_STATUS_SOK on success, else error code
 *
 *******************************************************************************
*/
Int32 AlgorithmLink_releaseInputBuffer(void              *ptr,
                                       UInt32             inputQId,
                                       UInt32             prevLinkId,
                                       UInt16             prevLinkQueId,
                                       System_BufferList *pBufList,
                                       const Bool        *pBufDropFlag)
{

    Int32              status = SYSTEM_LINK_STATUS_SOK;
    Int32              bufferId;
    System_Buffer     *pSysBuffer;
    System_BufferList  bufListRelease;

    AlgorithmLink_ReleaseStatus relStatus;

    AlgorithmLink_Obj * pObj;
    pObj = (AlgorithmLink_Obj*) ptr;

    UTILS_assert(inputQId < pObj->numInputQUsed);

    if(pObj->inputQInfo[inputQId].qMode == ALGORITHM_LINK_QUEUEMODE_NOTINPLACE)
    {
        System_putLinksEmptyBuffers(prevLinkId, prevLinkQueId, pBufList);
    }
    else if(pObj->inputQInfo[inputQId].qMode==ALGORITHM_LINK_QUEUEMODE_INPLACE)
    {
        bufListRelease.numBuf = 0;
        for(bufferId = 0; bufferId < pBufList->numBuf; bufferId++)
        {
            pSysBuffer = pBufList->buffers[bufferId];

            if(pBufDropFlag == NULL)
            {
                relStatus  = AlgorithmLink_sysBufRelStatusUpdate(
                                pSysBuffer,
                                &(pObj->inputQFreeStatus[inputQId][0])
                                );
            }
            /* MISRA.PTR.ARITH
             * MISRAC_2004 Rule_17.1 and MISRAC_2004 Rule_17.4
             * MISRAC_WAIVER:
             * Pointer is initialized with the address of array variable and
             * then accessed as an array
             * Null check is included. Access will not exceed pBufList->numBuf
             * entries.
             */
            else if(pBufDropFlag[bufferId] == TRUE)
            {
                relStatus = ALGORITHM_LINK_RELSTATUS_READY;
            }
            else
            {
                relStatus  = AlgorithmLink_sysBufRelStatusUpdate(
                                pSysBuffer,
                                &(pObj->inputQFreeStatus[inputQId][0])
                                );
            }

            if(relStatus == ALGORITHM_LINK_RELSTATUS_READY)
            {
                bufListRelease.buffers[bufListRelease.numBuf] = pSysBuffer;
                bufListRelease.numBuf ++;
            }
        }

        if(bufListRelease.numBuf)
        {
            System_putLinksEmptyBuffers(prevLinkId,
                                        prevLinkQueId,
                                        &bufListRelease);
        }

    }
    else
    {
        status = SYSTEM_LINK_STATUS_EFAIL;
    }

    return status;
}

/**
 *******************************************************************************
 *
 *   \brief Algorithm link function to return (free) output buffers
 *
 *          In algorithm link, releasing output buffer,
 *          depends on the mode of operation as follows -
 *          1) Non-Inplace: In this case,  output buffer can be returned to
 *          empty buffer pool only after both the current link algorithm and
 *          next link algorithm free up the buffer.
 *          2) Inplace: In this case, once the next link returns (frees up)
 *          output buffer, it is passed onto input side of the current link.
 *          Only after current link alg releases it (As input buffer release),
 *          it can be returned to previous link.
 *          This function does the necessary book keeping to facilitate
 *          output buffer release, in both the modes.
 *
 *          Note that in case of Inplace computations, algorithm plugin
 *          functions should not call this function, since the buffer is
 *          expected to be released via releaseInputBuffer function.
 *
 *   \param ptr                [IN] Current link object
 *   \param outputQId          [IN] Output Q ID of current link
 *   \param pBufList           [IN] List of buffers released to previous link
 *
 *   \return SYSTEM_LINK_STATUS_SOK on success, else error code
 *
 *******************************************************************************
*/
Int32 AlgorithmLink_releaseOutputBuffer(void              *ptr,
                                        UInt32             outputQId,
                                        System_BufferList *pBufList)
{
    Int32              status = SYSTEM_LINK_STATUS_SOK;
    Int32              bufferId;
    System_Buffer     *pSysBuffer;
    UInt32             chId;

    AlgorithmLink_ReleaseStatus relStatus;

    AlgorithmLink_Obj * pObj;
    pObj = (AlgorithmLink_Obj*) ptr;

    UTILS_assert(outputQId < pObj->numOutputQUsed);

    if(pObj->outputQInfo[outputQId].qMode == ALGORITHM_LINK_QUEUEMODE_INPLACE)
    {
        AlgorithmLink_releaseInputBuffer(
            pObj,
            pObj->outputQInfo[outputQId].inputQId,
            pObj->outputQInfo[outputQId].inQueParams.prevLinkId,
            pObj->outputQInfo[outputQId].inQueParams.prevLinkQueId,
            pBufList,
            NULL);
    }
    else if(pObj->outputQInfo[outputQId].qMode ==
                ALGORITHM_LINK_QUEUEMODE_NOTINPLACE)
    {

        for(bufferId = 0; bufferId < pBufList->numBuf; bufferId++)
        {

            pSysBuffer = pBufList->buffers[bufferId];
            relStatus  = AlgorithmLink_sysBufRelStatusUpdate(
                            pSysBuffer,
                            &(pObj->outputQFreeStatus[outputQId][0])
                            );

            if(relStatus == ALGORITHM_LINK_RELSTATUS_READY)
            {
                chId = pSysBuffer->chNum;
                status = Utils_quePut(
                            &(pObj->emptyOutputQ[outputQId][chId].queHandle),
                            pSysBuffer,
                            BSP_OSAL_NO_WAIT);

                UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
            }

        }
    }
    else
    {
        status = SYSTEM_LINK_STATUS_EFAIL;
    }
    return status;
}

/**
 *******************************************************************************
 *
 *   \brief Algorithm link function to check release status
 *
 *          SystemBuffer (pSysBuffer) is searched in the array of
 *          AlgorithmLink_BufferFreeStatus (pBufferFreeStatusBase).
 *          If a match is NOT found, then this SystemBuffer is inserted in a
 *          freely available position. Also it means that this
 *          buffer has been released only once (this time). So it is not
 *          ready for final release.
 *          If a match is found, it means that this buffer has already been
 *          released once and this is the second time. Hence this buffer is
 *          ready for final release. Also this buffer will be evicted from
 *          Free Status array, by inserting NULL.
 *          In case of match NOT found and insertion, the logic is to
 *          insert in the earliest freely available position, so that the
 *          future matches occur early in looping and hence fewer cycles.
 *
 *          Since this function can be called be several threads and they
 *          can operate on common pBufferFreeStatusBase, interrupts are
 *          disabled at the beginning of function and just before function
 *          return, it is enabled.
 *
 *   \param pSysBuffer               [IN] System buffer
 *   \param pBufferFreeStatusBase    [IN] Base pointer of FreeStatus array
 *                                        to be searched.
 *
 *   \return AlgorithmLink_ReleaseStatus
 *
 *******************************************************************************
*/
AlgorithmLink_ReleaseStatus AlgorithmLink_sysBufRelStatusUpdate(
                        System_Buffer                   *pSysBuffer,
                        AlgorithmLink_BufferFreeStatus  *pBufferFreeStatusBase)
{
    UInt32 bufferId;
    UInt32 freeBufferId;
    AlgorithmLink_BufferFreeStatus  *pBufferFreeStatus;
    UInt32 cookie;
    AlgorithmLink_ReleaseStatus  status = ALGORITHM_LINK_RELSTATUS_ERROR;

    pBufferFreeStatus = pBufferFreeStatusBase;
    freeBufferId      = 0xFFFFFFFFU;

    cookie = Hwi_disable();

    for(bufferId = 0;
        (bufferId < ALGORITHM_LINK_MAX_QUEUELENGTH) &&
        (pBufferFreeStatus != NULL);
        bufferId++)
    {
        if(pBufferFreeStatus->pBuff == pSysBuffer)
        {
            pBufferFreeStatus->pBuff = NULL;

            Hwi_restore(cookie);
            status = ALGORITHM_LINK_RELSTATUS_READY;
            break;
        }
        else if((freeBufferId == 0xFFFFFFFFU) &&
                 (pBufferFreeStatus->pBuff == NULL))
        {
            freeBufferId = bufferId;
        }
        else
        {
            status = ALGORITHM_LINK_RELSTATUS_ERROR;
        }

        /* MISRA.PTR.ARITH
         * MISRAC_2004 Rule_17.1 and MISRAC_2004 Rule_17.4
         * MISRAC_WAIVER:
         * Pointer is initialized with the address of array variable and
         * then accessed as an array
         * freeBufferId is ensured to be within bounds.
         */
        pBufferFreeStatus++;
    }

    if(ALGORITHM_LINK_RELSTATUS_READY != status)
    {
        if(freeBufferId == 0xFFFFFFFFU)
        {
            Hwi_restore(cookie);
            status = ALGORITHM_LINK_RELSTATUS_ERROR;
        }
        else
        {
            /* MISRA.PTR.ARITH
             * MISRAC_2004 Rule_17.1 and MISRAC_2004 Rule_17.4
             * MISRAC_WAIVER:
             * Pointer is initialized with the address of array variable and
             * then accessed as an array
             * Increment is ensured to be within bounds defined by
             * ALGORITHM_LINK_MAX_QUEUELENGTH.
             */
            pBufferFreeStatusBase[freeBufferId].pBuff = pSysBuffer;

            Hwi_restore(cookie);
            status = ALGORITHM_LINK_RELSTATUS_NOTREADY;
        }
    }

    return status;
}

/* Nothing beyond this point */
