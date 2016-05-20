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
#include "syncLink_priv.h"

/**
 *******************************************************************************
 * \brief Link Stack
 *******************************************************************************
 */
/* MISRA.PRAGMA
 * MISRAC_2004 Rule_3.4
 * MISRAC_WAIVER:
 * KW tool cannot understand the pragma supported by CGTools like
 * DATA_ALIGN and DATA_SECTION. But these are valid pragmas.
 */
#pragma DATA_ALIGN(gSyncLink_tskStack, 32)

/* MISRA.PRAGMA
 * MISRAC_2004 Rule_3.4
 * MISRAC_WAIVER:
 * KW tool cannot understand the pragma supported by CGTools like
 * DATA_ALIGN and DATA_SECTION. But these are valid pragmas.
 */
#pragma DATA_SECTION(gSyncLink_tskStack, ".bss:taskStackSection")

UInt8 gSyncLink_tskStack[SYNC_LINK_OBJ_MAX][SYNC_LINK_TSK_STACK_SIZE];

/**
 *******************************************************************************
 *
 * \brief Create task for this link
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
*/
Int32 SyncLink_tskCreate(UInt32 instId)
{
    Int32                status;
    SyncLink_Obj        *pObj;
    char                 tskName[32];

    pObj = &gSyncLink_obj[instId];

    snprintf(tskName, sizeof(tskName), "SYNC%u", instId);

    /*
     * Create link task, task remains in IDLE state.
     * SyncLink_tskMain is called when a message command is received.
     */
    status = Utils_tskCreate(&pObj->tsk,
                             SyncLink_tskMain,
                             SYNC_LINK_TSK_PRI,
                             gSyncLink_tskStack[instId],
                             SYNC_LINK_TSK_STACK_SIZE,
                             pObj,
                             tskName);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    return status;
}

