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
#include "utils_mem_priv.h"
#include <ti/ipc/SharedRegion.h>

IHeap_Handle Utils_memGetHeapHandleSR(UInt32 heapId);

#ifdef UTILS_MEM_HEAP_DDR_CACHED_SIZE
#pragma DATA_ALIGN(gUtils_memHeapDDR, UTILS_MEM_SR_HEAP_MIN_ALIGN)
#pragma DATA_SECTION(gUtils_memHeapDDR, ".bss:heapMemDDR")
UInt8 gUtils_memHeapDDR[UTILS_MEM_HEAP_DDR_CACHED_SIZE];
#endif

#ifdef UTILS_MEM_HEAP_OCMC_SIZE
#pragma DATA_ALIGN(gUtils_memHeapOCMC, UTILS_MEM_SR_HEAP_MIN_ALIGN)
#pragma DATA_SECTION(gUtils_memHeapOCMC, ".bss:heapMemOCMC")
UInt8 gUtils_memHeapOCMC[UTILS_MEM_HEAP_OCMC_SIZE];
#endif

Int32 Utils_memHeapSetup(void)
{
    #ifdef UTILS_MEM_HEAP_DDR_CACHED_SIZE
    {
        Utils_MemHeapObj *pObj;
        HeapMem_Params heapParams;

        pObj = &gUtils_memHeapObj[UTILS_HEAPID_DDR_CACHED_SR];

        HeapMem_Params_init(&heapParams);

        heapParams.buf = gUtils_memHeapDDR;
        heapParams.size = sizeof(gUtils_memHeapDDR);

        HeapMem_construct(&pObj->heapStruct, &heapParams);

        /* MISRA.CAST.PTR_TO_INT
         * MISRAC_2004_Rule_11.3
         * MISRAC_WAIVER:
         * Typecasting allocated buffer pointer to int as type of the
         * heapAddr is int.
         */
        pObj->heapAddr   = (UInt32)heapParams.buf;
        pObj->heapSize   = heapParams.size;
        pObj->heapHandle = HeapMem_Handle_upCast(
                            HeapMem_handle(&pObj->heapStruct)
                            );
    }
    #endif

    #ifdef UTILS_MEM_HEAP_OCMC_SIZE
    {
        Utils_MemHeapObj *pObj;
        HeapMem_Params heapParams;

        pObj = &gUtils_memHeapObj[UTILS_HEAPID_OCMC_SR];

        HeapMem_Params_init(&heapParams);

        heapParams.buf = gUtils_memHeapOCMC;
        heapParams.size = sizeof(gUtils_memHeapOCMC);

        HeapMem_construct(&pObj->heapStruct, &heapParams);

        /* MISRA.CAST.PTR_TO_INT
         * MISRAC_2004_Rule_11.3
         * MISRAC_WAIVER:
         * Typecasting allocated buffer pointer to int as type of the
         * heapAddr is int.
         */
        pObj->heapAddr   = (UInt32)heapParams.buf;
        pObj->heapSize   = heapParams.size;
        pObj->heapHandle = HeapMem_Handle_upCast(
                            HeapMem_handle(&pObj->heapStruct)
                            );
    }
    #endif

    #ifdef ENABLE_HEAP_SR0
    {
        Utils_MemHeapObj *pObj;
        SharedRegion_Entry srEntry;

        pObj = &gUtils_memHeapObj[UTILS_HEAPID_DDR_NON_CACHED_SR0];

        SharedRegion_entryInit(&srEntry);
        SharedRegion_getEntry(0, &srEntry);

        /* MISRA.CAST.PTR_TO_INT
         * MISRAC_2004_Rule_11.3
         * MISRAC_WAIVER:
         * Typecasting allocated buffer pointer to int as type of the
         * heapAddr is int.
         */
        pObj->heapAddr   = (UInt32)srEntry.base;
        pObj->heapSize   = srEntry.len;
        pObj->heapHandle = SharedRegion_getHeap(0);
    }
    #endif

    return SYSTEM_LINK_STATUS_SOK;
}

IHeap_Handle Utils_memGetHeapHandleSR(UInt32 heapId)
{
    IHeap_Handle heapHandle = NULL;

    if((UTILS_HEAPID_DDR_CACHED_SR == heapId) ||
       (UTILS_HEAPID_OCMC_SR == heapId) ||
       (UTILS_HEAPID_DDR_NON_CACHED_SR0 == heapId))
    {
        heapHandle = gUtils_memHeapObj[heapId].heapHandle;
    }

    return heapHandle;
}

Int32 Utils_memAllocSR(SystemCommon_AllocBuffer *pPrm)
{
    volatile Ptr addr = NULL;
    UInt32 align, size;
    Error_Block ebObj;
    Error_Block *eb = &ebObj;
    IHeap_Handle heapHandle = NULL;

    Error_init(eb);

    align = SystemUtils_align(pPrm->align, UTILS_MEM_SR_HEAP_MIN_ALIGN);
    size  = SystemUtils_align(pPrm->size, UTILS_MEM_SR_HEAP_MIN_ALIGN);

    heapHandle = Utils_memGetHeapHandleSR(pPrm->heapId);

    if(heapHandle)
    {
            /*
             * Heap is present in this CPU, allocate memory
             */
            addr = Memory_alloc(
                        heapHandle,
                        size,
                        align,
                        eb);
    }

    /* MISRA.CAST.PTR_TO_INT
     * MISRAC_2004_Rule_11.3
     * MISRAC_WAIVER:
     * Typecasing pointer to Uint as buffer pointer pointer expects
     * in uint format
     * This API is called from memAlloc API, in case, this is failed,
     * Utils_memAlloc, then returns a NULL address
     */
    pPrm->bufferPtr = (UInt32)addr;

    /* MISRA.CAST.PTR_TO_INT
     * MISRAC_2004_Rule_11.3
     * MISRAC_WAIVER:
     * Typecasting here just to make sure that previous write is
     * completed to memory
     */
    /* read back value to ensure previous write, if it is to shared memory gets
     * written completely to memory
     */
    addr = (Ptr)pPrm->bufferPtr;

    return SYSTEM_LINK_STATUS_SOK;
}

Int32 Utils_memFreeSR(SystemCommon_FreeBuffer *pPrm)
{
    UInt32 size;
    IHeap_Handle heapHandle = NULL;

    size  = SystemUtils_align(pPrm->size, UTILS_MEM_SR_HEAP_MIN_ALIGN);

    heapHandle = Utils_memGetHeapHandleSR(pPrm->heapId);

    if(heapHandle)
    {
        /*
         * Heap is present in this CPU, free memory
         */
        /* MISRA.CAST.PTR_TO_INT
         * MISRAC_2004_Rule_11.3
         * MISRAC_WAIVER:
         * Typecasting allocated int to poiner as memory_free expects
         * pointer as an argument.
         * Memory_free checks for the validity of the passed address in
         * the given heap
         */
        Memory_free(
                    heapHandle,
                    (Ptr)pPrm->bufferPtr,
                    size);
    }

    return SYSTEM_LINK_STATUS_SOK;
}
