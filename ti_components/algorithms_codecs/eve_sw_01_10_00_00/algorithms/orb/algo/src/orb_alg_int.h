/*==========================================================================*/
/*      Copyright (C) 2009-2013 Texas Instruments Incorporated.             */
/*                      All Rights Reserved                                 */
/*==========================================================================*/

/**
 *  @file       orb_alg_int.h
 *
 *  @brief      This file defines the functions for ORB algorithm.
 *              All content of this file are internal to algorithm and not
 *              required by end user
 *  @version 0.0 (Dec 2013) : Base version.
 */


#ifndef ORB_ALG_INT_H
#define ORB_ALG_INT_H

/*--------------------------------------------------------------------------*/
/* This header file inclusion is only for XDAIS module. It accepts specific */
/* data types defined externally                                            */
/*--------------------------------------------------------------------------*/
#include <xdais_types.h>

#include "iorb_ti.h"

/**
 *  @enum   eMemrecs
 *  @brief  Memory records for ORB algorithm
*/
typedef enum
{
  /* Memory records for handle */
 /* Alg Handle should be first entry in this enum table
  as IALG mandates it */
  ALG_HANDLE_MEMREC,
  ALG_HANDLE_INT_MEMREC,
  XY_LIST_MEMREC,

  APPS_SCRATCH_INT,
  APPS_SCRATCH_EXT,
  APPS_PERSIST_INT,
  APPS_PERSIST_EXT,

  DUMMY_MEMREC_INFO,

  NUM_MEMRECS
} eMemrecs;

/**
 *  @enum   eAlgState
 *  @brief  State of ORB algorithm
*/
typedef enum
{
  ALG_NOT_ACTIVE,
  ALG_ACTIVE
} eAlgState;

/**
 *  @enum eORBBufDesc
 *  @brief  Different buf descriptor names - few are from user and few are internal to 
 *          ALG and supplied to apps
 *  @param IMG_ALG_BUFDESC - alg specific buf descriptor to hold all image planes
 *  @param XY_ALG_BUFDESC   - alg specific buf descriptor to hold all XY points - 
 *                            shall be used for FAST9 CD out, FAST9 BFFT in
 *                                and out
 *  @param ORBDESC_USER_BUFDESC - user provided buf descriptor to hold ORB 256 bit descriptor - 
 *                                should be used for RBRIEF out
 *  @param XY_USER_BUFDESC - user provided buf descriptor to hold key point position - 
 *                           should be used for Harris BFFT out
 *  @param LEVEL_USER_BUFDESC - user provided buf descriptor to hold level ID - 
 *                           should be used for Harris BFFT out
 *
 *   FAST9 CD ORDER NEED     (IN)  - IMG_ALG_BUFDESC
 *   FAST9 CD ORDER NEED     (OUT) - XY_ALG_BUFDESC
 *
 *   FAST9 BFFT ORDER NEED   (IN)  - IMG_ALG_BUFDESC, XY_ALG_BUFDESC
 *   FAST9 BFFT ORDER NEED   (OUT) - XY_ALG_BUFDESC
 *   
 *   HARRIS BFFT ORDER NEED (IN)   - IMG_ALG_BUFDESC, XY_ALG_BUFDESC
 *   HARRIS BFFT ORDER NEED (OUT)  - XY_USER_BUFDESC, LEVEL_USER_BUFDESC
 *
 *   RBRIEF ORDER NEED (IN) - IMG_ALG_BUFDESC1, XY_USER_BUFDESC, LEVEL_USER_BUFDESC,
 *   RBRIEF ORDER NEED (OUT) - ORBDESC_USER_BUFDESC
 *
 *   ORB (IN) : IMG_USER_BUFDESC
 *   ORB (OUT) : ORBDESC_USER_BUFDESC, XY_USER_BUFDESC, LEVEL_USER_BUFDESC
*/

typedef enum
{
  IMG_PYRAMID_ALG_BUFDESC =0,
  IMG_ALG_BUFDESC,
  XY_ALG_BUFDESC,
  ORB_ALG_BUFDESC_NUM,
  IMG_ALG_BUFDESC1  = ORB_ALG_BUFDESC_NUM,
  XY_USER_BUFDESC,
  LEVEL_USER_BUFDESC,
  ORB_TOTAL_BUFDESC_NUM
} eORBBufDesc ;

/**
 *  @enum   eORBALGBuffers
 *  @brief  Algorithm need to allocate buffers for intermediate data. These enumeration defines them
 *  IMG_BUF       : For all scaled down versions of the image buffers
*   XY_ALG_BUF : For XY List of corner points
*/

typedef enum
{
  IMG_BUF = 0,
  XY_ALG_BUF,
  ORB_MULTI_TOTAL
} eORBALGBuffers;


/**
  @struct ORB_TI_Obj
  @brief  This structure is the main handle of orb algorithm.

  @param  memRec     Array of memory records
  @param  ivision    All public function pointers
  @param  algState   State of algorithm to indicate
  @param  numMemRecs Number of memory records

  @param  numLevels  Number of levels for image scaling down (1 means no scale down)

  @param  fast9Threshold
          Threshold on difference between intensity of the central pixel and pixels
          of a circle around this pixel for FAST9 corner detect applet.

  @param  bestNFeaturesOut
          Max features to output by ORB across all levels
  @param  bestNFeaturesOutFast9BFTF
          best features given out by FAST9 BFTF for each level

  @param  fast9CornerDetectHandle   Handle of fast9 corner detect applet
  @param  fast9BFFTHandle           Handle of fast9 best feature to front applet
  @param  harrisBFFTHandle          Handle of harris best feature to front applet
  @param  rBriefHandle              Handle of rBRIEF - descriptor compute applet
  @param  imgPyramidHandle          Handle of Image Pyramid generation applet

  @param  pBufDescList              Array of pointers to hold different buf descriptor 
                                    required in ORB algorithm
  @param  bufDesc                   Buf descriptor array for algorithm internal buffers
  @param  bufPtrsAlg                Pointers to key buffers required by algorithm

*/
typedef struct
{
  IVISION_Fxns*     ivision;
  IALG_MemRec       memRec[NUM_MEMRECS];
  uint8_t           algState;
  uint32_t          numMemRecs;
  uint8_t           numLevels ;
  uint8_t           fast9Threshold;
  uint16_t          bestNFeaturesOut ;
  uint16_t          bestNFeaturesOutFast9BFTF[ORB_TI_MAXLEVELS];

  IVISION_Handle    fast9CornerDetectHandle ;
  IVISION_Handle    fast9BFFTHandle ;
  IVISION_Handle    harrisBFFTHandle ;
  IVISION_Handle    rBriefHandle ;
  IVISION_Handle    imgPyramidHandle;
                    
  IVISION_BufDesc*  pBufDescList[ORB_TOTAL_BUFDESC_NUM];
  IVISION_BufDesc   bufDesc[ORB_ALG_BUFDESC_NUM];
  uint8_t*          bufPtrsAlg[ORB_MULTI_TOTAL][ORB_TI_MAXLEVELS];  
} ORB_TI_Obj;

typedef ORB_TI_Obj * ORB_TI_Handle;


/*--------------------------------------------------------*/
/* IALG functions                                         */
/* Refer XDAIS ialg.h file for details on these functions */
/*--------------------------------------------------------*/
XDAS_Int32 ORB_TI_numAlloc(void);
XDAS_Int32 ORB_TI_alloc(const IALG_Params *params,
  struct IALG_Fxns **parentFxns, IALG_MemRec *memRec);
XDAS_Int32 ORB_TI_init(IALG_Handle handle,
  const IALG_MemRec *memRec, IALG_Handle parent, const IALG_Params *params);
XDAS_Void  ORB_TI_activate(IALG_Handle handle);
XDAS_Void  ORB_TI_deactivate(IALG_Handle handle);
XDAS_Int32 ORB_TI_free(IALG_Handle handle, IALG_MemRec *memRec);

XDAS_Int32 ORB_TI_control(IVISION_Handle handle, IALG_Cmd cmd,
    const IALG_Params *inParams, IALG_Params *outParams);

XDAS_Int32 ORB_TI_process(IVISION_Handle handle, IVISION_InBufs *inBufs,
    IVISION_OutBufs *outBufs, IVISION_InArgs *inArgs,
    IVISION_OutArgs *outArgs);

/*--------------------------------------------------------*/
/* IRES functions                                         */
/*--------------------------------------------------------*/
XDAS_Int32 ORB_TI_getResourceDescriptors(IALG_Handle handle, IRES_ResourceDescriptor *resDesc);

#endif /* ORB_ALG_INT_H */

