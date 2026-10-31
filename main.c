/*
Documentation

Name        : Varun M M
Student id  : 26018_042
Batch id    : 26018D
Start Date  : 08/10/2026
End date    : 22/10/2026

Description:
The following main functions are implemented in the Steganography project to hide secret text data inside a BMP image and retrieve the hidden data using the Least Significant Bit (LSB) technique.

main()                              :-> Handles command-line arguments and selects the required operation such as encoding or decoding.
                                     -> Validates the arguments and calls the appropriate encoding or decoding functions.

do_encoding()                       :-> Hides secret text data inside a BMP image using the LSB technique.
                                     -> Copies the BMP header, encodes the magic string, secret file extension, file size and secret data, and copies the remaining image data.

do_decoding()                       :-> Extracts hidden secret data from a stego image.
                                     -> Validates the magic string, retrieves the secret file extension and file size, and writes the decoded data into an output file.

read_and_validate_encode_args()     :-> Validates the source BMP image, secret text file and optional destination BMP filename.
                                     -> Stores the filenames required for encoding.

read_and_validate_decode_args()     :-> Validates the stego BMP image and optional output text filename.
                                     -> Sets the default output filename when one is not specified.

check_capacity()                    :-> Checks whether the source BMP image has sufficient capacity to store the secret data.
                                     -> Compares the image capacity with the space required for the secret file and its metadata.

encode_byte_to_lsb()                :-> Encodes one byte of secret data into the Least Significant Bits of image data bytes.

decode_byte_from_lsb()              :-> Extracts one byte of hidden data from the Least Significant Bits of image data bytes.

encode_magic_string()               :-> Stores a predefined magic string in the image to identify encoded images.

decode_magic_string()               :-> Extracts and validates the magic string from the stego image.

encode_secret_file_data()           :-> Reads the secret text data and embeds it into the source BMP image.

decode_secret_file_data()           :-> Extracts the hidden secret data and writes it into the output file.


Sample input:

Encoding:
$ ./a.out -e beautiful.bmp secret.txt stego.bmp

Output:
-----------------------------------------
INFO: Opening required files
INFO: Opened beautiful.bmp
INFO: Opened secret.txt
INFO: Opened stego.bmp
INFO: Done

-----------------------------------------
INFO: ## Encoding Procedure Started ##
INFO: Checking for secret.txt size
Width = 1024
Height = 768
Total image capacity = 2359296 bytes
INFO: Done. Not Empty
INFO: Checking for beautiful.bmp capacity to handle secret.txt
INFO: Done. Found OK
INFO: Copying Image Header
INFO: Done
INFO: Encoding Magic String Signature
INFO: Done
INFO: Encoding secret.txt File Extension
INFO: Done
INFO: Encoding secret.txt File Size
INFO: Done
INFO: Encoding secret.txt File Data
INFO: Done
INFO: Copying Left Over Data
INFO: Done
INFO: ## Encoding Done Successfully ##
-----------------------------------------

Decoding:
$ ./a.out -d stego.bmp output.txt

Sample output:
-----------------------------------------
INFO: Opening required files
INFO: Opened stego.bmp
INFO: Done

-----------------------------------------
INFO: ## Decoding Procedure Started ##
INFO: Decoding Magic String Signature
INFO: Done
INFO: Decoding output file Extension Size
INFO: Done
INFO: Decoding output file Extension
INFO: Done
INFO: Opened decoded.txt
INFO: Decoding decoded.txt File Size
INFO: Done
INFO: Decoding decoded.txt File Data
INFO: Done
INFO: ## Decoding Done Successfully ##
-----------------------------------------
*/

#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

OperationType check_operation_type(char *);

int main(int argc, char *argv[]) // .aout -e <source_file> <secret_data_file> <optional>
{
    // step 1 -> check check_operation_type(argv[1]) is returning e_encode or not
    OperationType operation;
    operation = (argc > 1) ? check_operation_type(argv[1]) : e_unsupported;

    // yes -> EncodeInfo encInfo;
    if (operation == e_encode)
    {
        EncodeInfo encInfo = {0};
        // check read_and_validate_encode_args(argv, &encInfo) is returning e_success or e_failure
        if ((argc != 4 && argc != 5) || read_and_validate_encode_args(argv, &encInfo) != e_success)
        {
            // failure -> print error msg and stop
            printf("---------------------------------------------------------------------\n");
            printf("ERROR: Invalid encoding arguments.\n");
            printf("USAGE: \n");
            printf("---------------------------------------------------------------------\n");
            printf("./a.out: Encoding : ./a.out -e <.bmp file> <.txt file> [output file]\n");
            printf("---------------------------------------------------------------------\n");
            return e_failure;
        }
        else
        {
            // success -> print success msg
            // check do_encoding(&encInfo) is returning e_success or e_failure
            if (do_encoding(&encInfo) != e_success)
            {
                // failure -> print error msg and stop
                if (encInfo.fptr_src_image != NULL)
                    fclose(encInfo.fptr_src_image);
                if (encInfo.fptr_secret != NULL)
                    fclose(encInfo.fptr_secret);
                if (encInfo.fptr_stego_image != NULL)
                    fclose(encInfo.fptr_stego_image);
                return e_failure;
            }
            else
            {
                // success -> print success msg and stop
                if (encInfo.fptr_src_image != NULL)
                    fclose(encInfo.fptr_src_image);
                if (encInfo.fptr_secret != NULL)
                    fclose(encInfo.fptr_secret);
                if (encInfo.fptr_stego_image != NULL)
                    fclose(encInfo.fptr_stego_image);
                return e_success;
            }
        }
    }

    // Check whether decoding operation is selected
    if (operation == e_decode)
    {
        DecodeInfo decInfo = {0};
        if ((argc != 3 && argc != 4) ||
            read_and_validate_decode_args(argv, &decInfo) != e_success)
        {
            printf("--------------------------------------------------------------------------\n");
            printf("ERROR: Invalid decoding arguments. Use -d <stego.bmp> [output_file]\n");
            printf("--------------------------------------------------------------------------\n");
            return e_failure;
        }
        if (do_decoding(&decInfo) != e_success)
        {
            if (decInfo.fptr_stego_image != NULL)
                fclose(decInfo.fptr_stego_image);
            if (decInfo.fptr_decoded_file != NULL)
                fclose(decInfo.fptr_decoded_file);
            return e_failure;
        }
        if (decInfo.fptr_stego_image != NULL)
            fclose(decInfo.fptr_stego_image);
        if (decInfo.fptr_decoded_file != NULL)
            fclose(decInfo.fptr_decoded_file);
        return e_success;
    }

    // return error
    printf("----------------------------------------------------------------------\n");
    printf("ERROR! Invalid Argument: \n");
    printf("USAGE: \n");
    printf("----------------------------------------------------------------------\n");
    printf("./a.out: Encoding : ./a.out -e <.bmp file> <.txt file> [output file]\n");
    printf("./a.out: Decoding : ./a.out -d <.bmp file> [output file]\n");
    printf("----------------------------------------------------------------------\n");
    return e_failure;
}

OperationType check_operation_type(char *symbol)
{
    // step 1 -> check symbol is -e or not
    if (symbol != NULL && strcmp(symbol, "-e") == 0)
        // yes -> return e_encode
        return e_encode;

    // step 2 -> check symbol is -d or not
    if (symbol != NULL && strcmp(symbol, "-d") == 0)
        // yes -> return e_decode
        return e_decode;

    // return e_unsupported
    return e_unsupported;
}
