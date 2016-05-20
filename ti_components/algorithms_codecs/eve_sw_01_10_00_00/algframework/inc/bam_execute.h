/*==========================================================================*/
/*      Copyright (C) 2009-2013 Texas Instruments Incorporated.             */
/*                      All Rights Reserved                                 */
/*==========================================================================*/


/**
 *  @file       bam_execute.h
 *
 *  @brief      This header defines interfaces for BAM Graph execution
 */

#ifndef BAM_EXECUTE_H
#define BAM_EXECUTE_H

#pragma CHECK_MISRA ("none")
#include <stdint.h>
#include <stdlib.h>
#pragma RESET_MISRA ("required")

#include "bam_common.h"
#include "bam_kernel_interface.h"

/**
 *  @brief      Specifies the priority of processing.
 */
typedef enum
{
    BAM_COMPUTE_FIRST = 1, /**< Prioritize compute nodes over source/sink nodes, for compute-bound graphs (default behaviour) */
    BAM_DATA_FIRST = 2,   /**< Prioritize source/sink nodes over compute nodes, for IO bound graphs */
    BAM_SINGLE_BLOCK = 3,   /**< Enable optimized processing schedule where only one single block fitting in image buffer is processed. The single block can be located in external memory as EDMA is still used to fetch it */
    BAM_SINGLE_BLOCK_NO_EDMA = 4 /**< Enable optimized processing schedule where only one single block fitting in image buffer is processed. The single block must already be in VCOP memory as EDMA is NOT used to fetch it */
} BAM_ProcessPri;

/**
 *  @brief      Optional structure that provides processing 'hint' to BAM_process()
 */
typedef struct {
    BAM_ProcessPri priority; /**< Specify whether processing schedule should prioritize compute nodes (default) over data transfers nodes */
} BAM_ProcessHints;

/**
 *  @brief      Performs the algorithm process on a single frame or slice.
 *
 *  @param[in] graphHandle      Pointer to the graph handle containig information about kernels,
 *                              kernel contexts, input, output and internal datablock information regarding each kernel and graph connections etc.
 *  @param[in] inArgs           Pointer to algorithm input arguments needed to be passed for frame/slice level processing.
 *  @param[in] outArgs          Pointer to algorithm output arguments needed to be returned after frame/slice level processing.
 *  @param[in] hints            Pointer to hints structure. If NULL, then argument is ignored. Processing schedule gives priority to compute nodes by default, assuming the graph is compute-bound.
 *
 *  @retval     int32_t
 *
 *  @remarks    This function expects a valid graphHandle and input and output buffers and arguments.
 *              This function assumes suffcient amount of stack is allocated to avoid stack overflow.
 *
 *  @remarks    Inside this function InitInstance() of each node is called once per entire algorithm execution and
 *              InitFrame() of each node at the beginning of every frame/slice processing. At present it can support only single source node and single sink node.
 *
 */
int32_t BAM_process(const BAM_GraphHandle graphHandle, const BAM_InArgs *inArgs, BAM_OutArgs *outArgs, const BAM_ProcessHints *hints);

/**
 *  @brief      Performs the algorithm process on a single block.
 *
 *  @param[in] graphHandle      Pointer to the graph handle containig information about kernels,
 *                              kernel contexts, input, output and internal datablock information regarding each kernel and graph connections etc.
 *  @param[in] inBufs           Pointer to algorithm input buffer descriptor containing infromation about the input frame/slice data buffer.
 *  @param[in] outBufs          Pointer to algorithm input buffer descriptor containing infromation about the output frame/slice data buffer.
 *  @param[in] inArgs           Pointer to algorithm input arguments needed to be passed for frame/slice level processing.
 *  @param[in] outArgs          Pointer to algorithm output arguments needed to be returned after frame/slice level processing.
 *  @param[in] hints            Pointer to hints structure. If NULL, then argument is ignored. Processing schedule gives priority to compute nodes by default, assuming the graph is compute-bound.
 *
 *  @retval     int32_t
 *
 *  @remarks    This function expects a valid graphHandle and input and output buffers and arguments.
 *              This function assumes suffcient amount of stack is allocated to avoid stack overflow.
 *
 *  @remarks    Inside this function InitInstance() of each node is called once per entire algorithm execution and
 *              InitFrame() of each node at the beginning of every frame/slice processing. At present it can support only single source node and single sink node.
 *
 */
int32_t BAM_processSingleBlock(const BAM_GraphHandle graphHandle, const BAM_InBufs *inBufs, const BAM_OutBufs *outBufs, const BAM_InArgs *inArgs, BAM_OutArgs *outArgs, const BAM_ProcessHints *hints);

/**
 *  @brief      BAM API for saving a memRec for the purpose of context save. This function basically copies the contents pointed
                by the .base field of the memRec to the memory pointed by .ext field. It refers to .size field for the amount of data to be copied.
                memRecs with attributes BAM_MEMATTRS_CONST & BAM_MEMATTRS_SCRATCH will not be saved.
 *
 *  @param[in]  memRec          Pointer to the memRec to be saved.
 *
 *  @retval     int32_t
 *
 *  @remarks    This function internally uses QDMA for data transfer. Please refer to the QDMA interface for any possible QDMA resource conflict.
 */
int32_t BAM_contextSaveMemRec(BAM_MemRec *memRec);

/**
 *  @brief      BAM API for restoring a memRec for the purpose of context restore. This function basically copies the contents pointed
                by the .ext field of the memRec to the memory pointed by .base field. It refers to .size field for the amount of data to be copied.
                All memRecs except those with attribute BAM_MEMATTRS_SCRATCH will not be restored.
 *
 *  @param[in]  memRec          Pointer to the memRec to be saved.
 *
 *  @retval     int32_t
 *
 *  @remarks    This function internally uses QDMA for data transfer. Please refer to the QDMA interface for any possible QDMA resource conflict.
 */
int32_t BAM_contextRestoreMemRec(BAM_MemRec *memRec);

/**
 *  @brief      BAM API for restoring an entire graph so that context can be switched. This function basically copies the contents pointed
                by the .ext field of each memRec of the graph to the memory pointed by its .base field. It refers to .size field for the amount of data to be copied.
                memRecs with attributes BAM_MEMATTRS_CONST will not be restored.
 *
*  @param[in]  graphHandle     handle to the graph object which may have been previously copied from external memory to internal data memory
 *
 *  @retval     int32_t
 *
 *  @remarks    This function internally uses QDMA for data transfer. Please refer to the QDMA interface for any possible QDMA resource conflict.
 */
int32_t BAM_activateGraph(const BAM_GraphHandle graphHandle);

/**
 *  @brief      BAM API for saving an entire graph so that context can be switched. This function basically copies the contents pointed
                by the .base field of each memRec of the graph to the memory pointed by its .ext field. It refers to .size field for the amount of data to be copied.
                memRecs with attributes BAM_MEMATTRS_CONST & BAM_MEMATTRS_SCRATCH will not be saved.
 *
 *  @param[in]  graphHandle     handle to the graph object which may have been previously copied from internal data memory to external memory
 *
 *  @retval     int32_t
 *
 *  @remarks    This function internally uses QDMA for data transfer. Please refer to the QDMA interface for any possible QDMA resource conflict.
 */
int32_t BAM_deActivateGraph(const BAM_GraphHandle graphHandle);

/**
 *  @brief      BAM_controlNode() can either query or modify the value of one or several arguments associated to the node referred by nodeIndex composing the graph pointed by graphHandle.
                Actually BAM_controlNode() just calls the BAM_KernelExecFundDef::kernelCtl()of the kernel associated to the node. In some way, it acts as a wrapper, by looking up the function 
                to call and passing the generic argument structure controlArgs. The underlying behavior, e.g. whether to query of modify any argument values is implemented by the kernel execute function 
                BAM_KernelExecFundDef::kernelCtl() which has the following prototype:
                typedef uint32_t (*BAM_KernelCtlFunc)(void *kernelContext, void * controlArgs);
 *
 *  @param[in]  graphHandle     handle to the graph object which may have been previously copied from internal data memory to external memory
 *
 *  @param[in]  nodeIndex       set to one of the identifier used when specifying the node list BAM_CreateGraphParams::nodeList during graph creation.
 *
 *  @param[in]  controlArgs     points to a custom control argument structure, specific to a kernel.
 *
 *  @retval     int32_t
 *
 *  @remarks    
 */
BAM_Status BAM_controlNode(BAM_GraphHandle graphHandle, uint8_t nodeIndex, void *controlArgs);

#endif /* BAM_EXEUTE_H */
