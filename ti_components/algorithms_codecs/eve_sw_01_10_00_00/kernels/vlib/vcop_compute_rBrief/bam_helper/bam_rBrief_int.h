/*==========================================================================*/
/*      Copyright (C) 2009-2013 Texas Instruments Incorporated.             */
/*                      All Rights Reserved                                 */
/*==========================================================================*/

/**
 *  @file       bam_rBrief_int.h
 *
 *  @brief      This header defines internal structures  for integrating rBrief kernel average 2x2 into BAM
 */

#ifndef BAM_RBRIEF_INT_H_
#define BAM_RBRIEF_INT_H_

#include <stdint.h>
#include <vcop.h>

#include "evealgframework.h"
#include "bam_rBrief.h"

#define RBRIEF_NUM_ZERO_ROWS            (5U)

#define RBRIEF_PING_PING_BUFFER_OFFSET (0x20000U)

#define MOMENTS_PATCH_SIZE (32U)
#define PATCH_SIZE (48U)
#define PATCH_SIZE_P2 (PATCH_SIZE + 2U) /* 50 */
#define PATCH_SIZE_P5 (PATCH_SIZE + 5U) /* 53 */

#define BYTE_OFFSET (PATCH_SIZE_P2 * 2U) /* 100 */

#define N2         (PATCH_SIZE_P5 * PATCH_SIZE)/* (53 * 48) */
#define N1         (PATCH_SIZE_P5 * PATCH_SIZE_P2) /* (53 * 50) */
#define N3         (PATCH_SIZE    * PATCH_SIZE) /* (48 * 48) */


#define NUM_IN_BLOCKS       (1U)
#define NUM_OUT_BLOCKS      (1U)
#define NUM_INTERNAL_BLOCKS (7U)

/* Scratch buffers to be places in WBUF */
typedef struct
{
  int16_t  moments_m01[16];
  int16_t cos_pt[32];
  int16_t row_col_sum[N3*2U];
  int8_t  rot_src_dst_y[512];
}RBRIEF_INTERNAL_MEMORY_WBUF;

/* Constant tables to be places in WBUF */
typedef struct
{
  int8_t    moments_col_mask[MOMENTS_PATCH_SIZE*MOMENTS_PATCH_SIZE];
  int8_t    moments_row_mask[MOMENTS_PATCH_SIZE*MOMENTS_PATCH_SIZE];
  uint16_t  tran_offset[16];
  int16_t   sinQ14_2[48];
}RBRIEF_INTERNAL_MEMORY_WBUF_tables;

/* Scratch buffers to be places in IBUFLA */
typedef struct
{
  int16_t   moments_row_sum[MOMENTS_PATCH_SIZE];
  int16_t   moments_m10[16];
  int16_t   sin_pt[32];
  int8_t    rot_src_dst_x[512];
  uint16_t  rot_src_lin[256];
  int16_t   tlu_dst[256];
}RBRIEF_INTERNAL_MEMORY_IBUFLA;

/* Constant tables to be places in IBUFLA */
typedef struct
{
  int16_t  cosQ14_2[48];
}RBRIEF_INTERNAL_MEMORY_IBUFLA_tables;

/* Scratch buffers to be places in IBUFHA */
typedef struct
{
  int16_t moments_col_sum[MOMENTS_PATCH_SIZE];
  uint8_t arctan_pack_decision[32];
  int16_t col_sum[2672];/* Aligning PATCH_SIZE_P5 * PATCH_SIZE_P2 to nearest 32 byte */
  int16_t tlu_src[256];
  uint16_t rot_dst_lin[256];
}RBRIEF_INTERNAL_MEMORY_IBUFHA;

/* Constant tables to be places in IBUFHA */
typedef struct
{
  int8_t zero_src_dst_x[512];
  int8_t zero_src_dst_y[512];
  uint16_t arctan_xthr[16];
}RBRIEF_INTERNAL_MEMORY_IBUFHA_tables;

#define RBRIEF_IN_IMG_IDX                   (BAM_RBRIEF_INPUT_IMG_PORT)

#define RBRIEF_OUT_BRIEF_DESCRIPTOR_IDX     (BAM_RBRIEF_OUTPUT_PORT)

#define RBRIEF_INTERNAL_PARAMS_IDX                    (0)
/* CONSTANTS and TLU's */
#define RBRIEF_INTERNAL_WBUF_IDX               (1U)
#define RBRIEF_INTERNAL_WBUF_TABLES_IDX        (2U)
#define RBRIEF_INTERNAL_IBUFLA_IDX             (3U)
#define RBRIEF_INTERNAL_IBUFLA_TABLES_IDX      (4U)
#define RBRIEF_INTERNAL_IBUFHA_IDX             (5U)
#define RBRIEF_INTERNAL_IBUFHA_TABLES_IDX      (6U)

extern const int8_t BAM_RBRIEF_TI_MOMENTS_COL_MASK[MOMENTS_PATCH_SIZE*MOMENTS_PATCH_SIZE];
extern const int8_t BAM_RBRIEF_TI_MOMENTS_ROW_MASK[MOMENTS_PATCH_SIZE*MOMENTS_PATCH_SIZE];
extern const int16_t  BAM_RBRIEF_TI_SINQ14_2[33] ;
extern const int16_t  BAM_RBRIEF_TI_COSQ14_2[33];
extern const uint16_t BAM_RBRIEF_TI_ARCTAN_XTHR[8];
extern const uint16_t  BAM_RBRIEF_TI_TRAN_OFFSET[8];

typedef struct bam_rBrief_context
{
    /* Must always follow this order: pInternalBlock[], pInBlock[], pOutputBlock[], args */
    void *pInternalBlock[NUM_INTERNAL_BLOCKS];
    void *pInBlock[NUM_IN_BLOCKS];
    void *pOutBlock[NUM_OUT_BLOCKS];
    BAM_rBrief_Args kernelArgs;
} BAM_rBrief_Context;

#endif /*#ifdef BAM_RBRIEF_INT_H_*/

