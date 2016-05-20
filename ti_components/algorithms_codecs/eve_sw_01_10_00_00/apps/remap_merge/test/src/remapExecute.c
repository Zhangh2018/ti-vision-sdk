/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2015 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */
#pragma CHECK_MISRA ("none")
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <vcop.h>

#include "eve_profile.h"
#include "ti_file_io.h"

#include "GlobalTypes.h"
#include "baseaddress/vayu/eve/baseaddress.h"
#include "pcache.h"
#include "sctm.h"

#include "evestarterware.h"
#include "alg_osal.h"

#include "xdais_types.h"

#include "iremap_merge_ti.h"

#include "convert_map.h"
#include "remap_common.h"
#include "bam_common.h"
#include "remap_test_print.h"
#include "remapExecute_config.h"
#include "remapExecute.h"

#if !(VCOP_HOST_EMULATION)
#define ENABLE_PROFILE 1
#endif

void remapNatc(uint8_t *srcY, uint8_t *srcUV, uint8_t *dstY, uint8_t *dstUV, RemapParms *params);

#define PRINT_ERRORE_MSG() TEST_PRINTF("Error at line: %5d : in file %22s, of function : %s \n",__LINE__, __FILE__,__FUNCTION__)

/* Defined in stereoVision_tb.c */
int32_t TestApp_AllocMemRecords(IALG_MemRec * memRec,int32_t numMemRec);
int32_t TestApp_FreeMemRecords(IALG_MemRec * memRec,int32_t numMemRec);

typedef struct IM_Fxns
{
    IVISION_Fxns * ivision;

} IM_Fxns;

extern uint64_t gTest_TI_profileSctm[2], gTest_TI_profileTsc;

int32_t remapExecute(sremapExecute_Config * gConfig_remapExecute, uint8_t *src, uint8_t *dst) {
    RemapParms     *params;
    uint32_t *coordMap;
    uint32_t lengthOfConvertedMap;
    uint8_t *convertedMap;
    char filepath[256];
    TI_FILE *fid;
    IM_Fxns          *handle;
    REMAP_MERGE_TI_CreateParams   createParams;
    IVISION_InArgs    inArgs;
    IVISION_InBufs    inBufs;
    IVISION_OutArgs   outArgs;
    IVISION_OutBufs   outBufs;
    /*-----------------------------------------------------------------
         For test purpose it is allocated in stack. Application may get
         this descriptors from its buffer manager
        -----------------------------------------------------------------*/
    IVISION_BufDesc   inputBlkBufDesc, lutBlkBufDesc;
    IVISION_BufDesc  *inBufDescList[4]; /* Can be set to 2 as we are not doing any merging */
    IVISION_BufDesc   outputBlkBufDesc;
    IVISION_BufDesc  *outBufDescList[1];

    uint32_t        srcStride, roiWidth, roiHeight, dstStride, srcHeight;

#if (ENABLE_PROFILE)
    uint32_t        setupTimerDiff, execTimerDiff, execVcopTimerDiff;
#endif

    /*--------------------------------------------------------------------------*/
    /*Create memory handle for ARP32 DMEM.                                      */
    /*--------------------------------------------------------------------------*/
    uint32_t         blockMap_LEN, numMemRec;
    IALG_MemRec    *memRec;
    uint8_t *srcBlkMap;

    int32_t status= IALG_EOK;

#if (ENABLE_PROFILE)
    profiler_init();
#endif

    memRec  = NULL;
    coordMap= NULL;
    srcBlkMap= NULL;
    convertedMap=  NULL;

    srcStride                           = gConfig_remapExecute->inputWidth;
    srcHeight                           = gConfig_remapExecute->inputHeight;
    dstStride                           = gConfig_remapExecute->remapWidth;
    roiWidth                            = gConfig_remapExecute->remapWidth;
    roiHeight                           = gConfig_remapExecute->remapHeight;

    params                          = &createParams.remapParams;

    params->maps.isSrcMapFloat          = 0;
    params->maps.srcFormat              = gConfig_remapExecute->colorFormat;
    params->maps.qShift                 = 2;
    params->interpolationLuma           = BILINEAR;
    params->interpolationChroma         = NN;
    params->rightShift                  = 0;
    params->sat_high                    = 255;
    params->sat_high_set                = 255;
    params->sat_low                     = 0;
    params->sat_low_set                 = 0;
    createParams.dstFormat              = gConfig_remapExecute->colorFormat;
    createParams.enableMerge            = 0;

    params->maps.srcImageDim.width      = srcStride;
    params->maps.srcImageDim.height     = srcHeight;
    params->maps.mapDim.width           = roiWidth;
    params->maps.mapDim.height          = roiHeight;

    params->maps.outputBlockDim.width   = 0; /* Does not matter, will be overridden */
    params->maps.outputBlockDim.height  = 0; /* Does not matter, will be overridden */

    sprintf(filepath, "%s", gConfig_remapExecute->convertedBinMapFile);
    if ((fid= FOPEN(filepath,"rb"))== NULL) {
        PRINT_ERRORE_MSG();
        goto EXIT;
    }

    FREAD((void*)&lengthOfConvertedMap, 4,  1, fid);

    if ( (convertedMap = (uint8_t*)malloc(lengthOfConvertedMap)) == NULL){
        status= IALG_EFAIL;
        goto EXIT;
    }

    FREAD((void*)convertedMap, 1,  lengthOfConvertedMap, fid); //////CCS FILE LOAD: Comment this line and set breakpoint. Data of size lengthOfConvertedMap bytes must be uploaded to address convertedMap.
    FCLOSE(fid);

    remapExecute_getBlockMap(&srcBlkMap, &blockMap_LEN, &params->maps, convertedMap);


#if (ENABLE_PROFILE)
        profiler_start();
#endif
    /*-----------------------------------------------------------------
          Query alogorithm memory requirement using standard IALG interface
      -----------------------------------------------------------------*/
    numMemRec   = REMAP_MERGE_TI_VISION_FXNS.ialg.algNumAlloc();
    memRec  = (IALG_MemRec *)malloc(numMemRec * sizeof(IALG_MemRec));

    status    = REMAP_MERGE_TI_VISION_FXNS.ialg.algAlloc((IALG_Params *)(&createParams), NULL, memRec);
    if(status != IALG_EOK)
    {
        PRINT_ERRORE_MSG();
        goto EXIT;
    }

    /*-----------------------------------------------------------------
          Allocate memory for all the mem records
          Application has to implement the below function
          Refer the example implementation below in this file
     -----------------------------------------------------------------*/
    status = TestApp_AllocMemRecords(memRec, numMemRec);
    if(status != IALG_EOK)
    {
        PRINT_ERRORE_MSG();
        goto EXIT;
    }

    /*-----------------------------------------------------------------
          Intialize the algorithm instance with the alocated memory
          and user create parameters
     -----------------------------------------------------------------*/
    status = REMAP_MERGE_TI_VISION_FXNS.ialg.algInit((IALG_Handle)(&handle),
            memRec, NULL, (IALG_Params *)(&createParams));


    handle = (IM_Fxns *)memRec[0].base;

    if(status != IALG_EOK)
    {
        printf("Remap initialization error: decrease blockWidth and blockHeight values for the use case processed by remapConvertTable\n");
        goto EXIT;
    }

#if (ENABLE_PROFILE)
        profiler_end_print(0);
        setupTimerDiff = gTest_TI_profileTsc;
#endif
    /*-----------------------------------------------------------------
           Set buffer descriptor pointers array base
      -----------------------------------------------------------------*/
    inArgs.subFrameInfo = 0;
    inArgs.size         = sizeof(IVISION_InArgs);
    outArgs.size        = sizeof(IVISION_OutArgs);

    inBufs.bufDesc      = inBufDescList;
    outBufs.bufDesc     = outBufDescList;

    inBufs.numBufs      = 2;
    outBufs.numBufs     = 1;

    inBufDescList[0]    = &inputBlkBufDesc;
    inBufDescList[1]    = &lutBlkBufDesc;
    outBufDescList[0]   = &outputBlkBufDesc;

    inputBlkBufDesc.numPlanes                          = 1;
    inputBlkBufDesc.bufPlanes[0].buf                   = (uint8_t *)src;
    inputBlkBufDesc.bufPlanes[0].width                 = srcStride;
    inputBlkBufDesc.bufPlanes[0].height                = srcHeight;
    inputBlkBufDesc.bufPlanes[0].frameROI.topLeft.x    = 0;
    inputBlkBufDesc.bufPlanes[0].frameROI.topLeft.y    = 0;
    inputBlkBufDesc.bufPlanes[0].frameROI.width        = srcStride;
    inputBlkBufDesc.bufPlanes[0].frameROI.height       = srcHeight;

    lutBlkBufDesc.numPlanes                          = 1;
    lutBlkBufDesc.bufPlanes[0].buf                   = (uint8_t *)srcBlkMap;
    lutBlkBufDesc.bufPlanes[0].width                 = blockMap_LEN;
    lutBlkBufDesc.bufPlanes[0].height                = 1;
    lutBlkBufDesc.bufPlanes[0].frameROI.topLeft.x    = 0;
    lutBlkBufDesc.bufPlanes[0].frameROI.topLeft.y    = 0;
    lutBlkBufDesc.bufPlanes[0].frameROI.width        = blockMap_LEN;
    lutBlkBufDesc.bufPlanes[0].frameROI.height       = 1;

    outputBlkBufDesc.numPlanes                          = 1;
    outputBlkBufDesc.bufPlanes[0].buf                   = (uint8_t *)dst;
    outputBlkBufDesc.bufPlanes[0].width                 = dstStride;
    outputBlkBufDesc.bufPlanes[0].height                = roiHeight;
    outputBlkBufDesc.bufPlanes[0].frameROI.topLeft.x    = 0;
    outputBlkBufDesc.bufPlanes[0].frameROI.topLeft.y    = 0;
    outputBlkBufDesc.bufPlanes[0].frameROI.width        = roiWidth;
    outputBlkBufDesc.bufPlanes[0].frameROI.height       = roiHeight;

#if 1 /*#if !(VCOP_HOST_EMULATION) */

#if (ENABLE_PROFILE)
        profiler_start();
#endif
    /*-----------------------------------------------------------------
           Initiate frame processing
     -----------------------------------------------------------------*/
    status = handle->ivision->algProcess((IVISION_Handle)handle, &inBufs, &outBufs, &inArgs, &outArgs);

    if(status != IALG_EOK)
    {
        PRINT_ERRORE_MSG();
        goto EXIT;
    }

#if (ENABLE_PROFILE)
        profiler_end_print(1);
        execTimerDiff     = gTest_TI_profileTsc;
        execVcopTimerDiff = gTest_TI_profileSctm[0];

        printf("\nGraph create: %d ARP32 cycles | Execution (control + VCOP + EDMA): %6.2f VCOP cycles/pixel | Execution (VCOP only): %6.2f VCOP cycles/pixel\n", setupTimerDiff/2, (float)execTimerDiff/(roiWidth*roiHeight), (float)execVcopTimerDiff/(roiWidth*roiHeight));

#endif


#else

    /*********  LUT loading  ************/
    coordMap= (uint32_t*)malloc(sizeof(uint32_t)*2*srcStride*srcHeight);
    sprintf(filepath, "%s", gConfig_remapExecute->originalMapFile);
    if ((fid= FOPEN(filepath,"rb"))== NULL) {
        PRINT_ERRORE_MSG();
        goto EXIT;
    }
    FREAD((void*)coordMap, 4,  2*srcStride*srcHeight, fid);
    FCLOSE(fid);

    params->maps.srcMap = (void*)coordMap;
    remapNatc(src, NULL, dst, NULL, params);

#endif

    EXIT:
    /*-----------------------------------------------------------------
                   Call algFree
       -----------------------------------------------------------------*/
    if (handle->ivision->ialg.algFree((IALG_Handle)(handle), memRec) != IALG_EOK)
    {
        PRINT_ERRORE_MSG();
    }

    /* Here Free memory for all the mem records */
    if (TestApp_FreeMemRecords(memRec, numMemRec) != IALG_EOK)
    {
        PRINT_ERRORE_MSG();
    }

    if ( memRec != NULL )
    {
        free(memRec);
    }

    if (coordMap != NULL) {
        free(coordMap);
    }

    if (convertedMap != NULL) {
        free(convertedMap);
    }

    return status;
}

void remapExecute_getBlockMap(uint8_t **srcBlkMap, uint32_t *blockMap_LEN, sConvertMap *maps, uint8_t *convertedMap) {

    uint32_t i;
    uint8_t *p;
    p= (uint8_t*)maps;

    for (i=0; i< sizeof(sConvertMap);i++)
        *p++= *convertedMap++;

    p= (uint8_t*)blockMap_LEN;
    for (i=0; i< sizeof(*blockMap_LEN);i++)
        *p++= *convertedMap++;

    *srcBlkMap= convertedMap;
}
