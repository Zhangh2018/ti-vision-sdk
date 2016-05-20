/*==========================================================================*/
/*      Copyright (C) 2009-2014 Texas Instruments Incorporated.             */
/*                      All Rights Reserved                                 */
/*==========================================================================*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "fileIO.h"

int32_t readFilePGM(const int8_t *infilename,
             uint8_t **image,
             uint16_t *n_rows,
             uint16_t *n_cols,
             uint16_t *bytesPerPixel,
             uint16_t *maxBitPerPixel,
             int32_t *n_maxValue)
{
    FILE *fp;
    int chr;
    int8_t buf[71];
    int32_t width, height, maxValue;
    int32_t status= 0;

    /***************************************************************************
    * Open the input image file for reading if a filename was given. If no
    * filename was provided, set fp to read from standard input.
    ***************************************************************************/
    //printf("\n\tStarting ReadFilePGM() routine.\n");fflush(stdout);
    if (infilename == NULL) fp = stdin;
    else
    {
       if ((fp = fopen((const char*)infilename, "rb")) == NULL) {
           fprintf(stderr, "Error reading the file %s in ReadFile_PGM().\n", infilename);fflush(stdout);
          printf("\nError reading the file %s in ReadFile_PGM().", infilename);
          status= -1;
          goto Exit;
       }
    }
    // printf("\n\t\tOpened file successfully in ReadFilePGM().");fflush(stdout);
    /***************************************************************************
    * Verify that the image is in PGM format, read in the number of columns
    * and rows in the image and scan past all of the header information.
    ***************************************************************************/
    fgets((char *)buf, 70, fp);
    if(strncmp((const char*)buf,"P5",2) != 0){
       printf("\nThe file %s is not in PGM format!", infilename);fflush(stdout);
       fprintf(stderr, "The file %s is not in PGM format!", infilename);
       if(fp != stdin) fclose(fp);
       status= -1;
       goto Exit;
    }
    do{ 
        fgets((char*)buf, 70, fp); 
    } while(buf[0] == '#');  /* skip all comment lines */
    sscanf((const char*)buf, "%d %d", &width, &height);
   // printf("\n\t\tRead format and image dimensions in ReadFilePGM().");fflush(stdout);
    {
        int n;

        /* Loop until we can read an integer. */

        while (fscanf(fp, "%d", &n) == 0)
        {
            /* Skip comments. */
            fscanf(fp, "#%n", &n);

            /* If we found a comment, skip all characters until EOL is reached. */
            if (n > 0)
                while(fgetc(fp) != '\n');

            /* Skip newline. */
            fscanf(fp, "\n");
        }

        maxValue = n;
    }


    //fgets(buf, 70, fp);
    //sscanf(buf, "%d", &maxValue);

    *n_cols = (uint16_t)width;
    *n_rows = (uint16_t)height;
    if (maxValue < 256)
    
        *bytesPerPixel = 1;
    else
        *bytesPerPixel = 2;

    {
        int bpp = 0;
        int val = 1;
        while (val < maxValue)
        { 
            val *= 2;
            bpp++;
        }
        *maxBitPerPixel = bpp;
    }
     *n_maxValue = maxValue;
 
    // printf("\n\t\tRead maxValue=%d in ReadFilePGM().\n", maxValue);fflush(stdout);
     do{ 
        chr = fgetc(fp); 
    } while(chr != 10);  // search for NL(0x0A), e.g., 'new line' 

    /***************************************************************************
    * Allocate memory to store the image then read the image from the file.
    ***************************************************************************/
     if (*image== NULL) {
         if(((*image) = (uint8_t *) malloc(width*height*(*bytesPerPixel))) == NULL) {
             printf("\nMemory allocation failure in ReadFilePGM().\n");fflush(stdout);
             fprintf(stderr, "Memory allocation failure in ReadFile_PGM().\n");
             if(fp != stdin) fclose(fp);
             status=1;
             goto Exit;
         }
     }
 //   printf("\n\t\tAllocated %d bytes in ReadFilePGM().\n", width*height*(*bytesPerPixel));fflush(stdout);

    // TODO: check endianess to see if we need to switch the order later
    //chr = (width*height);
    chr = fread((*image), (*bytesPerPixel), (*n_cols)*(*n_rows), fp);
   if (chr != (width*height)){
       printf("\nError reading the image data in ReadFilePGM().");fflush(stdout);
       fprintf(stderr, "Error reading the image data in ReadFile_PGM().\n");
       if(fp != stdin) fclose(fp);
       free((*image));
       status= -1;
       goto Exit;
    }
  //  printf("\n\t\tRead %d pixels in ReadFilePGM().", chr);fflush(stdout);
    if(fp != stdin) 
        fclose(fp);

   // printf("\n\tClosed file in ReadFilePGM().\n");fflush(stdout);
Exit:
    return status;
}

int32_t writeFile_PGM(const int8_t *outfilename,
              const uint8_t *image,
              uint16_t n_rows,
              uint16_t n_cols,
              const int8_t *comment,
              int32_t maxval)
{
    FILE *fp;
    int32_t status= 0;

    /***************************************************************************
    * Open the output image file for writing if a filename was given. If no
    * filename was provided, set fp to write to standard output.
    ***************************************************************************/
    if(outfilename == NULL) fp = stdout;
    else{
        if((fp = fopen((const char*)outfilename, "wb")) == NULL){
            fprintf(stderr, "Error writing the file %s in WriteFile_PGM().\n", outfilename);
            status= -1;
            goto Exit;
        }
    }

    /***************************************************************************
    * Write the header information to the PGM file.
    ***************************************************************************/
    fprintf(fp, "P5\n%d %d\n", n_cols, n_rows);
    if(comment != NULL)
        if(strlen((const char*)comment) <= 70) fprintf(fp, "# %s\n", comment);
    fprintf(fp, "%d\n", maxval);

    /***************************************************************************
    * Write the image data to the file.
    ***************************************************************************/
    if(n_rows != fwrite(image, n_cols, n_rows, fp)){
        fprintf(stderr, "Error writing the image data in WriteFile_PGM().\n");
        if(fp != stdout) fclose(fp);
        status= -1;
        goto Exit;
    }
Exit:
    if(fp != stdout) fclose(fp);
    return status;
}

int32_t writeFile_PPM(const int8_t *outfilename,
              const uint8_t *image_red,
              const uint8_t *image_grn,
              const uint8_t *image_blu,
              uint16_t n_rows,
              uint16_t n_cols,
              const int8_t *comment,
              int32_t maxval)
{
    FILE *fp;
    uint32_t size, p;
    int32_t status= 0;

    /***************************************************************************
    * Open the output image file for writing if a filename was given. If no
    * filename was provided, set fp to write to standard output.
    ***************************************************************************/
    if(outfilename == NULL) fp = stdout;
    else{
        if((fp = fopen((const char*)outfilename, "wb")) == NULL){
            fprintf(stderr, "Error writing the file %s in PPM_RESULT_writeFilePPM().\n", outfilename);
            status= -1;
            goto Exit;
        }
    }

    /***************************************************************************
    * Write the header information to the PPM file.
    ***************************************************************************/
    fprintf(fp, "P6\n%d %d\n", n_cols, n_rows);
    if (comment != NULL)
        if(strlen((const char*)comment) <= 70) fprintf(fp, "# %s\n", comment);
    fprintf(fp, "%d\n", maxval);

    /***************************************************************************
    * Write the image data to the file.
    ***************************************************************************/
    size = n_rows * n_cols;
    for(p=0;p<size;p++){       /* Write the image in pixel interleaved format. */
        fputc(image_red[p], fp);
        fputc(image_grn[p], fp);
        fputc(image_blu[p], fp);
    }

    if(fp != stdout) fclose(fp);
Exit:
    return status;
}

