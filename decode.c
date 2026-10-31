#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decode.h"
#include "common.h"

/* Function Definitions */

/* Read and validate decoding arguments from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    /* Check that the stego image argument is present */
    if (argv[2] == NULL)
    {
        printf("ERROR: Missing stego image file name\n");
        return e_failure;
    }

    /* Checking that the supplied image has the BMP extension */
    const char *dot = strrchr(argv[2], '.');
    if (dot == NULL || strcmp(dot, ".bmp") != 0)
    {
        printf("ERROR: Stego image must have .bmp extension\n");
        return e_failure;
    }

    /* Storeing the stego image file name */
    decInfo->stego_image_fname = argv[2];

    /* Seting the output filename provided by the user or use the default */
    if (argv[3] != NULL)
    {
        strncpy(decInfo->decoded_fname, argv[3],
                sizeof(decInfo->decoded_fname) - 1);
        decInfo->decoded_fname[sizeof(decInfo->decoded_fname) - 1] = '\0';
    }
    else
    {
        strcpy(decInfo->decoded_fname, "decoded.txt");
        printf("INFO: Output File not mentioned. Creating decoded.txt as default\n");
    }

    /* Return success when the decoding arguments are valid */
    return e_success;
}

/* Open the stego image and prepare the decoded output file */
Status open_decode_files(DecodeInfo *decInfo)
{
    /* Open the stego image in binary read mode */
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");
    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        printf("ERROR: Unable to open file %s\n", decInfo->stego_image_fname);
        return e_failure;
    }

    /* Move to the beginning of the BMP pixel data, after its 54-byte header */
    if (fseek(decInfo->fptr_stego_image, 54, SEEK_SET) != 0)
    {
        printf("ERROR: Unable to seek to image data\n");
        return e_failure;
    }

    /* Return success when the input image is ready */
    return e_success;
}

/* Decode one byte from the least significant bits of image data */
Status decode_byte_from_lsb(char *data, FILE *fptr_stego_image)
{
    int i;
    unsigned char image_byte;
    unsigned char value = 0;

    /* Repeat for all 8 bits of the hidden byte */
    for (i = 0; i < 8; i++)
    {
        /* Read one image byte containing one hidden bit */
        if (fread(&image_byte, 1, 1, fptr_stego_image) != 1)
            return e_failure;

        /* Extract the least significant bit and append it to the result */
        value = (unsigned char)((value << 1) | (image_byte & 1));
    }

    /* Store the reconstructed byte for the caller */
    *data = (char)value;

    /* Return success after decoding all 8 bits */
    return e_success;
}

/* Decode an integer size from the least significant bits of image data */
Status decode_size_from_lsb(int *size, FILE *fptr_stego_image)
{
    int i;
    unsigned char image_byte;
    unsigned int value = 0;

    /* Repeat for all 32 bits of the encoded integer */
    for (i = 0; i < 32; i++)
    {
        /* Read one image byte containing one hidden size bit */
        if (fread(&image_byte, 1, 1, fptr_stego_image) != 1)
            return e_failure;

        /* Extract the LSB and append it to the integer */
        value = (value << 1) | (image_byte & 1U);
    }

    /* Store the reconstructed size */
    *size = (int)value;

    /* Return success after decoding all 32 bits */
    return e_success;
}

/* Decode and validate the magic string signature */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    size_t i;
    char decoded_char;

    /* Decode each character of the expected magic string */
    for (i = 0; i < strlen(magic_string); i++)
    {
        /* Read one hidden character from the image LSBs */
        if (decode_byte_from_lsb(&decoded_char, decInfo->fptr_stego_image) != e_success)
            return e_failure;

        /* Compare the decoded character with the expected signature */
        if (decoded_char != magic_string[i])
        {
            printf("ERROR: Magic string validation failed\n");
            return e_failure;
        }
    }

    /* Return success when the complete magic string matches */
    return e_success;
}

/* Decode the secret file extension size */
Status decode_secret_file_extn_size(int *size, DecodeInfo *decInfo)
{
    /* Decode the 32-bit extension length from the image */
    if (decode_size_from_lsb(size, decInfo->fptr_stego_image) != e_success)
        return e_failure;

    /* Validate the extension length */
    if (*size != 4)
    {
        printf("ERROR: Only .txt files are supported\n");
        return e_failure;
    }

    /* Return success when the extension length is valid */
    return e_success;
}

/* Decode the secret file extension */
Status decode_secret_file_extn(int size, DecodeInfo *decInfo)
{
    int i;

    /* Decode each character in the secret file extension */
    for (i = 0; i < size; i++)
    {
        /* Read one extension character from the image */
        if (decode_byte_from_lsb(&decInfo->extn_secret_file[i],
                                 decInfo->fptr_stego_image) != e_success)
            return e_failure;
    }

    /* Add the string terminator after the decoded extension */
    decInfo->extn_secret_file[size] = '\0';

    /* Validate that only .txt files are supported */
    if (strcmp(decInfo->extn_secret_file, ".txt") != 0)
    {
        printf("ERROR: Only .txt files are supported\n");
        return e_failure;
    }

    /* Return success after validating the extension */
    return e_success;
}

/* Decode the secret file size */
Status decode_secret_file_size(long *file_size, DecodeInfo *decInfo)
{
    int decoded_size;

    /* Decode the 32-bit secret data size */
    if (decode_size_from_lsb(&decoded_size, decInfo->fptr_stego_image) != e_success)
        return e_failure;

    /* Reject negative sizes before storing the value */
    if (decoded_size < 0)
        return e_failure;

    /* Store the decoded size as a long integer */
    *file_size = decoded_size;
    decInfo->size_secret_file = *file_size;

    /* Return success after decoding the secret file size */
    return e_success;
}

/* Decode the secret file data and write it to the output file */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    long i;
    char decoded_byte;

    /* Decode exactly the number of bytes stored in the size field */
    for (i = 0; i < decInfo->size_secret_file; i++)
    {
        /* Decode one byte from the image */
        if (decode_byte_from_lsb(&decoded_byte, decInfo->fptr_stego_image) != e_success)
            return e_failure;

        /* Write the decoded byte to the recovered secret file */
        if (fwrite(&decoded_byte, 1, 1, decInfo->fptr_decoded_file) != 1)
            return e_failure;
    }

    /* Return success after writing all secret bytes */
    return e_success;
}

/* Performs the complete decoding procedure */

Status do_decoding(DecodeInfo *decInfo)
{
    int extn_size;
    size_t base_length;

    /* Print the required file-opening section heading */
    printf("-----------------------------------------\n");
    printf("INFO: Opening required files\n");

    /* Open the stego image and validate that it can be read */
    if (open_decode_files(decInfo) != e_success)
    {
        return e_failure;
    }

    printf("INFO: Opened %s\n", decInfo->stego_image_fname);

    /* Print completion of the file-opening section */
    printf("INFO: Done\n\n");
    printf("-----------------------------------------\n");

    /* Print the decoding procedure heading */
    printf(GREEN "INFO: ## Decoding Procedure Started ##\n" RESET);

    /* Decode and validate the magic string signature */
    printf("INFO: Decoding Magic String Signature\n");
    if (decode_magic_string(MAGIC_STRING, decInfo) != e_success)
    {
        fclose(decInfo->fptr_stego_image);
        decInfo->fptr_stego_image = NULL;
        return e_failure;
    }
    printf("INFO: Done\n");

    /* Decode the size of the secret file extension */
    printf("INFO: Decoding output file Extension Size\n");
    if (decode_secret_file_extn_size(&extn_size, decInfo) != e_success)
    {
        fclose(decInfo->fptr_stego_image);
        decInfo->fptr_stego_image = NULL;
        return e_failure;
    }
    printf("INFO: Done\n");

    /* Decode the secret file extension */
    printf("INFO: Decoding output file Extension\n");
    if (decode_secret_file_extn(extn_size, decInfo) != e_success)
    {
        fclose(decInfo->fptr_stego_image);
        decInfo->fptr_stego_image = NULL;
        return e_failure;
    }
    printf("INFO: Done\n");

    /* Append the decoded extension only if the output filename has no extension */
    if (strrchr(decInfo->decoded_fname, '.') == NULL)
    {
        base_length = strlen(decInfo->decoded_fname);

        if (base_length + strlen(decInfo->extn_secret_file) >=
            sizeof(decInfo->decoded_fname))
        {
            printf("ERROR: Output filename is too long\n");
            fclose(decInfo->fptr_stego_image);
            decInfo->fptr_stego_image = NULL;
            return e_failure;
        }

        strcat(decInfo->decoded_fname, decInfo->extn_secret_file);
    }

    /* Open the recovered secret file in binary write mode */
    decInfo->fptr_decoded_file = fopen(decInfo->decoded_fname, "wb");
    if (decInfo->fptr_decoded_file == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to create file %s\n",
                decInfo->decoded_fname);
        fclose(decInfo->fptr_stego_image);
        decInfo->fptr_stego_image = NULL;
        return e_failure;
    }

    printf("INFO: Opened %s\n", decInfo->decoded_fname);

    /* Decode the size of the secret file data */
    printf("INFO: Decoding %s File Size\n", decInfo->decoded_fname);
    if (decode_secret_file_size(&decInfo->size_secret_file, decInfo) != e_success)
    {
        fclose(decInfo->fptr_decoded_file);
        decInfo->fptr_decoded_file = NULL;
        fclose(decInfo->fptr_stego_image);
        decInfo->fptr_stego_image = NULL;
        return e_failure;
    }
    printf("INFO: Done\n");

    /* Decode and save the secret file contents */
    printf("INFO: Decoding %s File Data\n", decInfo->decoded_fname);
    if (decode_secret_file_data(decInfo) != e_success)
    {
        fclose(decInfo->fptr_decoded_file);
        decInfo->fptr_decoded_file = NULL;
        fclose(decInfo->fptr_stego_image);
        decInfo->fptr_stego_image = NULL;
        return e_failure;
    }
    printf("INFO: Done\n");

    /* Close the recovered secret file */
    fclose(decInfo->fptr_decoded_file);
    decInfo->fptr_decoded_file = NULL;

    /* Close the stego image file */
    fclose(decInfo->fptr_stego_image);
    decInfo->fptr_stego_image = NULL;

    /* Print the successful decoding completion message */
    printf(GREEN "INFO: ## Decoding Done Successfully ##\n" RESET);
    printf("-----------------------------------------\n");

    /* Return success when all decoding stages are complete */
    return e_success;
}
