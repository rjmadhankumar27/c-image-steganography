#ifndef DECODE_H
#define DECODE_H

#include "types.h" 

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 5
#define MAX_MAGIC_SIZE 5
#define MAX_FILE_NAME_LENGTH 50

/* Holds the image, output file, and hidden message details during decoding. */
typedef struct 
{
    /* Source Image info */
    char src_image_fname[MAX_FILE_NAME_LENGTH];
    FILE *fptr_src_image;
    char image_data[MAX_IMAGE_BUF_SIZE];

    /*Magic string*/
    int Magic_String_length;
    char Magic_String[MAX_MAGIC_SIZE];

    /* Secret File Info */
    char output_fname[MAX_FILE_NAME_LENGTH];
    FILE *fptr_output;
    char extn_secret_file[MAX_FILE_SUFFIX];
    int extn_secret_file_length;
    char output_data[MAX_IMAGE_BUF_SIZE];
    int size_secret_file;


} DecodeInfo;

/* Checks the input filenames and prepares the files for decoding. */
Status read_and_validate_decode_args(char* argv[], DecodeInfo *decInfo);

/* Opens the stego image and the decoded text output file. */
Status open_files_decode(DecodeInfo* decInfo);

/* Runs the checks and extracts the hidden message. */
Status do_decoding(DecodeInfo* decInfo);

/* Reads the stored magic string and compares it with the user's input. */
Status check_magic_string(DecodeInfo* decInfo);

/* Rebuilds a 32-bit size value from the image's least significant bits. */
Status decoding_lsb_to_size(int* data,char* buffer);

/* Rebuilds one character from eight image bytes. */
Status decoding_lsb_to_byte(char* data,char* buffer);

/* Extracts the hidden text and writes it to the output file. */
Status decode_file(DecodeInfo* decInfo);

/* Reads the hidden extension and checks it against the output filename. */
Status check_extn_file(DecodeInfo* decInfo);

#endif