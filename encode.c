#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("Width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("Height = %u\n", height);

    // Return image capacity
    rewind(fptr_image);
    printf("Total image capacity = %d bytes\n", width * height * 3);
    return width * height * 3;
}

uint get_file_size(FILE *fptr)
{
    long current_position, file_size;
    // Find the size of secret file data
    current_position = ftell(fptr);
    if (fseek(fptr, 0, SEEK_END) != 0)
        return 0;
    file_size = ftell(fptr);
    if (file_size < 0)
        return 0;
    // return secret file size
    if (current_position >= 0)
        fseek(fptr, current_position, SEEK_SET);
    else
        rewind(fptr);
    return (uint)file_size;
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    char *dot;
    // step 1 -> check argv[2] is having .bmp or not
    if (argv[2] != NULL && (dot = strrchr(argv[2], '.')) != NULL && strcmp(dot, ".bmp") == 0)
    {
        // yes -> store the file name into encInfo -> src_image_fname = argv[2]
        encInfo->src_image_fname = argv[2];
    }
    else
    {
        // no - > return e_failure
        return e_failure;
    }

    // step 2 -> check argv[3] is having extn there or not
    if (argv[3] != NULL && (dot = strrchr(argv[3], '.')) != NULL && dot[1] != '\0' && strlen(dot) < sizeof(encInfo->extn_secret_file))
    {
        // yes - > store the file name into encInfo -> secret_fname = argv[3]
        encInfo->secret_fname = argv[3];
        strcpy(encInfo->extn_secret_file, dot);
    }
    else
    {
        // no -> return e_failure
        return e_failure;
    }
    if (strcmp(encInfo->extn_secret_file, ".bmp") == 0)
        return e_failure;

    // step 3 -> check argv[4] is NULL or not
    if (argv[4] != NULL)
    {
        // yes -> check argv[4] is having .bmp or not
        char *dot = strrchr(argv[4], '.');

        if (dot != NULL && strcmp(dot, ".bmp") == 0)
        {
            // yes -> store the file name into encInfo -> stego_image_fname = argv[4]
            encInfo->stego_image_fname = argv[4];
        }
        else
        {
            // no - > return e_failure
            return e_failure;
        }
    }
    else
    {
        // no -> store default name the encInfo -> stego_image_fname = "stego.bmp"
        encInfo->stego_image_fname = "stego.bmp";

        printf("INFO: Output File not mentioned. Creating %s as default\n", encInfo->stego_image_fname);
    }

    // return e_success;
    return e_success;
}

Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    printf("-----------------------------------------\n");
    printf("INFO: Opening required files\n");
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);
        return e_failure;
    }
    printf("INFO: Opened %s\n", encInfo->src_image_fname);

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);
        fclose(encInfo->fptr_src_image);
        encInfo->fptr_src_image = NULL;
        return e_failure;
    }
    printf("INFO: Opened %s\n", encInfo->secret_fname);

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);
        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        encInfo->fptr_src_image = NULL;
        encInfo->fptr_secret = NULL;
        return e_failure;
    }
    printf("INFO: Opened %s\n", encInfo->stego_image_fname);

    // No failure return e_success
    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
    // Magic                 = 2 × 8 = 16
    // Extension size        = 4 × 8 = 32
    // Extension             = 4 × 8 = 32
    // Secret size           = 4 × 8 = 32
    // Secret data           = 9 × 8 = 72

    // step 1 -> encInfo -> image_capacity = call get_image_size_for_bmp(encoInfo -> fptr_src_image)
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);

    // step 2 -> encInfo -> size_secret_file = call get_file_size(encInfo -> fptr_secret)
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    // step 3 -> check encInfo -> image_capacity > 16 + 32 + (strlen(encInfo -> extn_secret_file) * 8) + 32 + (encInfo -> size_secret_file * 8)
    {
        unsigned long long required = (unsigned long long)strlen(MAGIC_STRING) * 8ULL + 32ULL +
                                      (unsigned long long)strlen(encInfo->extn_secret_file) * 8ULL + 32ULL +
                                      (unsigned long long)encInfo->size_secret_file * 8ULL;
        if (encInfo->image_capacity > 0 && required <= encInfo->image_capacity)
        {
            // t -> return e_success;
            return e_success;
        }
        // f -> return e_failure
        return e_failure;
    }
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    // step 1 -> rewind source file pointer
    rewind(fptr_src_image);
    // declare one character buffer of size 54
    char header[54];

    // step 2 -> read 54 bytes from source file fread()
    if (fread(header, 1, sizeof(header), fptr_src_image) != sizeof(header))
        return e_failure;

    // step 3 -> write 54 bytes to dest file fwrite()
    if (fwrite(header, 1, sizeof(header), fptr_dest_image) != sizeof(header))
        return e_failure;

    // return e_success
    return e_success;
}
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    // char buffer[8];
    char buffer[8];
    int i;

    // step 1 -> read 8 bytes from source file
    // step 2 -> call encode_byte_to_lsb(magic_string[i], buffer)
    // step 3 -> write the buffer into dest file
    //  repeat this for strlen(magic_string) times from step 1
    for (i = 0; magic_string[i] != '\0'; i++)
    {
        if (fread(buffer, 1, 8, encInfo->fptr_src_image) != 8)
            return e_failure;
        if (encode_byte_to_lsb(magic_string[i], buffer) != e_success)
            return e_failure;
        if (fwrite(buffer, 1, 8, encInfo->fptr_stego_image) != 8)
            return e_failure;
    }
    return e_success;
}
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    // char buffer[32];
    char buffer[32];

    // step 1 -> read 32 bytes from source file
    if (fread(buffer, 1, 32, encInfo->fptr_src_image) != 32)
        return e_failure;

    // step 2 -> call encode_size_to_lsb(size, buffer)
    if (encode_size_to_lsb(size, buffer) != e_success)
        return e_failure;

    // step 3 -> write the buffer into dest file
    if (fwrite(buffer, 1, 32, encInfo->fptr_stego_image) != 32)
        return e_failure;
    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    // char buffer[8];
    char buffer[8];
    int i;

    // step 1 -> read 8 bytes from source file
    // step 2 -> call encode_byte_to_lsb(file_extn[i], buffer)
    // step 3 -> write the buffer into dest file
    //  repeat this for strlen(file_extn) times from step 1
    for (i = 0; file_extn[i] != '\0'; i++)
    {
        if (fread(buffer, 1, 8, encInfo->fptr_src_image) != 8)
            return e_failure;
        if (encode_byte_to_lsb(file_extn[i], buffer) != e_success)
            return e_failure;
        if (fwrite(buffer, 1, 8, encInfo->fptr_stego_image) != 8)
            return e_failure;
    }
    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    // char buffer[32];
    char buffer[32];

    // step 1 -> read 32 bytes from source file
    if (fread(buffer, 1, 32, encInfo->fptr_src_image) != 32)
        return e_failure;

    // step 2 -> call encode_size_to_lsb(file_size, buffer)
    if (encode_size_to_lsb((int)file_size, buffer) != e_success)
        return e_failure;

    // step 3 -> write the buffer into dest file
    if (fwrite(buffer, 1, 32, encInfo->fptr_stego_image) != 32)
        return e_failure;
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    // Move pointer to beginning of secret file
    rewind(encInfo->fptr_secret);

    //  char buffer[8];
    char buffer[8];
    long i;

    //  repeat this for size_secret_file times from step 2
    for (i = 0; i < encInfo->size_secret_file; i++)
    {
        // step 1 -> read secret data from file and store it into encInfo -> secret_data
        if (fread(&encInfo->secret_data[0], 1, 1, encInfo->fptr_secret) != 1)
            return e_failure;
        // step 2 -> read 8 bytes from source file
        if (fread(buffer, 1, 8, encInfo->fptr_src_image) != 8)
            return e_failure;
        // step 3 -> call encode_byte_to_lsb(encInfo -> secret_data[i], buffer)
        if (encode_byte_to_lsb(encInfo->secret_data[0], buffer) != e_success)
            return e_failure;
        // step 4 -> write the buffer into dest file
        if (fwrite(buffer, 1, 8, encInfo->fptr_stego_image) != 8)
            return e_failure;
    }
    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;

    while (fread(&ch, 1, 1, fptr_src) == 1) // Read remaining bytes
    {
        if (fwrite(&ch, 1, 1, fptr_dest) != 1) // Write to destination
            return e_failure;
    }

    if (ferror(fptr_src) == 0 && ferror(fptr_dest) == 0) // Check if both pointers match
    {
        return e_success;
    }
    return e_failure;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    int i;
    // Repeat for 8 bits
    for (i = 0; i < 8; i++)
    {
        // Extract LSB from data
        int bit = ((unsigned char)data >> (7 - i)) & 1;
        // Clear LSB of image byte
        image_buffer[i] = (char)((unsigned char)image_buffer[i] & 0xFE);
        // Put data bit into image LSB
        image_buffer[i] = (char)((unsigned char)image_buffer[i] | bit);
        // Shift data right
    }
    return e_success;
}

Status encode_size_to_lsb(int size, char *imageBuffer)
{
    // write logic to encode the int
    int i;
    for (i = 0; i < 32; i++)
    {
        int bit = ((unsigned int)size >> (31 - i)) & 1U;
        imageBuffer[i] = (char)((unsigned char)imageBuffer[i] & 0xFE);
        imageBuffer[i] = (char)((unsigned char)imageBuffer[i] | bit);
    }
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    // step 1 -> check open_files(encInfo) is returning e_success or not
    if (open_files(encInfo) != e_success)
    {
        // no -> print error return e_failure
        return e_failure;
    }
    else
    {
        // yes -> print success and goto next step
        printf("INFO: Done\n\n");
    }

    printf("-----------------------------------------\n");
    printf(GREEN "INFO: ## Encoding Procedure Started ##\n" RESET);
    printf("INFO: Checking for %s size\n", encInfo->secret_fname);
    // step 2 -> check check_capacity(encInfo) is returning e_success or not
    if (check_capacity(encInfo) != e_success)
    {
        // no -> print error return e_failure
        printf("ERROR: Image does not have enough capacity or secret file is empty\n");
        return e_failure;
    }
    else
    {
        // yes -> print success and goto next step
        if (encInfo->size_secret_file == 0)
        {
            printf("ERROR: Secret file is empty\n");
            return e_failure;
        }
        printf("INFO: Done. Not Empty\n");
    }

    printf("INFO: Checking for %s capacity to handle %s\n", encInfo->src_image_fname, encInfo->secret_fname);
    if (encInfo->image_capacity < (unsigned long long)strlen(MAGIC_STRING) * 8 + 32 +
                                      (unsigned long long)strlen(encInfo->extn_secret_file) * 8 + 32 +
                                      (unsigned long long)encInfo->size_secret_file * 8)
    {
        printf("ERROR: Image capacity is insufficient\n");
        return e_failure;
    }
    printf("INFO: Done. Found OK\n");

    printf("INFO: Copying Image Header\n");
    // step 3 -> call copy_bmp_header(encInfo -> fptr_src_image, encInfo -> fptr_stego_image)is returning e_success or not
    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success)
    {
        // no -> print error return e_failure
        printf("ERROR: Unable to copy BMP header\n");
        return e_failure;
    }
    else
    {
        // yes -> print success and goto next step
        printf("INFO: Done\n");
    }

    printf("INFO: Encoding Magic String Signature\n");
    // step 4 -> call encode_magic_string(MAGIC_STRING, encInfo)
    if (encode_magic_string(MAGIC_STRING, encInfo) != e_success)
        return e_failure;
    printf("INFO: Done\n");

    printf("INFO: Encoding %s File Extension\n", encInfo->secret_fname);
    // step 5 -> call encode_secret_file_extn_size(strlen(encInfo -> extn_secret_file), encInfo)
    if (encode_secret_file_extn_size((int)strlen(encInfo->extn_secret_file), encInfo) != e_success)
        return e_failure;

    // step 6 -> call encode_secret_file_extn(encInfo -> extn_secret_file, encInfo)
    if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) != e_success)
        return e_failure;
    printf("INFO: Done\n");

    printf("INFO: Encoding %s File Size\n", encInfo->secret_fname);
    // step 7 -> call encode_secret_file_size(encInfo ->  size_secret_file, encInfo)
    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) != e_success)
        return e_failure;
    printf("INFO: Done\n");

    printf("INFO: Encoding %s File Data\n", encInfo->secret_fname);
    // step 8 -> call encode_secret_file_data(encInfo)
    if (encode_secret_file_data(encInfo) != e_success)
        return e_failure;
    printf("INFO: Done\n");

    printf("INFO: Copying Left Over Data\n");
    // step 9 -> copy_remaining_img_data(encInfo -> fptr_src_image, encInfo -> fptr_stego_image)is returning e_success or not
    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success)
    {
        // no -> print error return e_failure
        printf("ERROR: Unable to copy remaining image data\n");
        return e_failure;
    }
    else
    {
        // yes -> print success
        printf("INFO: Done\n");
    }

    printf(GREEN "INFO: ## Encoding Done Successfully ##\n" RESET);
    printf("-----------------------------------------\n");
    // return e_success
    return e_success;
}
