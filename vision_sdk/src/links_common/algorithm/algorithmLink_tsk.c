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
 * \file algorithmLink_tsk.c
 *
 * \brief  This file has the implementation of Algorithm Link
 *
 *         This file implements the state machine logic for this link.
 *         This file also implements the algorithm link tear down functionality
 *
 * \version 0.0 (Jul 2013) : [PS] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */

#include "algorithmLink_priv.h"
#include <include/link_api/algorithmLink_algPluginSupport.h>
#include "algorithmLink_cfg.h"
#if defined (BUILD_ARP32)
#include <src/utils_common/include/utils_idle.h>
#endif

/*******************************************************************************
 *  FUNCTION DECLARATIONS
 *******************************************************************************
 */
Int32 AlgorithmLink_pluginCreate(AlgorithmLink_Obj *pObj,
                                 const Utils_MsgHndl *pMsg);
Int32 AlgorithmLink_pluginStop(AlgorithmLink_Obj *pObj);

/**
 *******************************************************************************
 *
 * \brief Implementation of Create state for alg link
 *
 *        This link implements create state for algorithm link.
 *
 * \param  pObj     [IN] Algorithm link object handle
 * \param  pMsg     [IN] Message Handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_pluginCreate(AlgorithmLink_Obj *pObj,
                                 const Utils_MsgHndl *pMsg)
{
    AlgorithmLink_CreateParams *pCreateParams;
    UInt32 algId;
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    pCreateParams = (AlgorithmLink_CreateParams *)Utils_msgGetPrm(pMsg);
    pObj->algId   = pCreateParams->algId;
    pObj->callbackPutEmptyBuffers = NULL;
    algId         =  pObj->algId;

    if(algId>=ALGORITHM_LINK_ALG_MAXNUM)
    {
        Vps_printf(" ALG: ERROR: Invalid alg ID specified "
                   " (algId = %d) !!!\n", pObj->algId);

        status = SYSTEM_LINK_STATUS_EINVALID_PARAMS;
    }

    if ((gAlgorithmLinkFuncTable[algId].AlgorithmLink_AlgPluginCreate == NULL)
         && (SYSTEM_LINK_STATUS_SOK == status))
    {
        Vps_printf(" ALG: ERROR: No alg create function specified "
                   " (algId = %d) !!!\n", pObj->algId);
        status = SYSTEM_LINK_STATUS_EINVALID_PARAMS;
    }

    /*
     * Create command received, create the algorithm here
     */
    if((gAlgorithmLinkFuncTable[algId].AlgorithmLink_AlgPluginCreate != NULL)
         && (SYSTEM_LINK_STATUS_SOK == status))
    {
        #ifdef SYSTEM_DEBUG_ALGORITHM
        Vps_printf(" ALGORITHM: Create in progress"
                   " (algId = %d) !!!\n", pObj->algId);
        #endif

        UTILS_MEMLOG_USED_START();

        status =
            (*(gAlgorithmLinkFuncTable[algId].AlgorithmLink_AlgPluginCreate))
             (pObj, pCreateParams);

        UTILS_MEMLOG_USED_END(pObj->memUsed);
        UTILS_MEMLOG_PRINT("ALGORITHM:",
                       pObj->memUsed,
                       UTILS_ARRAYSIZE(pObj->memUsed));

        #ifdef SYSTEM_DEBUG_ALGORITHM
        Vps_printf(" ALGORITHM: Create Done"
                   " (algId = %d) !!!\n", pObj->algId);
        #endif
    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of staop state for alg link
 *
 * \param  pObj     [IN] Algorithm link object handle
 * \param  pMsg     [IN] Message Handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_pluginStop(AlgorithmLink_Obj *pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    #ifdef SYSTEM_DEBUG_ALGORITHM
    Vps_printf(" ALGORITHM: Stop in progress"
               " (algId = %d) !!!\n", pObj->algId);
    #endif

    if(gAlgorithmLinkFuncTable[pObj->algId].AlgorithmLink_AlgPluginStop
    != NULL)
    {
        status =
        (*(gAlgorithmLinkFuncTable[pObj->algId].AlgorithmLink_AlgPluginStop))
            ((void *) pObj);
    }

    #ifdef SYSTEM_DEBUG_ALGORITHM
    Vps_printf(" ALGORITHM: Stop Done"
               " (algId = %d) !!!\n", pObj->algId);
    #endif

    return status;
}

/**
 *******************************************************************************
 *
 * \brief This function implements the CREATE state of algorithm Link.
 *
 *        In this state link gets commands to
 *         - Create the Algorithm
 *         - Moves to RUN state
 *
 * \param  pTsk [IN] Task Handle
 * \param  pMsg [IN] Message Handle
 *
 * \return  void
 *
 *******************************************************************************
 */
Void AlgorithmLink_tskMain(struct Utils_TskHndl_t *pTsk, Utils_MsgHndl *pMsg)
{
    /* MISRA.PPARAM.NEEDS.CONST
     * MISRAC_2004 Rule_16.7
     * MISRAC_WAIVER:
     * Generic function prototype.
     */

    UInt32 cmd = Utils_msgGetCmd(pMsg);
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    UInt32          flushCmds[1];

    AlgorithmLink_Obj *pObj = (AlgorithmLink_Obj *) pTsk->appData;
    AlgorithmLink_ControlParams *pControlParams;

    /*
     * Different commands are serviced via this switch case. For each
     * command, after servicing, ACK or free message is sent before
     * proceeding to next state.
     */
    switch (cmd)
    {
        case SYSTEM_CMD_CREATE:
            if(pObj->state==SYSTEM_LINK_STATE_IDLE)
            {
#if defined (BUILD_ARP32)
                Utils_idleEnableEveDMA();
#endif
                status = AlgorithmLink_pluginCreate(pObj, pMsg);
                if(status==SYSTEM_LINK_STATUS_SOK)
                {
                    pObj->state = SYSTEM_LINK_STATE_RUNNING;
                }
#if defined (BUILD_ARP32)
                Utils_idleDisableEveDMA();
#endif
            }
            Utils_tskAckOrFreeMsg(pMsg, status);
            break;
        case SYSTEM_CMD_NEW_DATA:

          Utils_tskAckOrFreeMsg(pMsg, 0);

          flushCmds[0U] = SYSTEM_CMD_NEW_DATA;
          Utils_tskFlushMsg(pTsk, flushCmds, 1U);

          if(pObj->state==SYSTEM_LINK_STATE_RUNNING)
          {
              if(gAlgorithmLinkFuncTable[pObj->algId].AlgorithmLink_AlgPluginProcess
              != NULL)
              {
#if defined (BUILD_ARP32)
                  Utils_idleEnableEveDMA();
#endif
                  status =
               (*(gAlgorithmLinkFuncTable[pObj->algId].AlgorithmLink_AlgPluginProcess))
                    ((void *) pObj);

#if defined (BUILD_ARP32)
                  Utils_idleDisableEveDMA();
#endif
              }
          }

          break;

        case ALGORITHM_LINK_CMD_CONFIG:

          if(pObj->state==SYSTEM_LINK_STATE_RUNNING)
          {
              pControlParams =
                (AlgorithmLink_ControlParams *)Utils_msgGetPrm(pMsg);
#if defined (BUILD_ARP32)
              Utils_idleEnableEveDMA();
#endif
              if(gAlgorithmLinkFuncTable[pObj->algId].AlgorithmLink_AlgPluginControl
              != NULL)
              {
               status =
               (*(gAlgorithmLinkFuncTable[pObj->algId].AlgorithmLink_AlgPluginControl))
                    ((void *) pObj,
                     (void *) pControlParams
                    );
              }
#if defined (BUILD_ARP32)
              Utils_idleDisableEveDMA();
#endif
          }

          Utils_tskAckOrFreeMsg(pMsg, status);
          break;

        case SYSTEM_CMD_STOP:

          if(pObj->state==SYSTEM_LINK_STATE_RUNNING)
          {
#if defined (BUILD_ARP32)
              Utils_idleEnableEveDMA();
#endif
              status = AlgorithmLink_pluginStop(pObj);
              pObj->state=SYSTEM_LINK_STATE_STOPPED;
#if defined (BUILD_ARP32)
              Utils_idleDisableEveDMA();
#endif
          }
          Utils_tskAckOrFreeMsg(pMsg, status);
          break;

        /*
         * Exiting Ready state after deleting the driver
         */
        case SYSTEM_CMD_DELETE:
#if defined (BUILD_ARP32)
          Utils_idleEnableEveDMA();
#endif
          if(pObj->state==SYSTEM_LINK_STATE_RUNNING)
          {
              status = AlgorithmLink_pluginStop(pObj);
              pObj->state=SYSTEM_LINK_STATE_STOPPED;
          }
          if(pObj->state==SYSTEM_LINK_STATE_STOPPED)
          {
              #ifdef SYSTEM_DEBUG_ALGORITHM
              Vps_printf(" ALGORITHM: Delete in progress"
                           " (algId = %d) !!!\n", pObj->algId);
              #endif

              if(gAlgorithmLinkFuncTable[pObj->algId].AlgorithmLink_AlgPluginDelete
              != NULL)
              {
               status =
               (*(gAlgorithmLinkFuncTable[pObj->algId].AlgorithmLink_AlgPluginDelete))
                    ((void *) pObj);
              }

              #ifdef SYSTEM_DEBUG_ALGORITHM
              Vps_printf(" ALGORITHM: Delete Done"
                           " (algId = %d) !!!\n", pObj->algId);
              #endif

              pObj->state=SYSTEM_LINK_STATE_IDLE;
          }
#if defined (BUILD_ARP32)
          Utils_idleDisableEveDMA();
#endif
          Utils_tskAckOrFreeMsg(pMsg, status);

          break;

        /*
         * Start is made dummy for this link, since there is no need.
         */
        case SYSTEM_CMD_START:
            /*
             * Invalid command for this state.  ACK it and continue RUN
             */
            Utils_tskAckOrFreeMsg(pMsg, status);
            break;

        default:
            if(pObj->state==SYSTEM_LINK_STATE_RUNNING)
            {
                AlgorithmLink_ControlParams controlParams;

                /* all other commands, forward as control command */
                controlParams.size = sizeof(controlParams);
                controlParams.controlCmd = cmd;

                if(gAlgorithmLinkFuncTable[pObj->algId].AlgorithmLink_AlgPluginControl
                  != NULL)
                {
#if defined (BUILD_ARP32)
                    Utils_idleEnableEveDMA();
#endif
                    status =
                    (*(gAlgorithmLinkFuncTable[pObj->algId].AlgorithmLink_AlgPluginControl))
                        ((void *) pObj,
                         (void *) &controlParams
                        );
#if defined (BUILD_ARP32)
                    Utils_idleDisableEveDMA();
#endif
                }
            }
            Utils_tskAckOrFreeMsg(pMsg, status);
            break;
    }
    return;

}

/**
 *******************************************************************************
 *
 *   \brief Algorithm link register and init function
 *
 *          For each algorithm instance
 *          - Creates link task
 *          - Registers as a link with the system API
 *
 *   \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 AlgorithmLink_init(void)
{
    Int32 status;
    System_LinkObj linkObj;
    UInt32 algLinkId;

    AlgorithmLink_Obj *pObj;
    UInt32 procId = System_getSelfProcId();

    for (algLinkId = 0U; algLinkId < ALGORITHM_LINK_OBJ_MAX; algLinkId++)
    {
        pObj = &gAlgorithmLink_obj[algLinkId];

        memset(pObj, 0, sizeof(AlgorithmLink_Obj));

        pObj->linkId = SYSTEM_MAKE_LINK_ID(procId, SYSTEM_LINK_ID_ALG_0) +
                       algLinkId;

        pObj->state=SYSTEM_LINK_STATE_IDLE;

        linkObj.pTsk                = &pObj->tsk;
        linkObj.linkGetFullBuffers  = &AlgorithmLink_getFullBuffers;
        linkObj.linkPutEmptyBuffers = &AlgorithmLink_putEmptyBuffers;
        linkObj.getLinkInfo         = &AlgorithmLink_getInfo;

        System_registerLink(pObj->linkId, &linkObj);

        /*
         * Create link task, task remains in IDLE state.
         * AlgorithmLink_tskMain is called when a message command is received.
         */
        status = AlgorithmLink_tskCreate(algLinkId);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    }

    AlgorithmLink_initAlgPlugins();

    return status;
}

/**
 *******************************************************************************
 *
 *   \brief Algorithm link de-register and de-init function
 *
 *          For each algorithm instance
 *          - Deletes link task
 *          - De-registers as a link with the system API
 *
 *   \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 AlgorithmLink_deInit(void)
{
    UInt32 algorithmId;

    for (algorithmId = 0U; algorithmId < ALGORITHM_LINK_OBJ_MAX; algorithmId++)
    {
        Utils_tskDelete(&gAlgorithmLink_obj[algorithmId].tsk);
    }
    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Get the buffer and queue information about link.
 *
 * \param  ptr  [IN] Task Handle
 * \param  info [IN] Pointer to link information handle

 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 AlgorithmLink_getInfo(Void * ptr, System_LinkInfo * info)
{
    Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;

    AlgorithmLink_Obj *pObj = (AlgorithmLink_Obj *) pTsk->appData;

    memcpy(info, &pObj->linkInfo, sizeof(System_LinkInfo));

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Callback function implemented by algorithm link to give full buffers
 *        to next link.
 *
 *        Algorithm link sends message to next link about availability
 *        of buffers. Next link calls this callback function to get full buffers
 *        from algorithm output queue.
 *
 * \param  ptr      [IN] Task Handle
 * \param  queId    [IN] queId from which buffers are required.
 * \param  pBufList [IN] pointer to system buffer list to hold system buffers
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 AlgorithmLink_getFullBuffers(Void              * ptr,
                                   UInt16              queId,
                                   System_BufferList * pBufList)
{
    Utils_TskHndl *pTsk     = (Utils_TskHndl *) ptr;
    AlgorithmLink_Obj *pObj = (AlgorithmLink_Obj *) pTsk->appData;
    Int32 status;
    UInt32 idx;

    UTILS_assert(queId < pObj->numOutputQUsed);

    for (idx = 0U; idx < SYSTEM_MAX_BUFFERS_IN_BUFFER_LIST; idx++)
    {
        /* MISRA.CAST.PTR
         * MISRAC_2004 Rule_11.4
         * MISRAC_WAIVER:
         * This is a generic queue implementation which supports all dataTypes.
         * This typecasting is necessary.
         */
        status = Utils_queGet(&(pObj->fullOutputQ[queId].queHandle),
                              (Ptr *)&(pBufList->buffers[idx]),
                              1U,
                              BSP_OSAL_NO_WAIT);

        if (status != SYSTEM_LINK_STATUS_SOK)
        {
            break;
        }
    }

    pBufList->numBuf = idx;

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Callback function implemented by link to receive empty buffers from
 *        next link.
 *
 *
 *
 * \param  ptr      [IN] Task Handle
 * \param  queId    [IN] queId for which buffers are received.
 * \param  pBufList [IN] Pointer to link information handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 AlgorithmLink_putEmptyBuffers(Void              *ptr,
                                    UInt16             queId,
                                    System_BufferList *pBufList)
{
    Utils_TskHndl *pTsk     = (Utils_TskHndl *) ptr;
    AlgorithmLink_Obj *pObj = (AlgorithmLink_Obj *) pTsk->appData;
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    UTILS_assert(queId < pObj->numOutputQUsed);

    if(pObj->callbackPutEmptyBuffers)
    {
        pObj->callbackPutEmptyBuffers(pObj, (UInt32)queId, pBufList);
    }

    status = AlgorithmLink_releaseOutputBuffer(pObj,
                                                   (UInt32) queId,
                                                   pBufList);

    return status;
}


/* Nothing beyond this point */
