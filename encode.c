#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include <unistd.h>


#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define ORANGE  "\033[38;5;208m"
#define PURPLE  "\033[35m"
#define CYAN    "\033[36m"
#define MAGENTA "\033[95m"
#define RESET   "\033[0m"

/* Reads image width and height to calculate how many bytes can hold hidden data.
 * Important: width * height * 3 estimates the available RGB image bytes.
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    /* Get the image dimensions from the BMP header */
    printf(MAGENTA "INFO : Getting image size...\n" RESET);
    usleep(300000);

    uint width, height;

    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("Width  : %u pixels\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("Height : %u pixels\n", height);

    printf(GREEN "DONE : Image size retrieved successfully.\n" RESET);

    // Return image capacity
    return width * height * 3;
}

/* Opens the cover image and secret text for reading, and the stego image for writing.
 * Important: the fopen modes determine whether each file is read or written.
 */
Status open_files(EncodeInfo *encInfo)
{
    /* Open source image, secret file, and stego image files */
    printf(MAGENTA "INFO : Opening required files...\n" RESET);
    usleep(300000);

    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, RED "ERROR : Unable to open file %s\n" RESET, encInfo->src_image_fname);

        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, RED "ERROR : Unable to open file %s\n" RESET, encInfo->secret_fname);

        return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, RED "ERROR : Unable to open file %s\n" RESET, encInfo->stego_image_fname);

        return e_failure;
    }

    printf(GREEN "DONE : All required files opened successfully.\n" RESET);

    // No failure return e_success
    return e_success;
}

/* Checks the input filenames and saves them in the encoding information.
 * Important: the extension checks ensure the cover and output are BMP, and the secret is TXT.
 */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    /* Validate the source image, secret file, and output file arguments */
    printf(MAGENTA "INFO : Validating encoding arguments...\n" RESET);
    usleep(300000);

    int len = strlen(argv[2]);
    
    char* str = &argv[2][len-4];

    if(strcmp(str,".bmp")==0)
    {
        len = strlen(argv[3]);
        if(len < 4)
        {
            return e_failure;
        }

        str = &argv[3][len-4];

        if(strcmp(str,".txt")==0)
        {
            strcpy(encInfo->extn_secret_file,".txt");

            if(argv[4]==NULL)
            {
                strcpy(encInfo->src_image_fname,argv[2]);
                strcpy(encInfo->secret_fname,argv[3]);
                strcpy(encInfo->stego_image_fname,"stego.bmp");

                printf(GREEN "DONE : Encoding arguments validated successfully.\n" RESET);

                return open_files(encInfo);
            }
            else
            {
                len = strlen(argv[4]);
                str = &argv[4][len-4];

                if(strcmp(str,".bmp")==0)
                {
                    strcpy(encInfo->src_image_fname,argv[2]);
                    strcpy(encInfo->secret_fname,argv[3]);
                    strcpy(encInfo->stego_image_fname,argv[4]);

                    printf(GREEN "DONE : Encoding arguments validated successfully.\n" RESET);

                    return open_files(encInfo);
                }
                else
                {
                    printf(RED "ERROR : Output file must have the '.bmp' extension.\n" RESET);
                    return e_failure;
                }
            }
        }
        else
        {
            printf(RED "ERROR : Secret file must have the '.txt' extension.\n" RESET);
            return e_failure;
        }
    }
    else
    {
        printf(RED "ERROR : Source image file must have the '.bmp' extension.\n" RESET);
        return e_failure;
    }
}

/* Runs the encoding steps in order to create the image with the hidden message.
 * Important: check_capacity stops encoding if the image cannot hold the data.
 */
Status do_encoding(EncodeInfo *encInfo)
{
    /* Perform the complete encoding process */
    printf(MAGENTA "INFO : Starting the encoding process...\n" RESET);
    usleep(300000);

    printf(YELLOW "Enter the Magic String : " RESET);
    scanf("%4s",encInfo->Magic_String);

    Status state = check_capacity(encInfo);
    if(state == e_failure)
    {
        return e_failure;
    }
    
    copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image);

    encode_magic_string(encInfo);

    encode_secret_file_extn_size(encInfo);

    encode_secret_file_extn(encInfo);

    encode_secret_file_size(encInfo);

    encode_secret_file_data(encInfo);

    copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image);

    printf(GREEN "DONE : Encoding process completed successfully.\n" RESET);
    
    return e_success;
}

/* Checks that the cover image has enough room for the message and its metadata.
 * Important: the capacity comparison decides whether encoding can continue.
 */
Status check_capacity(EncodeInfo *encInfo)
{
    /* Check whether the source image has enough capacity for encoding */
    printf(MAGENTA "INFO : Checking image capacity...\n" RESET);
    usleep(300000);

    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    int len_magic_string = strlen(encInfo->Magic_String);
    int len_file_extn = strlen(encInfo->extn_secret_file);
    uint Total_size = (encInfo->size_secret_file + 4 + len_file_extn + 4 + len_magic_string + 4) * 8;

    if(encInfo->image_capacity >= Total_size)
    {
        printf(GREEN "DONE : Image has sufficient capacity for encoding.\n" RESET);
        return e_success;
    }

    printf(RED "ERROR : Insufficient image capacity for encoding.\n" RESET);
    return e_failure;
}

/* Finds the secret file length so its size can be stored in the image.
 * Important: fseek moves to the end, and ftell returns the current position.
 */
uint get_file_size(FILE *fptr)
{
    /* Determine the size of the secret file */
    printf(MAGENTA "INFO : Getting secret file size...\n" RESET);
    usleep(300000);

    fseek(fptr,0,SEEK_END);
    uint size = ftell(fptr);

    printf(GREEN "DONE : Secret file size retrieved successfully.\n" RESET);

    return size;
}

/* Copies the BMP header so the output image remains recognizable as a bitmap.
 * Important: copying the first 54 bytes preserves the standard BMP header.
 */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    /* Copy the BMP header from the source image to the stego image */
    printf(MAGENTA "INFO : Copying BMP header...\n" RESET);
    usleep(300000);

    rewind(fptr_src_image);
    char buffer[54];
    fread(buffer,1,54,fptr_src_image);
    fwrite(buffer,1,54,fptr_dest_image);

    printf(GREEN "DONE : BMP header copied successfully.\n" RESET);

    return e_success;
}

/* Stores the magic string length and characters in the image data.
 * Important: encode_byte_to_lsb writes each character into eight image bytes.
 */
Status encode_magic_string(EncodeInfo *encInfo)
{
    /* Encode the magic string length and data into the image */
    printf(MAGENTA "INFO : Encoding magic string...\n" RESET);
    usleep(300000);

    /* Finding length of the magic string */
    int len = strlen(encInfo->Magic_String);

    /* Encode magic string size */
    encode_size_to_lsb(len, encInfo->fptr_src_image, encInfo->fptr_stego_image);
    
    /* Encode magic string */
    for(int i=0;i<len;i++)
    {
        fread(encInfo->image_data,1,8,encInfo->fptr_src_image);
        encode_byte_to_lsb(encInfo->Magic_String[i], encInfo->image_data);
        fwrite(encInfo->image_data,1,8,encInfo->fptr_stego_image);
    }

    printf(GREEN "DONE : Magic string encoded successfully.\n" RESET);
    
    return e_success;
}

/* Hides one character by placing its bits in the low bits of eight image bytes.
 * Important: the loop handles one bit of the character on each pass.
 */
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    /* Encode one byte into the least significant bits of image data */

    for(int i=0;i<8;i++)
    {
        image_buffer[i] = image_buffer[i] & ~1;
        int get = (data & (1<<(7-i))) >> (7-i);
        image_buffer[i] = image_buffer[i] | get;
    }


    return e_success;
}

/* Stores a 32-bit size value in the low bits of 32 image bytes.
 * Important: the loop writes one size bit into each image byte.
 */
Status encode_size_to_lsb(int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    /* Encode a 32-bit integer into the least significant bits */

    char ch[32];
    fread(ch,1,32,fptr_src_image);

    for(int i=0;i<32;i++)
    {
        ch[i] = ch[i] & ~1;
        int get = (size & (1U<<(31-i))) >> (31-i);
        ch[i] |= get;
    }

    fwrite(ch,1,32,fptr_stego_image);

    return e_success;
}

/* Stores how many characters are in the hidden file extension.
 * Important: this calls encode_size_to_lsb to save the extension length.
 */
Status encode_secret_file_extn_size(EncodeInfo *encInfo)
{
    /* Encode the length of the secret file extension */
    printf(MAGENTA "INFO : Encoding secret file extension size...\n" RESET);
    usleep(300000);

    int len = strlen(encInfo->extn_secret_file);

    encode_size_to_lsb(len, encInfo->fptr_src_image, encInfo->fptr_stego_image);

    printf(GREEN "DONE : Secret file extension size encoded successfully.\n" RESET);

    return e_success;
}

/* Stores the secret file extension characters in the image.
 * Important: each extension character is encoded across eight image bytes.
 */
Status encode_secret_file_extn(EncodeInfo *encInfo)
{
    /* Encode the secret file extension into the image */
    printf(MAGENTA "INFO : Encoding secret file extension...\n" RESET);
    usleep(300000);

    int len = strlen(encInfo->extn_secret_file);

    for(int i=0;i<len;i++)
    {
        fread(encInfo->image_data,1,8,encInfo->fptr_src_image);
        encode_byte_to_lsb(encInfo->extn_secret_file[i], encInfo->image_data);
        fwrite(encInfo->image_data,1,8,encInfo->fptr_stego_image);
    }

    printf(GREEN "DONE : Secret file extension encoded successfully.\n" RESET);

    return e_success;
}

/* Stores the secret text length in the image.
 * Important: size_secret_file is passed to encode_size_to_lsb as the value to hide.
 */
Status encode_secret_file_size(EncodeInfo *encInfo)
{
    /* Encode the size of the secret file */
    printf(MAGENTA "INFO : Encoding secret file size...\n" RESET);
    usleep(300000);

    encode_size_to_lsb(encInfo->size_secret_file, encInfo->fptr_src_image,encInfo->fptr_stego_image);

    printf(GREEN "DONE : Secret file size encoded successfully.\n" RESET);

    return e_success;
}

/* Reads the secret text file and stores its characters in the image.
 * Important: each character is passed to encode_byte_to_lsb for hiding.
 */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    /* Encode the secret file data into the image */
    printf(MAGENTA "INFO : Encoding secret file data...\n" RESET);
    usleep(300000);

    rewind(encInfo->fptr_secret);
    char ch;

    for(int i=0;i<encInfo->size_secret_file;i++)
    {
        fread(encInfo->image_data,1,8,encInfo->fptr_src_image);
        ch = fgetc(encInfo->fptr_secret);
        encode_byte_to_lsb(ch,encInfo->image_data);
        fwrite(encInfo->image_data,1,8,encInfo->fptr_stego_image);
    }

    printf(GREEN "DONE : Secret file data encoded successfully.\n" RESET);

    return e_success;
}

/* Copies all image bytes after the hidden data into the output image unchanged.
 * Important: the loop continues until fread reaches the end of the source image.
 */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    /* Copy the remaining image data without modification */
    printf(MAGENTA "INFO : Copying remaining image data...\n" RESET);
    usleep(300000);

    char buffer[100];
    int n;

    while((n=fread(buffer,1,100,fptr_src))>0)
    {
        fwrite(buffer,1,n,fptr_dest);
    }


    printf(GREEN "DONE : Remaining image data copied successfully.\n" RESET);

    return e_success;
}