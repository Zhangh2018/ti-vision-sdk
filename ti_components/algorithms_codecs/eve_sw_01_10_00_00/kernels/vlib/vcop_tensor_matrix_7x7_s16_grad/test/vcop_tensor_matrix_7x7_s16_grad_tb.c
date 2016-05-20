/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  NAME                                                                    */
/*      vcop_tensor_matrix_7x7_s16_grad_tb.c                                */
/*                                                                          */
/*  Driver file for testing tensor matrix computation implemenation on EVE  */
/*                                                                          */
/*  USAGE                                                                   */
/*      This code contains a driver program for testing the 'vcop_tensor    */
/* _matrix_7x7_s16_grad_kernel function by injecting it with the same       */
/* data set as in the C testbench functions.                                */
/*                                                                          */
/*  This testbench then compares the results with the expected output       */
/*  array by doing a standard "memcmp" and printing a pass/fail             */
/*  message. If it failed it prints a byte by byte trace.                   */
/*                                                                          */
/*  To build the "C" executable type:                                       */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2013 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vcop.h>
#include "eve_profile.h"
#include "configparser.h"
#include "vcop_tensor_matrix_7x7_s16_grad_cn.h"

#define MAX_CONFIG_LINE_SIZE     (300)
#define MAX_FILE_NAME_SIZE       (200)
#define MAX_PARAM_STRING_SIZE    (30)
#define MAX_INPUT_LINE_SIZE      (300)
#define MAX_INPUT_VALUE_SIZE      (20)

#define CONFIG_LIST_FILE_NAME    ("../testvecs/config/config_list.txt")

typedef struct 
{
  char gradXName[MAX_FILE_NAME_SIZE];
  char gradYName[MAX_FILE_NAME_SIZE];
  char outFileName[MAX_FILE_NAME_SIZE];
  int  numKeyPoints;
  int  writeOutput;
  int seedForRND;
  char inputSrc[MAX_FILE_NAME_SIZE];
  char configLine[MAX_CONFIG_LINE_SIZE];
  char configParam[MAX_PARAM_STRING_SIZE];

} config_params;

config_params gParams ;


sTokenMapping gsTokenMap_sof[MAX_ITEMS_TO_PARSE] = 
{
  {(char* )"inputSrc",                  &(gParams.inputSrc)                   , STRING, SINGLE},                    
  {(char* )"gradXName",                 &(gParams.gradXName)                  , STRING, SINGLE},                    
  {(char* )"gradYName",                 &(gParams.gradYName)                  , STRING, SINGLE},                    
  {(char* )"outFileName",               &(gParams.outFileName)                , STRING, SINGLE},                    
  {(char* )"numKeyPoints",              &gParams.numKeyPoints                 , INT_32, SINGLE},                    
  {(char* )"writeOutput",               &gParams.writeOutput                  , INT_32, SINGLE},                    
  {(char* )"seedForRND",                &gParams.seedForRND                   , INT_32, SINGLE},                    
} ;

#define COMPARE_REFERENCE
#define NUM_ITER (1)


#define BLK_WIDTH  (7)
#define BLK_HEIGHT (7)
#define BLK_STRIDE (8)

#define MAX_NUM_POINTS (32)

#define SCATCH_SIZE ((MAX_NUM_POINTS/VCOP_SIMD_WIDTH)*8*9)


#pragma DATA_SECTION(gradX, "Adata");
static signed short gradX[BLK_STRIDE*BLK_HEIGHT*MAX_NUM_POINTS] = {0}; 
#pragma DATA_SECTION(gradY, "Bdata");
static signed short gradY[BLK_STRIDE*BLK_HEIGHT*MAX_NUM_POINTS] = {0}; 

#pragma DATA_SECTION(outIx2, "Adata");
static signed int outIx2[SCATCH_SIZE] = {0};
#pragma DATA_SECTION(outIy2, "Bdata");
static signed int outIy2[SCATCH_SIZE] = {0};
#pragma DATA_SECTION(outIxy, "Cdata");
static signed int outIxy[SCATCH_SIZE] = {0};

#pragma DATA_SECTION(refIx2, "EXTDMEM");
static signed int refIx2[MAX_NUM_POINTS] = {0};
#pragma DATA_SECTION(refIy2, "EXTDMEM");
static signed int refIy2[MAX_NUM_POINTS] = {0};
#pragma DATA_SECTION(refIxy, "EXTDMEM");
static signed int refIxy[MAX_NUM_POINTS] = {0};

#if VCOP_HOST_EMULATION
#include "../src_kernelC/vcop_tensor_matrix_7x7_s16_grad_kernel.k"
#else
#include "vcop_tensor_matrix_7x7_s16_grad_kernel.h"
#endif

char * moveToNextElement(char *in, char *LinePtr)
{
  LinePtr += strlen(in);
  while((LinePtr[0] == ' ')||(LinePtr[0] == '	'))
  {
    LinePtr++;
  }
  return(LinePtr);
}
void fillDefaultConfig(config_params * params)
{
  strcpy(params->inputSrc, "RND_PTRN");
  params->writeOutput  = 0;
  params->numKeyPoints = 8;
  params->seedForRND   = 0;
}
static int writeOutputToTxtFile(unsigned short n, int * ptr1, int * ptr2,int * ptr3, char * fileName)
{
  int i;
  FILE * fp;
  fp = fopen(fileName, "w+");
  if(fp== NULL)
  {
    printf("Error at line: %5d : in file %22s, of function : %s \n",__LINE__, __FILE__,__FUNCTION__);
    return (-1);
  }
  for(i = 0; i < n; i++) 
  {
      fprintf(fp,"%12d %12d %12d\n",ptr1[i],ptr2[i],ptr3[i]);
  }
  fclose(fp);
  return(0);
}

static int readInputFromTxtFile(signed short *p, unsigned short n, unsigned short w, unsigned short h, unsigned short stride,char * fileName)
{
  int i, j, k;
  FILE * fp;
  char * LinePtr;
  int status;
  int value;
  char inputLine[MAX_INPUT_LINE_SIZE];
  char inputValue[MAX_INPUT_VALUE_SIZE];

  fp = fopen(fileName, "r");
  if(fp== NULL)
  {
    printf("Error at line: %5d : in file %22s, of function : %s \n",__LINE__, __FILE__,__FUNCTION__);
    return (-1);
  }
  for(k = 0; k < n; k++) 
  {
    for(i = 0; i < h; i++) 
    {
      memset(inputLine, 0, MAX_INPUT_LINE_SIZE);
      status = (int)fgets(inputLine,MAX_CONFIG_LINE_SIZE,fp);
      if ((status == EOF) || (status == 0))
      {
        printf("Error at line: %5d : in file %22s, of function : %s \n",__LINE__, __FILE__,__FUNCTION__);
        return(-1);
      }
      LinePtr = inputLine;
      for(j = 0; j < w; j++) 
      {
        sscanf(LinePtr, "%d",&value);
        sscanf(LinePtr, "%s",inputValue);
        p[k*stride*h + i*stride+j] = value;
        LinePtr = moveToNextElement(inputValue,LinePtr);
      }
    }
  }
  fclose(fp);
  return(0);
}


static void CreateRandPatternS9(signed short *p, unsigned short n, unsigned short w, unsigned short h, unsigned short stride)
{
    int i, j, k;

#ifdef __DEBUG
    printf("Input Block:\n");
#endif
    for(k = 0; k < n; k++) 
    {
      for(i = 0; i < h; i++) 
      {
          for(j = 0; j < w; j++) 
          {
              p[k*stride*h + i*stride+j] = (rand() % 255) * (rand()&1 ? -1:1);
#ifdef __DEBUG
              printf("%4d, ",  p[k*stride*h + i*stride+j]);
#endif
        }
#ifdef __DEBUG
          printf("\n");
#endif
      }
#ifdef __DEBUG
        printf("\n");
#endif
    }
}


int test_bench(config_params * params)
{
    int fail,i;
    profiler_init();
    fail = 0;
    
        if(strcmp(params->inputSrc, "RND_PTRN")==0)
        {
          printf("Pattern Generation Seed = %d\n", params->seedForRND);
          srand(params->seedForRND);
          CreateRandPatternS9(gradX, params->numKeyPoints, BLK_WIDTH, BLK_HEIGHT, BLK_STRIDE);        
          CreateRandPatternS9(gradY, params->numKeyPoints, BLK_WIDTH, BLK_HEIGHT, BLK_STRIDE);        
        }
        else if(strcmp(params->inputSrc, "TXT_FILE")==0)
        {
           readInputFromTxtFile(gradX, params->numKeyPoints, BLK_WIDTH, BLK_HEIGHT, BLK_STRIDE,params->gradXName);        
           readInputFromTxtFile(gradY, params->numKeyPoints, BLK_WIDTH, BLK_HEIGHT, BLK_STRIDE,params->gradYName);        
        }
       
        profiler_start();
 
        vcop_tensor_matrix_7x7_s16_grad(gradX,gradY,BLK_STRIDE,params->numKeyPoints,outIx2,outIy2,outIxy,outIx2,outIy2,outIxy);
                           
        profiler_end();
    
#ifdef COMPARE_REFERENCE
        vcop_tensor_matrix_7x7_s16_grad_cn(gradX,gradY,BLK_STRIDE,params->numKeyPoints,refIx2,refIy2,refIxy);

        fail = memcmp(refIx2,outIx2,params->numKeyPoints*4);
        fail |= memcmp(refIy2,outIy2,params->numKeyPoints*4);
        fail |= memcmp(refIxy,outIxy,params->numKeyPoints*4);
        printf(fail ? "FAIL\n\n" : "PASS\n\n");

    if(fail != 0) 
    {
        for(i = 0; i < params->numKeyPoints; i++) 
        {
           printf("At %3d - Ix2 = %9d:%9d, Iy2 = %9d:%9d, Ixy = %9d:%9d\n",i,outIx2[i],refIx2[i],outIy2[i],refIy2[i],outIxy[i],refIxy[i]);
        }
    }

#endif

    profiler_deinit();

    if(params->writeOutput)
    {
      writeOutputToTxtFile(params->numKeyPoints, refIx2, refIy2, refIxy,params->outFileName);        
    }

    return (0);
}

int main()
{
  

  FILE * fp;
  char * LinePtr;
  int status;
  int lineNum = -1;
  config_params * params;
  int config_cmd;
  char configFileName[MAX_FILE_NAME_SIZE];

  params = (config_params *)(&gParams);

  fp = fopen(CONFIG_LIST_FILE_NAME, "r");
  if(fp== NULL)
  {
    printf("Could not open config list file : %s , Runing in defult mode \n",CONFIG_LIST_FILE_NAME);
    fillDefaultConfig(params);
    test_bench(params);
    return(0);
  }

  while(1)
  {
    memset(params->configLine, 0, MAX_CONFIG_LINE_SIZE);
    status = (int)fgets(params->configLine,MAX_CONFIG_LINE_SIZE,fp);
    LinePtr = params->configLine;
    if ((status == EOF) || (status == 0))
      break;
    lineNum++;
    sscanf(LinePtr, "%d",&config_cmd);
    sscanf(LinePtr, "%s",params->configParam);
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
      LinePtr = moveToNextElement(params->configParam,LinePtr);
      status  = sscanf(LinePtr, "%s",configFileName);
      printf("Processing config file %s !\n", configFileName);
      fillDefaultConfig(params);
      status = readparamfile(configFileName, &gsTokenMap_sof[0]) ;
      if(status == -1)
      {
        printf("Parser Failed");
        return -1 ;
      }     
      test_bench(params);
    }
    else
    {
      printf(" Unsupported config list command parameter at line num : %4d !\n", lineNum);
    }
  }
  fclose(fp);
  return (0);

}
