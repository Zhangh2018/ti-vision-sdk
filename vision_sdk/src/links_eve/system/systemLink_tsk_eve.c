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
 * \file systemLink_tsk_eve.c
 *
 * \brief  This file has the implementataion of systemlink task and
 *         handles the commands received.
 *
 * \version 0.0 (Jun 2013) : [SS] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "systemLink_priv_eve.h"
#include <src/utils_common/include/utils_idle.h>

/**
 *******************************************************************************
 * \brief Link Object holds the task id and handle
 *******************************************************************************
 */
SystemLink_Obj gSystemLink_obj;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

static Int32 SystemLink_cmdHandler(SystemLink_Obj * pObj, UInt32 cmd, Void * pPrm);

Void SystemLink_tskMain(struct Utils_TskHndl_t * pTsk, Utils_MsgHndl * pMsg);

/**
 *******************************************************************************
 *
 * \brief Handles the Commands received.
 *        Handles the commands for Profiling and core stats
 *
 * \param   pObj           [IN] SystemLink_Obj
 *
 * \param   cmd            [IN] input command
 *
 * \param   pPrm           [IN] input message
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
static Int32 SystemLink_cmdHandler(SystemLink_Obj * pObj, UInt32 cmd, Void * pPrm)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    switch (cmd)
    {
        case SYSTEM_COMMON_CMD_CPU_LOAD_CALC_START:
            Utils_prfLoadCalcStart();
            break;

        case SYSTEM_COMMON_CMD_CPU_LOAD_CALC_STOP:
            Utils_prfLoadCalcStop();
            break;

        case SYSTEM_COMMON_CMD_CPU_LOAD_CALC_RESET:
            Utils_prfLoadCalcReset();
            break;

        case SYSTEM_COMMON_CMD_PRINT_STATUS:
        {
            SystemCommon_PrintStatus *prm = (SystemCommon_PrintStatus *) pPrm;

            if (prm->printCpuLoad)
            {
                status = Utils_prfLoadPrintAll(prm->printTskLoad);
            }
            if (prm->printHeapStatus)
            {
                System_memPrintHeapStatus();
            }
        }
            break;

        case SYSTEM_COMMON_CMD_CORE_STATUS:
            Vps_printf(" Core is active\n");
            break;

        case SYSTEM_COMMON_CMD_GET_LOAD:
            Utils_prfGetLoad(pPrm);
            break;

        case SYSTEM_COMMON_CMD_RUN_DMA_TEST:
            Utils_dmaTestCopyFill(FALSE);
            break;
        case SYSTEM_COMMON_CMD_RUN_TIMER_TEST:
            Utils_TestTimer((Int32 *)pPrm);
            break;

        default:
            break;
    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Creates Command handler task
 *        handles the commands for Profiling and core stats
 *
 * \param   pTsk    [IN]  Utils_TskHndl
 *
 * \param   pMsg    [IN]  Utils_MsgHndl
 *
 *******************************************************************************
*/
    /*
    * MISRA.PPARAM.NEEDS.CONS
    * MISRAC_2004_Rule_16.7
    * Function parameter is not declared as a pointer to const.
    * In Other Links it calls Utils_tskRecvMsg where the memory
    * pointed by pTsk will be updated .So ptask cannot be made const
    * KW State: Defer -> Waiver -> Case by case
    */

Void SystemLink_tskMain(struct Utils_TskHndl_t * pTsk, Utils_MsgHndl * pMsg)
{
    Int32 status;
    SystemLink_Obj *pObj = (SystemLink_Obj *) pTsk->appData;

    status = SystemLink_cmdHandler(pObj,
                                   Utils_msgGetCmd(pMsg),
                                   Utils_msgGetPrm(pMsg));

    SystemLink_userCmdHandler(Utils_msgGetCmd(pMsg),
                            Utils_msgGetPrm(pMsg));

    Utils_tskAckOrFreeMsg(pMsg, status);

    return;
}

/**
 *******************************************************************************
 *
 * \brief Initialize the ipu systemLink
 *        Initializes the linkObj and creates the task for SystemLink_tskMain
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 SystemLink_init(void)
{
    Int32 status;
    System_LinkObj linkObj;
    SystemLink_Obj *pObj;
    UInt32 coreId;

    pObj = &gSystemLink_obj;

    memset(pObj, 0, sizeof(*pObj));
    coreId = System_getSelfProcId();
    pObj->tskId = SYSTEM_MAKE_LINK_ID(coreId, SYSTEM_LINK_ID_PROCK_LINK_ID);

    memset(&linkObj, 0, sizeof(linkObj));
    linkObj.pTsk = &pObj->tsk;
    linkObj.linkGetFullBuffers = NULL;
    linkObj.linkPutEmptyBuffers = NULL;
    linkObj.getLinkInfo = NULL;

    System_registerLink(pObj->tskId, &linkObj);

    status = SystemLink_tskCreate();
    UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief De-Initialize the ipu systemLink
 *        Delete the SystemLink_tskMain created
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 SystemLink_deInit(void)
{
    Utils_tskDelete(&gSystemLink_obj.tsk);

    return SYSTEM_LINK_STATUS_SOK;
}

/* Nothing beyond this point */
