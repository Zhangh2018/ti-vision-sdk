/*==========================================================================*/
/*      Copyright (C) 2009-2013 Texas Instruments Incorporated.             */
/*                      All Rights Reserved                                 */
/*==========================================================================*/

/** @file glcm_tb.c
*
*  @brief  This file contains test code for Gray-level co-occurrence matrix
*          applet
*
*  @date   Jan 2014
*
*  Description
*    This file contains test code to validate the frame level applet for
*    obtaining gray-level co-occurrence matrix. It checks the
*    output against c reference outputs and profiles the performance and
*    computes the system overheads.
*/

#pragma CHECK_MISRA ("none")
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vcop.h>

#include "evestarterware.h"
#include "alg_osal.h"
#include "glcm_config.h"
#include "xdais_types.h"
#include "iglcm_ti.h"
#include "glcm_ref_cn.h"

#include "ti_mem_manager.h"
#include "eve_profile.h"
#include "ti_file_io.h"

#define ENABLE_TRACES 0

#define PRINT_ERROR_MSG()  TEST_PRINTF("Error at line: %5d : in file %22s, of function : %s \n",__LINE__, __FILE__,__FUNCTION__)

#define DMEM_SIZE (16*1024)

#pragma DATA_SECTION (DMEM_SCRATCH, ".dmemSect");
/* This will be used for allocation of memory in DMEM*/
uint8_t DMEM_SCRATCH[DMEM_SIZE];

extern uint64_t gTest_TI_profileSctm[2], gTest_TI_profileTsc;

TIMemObject memObj_DMEM;
int32_t TestApp_AllocMemRecords(IALG_MemRec * memRec,int32_t numMemRec)
{
  int32_t i;
  TIMemHandle memHdl_DMEM = &memObj_DMEM;
  for (i = 0; i < numMemRec; i++)
  {
    if(memRec[i].space == IALG_DARAM0) {
      memRec[i].base = TI_GetMemoryChunk(memHdl_DMEM, memRec[i].size, memRec[i].alignment);
    }
    else {
      memRec[i].base = (Void *) malloc( memRec[i].size);
    }
    if(memRec[i].base == NULL)
    {
      return IALG_EFAIL;
    }
  }
  return IALG_EOK;
}

int32_t TestApp_FreeMemRecords(IALG_MemRec * memRec,int32_t numMemRec)
{
  int32_t i;
  for (i = 0; i < numMemRec; i++)
  {
    if(memRec[i].base == NULL)
    {
      return IALG_EFAIL;
    }
    if(memRec[i].space != IALG_DARAM0) {
        free(memRec[i].base);
    }
  }
  return IALG_EOK;
}

typedef struct IM_Fxns
{
  IVISION_Fxns * ivision;

} IM_Fxns;


char * moveToNextElement(char *in, char *LinePtr)
{
  LinePtr += strlen(in);
  while((LinePtr[0] == ' ')||(LinePtr[0] == '	')||(LinePtr[0] == ','))
  {
    LinePtr++;
  }
  return(LinePtr);
}


void fillDefaultConfig(sGlcm_Config * params)
{
  strcpy(params->inImage, "");

  params->srcImageWidth  = 400;
  params->srcImageHeight = 400;
  params->srcImagePitch  = 400;
  strcpy(params->outGlcm, "");

  params->loPixelVal  = 0;
  params->hiPixelVal  = 255;
  params->numLevels   = 10;

  params->numDirections  = 4;

  params->rowOffset[0] = 0;
  params->colOffset[0] = 1;

  params->rowOffset[1] = -1;
  params->colOffset[1] = 1;

  params->rowOffset[2] = -1;
  params->colOffset[2] = 0;

  params->rowOffset[3] = -1;
  params->colOffset[3] = -1;

  params->randSeed     = 0;

  params->performanceTestcase = 1;
  strcpy((char*)params->testCaseName, " ");
  strcpy((char*)params->testCaseDesc, " ");
}


int32_t glcmTest(sGlcm_Config * params)
{
  uint64_t profile_tsc, profile_sctm, profile_create_tsc;
  uint32_t seed;
  uint8_t *wbufToZero;
  int32_t numMemRec;
  int32_t i, j;
  IALG_MemRec *memRec;
  int32_t status;
  int32_t fail = 0;

  int8_t rowOffset[MAX_NUM_OFFSETS], colOffset[MAX_NUM_OFFSETS];
  uint8_t *input = NULL;
  uint16_t *outGlcm = NULL, *refOutGlcm = NULL;
  uint32_t sizeInput, sizeOutGlcm;

  uint32_t numBytes;

  char  filepath[256];
  TI_FILE *fid;
  char *statusStr = NULL;

  /*-----------------------------------------------------------------
  Pointer to algorithm instance
  -----------------------------------------------------------------*/
  IM_Fxns *     handle;

  /*-----------------------------------------------------------------
  Algorithm parameters memory in stack
  -----------------------------------------------------------------*/
  GLCM_TI_CreateParams   createParams;

  IVISION_InArgs    inArgs;
  IVISION_InBufs    inBufs;
  /*-----------------------------------------------------------------
  For test purpose it is allocated in stack. Application may get
  this descriptors from its buffer manager
  -----------------------------------------------------------------*/
  IVISION_BufDesc   inImgBufDesc;
  IVISION_BufDesc   *inBufDescList[1];

  IVISION_OutArgs   outArgs;
  IVISION_OutBufs   outBufs;
  /*-----------------------------------------------------------------
  For test purpose it is allocated in stack. Application may get
  this descriptors from its buffer manager
  -----------------------------------------------------------------*/
  IVISION_BufDesc   outGlcmBufDesc;
  IVISION_BufDesc   *outBufDescList[1];

  outArgs.size = sizeof(IVISION_OutArgs);
  /*--------------------------------------------------------------------------*/
  /*Create memory handle for ARP32 DMEM.                                      */
  /*--------------------------------------------------------------------------*/
  uint8_t *pDmem;
  TIMemHandle memHdl_DMEM;


  pDmem = (uint8_t *)&DMEM_SCRATCH;
  memHdl_DMEM = &memObj_DMEM;
  TI_CreateMemoryHandle(memHdl_DMEM, pDmem, DMEM_SIZE);

  /*-----------------------------------------------------------------
  Set buffer descriptor pointers array base
  -----------------------------------------------------------------*/
  inBufs.bufDesc  = inBufDescList;
  outBufs.bufDesc = outBufDescList;

  inBufs.numBufs  = 1;
  outBufs.numBufs = 1;

  inBufDescList[0]     = &inImgBufDesc;
  outBufDescList[0]    = &outGlcmBufDesc;

  /*-----------------------------------------------------------------
  Set algorithm parameters
  -----------------------------------------------------------------*/
  createParams.visionParams.algParams.size = sizeof(GLCM_TI_CreateParams);
  createParams.visionParams.cacheWriteBack = NULL;
  createParams.imageWidth             = params->srcImageWidth;
  createParams.imageHeight            = params->srcImageHeight;
  createParams.loPixelVal             = params->loPixelVal;
  createParams.hiPixelVal             = params->hiPixelVal;
  createParams.numLevels              = params->numLevels;
  createParams.numDirections          = params->numDirections;

  for(i = 0; i < params->numDirections; i++) {
    rowOffset[i] = params->rowOffset[i];
    colOffset[i] = params->colOffset[i];
  }
  createParams.rowOffset             = rowOffset;
  createParams.colOffset             = colOffset;

  /*-----------------------------------------------------------------
  Query algorithm memory requirement using standard IALG interface
  -----------------------------------------------------------------*/
#if ENABLE_TRACES
  TEST_PRINTF("Algorithm Init Start\n");
#endif
  profiler_init();
  profiler_initStack(profiler_getSP());
  profiler_start();

  numMemRec = GLCM_TI_VISION_FXNS.ialg.algNumAlloc();
  memRec    = (IALG_MemRec *)malloc(numMemRec*sizeof(IALG_MemRec));

  status = GLCM_TI_VISION_FXNS.ialg.algAlloc(
    (IALG_Params *)(&createParams), NULL, memRec);

  if(status != IALG_EOK)
  {
    PRINT_ERROR_MSG();
    goto EXIT_LOOP;
  }

  /*-----------------------------------------------------------------
  Allocate memory for all the mem records
  Application has to implement the below function
  Refer the example implementation below in this file
  -----------------------------------------------------------------*/
  status = TestApp_AllocMemRecords(memRec,numMemRec);
  if(status != IALG_EOK)
  {
    PRINT_ERROR_MSG();
    goto EXIT_LOOP;
  }

  /*-----------------------------------------------------------------
  Intialize the algorithm instance with the alocated memory
  and user create parameters
  -----------------------------------------------------------------*/
  status = GLCM_TI_VISION_FXNS.ialg.algInit((IALG_Handle)(&handle),
    memRec,NULL,(IALG_Params *)(&createParams));

  profiler_end_print(0);
  profile_create_tsc = gTest_TI_profileTsc;

  handle = (IM_Fxns *) memRec[0].base;

  if(status != IALG_EOK)
  {
    PRINT_ERROR_MSG();
    goto EXIT_LOOP;
  }

#if ENABLE_TRACES
  TEST_PRINTF("Gray-level Co-occurrence Applet Init Done\n");
#endif

  inArgs.subFrameInfo = 0;
  inArgs.size                  = sizeof(IVISION_InArgs);

  /* Image buffer for input gray-scale image */
  sizeInput = params->srcImagePitch * params->srcImageHeight;
  inImgBufDesc.numPlanes       = 1;
  inImgBufDesc.bufPlanes[0].frameROI.topLeft.x    = 0;
  inImgBufDesc.bufPlanes[0].frameROI.topLeft.y    = 0;
  inImgBufDesc.bufPlanes[0].width                 = params->srcImagePitch;
  inImgBufDesc.bufPlanes[0].height                = params->srcImageHeight;
  inImgBufDesc.bufPlanes[0].frameROI.width        = params->srcImageWidth;
  inImgBufDesc.bufPlanes[0].frameROI.height       = inImgBufDesc.bufPlanes[0].height;

  input = (uint8_t *)malloc(sizeInput);
  inImgBufDesc.bufPlanes[0].buf = (uint8_t *)input;

  if(inImgBufDesc.bufPlanes[0].buf == NULL)
  {
    PRINT_ERROR_MSG();
    goto EXIT_LOOP;
  }

  /* Buffer for output GLCM matrix */
  sizeOutGlcm = sizeof(uint16_t)*params->numDirections*params->numLevels*params->numLevels;
  outGlcmBufDesc.numPlanes                          = 1;
  outGlcmBufDesc.bufPlanes[0].frameROI.topLeft.x    = 0;
  outGlcmBufDesc.bufPlanes[0].frameROI.topLeft.y    = 0;
  outGlcmBufDesc.bufPlanes[0].width                 = sizeOutGlcm;
  outGlcmBufDesc.bufPlanes[0].height                = 1;
  outGlcmBufDesc.bufPlanes[0].frameROI.width        = outGlcmBufDesc.bufPlanes[0].width;
  outGlcmBufDesc.bufPlanes[0].frameROI.height       = outGlcmBufDesc.bufPlanes[0].height;

  outGlcm = (uint16_t *)malloc(sizeOutGlcm);
  outGlcmBufDesc.bufPlanes[0].buf = (uint8_t *)outGlcm;

  if(outGlcmBufDesc.bufPlanes[0].buf == NULL)
  {
    PRINT_ERROR_MSG();
    goto EXIT_LOOP;
  }

  if(strcmp(params->inImage, ""))
  {
    sprintf(filepath, "../testvecs/input/%s", params->inImage);
    fid = FOPEN(filepath,"rb");
    if(fid == NULL) {
      PRINT_ERROR_MSG();
      TEST_PRINTF("\nError, unable to open file %s \n", filepath);
      goto EXIT_LOOP;
    }
    input = (uint8_t *)inImgBufDesc.bufPlanes[0].buf;
    numBytes = FREAD(input, 1, sizeInput, fid);
    assert(numBytes== sizeInput);
    FCLOSE(fid);
  }
  else
  {

    seed = (params->randSeed == 0) ? _tsc_gettime() : params->randSeed;
    srand(seed);
    TEST_PRINTF("Pattern Generation Seed = %d\n", seed);

    /* Create Random input data in Image Buffer */
    for(i = 0 ; i < params->srcImageHeight; i++) {
      for(j = 0; j < params->srcImageWidth; j++) {
        input[i*params->srcImagePitch + j] =  params->loPixelVal + (rand() % (params->hiPixelVal - params->loPixelVal + 1));
      }
    }
  }

  memset(outGlcm, 0, sizeOutGlcm);

  vcop_minit();
  wbufToZero= (uint8_t*)vcop_malloc(VCOP_WMEM, 27*1024);
  memset(wbufToZero, 0xcd, 27*1024);
  vcop_free(wbufToZero);

#if ENABLE_TRACES
  TEST_PRINTF("Processing start \n");
#endif

  profiler_start();
  status = handle->ivision->algProcess((IVISION_Handle)handle,
    &inBufs,&outBufs,&inArgs,(IVISION_OutArgs *)&outArgs);
  profiler_end_print(1);

  profile_tsc = gTest_TI_profileTsc;
  profile_sctm = gTest_TI_profileSctm[0];

  if(status != IALG_EOK)
  {
    PRINT_ERROR_MSG();
    goto EXIT_LOOP;
  }
#if ENABLE_TRACES
  TEST_PRINTF("Processing End\n");
#endif

  if (strcmp(params->outGlcm, ""))
  {
    /* Write output GLCM */
    sprintf(filepath, "../testvecs/output/%s", params->outGlcm);
    fid= FOPEN(filepath,"wb");
    if ( fid == NULL)
    {
      PRINT_ERROR_MSG();
      TEST_PRINTF("\nError, unable to open file %s \n", filepath);
      goto EXIT_LOOP;
    }
    outGlcm = (uint16_t *)outGlcmBufDesc.bufPlanes[0].buf;
    numBytes = FWRITE(outGlcm, 1, sizeOutGlcm, fid);
    assert(numBytes == sizeOutGlcm);
    FCLOSE(fid);
  }

  refOutGlcm = (uint16_t *)malloc(sizeOutGlcm);
  if(refOutGlcm == NULL)
  {
    PRINT_ERROR_MSG();
    goto EXIT_LOOP;
  }

  glcm_ref_cn(input, refOutGlcm, params->srcImageWidth, params->srcImageHeight,
    params->srcImagePitch, params->loPixelVal, params->hiPixelVal, params->numLevels,
    params->numDirections, (int8_t *)rowOffset, (int8_t *)colOffset);

  fail = 0 ;
  fail = memcmp(refOutGlcm, outGlcm, sizeOutGlcm);

  if(fail) {
    for(i = 0; i < sizeOutGlcm/2; i++) {
      if(refOutGlcm[i] != outGlcm[i]) {
        TEST_PRINTF("GLCM (%d): Expected = %d, Output = %d\n",
          i, refOutGlcm[i], outGlcm[i]);
      }
    }
  }

  profiler_printStatus(fail);

  statusStr = fail ? "FAILED" : "PASSED";

  TEST_PRINTF("\n%-15s %10s %15s %10s %15s %20s %16s %17s %16s    %s\n", \
    "Algorithm", "Status", "numDirections", "numLevels", "MegaCycPerFr", \
    "MegaCycPerSec(30Fr)", "TotalCycPerPoint", "VcopBusyCycPerPix","TimePerFr(MSec)", "Comments");

  TEST_PRINTF("%-15s %10s %15d %10d %15.2f %20.2f %16.2f %17.2f %16.2f\
    Create graph is %2.2f MegaCycPerFr (one-time)\n",\
    "GLCM", statusStr, params->numDirections, params->numLevels,\
    (2*profile_tsc/1000000.0), 30*(2*profile_tsc/1000000.0),\
    (profile_tsc*2.0)/(params->srcImageWidth*params->srcImageHeight*params->numDirections),\
    (profile_sctm*2.0)/(params->srcImageWidth*params->srcImageHeight*params->numDirections),\
    (2*profile_tsc/500000.0), (profile_create_tsc*2.0)/1000000.0);

  fflush(stdout);

EXIT_LOOP:
  status = handle->ivision->ialg.algFree((IALG_Handle)(handle), memRec);
  if(status != IALG_EOK)
  {
    PRINT_ERROR_MSG();
  }
  /* Here Free memory for all the mem records */
  status = TestApp_FreeMemRecords(memRec,numMemRec);
  if(status != IALG_EOK)
  {
    PRINT_ERROR_MSG();
  }

  if ( memRec != NULL )
  {
    free(memRec);
  }
  if ( input != NULL )
  {
    free(input);
  }
  if (outGlcm != NULL)
  {
    free(outGlcm);
  }
  if (refOutGlcm != NULL)
  {
    free(refOutGlcm);
  }

  /* Get the stack usage */
#if !(VCOP_HOST_EMULATION)
  profiler_getStackDepth();
#endif

  return status;
}

int main(int argc, char *argv[])
{
  int32_t status = 0;
  TI_FILE *fp;
  int8_t *LinePtr;
  int32_t lineNum = -1;
  int32_t config_cmd;
  int8_t configFileName[FILE_NAME_SIZE];
  sGlcm_Config *params;

  report_printAppletInfo((int8_t *)"GLCM_TI_VISION");

  params = (sGlcm_Config *)(&gConfig_glcm);

  if (argc > 1)
  {
    strcpy((char *)configFileName, argv[1]) ;
  }
  else
  {
    strcpy((char *)configFileName, CONFIG_LIST_FILE_NAME);
  }

  fp = FOPEN((char *)configFileName , "r");
  if(fp== NULL)
  {
    TEST_PRINTF("Could not open config list file : %s , Runing in default mode \n",CONFIG_LIST_FILE_NAME);
    fillDefaultConfig(params);
    /* Validate the applet */
    status = glcmTest(params);

    return(status);
  }

  while(1)
  {
    memset(params->configLine, 0, CONFIG_LINE_SIZE);
    status = (int)FGETS(params->configLine, CONFIG_LINE_SIZE, fp);
    LinePtr = (int8_t *)params->configLine;
    if ((status == EOF) || (status == 0))
      break;
    status = 0;
    lineNum++;
    sscanf((const char *)LinePtr, "%d",&config_cmd);
    sscanf((const char *)LinePtr, "%s",params->configParam);
    if(config_cmd == 0)
    {
      TEST_PRINTF("End of config list found !\n");
      break;
    }
    else if(config_cmd == 2)
    {
      continue;
    }
    else if(config_cmd == 1)
    {
      LinePtr = (int8_t *)moveToNextElement(params->configParam,(char *)LinePtr);
      status  = sscanf((const char *)LinePtr, "%s",configFileName);
      TEST_PRINTF("Processing config file %s !\n", configFileName);
      fillDefaultConfig(params);
      status = readparamfile((char *)configFileName, gsTokenMap_glcm) ;
      if(status == -1)
      {
        TEST_PRINTF("Parser Failed\n");
        return -1 ;
      }

      profiler_testCaseInfo((int8_t *)params->testCaseName,
        (int8_t *)params->testCaseDesc, params->performanceTestcase);
      printparams(gsTokenMap_glcm, (int32_t)MAX_ITEMS_TO_PARSE,
        (uint32_t)params, sizeof(sGlcm_Config));

      /* Validate the applet */
      status = glcmTest(params);
      if(status == -1)
      {
        return status;
      }
    }
    else
    {
      TEST_PRINTF("Unsupported config list command parameter at line num : %4d !\n", lineNum);
    }
  }
  FCLOSE(fp);

#if VCOP_HOST_EMULATION
    TEST_PRINTF("Press any key to continue ...\n");
    scanf("%c", configFileName);
#endif

  return status;
}

