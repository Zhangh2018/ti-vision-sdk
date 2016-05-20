/*==========================================================================*/
/*      Copyright (C) 2009-2013 Texas Instruments Incorporated.             */
/*                      All Rights Reserved                                 */
/* ======================================================================== */
/*                                                                          */
/*  NAME                                                                    */
/*      vcop_nonMaxSuppress_mxn_32s_tb                                      */
/*                                                                          */
/*  Driver file for testing 32 bit mxn non-maximum suppression kernel       */
/*                                                                          */
/*  USAGE                                                                   */
/*      This code contains a driver program for testing the 'vcop_          */
/*  nonMaxSuppress_mxn_32s" function by injecting it with the same data set */
/*  as in the  natural C testbench functions.                               */
/*                                                                          */
/*  This testbench then compares the results with the expected output       */
/*  array and prints a pass/fail message. If it failed it prints a byte by  */
/*  byte trace.                                                             */
/*                                                                          */
/* ======================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


#include "vcop.h"
#include "eve_profile.h"

#include "configparser.h"
#include "vcop_nonMaxSuppress_mxn_32s_config.h"



/* ------------------------------------------------------------------------ */
/*  Include prototype for the "C" implementation of non-maximum suppression */
/*  function.                                                               */
/* ------------------------------------------------------------------------ */
#include "vcop_nonMaxSuppress_mxn_32s_c.h"


#define MAX_CONFIG_LINE_SIZE     (300)
#define MAX_FILE_NAME_SIZE       (200)
#define MAX_PARAM_STRING_SIZE    (30)
#define CONFIG_LIST_FILE_NAME    ("../testvecs/config/config_list.txt")

#define WIN_HEIGHT (3)
#define WIN_WIDTH  (3)

#define BLOCK_HEIGHT (32 + WIN_HEIGHT - 1)
#define BLOCK_WIDTH  (32 + WIN_WIDTH - 1)

#define INPUT_STRIDE (BLOCK_WIDTH+3)

#if VCOP_HOST_EMULATION
  #include "../src_kernelC/vcop_nonMaxSuppress_mxn_32s_kernel.k" // kernel source
#else
  #include "vcop_nonMaxSuppress_mxn_32s_kernel.h"
#endif

#define ALIGN_SIMD(a)   (((a) + VCOP_SIMD_WIDTH-1) & ~(VCOP_SIMD_WIDTH-1))
#define ALIGN_2SIMD(a)    ((a + 2*VCOP_SIMD_WIDTH-1) & ~(2*VCOP_SIMD_WIDTH-1))

static void CreateRandPattern32S(int *p, unsigned short w, unsigned short h, unsigned short pitch)
{
    int i, j;
    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
          p[i + j * pitch] = (int)rand();
        }
    }
}


int32_t nmsMxn32sTest(sNms_mxn_32s_Config * testcaseParams)
{
  int32_t iter ,i ,j , status = 0;
  uint32_t seed;
  FILE *fp_in = NULL, *fp_out = NULL;
  uint32_t   size;

  int32_t * inputPtr = NULL;
  uint8_t  * outputPtr = NULL;
  int32_t * scratchBuf1 = NULL;
  int32_t * scratchBuf2 = NULL;
  uint8_t  * outputPtrRef = NULL;
  uint16_t   inBlockWidth;
  uint16_t   inBlockHeight;
  char  *statusStr = NULL;

  uint16_t   outBlockWidth;
  uint16_t   outBlockHeight;
  uint8_t   mask;
  uint16_t  numHorzElem;
  profiler_init();
  vcop_minit();

  inBlockWidth    = testcaseParams->width;
  inBlockHeight   = testcaseParams->height;

  outBlockWidth   = testcaseParams->width - (testcaseParams->windowWidth - 1);
  outBlockHeight  = testcaseParams->height - (testcaseParams->windowHeight - 1);

  #if (!VCOP_HOST_EMULATION)
  inputPtr      = (int32_t*)vcop_malloc(VCOP_IBUFLA , testcaseParams->inputStride * inBlockHeight * sizeof(int32_t) +
                                                      ALIGN_2SIMD(inBlockWidth)  * sizeof (int32_t));
  /* We are allocating more memory for output block then needed because we initialize
  outputPtr to zero in a loop which is running for more number of times */
  outputPtr     = (uint8_t*)vcop_malloc(VCOP_IBUFHA, ALIGN_2SIMD(inBlockWidth) * outBlockHeight );
  scratchBuf1   = (int32_t*)vcop_malloc(VCOP_WMEM, ALIGN_2SIMD(inBlockWidth) * inBlockHeight * sizeof(int32_t));
  scratchBuf2   = (int32_t*)vcop_malloc(VCOP_IBUFHA, ALIGN_SIMD(inBlockWidth) * inBlockHeight * sizeof(int32_t));
#else
  inputPtr      = (int32_t*)malloc(testcaseParams->inputStride * inBlockHeight * sizeof(int32_t) +
                                                      ALIGN_2SIMD(inBlockWidth)  * sizeof (int32_t));
  /* We are allocating more memory for output block then needed because we initialize
  outputPtr to zero in a loop which is running for more number of times */
  outputPtr     = (uint8_t*)malloc( ALIGN_2SIMD(inBlockWidth) * outBlockHeight);
  scratchBuf1   = (int32_t*)malloc( ALIGN_2SIMD(inBlockWidth) * inBlockHeight * sizeof(int32_t));
  scratchBuf2   = (int32_t*)malloc( ALIGN_SIMD(inBlockWidth) * inBlockHeight * sizeof(int32_t));

#endif

  outputPtrRef  = (uint8_t*)malloc( ALIGN_SIMD(inBlockWidth) * outBlockHeight);

  if(strcmp(testcaseParams->outputImage, ""))
  {
    fp_out = fopen((const char*)testcaseParams->outputImage, "wb+");
  }

  if(fp_out == NULL)
  {
    printf("Failed to open Output file.\n");
  }

  for (iter = 0; iter < testcaseParams->numIter; iter++)
  {

    if ( strcmp(testcaseParams->inputImage, "" ) )
    {
      printf("File Test\n");
      fp_in = fopen((const char *)testcaseParams->inputImage, "rb");

      if(fp_in == NULL)
      {
          printf("Failed to open input file: %s.\n", testcaseParams->inputImage);
          return -1;
      }

      size = testcaseParams->inputStride * inBlockHeight * sizeof(uint32_t);
      fread(inputPtr, size, 1, fp_in);

      fclose(fp_in);
    }
    else
    {
      seed = (testcaseParams->randSeed == 0) ? _tsc_gettime() : testcaseParams->randSeed;
      srand(seed);
      printf("Random pattern test: Seed = %d\n", seed);
      CreateRandPattern32S(inputPtr, inBlockWidth , inBlockHeight, testcaseParams->inputStride) ;

    }


#if !(VCOP_HOST_EMULATION)
    VCOP_BUF_SWITCH_SET(WBUF_SYST, IBUFHB_SYST, IBUFLB_SYST,
                        IBUFHA_SYST, IBUFLA_SYST);

    if ( testcaseParams->outputFormat == 0 )
    {

      vcop_nonMaxSuppress_mxn_32s_init(inputPtr,
                                    outputPtr,
                                    scratchBuf1,
                                    scratchBuf2,
                                    inBlockWidth,
                                    inBlockHeight,
                                    testcaseParams->inputStride,
                                    testcaseParams->outputStride,
                                    testcaseParams->windowWidth,
                                    testcaseParams->windowHeight,
                                    testcaseParams->nmsThreshold,
                                     __pblock_vcop_nonMaxSuppress_mxn_32s);
    }
    else
    {

      vcop_nonMaxSuppress_mxn_32s_bitPack_init(inputPtr,
                                    outputPtr,
                                    scratchBuf1,
                                    scratchBuf2,
                                    inBlockWidth,
                                    inBlockHeight,
                                    testcaseParams->inputStride,
                                    testcaseParams->outputStride,
                                    testcaseParams->windowWidth,
                                    testcaseParams->windowHeight,
                                    testcaseParams->nmsThreshold,
                                     __pblock_vcop_nonMaxSuppress_mxn_32s_bitPack);
    }
#endif


#if VCOP_HOST_EMULATION
    if ( testcaseParams->outputFormat == 0 )
    {

      vcop_nonMaxSuppress_mxn_32s(inputPtr,
                                  outputPtr,
                                  scratchBuf1,
                                  scratchBuf2,
                                  inBlockWidth,
                                  inBlockHeight,
                                  testcaseParams->inputStride,
                                  testcaseParams->outputStride,
                                  testcaseParams->windowWidth,
                                  testcaseParams->windowHeight,
                                  testcaseParams->nmsThreshold);
    }
    else
    {
      vcop_nonMaxSuppress_mxn_32s_bitPack(inputPtr,
                                        outputPtr,
                                        scratchBuf1,
                                        scratchBuf2,
                                        inBlockWidth,
                                        inBlockHeight,
                                        testcaseParams->inputStride,
                                        testcaseParams->outputStride,
                                        testcaseParams->windowWidth,
                                        testcaseParams->windowHeight,
                                        testcaseParams->nmsThreshold);

    }
#else
    VCOP_BUF_SWITCH_SET(WBUF_VCOP, IBUFHB_VCOP, IBUFLB_VCOP,
                    IBUFHA_VCOP, IBUFLA_VCOP);

    /*---------------------------------------------------------------------*/
    /*  Function call to the VCOP-C API to be tested                       */
    /*---------------------------------------------------------------------*/
    if ( testcaseParams->outputFormat == 0 )
    {

      profiler_start();
      vcop_nonMaxSuppress_mxn_32s_vloops(__pblock_vcop_nonMaxSuppress_mxn_32s);
      _vcop_vloop_done();
      profiler_end_print(1);
    }
    else
    {

      profiler_start();
      vcop_nonMaxSuppress_mxn_32s_bitPack_vloops(__pblock_vcop_nonMaxSuppress_mxn_32s_bitPack);
      _vcop_vloop_done();
      profiler_end_print(1);
    }

    VCOP_BUF_SWITCH_SET(WBUF_SYST, IBUFHB_SYST, IBUFLB_SYST,
                    IBUFHA_SYST, IBUFLA_SYST);

#endif


    /*---------------------------------------------------------------------*/
    /*  Function call to the Natural-C API to be tested                    */
    /*---------------------------------------------------------------------*/
    if ( testcaseParams->outputFormat == 0 )
    {

      vcop_nonMaxSuppress_mxn_32s_cn(inputPtr,
                                      outputPtrRef,
                                      inBlockWidth,
                                      inBlockHeight,
                                      testcaseParams->inputStride,
                                      testcaseParams->outputStride,
                                      testcaseParams->windowWidth,
                                      testcaseParams->windowHeight,
                                      testcaseParams->nmsThreshold);
    }
    else
    {
      vcop_nonMaxSuppress_mxn_32s_bitPack_cn(inputPtr,
                                      outputPtrRef,
                                      inBlockWidth,
                                      inBlockHeight,
                                      testcaseParams->inputStride,
                                      testcaseParams->outputStride,
                                      testcaseParams->windowWidth,
                                      testcaseParams->windowHeight,
                                      testcaseParams->nmsThreshold);
    }

    numHorzElem  = outBlockWidth;
    if ( testcaseParams->outputFormat == 1 )
    {
      numHorzElem  = ALIGN_SIMD(outBlockWidth) / 8;
    }

    for ( j = 0 ; j < outBlockHeight; j++)
    {
      for ( i = 0; i < numHorzElem; i++)
      {
        if ( testcaseParams->outputFormat == 1 )
        {
          if ( i == ((ALIGN_SIMD(outBlockWidth) / 8) - 1) )
          {
            mask = 0xFF >> (ALIGN_SIMD(outBlockWidth) - outBlockWidth);
          }
          else
          {
            mask = 0xFFU;
          }
        }
        else
        {
          mask = 0xFFU;
        }

        if ( (outputPtr[i + (j * testcaseParams->outputStride)] & mask) !=
          (outputPtrRef[i + (j * testcaseParams->outputStride)] & mask))
        {
          status = -1;
          printf ( "Output not Bit matching for iteration %d at (%d,%d) \n", iter,i,j);
          break;
        }
      }
      if ( status != 0 )
      {
        break;
      }
    }

    statusStr = status ? "FAILED":"PASSED";
    printf(" nmsMxn32sTest : Itereatio Number %d : %s \n",iter , statusStr);

  }

  if(fp_out != NULL)
  {
    fwrite(outputPtr , outBlockWidth * outBlockHeight,
                    1, fp_out);
    fclose(fp_out);
  }

#if (VCOP_HOST_EMULATION)
  if ( inputPtr != NULL )
  {
    free ( inputPtr);
  }

  if ( scratchBuf1 != NULL )
  {
    free ( scratchBuf1);
  }

  if ( scratchBuf2 != NULL )
  {
    free ( scratchBuf2);
  }

  if ( outputPtr != NULL )
  {
    free ( outputPtr);
  }

#else
  if ( inputPtr != NULL )
  {
    vcop_free ( inputPtr);
  }

  if ( scratchBuf1 != NULL )
  {
    vcop_free ( scratchBuf1);
  }

  if ( scratchBuf2 != NULL )
  {
    vcop_free( scratchBuf2);
  }

  if ( outputPtr != NULL )
  {
    vcop_free ( outputPtr);
  }

#endif
  if ( outputPtrRef != NULL )
  {
    free ( outputPtrRef);
  }

  return status;

}

char * moveToNextElement(char *in, char *LinePtr)
{
  LinePtr += strlen(in);
  while((LinePtr[0] == ' ')||(LinePtr[0] == '	')||(LinePtr[0] == ','))
  {
    LinePtr++;
  }
  return(LinePtr);
}

int main(int argc, char *argv[])
{
  int status = 0;
  char configFile[FILE_NAME_SIZE]={0};
  char configLine[MAX_CONFIG_LINE_SIZE];
  char configParam[MAX_PARAM_STRING_SIZE];
  int8_t         *LinePtr;
  FILE * fp;
  int32_t         lineNum = -1;
  int32_t         config_cmd;

  if (argc > 1)
  {
    strcpy(configFile, argv[1]) ;
  }
  else
  {
    strcpy(configFile,CONFIG_LIST_FILE_NAME);
  }

  fp = fopen((const char *)configFile , "r");
  if(fp== NULL)
  {
    printf("Could not open config list file : %s \n",CONFIG_LIST_FILE_NAME);
    return(status);
  }

  CF_profile_init(3, "vcop_nonMaxSuppress_mxn_32s");

  while ( 1 )
  {
    memset(configLine, 0, MAX_CONFIG_LINE_SIZE);
    status = (int)fgets(configLine,MAX_CONFIG_LINE_SIZE,fp);
    LinePtr = (int8_t *)configLine;
    if ((status == EOF) || (status == 0))
    {
      break;
    }
    status = 0;
    lineNum++;
    sscanf((const char *)LinePtr, "%d",&config_cmd);
    sscanf((const char *)LinePtr, "%s",configParam);

    if(config_cmd == 0)
    {
      printf("End of config list found !\n");
      break;
    }
    else if(config_cmd == 2)
    {
      continue;
    }
    else if(config_cmd == 1)
    {
      LinePtr = (int8_t *)moveToNextElement(configParam,(char *)LinePtr);
      status  = sscanf((const char *)LinePtr, "%s",configFile);
      printf("\n Processing config file %s !\n", configFile);
      status = readparamfile(configFile, &gsTokenMap_nms_mxn_32s[0]);
      if(status == -1)
      {
        printf("Parser Failed");
        return -1 ;
      }

      /* Validate the kernel */
      profiler_testCaseInfo((int8_t *)gConfig_nms_mxn_32s.testCaseName, (int8_t *)" ", 0);
      printparams(&gsTokenMap_nms_mxn_32s[0], (int32_t)MAX_ITEMS_TO_PARSE,
                  (uint32_t)(&gConfig_nms_mxn_32s),sizeof(sNms_mxn_32s_Config));

      /* Validate the kernel  */
      status = nmsMxn32sTest(&gConfig_nms_mxn_32s);

      if(status == -1)
      {
        return status;
      }
    }
    else
    {
      printf(" Unsupported config list command parameter at line num : %4d !\n", lineNum);
    }

  }

  fclose(fp);

  return status;
}

