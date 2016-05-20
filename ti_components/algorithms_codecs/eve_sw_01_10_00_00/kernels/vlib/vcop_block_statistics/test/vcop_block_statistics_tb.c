/*==========================================================================*/
/*      Copyright (C) 2009-2013 Texas Instruments Incorporated.             */
/*                      All Rights Reserved                                 */
/* ======================================================================== */
/*                                                                          */
/*  NAME                                                                    */
/*      vcop_block_statistics                                               */
/*                                                                          */
/*  Driver file for testing Block Statistics Kernel                         */
/*                                                                          */
/*  USAGE                                                                   */
/*      This code contains a driver program for testing the 'vcop_block_    */
/*  statistics" function by injecting it with the same data set as in the   */
/*  natural C testbench functions.                                          */
/*                                                                          */
/*  This testbench then compares the results with the expected output       */
/*  array by doing a standard "memcmp" function and printing a pass/fail    */
/*  message. If it failed it prints a byte by byte trace.                   */
/*                                                                          */
/* ======================================================================== */

#include <stdio.h>
#include <stdlib.h>

#include "vcop.h"
#include "eve_profile.h"

/* ------------------------------------------------------------------------ */
/*  Include the "C" implementation of block statistics function.            */
/* ------------------------------------------------------------------------ */
#include "vcop_block_statistics_cn.h"

#if VCOP_HOST_EMULATION
    #include "vcop_block_statistics_host.h"
#else
    #include "vcop_block_statistics_kernel.h"
#endif

//#define __DEBUG

//#define __DEBUG_MIN
//#define __DEBUG_MAX
//#define __DEBUG_MEAN
//#define __DEBUG_VAR

#define NUM_ITER 1

#define STAT_BLOCK_HEIGHT (8)
#define STAT_BLOCK_WIDTH  (8)

#define BLK_HEIGHT (8*STAT_BLOCK_HEIGHT)
#define BLK_WIDTH  (8*STAT_BLOCK_WIDTH)
#define BLK_STRIDE (BLK_WIDTH)

#define OUT_WIDTH  (BLK_WIDTH/STAT_BLOCK_WIDTH)
#define OUT_HEIGHT (BLK_HEIGHT/STAT_BLOCK_HEIGHT)

#pragma DATA_SECTION(IMAGE, "Adata");
unsigned char IMAGE[BLK_HEIGHT*BLK_STRIDE] = {0x0};

#pragma DATA_SECTION(STAT_OUT, "Bdata");
unsigned char STAT_OUT[8*36] = {0x0};

#pragma DATA_SECTION(VAR_OUT, "Adata");
unsigned int VAR_OUT[OUT_HEIGHT*OUT_WIDTH+7] = {0x0};

#pragma DATA_SECTION(SCRATCH_C, "Cdata");
unsigned char SCRATCH_C[8*((BLK_WIDTH+7)/8)*36 + 8*36] = {0x0};

#pragma DATA_SECTION(SCRATCH_A, "Adata");
unsigned short SCRATCH_A[8*((BLK_WIDTH+7)/8)*18] = {0x0};

#pragma DATA_SECTION(SCRATCH_B, "Bdata");
unsigned int SCRATCH_B[8*((BLK_WIDTH+7)/8)*9] = {0x0};

#pragma DATA_SECTION(REF_MEAN_OUT, "Eoutdata");
unsigned short REF_MEAN_OUT[OUT_HEIGHT*OUT_WIDTH] = {0x0};

#pragma DATA_SECTION(REF_VAR_OUT, "Eoutdata");
unsigned int REF_VAR_OUT[OUT_HEIGHT*OUT_WIDTH] = {0x0};

#pragma DATA_SECTION(REF_MIN_OUT, "Eoutdata");
unsigned char REF_MIN_OUT[OUT_HEIGHT*OUT_WIDTH] = {0x0};

#pragma DATA_SECTION(REF_MAX_OUT, "Eoutdata");
unsigned char REF_MAX_OUT[OUT_HEIGHT*OUT_WIDTH] = {0x0};


static void CreateRandPattern8(unsigned char *p, unsigned short w, unsigned short h)
{
    int i;
    for (i = 0; i < w*h; i++)
    {
        p[i] = rand()&0xFF;
    }
}


int main()
{
    int i, j, iter, fail = 0, status = 0;
    unsigned char *pMin, *pMax;
    unsigned short *pMean;
    unsigned int *pVariance;
    unsigned int seed;

    pMin = (unsigned char *)STAT_OUT;
    pMax = (unsigned char *)(STAT_OUT + 8);
    pMean = (unsigned short *)(STAT_OUT + 16);
    pVariance = (unsigned int *)(VAR_OUT);

    profiler_init();

    printf("\nBlock Statistics Kernel block-level testing: %dx%d statistics for %dx%d block\n",
               STAT_BLOCK_HEIGHT, STAT_BLOCK_WIDTH, BLK_WIDTH, BLK_HEIGHT);

    VCOP_BUF_SWITCH_SET(WBUF_SYST, IBUFHB_SYST, IBUFLB_SYST,
                        IBUFHA_SYST, IBUFLA_SYST);

    vcop_block_statistics_init(IMAGE, BLK_STRIDE, BLK_WIDTH, BLK_HEIGHT, STAT_BLOCK_WIDTH,
                               STAT_BLOCK_HEIGHT, SCRATCH_C, SCRATCH_A, SCRATCH_B,
                               (unsigned int *)(SCRATCH_C + 8*((BLK_WIDTH+7)/8)*36),
                               (unsigned short*)(SCRATCH_C + 8*((BLK_WIDTH+7)/8)*36),
                               (unsigned short*)(SCRATCH_C + 8*((BLK_WIDTH+7)/8)*36 + 2),
                                pMin, pMax, pMean, pVariance,
                               __pblock_vcop_block_statistics);

    for (iter = 0; iter < NUM_ITER; iter++)
    {
        printf("Iteration %d:\n", iter);

        seed = _tsc_gettime();
        srand(seed);
        printf("Pattern Generation Seed = %d\n", seed);
        CreateRandPattern8(IMAGE, BLK_WIDTH, BLK_HEIGHT);

#ifdef __DEBUG
        printf("\nInput Image Block:\n");
        for(i = 0; i < BLK_HEIGHT; i++) {
            for(j = 0; j < BLK_WIDTH; j++) {
                printf("%3d ",IMAGE[i*BLK_WIDTH+j]);
            }
            printf("\n");
        }
        printf("\n");
#endif

        /*---------------------------------------------------------------------*/
        /*  Function call to the Natural-C API to be tested                    */
        /*---------------------------------------------------------------------*/
        vcop_block_statistics_cn(IMAGE, BLK_STRIDE, BLK_WIDTH, BLK_HEIGHT,
                                 STAT_BLOCK_WIDTH, STAT_BLOCK_HEIGHT, REF_MIN_OUT,
                                 REF_MAX_OUT, REF_MEAN_OUT, REF_VAR_OUT);

        profiler_start();

        VCOP_BUF_SWITCH_SET(WBUF_VCOP, IBUFHB_VCOP, IBUFLB_VCOP,
                        IBUFHA_VCOP, IBUFLA_VCOP);

        /*---------------------------------------------------------------------*/
        /*  Function call to the VCOP-C API to be tested                       */
        /*---------------------------------------------------------------------*/
        vcop_block_statistics_vloops(__pblock_vcop_block_statistics);
        _vcop_vloop_done();

        VCOP_BUF_SWITCH_SET(WBUF_SYST, IBUFHB_SYST, IBUFLB_SYST,
                        IBUFHA_SYST, IBUFLA_SYST);

        profiler_end();

        /*---------------------------------------------------------------------*/
        /*  Compare the results of output arrays with expected output arrays.  */
        /*---------------------------------------------------------------------*/
        fail = 0;
        for (i = 0; i < OUT_HEIGHT; i++)
        {
            for (j = 0; j < OUT_WIDTH; j++)
            {
                if (pMean[18*i+j] != REF_MEAN_OUT[OUT_WIDTH*i+j])
                {
#ifdef __DEBUG
                    printf("Error: pixel mean at     (%d, %d) Output = %d, Expected = %d\n",
                           i, j, pMean[18*i+j], (REF_MEAN_OUT[OUT_WIDTH*i+j]<<8));
#endif
                    fail++;
                }
                if (pVariance[OUT_WIDTH*i+j] != REF_VAR_OUT[OUT_WIDTH*i+j])
                {
#ifdef __DEBUG
                    printf("Error: pixel variance at (%d, %d) Output = %d, Expected = %d\n",
                           i, j, pVariance[OUT_WIDTH*i+j], REF_VAR_OUT[OUT_WIDTH*i+j]);
#endif
                    fail++;
                }
                if (pMin[36*i+j] != REF_MIN_OUT[OUT_WIDTH*i+j])
                {
#ifdef __DEBUG
                    printf("Error: pixel min at      (%d, %d) Output = %d, Expected = %d\n",
                           i, j, pMin[36*i+j], REF_MIN_OUT[OUT_WIDTH*i+j]);
#endif
                    fail++;
                }
                if (pMax[36*i+j] != REF_MAX_OUT[OUT_WIDTH*i+j])
                {
#ifdef __DEBUG
                    printf("Error: pixel max at      (%d, %d) Output = %d, Expected = %d\n",
                           i, j, pMax[36*i+j], REF_MAX_OUT[OUT_WIDTH*i+j]);
#endif
                    fail++;
                }
            }
        }

#ifdef __DEBUG
    #ifdef __DEBUG_MIN
        printf("\nScratch Row Minimum:\n");
        for(i = 0; i < BLK_WIDTH; i++) {
            for(j = 0; j < OUT_HEIGHT; j++) {
                printf("%3d ",SCRATCH_C[i*36+j]);
            }
            printf("\n");
        }
        printf("\n");

        printf("\nOutput Block Minimum:\n");
        for(i = 0; i < OUT_HEIGHT; i++) {
            for(j = 0; j < OUT_WIDTH; j++) {
                printf("%3d ",pMin[i*36+j]);
            }
            printf("\n");
        }
        printf("\n");

        printf("\nReference Output Block Minimum:\n");
        for(i = 0; i < OUT_HEIGHT; i++) {
            for(j = 0; j < OUT_WIDTH; j++) {
                printf("%3d ",REF_MIN_OUT[i*OUT_WIDTH+j]);
            }
            printf("\n");
        }
        printf("\n");
    #endif

    #ifdef __DEBUG_MAX
        printf("\nScratch Row Maximum:\n");
        for(i = 0; i < BLK_WIDTH; i++) {
            for(j = 0; j < OUT_HEIGHT; j++) {
                printf("%3d ",(SCRATCH_C+8)[i*36+j]);
            }
            printf("\n");
        }
        printf("\n");

        printf("\nOutput Block Maximum:\n");
        for(i = 0; i < OUT_HEIGHT; i++) {
            for(j = 0; j < OUT_WIDTH; j++) {
                printf("%3d ",pMax[i*36+j]);
            }
            printf("\n");
        }
        printf("\n");

        printf("\nReference Output Block Maximum:\n");
        for(i = 0; i < OUT_HEIGHT; i++) {
            for(j = 0; j < OUT_WIDTH; j++) {
                printf("%3d ",REF_MAX_OUT[i*OUT_WIDTH+j]);
            }
            printf("\n");
        }
        printf("\n");
    #endif

    #ifdef __DEBUG_MEAN
        printf("\nScratch Row Sum:\n");
        for(i = 0; i < BLK_WIDTH; i++) {
            for(j = 0; j < OUT_HEIGHT; j++) {

                printf("%5d ",((unsigned short*)(SCRATCH_C+16+i*36))[j]);
            }
            printf("\n");
        }
        printf("\n");

        printf("\nOutput Block Sum:\n");
        for(i = 0; i < OUT_HEIGHT; i++) {
            for(j = 0; j < OUT_WIDTH; j++) {
                printf("%5d ",pMean[i*18+j]);
            }
            printf("\n");
        }
        printf("\n");

        printf("\nReference Output Block Sum:\n");
        for(i = 0; i < OUT_HEIGHT; i++) {
            for(j = 0; j < OUT_WIDTH; j++) {
                printf("%5d ",REF_MEAN_OUT[i*OUT_WIDTH+j]);
            }
            printf("\n");
        }
        printf("\n");
    #endif

    #ifdef __DEBUG_VAR
        printf("\nScratch Row Sum of Squares:\n");
        for(i = 0; i < BLK_WIDTH; i++) {
            for(j = 0; j < OUT_HEIGHT; j++) {

                printf("%7d ",SCRATCH_B[9*i + j]);
            }
            printf("\n");
        }
        printf("\n");

        printf("\nScratch Sum of Squares:\n");
        for(i = 0; i < OUT_HEIGHT; i++) {
            for(j = 0; j < OUT_WIDTH; j++) {

                printf("%7d ",((unsigned int*)(SCRATCH_C + 8*((BLK_WIDTH+7)/8)*36))[i*9+j]);
            }
            printf("\n");
        }
        printf("\n");

        printf("\nOutput Block Sum:\n");
        for(i = 0; i < OUT_HEIGHT; i++) {
            for(j = 0; j < OUT_WIDTH; j++) {
                printf("%5d ",pMean[i*18+j]);
            }
            printf("\n");
        }
        printf("\n");

        printf("\nOutput Block Variance:\n");
        for(i = 0; i < OUT_HEIGHT; i++) {
            for(j = 0; j < OUT_WIDTH; j++) {
                printf("%7d ",pVariance[i*OUT_WIDTH+j]);
            }
            printf("\n");
        }
        printf("\n");

        printf("\nReference Output Block Variance:\n");
        for(i = 0; i < OUT_HEIGHT; i++) {
            for(j = 0; j < OUT_WIDTH; j++) {
                printf("%7d ",REF_VAR_OUT[i*OUT_WIDTH+j]);
            }
            printf("\n");
        }
        printf("\n");
    #endif
#endif

        printf(fail ? "FAIL\n":"PASS\n");
        status |= fail;
    }

    profiler_deinit();

    printf(status ? "\nBlock statistics: FAIL\n":"\nBlock statistics: PASS\n");
}
