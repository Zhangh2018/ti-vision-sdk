/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
  ******************************************************************************
 * \file grpxSrcLink_sved_layout.c
 *
 * \brief  This file has the implementation of GRPX layout for Surround view
 *         + Edge detect demo
 *
 * \version 0.0 (Oct 2013) : [NN] First version
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "grpxSrcLink_priv.h"


/**
 *******************************************************************************
 *
 * \brief Background Color of the Graphics Buffer
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SVED_BACKGROUND_COLOR ((UInt16)(RGB888_TO_RGB565(16,16,16)))

#define SVED_FRAME_THICKNESS  (10)


Int32 GrpxSrcLink_drawSurroundViewEdgeDetectLayout(GrpxSrcLink_Obj *pObj)
{
    Draw2D_RegionPrm region;
    Draw2D_BmpPrm bmpPrm;
    Draw2D_FontPrm fontPrm;

    /* fill full buffer with background color */
    region.color  = SVED_BACKGROUND_COLOR;
    region.colorFormat = SYSTEM_DF_BGR16_565;
    region.startX = 0;
    region.startY = 0;
    region.height = pObj->info.queInfo[0].chInfo[0].height;
    region.width  = pObj->info.queInfo[0].chInfo[0].width;

    Draw2D_fillRegion(pObj->draw2DHndl,&region);

    /* fill transprenecy color in portions where video should be visible */

    /* Live camera 1 */
    region.color  = DRAW2D_TRANSPARENT_COLOR;
    region.colorFormat = DRAW2D_TRANSPARENT_COLOR_FORMAT;
    region.startX = 25;
    region.startY = (100 + 35+(180+35)*0 + 5);
    region.height = 175 - 5;
    region.width  = 320;

    Draw2D_fillRegion(pObj->draw2DHndl,&region);

    /* Live camera 2 */
    region.color  = DRAW2D_TRANSPARENT_COLOR;
    region.colorFormat = DRAW2D_TRANSPARENT_COLOR_FORMAT;
    region.startX = 25;
    region.startY = (100 + 35+(180+35)*1 + 5);
    region.height = 175 - 5;
    region.width  = 320;

    Draw2D_fillRegion(pObj->draw2DHndl,&region);

    /* Live Camera 3 */
    region.color  = DRAW2D_TRANSPARENT_COLOR;
    region.colorFormat = DRAW2D_TRANSPARENT_COLOR_FORMAT;
    region.startX = 25;
    region.startY = (100 + 35+(180+35)*2 + 5);
    region.height = 175 - 5;
    region.width  = 320;

    Draw2D_fillRegion(pObj->draw2DHndl,&region);

    /* Live Camera 4 */
    region.color  = DRAW2D_TRANSPARENT_COLOR;
    region.colorFormat = DRAW2D_TRANSPARENT_COLOR_FORMAT;
    region.startX = 25;
    region.startY = (100 + 35+(180+35)*3 + 5);
    region.height = 175 - 5;
    region.width  = 320;

    Draw2D_fillRegion(pObj->draw2DHndl,&region);

    /* Surround view video */
    region.color  = DRAW2D_TRANSPARENT_COLOR;
    region.colorFormat = DRAW2D_TRANSPARENT_COLOR_FORMAT;
    region.startX = 25+320+SVED_FRAME_THICKNESS;
    region.startY = 0 + 5;
    region.height = pObj->info.queInfo[0].chInfo[0].height - 10;
    region.width  = 880;

    Draw2D_fillRegion(pObj->draw2DHndl,&region);

    /* Live Camera 5 - for analytics input */
    region.color  = DRAW2D_TRANSPARENT_COLOR;
    region.colorFormat = DRAW2D_TRANSPARENT_COLOR_FORMAT;
    region.startX = pObj->info.queInfo[0].chInfo[0].width - 25 - 640;
    region.startY = 100+SVED_FRAME_THICKNESS + 40;
    region.height = 355;
    region.width  = 640;

    Draw2D_fillRegion(pObj->draw2DHndl,&region);

    /* Analytics output */
    region.color  = DRAW2D_TRANSPARENT_COLOR;
    region.colorFormat = DRAW2D_TRANSPARENT_COLOR_FORMAT;
    region.startX = pObj->info.queInfo[0].chInfo[0].width - 25 - 640;
    region.startY = 100+SVED_FRAME_THICKNESS+40+360+SVED_FRAME_THICKNESS+40;
    region.height = 340;
    region.width  = 640;

    Draw2D_fillRegion(pObj->draw2DHndl,&region);

    /* draw bitmap's */

    /* TI logo */
    bmpPrm.bmpIdx = DRAW2D_BMP_IDX_TI_LOGO_1;
    Draw2D_drawBmp(pObj->draw2DHndl,
                   25,
                   SVED_FRAME_THICKNESS,
                   &bmpPrm
                   );

    /* Front cam analytics logo */
    bmpPrm.bmpIdx = DRAW2D_BMP_IDX_FRONT_CAM_ANALYTICS;
    Draw2D_drawBmp(pObj->draw2DHndl,
                   pObj->info.queInfo[0].chInfo[0].width - 685,
                   0,
                   &bmpPrm
                   );

    /* Surround view logo */
    bmpPrm.bmpIdx = DRAW2D_BMP_IDX_SURROUND_VIEW;
    Draw2D_drawBmp(pObj->draw2DHndl,
                   25+320+SVED_FRAME_THICKNESS+80,
                   0,
                   &bmpPrm
                   );

    if (pObj->createArgs.enableJeepOverlay == TRUE)
    {
        /* Jeep Image */
        /* TODO : Change Co-ordinates as per the requirement */
        bmpPrm.bmpIdx = DRAW2D_BMP_IDX_JEEP_IMAGE_TRUESCALE;
        Draw2D_drawBmp(pObj->draw2DHndl,
                    (326+355), //355 comes from layout
                    (324+5), //5 comes from layout
                    &bmpPrm
                    );
    }

    if (pObj->createArgs.surroundViewDOFLayoutEnable)
    {
        /* DOF LUT */
        bmpPrm.bmpIdx = DRAW2D_BMP_IDX_OPTFLOW_LUT_1;
        Draw2D_drawBmp(pObj->draw2DHndl,
                    (1920-25-640),
                    pObj->info.queInfo[0].chInfo[0].height - (129 + 25)- (100+50),
                    &bmpPrm
                    );
    }

    /* String for Live Camera 1 */
    fontPrm.fontIdx = 5;
    Draw2D_drawString(pObj->draw2DHndl,
          25+40,
          (100 + 5+(180+35)*0),
          "  FRONT CAMERA  ",
          &fontPrm
          );

    /* String for Live Camera 2 */
    fontPrm.fontIdx = 5;
    Draw2D_drawString(pObj->draw2DHndl,
          25+40,
          (100 + 5+(180+35)*1),
          "  RIGHT CAMERA  ",
          &fontPrm
          );

    /* String for Live Camera 3 */
    fontPrm.fontIdx = 5;
    Draw2D_drawString(pObj->draw2DHndl,
          25+40,
          (100 + 5+(180+35)*2),
          "   REAR CAMERA  ",
          &fontPrm
          );

    /* String for Live Camera 4 */
    fontPrm.fontIdx = 5;
    Draw2D_drawString(pObj->draw2DHndl,
          25+40,
          (100 + 5+(180+35)*3),
          "   LEFT CAMERA  ",
          &fontPrm
          );

    /* String for Analytics Camera  */
    fontPrm.fontIdx = 4;
    if (pObj->createArgs.surroundViewEdgeDetectLayoutEnable &&
        ~pObj->createArgs.surroundViewDOFLayoutEnable &&
        pObj->createArgs.surroundViewPdTsrLayoutEnable &&
        ~pObj->createArgs.surroundViewLdLayoutEnable)
    {
        Draw2D_drawString(pObj->draw2DHndl,
              pObj->info.queInfo[0].chInfo[0].width - 25 - 640 + 160,
              100+SVED_FRAME_THICKNESS,
              "PEDESTRIAN DETECTION",
              &fontPrm
              );
    }
    else if (pObj->createArgs.surroundViewEdgeDetectLayoutEnable ||
        pObj->createArgs.surroundViewDOFLayoutEnable
    )
    {
        Draw2D_drawString(pObj->draw2DHndl,
              pObj->info.queInfo[0].chInfo[0].width - 25 - 640 + 160,
              100+SVED_FRAME_THICKNESS,
              "ANALYTICS CAMERA",
              &fontPrm
              );
    }
    else if (pObj->createArgs.surroundViewPdTsrLayoutEnable &&
                 pObj->createArgs.surroundViewLdLayoutEnable)
    {
        Draw2D_drawString(pObj->draw2DHndl,
              pObj->info.queInfo[0].chInfo[0].width - 25 - 640 + 160,
              100+SVED_FRAME_THICKNESS,
                  "     PD + TSR",
              &fontPrm
              );
    }


    /* String for Edge detect output */
    fontPrm.fontIdx = 4;
    if (pObj->createArgs.surroundViewEdgeDetectLayoutEnable &&
        ~pObj->createArgs.surroundViewDOFLayoutEnable &&
        pObj->createArgs.surroundViewPdTsrLayoutEnable &&
        ~pObj->createArgs.surroundViewLdLayoutEnable)
    {
        Draw2D_drawString(pObj->draw2DHndl,
              pObj->info.queInfo[0].chInfo[0].width - 25 - 640 + 160 -40,
              100+SVED_FRAME_THICKNESS+360+40+SVED_FRAME_THICKNESS ,
              "TRAFFIC SIGN RECOGNITION",
              &fontPrm
              );
    }
    else if (pObj->createArgs.surroundViewDOFLayoutEnable)
    {
        Draw2D_drawString(pObj->draw2DHndl,
              pObj->info.queInfo[0].chInfo[0].width - 25 - 640 + 160,
              100+SVED_FRAME_THICKNESS+40+360+SVED_FRAME_THICKNESS,
              "DENSE OPTICAL FLOW",
              &fontPrm
              );
    }
    else if (pObj->createArgs.surroundViewEdgeDetectLayoutEnable)
    {
        Draw2D_drawString(pObj->draw2DHndl,
              pObj->info.queInfo[0].chInfo[0].width - 25 - 640 + 160,
              100+SVED_FRAME_THICKNESS+40+360+SVED_FRAME_THICKNESS,
              " EDGE DETECTION ",
              &fontPrm
              );
    }
    else if (pObj->createArgs.surroundViewPdTsrLayoutEnable &&
             pObj->createArgs.surroundViewLdLayoutEnable
    )
    {
        Draw2D_drawString(pObj->draw2DHndl,
              pObj->info.queInfo[0].chInfo[0].width - 25 - 640 + 160 -40,
              100+SVED_FRAME_THICKNESS+360+40+SVED_FRAME_THICKNESS ,
              "     LANE DETECTION     ",
              &fontPrm
              );
    }


    /* String for input resolution / frame-rate etc */
    fontPrm.fontIdx = 5;
    Draw2D_drawString(pObj->draw2DHndl,
          25,
          pObj->info.queInfo[0].chInfo[0].height - 120 + 15,
          "RESOLUTION: 1280x720",
          &fontPrm
          );

    fontPrm.fontIdx = 5;
    Draw2D_drawString(pObj->draw2DHndl,
          25,
          pObj->info.queInfo[0].chInfo[0].height - 120 + 15 + 30,
          "FRAME-RATE: 30fps   ",
          &fontPrm
          );

    fontPrm.fontIdx = 5;
    Draw2D_drawString(pObj->draw2DHndl,
          25,
          pObj->info.queInfo[0].chInfo[0].height - 120 + 15 + 30*2,
          "   NETWORK: FPDLink ",
          &fontPrm
          );

    /* String for output resolution */
    fontPrm.fontIdx = 4;
    Draw2D_drawString(pObj->draw2DHndl,
          25+320+SVED_FRAME_THICKNESS + 880 - 430,
          pObj->info.queInfo[0].chInfo[0].height - 120 + 15 + 30,
          "RESOLUTION: 880x1080",
          &fontPrm
          );


    return SYSTEM_LINK_STATUS_SOK;
}

Void GrpxSrcLink_displaySurroundViewEdgeDrawCpuLoadBar(
                    GrpxSrcLink_Obj *pObj,
                    UInt32 cpuLoadInt,
                    UInt32 cpuLoadFract,
                    UInt32 startX,
                    UInt32 startY,
                    UInt32 width,
                    UInt32 height
                )
{
    Draw2D_RegionPrm region;
    UInt32 color[2];
    UInt32 barHeight[2];

    color[0] = RGB888_TO_RGB565(40, 40, 40);
    color[1] = RGB888_TO_RGB565(0, 160, 0);

    if(cpuLoadFract>=5)
        cpuLoadInt++;

    barHeight[0] = (height * (100 - cpuLoadInt))/100;

    if(barHeight[0] > height)
        barHeight[0] = height;

    barHeight[1] = height - barHeight[0];

    /* fill in in active load color */
    region.color  = color[0];
    region.colorFormat = SYSTEM_DF_BGR16_565;
    region.startX = startX;
    region.startY = startY;
    region.height = barHeight[0];
    region.width  = width;

    Draw2D_fillRegion(pObj->draw2DHndl,&region);

    /* fill active load color */
    region.color  = color[1];
    region.colorFormat = SYSTEM_DF_BGR16_565;
    region.startX = startX;
    region.startY = startY + barHeight[0];
    region.height = barHeight[1];
    region.width  = width;

    Draw2D_fillRegion(pObj->draw2DHndl,&region);

}

Int32 GrpxSrcLink_displaySurroundViewEdgeDetectStats(GrpxSrcLink_Obj *pObj)
{
    Draw2D_FontPrm fontPrm;
    UInt32 procId;
    Utils_SystemLoadStats *loadStats;
    char loadString[GRPX_SRC_LINK_STR_SZ];
    Draw2D_FontProperty fontProp;
    UInt32 startX, startY, startX1, startY1;
    UInt32 statsHeight;

    fontPrm.fontIdx = 3;

    Draw2D_getFontProperty(&fontPrm, &fontProp);

    statsHeight = pObj->info.queInfo[0].chInfo[0].height - 900;
    startY =  pObj->info.queInfo[0].chInfo[0].height - fontProp.height;;
    startX =  pObj->info.queInfo[0].chInfo[0].width - 640;
    startX1 = pObj->info.queInfo[0].chInfo[0].width - 640;
    startY1 = 900 + SVED_FRAME_THICKNESS;


    for (procId = 0; procId < SYSTEM_PROC_MAX; procId++)
    {
            loadStats = &pObj->statsDisplayObj.systemLoadStats[procId];

            if(Bsp_platformIsTda3xxFamilyBuild()
                &&
                (procId == SYSTEM_PROC_A15_0
                    ||
                 procId == SYSTEM_PROC_EVE2
                    ||
                 procId == SYSTEM_PROC_EVE3
                    ||
                 procId == SYSTEM_PROC_EVE4
                )
              )
            {
                /* These CPUs dont exist in TDA3xx */
                continue;
            }

            snprintf(loadString, GRPX_SRC_LINK_STR_SZ,
                          "%-4s\n",
                          System_getProcName(procId)
                          );

            if (SYSTEM_PROC_IPU1_0 == procId)
            {
               snprintf(loadString, GRPX_SRC_LINK_STR_SZ,
                          "%-4s\n",
                          "M4-0"
                          );
            }
            else if (SYSTEM_PROC_IPU1_1 == procId)
            {
               snprintf(loadString, GRPX_SRC_LINK_STR_SZ,
                          "%-4s\n",
                          "M4-1"
                          );
            }
            else if (SYSTEM_PROC_A15_0 == procId)
            {
               snprintf(loadString, GRPX_SRC_LINK_STR_SZ,
                          "%-4s\n",
                          "A15"
                          );
            }
            else
            {
                /*
                 * Avoid MISRA C Warnings
                 */
            }

            /* draw CPU name */
            Draw2D_clearString(pObj->draw2DHndl,
                      startX,
                      startY,
                      strlen(loadString),
                      &fontPrm
                      );

            Draw2D_drawString(pObj->draw2DHndl,
                      startX,
                      startY,
                      loadString,
                      &fontPrm
                      );

            GrpxSrcLink_displaySurroundViewEdgeDrawCpuLoadBar
                (
                    pObj,
                    loadStats->totalLoadParams.integerValue, /* CPU load integer value */
                    loadStats->totalLoadParams.fractionalValue, /* CPU load integer value */
                    startX,
                    pObj->info.queInfo[0].chInfo[0].height - statsHeight + fontProp.height + SVED_FRAME_THICKNESS*2,
                    30,
                    (statsHeight - SVED_FRAME_THICKNESS) - (fontProp.height + SVED_FRAME_THICKNESS)*2
                );

            /* draw CPU load as text */
            snprintf(loadString, GRPX_SRC_LINK_STR_SZ,
                          "%02d.%d%%\n",
                          loadStats->totalLoadParams.integerValue,
                          loadStats->totalLoadParams.fractionalValue
                          );
            Draw2D_clearString(pObj->draw2DHndl,
                      startX1,
                      startY1,
                      strlen(loadString),
                      &fontPrm
                      );

            Draw2D_drawString(pObj->draw2DHndl,
                      startX1,
                      startY1,
                      loadString,
                      &fontPrm
                      );
           startX1 = startX1 + fontProp.width*6 + 10;
           startX = startX+fontProp.width*6 + 10;
    }
    return 0;
}
