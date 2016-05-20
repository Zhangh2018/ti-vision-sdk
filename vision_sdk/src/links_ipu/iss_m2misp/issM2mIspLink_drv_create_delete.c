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
 * \file issM2mIspLink_drv_create_delete.c
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "issM2mIspLink_priv.h"

/**
 *******************************************************************************
 *
 * \brief This function creates the driver instance for a given pass Id
 *
 * \param  pObj     [IN] Global link object
 * \param  chId     [IN] Id of the channel
 * \param  passId   [IN] First or second pass
 * \param  pPassCfg [IN] Configuration for the given pass
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssM2mIspLink_drvCreateDrv(IssM2mIspLink_Obj *pObj,
                                  UInt32             chId,
                                  UInt32             passId
                                  )
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    IssM2mIspLink_ChannelObject *pChObj;
    Fvid2_CbParams             cbPrms;
    Vps_M2mIntfCreateStatus    createStatusPrms;
    vpsissIspOpenRetParams_t   retPrms;
    Vps_M2mIntfCreateParams    createPrms;
    IssM2mIspLink_PassObj *pPassCfg;
    IssM2mIspLink_OperatingMode operatingMode;
    Bool h3aEnable;

    pChObj = &pObj->chObj[chId];
    pPassCfg = &pChObj->passCfg[passId];
    operatingMode = pObj->createArgs.channelParams[chId].operatingMode;
    h3aEnable = (Bool)pObj->createArgs.channelParams[chId].enableOut[ISSM2MISP_LINK_OUTPUTQUE_H3A];

    Fvid2CbParams_init(&cbPrms);
    cbPrms.cbFxn = &IssM2mIspLink_drvCallBack;
    cbPrms.appData = pObj;

    VpsM2mIntfCreateParams_init(&createPrms);
    createPrms.numCh = 1U;
    createPrms.chInQueueLength = 1U;
    createPrms.maxStatsInst = 0U;
    createPrms.pAdditionalArgs = (Ptr)&pPassCfg->openPrms;

    VpsM2mIntfCreateStatus_init(&createStatusPrms);
    createStatusPrms.pAdditionalStatus = (Ptr) &retPrms;

    pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_IPIPEIF]= (UInt32)FALSE;
    pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_NSF3]   = (UInt32)FALSE;
    pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_GLBCE]  = (UInt32)FALSE;
    pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_ISIF]   = (UInt32)FALSE;
    pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_IPIPE]  = (UInt32)FALSE;
    pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_RSZ]    = (UInt32)FALSE;
    pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_CNF]    = (UInt32)FALSE;
    pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_H3A]    = (UInt32)FALSE;
    pPassCfg->openPrms.arg = NULL;

    if (operatingMode == ISSM2MISP_LINK_OPMODE_12BIT_LINEAR)
    {
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_IPIPEIF]= (UInt32)TRUE;
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_NSF3]   = (UInt32)TRUE;
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_ISIF]   = (UInt32)TRUE;
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_IPIPE]  = (UInt32)TRUE;
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_RSZ]    = (UInt32)TRUE;
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_CNF]    = (UInt32)TRUE;
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_H3A]    = (UInt32)h3aEnable;
        pPassCfg->openPrms.arg = NULL;
    }
    else if (operatingMode == ISSM2MISP_LINK_OPMODE_1PASS_WDR)
    {
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_IPIPEIF] = (UInt32)TRUE;
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_NSF3]    = (UInt32)TRUE;
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_GLBCE] = (UInt32)TRUE;
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_ISIF]  = (UInt32)TRUE;
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_IPIPE] = (UInt32)TRUE;
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_RSZ]   = (UInt32)TRUE;
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_CNF]   = (UInt32)TRUE;
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_H3A]   = (UInt32)h3aEnable;
        pPassCfg->openPrms.arg = NULL;
    }
    else if(((Bool)TRUE == IssM2mIspLink_isWdrMode(operatingMode)) &&
            (passId == ISSM2MISP_LINK_FIRST_PASS))
    {
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_IPIPEIF] = (UInt32)TRUE;
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_NSF3]    = (UInt32)TRUE;
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_ISIF]  = (UInt32)TRUE;
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_IPIPE] = (UInt32)TRUE;
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_RSZ]   = (UInt32)TRUE;
        /* using H3A only in the second pass */
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_H3A]   = (UInt32)FALSE;
        pPassCfg->openPrms.arg = NULL;
    }
    else if(((Bool)TRUE == IssM2mIspLink_isWdrMode(operatingMode)) &&
            (passId == ISSM2MISP_LINK_SECOND_PASS))
    {
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_IPIPEIF] = (UInt32)TRUE;
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_NSF3]    = (UInt32)TRUE;
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_GLBCE] = (UInt32)TRUE;
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_ISIF]  = (UInt32)TRUE;
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_IPIPE] = (UInt32)TRUE;
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_RSZ]   = (UInt32)TRUE;
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_CNF]   = (UInt32)TRUE;
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_H3A]   = (UInt32)h3aEnable;
        pPassCfg->openPrms.arg = NULL;
    }
    else if (ISSM2MISP_LINK_OPMODE_12BIT_MONOCHROME == operatingMode)
    {
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_IPIPEIF] = (UInt32)TRUE;
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_NSF3]    = (UInt32)TRUE;
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_GLBCE] = (UInt32)TRUE;
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_ISIF]  = (UInt32)TRUE;

        /* IPIPE is requred for the DPC */
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_IPIPE] = (UInt32)TRUE;
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_RSZ]   = (UInt32)TRUE;

        /* CNF is not enabled as output is luma only */
        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_CNF]   = (UInt32)FALSE;

        pPassCfg->openPrms.isModuleReq[VPS_ISS_ISP_MODULE_H3A]   = (UInt32)h3aEnable;
        pPassCfg->openPrms.arg = NULL;
    }
    else
    {
        UTILS_assert((Bool)0U);
    }

    pPassCfg->drvHandle = Fvid2_create(
        FVID2_VPS_COMMON_M2M_INTF_DRV,
        VPS_M2M_ISS_INST_CAL_ISP,
        &createPrms,
        &createStatusPrms,
        &cbPrms);

    if(NULL == pPassCfg->drvHandle)
    {
        Vps_printf("ISSM2MISP: ERROR: CH%d: FIVD2 Create failed (Pass Id = %d)!!!",
                    chId, passId);
        UTILS_assert((Bool)0U);
    }

    if ((UInt32)TRUE == pPassCfg->openPrms.
        isModuleReq[VPS_ISS_ISP_MODULE_GLBCE])
    {
        pChObj->glbceCtrl.module    = VPS_ISS_GLBCE_MODULE_GET_STATS_INFO;
        pChObj->glbceCtrl.statsInfo = &pObj->glbceStatsInfo;
        status = Fvid2_control(
            pPassCfg->drvHandle,
            VPS_ISS_GLBCE_IOCTL_GET_CONFIG,
            &pChObj->glbceCtrl,
            NULL);

        UTILS_assert(status == FVID2_SOK);
    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief API for setting ISP parameters for the link.
 *
 * \param  pObj     [IN] Link global handle
 * \param  chId     [IN]
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssM2mIspLink_drvSetDrvParams(IssM2mIspLink_Obj * pObj,
                                    UInt32 chId,
                                    UInt32 passId)
{
    Int32 status;
    IssM2mIspLink_CreateParams *pCreatePrms;
    System_LinkChInfo *pPrevLinkChInfo;
    IssM2mIspLink_ChannelObject *pChObj;
    IssM2mIspLink_PassObj *pPassCfg;
    IssM2mIspLink_OperatingMode operatingMode;
    Bool h3aEnable, rszAEnable, rszBEnable;
    vpsissIspVp2Config_t vp2Cfg;

    pCreatePrms = &pObj->createArgs;

    pChObj = &pObj->chObj[chId];
    pPassCfg = &pChObj->passCfg[passId];
    operatingMode = pObj->createArgs.channelParams[chId].operatingMode;
    h3aEnable = (Bool)pCreatePrms->channelParams[chId].enableOut[ISSM2MISP_LINK_OUTPUTQUE_H3A];
    rszAEnable = (Bool)pCreatePrms->channelParams[chId].enableOut[ISSM2MISP_LINK_OUTPUTQUE_IMAGE_RSZ_A];
    rszBEnable = (Bool)pCreatePrms->channelParams[chId].enableOut[ISSM2MISP_LINK_OUTPUTQUE_IMAGE_RSZ_B];

    pChObj->expRatio = SENSOR_EXPOSURE_RATIO;
    pChObj->evRatio = SENSOR_EV_RATIO;
    pChObj->pGlbceStatsBuffer = NULL;

    pPrevLinkChInfo = &pObj->inQueInfo.chInfo[chId];

    pPassCfg->ispPrms.inFmt.width         = pPrevLinkChInfo->width;
    pPassCfg->ispPrms.inFmt.height        = pPrevLinkChInfo->height;
    pPassCfg->ispPrms.inFmt.pitch[0u]     = pPrevLinkChInfo->pitch[0U];
    pPassCfg->ispPrms.inFmt.pitch[1u]     = pPrevLinkChInfo->pitch[1];
    pPassCfg->ispPrms.inFmt.pitch[2u]     = pPrevLinkChInfo->pitch[2];
    pPassCfg->ispPrms.inFmt.bpp           = pCreatePrms->channelParams[chId].inBpp;
    pPassCfg->ispPrms.inFmt.dataFormat    = SYSTEM_LINK_CH_INFO_GET_FLAG_DATA_FORMAT(pPrevLinkChInfo->flags);

    /* N-1 frame input format is same as input frame N format */
    pPassCfg->ispPrms.inFmtN_1            = pPassCfg->ispPrms.inFmt;

    pPassCfg->ispPrms.enableWdrMerge                               = (UInt32)FALSE;
    pPassCfg->ispPrms.enableVportCompInput                         = (UInt32)FALSE;
    pPassCfg->ispPrms.enableDfs                                    = (UInt32)FALSE;
    pPassCfg->ispPrms.nsf3Path                                     = VPS_ISS_NSF3_PATH_ISP;
    pPassCfg->ispPrms.glbcePath                                    = VPS_ISS_GLBCE_PATH_DISABLED;
    pPassCfg->ispPrms.enableDpcPreNsf3                             = (UInt32)TRUE;
    pPassCfg->ispPrms.enableCnf                                    = (UInt32)FALSE;
    pPassCfg->ispPrms.enableRszInputFromIpipeif                    = (UInt32)FALSE;
    pPassCfg->ispPrms.enableStreams[VPS_ISS_STREAM_CAL_RD_INPUT_0] = (UInt32)TRUE;
    pPassCfg->ispPrms.enableStreams[VPS_ISS_STREAM_ID_RSZ_A]       = (UInt32)FALSE;
    pPassCfg->ispPrms.enableStreams[VPS_ISS_STREAM_ID_RSZ_B]       = (UInt32)FALSE;
    pPassCfg->ispPrms.enableStreams[VPS_ISS_STREAM_ID_AEWB]        = (UInt32)FALSE;
    pPassCfg->ispPrms.enableStreams[VPS_ISS_STREAM_ID_INPUT_N1]    = (UInt32)FALSE;
    pPassCfg->ispPrms.enableStreams[VPS_ISS_STREAM_ID_AF]          = (UInt32)FALSE;

    pPassCfg->ispPrms.useWen = (UInt32)FALSE;
    pPassCfg->ispPrms.hdPol  = FVID2_POL_HIGH;
    pPassCfg->ispPrms.vdPol  = FVID2_POL_HIGH;

    /* 20bit WDR Line Interleaved, input frame size for the current input
       and previous input is different. Frame size is provided in
       the createPrms.wdrOffsetPrms. Also the pitch is same as the
       input pitch but it will 2 times the input pitch is
       short/long exposure are stored in every alternate lines */
    if (operatingMode == ISSM2MISP_LINK_OPMODE_2PASS_WDR_LINE_INTERLEAVED)
    {
        IssM2mIspLink_WdrOffsetParams_t *wdrOffsetPrms =
            &pCreatePrms->channelParams[chId].wdrOffsetPrms;

        /* Change the current frame input parameters */
        pPassCfg->ispPrms.inFmt.width = wdrOffsetPrms->width;
        pPassCfg->ispPrms.inFmt.height = wdrOffsetPrms->height;

        /* Assuming long and short exposure are stored exactly at alternate
           lines */
        pPassCfg->ispPrms.inFmt.pitch[0u] =
            pPrevLinkChInfo->pitch[0u] * 2U;
        pPassCfg->ispPrms.inFmt.pitch[1u] =
            pPrevLinkChInfo->pitch[1u] * 2U;
        pPassCfg->ispPrms.inFmt.pitch[2u] =
            pPrevLinkChInfo->pitch[2u] * 2U;

        /* Set the parameters for the previous input */
        pPassCfg->ispPrms.inFmtN_1 = pPassCfg->ispPrms.inFmt;

        /* Pitch for the previous input is 2 times width,
           Make sure that the pitch of the first output in resizer is
           also set to 2 times width for this mode */
        pPassCfg->ispPrms.inFmtN_1.pitch[0u] =
            pPassCfg->ispPrms.inFmtN_1.width * 2u;
        pPassCfg->ispPrms.inFmtN_1.pitch[1u] =
            pPassCfg->ispPrms.inFmtN_1.width * 2u;
        pPassCfg->ispPrms.inFmtN_1.pitch[2u] =
            pPassCfg->ispPrms.inFmtN_1.width * 2u;
    }

    if (operatingMode == ISSM2MISP_LINK_OPMODE_12BIT_LINEAR)
    {
        pPassCfg->ispPrms.enableCnf      = (UInt32)TRUE;
        pPassCfg->ispPrms.enableStreams[VPS_ISS_STREAM_ID_RSZ_A]       = (UInt32)rszAEnable;
        pPassCfg->ispPrms.enableStreams[VPS_ISS_STREAM_ID_RSZ_B]       = (UInt32)rszBEnable;
        pPassCfg->ispPrms.enableStreams[VPS_ISS_STREAM_ID_AEWB]        = (UInt32)h3aEnable;
    }
    else if (operatingMode == ISSM2MISP_LINK_OPMODE_1PASS_WDR)
    {
        pPassCfg->ispPrms.enableCnf                              = (UInt32)TRUE;
        pPassCfg->ispPrms.enableVportCompInput                   = (UInt32)FALSE;
        pPassCfg->ispPrms.glbcePath                              = VPS_ISS_GLBCE_PATH_ISP;
        pPassCfg->ispPrms.nsf3Path                               = VPS_ISS_NSF3_PATH_ISP;
        pPassCfg->ispPrms.enableDpcPreNsf3                       = (UInt32)TRUE;
        pPassCfg->ispPrms.enableStreams[VPS_ISS_STREAM_ID_RSZ_A] = (UInt32)rszAEnable;
        pPassCfg->ispPrms.enableStreams[VPS_ISS_STREAM_ID_RSZ_B] = (UInt32)rszBEnable;
        pPassCfg->ispPrms.enableStreams[VPS_ISS_STREAM_ID_AEWB]  = (UInt32)h3aEnable;
    }
    else if(((Bool)TRUE == IssM2mIspLink_isWdrMode(operatingMode)) &&
            (passId == ISSM2MISP_LINK_FIRST_PASS))
    {
        pPassCfg->ispPrms.enableStreams[VPS_ISS_STREAM_ID_RSZ_A]       = (UInt32)TRUE;
        /* Using H3A only in the second pass */
        pPassCfg->ispPrms.enableStreams[VPS_ISS_STREAM_ID_AEWB]        = (UInt32)FALSE;
        pPassCfg->ispPrms.enableRszInputFromIpipeif = (UInt32)TRUE;
    }
    else if(((Bool)TRUE == IssM2mIspLink_isWdrMode(operatingMode)) &&
            (passId == ISSM2MISP_LINK_SECOND_PASS))
    {
        pPassCfg->ispPrms.enableWdrMerge = (UInt32)TRUE;
        pPassCfg->ispPrms.enableCnf      = (UInt32)TRUE;
        pPassCfg->ispPrms.enableStreams[VPS_ISS_STREAM_ID_INPUT_N1]    = (UInt32)TRUE;
        pPassCfg->ispPrms.enableStreams[VPS_ISS_STREAM_ID_RSZ_A]       = (UInt32)rszAEnable;
        pPassCfg->ispPrms.enableStreams[VPS_ISS_STREAM_ID_RSZ_B]       = (UInt32)rszBEnable;
        pPassCfg->ispPrms.enableStreams[VPS_ISS_STREAM_ID_AEWB]        = (UInt32)h3aEnable;
        pPassCfg->ispPrms.glbcePath      = VPS_ISS_GLBCE_PATH_ISP;
    }
    else if (ISSM2MISP_LINK_OPMODE_12BIT_MONOCHROME == operatingMode)
    {
        /* NSF3 and GLBCE are used in the ISP path,
           NSF3 is used for noise removal and
           GLBCE is used for gamma correction */
        pPassCfg->ispPrms.glbcePath = VPS_ISS_GLBCE_PATH_ISP;
        pPassCfg->ispPrms.nsf3Path = VPS_ISS_NSF3_PATH_ISP;

        /* DPC is used before nsf to remove defective pixels */
        pPassCfg->ispPrms.enableDpcPreNsf3 = (UInt32)TRUE;

        /* None of the IPIPE modules are used in this mode */
        pPassCfg->ispPrms.enableRszInputFromIpipeif = (UInt32)TRUE;

        /* Enable the outputs based on flags */
        pPassCfg->ispPrms.enableStreams[VPS_ISS_STREAM_ID_RSZ_A] = (UInt32)rszAEnable;
        pPassCfg->ispPrms.enableStreams[VPS_ISS_STREAM_ID_RSZ_B] = (UInt32)rszBEnable;
        pPassCfg->ispPrms.enableStreams[VPS_ISS_STREAM_ID_AEWB] = (UInt32)h3aEnable;
    }
    else
    {
        UTILS_assert((Bool)0U);
    }

#ifdef ISSM2MISP_LINK_ENABLE_GLBCE_OUTPUT
    if ((((Bool)TRUE == IssM2mIspLink_isWdrMode(operatingMode)) &&
         (passId == ISSM2MISP_LINK_SECOND_PASS)) ||
        (ISSM2MISP_LINK_OPMODE_1PASS_WDR == operatingMode))
    {
        pPassCfg->ispPrms.enableRszInputFromIpipeif = (UInt32)TRUE;
    }
#endif

#ifdef ISSM2MISP_LINK_ENABLE_IPIPEIF_OUTPUT
    if((((Bool)TRUE == IssM2mIspLink_isWdrMode(operatingMode)) &&
        (passId == ISSM2MISP_LINK_SECOND_PASS)) ||
        (ISSM2MISP_LINK_OPMODE_1PASS_WDR == operatingMode))
    {
        pPassCfg->ispPrms.glbcePath      = VPS_ISS_GLBCE_PATH_DISABLED;
        pPassCfg->ispPrms.enableRszInputFromIpipeif = (UInt32)TRUE;
    }
#endif

    status = Fvid2_control(
        pPassCfg->drvHandle,
        IOCTL_VPS_ISS_M2M_SET_ISP_PARAMS,
        &pPassCfg->ispPrms,
        NULL);

    if(FVID2_SOK != status)
    {
        Vps_printf(" ISSM2MISP: ERROR: CH%d:"
                   " IOCTL_VPS_ISS_M2M_SET_ISP_PARAMS failed (Pass Id = %d)\n",
                   chId, passId);
        UTILS_assert((Bool)0U);
    }

    /* For monochroma path and 16bit WDR mode, it is required to
       shift up the input before GLBCE and shift down by 4 bits
       after GLBCE. */
    if ((ISSM2MISP_LINK_OPMODE_12BIT_MONOCHROME == operatingMode) ||
        (ISSM2MISP_LINK_OPMODE_1PASS_WDR == operatingMode))
    {
        status = Fvid2_control(
            pPassCfg->drvHandle,
            VPS_ISS_IPIPE_IOCTL_GET_VP2_CONFIG,
            &vp2Cfg,
            NULL);

        if(FVID2_SOK != status)
        {
            Vps_printf(" ISSM2MISP: ERROR: CH%d:"
                       " VPS_ISS_IPIPE_IOCTL_GET_VP2_CONFIG failed (Pass Id = %d)\n",
                       chId, passId);
            UTILS_assert((Bool)0U);
        }

        /* GLBCE input is first shifted up by 4bits and then output
           is shifted down by 4 bits using Videp Port2 */
        vp2Cfg.inMsbPos   = VPS_ISS_INPUT_MSB_POS_BIT11;
        vp2Cfg.outMsbPos  = VPS_ISS_OUTPUT_MSB_POS_BIT12;

        status = Fvid2_control(
            pPassCfg->drvHandle,
            VPS_ISS_IPIPE_IOCTL_SET_VP2_CONFIG,
            &vp2Cfg,
            NULL);

        if(FVID2_SOK != status)
        {
            Vps_printf(" ISSM2MISP: ERROR: CH%d:"
                       " VPS_ISS_IPIPE_IOCTL_SET_VP2_CONFIG failed (Pass Id = %d)\n",
                       chId, passId);
            UTILS_assert((Bool)0U);
        }
    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief API for setting parameters for certain config structures
 *
 *        There are certain config structures, which need not be exposed
 *        to App since parameters for these can be derived internally, based
 *        on certain other parameters set by the App. Deriving those
 *        parameters is handled in this function.
 *
 * \param  pObj     [IN]
 * \param  chId     [IN]
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssM2mIspLink_drvSetPassConfigParams(IssM2mIspLink_Obj *pObj,
                                        UInt32 chId,
                                        UInt32 passId)
{
    Int32 status;
    IssM2mIspLink_CreateParams *pCreatePrms;
    System_LinkChInfo *pPrevLinkChInfo;
    System_LinkChInfo *pChInfo;
    IssM2mIspLink_ChannelObject *pChObj;
    IssM2mIspLink_PassObj *pPassCfg;
    IssM2mIspLink_OperatingMode operatingMode;
    Bool rszAEnable, rszBEnable;
    UInt32 i;

    pCreatePrms = &pObj->createArgs;

    pChObj = &pObj->chObj[chId];
    pPassCfg = &pChObj->passCfg[passId];
    operatingMode = pObj->createArgs.channelParams[chId].operatingMode;

    for(i=0; i<FVID2_BAYER_COLOR_COMP_MAX; i++)
    {
        pPassCfg->ipipeWbCfg.offset[i] = 0U;
        pPassCfg->ipipeWbCfg.gain[i] = 0x200U;

        pPassCfg->isifWbCfg.gain[i] = 0x200U;
        pPassCfg->isifWbCfg.offset = 0U;
    }
    pPassCfg->isifWbCfg.gainEnable[VPS_ISS_ISIF_OUTPUT_H3A] = (UInt32)TRUE;
    pPassCfg->isifWbCfg.gainEnable[VPS_ISS_ISIF_OUTPUT_IPIPE] = (UInt32)TRUE;
    pPassCfg->isifWbCfg.gainEnable[VPS_ISS_ISIF_OUTPUT_MEMORY] = (UInt32)TRUE;

    pPassCfg->isifWbCfg.offsetEnable[VPS_ISS_ISIF_OUTPUT_H3A] = (UInt32)TRUE;
    pPassCfg->isifWbCfg.offsetEnable[VPS_ISS_ISIF_OUTPUT_IPIPE] = (UInt32)TRUE;
    pPassCfg->isifWbCfg.offsetEnable[VPS_ISS_ISIF_OUTPUT_MEMORY] = (UInt32)TRUE;

    rszAEnable = (Bool)pCreatePrms->channelParams[chId].enableOut[ISSM2MISP_LINK_OUTPUTQUE_IMAGE_RSZ_A];
    rszBEnable = (Bool)pCreatePrms->channelParams[chId].enableOut[ISSM2MISP_LINK_OUTPUTQUE_IMAGE_RSZ_B];

    pPrevLinkChInfo = &pObj->inQueInfo.chInfo[chId];

    /* Resetting resizer config to 0 so that all flags are reset */
    memset(&pPassCfg->rszCfg, 0, sizeof(pPassCfg->rszCfg));

    /* Resizer configuration */
    pChObj->rszCtrl.module = VPS_ISS_RSZ_MODULE_RSZCFG;
    pChObj->rszCtrl.rszCfg = &pPassCfg->rszCfg;

    status = Fvid2_control(
        pPassCfg->drvHandle,
        VPS_ISS_RSZ_IOCTL_GET_CONFIG,
        &pChObj->rszCtrl,
        NULL);

    UTILS_assert(FVID2_SOK == status);

    pPassCfg->rszCfg.inCfg.procWin.cropStartX = 0u;
    pPassCfg->rszCfg.inCfg.procWin.cropStartY = 0u;
    pPassCfg->rszCfg.inCfg.procWin.cropWidth  = pPrevLinkChInfo->width;
    pPassCfg->rszCfg.inCfg.procWin.cropHeight = pPrevLinkChInfo->height;

    /* For monochrome mode, resizer input is raw, but resizer will process
       only lower 8bits as if it is chroma only operation,
       so need to override resizer input format to yuv420
       only yuv420 input format supports chroma or luma only operation */
    if (ISSM2MISP_LINK_OPMODE_12BIT_MONOCHROME == operatingMode)
    {
        /* Override Input Format */
        pPassCfg->rszCfg.inCfg.overrideInCfg = (UInt32)TRUE;
        pPassCfg->rszCfg.inCfg.inDataFormat  = FVID2_DF_YUV420SP_UV;
    }

    for(i=0; i<VPS_ISS_RSZ_SCALER_MAX; i++)
    {
        if(i==0)
        {
            pPassCfg->rszCfg.instCfg[i].enable = (UInt32)rszAEnable;
        }
        else
        {
            pPassCfg->rszCfg.instCfg[i].enable = (UInt32)rszBEnable;
        }
        pPassCfg->rszCfg.instCfg[i].flipCtrl =
            VPS_ISS_RSZ_STR_MODE_NORMAL;
        pPassCfg->rszCfg.instCfg[i].startPos.startX = 0u;
        pPassCfg->rszCfg.instCfg[i].startPos.startY = 0u;

        pPassCfg->rszCfg.instCfg[i].scaleMode       =
            VPS_ISS_RSZ_SCALE_MODE_NORMAL;
        pPassCfg->rszCfg.instCfg[i].filtCfg.horzLumaFilter =
            VPS_ISS_RSZ_FILTER_4TAP_CUBIC;
        pPassCfg->rszCfg.instCfg[i].filtCfg.vertLumaFilter =
            VPS_ISS_RSZ_FILTER_4TAP_CUBIC;
        pPassCfg->rszCfg.instCfg[i].filtCfg.horzChromaFilter =
            VPS_ISS_RSZ_FILTER_4TAP_CUBIC;
        pPassCfg->rszCfg.instCfg[i].filtCfg.vertChromaFilter =
            VPS_ISS_RSZ_FILTER_4TAP_CUBIC;

        pPassCfg->rszCfg.inCfg.opMode = VPS_ISS_RSZ_OP_MODE_RESIZING;

        if(i==0)
        {
            pChInfo = &pObj->linkInfo.queInfo[ISSM2MISP_LINK_OUTPUTQUE_IMAGE_RSZ_A].chInfo[chId];
        }
        else
        {
            pChInfo = &pObj->linkInfo.queInfo[ISSM2MISP_LINK_OUTPUTQUE_IMAGE_RSZ_B].chInfo[chId];
        }

        pPassCfg->rszCfg.instCfg[i].outFmt.dataFormat = SYSTEM_LINK_CH_INFO_GET_FLAG_DATA_FORMAT(pChInfo->flags);
        pPassCfg->rszCfg.instCfg[i].outFmt.width      = pChInfo->width;
        pPassCfg->rszCfg.instCfg[i].outFmt.height     = pChInfo->height;
        pPassCfg->rszCfg.instCfg[i].outFmt.pitch[0U]  = pChInfo->pitch[0U];
        pPassCfg->rszCfg.instCfg[i].outFmt.pitch[1U]  = pChInfo->pitch[1];
        pPassCfg->rszCfg.instCfg[i].outFmt.pitch[2U]  = pChInfo->pitch[2];

        pPassCfg->rszCfg.instCfg[i].intensityCfg.horzLumaIntensity   = 0x595u;
        pPassCfg->rszCfg.instCfg[i].intensityCfg.horzChromaIntensity = 0x0u;
        pPassCfg->rszCfg.instCfg[i].intensityCfg.vertLumaIntensity   = 0x3CFu;
        pPassCfg->rszCfg.instCfg[i].intensityCfg.vertChromaIntensity = 0x0u;

        pPassCfg->rszCfg.instCfg[i].yuvRszMode =
            VPS_ISS_RSZ_YUV_RSZ_MODE_LUMA_AND_CHROMA;

        /* Select Chroma only operation for Monochrome operating mode */
        if (ISSM2MISP_LINK_OPMODE_12BIT_MONOCHROME == operatingMode)
        {
            /* Only chroma output is used for monochrome mode */
            pPassCfg->rszCfg.instCfg[i].yuvRszMode =
                VPS_ISS_RSZ_YUV_RSZ_MODE_CHROMA_ONLY;
        }
    }

    /*
     * First pass of 20-bit WDR uses resizer A in bypass mode
     * and resizer B is not used
     */
    if (passId == ISSM2MISP_LINK_FIRST_PASS)
    {
        if (operatingMode == ISSM2MISP_LINK_OPMODE_2PASS_WDR)
        {
            pPassCfg->rszCfg.inCfg.opMode = VPS_ISS_RSZ_OP_MODE_BYPASS;

            pPassCfg->rszCfg.instCfg[0].enable = (UInt32)TRUE;
            pPassCfg->rszCfg.instCfg[1].enable = (UInt32)FALSE;

            pPassCfg->rszCfg.instCfg[0U].outFmt.dataFormat = FVID2_DF_BAYER_RAW;
            pPassCfg->rszCfg.instCfg[0U].outFmt.width      = pPrevLinkChInfo->width;
            pPassCfg->rszCfg.instCfg[0U].outFmt.height     = pPrevLinkChInfo->height;
            pPassCfg->rszCfg.instCfg[0U].outFmt.pitch[0U]  = pPrevLinkChInfo->pitch[0U];
            pPassCfg->rszCfg.instCfg[0U].outFmt.pitch[1U]  = pPrevLinkChInfo->pitch[1U];
            pPassCfg->rszCfg.instCfg[0U].outFmt.pitch[2U]  = pPrevLinkChInfo->pitch[2U];
        }
    }

    if (operatingMode == ISSM2MISP_LINK_OPMODE_2PASS_WDR_LINE_INTERLEAVED)
    {
        IssM2mIspLink_WdrOffsetParams_t *wdrOffsetPrms =
            &pObj->createArgs.channelParams[chId].wdrOffsetPrms;

        /* This must be set for both the passes */
        pPassCfg->rszCfg.inCfg.procWin.cropWidth  = wdrOffsetPrms->width;
        pPassCfg->rszCfg.inCfg.procWin.cropHeight = wdrOffsetPrms->height;

        if (passId == ISSM2MISP_LINK_FIRST_PASS)
        {
            pPassCfg->rszCfg.inCfg.opMode = VPS_ISS_RSZ_OP_MODE_BYPASS;

            pPassCfg->rszCfg.instCfg[0].enable = (UInt32)TRUE;
            pPassCfg->rszCfg.instCfg[1].enable = (UInt32)FALSE;

            pPassCfg->rszCfg.instCfg[0U].outFmt.dataFormat = FVID2_DF_BAYER_RAW;


            pPassCfg->rszCfg.instCfg[0U].outFmt.width      = wdrOffsetPrms->width;
            pPassCfg->rszCfg.instCfg[0U].outFmt.height     = wdrOffsetPrms->height;

            /* For the second pass previous input, set the pitch equal
               to two times width */
            pPassCfg->rszCfg.instCfg[0U].outFmt.pitch[0U]  =
                wdrOffsetPrms->width * 2u;
            pPassCfg->rszCfg.instCfg[0U].outFmt.pitch[1U]  =
                wdrOffsetPrms->width * 2u;
            pPassCfg->rszCfg.instCfg[0U].outFmt.pitch[2U]  =
                wdrOffsetPrms->width * 2u;
        }
    }

    memset(&pPassCfg->satCfg, 0, sizeof(pPassCfg->satCfg));

    /* Saturation configuration needed only in case of WDR */
    {
        System_BitsPerPixel inBpp;
        UInt32 sat = 0U;

        pPassCfg->satCfg.vportSatCfg.enable = (UInt32)TRUE;
        pPassCfg->satCfg.vportSatCfg.dcClmp = 0;

        inBpp = pCreatePrms->channelParams[chId].inBpp;

        /* by default assume 12-bits */
        if(inBpp==SYSTEM_BPP_BITS12)
        {
            sat = (UInt32)0xFFFU;
        }
        else if(inBpp==SYSTEM_BPP_BITS10)
        {
            sat = (UInt32)0x3FFU;
        }
        else if(inBpp==SYSTEM_BPP_BITS14)
        {
            sat = (UInt32)0x3FFFU;
        }
        else
        {
            UTILS_assert((Bool)0U);
        }

        if(operatingMode == ISSM2MISP_LINK_OPMODE_2PASS_WDR)
        {
            if (passId == ISSM2MISP_LINK_FIRST_PASS)
            {
                pPassCfg->satCfg.vportSatCfg.sat = sat * pChObj->expRatio;
                pPassCfg->satCfg.vportSatCfg.dsf = pChObj->evRatio;
            }
            else
            {
                pPassCfg->satCfg.vportSatCfg.sat = (UInt32)sat;
                pPassCfg->satCfg.vportSatCfg.dsf = (UInt32)0U;
            }
        }
        else if(operatingMode == ISSM2MISP_LINK_OPMODE_1PASS_WDR)
        {
            pPassCfg->satCfg.vportSatCfg.sat = (UInt32)0xFFFU;
            pPassCfg->satCfg.vportSatCfg.dsf = (UInt32)0U;
            pPassCfg->satCfg.vportSatCfg.enable = (UInt32)FALSE;
        }
        else if((operatingMode == ISSM2MISP_LINK_OPMODE_12BIT_LINEAR) ||
                (operatingMode ==
                    ISSM2MISP_LINK_OPMODE_2PASS_WDR_LINE_INTERLEAVED) ||
                (operatingMode == ISSM2MISP_LINK_OPMODE_12BIT_MONOCHROME))
        {
            /* Saturation need to be disabled for 12bit linear mode or
                monochrome mode,
                No need to enable Saturation even for 20bit wdr line
                interleaved since input image contains short and long
                independent channels */
            pPassCfg->satCfg.vportSatCfg.enable = (UInt32)FALSE;
            pPassCfg->satCfg.vportSatCfg.sat = (UInt32)sat;
            pPassCfg->satCfg.vportSatCfg.dsf = (UInt32)0;
        }
        else
        {
            UTILS_assert((Bool)0U);
        }
    }

    /* Get Default Parameters for WB and RGB2RGB modules */
    pChObj->ipipeCtrl.module = VPS_ISS_IPIPE_MODULE_WB;
    pChObj->ipipeCtrl.wbCfg = &pPassCfg->ipipeWbCfg;
    status = Fvid2_control(
        pPassCfg->drvHandle,
        VPS_ISS_IPIPE_IOCTL_GET_CONFIG,
        &pChObj->ipipeCtrl,
        NULL);
    UTILS_assert(status == FVID2_SOK);

    pChObj->ipipeCtrl.module = VPS_ISS_IPIPE_MODULE_RGB2RGB1;
    pChObj->ipipeCtrl.rgb2RgbCfg = &pPassCfg->rgb2rgb1;
    status = Fvid2_control(
        pPassCfg->drvHandle,
        VPS_ISS_IPIPE_IOCTL_GET_CONFIG,
        &pChObj->ipipeCtrl,
        NULL);
    UTILS_assert(status == FVID2_SOK);

    pChObj->ipipeCtrl.module = VPS_ISS_IPIPE_MODULE_RGB2RGB2;
    pChObj->ipipeCtrl.rgb2RgbCfg = &pPassCfg->rgb2rgb2;
    status = Fvid2_control(
        pPassCfg->drvHandle,
        VPS_ISS_IPIPE_IOCTL_GET_CONFIG,
        &pChObj->ipipeCtrl,
        NULL);
    UTILS_assert(status == FVID2_SOK);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief This function creates the driver instance
 *
 * Copy/map all the link parameters to driver parameters. Call the appropriate
 * driver create APIs. Currently core APIs are used.
 *
 * \param  pObj     [IN] Global link object
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssM2mIspLink_drvCreateChObj(IssM2mIspLink_Obj * pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    UInt32 chId;
    UInt32 passId;
    IssM2mIspLink_OperatingMode operatingMode;

    for(chId = 0; chId<pObj->inQueInfo.numCh; chId++)
    {
        operatingMode = pObj->createArgs.channelParams[chId].operatingMode;

        passId = ISSM2MISP_LINK_FIRST_PASS;
        IssM2mIspLink_drvCreateDrv(pObj, chId, passId);
        IssM2mIspLink_drvSetDrvParams(pObj, chId, passId);
        IssM2mIspLink_drvSetPassConfigParams(pObj, chId, passId);

        if((Bool)TRUE == IssM2mIspLink_isWdrMode(operatingMode))
        {
            passId   = ISSM2MISP_LINK_SECOND_PASS;
            IssM2mIspLink_drvCreateDrv(pObj, chId, passId);
            IssM2mIspLink_drvSetDrvParams(pObj, chId, passId);
            IssM2mIspLink_drvSetPassConfigParams(pObj, chId, passId);
        }
    }

    pObj->semProcessCall = BspOsal_semCreate((Int32)0U, (Bool)TRUE);
    UTILS_assert(pObj->semProcessCall != NULL);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Create output queue related information
 *
 * \param  pObj     [IN] Global link object
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssM2mIspLink_drvCreateOutObj(IssM2mIspLink_Obj  *pObj)
{
    UInt32 queId, chId;
    Int32 status;
    System_LinkInfo *pLinkInfo;
    System_LinkChInfo *outChInfo;
    IssM2mIspLink_OutObj *pOutObj;

    /*
     * Logic common for all output queues
     */
    for (queId = 0; queId < ISSM2MISP_LINK_OUTPUTQUE_MAXNUM; queId++)
    {
        pOutObj= &pObj->linkOutObj[queId];
        status = Utils_queCreate(&pOutObj->fullBufQue,
                                  ISSM2MISP_LINK_MAX_FRAMES,
                                  pOutObj->fullBufsMem,
                                  UTILS_QUE_FLAG_NO_BLOCK_QUE);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

        for (chId=0; chId < pObj->inQueInfo.numCh; chId++)
        {
            status = Utils_queCreate(&pOutObj->emptyBufQue[chId],
                                      ISSM2MISP_LINK_MAX_FRAMES_PER_CH,
                                      pOutObj->emptyBufsMem[chId],
                                      UTILS_QUE_FLAG_NO_BLOCK_QUE);

            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
        }
    }

    /*
     * Population of link info, which will be queried by successor link
     */
    pLinkInfo = &pObj->linkInfo;
    pLinkInfo->numQue = ISSM2MISP_LINK_OUTPUTQUE_MAXNUM;

    for(queId=0; queId< pLinkInfo->numQue; queId++)
    {
        pOutObj = &pObj->linkOutObj[queId];
        pLinkInfo->queInfo[queId].numCh = pObj->inQueInfo.numCh;

        for (chId=0; chId < pObj->inQueInfo.numCh; chId++)
        {
            outChInfo = &pLinkInfo->queInfo[queId].chInfo[chId];

            outChInfo->startX = 0;
            outChInfo->startY = 0;
            if( (queId==ISSM2MISP_LINK_OUTPUTQUE_IMAGE_RSZ_A)
                ||
               (queId==ISSM2MISP_LINK_OUTPUTQUE_IMAGE_RSZ_B)
                )
            {
                IssM2mIspLink_OutputParams *pChOutParams;

                pChOutParams = &pObj->createArgs.channelParams[chId].outParams;

                if(queId==ISSM2MISP_LINK_OUTPUTQUE_IMAGE_RSZ_A)
                {
                    outChInfo->width  = pChOutParams->widthRszA;
                    outChInfo->height  = pChOutParams->heightRszA;
                }
                else
                {
                    outChInfo->width  = pChOutParams->widthRszB;
                    outChInfo->height  = pChOutParams->heightRszB;
                }

                if(pChOutParams->dataFormat == SYSTEM_DF_YUV422I_YUYV)
                {
                    outChInfo->pitch[0U] = (UInt32)SystemUtils_align(
                        outChInfo->width*2U,
                        (UInt32)SYSTEM_BUFFER_ALIGNMENT*2U);
                    outChInfo->pitch[1] = (UInt32)0;
                    outChInfo->pitch[2] = (UInt32)0;
                }
                if(pChOutParams->dataFormat == SYSTEM_DF_YUV420SP_UV)
                {
                    outChInfo->pitch[0U] = (UInt32)SystemUtils_align(outChInfo->width, (UInt32)SYSTEM_BUFFER_ALIGNMENT*2U);
                    outChInfo->pitch[1] = (UInt32)outChInfo->pitch[0U];
                    outChInfo->pitch[2] = (UInt32)outChInfo->pitch[0U];
                }

                SYSTEM_LINK_CH_INFO_SET_FLAG_DATA_FORMAT(outChInfo->flags,
                    pChOutParams->dataFormat);

                SYSTEM_LINK_CH_INFO_SET_FLAG_SCAN_FORMAT(outChInfo->flags,
                    SYSTEM_SF_PROGRESSIVE);

                SYSTEM_LINK_CH_INFO_SET_FLAG_BUF_TYPE(outChInfo->flags,
                    SYSTEM_BUFFER_TYPE_VIDEO_FRAME);
            }
            if(queId==ISSM2MISP_LINK_OUTPUTQUE_H3A)
            {
                outChInfo->width = 0;
                outChInfo->height = 0;
                outChInfo->pitch[0U] = 0;
                outChInfo->pitch[1] = 0;
                outChInfo->pitch[2] = 0;

                SYSTEM_LINK_CH_INFO_SET_FLAG_BUF_TYPE(outChInfo->flags,
                    SYSTEM_BUFFER_TYPE_METADATA);
            }
        }
    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Allocate frame buffers and do the necessary initializations
 *
 *  \param pObj   [IN] Iss capture link obj
 *
 *  \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssM2mIspLink_allocContextBuffer(IssM2mIspLink_Obj * pObj)
{
    UInt32 chId;
    Bool isGlbceReq = (Bool)FALSE, allocDmaCh = (Bool)FALSE;
    Int32 status = 0;
    IssM2mIspLink_ChannelObject *pChObj;
    IssM2mIspLink_CreateParams *pCreateParams = NULL;
    Utils_DmaChCreateParams dmaParams;

    pCreateParams = &pObj->createArgs;
    if (pObj->inQueInfo.numCh > 1U)
    {
        for (chId = 0U; chId < pObj->inQueInfo.numCh; chId ++)
        {
            pChObj = &pObj->chObj[chId];
            isGlbceReq = (Bool)FALSE;
            if (IssM2mIspLink_isWdrMode(pCreateParams->
                    channelParams[chId].operatingMode) == (Bool)TRUE)
            {
                isGlbceReq = (Bool)pChObj->passCfg[ISSM2MISP_LINK_SECOND_PASS].
                    openPrms.isModuleReq[VPS_ISS_ISP_MODULE_GLBCE];
            }
            else
            {
                isGlbceReq = (Bool)pChObj->passCfg[ISSM2MISP_LINK_FIRST_PASS].
                    openPrms.isModuleReq[VPS_ISS_ISP_MODULE_GLBCE];
            }

            allocDmaCh = isGlbceReq;
            if ((Bool)TRUE == isGlbceReq)
            {
                pChObj->pGlbceStatsBuffer =
                    Utils_memAlloc(
                        UTILS_HEAPID_DDR_CACHED_SR,
                        pObj->glbceStatsInfo.size,
                        SYSTEM_BUFFER_ALIGNMENT*2U);
                UTILS_assert(NULL != pChObj->pGlbceStatsBuffer);
            }
        }

        if ((Bool)TRUE == allocDmaCh)
        {
            /* Initialize DMA parameters and create object for Frame Dumping */
            Utils_DmaChCreateParams_Init(&dmaParams);
            status = Utils_dmaCreateCh(
                            &pObj->saveCtxDmaObj,
                            &dmaParams);
            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
        }
    }

    return (status);
}

Void IssM2mIspLink_freeContextBuffer(IssM2mIspLink_Obj * pObj)
{
    UInt32 chId;
    Bool allocatedDmaCh = (Bool)FALSE;
    Int32 status = 0;
    IssM2mIspLink_ChannelObject *pChObj;

    for (chId = 0U; chId < pObj->inQueInfo.numCh; chId ++)
    {
        pChObj = &pObj->chObj[chId];
        if (NULL != pChObj->pGlbceStatsBuffer)
        {
            status =
                Utils_memFree(
                    UTILS_HEAPID_DDR_CACHED_SR,
                    pChObj->pGlbceStatsBuffer,
                    pObj->glbceStatsInfo.size);
            UTILS_assert(status==0);

            allocatedDmaCh = (Bool)TRUE;
        }
    }
    if ((Bool)TRUE == allocatedDmaCh)
    {
        /* Free up the DMA channel object */
        Utils_dmaDeleteCh(&pObj->saveCtxDmaObj);
    }
}


/**
 *******************************************************************************
 *
 * \brief Allocate frame buffers and do the necessary initializations
 *
 *  \param pObj   [IN] Iss capture link obj
 *
 *  \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssM2mIspLink_drvAllocFrames(IssM2mIspLink_Obj * pObj)
{
    UInt32 queId, frameIdx, numFrames, maxInPitch, maxInHeight;
    UInt32 chId, temp;
    Int32 status;
    System_Buffer *pSystemBuffer;
    System_VideoFrameBuffer *pSystemVideoFrameBuffer;
    System_MetaDataBuffer *pSystemH3ABuffer;
    IssM2mIspLink_CreateParams * pCreateParams;
    IssM2mIspLink_OutObj *pOutObj;
    System_LinkChInfo *outChInfo;

    pCreateParams = &pObj->createArgs;

    maxInPitch = 0;
    maxInHeight = 0;

    for(chId = 0; chId < pObj->inQueInfo.numCh; chId++)
    {
        if (ISSM2MISP_LINK_OPMODE_2PASS_WDR ==
                pCreateParams->channelParams[chId].operatingMode)
        {
            if(pObj->inQueInfo.chInfo[chId].pitch[0U]
                >
                maxInPitch
                )
            {
                maxInPitch = pObj->inQueInfo.chInfo[chId].pitch[0U];
            }
            if(pObj->inQueInfo.chInfo[chId].height
                >
                maxInHeight
                )
            {
                maxInHeight = pObj->inQueInfo.chInfo[chId].height;
            }
        }

        if (ISSM2MISP_LINK_OPMODE_2PASS_WDR_LINE_INTERLEAVED ==
                pCreateParams->channelParams[chId].operatingMode)
        {
            if ((pCreateParams->channelParams[chId].wdrOffsetPrms.width * 2U) > (UInt32)maxInPitch)
            {
                maxInPitch = (UInt32) (pCreateParams->channelParams[chId].wdrOffsetPrms.width *2U);
            }

            if (pCreateParams->channelParams[chId].wdrOffsetPrms.height >(UInt32)maxInHeight)
            {
                maxInHeight = (UInt32)pCreateParams->channelParams[chId].wdrOffsetPrms.height;
            }
        }

        if(pCreateParams->channelParams[chId].numBuffersPerCh >
            ISSM2MISP_LINK_MAX_FRAMES_PER_CH)
        {
            pCreateParams->channelParams[chId].numBuffersPerCh =
                ISSM2MISP_LINK_MAX_FRAMES_PER_CH;
        }

        numFrames  = pCreateParams->channelParams[chId].numBuffersPerCh;

        for(queId=0; queId < ISSM2MISP_LINK_OUTPUTQUE_MAXNUM ; queId++)
        {
            pOutObj = &pObj->linkOutObj[queId];
            outChInfo = &pObj->linkInfo.queInfo[queId].chInfo[chId];

            if(pCreateParams->channelParams[chId].enableOut[queId]!=FALSE)
            {

            for(frameIdx = 0; frameIdx < numFrames; frameIdx++)
            {
                pSystemBuffer           = &pOutObj->buffers[chId][frameIdx];

                status = Utils_quePut(&pOutObj->emptyBufQue[chId],
                                    (Ptr)pSystemBuffer,
                                    (UInt32)BSP_OSAL_NO_WAIT);
                UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

                    if( (queId==ISSM2MISP_LINK_OUTPUTQUE_IMAGE_RSZ_A)
                    ||
                       (queId==ISSM2MISP_LINK_OUTPUTQUE_IMAGE_RSZ_B)
                    )
                {
                    if(queId==ISSM2MISP_LINK_OUTPUTQUE_IMAGE_RSZ_A)
                    {
                        pSystemVideoFrameBuffer
                            = &pObj->videoFramesRszA[chId][frameIdx];
                    }
                    if(queId==ISSM2MISP_LINK_OUTPUTQUE_IMAGE_RSZ_B)
                    {
                        pSystemVideoFrameBuffer
                            = &pObj->videoFramesRszB[chId][frameIdx];
                    }

                    pSystemBuffer->payload
                        = pSystemVideoFrameBuffer;
                    pSystemBuffer->payloadSize
                        = sizeof(System_VideoFrameBuffer);
                    pSystemBuffer->bufType
                        = SYSTEM_BUFFER_TYPE_VIDEO_FRAME;
                    pSystemBuffer->chNum
                        = chId;

                    pSystemVideoFrameBuffer->chInfo
                        = *outChInfo;

                    pOutObj->bufSize[chId] =
                        outChInfo->height*outChInfo->pitch[0U];
                    if(SYSTEM_LINK_CH_INFO_GET_FLAG_DATA_FORMAT(
                            outChInfo->flags) == SYSTEM_DF_YUV420SP_UV)
                    {
                        pOutObj->bufSize[chId]
                            += ((outChInfo->height/2)*outChInfo->pitch[0U]);
                    }

                    if (0U != pCreateParams->channelParams[chId].useBufSizePrmsFlag)
                    {
                        if (SYSTEM_DF_YUV422I_YUYV ==
                                pCreateParams->channelParams[chId].
                                outParams.dataFormat)
                        {
                            pOutObj->bufSize[chId] =
                                pCreateParams->channelParams[chId].
                                    maxBufHeight *
                                SystemUtils_align(pCreateParams->
                                    channelParams[chId].maxBufWidth * 2U,
                                    SYSTEM_BUFFER_ALIGNMENT*2U);
                        }
                        else if(SYSTEM_DF_YUV420SP_UV  ==
                                pCreateParams->channelParams[chId].
                                outParams.dataFormat)
                        {
                            pOutObj->bufSize[chId] =
                                (pCreateParams->channelParams[chId].
                                    maxBufHeight *
                                SystemUtils_align(pCreateParams->
                                    channelParams[chId].maxBufWidth,
                                    SYSTEM_BUFFER_ALIGNMENT*2U)) +
                                ((pCreateParams->channelParams[chId].
                                    maxBufHeight / 2U) *
                                  SystemUtils_align(pCreateParams->
                                    channelParams[chId].maxBufWidth,
                                    SYSTEM_BUFFER_ALIGNMENT*2U)) ;
                        }
                        else /* unsupported Data Format */
                        {
                            UTILS_assert((Bool)FALSE);
                        }
                    }

                    if(System_useLinkMemAllocInfo(
                            &pObj->createArgs.memAllocInfo)==FALSE)
                    {
                        pSystemVideoFrameBuffer->bufAddr[0]
                            = Utils_memAlloc(
                                UTILS_HEAPID_DDR_CACHED_SR,
                                pOutObj->bufSize[chId],
                                SYSTEM_BUFFER_ALIGNMENT*2U);
                    }
                    else
                    {
                        pSystemVideoFrameBuffer->bufAddr[0]
                            = System_allocLinkMemAllocInfo(
                                &pObj->createArgs.memAllocInfo,
                                pOutObj->bufSize[chId],
                                SYSTEM_BUFFER_ALIGNMENT*2U);
                    }
                    UTILS_assert(pSystemVideoFrameBuffer->bufAddr[0]!=NULL);

                    temp = outChInfo->height * outChInfo->pitch[0U];

                    /* MISRA.CAST.PTR_TO_INT
                     * MISRAC_2004 Rule 11.3
                     * Cast between a pointer and an integral type
                     * MISRAC_WAIVER:
                     * Pointer address is typecasted to int to calculate and to
                     * get the offset of the chroma buffer.
                     * allocated buffer address is already checked using
                     * Utils_assert
                     */
                    temp = (UInt32)pSystemVideoFrameBuffer->bufAddr[0] + temp;

                    /* MISRA.CAST.PTR_TO_INT
                     * MISRAC_2004 Rule 11.3
                     * Cast between a pointer and an integral type
                     * MISRAC_WAIVER:
                     * Calculated chroma buffer offset is assigned to
                     * chroma buffer pointer, luma buffer pointer is already
                     * checked againts null using UTILS_assert
                     */
                    pSystemVideoFrameBuffer->bufAddr[1] = (Void *)temp;
                        if (0U != pCreateParams->channelParams[chId].useBufSizePrmsFlag)
                    {
                        if (SYSTEM_DF_YUV422I_YUYV ==
                                pCreateParams->channelParams[chId].
                                outParams.dataFormat)
                        {
                            UInt32 maxHeight =  pCreateParams->channelParams[chId].maxBufHeight;
                            UInt32 maxWidth =  pCreateParams->channelParams[chId].maxBufWidth * (UInt32)2U;
                            UInt32 maxWidthAligned =  SystemUtils_align(maxWidth, SYSTEM_BUFFER_ALIGNMENT*2U);
                            UInt32 byteOffset =  maxHeight * maxWidthAligned;

                            /* MISRA.CAST.PTR_TO_INT
                             * MISRAC_2004 Rule 11.3
                             * Cast between a pointer and an integral type
                             * MISRAC_WAIVER:
                             * Pointer address is typecasted to int to calculate and to
                             * get the offset of the chroma buffer.
                             * allocated buffer address is already checked using
                             * Utils_assert
                             */
                            byteOffset = (UInt32)pSystemVideoFrameBuffer->bufAddr[0] + byteOffset;

                            /* MISRA.CAST.PTR_TO_INT
                             * MISRAC_2004 Rule 11.3
                             * Cast between a pointer and an integral type
                             * MISRAC_WAIVER:
                             * Calculated chroma buffer offset is assigned to
                             * chroma buffer pointer, luma buffer pointer is already
                             * checked againts null using UTILS_assert
                             */
                            pSystemVideoFrameBuffer->bufAddr[1] = (Void*)byteOffset;
                        }
                        else if(SYSTEM_DF_YUV420SP_UV  ==
                                pCreateParams->channelParams[chId].
                                outParams.dataFormat)
                        {
                            UInt32 maxHeight =  pCreateParams->channelParams[chId].maxBufHeight;
                            UInt32 maxWidth =  pCreateParams->channelParams[chId].maxBufWidth;
                            UInt32 maxWidthAligned =  SystemUtils_align(maxWidth, SYSTEM_BUFFER_ALIGNMENT*2U);
                            UInt32 byteOffset =  maxHeight * maxWidthAligned;

                            /* MISRA.CAST.PTR_TO_INT
                             * MISRAC_2004 Rule 11.3
                             * Cast between a pointer and an integral type
                             * MISRAC_WAIVER:
                             * Pointer address is typecasted to int to calculate and to
                             * get the offset of the chroma buffer.
                             * allocated buffer address is already checked using
                             * Utils_assert
                             */
                            byteOffset = (UInt32)pSystemVideoFrameBuffer->bufAddr[0] + byteOffset;

                            /* MISRA.CAST.PTR_TO_INT
                             * MISRAC_2004 Rule 11.3
                             * Cast between a pointer and an integral type
                             * MISRAC_WAIVER:
                             * Calculated chroma buffer offset is assigned to
                             * chroma buffer pointer, luma buffer pointer is already
                             * checked againts null using UTILS_assert
                             */
                            pSystemVideoFrameBuffer->bufAddr[1] = (Void*)byteOffset;
                        }
                        else /* unsupported data format */
                        {
                                     UTILS_assert((Bool)FALSE);
                        }
                    }

                    if (ISSM2MISP_LINK_OPMODE_12BIT_MONOCHROME ==
                        pCreateParams->channelParams[chId].operatingMode)
                    {
                        /* Resetting buffer with 0x80 so that luma
                           component can be displayed */
                        memset(
                            pSystemVideoFrameBuffer->bufAddr[0],
                            0x80,
                            pOutObj->bufSize[chId]);
                    }
                    if (pCreateParams->channelParams[chId].useBufSizePrmsFlag)
                    {
                        /* Resetting buffer with 0x80 so that luma
                           component can be displayed */
                        memset(
                            pSystemVideoFrameBuffer->bufAddr[0],
                            0x00,
                            pOutObj->bufSize[chId]);
                    }

                }
                if(queId==ISSM2MISP_LINK_OUTPUTQUE_H3A)
                {
                    IssM2mIspLink_OutputParams *pChOutParams;

                    pChOutParams
                        = &pObj->createArgs.channelParams[chId].outParams;

                    pSystemH3ABuffer = &pObj->h3aBuffer[chId][frameIdx];

                    pSystemBuffer->payload     = pSystemH3ABuffer;
                    pSystemBuffer->payloadSize = sizeof(System_MetaDataBuffer);
                    pSystemBuffer->bufType     = SYSTEM_BUFFER_TYPE_METADATA;
                    pSystemBuffer->chNum       = chId;

                    if (ISSM2MISP_LINK_OPMODE_2PASS_WDR_LINE_INTERLEAVED ==
                            pCreateParams->channelParams[chId].operatingMode)
                    {
                        pOutObj->bufSize[chId] =
                            ((pCreateParams->channelParams[chId].
                                wdrOffsetPrms.width /
                                    pChOutParams->winWidthH3a) + 1) *
                            ((pCreateParams->channelParams[chId].
                                wdrOffsetPrms.height /
                                    pChOutParams->winHeightH3a) + 1) *
                            (sizeof(IssAwebH3aOutSumModeOverlay) +
                             sizeof(IssAwebH3aOutUnsatBlkCntOverlay))
                            ;
                    }
                    else
                    {
                        pOutObj->bufSize[chId] =
                            ((pObj->inQueInfo.chInfo[chId].width /
                                    pChOutParams->winWidthH3a) + 1) *
                            ((pObj->inQueInfo.chInfo[chId].height /
                                    pChOutParams->winHeightH3a) + 1) *
                            (sizeof(IssAwebH3aOutSumModeOverlay) +
                             sizeof(IssAwebH3aOutUnsatBlkCntOverlay))
                            ;
                    }

                    pSystemH3ABuffer->metaBufSize[0] = pOutObj->bufSize[chId];
                    pSystemH3ABuffer->metaFillLength[0]
                        = pSystemH3ABuffer->metaBufSize[0];
                    pSystemH3ABuffer->numMetaDataPlanes = 1;

                    if(System_useLinkMemAllocInfo(
                            &pObj->createArgs.memAllocInfo)==FALSE)
                    {
                        pSystemH3ABuffer->bufAddr[0] = Utils_memAlloc(
                                            UTILS_HEAPID_DDR_CACHED_SR,
                                            pSystemH3ABuffer->metaBufSize[0],
                                            SYSTEM_BUFFER_ALIGNMENT*2U);
                    }
                    else
                    {
                        pSystemH3ABuffer->bufAddr[0] =
                                    System_allocLinkMemAllocInfo(
                                           &pObj->createArgs.memAllocInfo,
                                           pSystemH3ABuffer->metaBufSize[0],
                                           SYSTEM_BUFFER_ALIGNMENT*2U);
                    }
                    UTILS_assert(pSystemH3ABuffer->bufAddr[0] != NULL);

                    /*if(pCreateParams->channelParams[chId].operatingMode
                        == ISSM2MISP_LINK_OPMODE_2PASS_WDR)
                    {
                        pSystemH3ABuffer->numMetaDataPlanes = 2;

                        pSystemH3ABuffer->metaBufSize[1]
                            = pSystemH3ABuffer->metaBufSize[0];

                        pSystemH3ABuffer->metaFillLength[1]
                            = pSystemH3ABuffer->metaBufSize[1];

                        if(System_useLinkMemAllocInfo(
                                &pObj->createArgs.memAllocInfo)==FALSE)
                        {
                            pSystemH3ABuffer->bufAddr[1] = Utils_memAlloc(
                                                UTILS_HEAPID_DDR_CACHED_SR,
                                                pSystemH3ABuffer->metaBufSize[1],
                                                SYSTEM_BUFFER_ALIGNMENT*2U);
                        }
                        else
                        {
                            pSystemH3ABuffer->bufAddr[1] =
                                            System_allocLinkMemAllocInfo(
                                                &pObj->createArgs.memAllocInfo,
                                                pSystemH3ABuffer->metaBufSize[1],
                                                SYSTEM_BUFFER_ALIGNMENT*2U);
                        }
                        UTILS_assert(pSystemH3ABuffer->bufAddr[1] != NULL);
                    }*/
                }
            }
        }

        }
    }

    pObj->pIntermediateBufAddr = NULL;
    pObj->intermediateBufSize = maxInPitch*maxInHeight;
    if(pObj->intermediateBufSize)
    {
        if(System_useLinkMemAllocInfo(&pObj->createArgs.memAllocInfo)==FALSE)
        {
            pObj->pIntermediateBufAddr = Utils_memAlloc(
                                    UTILS_HEAPID_DDR_CACHED_SR,
                                    pObj->intermediateBufSize,
                                    SYSTEM_BUFFER_ALIGNMENT*2U
                                 );
        }
        else
        {
            pObj->pIntermediateBufAddr =
                               System_allocLinkMemAllocInfo(
                                    &pObj->createArgs.memAllocInfo,
                                    pObj->intermediateBufSize,
                                    SYSTEM_BUFFER_ALIGNMENT*2U
                                 );
        }
        UTILS_assert(pObj->pIntermediateBufAddr != NULL);
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Delete output queue related information
 *
 * \param  pObj     [IN] Global link object
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssM2mIspLink_drvDeleteOutObj(IssM2mIspLink_Obj  *pObj)
{
    UInt32 queId, chId;
    Int32 status;
    IssM2mIspLink_OutObj *pOutObj;

    /*
     * Logic common for all output queues
     */
    for (queId = 0; queId < ISSM2MISP_LINK_OUTPUTQUE_MAXNUM; queId++)
    {
        pOutObj= &pObj->linkOutObj[queId];

        status = Utils_queDelete(&pOutObj->fullBufQue);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

        for (chId=0; chId < pObj->inQueInfo.numCh; chId++)
        {
            status = Utils_queDelete(&pOutObj->emptyBufQue[chId]);
            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
        }
    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Free frame buffers and do the necessary initializations
 *
 *  \param pObj   [IN] Iss capture link obj
 *
 *  \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssM2mIspLink_drvFreeFrames(IssM2mIspLink_Obj * pObj)
{
    UInt32 queId, frameIdx, numFrames;
    UInt32 chId;
    Int32 status;
    System_VideoFrameBuffer *pSystemVideoFrameBuffer;
    System_MetaDataBuffer *pSystemH3ABuffer;
    IssM2mIspLink_CreateParams * pCreateParams;
    IssM2mIspLink_OutObj *pOutObj;

    pCreateParams = &pObj->createArgs;

    for(chId = 0; chId < pObj->inQueInfo.numCh; chId++)
    {
        numFrames  = pCreateParams->channelParams[chId].numBuffersPerCh;

        for(queId=0; queId < ISSM2MISP_LINK_OUTPUTQUE_MAXNUM ; queId++)
        {
            pOutObj = &pObj->linkOutObj[queId];

            if(pCreateParams->channelParams[chId].enableOut[queId]==TRUE)
            {

            for(frameIdx = 0; frameIdx < numFrames; frameIdx++)
            {
                    if( (queId==ISSM2MISP_LINK_OUTPUTQUE_IMAGE_RSZ_A)
                    ||
                       (queId==ISSM2MISP_LINK_OUTPUTQUE_IMAGE_RSZ_B)
                    )
                {
                    if(queId==ISSM2MISP_LINK_OUTPUTQUE_IMAGE_RSZ_A)
                    {
                        pSystemVideoFrameBuffer
                            = &pObj->videoFramesRszA[chId][frameIdx];
                    }
                    if(queId==ISSM2MISP_LINK_OUTPUTQUE_IMAGE_RSZ_B)
                    {
                        pSystemVideoFrameBuffer
                            = &pObj->videoFramesRszB[chId][frameIdx];
                    }

                    if(System_useLinkMemAllocInfo(
                            &pObj->createArgs.memAllocInfo)==FALSE)
                    {
                        status  = Utils_memFree(
                                UTILS_HEAPID_DDR_CACHED_SR,
                                pSystemVideoFrameBuffer->bufAddr[0],
                                pOutObj->bufSize[chId]
                                );
                        UTILS_assert(status==0);
                    }
                }
                if(queId==ISSM2MISP_LINK_OUTPUTQUE_H3A)
                {
                    pSystemH3ABuffer = &pObj->h3aBuffer[chId][frameIdx];

                    if(System_useLinkMemAllocInfo(
                            &pObj->createArgs.memAllocInfo)==FALSE)
                    {
                        status = Utils_memFree(
                                    UTILS_HEAPID_DDR_CACHED_SR,
                                    pSystemH3ABuffer->bufAddr[0],
                                    pSystemH3ABuffer->metaBufSize[0]);
                        UTILS_assert(status==0);
                    }

                    /*if(pCreateParams->channelParams[chId].operatingMode
                        == ISSM2MISP_LINK_OPMODE_2PASS_WDR)
                    {
                        if(System_useLinkMemAllocInfo(
                                &pObj->createArgs.memAllocInfo)==FALSE)
                        {
                            status = Utils_memFree(
                                    UTILS_HEAPID_DDR_CACHED_SR,
                                    pSystemH3ABuffer->bufAddr[1],
                                    pSystemH3ABuffer->metaBufSize[1]);
                            UTILS_assert(status==0);
                        }
                    }*/
                }
            }
        }
    }
        }

    if(pObj->intermediateBufSize)
    {
        if(System_useLinkMemAllocInfo(&pObj->createArgs.memAllocInfo)==FALSE)
        {
            status = Utils_memFree(
                UTILS_HEAPID_DDR_CACHED_SR,
                pObj->pIntermediateBufAddr,
                pObj->intermediateBufSize
            );
            UTILS_assert(status == 0);
        }
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief This function delete the driver instance
 *
 * \param  pObj     [IN] Global link object
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssM2mIspLink_drvDeleteChObj(IssM2mIspLink_Obj * pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    UInt32 chId;
    UInt32 passId;
    IssM2mIspLink_OperatingMode operatingMode;

    for(chId = 0; chId<pObj->inQueInfo.numCh; chId++)
    {
        operatingMode = pObj->createArgs.channelParams[chId].operatingMode;

        passId = ISSM2MISP_LINK_FIRST_PASS;

        status = Fvid2_delete(
                    pObj->chObj[chId].passCfg[passId].drvHandle,
                    NULL
                    );
        UTILS_assert(status==0);

        if((Bool)TRUE == IssM2mIspLink_isWdrMode(operatingMode))
        {
            passId   = ISSM2MISP_LINK_SECOND_PASS;

            status = Fvid2_delete(
                        pObj->chObj[chId].passCfg[passId].drvHandle,
                        NULL
                        );
            UTILS_assert(status==0);
        }
    }

    BspOsal_semDelete(&pObj->semProcessCall);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Create API for link. Link gets created using this function.
 *
 *      Handles all link creation time functionality.
 *
 * \param  pObj     [IN] Link global handle
 * \param  pPrm     [IN] Link create parameters
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssM2mIspLink_drvCreate(IssM2mIspLink_Obj          * pObj,
                              const IssM2mIspLink_CreateParams * pPrm)
{
    Int32 status;
    UInt32 inQueId;

    #ifdef SYSTEM_DEBUG_ISSM2M
    Vps_printf(" ISSM2MISP: Create in progress !!!\n");
    #endif

    UTILS_MEMLOG_USED_START();

    memcpy(&pObj->createArgs, pPrm, sizeof(IssM2mIspLink_CreateParams));

    System_resetLinkMemAllocInfo(&pObj->createArgs.memAllocInfo);

    status = System_linkGetInfo(
                    pPrm->inQueParams
                            [ISSM2MISP_LINK_INPUTQUE_RAW_IMAGE].prevLinkId,
                     &pObj->prevLinkInfo);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    inQueId = pPrm->inQueParams
                            [ISSM2MISP_LINK_INPUTQUE_RAW_IMAGE].prevLinkQueId;

    UTILS_assert( inQueId < pObj->prevLinkInfo.numQue);

    pObj->inQueInfo = pObj->prevLinkInfo.queInfo[inQueId];

    status = IssM2mIspLink_drvCreateOutObj(pObj);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    status = IssM2mIspLink_drvAllocFrames(pObj);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    status = IssM2mIspLink_drvCreateChObj(pObj);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    status = IssM2mIspLink_allocContextBuffer(pObj);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    /* Assign pointer to link stats object */
    pObj->linkStatsInfo = Utils_linkStatsCollectorAllocInst(pObj->linkId, "ISSM2MISP");
    UTILS_assert(NULL != pObj->linkStatsInfo);

    pObj->isFirstFrameRecv = (Bool)FALSE;

    System_assertLinkMemAllocOutOfMem(
        &pObj->createArgs.memAllocInfo,
        "ISSM2MISP"
        );

    UTILS_MEMLOG_USED_END(pObj->memUsed);
    UTILS_MEMLOG_PRINT("ISSM2MISP:",
                       pObj->memUsed,
                       UTILS_ARRAYSIZE(pObj->memUsed));
    #ifdef SYSTEM_DEBUG_ISSM2M
    Vps_printf(" ISSM2MISP: Create Done !!!\n");
    #endif

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Delete iss M2m isp link and driver handle.
 *
 *
 * \param  pObj         [IN] Link object
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 IssM2mIspLink_drvDelete(IssM2mIspLink_Obj * pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

#ifdef SYSTEM_DEBUG_ISSM2M
    Vps_printf(" ISSM2MISP: Delete in progress !!!\n");
#endif

    IssM2mIspLink_freeContextBuffer(pObj);

    status = IssM2mIspLink_drvDeleteChObj(pObj);
    UTILS_assert(0 == status);
    status = IssM2mIspLink_drvDeleteOutObj(pObj);
    UTILS_assert(0 == status);
    status = IssM2mIspLink_drvFreeFrames(pObj);

    /* Free up Link stats instance */
    status = Utils_linkStatsCollectorDeAllocInst(pObj->linkStatsInfo);
    UTILS_assert(0 == status);

#ifdef SYSTEM_DEBUG_ISSM2M
    Vps_printf(" ISSM2MISP: Delete Done !!!\n");
#endif

    return status;
}

