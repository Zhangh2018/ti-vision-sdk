/******************************************************************************
 *                                                                            *
 * Copyright (c) 2012 Texas Instruments Incorporated - http://www.ti.com/     *
 *                                                                            *
 * All rights reserved. Property of Texas Instruments Incorporated.           *
 * Restricted rights to use, duplicate or disclose this code are              *
 * granted through contract.                                                  *
 *                                                                            *
 * The program may not be used without the written permission                 *
 * of Texas Instruments Incorporated or against the terms and conditions      *
 * stipulated in the agreement under which this program has been              *
 * supplied.                                                                  *
 *                                                                            *
 *****************************************************************************/
/**
 *  \file vpsdrv_m2mVpeFmd.h
 *
 *  \brief VPS VPE M2M driver internal header file.
 */
/*
 *
 *  REVISION:            $Revision: 1.2 $
 *
 *  VERSION HISTORY:
 *
 */

#ifndef VPSDRV_M2MVPEFMD_H_
#define VPSDRV_M2MVPEFMD_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  reg -- register data structure
 */
typedef struct {
    Int32  fmd_window_width;            /* FMD: fmd window width
                                         *                       */
    Int32  fmd_window_height;           /* FMD: fmd window height
                                         *                      */
    UInt16 mdt_num_field_mode;          /* MDT: number of fieleds used
                                         *                 */
    UInt16 fmd_diff_frame_thr;          /* FMD: frame difference threshold 1
                                         *           */
    UInt16 fmd_diff_frame_thr2;         /* FMD: frame difference threshold 2
                                         *           */
    UInt16 fmd_lock_cnt;                /* FMD: Lock count threshold
                                         *                   */
    UInt16 fmd_unlock_cnt;              /* FMD: Un-lock count threshold
                                         *                */
    UInt16 fmd_bed_enable;              /* FMD: Bad Edit Detection (BED)
                                         *               */
    UInt16 fmd_caf_thr;                 /* FMD: CAF threshold used for leaving
                                         *film mode*/
    UInt16 fmd_32_caf_thr;              /* FMD: CAF threshold used in 3-2 pull
                                         *down     */
    UInt16 fmd_22_caf_thr;              /* FMD: CAF threshold used in 2-2 pull
                                         *down     */
    UInt16 fmd_22_caf_thr2;             /* FMD: CAF threshold used in 2-2 pull
                                         *down     */
}fmd_reg_data;

/**
 *  fmd -- Film Mode Detection data structure
 */
typedef struct {
    UInt64 diff_field_1fd;      /* Motion sum delayed by 1 field         */
    UInt64 caf_1fd;             /* 1 field delay CAF                     */
    UInt64 diff_frame_1fd;      /* 1 field delay frame difference        */
    UInt64 diff_frame_array[4]; /* array of frame difference             */
    UInt16 motcmp22;            /* Motion compare 2:2 value              */
    UInt16 motcmp32;            /* Motion compare 3:2 value              */
    UInt16 lock_32;             /* 3:2 lock for the 3:2 state machine    */
    UInt16 seq_detect_32;       /* Sequence detect for 3:2 state machine */
    UInt16 jam_dir_32;          /* Jam direction for 3:2 state machine   */
    UInt16 lock_22;             /* 2:2 lock for 2:2 state machine        */
    UInt16 seq_detect_22;       /* Sequence detect for 2:2 state machine */
    UInt16 jam_dir_22;          /* Jam direction for 2:2 state machine   */
    UInt16 lock_cnt_32;         /* Lock count for 3:2 state machine      */
    UInt16 unlock_cnt_32;       /* Un-lock count for 3:2 state machine   */
    UInt16 lock_hsy_32;         /* Lock adjusted for hysteresis          */
    UInt16 lock_cnt_22;         /* Lock count for 2:2 state machine      */
    UInt16 unlock_cnt_22;       /* Un-lock count for 2:2 state machine   */
    UInt16 lock_hsy_22;         /* Lock adjusted for hysteresis          */
    UInt16 state22;             /* State of 2:2 state machine            */
    UInt16 lock_22_reset;       /* 3:2 state machine reset               */
    UInt16 lock_32_reset;       /* 2:2 state machine reset               */

    UInt16 cad32_state33_sh;    /* state 3:3 in cadence 3:2 pull down    */
    UInt16 cad32_state12_sh;    /* state 1:2 in cadence 3:2 pull down    */
    UInt16 cad32_state22_sh;    /* state 2:2 in cadence 3:2 pull down    */
    UInt16 cad32_state13_sh;    /* state 1:3 in cadence 3:2 pull down    */
    UInt16 cad32_state23_sh;    /* state 2:3 in cadence 3:2 pull down    */

    UInt16 fmd_diff_frame_thr;  /* FMD: frame difference threshold 1
                                 *           */
    UInt16 fmd_diff_frame_thr2; /* FMD: frame difference threshold 2
                                 *           */
    UInt16 fmd_caf_thr;         /* FMD: CAF threshold used for leaving film
                                 *mode*/
    UInt16 fmd_32_caf_thr;      /* FMD: CAF threshold used in 3-2 pull down
                                 *    */
    UInt16 fmd_22_caf_thr;      /* FMD: CAF threshold used in 2-2 pull down
                                 *    */
    UInt16 fmd_22_caf_thr2;     /* FMD: CAF threshold used in 2-2 pull down
                                 *    */
}fmd_data;

#ifdef __cplusplus
}
#endif

#endif  /* VPSDRV_M2MVPEFMD_H_ */
