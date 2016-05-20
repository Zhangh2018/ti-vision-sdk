/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *  \ingroup SYSTEM_LINK_API
 *  \defgroup SYSTEM_LINK_ID  System Link ID's
 *
 *  The unique 32-bit Link ID for the links present in the system are defined
 *  in this module
 *
 *  @{
*/

/**
 *******************************************************************************
 *
 *  \file system_linkId.h
 *  \brief  System Link ID's
 *
 *******************************************************************************
*/

#ifndef SYSTEM_LINK_ID_H_
#define SYSTEM_LINK_ID_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Max possible Link ID
 *
 *******************************************************************************
 */
#define SYSTEM_LINK_ID_MAX                  (128U)

/**
 *******************************************************************************
 *
 * \brief Invalid Link ID
 *
 *******************************************************************************
 */
#define SYSTEM_LINK_ID_INVALID              (0xFFFFFFFFU)


/* @{ */

/**
 *******************************************************************************
 *
 * \brief IPU1 Core 0 Proc ID
 *
 *******************************************************************************
*/
#define SYSTEM_PROC_IPU1_0      (0U)

/**
 *******************************************************************************
 *
 * \brief IPU Core 1 Proc ID
 *
 *******************************************************************************
*/
#define SYSTEM_PROC_IPU1_1      (1U)

/**
 *******************************************************************************
 *
 * \brief MPU Proc ID
 *
 *******************************************************************************
*/
#define SYSTEM_PROC_A15_0       (2U)

/**
 *******************************************************************************
 *
 * \brief DSP Proc ID
 *
 *******************************************************************************
*/
#define SYSTEM_PROC_DSP1        (3U)
/**
 *******************************************************************************
 *
 * \brief DSP Proc ID
 *
 *******************************************************************************
*/
#define SYSTEM_PROC_DSP2        (4U)

/**
 *******************************************************************************
 *
 * \brief EVE Proc ID
 *
 *******************************************************************************
*/
#define SYSTEM_PROC_EVE1        (5U)

/**
 *******************************************************************************
 *
 * \brief EVE Proc ID
 *
 *******************************************************************************
*/
#define SYSTEM_PROC_EVE2        (6U)

/**
 *******************************************************************************
 *
 * \brief EVE Proc ID
 *
 *******************************************************************************
*/
#define SYSTEM_PROC_EVE3        (7U)

/**
 *******************************************************************************
 *
 * \brief EVE Proc ID
 *
 *******************************************************************************
*/
#define SYSTEM_PROC_EVE4        (8U)

/**
 *******************************************************************************
 *
 * \brief Max supported processors
 *
 *******************************************************************************
*/
#define SYSTEM_PROC_MAX         (9U)

/**
 *******************************************************************************
 *
 * \brief Invalid proc Id, if received indicates some corruption
 *
 *******************************************************************************
*/
#define SYSTEM_PROC_INVALID     (0xFFFFU)

/* @} */

/**
 *******************************************************************************
 *
 * \brief Create link id which indicates the link & processor in which it
 *        resides
 *
 *******************************************************************************
*/
#define SYSTEM_MAKE_LINK_ID(p, x) ((((p) & 0xFU) << 8U) | ((x) & 0x000000FFU))

/**
 *******************************************************************************
 *
 * \brief Get the link id - strip off proc id
 *
 *******************************************************************************
*/
#define SYSTEM_GET_LINK_ID(x)     ((x) & 0x000000FFU)

/**
 *******************************************************************************
 *
 * \brief Get the proc id - strip off link id
 *
 *******************************************************************************
*/
#define SYSTEM_GET_PROC_ID(x)     (((x) & ~0x7FFFF0FFU)>> 8U)

/**
 *******************************************************************************
 *
 * \brief Set route bit - bit 32 of LinkId is used as route bit
 *        This is used only when message needs to be routed through
 *        some other core.
 *
 *******************************************************************************
*/
#define SYSTEM_LINK_ID_SET_ROUTE_BIT(x)    ((x) |= (1U << 0x1F))

/**
 *******************************************************************************
 *
 * \brief Clear route bit
 *
 *******************************************************************************
*/
#define SYSTEM_LINK_ID_CLEAR_ROUTE_BIT(x)  ((x) &= ~((1U) << 0x1F))

/**
 *******************************************************************************
 *
 * \brief Test route bit
 *
 *******************************************************************************
*/
#define SYSTEM_LINK_ID_TEST_ROUTE_BIT_TRUE(x)  ((x) & (1U << 0x1F))


/**
 *******************************************************************************
 *
 * \brief Type of payload used with notify payload: Payload is Link ID
 *
 *******************************************************************************
*/
#define SYSTEM_LINK_ID_NOTIFY_TYPE_LINK_ID         (0U)

/**
 *******************************************************************************
 *
 * \brief Type of payload used with notify payload: Payload is a message
 *
 *******************************************************************************
*/
#define SYSTEM_LINK_ID_NOTIFY_TYPE_MSG             (1U)

/**
 *******************************************************************************
 *
 * \brief Type of payload used with notify payload: Payload is a message ACK
 *
 *******************************************************************************
*/
#define SYSTEM_LINK_ID_NOTIFY_TYPE_MSG_ACK         (2U)

/**
 *******************************************************************************
 *
 * \brief Set route bit - bit 32 of LinkId is used as route bit
 *        This is used only when message needs to be routed through
 *        some other core.
 *
 *******************************************************************************
*/
#define SYSTEM_LINK_ID_MAKE_NOTIFY_TYPE(dest_p, src_p, t)       \
    ( SYSTEM_MAKE_LINK_ID((dest_p), (src_p)) | ( ( (t) & 0xFU ) << 12U) )

/**
 *******************************************************************************
 *
 * \brief Test route bit
 *
 *******************************************************************************
*/
#define SYSTEM_LINK_ID_GET_NOTIFY_TYPE(x)    (((x) & (0x0000F000U)) >> 12U)


/* @{ */

#define IPU1_0_LINK(x)          SYSTEM_MAKE_LINK_ID(SYSTEM_PROC_IPU1_0 , (x))

/**
 *******************************************************************************
 *
 * \brief IPU1_1 System Link - used for non-link specific proc level
 *        communication
 *
 *******************************************************************************
*/
#define IPU1_1_LINK(x)          SYSTEM_MAKE_LINK_ID(SYSTEM_PROC_IPU1_1 , (x))

/**
 *******************************************************************************
 *
 * \brief DSP1 System Link - used for non-link specific proc level communication
 *
 *******************************************************************************
*/
#define DSP1_LINK(x)            SYSTEM_MAKE_LINK_ID(SYSTEM_PROC_DSP1   , (x))

/**
 *******************************************************************************
 *
 * \brief DSP2 System Link - used for non-link specific proc level communication
 *
 *******************************************************************************
*/
#define DSP2_LINK(x)            SYSTEM_MAKE_LINK_ID(SYSTEM_PROC_DSP2   , (x))

/**
 *******************************************************************************
 *
 * \brief EVE1 System Link - used for non-link specific proc level communication
 *
 *******************************************************************************
*/
#define EVE1_LINK(x)            SYSTEM_MAKE_LINK_ID(SYSTEM_PROC_EVE1   , (x))

/**
 *******************************************************************************
 *
 * \brief EVE2 System Link - used for non-link specific proc level communication
 *
 *******************************************************************************
*/
#define EVE2_LINK(x)            SYSTEM_MAKE_LINK_ID(SYSTEM_PROC_EVE2   , (x))

/**
 *******************************************************************************
 *
 * \brief EVE3 System Link - used for non-link specific proc level communication
 *
 *******************************************************************************
*/
#define EVE3_LINK(x)            SYSTEM_MAKE_LINK_ID(SYSTEM_PROC_EVE3   , (x))

/**
 *******************************************************************************
 *
 * \brief EVE4 System Link - used for non-link specific proc level communication
 *
 *******************************************************************************
*/
#define EVE4_LINK(x)            SYSTEM_MAKE_LINK_ID(SYSTEM_PROC_EVE4   , (x))

/**
 *******************************************************************************
 *
 * \brief A15 Link - used for non-link specific proc level communication
 *
 *******************************************************************************
*/
#define A15_0_LINK(x)            SYSTEM_MAKE_LINK_ID(SYSTEM_PROC_A15_0 ,(x))


/* @} */

/* @{ */

/**
 *******************************************************************************
 *
 * \brief Link ID for the generic processor link task
 *
 *******************************************************************************
 */
#define SYSTEM_LINK_ID_PROCK_LINK_ID        (SYSTEM_LINK_ID_MAX-1U)

/**
 *******************************************************************************
 *
 * \brief IPU1_0 System Link - used for non-link specific proc level
 *        communication
 *
 *******************************************************************************
 */
#define SYSTEM_LINK_ID_IPU1_0       IPU1_0_LINK(SYSTEM_LINK_ID_PROCK_LINK_ID)

/**
 *******************************************************************************
 *
 * \brief IPU1_1 System Link - used for non-link specific proc level
 *        communication
 *
 *******************************************************************************
 */
#define SYSTEM_LINK_ID_IPU1_1       IPU1_1_LINK(SYSTEM_LINK_ID_PROCK_LINK_ID)

/**
 *******************************************************************************
 *
 * \brief DSP1 System Link - used for non-link specific proc level
 *        communication
 *
 *******************************************************************************
 */
#define SYSTEM_LINK_ID_DSP1         DSP1_LINK(SYSTEM_LINK_ID_PROCK_LINK_ID)

/**
 *******************************************************************************
 *
 * \brief DSP2 System Link - used for non-link specific proc level
 *        communication
 *
 *******************************************************************************
 */
#define SYSTEM_LINK_ID_DSP2         DSP2_LINK(SYSTEM_LINK_ID_PROCK_LINK_ID)

/**
 *******************************************************************************
 *
 * \brief EVE1 System Link - used for non-link specific proc level
 *        communication
 *
 *******************************************************************************
 */
#define SYSTEM_LINK_ID_EVE1         EVE1_LINK(SYSTEM_LINK_ID_PROCK_LINK_ID)

/**
 *******************************************************************************
 *
 * \brief EVE2 System Link - used for non-link specific proc level
 *        communication
 *
 *******************************************************************************
 */
#define SYSTEM_LINK_ID_EVE2         EVE2_LINK(SYSTEM_LINK_ID_PROCK_LINK_ID)

/**
 *******************************************************************************
 *
 * \brief EVE3 System Link - used for non-link specific proc level
 *        communication
 *
 *******************************************************************************
 */
#define SYSTEM_LINK_ID_EVE3         EVE3_LINK(SYSTEM_LINK_ID_PROCK_LINK_ID)

/**
 *******************************************************************************
 *
 * \brief EVE4 System Link - used for non-link specific proc level
 *        communication
 *
 *******************************************************************************
 */
#define SYSTEM_LINK_ID_EVE4         EVE4_LINK(SYSTEM_LINK_ID_PROCK_LINK_ID)

/**
 *******************************************************************************
 *
 * \brief A15_0 System Link - used for non-link specific proc level
 *        communication
 *
 *******************************************************************************
 */
#define SYSTEM_LINK_ID_A15_0        A15_0_LINK(SYSTEM_LINK_ID_PROCK_LINK_ID)

/* @} */

/*******************************************************************************
 *  Enums
 *******************************************************************************
 */

 /**
 *******************************************************************************
 *
 *  \brief Link Id for links that can reside on any processor
 *
 *         These identifiers are not valid as-is. They need to be combined with
 *         processor Id in order to make a valid link ID.
 *
 *         The macros below can be used to create the processor specific
 *         link Id.
 *          - EVEx_LINK(common link id)
 *          - DSPx_LINK(common link id)
 *          - A15_x_LINK(common link id)
 *          - IPUx_y_LINK(common link id)
 *
 *         NOTE: Even though a link ID is listed here, for some core's
 *               the actually number of instances created will vary.
 *
 *               Ex, for EVE the number of link instances created is
 *                     IPC IN  = 1 instance
 *                     IPC OUT = 1 instance
 *                     ALG     = 1 instance
 *
 *         TO increase number of links on a given processor
 *               - Add more enum IDs for that link here
 *               - AND increase the number of link instances in file
 *               /vision_sdk/src/links_common/[link]/[link]Link_cfg.h
 *                 OR
 *               /vision_sdk/src/links_common/[link]/[link]Link_priv.h
 *
 *               - Modify "#define [link]_LINK_OBJ_MAX" to match number of enums
 *                 in this file
 *
 *         The processor specific link ID should be used with all link APIs
 *
 *******************************************************************************
 */
typedef enum
{
    SYSTEM_LINK_ID_IPC_OUT_0 = 0,
    /**< IPC Output Link Id  - used to xfr data across
     *   processors */

    SYSTEM_LINK_ID_IPC_OUT_1 = 1,
    /**< IPC Output Link Id  - used to xfr data across
     *   processors */

    SYSTEM_LINK_ID_IPC_OUT_2 = 2,
    /**< IPC Output Link Id  - used to xfr data across
     *   processors */

    SYSTEM_LINK_ID_IPC_OUT_3 = 3,
    /**< IPC Output Link Id  - used to xfr data across
     *   processors */

    SYSTEM_LINK_ID_IPC_OUT_4 = 4,
    /**< IPC Output Link Id  - used to xfr data across
     *   processors */

    SYSTEM_LINK_ID_IPC_OUT_5 = 5,
    /**< IPC Output Link Id  - used to xfr data across
     *   processors */

    SYSTEM_LINK_ID_IPC_OUT_6 = 6,
    /**< IPC Output Link Id  - used to xfr data across
     *   processors */

    SYSTEM_LINK_ID_IPC_OUT_7 = 7,
    /**< IPC Output Link Id  - used to xfr data across
     *   processors */

    SYSTEM_LINK_ID_IPC_IN_0 = 8,
    /**< IPC Input Link Id - used to xfr data across
      *  processors */

    SYSTEM_LINK_ID_IPC_IN_1 = 9,
    /**< IPC Input Link Id - used to xfr data across
     *   processors */

    SYSTEM_LINK_ID_IPC_IN_2 = 10,
    /**< IPC Input Link Id - used to xfr data across
     *   processors */

    SYSTEM_LINK_ID_IPC_IN_3 = 11,
    /**< IPC Input Link Id - used to xfr data across
     *   processors */

    SYSTEM_LINK_ID_IPC_IN_4 = 12,
    /**< IPC Input Link Id - used to xfr data across
     *   processors */

    SYSTEM_LINK_ID_IPC_IN_5 = 13,
    /**< IPC Input Link Id - used to xfr data across
     *   processors */

    SYSTEM_LINK_ID_IPC_IN_6 = 14,
    /**< IPC Input Link Id - used to xfr data across
     *   processors */

    SYSTEM_LINK_ID_IPC_IN_7 = 15,
    /**< IPC Input Link Id - used to xfr data across
     *   processors */

    SYSTEM_LINK_ID_NULL_0 = 16,
    /**< Null Link - Can be used as a tap point to verify various
     *   sub-chains. Doesnt do any processing : 4*/

    SYSTEM_LINK_ID_NULL_1 = 17,
    /**< Null Link - Can be used as a tap point to verify various
     *   sub-chains. Doesnt do any processing : 5*/

    SYSTEM_LINK_ID_GRPX_SRC_0 = 18,
    /**< Null source link - can be used as a source link providing
     *   dummy data */

    SYSTEM_LINK_ID_GRPX_SRC_1 = 19,
    /**< Null source link - can be used as a source link providing
     *   dummy data */

    SYSTEM_LINK_ID_DUP_0 = 20,
    /**< Dup Link - Duplicate frames and provides multiple outputs */

    SYSTEM_LINK_ID_DUP_1 = 21,
    /**< Dup Link - Duplicate frames and provides multiple outputs */

    SYSTEM_LINK_ID_DUP_2 = 22,
    /**< Dup Link - Duplicate frames and provides multiple outputs */

    SYSTEM_LINK_ID_DUP_3 = 23,
    /**< Dup Link - Duplicate frames and provides multiple outputs */

    SYSTEM_LINK_ID_DUP_4 = 24,
    /**< Dup Link - Duplicate frames and provides multiple outputs */

    SYSTEM_LINK_ID_GATE_0 = 25,
    /**< Gate Link - Acts a on/off switch and allows partial data
         flow to exist */

    SYSTEM_LINK_ID_GATE_1 = 26,
    /**< Gate Link - Acts a on/off switch and allows partial data
         flow to exist */

    SYSTEM_LINK_ID_GATE_2 = 27,
    /**< Gate Link - Acts a on/off switch and allows partial data
         flow to exist */

    SYSTEM_LINK_ID_GATE_3 = 28,
    /**< Gate Link - Acts a on/off switch and allows partial data
         flow to exist */

    SYSTEM_LINK_ID_SYNC_0 = 29,
    /**< Sync Link - creates a composite frame by composing multiple incoming
         Frames */

    SYSTEM_LINK_ID_SYNC_1 = 30,
    /**< Sync Link - creates a composite frame by composing multiple incoming
         Frames */

    SYSTEM_LINK_ID_SYNC_2 = 31,
    /**< Sync Link - creates a composite frame by composing multiple incoming
         Frames */

    SYSTEM_LINK_ID_SYNC_3 = 32,
    /**< Sync Link - creates a composite frame by composing multiple incoming
         Frames */

    SYSTEM_LINK_ID_MERGE_0 = 33,
    /**< Merge Link - Merge different input queue frames & provide them as
     *   single output source. Channel numbering is sequential wrt to
     *   input queues */

    SYSTEM_LINK_ID_MERGE_1 = 34,
    /**< Merge Link - Merge different input queue frames & provide them as
     *   single output source. Channel numbering is sequential wrt to
     *   input queues */

    SYSTEM_LINK_ID_MERGE_2 = 35,
    /**< Merge Link - Merge different input queue frames & provide them as
     *   single output source. Channel numbering is sequential wrt to
     *   input queues */

    SYSTEM_LINK_ID_MERGE_3 = 36,
    /**< Merge Link - Merge different input queue frames & provide them as
     *   single output source. Channel numbering is sequential wrt to
     *   input queues */

    SYSTEM_LINK_ID_MERGE_4 = 37,
    /**< Merge Link - Merge different input queue frames & provide them as
     *   single output source. Channel numbering is sequential wrt to
     *   input queues */


    SYSTEM_LINK_ID_SELECT_0 = 38,
    /**< Select Link enables configurable mapping of specific channels
     *   to be sent out in multiple queues */

    SYSTEM_LINK_ID_SELECT_1 = 39,
    /**< Select Link enables configurable mapping of specific channels
     *   to be sent out in multiple queues */

    SYSTEM_LINK_ID_ALG_0 = 40,
    /**< Algorithm link Id */

    SYSTEM_LINK_ID_ALG_1 = 41,
    /**< Algorithm link Id */

    SYSTEM_LINK_ID_ALG_2 = 42,
    /**< Algorithm link Id */

    SYSTEM_LINK_ID_ALG_3 = 43,
    /**< Algorithm link Id */

    SYSTEM_LINK_ID_ALG_4 = 44,
    /**< Algorithm link Id */

    SYSTEM_LINK_ID_ALG_5 = 45,
    /**< Algorithm link Id */

    SYSTEM_LINK_ID_ALG_6 = 46,
    /**< Algorithm link Id */

    SYSTEM_LINK_ID_ALG_7 = 47,
    /**< Algorithm link Id */

    SYSTEM_LINK_ID_NULL_SRC_0 = 48,
    /**< Null source link - can be used as a source link providing
     *   dummy data */

    SYSTEM_LINK_ID_AVB_RX = 49,
    /**< AvbRx link Id - can be used for supported CPUs only.
     *
     *   Supported CPUs: ipu1_0, ipu1_1, a15_0 in TDA2xx
     *   Supported CPUs: ipu1_0, ipu1_1        in TDA3xx
     */

    SYSTEM_LINK_ID_RTI_0 = 50,
    /* RTI link - only on TDA3x, on all cores except EVE/M4_1 */

    SYSTEM_LINK_COMMON_LINKS_MAX_ID = 51
    /**< Common Links - Max Id */
} SYSTEM_LINK_IDS_COMMON;

/**
 *******************************************************************************
 *
 *  \brief Links Ids specific to IPU1_0.
 *
 *         These are valid Link Ids available in IPU1_0.
 *         Few of the links create multiple instances with unique
 *         identifier - like xxx_0, xxx_1
 *
 *******************************************************************************
 */
typedef  enum
{
    SYSTEM_LINK_ID_CAPTURE_0           =
                    IPU1_0_LINK((uint32_t)SYSTEM_LINK_COMMON_LINKS_MAX_ID+1U),
    /**< Capture link. Present in IPU1_0 */

    SYSTEM_LINK_ID_CAPTURE_1           =
                    IPU1_0_LINK((uint32_t)SYSTEM_LINK_COMMON_LINKS_MAX_ID+2U),
    /**< Capture link. Present in IPU1_0 */

    SYSTEM_LINK_ID_CAPTURE             = SYSTEM_LINK_ID_CAPTURE_0,
    /**< Capture link ID to keep the name backward compatible */

    SYSTEM_LINK_ID_DISPLAYCTRL      =
                    IPU1_0_LINK((uint32_t)SYSTEM_LINK_COMMON_LINKS_MAX_ID+3U),
    /**< Display link Control - enables configurations of Vencs.
         Present in IPU1_0 */

    SYSTEM_LINK_ID_DISPLAY_0         =
                    IPU1_0_LINK((uint32_t)SYSTEM_LINK_COMMON_LINKS_MAX_ID+4U),
    /**< Display link - enables one of the outputs like HDTV, SDTV etc.
         Present in IPU1_0 */

    SYSTEM_LINK_ID_DISPLAY_1         =
                    IPU1_0_LINK((uint32_t)SYSTEM_LINK_COMMON_LINKS_MAX_ID+5U),
    /**< Display link - enables one of the outputs like HDTV, SDTV etc.
         Present in IPU1_0 */

    SYSTEM_LINK_ID_DISPLAY_2         =
                    IPU1_0_LINK((uint32_t)SYSTEM_LINK_COMMON_LINKS_MAX_ID+6U),

    /**< Display link - enables one of the outputs like HDTV, SDTV etc.
         Present in IPU1_0 */

    SYSTEM_LINK_ID_DISPLAY_3        =
                    IPU1_0_LINK((uint32_t)SYSTEM_LINK_COMMON_LINKS_MAX_ID+7U),
    /**< Display link - enables one of the outputs like HDTV, SDTV etc.
         Present in IPU1_0 */

    SYSTEM_LINK_ID_VPE_0            =
                    IPU1_0_LINK((uint32_t)SYSTEM_LINK_COMMON_LINKS_MAX_ID+8U),
    /**< VPE Link Id */

    SYSTEM_LINK_ID_VPE_1            =
                    IPU1_0_LINK((uint32_t)SYSTEM_LINK_COMMON_LINKS_MAX_ID+9U),
    /**< VPE Link Id */

    SYSTEM_LINK_ID_VPE_2            =
                    IPU1_0_LINK((uint32_t)SYSTEM_LINK_COMMON_LINKS_MAX_ID+10U),
    /**< VPE Link Id */

    SYSTEM_LINK_ID_VPE_3            =
                    IPU1_0_LINK((uint32_t)SYSTEM_LINK_COMMON_LINKS_MAX_ID+11U),
    /**< VPE Link Id */

    SYSTEM_LINK_ID_VENC_0           =
                    IPU1_0_LINK((uint32_t)SYSTEM_LINK_COMMON_LINKS_MAX_ID+12U),
    /**< Video encode link Id */

    SYSTEM_LINK_ID_VDEC_0           =
                    IPU1_0_LINK((uint32_t)SYSTEM_LINK_COMMON_LINKS_MAX_ID+13U),
    /**< Video decode link Id */

    SYSTEM_LINK_ID_ISSCAPTURE_0     =
                    IPU1_0_LINK((uint32_t)SYSTEM_LINK_COMMON_LINKS_MAX_ID+14U),
    /**< Iss Capture link 0. Present in IPU1_0 */

    SYSTEM_LINK_ID_ISSM2MISP_0     =
                    IPU1_0_LINK((uint32_t)SYSTEM_LINK_COMMON_LINKS_MAX_ID+15U),
    /**< Iss Capture link 0. Present in IPU1_0 */

    /**< Ultrasonic capture link Id */
    SYSTEM_LINK_ID_ISSM2MSIMCOP_0 =
                    IPU1_0_LINK((uint32_t)SYSTEM_LINK_COMMON_LINKS_MAX_ID+16U),
    /**< Simcop link Id */

    SYSTEM_LINK_ID_APP_CTRL         =
                    IPU1_0_LINK((uint32_t)SYSTEM_LINK_COMMON_LINKS_MAX_ID+17U),
    /**< Application specific control link Id */

    SYSTEM_LINK_ID_ULTRASONIC_CAPTURE =
                    IPU1_0_LINK((uint32_t)SYSTEM_LINK_COMMON_LINKS_MAX_ID+18U),
    /**< Ultrasonic capture link */

    SYSTEM_LINK_ID_HCF_0 =
                    IPU1_0_LINK((uint32_t)SYSTEM_LINK_COMMON_LINKS_MAX_ID+19U),
    /**< HCF link */

    SYSTEM_LINK_ID_SPLIT_0 =
                    IPU1_0_LINK((uint32_t)SYSTEM_LINK_COMMON_LINKS_MAX_ID+20U),
    /**< Split Link - Split frames into different queues */

    SYSTEM_LINK_ID_RADAR_CAPTURE =
                    IPU1_0_LINK((uint32_t)SYSTEM_LINK_COMMON_LINKS_MAX_ID+21U),
    /**< Radar Capture Link */

    SYSTEM_IPU1_0_LINK_ID_MAX = 0x3FFFFFFFU
    /**< Max link ID for IPU1-0 specfic links */

} SYSTEM_IPU1_0_LINK_IDS;

/**
 *******************************************************************************
 *
 *  \brief Links Ids specific to A15 (Linux).
 *
 *         These are valid Link Ids available in A15 (Linux).
 *         Few of the links create multiple instances with unique
 *         identifier - like xxx_0, xxx_1
 *
 *******************************************************************************
 */
typedef  enum
{
    SYSTEM_LINK_ID_SGXDISPLAY_0     =
                    A15_0_LINK((uint32_t)SYSTEM_LINK_COMMON_LINKS_MAX_ID+1U),
    /**< SgxDiaply link - enables DRM display along with SGX rendering,
         Present only on A15_0 */

    SYSTEM_LINK_ID_SGX3DSRV_0     =
                    A15_0_LINK((uint32_t)SYSTEM_LINK_COMMON_LINKS_MAX_ID+2U),
    /**< Sgx3Dsrv link - For 3D SRV output creation/rendering on SGX,
         Present only on A15_0 */

    SYSTEM_LINK_ID_SRV3D_0 =
                    A15_0_LINK((uint32_t)SYSTEM_LINK_COMMON_LINKS_MAX_ID+3U),
    /**< srv3D link - enables srv3D on InfoADAS,
         Present only on A15_0 */

    SYSTEM_LINK_ID_EP_0 =
                    A15_0_LINK((uint32_t)SYSTEM_LINK_COMMON_LINKS_MAX_ID+4U),
    /**< endpoint link id - 0,
         Present only on A15 */
    SYSTEM_LINK_ID_EP_1 =
                    A15_0_LINK((uint32_t)SYSTEM_LINK_COMMON_LINKS_MAX_ID+5U)
    /**< endpoint link id - 1,
         Present only on A15 */
} SYSTEM_A15_0_LINK_IDS;


#ifdef  __cplusplus
}
#endif

#endif

/*@}*/

