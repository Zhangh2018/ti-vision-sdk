/*==========================================================================*/
/*      Copyright (C) 2009-2013 Texas Instruments Incorporated.             */
/*                      All Rights Reserved                                 */
/*==========================================================================*/

/**
 *  @file       blockSort_u32_alg.c
 *
 *  @brief       This file defines the XDAIS/iVISION functions for block sorting applet
 *  @version     0.0 (Dec 2013) : Base version.
 */

#pragma CHECK_MISRA ("none")
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#pragma RESET_MISRA ("required")

#include "edma_utils_memcpy.h"
#include "blockSort_u32_alg_int.h"

/* CHECK_MISRA("-15.1")  -> Disable rule 15.1 */
/* Goto statement is used */
/* Deviation is allowed because we use GOTO several times throughout the code but they are all forward GOTO to a single exit point */

/**
 * IVISION interface of the algorithm
 */
const IVISION_Fxns BLOCK_SORT_U32_TI_VISION_FXNS =
{
        {
                (void *)&BLOCK_SORT_U32_TI_VISION_FXNS, /* module ID */
                &BLOCK_SORT_U32_TI_activate,    /* activate */
                &BLOCK_SORT_U32_TI_alloc,       /* algAlloc */
                NULL,                          /* control (not suported) */
                &BLOCK_SORT_U32_TI_deactivate,  /* deactivate */
                &BLOCK_SORT_U32_TI_free,        /* free */
                &BLOCK_SORT_U32_TI_init,        /* init */
                NULL,                          /* moved (not suported) */
                &BLOCK_SORT_U32_TI_numAlloc     /* algNumAlloc  */
        },
        &BLOCK_SORT_U32_TI_process,
        &BLOCK_SORT_U32_TI_control
};


/**
 * IRES interface of the algorithm
 */
const IRES_Fxns BLOCK_SORT_U32_TI_IRES = {
        (void *)&BLOCK_SORT_U32_TI_IRES,
        NULL,                                     /*getResourceDescriptors, */
        NULL,                                     /*numResourceDescriptors */
        NULL,                                     /*initResources,  */
        NULL,                                     /*reinitResources, */
        NULL,                                     /*deinitResources, */
        NULL,                                     /*activateResource,*/
        NULL,                                     /*activateAllResources, */
        NULL,                                     /*deactivateResource,   */
        NULL                                      /*deactivateAllResources */
};

/**
 * Other exposed functions
 */
int32_t BLOCK_SORT_U32_TI_numAlloc(void)
{
    return NUM_MEMRECS;
}

/*---------------------------------------------------------------*/
/* Function to supply memory requirement to user                 */
/* refer algAlloc function for details in XADIS (ialg.h file)    */
/*---------------------------------------------------------------*/
int32_t BLOCK_SORT_U32_TI_alloc(const IALG_Params *params,
        IALG_Fxns **parentFxns,
        IALG_MemRec memRec[])
{
    int32_t status = IALG_EOK;

    BAM_GraphMemReq graphMemReq;

    /*---------------------------------------------------------*/
    /* Request BAM public function about memories associated   */
    /* to graph creation                                       */
    /*---------------------------------------------------------*/
    BLOCK_SORT_U32_TI_getGraphMemReq(&graphMemReq);

    /*-----------------------------------------------------------------------------*/
    /*  For memory record ALG_HANDLE_INT_MEMREC and GRAPH_OBJ_INT_MEMREC,          */
    /*  which are scratch buffers in BAM_MEMSPACE_DMEM, they must be preceded      */
    /*  by a persistent memory record in IALG_EXTERNAL of the same size.           */
    /*  This persistent record will be used to backup the following scratch        */
    /*  memory record during context saving when BLOCK_SORT_U32_TI_deactivate */
    /*  is called. Likewise when BLOCK_SORT_U32_TI_activate it called,        */
    /*  the content of the persistent memory record is copied back to the          */
    /*  scratch memory record in BAM_MEMSPACE_DMEM                                 */
    /*-----------------------------------------------------------------------------*/
    memRec[ALG_HANDLE_MEMREC].size          = sizeof(BLOCK_SORT_U32_TI_Obj);
    memRec[ALG_HANDLE_MEMREC].space         = (IALG_MemSpace)IALG_EXTERNAL;
    memRec[ALG_HANDLE_MEMREC].attrs         = IALG_PERSIST;
    memRec[ALG_HANDLE_MEMREC].alignment     = 4;

    memRec[ALG_HANDLE_INT_MEMREC].size      = sizeof(BLOCK_SORT_U32_TI_Obj);
    memRec[ALG_HANDLE_INT_MEMREC].space     = (IALG_MemSpace)IALG_DARAM0;
    memRec[ALG_HANDLE_INT_MEMREC].attrs     = IALG_SCRATCH;
    memRec[ALG_HANDLE_INT_MEMREC].alignment = 4;

    /*-------------------------------------------------------------------------*/
    /* Below 5 memory records are used to store data related to the BAM graph: */
    /* 1. GRAPH_OBJ_MEMREC     - To hold graph object in external memory       */
    /* 2. GRAPH_OBJ_INT_MEMREC - To hold the graph object in internal memory   */
    /* GRAPH OBJECT has 2 instances - internal and external memory - it is to  */
    /* speed up the processing - Main processing will work on internal memory  */
    /* instance and before any one else needing internal memory, deacitavte    */
    /* has to be performed by caller to make sure the data is backed up to ext */
    /* memory                                                                  */
    /*                                                                         */
    /* 3. GRAPH_SCRATCH_MEMREC -                                               */
    /*  It is used as scratch memory during graph create time                  */
    /*  Scratch mem is currently placed in external memory.                    */
    /*  For faster graph creation time, you can place it in internal memory    */
    /*                                                                         */
    /* 4. GRAPH_CONTEXT_MEMREC                                                 */
    /*  It is used to backup into external memory the graph's                  */
    /*  nodes internal memory records that are persistent.                     */
    /*                                                                         */
    /* 5. GRAPH_ARGS_MEMREC  -                                                 */
    /*   It is used keep kernels arguments during graph create time            */
    /*-------------------------------------------------------------------------*/

    memRec[GRAPH_OBJ_MEMREC].size           = graphMemReq.graphObjSize;
    memRec[GRAPH_OBJ_MEMREC].space          = (IALG_MemSpace) IALG_EXTERNAL;
    memRec[GRAPH_OBJ_MEMREC].attrs          = IALG_PERSIST;
    memRec[GRAPH_OBJ_MEMREC].alignment      = 4;

    /*
    GRAPH_OBJ_INT_MEMREC is allocated in DMEM and is used when the backup is
    restored from external memory to faster internal memory when activate is called.
     */
    memRec[GRAPH_OBJ_INT_MEMREC].size       = graphMemReq.graphObjSize;
    memRec[GRAPH_OBJ_INT_MEMREC].space      = (IALG_MemSpace) IALG_DARAM0;
    memRec[GRAPH_OBJ_INT_MEMREC].attrs      = IALG_SCRATCH;
    memRec[GRAPH_OBJ_INT_MEMREC].alignment  = 4;

    memRec[GRAPH_SCRATCH_MEMREC].size       =  graphMemReq.graphScratchSize;
    memRec[GRAPH_SCRATCH_MEMREC].space      =  (IALG_MemSpace) IALG_EXTERNAL;
    memRec[GRAPH_SCRATCH_MEMREC].attrs      =  IALG_SCRATCH;
    memRec[GRAPH_SCRATCH_MEMREC].alignment  = 4;

    memRec[GRAPH_CONTEXT_MEMREC].size       =  graphMemReq.graphcontextSize;
    memRec[GRAPH_CONTEXT_MEMREC].space      =  (IALG_MemSpace) IALG_EXTERNAL;
    memRec[GRAPH_CONTEXT_MEMREC].attrs      =  IALG_SCRATCH;
    memRec[GRAPH_CONTEXT_MEMREC].alignment  = 4;

    memRec[GRAPH_ARGS_MEMREC].size          =  graphMemReq.graphArgsSize;
    memRec[GRAPH_ARGS_MEMREC].space         =  (IALG_MemSpace) IALG_EXTERNAL;
    memRec[GRAPH_ARGS_MEMREC].attrs         =  IALG_SCRATCH;
    memRec[GRAPH_ARGS_MEMREC].alignment     = 4;

    return (status);
}

/*-----------------------------------------------------------------------*/
/* Function to accept memory pointers from user and initialize algorithm */
/* Refer algInit function for details in XADIS (ialg.h file)             */
/*-----------------------------------------------------------------------*/
int32_t BLOCK_SORT_U32_TI_init(IALG_Handle handle,
        const IALG_MemRec memRec[],
        IALG_Handle parent,
        const IALG_Params * params)
{
    int32_t status = IALG_EOK;

    BLOCK_SORT_U32_TI_CreateParams * createParams =
            (BLOCK_SORT_U32_TI_CreateParams*)(void*)params;

    BLOCK_SORT_U32_TI_graphCreateParams graphCreateParams;

    BLOCK_SORT_U32_TI_Handle algHandle =
            (BLOCK_SORT_U32_TI_Handle)(void*)memRec[ALG_HANDLE_MEMREC].base;

    /*-----------------------------------------------------------------------*/
    /* Call BAM Graph creation function with passing all the memories        */
    /*-----------------------------------------------------------------------*/
    algHandle->graphMem.graphObj      = memRec[GRAPH_OBJ_MEMREC].base;
    algHandle->graphMem.graphScratch  = memRec[GRAPH_SCRATCH_MEMREC].base;
    algHandle->graphMem.graphcontext  = memRec[GRAPH_CONTEXT_MEMREC].base;
    algHandle->graphMem.graphArgs     = memRec[GRAPH_ARGS_MEMREC].base;

    graphCreateParams.imgFrameWidth= createParams->imgFrameWidth;
    graphCreateParams.imgFrameHeight= createParams->imgFrameHeight;
    graphCreateParams.blockWidth= createParams->blockWidth;
    graphCreateParams.blockHeight= createParams->blockHeight;
    graphCreateParams.srcImBuf= createParams->singleBlockSrcAddr;
    graphCreateParams.dstImBuf= createParams->singleBlockDstAddr;

    status = BLOCK_SORT_U32_TI_graphCreate(&(algHandle->graphMem),&graphCreateParams);

    if (status != IALG_EOK)
    {
        PRINTF("BLOCK_SORT_U32_TI_graphCreate() returned error!!!");
        goto Exit;
    }

    algHandle->activeImgWidth= graphCreateParams.activeImgWidth;
    algHandle->activeImgHeight= graphCreateParams.activeImgHeight;

    algHandle->outputBlockWidth = graphCreateParams.blockWidth;
    algHandle->outputBlockHeight = graphCreateParams.blockHeight;
    algHandle->srcImBuf= graphCreateParams.srcImBuf;
    algHandle->dstImBuf= graphCreateParams.dstImBuf;

    algHandle->numMemRecs  = NUM_MEMRECS;
    memcpy(algHandle->memRec,memRec, sizeof(IALG_MemRec)*algHandle->numMemRecs);
    algHandle->ivision = (IVISION_Fxns *)&BLOCK_SORT_U32_TI_VISION_FXNS;

    /*-----------------------------------------------------------------------*/
    /* State to indicate that internal context of algorithm is not active    */
    /*-----------------------------------------------------------------------*/
    algHandle->algState = ALG_NOT_ACTIVE;

    Exit:
    return (status);
}

/*-----------------------------------------------------------------------*/
/* Function to accept any run time updates with recreation of object     */
/* Refer algControl function for details in ivision.h file               */
/* Not implemented for this function                                     */
/*-----------------------------------------------------------------------*/
int32_t BLOCK_SORT_U32_TI_control(IVISION_Handle Handle,
        IALG_Cmd cmd,
        const IALG_Params *inParams,
        IALG_Params *outParams)
{
    int32_t status = IALG_EOK;

    return (status);
}

/*--------------------------------------------------------------------------*/
/* This is the main processing function and should be called for each frame */
/* Refer process function for details in ivision.h file                     */
/*--------------------------------------------------------------------------*/
/* CHECK_MISRA("-16.7")  -> Disable rule 16.7 */
/* Pointer parameter is not used to modify the addressed object but is not declared as a pointer to const */
/* Deviation is allowed for this rule */
int32_t BLOCK_SORT_U32_TI_process(IVISION_Handle Handle,
        IVISION_InBufs *inBufs,
        IVISION_OutBufs *outBufs,
        IVISION_InArgs *inArgs,
        IVISION_OutArgs *outArgs)
{
    BLOCK_SORT_U32_TI_Handle algHandle = (BLOCK_SORT_U32_TI_Handle)(void*)(Handle);
    BLOCK_SORT_U32_TI_Handle intAlgHandle;
    int32_t status            = IALG_EOK;
    IVISION_BufDesc    *(*inBufDesc)[1];
    IVISION_BufDesc    *(*outBufDesc)[1];
    int32_t j;
    BAM_ProcessHints hints;

    IBAMALG_InArgs          bamInArgs;
    IBAMALG_InBufs          bamInBufs;

    IBAMALG_OutArgs         bamOutArgs;
    IBAMALG_OutBufs         bamOutBufs;

    BLOCK_SORT_U32_TI_outArgs * blockSortOutArgs;

    inBufDesc  = (IVISION_BufDesc *(*)[1])(void *)inBufs->bufDesc;
    outBufDesc = (IVISION_BufDesc *(*)[1])(void *)outBufs->bufDesc;

    /*-----------------------------------------------------------------------*/
    /* Activate the algorithm to make sure that now onwards internal memory  */
    /* handle can be utilized                                                */
    /*-----------------------------------------------------------------------*/
    BLOCK_SORT_U32_TI_activate((IALG_Handle)(void*)Handle);

    intAlgHandle                      = (BLOCK_SORT_U32_TI_Handle)(void*)algHandle->memRec[ALG_HANDLE_INT_MEMREC].base;
    intAlgHandle->graphMem.graphObj      = intAlgHandle->memRec[GRAPH_OBJ_INT_MEMREC].base;

    /*-----------------------------------------------------------------------*/
    /* BAM specific In Args and OutArgs Assignment                           */
    /*-----------------------------------------------------------------------*/
    bamInArgs.size                  = sizeof(IBAMALG_InArgs);
    bamInArgs.sliceIndex            = 0;

    bamOutArgs.size                 = sizeof(IBAMALG_OutArgs);

    BAM_activateGraph(intAlgHandle->graphMem.graphObj);

    /*-----------------------------------------------------------------------*/
    /* Loop to support N Frame in single call from user                      */
    /*-----------------------------------------------------------------------*/
    for(j = 0; j < inBufs->numBufs; j++)
    {
        status= BLOCK_SORT_U32_TI_dmaControl(&(intAlgHandle->graphMem), (*inBufDesc)[j], (*outBufDesc)[j]);
        
        if (status!= IALG_EOK) {
            goto Exit;
        }

        /* If output block width equal to buffer's width and output block height equal to buffer's height then we switch to one block processing, which is has lower overhead */ 
        if ((((*inBufDesc)[j]->bufPlanes[0].frameROI.width>>2)== intAlgHandle->outputBlockWidth) && ((*inBufDesc)[j]->bufPlanes[0].frameROI.height== intAlgHandle->outputBlockHeight)) {
            /* We input buffer and output buffer 's pointer is equal to stored pointers we know that the data is already in internal memory, no need for EDMA */  
            if (((*inBufDesc)[j]->bufPlanes[0].buf== intAlgHandle->srcImBuf) && ((*outBufDesc)[j]->bufPlanes[0].buf== intAlgHandle->dstImBuf)) {
                hints.priority= BAM_SINGLE_BLOCK_NO_EDMA;
            }
            else {
                hints.priority= BAM_SINGLE_BLOCK;
            }
        }
        else {
            hints.priority= BAM_COMPUTE_FIRST;
        }

        /*-----------------------------------------------------------------------*/
        /* Call execute function                                                 */
        /*-----------------------------------------------------------------------*/
        status =  BLOCK_SORT_U32_TI_execute(&(intAlgHandle->graphMem), &bamInBufs, &bamOutBufs, &bamInArgs, &bamOutArgs, &hints);

        /*-----------------------------------------------------------------------*/
        /* iVISION doesn't allow multiple control output, so last utput will be  */
        /* available to user                                                     */
        /*-----------------------------------------------------------------------*/
        if (outArgs->size == sizeof (BLOCK_SORT_U32_TI_outArgs) )
        {
            blockSortOutArgs = (BLOCK_SORT_U32_TI_outArgs *)(void*)outArgs;
            blockSortOutArgs->outputBlockWidth= intAlgHandle->outputBlockWidth;
            blockSortOutArgs->outputBlockHeight= intAlgHandle->outputBlockHeight;
            blockSortOutArgs->activeImgWidth= intAlgHandle->activeImgWidth;
            blockSortOutArgs->activeImgHeight= intAlgHandle->activeImgHeight;
        }
    }
Exit:
    /* Below BAM_deActivateGraph() is commented out as there is no need to save the context after each BAM_process() because this particular evelib_blockSort_u32() function
     * doesn't change any of its internal context. The context always stays in the same state that was found after BAM_createGraph()
     * */
    /*
    BAM_deActivateGraph(intAlgHandle->graphMem.graphObj);
    */
    return status;
}
/*RESET_MISRA("16.7")  -> Reset rule 16.7*/

/*--------------------------------------------------------------------------*/
/* This is the function to do activation of algorithm. All scratch memory   */
/* are restored in this                                                     */
/* Refer algActivate function for details in ialg.h file                    */
/*--------------------------------------------------------------------------*/
XDAS_Void  BLOCK_SORT_U32_TI_activate(IALG_Handle handle)
{
    BLOCK_SORT_U32_TI_Handle algHandle = (BLOCK_SORT_U32_TI_Handle)(void*)(handle);
    uint32_t i;

    /*----------------------------------------------------------------*/
    /* Do Activation only if it is deactivated or just after creation */
    /*----------------------------------------------------------------*/
    if(algHandle->algState == ALG_NOT_ACTIVE)
    {
        for(i= 1U; i < algHandle->numMemRecs; i++)
        {
            if((i == ALG_HANDLE_INT_MEMREC) ||(i == GRAPH_OBJ_INT_MEMREC))
            {
                EDMA_UTILS_memcpy2D(algHandle->memRec[i].base, (const void*)algHandle->memRec[i-1u].base, (uint16_t)algHandle->memRec[i].size, 1U, 0, 0);
            }

        }
        algHandle->algState = ALG_ACTIVE;
    }

    return;
}

/*--------------------------------------------------------------------------*/
/* This is the function to do de-activation of algorithm. All scratch memory*/
/* are stoed to external memory in this                                     */
/* Refer algDeActivate function for details in ialg.h file                  */
/*--------------------------------------------------------------------------*/
XDAS_Void  BLOCK_SORT_U32_TI_deactivate(IALG_Handle handle)
{
    BLOCK_SORT_U32_TI_Handle algHandle = (BLOCK_SORT_U32_TI_Handle)(void*)(handle);
    uint32_t i;
    for(i= 1U; i < algHandle->numMemRecs; i++)
    {
        if((i == ALG_HANDLE_INT_MEMREC) ||(i == GRAPH_OBJ_INT_MEMREC) )
        {
            /* Need to replace with EDMA */
            EDMA_UTILS_memcpy2D(algHandle->memRec[i-1u].base, (const void*)algHandle->memRec[i].base, (uint16_t)algHandle->memRec[i].size, 1U, 0, 0);
        }

    }
    algHandle->algState = ALG_NOT_ACTIVE;
    return;
}

/*--------------------------------------------------------------------------*/
/* This is the function which user can call to get back the memory given to */
/* algo                                                                     */
/* Refer algFree function for details in ialg.h file                        */
/*--------------------------------------------------------------------------*/
int32_t BLOCK_SORT_U32_TI_free(IALG_Handle handle, IALG_MemRec *memRec)
{
    int32_t status            = IALG_EOK;
    BLOCK_SORT_U32_TI_Handle algHandle = (BLOCK_SORT_U32_TI_Handle)(void*)(handle);

    memcpy(memRec, algHandle->memRec, sizeof(IALG_MemRec)*algHandle->numMemRecs);

    return status;
}

/*RESET_MISRA("15.1")  -> Reset rule 15.1 for GOTO */

