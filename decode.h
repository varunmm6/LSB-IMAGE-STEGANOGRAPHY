#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "types.h"

/* Structure to store information required for decoding secret data from image */
typedef struct _DecodeInfo
{
    char *output_fname;
    FILE *fptr_output;

    /* Stego Image info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

    /* Decoded secret file info */
    char extn_secret_file[5];
    char decoded_fname[100];
    FILE *fptr_decoded_file;
    long size_secret_file;

} DecodeInfo;

/* Read and validate decoding arguments from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Open the stego image and prepare the decoded output file */
Status open_decode_files(DecodeInfo *decInfo);

/* Perform the complete decoding procedure */
Status do_decoding(DecodeInfo *decInfo);

/* Decode and validate the magic string signature */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo);

/* Decode the secret file extension size */
Status decode_secret_file_extn_size(int *size, DecodeInfo *decInfo);

/* Decode the secret file extension */
Status decode_secret_file_extn(int size, DecodeInfo *decInfo);

/* Decode the secret file size */
Status decode_secret_file_size(long *file_size, DecodeInfo *decInfo);

/* Decode the secret file data and write it to the output file */
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Decode one byte from the least significant bits of image data */
Status decode_byte_from_lsb(char *data, FILE *fptr_stego_image);

/* Decode an integer size from the least significant bits of image data */
Status decode_size_from_lsb(int *size, FILE *fptr_stego_image);

#endif
