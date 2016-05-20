/* ======================================================================
 *   Copyright (C) 2013 Texas Instruments Incorporated
 *
 *   All rights reserved. Property of Texas Instruments Incorporated.
 *   Restricted rights to use, duplicate or disclose this code are
 *   granted through contract.
 *
 *   The program may not be used without the written permission
 *   of Texas Instruments Incorporated or against the terms and conditions
 *   stipulated in the agreement under which this program has been
 *   supplied.
 * ==================================================================== */
/**
 *   Component:             SBL_TOOLS
 *
 *   Filename:               MulticoreImageGen.c
 *
 *   Description:           Generate multicore app image from the input ELF or
 *                          COFF files. Internally convert ELF or COFF
 *                          executables into RPRC files using Out2rprc tool.
 *                          Merge all the RPRC files with metaheader & generate
 *                          multicore app image.
 */

/****************************************************************
 *  INCLUDE FILES
 ****************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* ============================================================================
 * GLOBAL VARIABLES DECLARATIONS
 * =============================================================================
 */

/* ============================================================================
 * LOCAL VARIABLES DECLARATIONS
 * =============================================================================
 */
#define MAX_INPUT_FILES 10
#define debug_print(x) printf(x)

typedef struct _meta_header_start_
{
    char     magic_string_str[4];
    uint32_t num_files;
    uint32_t dev_id;
    uint32_t multicore_hdr_crc_h;
    uint32_t multicore_hdr_crc_l;
    uint32_t multicore_image_size;
    uint32_t rsvd;
}meta_header_start;

typedef struct _meta_header_core_
{
    uint32_t is_core_image_valid;
    uint32_t core_id;
    uint32_t image_offset;
    uint32_t core_image_crc_h;
    uint32_t core_image_crc_l;
    uint32_t image_size;
}meta_header_core;

typedef struct _meta_header_end_
{
    uint32_t rsvd;
    char     magic_string_end[4];
}meta_header_end;

typedef struct _input_file_core_
{
    uint32_t CoreId;
    FILE    *in_fp;
}input_file_core;

/* ============================================================================
 * LOCAL FUNCTIONS PROTOTYPES
 * =============================================================================
 */

static uint32_t tiimage_swap32(uint32_t data);

/* ============================================================================
 * FUNCTIONS
 * =============================================================================
 */
static uint32_t tiimage_swap32(uint32_t data)
{
    uint32_t result = 0;

    result  = (data & 0xFF000000) >> 24;
    result |= (data & 0x00FF0000) >> 8;
    result |= (data & 0x0000FF00) << 8;
    result |= (data & 0x000000FF) << 24;

    return result;
}

int32_t main(int32_t argc, char *argv[])
{
    FILE             *out_fp;
    input_file_core   in_fp_cr[MAX_INPUT_FILES];
    int32_t           image_size[MAX_INPUT_FILES];
    meta_header_start hdr_str;
    meta_header_core  hdr_core[MAX_INPUT_FILES];
    meta_header_end   hdr_end;
    int32_t           i = 0, j = 0, len;
    char             *boot;
    int32_t           num_input_files;
    int32_t          *swap_ptr;
    uint32_t          multicore_img_size;

    if (argc < 6)
    {
        printf("Usage : \n");
        printf(
            "Single image create takes the rprc images and adds the Meta Header  and creates single output image\n");
        printf("The resulting output is placed in the output image path\n");
        printf(
            "Syntax: ./<executable file name> <ENDIANESS> <Dev_ID> <output image path/name> <Core_ID> <input image1 path/name> [<Core_ID> <input image2 path/name>, ...]\n");
        printf(
            "ENDIAN: BE/LE --> specifies whether TI header is in Big or Little Endian format\n");
        printf("Dev_ID is the Device ID \n");
        printf(
            "For Input rprc images provide the Core_Id followed by input image name\n");
        return -1;
    }

    num_input_files = (argc - 3) / 2;
    printf("Number of Input Files %d\n", num_input_files);

    out_fp = fopen(argv[3], "wb+");
    if (!out_fp)
    {
        printf("Error opening/creating out image file!\n");
        return -1;
    }
    for (i = 0; i < num_input_files; i++)
    {
        in_fp_cr[i].CoreId = atoi(argv[(2 * i) + 4]);
        in_fp_cr[i].in_fp  = fopen(argv[(2 * i) + 5], "rb");
        if (!(in_fp_cr[i].in_fp))
        {
            printf("Error opening input image file! %s\n", argv[(2 * i) + 5]);
            // return -1;
            --num_input_files;
        }
    }
    if (num_input_files == 0)
    {
        return -1;
    }

    /* Calcualte the size of the input image and rewind to the begin of file */
    for (i = 0; i < num_input_files; i++)
    {
        fseek(in_fp_cr[i].in_fp, 0, SEEK_END);
        image_size[i] = ftell(in_fp_cr[i].in_fp);
        rewind(in_fp_cr[i].in_fp);
    }

    /* Populate Meta Header start structure */
    hdr_str.magic_string_str[0] = 'M';
    hdr_str.magic_string_str[1] = 'S';
    hdr_str.magic_string_str[2] = 'T';
    hdr_str.magic_string_str[3] = 'R';

    hdr_str.num_files = num_input_files;
    hdr_str.dev_id    = atoi(argv[2]);

    hdr_str.multicore_hdr_crc_h  = 0;
    hdr_str.multicore_hdr_crc_l  = 0;
    hdr_str.multicore_image_size = 0;
    hdr_str.rsvd = 0;

    /* Populate Meta Header Core structure */
    for (i = 0; i < num_input_files; i++)
    {
        hdr_core[i].is_core_image_valid = 1;
        hdr_core[i].core_id          = in_fp_cr[i].CoreId;
        hdr_core[i].image_size       = image_size[i];
        hdr_core[i].core_image_crc_h = 0;
        hdr_core[i].core_image_crc_l = 0;

        if (i == 0)
        {
            /* This is first input file offset is equal to length of Meta Header
            **/
            hdr_core[i].image_offset = sizeof (meta_header_start) +
                                       sizeof (meta_header_end) +
                                       (num_input_files *
                                        sizeof (meta_header_core));
        }
        else
        {
            /* This is second or subsequent file, Offset is equal to offset of
             * previous file + size of previous file
             */
            hdr_core[i].image_offset =
                hdr_core[i - 1].image_offset + image_size[i - 1];
        }
    }

    /* Populate Meta Header End structure */
    hdr_end.rsvd = 0;
    hdr_end.magic_string_end[0] = 'M';
    hdr_end.magic_string_end[1] = 'E';
    hdr_end.magic_string_end[2] = 'N';
    hdr_end.magic_string_end[3] = 'D';

    if (0 == strcmp(argv[1], "BE"))
    {
        swap_ptr = (int32_t *) &hdr_str;
        for (i = 0; i < sizeof (hdr_str) / 4; i++)
        {
            *swap_ptr = tiimage_swap32(*swap_ptr);
            swap_ptr++;
        }
        swap_ptr = (int32_t *) &hdr_end;
        for (i = 0; i < sizeof (hdr_end) / 4; i++)
        {
            *swap_ptr = tiimage_swap32(*swap_ptr);
            swap_ptr++;
        }
        for (j = 0; j < num_input_files; j++)
        {
            swap_ptr = (int32_t *) &(hdr_core[j]);
            for (i = 0; i < sizeof (meta_header_core) / 4; i++)
            {
                *swap_ptr = tiimage_swap32(*swap_ptr);
                swap_ptr++;
            }
        }
    }
    /* Insert Meta Header Start */
    fwrite(&hdr_str, sizeof (hdr_str), 1, out_fp);

    /* Insert all core info in Meta Header */
    for (i = 0; i < num_input_files; i++)
    {
        fwrite(&hdr_core[i], sizeof (meta_header_core), 1, out_fp);
    }

    /* Insert Meta Header End */
    fwrite(&hdr_end, sizeof (hdr_end), 1, out_fp);

    /* Insert All the the actual image */
    for (i = 0; i < num_input_files; i++)
    {
        for (j = 0; j < (image_size[i] / 4); j++)
        {
            uint32_t temp;
            fread(&temp, sizeof (temp), 1, in_fp_cr[i].in_fp);

            if (0 == strcmp(argv[1], "BE"))
            {
                temp = tiimage_swap32(temp);
            }

            fwrite(&temp, sizeof (temp), 1, out_fp);
        }
    }

    /* Insert File Size */
    fseek(out_fp, 0, SEEK_END);
    multicore_img_size = ftell(out_fp);
    fseek(out_fp, 0, SEEK_SET);
    hdr_str.multicore_image_size = multicore_img_size;

    if (0 == strcmp(argv[1], "BE"))
    {
        swap_ptr  = (int32_t *) &hdr_str.multicore_image_size;
        *swap_ptr = tiimage_swap32(*swap_ptr);
    }
    /* Insert Updated Meta Header Start */
    fwrite(&hdr_str, sizeof (hdr_str), 1, out_fp);

    printf("\n");
    return 0;
}

