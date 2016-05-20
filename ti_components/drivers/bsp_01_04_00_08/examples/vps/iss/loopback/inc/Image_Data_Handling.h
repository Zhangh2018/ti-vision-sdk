#ifndef IMAGE_DATA_HANDLING_H_
#define IMAGE_DATA_HANDLING_H_

/************************************************************************************//**
* TI ISP Library Refernce Code
* This library is a software module developed for simulation of vision and ISP
* algorithms ported on TI's embedded platforms
* Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
* @file name
* @brief        This header file provides necessary functions to handle and process image data
* @author       Hrushikesh Garud (A0393878)
* @version      1.0
*
****************************************************************************************/

/******************************************************************************//*
* STRUCTURE: Image,
* FIELDS:
*        number of rows in the image (height)
*        number of columns in the image (width),
*        bits per pixel (8/24/32),
*        resolution (pixels/meter)
*        integer pointer to the red channel data (Present for 24/32 bit images)
*        integer pointer to the green channel data (Present for 24/32 bit images)
*        integer pointer to the blue channel data (Present for 24/32 bit images)
*        integer pointer to the alpha channel data (Present for 8/32 bit images)
*        Size of data array [rows*columns*sizeof(int)]
*        Define variable of type Image for image data handling and processing
********************************************************************************/

typedef struct {
    int  rows;
    int  cols;
    int  bpp;
    int  res;
    int *rdata;
    int *gdata;
    int *bdata;
    int *adata;
}Image; // Image data structure

typedef struct {
    int table[256];
}lookUpTable;  // Data structure to store the lookup tables used in image remapping

/***************************************************************************//**
* FUNCTION: int IDH_freeImage(Image *ptr);
* Frees all the contents in the structure pointed to by ptr
* [RET]     1 on success 0 otherwise
*******************************************************************************/
int IDH_freeImage(Image *ptr);

/***************************************************************************//**
* FUNCTION:  int IDH_createImage(Image *ptr, int rows, int columns, int bpp, int overwrite);
* Creates blank image (Height=rows, Width=columns, Bits Per Pixel=bpp)in Image
* structure pointed to by ptr.
* If overwrite option is enabled existing data(if any) is lost.
* [Out]     ptr         Image structure with appropriate memmories allocations
* [IN]      rows         number of rows
* [IN]      columns      number of columns
* [IN]      bpp          Bits per pixel (8 = grayscale, 24 = rgb 8bits/channel, 32 =  rgba 8bits/channel)
* [IN]      overwrite    Overwrite? (1 = yes, 0 = no)
* [RET]     -1 if can not IDH_freeImage fails.
*           0 if overwrite==0 and bpp!=0 (i.e. bpp==8||24||32)
*           1 on successful operation
*******************************************************************************/
int IDH_createImage(Image *ptr, int rows, int columns, int bpp, int overwrite);

/***************************************************************************//**
* FUNCTION:  int IDH_createImage(Image *ptr, int rows, int columns, int bpp, int overwrite);
* Creates blank image (Height=rows, Width=columns, Bits Per Pixel=bpp)in Image
* structure pointed to by ptr.
* If overwrite option is enabled existing data(if any) is lost.
* [IN/Out]  ptr         Image structure with appropriate memmories allocations
* [IN]      op          Channel which needs to be remapped
* [IN]      LUT         Lookup table for remapping
* [RET]     -1 if can not IDH_freeImage fails.
*           0 if overwrite==0 and bpp!=0 (i.e. bpp==8||24||32)
*           1 on successful operation
*******************************************************************************/
int IDH_remapFromLUT(Image *ptr, char op, lookUpTable *LUT);

# endif
