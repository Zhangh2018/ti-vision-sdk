/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "splitLink_priv.h"

/**
 *******************************************************************************
 * \brief Link Stack
 *******************************************************************************
 */
#pragma DATA_ALIGN(gSplitLink_tskStack, 32)
#pragma DATA_SECTION(gSplitLink_tskStack, ".bss:taskStackSection")

UInt8 gSplitLink_tskStack[SPLIT_LINK_OBJ_MAX][SPLIT_LINK_TSK_STACK_SIZE];

/**
 *******************************************************************************
 *
 * \brief Create task for this link
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
*/
Int32 SplitLink_tskCreate(UInt32 instId)
{
    Int32                status;
    SplitLink_Obj        *pObj;
    char                 tskName[32];

    pObj = &gSplitLink_obj[instId];

    sprintf(tskName, "SPLIT%u", (unsigned int)instId);

    /*
     * Create link task, task remains in IDLE state.
     * SplitLink_tskMain is called when a message command is received.
     */
    status = Utils_tskCreate(&pObj->tsk,
                             SplitLink_tskMain,
                             SPLIT_LINK_TSK_PRI,
                             gSplitLink_tskStack[instId],
                             SPLIT_LINK_TSK_STACK_SIZE,
                             pObj,
                             tskName);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    return status;
}

