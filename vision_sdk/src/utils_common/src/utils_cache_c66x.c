/*
 *******************************************************************************
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \file utils_idle_c66x.c
 *
 * \brief  APIs from this file are used to interface the CPU Idle functions
 *
 *         The APIs allow a user to enable CPU idle on a given processor.
 *
 * \version 0.0 (Dec 2014) : [CM] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <ti/sysbios/family/c66/Cache.h>
#include <include/link_api/system.h>
#include <src/utils_common/include/utils_mem_cfg.h>

#define DSP_SRAM_SIZE_MAX   (288U) /* in KB's */

Void Utils_dspCacheInit(void);




/**
 *******************************************************************************
 *
 * \brief This function divids DSPL2  into a cache and heap
 *
 *******************************************************************************
 */
Void Utils_dspCacheInit(void)
{
    Cache_Size cachePrm;
    char *L1_size[] = { "0 KB", "4 KB", "8 KB", "16 KB", "32 KB"};
    char *L2_size[] = { "0 KB", "32 KB", "64 KB", "128 KB", "256 KB", "512 KB", "1024 KB"};

    Cache_getSize(&cachePrm);

    Vps_printf(" SYSTEM: CACHE: L1P = %s, L1D = %s, L2 = %s ... after boot !!!\n",
        L1_size[cachePrm.l1pSize],
        L1_size[cachePrm.l1dSize],
        L2_size[cachePrm.l2Size]
        );

    #ifdef ENABLE_HEAP_L2
    {
        UInt32 heapSize = (UInt32)((UInt32)UTILS_MEM_HEAP_L2_SIZE/(UInt32)1024U);

        if(heapSize <= (DSP_SRAM_SIZE_MAX-(UInt32)256U))
        {
            cachePrm.l2Size = Cache_L2Size_256K;
        }
        else if(heapSize <= (DSP_SRAM_SIZE_MAX-(UInt32)128U))
        {
            cachePrm.l2Size = Cache_L2Size_128K;
        }
        else if(heapSize <= (DSP_SRAM_SIZE_MAX-(UInt32)64U))
        {
            cachePrm.l2Size = Cache_L2Size_64K;
        }
        else if(heapSize <= (DSP_SRAM_SIZE_MAX-(UInt32)32U))
        {
            cachePrm.l2Size = Cache_L2Size_32K;
        }
        else
        {
            cachePrm.l2Size = Cache_L2Size_0K;
        }

        Cache_setSize(&cachePrm);
    }
    #endif

    Cache_getSize(&cachePrm);

    Vps_printf(" SYSTEM: CACHE: L1P = %s, L1D = %s, L2 = %s ... after update by APP !!!\n",
        L1_size[cachePrm.l1pSize],
        L1_size[cachePrm.l1dSize],
        L2_size[cachePrm.l2Size]
        );
}


/* Nothing beyond this point */
