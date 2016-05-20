/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  NAME                                                                    */
/*      eve_generateList_tb.c                                               */
/*                                                                          */
/*  Driver file to test generalist kernel                                   */
/*                                                                          */
/*  USAGE                                                                   */
/*      This code is a driver file for testing the 'vcop_generateList'      */
/*  function by injecting it with the same data set as in the               */
/*  assembly testbench functions.                                           */
/*                                                                          */
/*  This testbench then compares the results with the expected output       */
/*  array "EOut" by doing a standard "memcmp" and printing a pass/fail      */
/*  message. If it failed it prints a byte by byte trace.                   */
/*                                                                          */
/*  To build the "C" executable type:                                       */
/*                                                                          */
/*  gcc *.c -lm -o a.out                                                    */
/*==========================================================================*/
/*      Copyright (C) 2009-2012 Texas Instruments Incorporated.             */
/*                      All Rights Reserved                                 */
/*==========================================================================*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vcop.h>
#include <stdint.h>

#include "test_profile.h"

#define ALIGN_SIMD(a)   (((a) + VCOP_SIMD_WIDTH-1) & ~(VCOP_SIMD_WIDTH-1))

typedef unsigned char  Uint8;
typedef unsigned short Uint16;
typedef unsigned int   Uint32;

typedef char    Int8;
typedef short   Int16;
typedef int     Int32;

#define   IN_W        16
#define   IN_H        8

#define   BLK_W       12
#define   BLK_H       8

#define   N0           (IN_W  * IN_H )
#define   N1           (OUT_W * OUT_H )

#define NUM_HORZ_BLOCKS  24
#define NUM_VERT_BLOCKS  11

#define NOUT (4*7)
#define MAXNUMELMTS (NOUT+10)
#define INITIAL_NUMELEMENTS 2

#define QSHIFT 2

#pragma DATA_SECTION(in_data, "Adata");
Uint8 in_data[N0] =
{
    0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x03, 0x01, 0x03, 0x01,
    0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x03, 0x01, 0x03, 0x01,
    0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x03, 0x01, 0x03, 0x01,
    0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x03, 0x01, 0x03, 0x01,
    0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x03, 0x01, 0x03, 0x01,
    0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x03, 0x01, 0x03, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x01, 0x03, 0x01,
    0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x03, 0x01, 0x03, 0x01
};

#pragma DATA_SECTION(refX, "EOutdata");
#pragma DATA_SECTION(refY, "EOutdata");

Uint16 refX[NOUT] =
{
  1,   3,   5,   8,
  1,   3,   5,   8,
  1,   3,   5,   8,
  1,   3,   5,   8,
  1,   3,   5,   8,
  1,   3,   5,   8,
  1,   3,   5,   8
};

Uint16 refY[NOUT] =
{
  0,   0,   0,   0,
  1,   1,   1,   1,
  2,   2,   2,   2,
  3,   3,   3,   3,
  4,   4,   4,   4,
  5,   5,   5,   5,
  7,   7,   7,   7
};

#pragma DATA_SECTION(outX, "Bdata");
#pragma DATA_SECTION(outY, "Bdata");

uint16_t outX[BLK_W * BLK_H + 8] = {0};
uint16_t outY[BLK_W * BLK_H + 8] = {0};

#pragma DATA_SECTION(endRowPadVal, "Cdata");
#pragma DATA_SECTION(xVal,       "Cdata");
#pragma DATA_SECTION(yVal,       "Cdata");
#pragma DATA_SECTION(blockX,       "Cdata");
#pragma DATA_SECTION(blockY,       "Cdata");
#pragma DATA_SECTION(numElmtsX,       "Cdata");
#pragma DATA_SECTION(numElmtsY,       "Cdata");

uint8_t endRowPadVal[8];
uint16_t xVal[ALIGN_SIMD(BLK_W)];
uint16_t yVal[BLK_H];
uint16_t blockX, blockY, refBlockX, refBlockY;
uint16_t numElmtsX, numElmtsY;


#if VCOP_HOST_EMULATION
  #include "../src_kernelC/vcop_generateList_kernel.k"    // kernel source
#else
  #include "../inc/vcop_generateList_kernel.h"    // translated kernel
#endif

#include "../inc/vcop_generateList_c.h"
#include "../src_C/vcop_generateList_c.c"
#include "../src_C/get_generateListPtr.c"

int32_t get_generateListPtr(
        const uint16_t* pblock,
        uint16_t    paramCount,
        uint32_t    pListX,
        uint32_t    pListY,
        uint16_t    *listXoffset,
        uint16_t    *listYoffset
);

int main()
{
    int  i, j, x, y;
    uint16_t listXoffset, listYoffset;

    int ok;
#ifdef REPORT
FILE* fpReport = fopen("../../../report.csv","a+");
#ifdef SCTM
unsigned int sctm_t1, sctm_t2, exec_diff;
unsigned int overflow;
EVE_SCTM_Enable(CTM);
EVE_SCTM_CounterConfig(CTM, SCTM_Counter_0, VCOP_BUSY, SCTM_DURATION);
EVE_SCTM_CounterTimerEnable(CTM, SCTM_Counter_0);
#endif
#endif

for(x=0;x<ALIGN_SIMD(BLK_W);x++) {
    xVal[x]= x;
}

for(y=0;y<BLK_H;y++) {
    yVal[y]= y;
}

for(i=0;i<BLK_W - (ALIGN_SIMD(BLK_W) - ALIGN_SIMD(1));i++){
    endRowPadVal[i]=0xFF;
}

for(i=BLK_W - (ALIGN_SIMD(BLK_W) - ALIGN_SIMD(1));i<ALIGN_SIMD(1);i++){
    endRowPadVal[i]= 0;
}

blockX= 0;//NUM_HORZ_BLOCKS-1;
blockY= 0;//NUM_VERT_BLOCKS-1;

/* Need to adjust reference output */
for(i=0;i<NOUT;i++) {
    refX[i]+= blockX;
    refX[i]<<=QSHIFT;
    refY[i]+= blockY;
    refY[i]<<=QSHIFT;
}

#ifdef REPORT
#ifdef SCTM
sctm_t1 = EVE_SCTM_CounterRead (CTM, SCTM_Counter_0);
#endif
#endif

refBlockX= blockX+BLK_W;
refBlockY= blockY;

if (refBlockX== NUM_HORZ_BLOCKS*BLK_W) {
    refBlockX=0;
    refBlockY+=BLK_H;
    if (refBlockY== NUM_VERT_BLOCKS*BLK_H)
        refBlockY= 0;
}

numElmtsX= INITIAL_NUMELEMENTS;

vcop_generateList(
        in_data,
        endRowPadVal,
        xVal,
        yVal,
        &blockX, /* pointer to X coordinate of the current block processed */
        &blockY, /* pointer to Y coordinate of the current block processed */
        NUM_HORZ_BLOCKS,   /* number of horizontal blocks in the frame */
        NUM_VERT_BLOCKS,   /* number of vertical blocks in the frame */
        outX,
        outY,
        &numElmtsX,
        &numElmtsY,
        MAXNUMELMTS,
        BLK_W,
        BLK_H,
        IN_W,
        QSHIFT
);

get_generateListPtr(
        &__pblock_vcop_generateList[0],
        vcop_generateList_param_count(),
        (uint32_t) outX,
        (uint32_t) outY,
        &listXoffset,
        &listYoffset);

vcop_updateGenerateList(
        (uint32_t*)&__pblock_vcop_generateList[0],
        (uint32_t) outX,
        (uint32_t) outY,
        &numElmtsX,
        listXoffset,
        listYoffset);

#ifdef REPORT
#ifdef SCTM
sctm_t2 = EVE_SCTM_CounterRead (CTM, SCTM_Counter_0);
		overflow= EVE_SCTM_OverflowCheck(CTM,SCTM_Counter_0);
		assert(overflow!=1);

exec_diff = sctm_t2 - sctm_t1;
EVE_SCTM_CounterTimerDisable(CTM, SCTM_Counter_0);
#endif
#endif
#ifdef REPORT
fprintf(fpReport,"vcop_generateList,");
fprintf(fpReport,"%d,",IN_W);
fprintf(fpReport,"%d,",IN_H);
fprintf(fpReport,"%d,",BLK_W);
fprintf(fpReport,"%d,",BLK_H);
fprintf(fpReport,"1,");
#endif
    /*-------------------------------------------------------------------*/
    /* Print out the results here.                                       */
    /*-------------------------------------------------------------------*/

   ok = 1;

   for ( j = 0; j < NOUT; j++)
   {
           if (( outX[j] != refX[j] ) || ( outY[j] != refY[j] ))
           {
#ifdef CONSOLE
               printf("ERROR at index %d, outX = %d, refX = %d, outY = %d, refY = %d, *\n", j, outX[j], refX[j], outY[j], refY[j]);
#endif
               ok = 0;
           }
   }

   if (numElmtsY != INITIAL_NUMELEMENTS + NOUT) {
       printf("numElemnts= %d mismatches reference %d\n", numElmtsY, NOUT);
       ok= 0;
   }

   if (*((uint32_t*)&__pblock_vcop_generateList[0] + listXoffset) != (uint32_t)outX + sizeof(uint16_t)*numElmtsX) {
       printf("outX not updated correctly\n");
       ok= 0;
   }

   if (*((uint32_t*)&__pblock_vcop_generateList[0] + listYoffset) != (uint32_t)outY + sizeof(uint16_t)*numElmtsY) {
          printf("outY not updated correctly\n");
          ok= 0;
      }

   if (blockX != refBlockX) {
       printf("blockX= %d mismatches refBlockX= %d\n", blockX, refBlockX);
       ok= 0;
   }

   if (blockY != refBlockY) {
       printf("blockY= %d mismatches refBlockY= %d\n", blockY, refBlockY);
       ok= 0;
   }

#ifdef CONSOLE
   if (ok)
       printf(" \n\n ***** generate list-- Kernel-C -code: PASSED! *****\n");
   else
       printf(" \n\n ***** generate list-- Kernel-C -code: FAIL! ****\n");
#endif

   /* Execute natural C code this time */
   memset(outX, 0, sizeof(outX));
   memset(outY, 0, sizeof(outY));

   numElmtsX= INITIAL_NUMELEMENTS;

   blockX= 0;//NUM_HORZ_BLOCKS-1;
   blockY= 0;//NUM_VERT_BLOCKS-1;

vcop_generateList_cn
   (
           in_data,
           &blockX, /* pointer to X coordinate of the current block processed */
           &blockY, /* pointer to Y coordinate of the current block processed */
           NUM_HORZ_BLOCKS,   /* number of horizontal blocks in the frame */
           NUM_VERT_BLOCKS,   /* number of vertical blocks in the frame */
           outX,
           outY,
           &numElmtsX,
           &numElmtsY,
           MAXNUMELMTS,
           BLK_W,
           BLK_H,
           IN_W,
           QSHIFT
   );

/*-------------------------------------------------------------------*/
/* Print out the results here.                                       */
/*-------------------------------------------------------------------*/

ok = 1;

for ( j = 0; j < NOUT; j++)
{
       if (( outX[j] != refX[j] ) || ( outY[j] != refY[j] ))
       {
#ifdef CONSOLE
           printf("ERROR at index %d, outX = %d, refX = %d, outY = %d, refY = %d, *\n", j, outX[j], refX[j], outY[j], refY[j]);
#endif
           ok = 0;
       }
}

if (numElmtsY != INITIAL_NUMELEMENTS + NOUT) {
   printf("numElemnts= %d mismatches reference %d\n", numElmtsY, NOUT);
   ok= 0;
}

if (blockX != refBlockX) {
   printf("blockX= %d mismatches refBlockX= %d\n", blockX, refBlockX);
   ok= 0;
}

if (blockY != refBlockY) {
   printf("blockY= %d mismatches refBlockY= %d\n", blockY, refBlockY);
   ok= 0;
}

#ifdef CONSOLE
if (ok)
   printf(" \n\n ***** generate list-- Natural-C -code: PASSED! *****\n\n");
else
   printf(" \n\n ***** generate list-- Natural-C -code: FAIL! ****\n\n");
#endif

#ifdef REPORT
if(ok){
fprintf(fpReport,"PASS,");
}
else{
fprintf(fpReport,"FAIL,");
}
#ifdef SCTM
fprintf(fpReport,"%d\n",exec_diff);
#endif
if(fpReport){
fclose(fpReport);
}
#endif

return 0;
}



