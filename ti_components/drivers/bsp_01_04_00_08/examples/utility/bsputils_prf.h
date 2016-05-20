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
 *  \ingroup BSPUTILS_API
 *  \defgroup BSPUTILS_PRF_API Profiling API
 *  @{
 */

/**
 *  \file bsputils_prf.h
 *
 *  \brief Profiling API.
 *
 *   - APIs to measure and print elasped time @ 64-bit precision
 *      - BspUtils_prfTsXxxx
 *
 *   - APIs to measure and print CPU load at task, HWI, SWI, global level
 *      - BspUtils_prfLoadXxxx
 */

#ifndef BSPUTILS_PRF_H_
#define BSPUTILS_PRF_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ti/sysbios/knl/Task.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/** \brief Maximum supported profiling objects */
#define BSPUTILS_PRF_MAX_HNDL           (64U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 *  \brief Profiling load object.
 */
typedef struct
{
    Int32 cpuLoad;
    /**< CPU load. */
    Int32 hwiLoad;
    /**< HWI load. */
    Int32 swiLoad;
    /**< SWI load. */
    Int32 tskLoad;
    /**< TSK load. */
} BspUtils_PrfLoad;

/** \brief Typedef for the loadupdate function for the user. */
typedef void (*BspUtils_loadUpdate)(BspUtils_PrfLoad *prfLoad);

/**
 *  \brief Profiling time-stamp object.
 */
typedef struct
{
    Char   name[32];
    /**< Name. */
    Bool   isAlloc;
    /**< Indicates if the object is initialized (used). */
    UInt64 startTs;
    /**< Start time stamp value. */
    UInt64 totalTs;
    /**< Total time stamp value. */
    UInt32 count;
    /**< Number of times time stamp delta is calculated. */
    UInt32 numFrames;
    /**< Total number of frames being used for time-stamping. */
} BspUtils_PrfTsHndl;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 *  \brief Initializes the profiling utility.
 *
 *  This function must be called before using any peformance or Timestamp utils
 *
 *  \return 0 on success, else failure
 */
Int32 BspUtils_prfInit(void);

/**
 *  \brief De-initializes the profiling utility.
 *
 *  \return 0 on success, else failure
 */
Int32 BspUtils_prfDeInit(void);

/**
 *  \brief Creates the handle for the time stamp taking.
 *
 *  \param name     [IN] Name of the time stamp object
 *
 *  \return Valid handle on success, else NULL
 */
BspUtils_PrfTsHndl *BspUtils_prfTsCreate(const Char *name);

/**
 *  \brief Deletes the handle for the timestamp.
 *
 *  \param pHndl    [IN] Handle to be deleted
 *
 *  \return 0 on success, else failure
 */
Int32 BspUtils_prfTsDelete(BspUtils_PrfTsHndl *pHndl);

/**
 *  \brief Start taking the timestamp.
 *
 *  \param pHndl    [IN] Handle to be time stamp object
 *
 *  \return Initial time stamp value on success
 */
UInt64 BspUtils_prfTsBegin(BspUtils_PrfTsHndl *pHndl);

/**
 *  \brief Stop taking the timestamp.
 *
 *  \param pHndl     [IN] Handle to be time stamp object
 *  \param numFrames [IN] Number of associated with the time stamp
 *
 *  \return Final time stamp value on success
 */
UInt64 BspUtils_prfTsEnd(BspUtils_PrfTsHndl *pHndl, UInt32 numFrames);

/**
 *  \brief Update the CPU load information for all profile handles
 */
void BspUtils_prfLoadUpdate(void);

/**
 *  \brief Calculates the difference between the timestamp.
 *
 *  \param pHndl     [IN] Handle to be time stamp object
 *  \param startTime [IN] Start time stamp value
 *  \param numFrames [IN] Number of associated with the time stamp
 *
 *  \return Final time stamp value on success
 */
UInt64 BspUtils_prfTsDelta(BspUtils_PrfTsHndl *pHndl,
                           UInt64              startTime,
                           UInt32              numFrames);

/**
 *  \brief Resets the timestamp counter for that handle.
 *
 *  \param pHndl     [IN] Handle to be time stamp object
 *
 *  \return 0 on success, else failure
 */
Int32 BspUtils_prfTsReset(BspUtils_PrfTsHndl *pHndl);

/**
 *  \brief Gets the 64-bit timer ticks
 *
 *  \return Current 64-bit timer ticks value on success
 */
UInt64 BspUtils_prfTsGet64(void);

/**
 *  \brief Prints the timestamp difference and resets the counter thereafter
 *         depending on specified resetAfterPrint parameter.
 *
 *  \param pHndl           [IN] Handle to be time stamp object
 *  \param resetAfterPrint [IN] Indicates whether time stamp values should be
 *                              reset after printing
 *
 *  \return 0 on success, else failure
 */
Int32 BspUtils_prfTsPrint(BspUtils_PrfTsHndl *pHndl, Bool resetAfterPrint);

/**
 *  \brief Prints the timestamp difference for all registered handles and resets
 *         the counter thereafter depending on specified resetAfterPrint
 *         parameter.
 *
 *  \param resetAfterPrint [IN] Indicates whether time stamp values should be
 *                              reset after printing
 *
 *  \return 0 on success, else failure
 */
Int32 BspUtils_prfTsPrintAll(Bool resetAfterPrint);

/**
 *  \brief Registers a task for load calculation.
 *
 *  \param pTsk            [IN] Handle to task object to be registered for load
 *                              calculation
 *  \param name            [IN] Name to be associated with the registered handle
 *
 *  \return 0 on success, else failure
 */
Int32 BspUtils_prfLoadRegister(Task_Handle pTsk,
                               const Char *name);
/**
 *  \brief Un-registers a task for load calculation.
 *
 *  \param pTsk            [IN] Handle to task object to be unregistered for
 *                              load calculation
 *
 *  \return 0 on success, else failure
 */
Int32 BspUtils_prfLoadUnRegister(Task_Handle pTsk);

/**
 *  \brief Prints loads for all the registered tasks. Also prints information
 *         for each task depending on the specified printTskLoad parameter.
 *
 *  \param printTskLoad    [IN] Indicates whether load information for each
 *                              registered task should be printed.
 *
 *  \return 0 on success, else failure
 */
Int32 BspUtils_prfLoadPrintAll(Bool printTskLoad);

/**
 *  \brief Start taking the performance load for all the registered tasks.
 */
void BspUtils_prfLoadCalcStart(void);

/**
 *  \brief Stop taking the load for all the registered tasks.
 */
void BspUtils_prfLoadCalcStop(void);

/**
 *  \brief Reset the load calculation mainly for next cycle of run.
 */
void  BspUtils_prfLoadCalcReset(void);

#ifdef __cplusplus
}
#endif

#endif /* ifndef BSPUTILS_PRF_H_ */

/* @} */
