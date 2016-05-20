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
 * \file displayLink_tsk.c
 *
 * \brief  This file has the implementation of Display Link Init and Run API
 *
 *         This file implements the state machine logic for this link.
 *         DisplayLink_init() get calls from system_init and the same create
 *         the link task and basic messaging interfaces. Once the link is
 *         initiated it waits for the create cmd. This create cmd creates the
 *         complete link infrastructure.  Then waits for various data and
 *         control cmds.
 *
 *         This file also implements the display link tear down functionality
 *
 * \version 0.0 (Jun 2013) : [SS] First version
 * \version 0.1 (Jul 2013) : [SS] Updates as per code review comments
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "displayLink_priv.h"

/*******************************************************************************
 *                  Internal Function Declarations
 *******************************************************************************
 */
Int32 DisplayLink_tskRun(DisplayLink_Obj *pObj, Utils_TskHndl *pTsk,
                         Utils_MsgHndl **pMsg, Bool *done, Bool *ackMsg);

Void DisplayLink_tskMain(struct Utils_TskHndl_t *pTsk, Utils_MsgHndl *pMsg);

/**
 *******************************************************************************
 * \brief Link Stack
 *******************************************************************************
 */
#pragma DATA_ALIGN(gDisplayLink_tskStack, 32)
#pragma DATA_SECTION(gDisplayLink_tskStack, ".bss:taskStackSection")
UInt8 gDisplayLink_tskStack[DISPLAY_LINK_OBJ_MAX][DISPLAY_LINK_TSK_STACK_SIZE];

/**
 *******************************************************************************
 * \brief Display Link object, stores all link related information
 *******************************************************************************
 */
DisplayLink_Obj gDisplayLink_obj[DISPLAY_LINK_OBJ_MAX];

/**
 *******************************************************************************
 *
 * \brief This function implements the display link Run/Steady state
 *
 *        In this state link gets commands to
 *         - Stop/delete of link
 *         - Data events/cmds
 *         - All dynamic cmds that the link supports
 *         - All stats/status cmds
 *
 * \param  pObj     [IN]  Display link instance handle
 * \param  pTsk     [IN]  Link Task Handle
 * \param  pMsg     [IN]  Message Handle
 * \param  done     [IN]  display link Run state, set to TRUE once it get the
 *                        is DELETE CMD
 * \param  ackMsg   [OUT] ACK message
 *
 * \return status   SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 DisplayLink_tskRun(DisplayLink_Obj *pObj, Utils_TskHndl *pTsk,
                         Utils_MsgHndl **pMsg, Bool *done, Bool *ackMsg)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    Bool runDone, runAckMsg;
    Utils_MsgHndl *pRunMsg;
    UInt32 cmd;
    UInt32 flushCmds[1];

    *done = (Bool)FALSE;
    *ackMsg = (Bool)FALSE;

    runDone = (Bool)FALSE;
    runAckMsg = (Bool)FALSE;

    *pMsg = NULL;

    /*
     * This while loop implements RUN state. All the run time commands for
     * ackMsg Link are received and serviced in this while loop.
     * Control remains in this loop until delete commands arrives.
     */
    while (!runDone)
    {
        status = Utils_tskRecvMsg(pTsk, &pRunMsg, BSP_OSAL_WAIT_FOREVER);
        if (status != SYSTEM_LINK_STATUS_SOK)
        {
            break;
        }

        /* extract message command from message */
        cmd = Utils_msgGetCmd(pRunMsg);

        /*
         * Different commands are serviced via this switch case. For each
         * command, after servicing, ACK or free message is sent before
         * proceeding to next state.
         */
        switch (cmd)
        {
            case DISPLAY_LINK_CMD_RELEASE_FRAMES:
                Utils_tskAckOrFreeMsg(pRunMsg, status);

                flushCmds[0] = DISPLAY_LINK_CMD_RELEASE_FRAMES;
                Utils_tskFlushMsg(pTsk, flushCmds, 1U);

                DisplayLink_drvReleaseData(pObj);
                break;

            case DISPLAY_LINK_CMD_GET_STATISTICS:
                DisplayLink_drvGetStatistics(pObj, Utils_msgGetPrm(pRunMsg));
                Utils_tskAckOrFreeMsg(pRunMsg, status);
                break;

            case SYSTEM_CMD_STOP:
                runDone = (Bool)TRUE;
                runAckMsg = (Bool)TRUE;
                break;

            case SYSTEM_CMD_NEW_DATA:
                Utils_tskAckOrFreeMsg(pRunMsg, status);

                flushCmds[0] = SYSTEM_CMD_NEW_DATA;
                Utils_tskFlushMsg(pTsk, flushCmds, 1U);

                DisplayLink_drvProcessData(pObj);
                break;

            case SYSTEM_CMD_DELETE:
                *done = (Bool)TRUE;
                *ackMsg = (Bool)TRUE;
                *pMsg = pRunMsg;
                runDone = (Bool)TRUE;
                break;

            /*
             * Command to switch to a new input channel that is to be
             * displayed on the display
             */
            case DISPLAY_LINK_CMD_SWITCH_CH:
                status =
                    DisplayLink_drvSwitchCh(pObj,
                                            Utils_msgGetPrm(pRunMsg));
                Utils_tskAckOrFreeMsg(pRunMsg, status);
                break;

            case SYSTEM_CMD_PRINT_STATISTICS:
                DisplayLink_drvPrintStatistics(pObj);
                Utils_tskAckOrFreeMsg(pRunMsg, status);
                break;

            default:
                Utils_tskAckOrFreeMsg(pRunMsg, status);
                break;
        }

    }

    DisplayLink_drvStop(pObj);

    if (runAckMsg)
    {
        Utils_tskAckOrFreeMsg(pRunMsg, status);
    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief This function implements the START/RUN state of display Link.
 *
 *        In this state link gets commands to
 *         - Create the Display Driver
 *         - Start the Display Driver
 *         - Moves to RUN state
 * \param  pTsk [IN] Task Handle
 * \param  pMsg [IN] Message Handle
 *
 * \return  void
 *
 *******************************************************************************
 */
Void DisplayLink_tskMain(struct Utils_TskHndl_t *pTsk, Utils_MsgHndl *pMsg)
{
    UInt32 cmd = Utils_msgGetCmd(pMsg);
    Bool ackMsg, done;
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    DisplayLink_Obj *pObj = (DisplayLink_Obj *) pTsk->appData;

    /*
     * At this stage only create command is the expected command.
     * If other message gets received Ack with error status
     */
    if (cmd != SYSTEM_CMD_CREATE)
    {
        Utils_tskAckOrFreeMsg(pMsg, SYSTEM_LINK_STATUS_EUNSUPPORTED_CMD);
        status = SYSTEM_LINK_STATUS_EFAIL;
    }

    if(status == SYSTEM_LINK_STATUS_SOK)
    {
        /*
         * Create command received, create the driver
         */
        status = DisplayLink_drvCreate(pObj, Utils_msgGetPrm(pMsg));

        /* ACK based on create status */
        Utils_tskAckOrFreeMsg(pMsg, status);
    }

    if (status == SYSTEM_LINK_STATUS_SOK)
    {

        done = (Bool)FALSE;
        ackMsg = (Bool)FALSE;

        while (!done)
        {
            status = Utils_tskRecvMsg(pTsk, &pMsg, BSP_OSAL_WAIT_FOREVER);
            if (status != SYSTEM_LINK_STATUS_SOK)
            {
                break;
            }

            cmd = Utils_msgGetCmd(pMsg);

            switch (cmd)
            {
                /*
                 * CMD to prime and start the display driver. Link will be in
                 * running state and ready to displays the video/graphic frames
                 * available at the input side
                 */
                case SYSTEM_CMD_START:
                    /* Start display driver */
                    status = DisplayLink_drvStart(pObj);

                    /* ACK based on create status */
                    Utils_tskAckOrFreeMsg(pMsg, status);

                    /*
                     * Entering RUN state
                     */
                    if (SYSTEM_LINK_STATUS_SOK == status)
                    {
                        status =
                            DisplayLink_tskRun(pObj, pTsk, &pMsg, &done, &ackMsg);
                    }

                    break;
                case SYSTEM_CMD_DELETE:
                    /* exit READY state */
                    done = (Bool)TRUE;
                    ackMsg = (Bool)TRUE;
                    break;
                case DISPLAY_LINK_CMD_SWITCH_CH:
                    status =
                        DisplayLink_drvSwitchCh(pObj,
                                                Utils_msgGetPrm(pMsg));
                    Utils_tskAckOrFreeMsg(pMsg, status);
                    break;

                default:
                    /* invalid command for this state ACK it and continue READY
                    * loop */
                    Utils_tskAckOrFreeMsg(pMsg, status);
                    break;
            }
        }

        /* exiting READY state, delete driver */
        DisplayLink_drvDelete(pObj);

        /* ACK message if not previously ACK'ed */
        if ((ackMsg != (Bool)FALSE) && (pMsg != NULL))
        {
            Utils_tskAckOrFreeMsg(pMsg, status);
        }
    }
    /* entering IDLE state */
    return;
}

/**
 *******************************************************************************
 *
 *   \brief Display link register and init function
 *
 *          For each display instance (VID1, VID2, VID3 or GRPX1)
 *          - Creates link task
 *          - Registers as a link with the system API
 *
 *   \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 DisplayLink_init(void)
{
    Int32 status;
    System_LinkObj linkObj;
    UInt32 displayId;
    DisplayLink_Obj *pObj;
    char tskName[32];

    for (displayId = 0; displayId < DISPLAY_LINK_OBJ_MAX; displayId++)
    {
        pObj = &gDisplayLink_obj[displayId];

        memset(pObj, 0, sizeof(DisplayLink_Obj));

        pObj->tskId = SYSTEM_LINK_ID_DISPLAY_0 + displayId;

        linkObj.pTsk = &pObj->tsk;
        linkObj.linkGetFullBuffers = NULL;
        linkObj.linkPutEmptyBuffers = NULL;
        linkObj.getLinkInfo = NULL;

        System_registerLink(pObj->tskId, &linkObj);

        snprintf(tskName, sizeof(tskName)-1U, "DISPLAY%d", displayId);

        /*
         * Create link task, task remains in IDLE state.
         * DisplayLink_tskMain is called when a message command is received.
         */
        status = Utils_tskCreate(&pObj->tsk,
                                 DisplayLink_tskMain,
                                 DISPLAY_LINK_TSK_PRI,
                                 gDisplayLink_tskStack[displayId],
                                 DISPLAY_LINK_TSK_STACK_SIZE, pObj, tskName);
        UTILS_assert(SYSTEM_LINK_STATUS_SOK == status);
    }

    return status;
}

/**
 *******************************************************************************
 *
 *   \brief Display link de-register and de-init function
 *
 *          For each display instance (VID1, VID2, VID3 or GRPX1)
 *          - Deletes link task
 *          - De-registers as a link with the system API
 *
 *   \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 DisplayLink_deInit(void)
{
    UInt32 displayId;

    for (displayId = 0; displayId < DISPLAY_LINK_OBJ_MAX; displayId++)
    {
        Utils_tskDelete(&gDisplayLink_obj[displayId].tsk);
    }
    return SYSTEM_LINK_STATUS_SOK;
}

/* Nothing beyond this point */
