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
 *  \file bsp_boardPriv.h
 *
 *  \brief Board module private header file.
 */

#ifndef BSP_BOARD_PRIV_H_
#define BSP_BOARD_PRIV_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define BOARD_EEPROM_I2C_ADDR_0                 ((UInt8)0x50U)
#define BOARD_EEPROM_I2C_ADDR_1                 ((UInt8)0x51U)

#define BOARD_TVP7002_I2C_ADDR_0                (0x5DU)

#define BOARD_TVP5158_I2C_ADDR_0                ((UInt8)0x58U)
#define BOARD_TVP5158_I2C_ADDR_1                (0x59U)
#define BOARD_TVP5158_I2C_ADDR_2                (0x5AU)
#define BOARD_TVP5158_I2C_ADDR_3                (0x5BU)
#define BOARD_TVP5158_I2C_ADDR_4                (0x5CU)
#define BOARD_TVP5158_I2C_ADDR_5                (0x5DU)
#define BOARD_TVP5158_I2C_ADDR_6                (0x5EU)
#define BOARD_TVP5158_I2C_ADDR_7                (0x5FU)

#define BOARD_SIL9022_I2C_ADDR_0                (0x39U)
#define BOARD_SIL9022_I2C_ADDR_1                (0x3BU)

#define BOARD_SII9127_I2C_ADDR_0                (0x30U)
#define BOARD_SII9127_I2C_ADDR_1                ((UInt8)0x31U)

#define BOARD_ADV7611_I2C_ADDR_0                (0x4CU)
#define BOARD_ADV7611_I2C_ADDR_1                (0x4DU)

#define BOARD_DS90UH925_I2C_ADDR_0              (0x0CU)
#define BOARD_DS90UH925_I2C_ADDR_1              (0x0DU)
#define BOARD_DS90UH925_I2C_ADDR_2              (0x0EU)
#define BOARD_DS90UH925_I2C_ADDR_3              (0x0FU)
#define BOARD_DS90UH925_I2C_ADDR_4              (0x10U)
#define BOARD_DS90UH925_I2C_ADDR_5              (0x11U)
#define BOARD_DS90UH925_I2C_ADDR_6              (0x12U)
#define BOARD_DS90UH925_I2C_ADDR_7              (0x13U)
#define BOARD_DS90UH925_I2C_ADDR_8              (0x14U)
#define BOARD_DS90UH925_I2C_ADDR_9              (0x15U)
#define BOARD_DS90UH925_I2C_ADDR_10             (0x16U)
#define BOARD_DS90UH925_I2C_ADDR_11             (0x17U)
#define BOARD_DS90UH925_I2C_ADDR_12             (0x18U)
#define BOARD_DS90UH925_I2C_ADDR_13             (0x19U)
#define BOARD_DS90UH925_I2C_ADDR_14             (0x1AU)
#define BOARD_DS90UH925_I2C_ADDR_15             (0x1BU)

#define BOARD_DS90UH926_I2C_ADDR_0              (0x2CU)
#define BOARD_DS90UH926_I2C_ADDR_1              (0x2DU)
#define BOARD_DS90UH926_I2C_ADDR_2              (0x2EU)
#define BOARD_DS90UH926_I2C_ADDR_3              (0x2FU)
#define BOARD_DS90UH926_I2C_ADDR_4              (0x30U)
#define BOARD_DS90UH926_I2C_ADDR_5              (0x31U)
#define BOARD_DS90UH926_I2C_ADDR_6              (0x32U)
#define BOARD_DS90UH926_I2C_ADDR_7              (0x33U)
#define BOARD_DS90UH926_I2C_ADDR_8              (0x34U)
#define BOARD_DS90UH926_I2C_ADDR_9              (0x35U)
#define BOARD_DS90UH926_I2C_ADDR_10             (0x36U)
#define BOARD_DS90UH926_I2C_ADDR_11             (0x37U)
#define BOARD_DS90UH926_I2C_ADDR_12             (0x38U)
#define BOARD_DS90UH926_I2C_ADDR_13             (0x39U)
#define BOARD_DS90UH926_I2C_ADDR_14             (0x3AU)
#define BOARD_DS90UH926_I2C_ADDR_15             (0x3BU)

#define BOARD_MULTIDES_IO_EXP_I2C_ADDR_0        ((UInt8)0x25U)

#define BOARD_SII9135_I2C_ADDR_0                (0x30U)

#define BOARD_TLC59108_I2C_ADDR_0               (0x40U)

/** \brief I2C address of the Aptina MT9V022 Sensor. */
#define BOARD_MT9V022_I2C_ADDR_0                (0xB8U >> 1U)

/** \brief I2C address of the OmniVision 10630/10635 Sensor. */
#define BOARD_OV1063x_I2C_ADDR_0                (0x30U)

/** \brief I2C address of the Aptina MT9M024 Sensor. */
#define BOARD_MT9M024_I2C_ADDR_0                (0x20U >> 1U)

/** \brief I2C address of the LI OmniVision 10633 Sensor. */
#define BOARD_LI_OV1063X_I2C_ADDR_0             (0x37U)

/** \brief I2C address of the Aptina AR0132 Sensor. */
#define BOARD_APT_AR0132_I2C_ADDR_0             (0x10U)

/** \brief I2C address of the Aptina AR0140 Sensor. */
#define BOARD_APT_AR0140_I2C_ADDR_0             (0x10U)
/** \brief I2C address of the Aptina AR0140 Sensor in TIDA00262 port 0. */
#define BOARD_APT_TIDA00262_AR0140_I2C_ADDR_0   (0x40U)
/** \brief I2C address of the Aptina AR0140 Sensor in TIDA00262 port 1. */
#define BOARD_APT_TIDA00262_AR0140_I2C_ADDR_1   (0x42U)
/** \brief I2C address of the Aptina AR0140 Sensor in TIDA00262 port 2. */
#define BOARD_APT_TIDA00262_AR0140_I2C_ADDR_2   (0x44U)
/** \brief I2C address of the Aptina AR0140 Sensor in TIDA00262 port 3. */
#define BOARD_APT_TIDA00262_AR0140_I2C_ADDR_3   (0x46U)

/** \brief I2C address of the Aptina AR0132 RCCC Sensor. */
#define BOARD_APT_AR0132RCCC_I2C_ADDR_0             (0x18U)
#define BOARD_APT_AR0132RCCC_I2C_ADDR_1             (0x10U)

/** \I2C address of the Audio codec on MC board */
#define BOARD_TLV320AIC3106_I2C_ADDR_0          ((UInt8)0x0CU)

/** \brief I2C address of OV10640 Sensor For CSI2 */
#define BOARD_OV10640_I2C_ADDR_CSI2             (0x32U)
/** \brief I2C address of OV10640 Sensor For CPI,
       For old OV10640 Sensor, I2C address is 0x31,
           So change this macro for the old sensor. */
#define BOARD_OV10640_I2C_ADDR_CPI              (0x33U)

/** \brief I2C address of IMX224 Sensor For CSI2 */
#define BOARD_IMX224_I2C_ADDR_CSI2              (0x36U)

#define BOARD_IO_EXP_I2C_ADDR_0                 (0x20U)
#define BOARD_IO_EXP_I2C_ADDR_1                 (0x21U)
#define BOARD_IO_EXP_I2C_ADDR_2                 (0x22U)
#define BOARD_IO_EXP_I2C_ADDR_3                 (0x23U)
#define BOARD_IO_EXP_I2C_ADDR_4                 (0x24U)
#define BOARD_IO_EXP_I2C_ADDR_5                 (0x25U)
#define BOARD_IO_EXP_I2C_ADDR_6                 (0x26U)
#define BOARD_IO_EXP_I2C_ADDR_7                 (0x27U)

/* THS7353 filter I2C address present in VC daughter card */
#define BOARD_THS7353_I2C_ADDR                  (0x2CU)

/** \brief I2C address of the PCA9543 i2c switch. */
#define BOARD_PCA9543_I2C_ADDR                  (0x70U)

/** \brief I2C address of the STM touch screen used in 10-inch LCD board. */
#define BOARD_STM_TOUCH_I2C_ADDR0               (0x18U)

/** \brief I2C address of the STM touch screen used in 7-inch LCD board. */
#define BOARD_ATMXT224_TOUCH_I2C_ADDR0          (0x4AU)

/* Defines specific to on-board peripherals */
#define PCF8575_P0_0_MASK                       ((UInt32)0x01U)
#define PCF8575_P0_1_MASK                       ((UInt32)0x02U)
#define PCF8575_P0_2_MASK                       ((UInt32)0x04U)
#define PCF8575_P0_3_MASK                       ((UInt32)0x08U)
#define PCF8575_P0_4_MASK                       ((UInt32)0x10U)
#define PCF8575_P0_5_MASK                       ((UInt32)0x20U)
#define PCF8575_P0_6_MASK                       ((UInt32)0x40U)
#define PCF8575_P0_7_MASK                       ((UInt32)0x80U)
#define PCF8575_P1_0_MASK                       ((UInt32)0x01U)
#define PCF8575_P1_1_MASK                       ((UInt32)0x02U)
#define PCF8575_P1_2_MASK                       ((UInt32)0x04U)
#define PCF8575_P1_3_MASK                       ((UInt32)0x08U)
#define PCF8575_P1_4_MASK                       ((UInt32)0x10U)
#define PCF8575_P1_5_MASK                       ((UInt32)0x20U)
#define PCF8575_P1_6_MASK                       ((UInt32)0x40U)
#define PCF8575_P1_7_MASK                       ((UInt32)0x80U)

#define TCA64_P0_0_MASK                         ((UInt32)0x01U)
#define TCA64_P0_1_MASK                         ((UInt32)0x02U)
#define TCA64_P0_2_MASK                         ((UInt32)0x04U)
#define TCA64_P0_3_MASK                         ((UInt32)0x08U)
#define TCA64_P0_4_MASK                         ((UInt32)0x10U)
#define TCA64_P0_5_MASK                         ((UInt32)0x20U)
#define TCA64_P0_6_MASK                         ((UInt32)0x40U)
#define TCA64_P0_7_MASK                         ((UInt32)0x80U)
#define TCA64_P1_0_MASK                         ((UInt32)0x01U)
#define TCA64_P1_1_MASK                         ((UInt32)0x02U)
#define TCA64_P1_2_MASK                         ((UInt32)0x04U)
#define TCA64_P1_3_MASK                         ((UInt32)0x08U)
#define TCA64_P1_4_MASK                         ((UInt32)0x10U)
#define TCA64_P1_5_MASK                         ((UInt32)0x20U)
#define TCA64_P1_6_MASK                         ((UInt32)0x40U)
#define TCA64_P1_7_MASK                         ((UInt32)0x80U)
#define TCA64_P2_0_MASK                         ((UInt32)0x01U)
#define TCA64_P2_1_MASK                         ((UInt32)0x02U)
#define TCA64_P2_2_MASK                         ((UInt32)0x04U)
#define TCA64_P2_3_MASK                         ((UInt32)0x08U)
#define TCA64_P2_4_MASK                         ((UInt32)0x10U)
#define TCA64_P2_5_MASK                         ((UInt32)0x20U)
#define TCA64_P2_6_MASK                         ((UInt32)0x40U)
#define TCA64_P2_7_MASK                         ((UInt32)0x80U)

/*  VC
 *  PCF8575 - mappings
 *  P16, P17 - TVP7002 select
 *  P7 - THS73861_FILTER2
 *  P6 - THS73861_FILTER1
 *  P5 - THS73861_BYPASS
 *  P4 - THS73861_DISABLE
 *  P0 - TVP7002_RSTN
 */
#define BOARD_PCF8575_TVP_RESETn_MASK           (PCF8575_P0_0_MASK)
#define BOARD_PCF8575_THS73861_DISABLE_MASK     (PCF8575_P0_4_MASK)
#define BOARD_PCF8575_THS73861_BYPASS_MASK      (PCF8575_P0_5_MASK)
#define BOARD_PCF8575_THS73861_FILTER1_MASK     (PCF8575_P0_6_MASK)
#define BOARD_PCF8575_THS73861_FILTER2_MASK     (PCF8575_P0_7_MASK)
#define BOARD_PCF8575_SEL_TVP_S0_MASK           (PCF8575_P1_6_MASK)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Functions                                          */
/* ========================================================================== */

Int32 Bsp_boardTda2xxInit(void);
Bsp_BoardId Bsp_boardGetIdTda2xx(void);
Bsp_BoardRev Bsp_boardGetBaseBoardRevTda2xx(void);
Bsp_BoardRev Bsp_boardGetDcRevTda2xx(void);
const Bsp_BoardI2cData *Bsp_boardGetI2cDataTda2xx(void);
const Bsp_BoardData *Bsp_boardGetDataTda2xx(void);
Int32 Bsp_boardSetPinMuxTda2xx(
    UInt32        drvId,
    UInt32        instId,
    Bsp_BoardMode boardMode);

Int32 Bsp_boardTda2exInit(void);
Bsp_BoardId Bsp_boardGetIdTda2ex(void);
Bsp_BoardRev Bsp_boardGetBaseBoardRevTda2ex(void);
Bsp_BoardRev Bsp_boardGetDcRevTda2ex(void);
const Bsp_BoardI2cData *Bsp_boardGetI2cDataTda2ex(void);
const Bsp_BoardData *Bsp_boardGetDataTda2ex(void);
Int32 Bsp_boardSetPinMuxTda2ex(
    UInt32        drvId,
    UInt32        instId,
    Bsp_BoardMode boardMode);

Int32 Bsp_boardTda3xxInit(void);
Bsp_BoardId Bsp_boardGetIdTda3xx(void);
Bsp_BoardRev Bsp_boardGetBaseBoardRevTda3xx(void);
Bsp_BoardRev Bsp_boardGetDcRevTda3xx(void);
const Bsp_BoardI2cData *Bsp_boardGetI2cDataTda3xx(void);
const Bsp_BoardData *Bsp_boardGetDataTda3xx(void);
Int32 Bsp_boardSelectModeTda3xx(UInt32        devDrvId,
                                UInt32        instId,
                                Bsp_BoardMode boardMode);
Int32 Bsp_boardSetPinMuxTda3xx(
    UInt32        drvId,
    UInt32        instId,
    Bsp_BoardMode boardMode);

Bsp_BoardId Bsp_boardGetIdOmap5430(void);
Bsp_BoardRev Bsp_boardGetBaseBoardRevOmap5430(void);
Bsp_BoardRev Bsp_boardGetDcRevOmap5430(void);
const Bsp_BoardI2cData *Bsp_boardGetI2cDataOmap5430(void);
const Bsp_BoardData *Bsp_boardGetDataOmap5430(void);

Bsp_BoardId Bsp_boardGetIdTI814x(void);
Bsp_BoardRev Bsp_boardGetBaseBoardRevTI814x(void);
Bsp_BoardRev Bsp_boardGetDcRevTI814x(void);
const Bsp_BoardI2cData *Bsp_boardGetI2cDataTI814x(void);
const Bsp_BoardData *Bsp_boardGetDataTI814x(void);

#ifdef __cplusplus
}
#endif

#endif  /* #ifndef BSP_BOARD_PRIV_H_ */

/* @} */
