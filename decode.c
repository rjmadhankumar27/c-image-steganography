#include <stdio.h>
#include "types.h"
#include "decode.h"
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

/* Checks the input filenames and opens the files needed for decoding.
 * Important: the extension checks make sure the image is .bmp and output is .txt.
 */
Status read_and_validate_decode_args(char* argv[], DecodeInfo *decInfo)
{
    /* Validate the source image and output file arguments */
    printf(MAGENTA "INFO : Validating decoding arguments...\n" RESET);
    usleep(300000);

    int len = strlen(argv[2]);

    if(len < 5)
    {
        printf(RED "ERROR : Please enter a correct file name.\n" RESET);
    }

    char* str = &argv[2][len-4];

    if(strcmp(".bmp",str) == 0)
    {
        strcpy(decInfo->src_image_fname,argv[2]);

        if(argv[3] == NULL)
        {
            strcpy(decInfo->output_fname,"output.txt");

            printf(GREEN "DONE : Decoding arguments validated successfully.\n" RESET);

            return open_files_decode(decInfo);
        }
        else
        {
            len = strlen(argv[3]);

            if(len < 4)
            {
                printf(RED "ERROR : Please enter a correct file name.\n" RESET);
            }

            str = &argv[3][len-4];

            if(strcmp(".txt",str) == 0)
            {
                strcpy(decInfo->output_fname,argv[3]);

                printf(GREEN "DONE : Decoding arguments validated successfully.\n" RESET);

                return open_files_decode(decInfo);
            }
            else
            {
                printf(RED "ERROR : Output file must have the '.txt' extension.\n" RESET);
            }
        }
    }
    else
    {
        printf(RED "ERROR : Source file must have the '.bmp' extension.\n" RESET);
    }

    usleep(300000);
    printf(RED "Decoding argument validation failed.\n" RESET);

    return e_failure;
}


/* Opens the stego image for reading and the output text file for writing.
 * Important: fopen uses "rb" for the image and "w+" for the decoded text.
 */
Status open_files_decode(DecodeInfo* decInfo)
{
    /* Open the source image and output files */
    printf(MAGENTA "INFO : Opening decoding files...\n" RESET);
    usleep(300000);

    decInfo->fptr_src_image = fopen(decInfo->src_image_fname,"rb");
    
    if(decInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, RED "ERROR : Unable to open file %s\n" RESET, decInfo->src_image_fname);

        return e_failure;
    }

    decInfo->fptr_output = fopen(decInfo->output_fname,"w+");

    if(decInfo->fptr_output == NULL)
    {
        perror("fopen");
        fprintf(stderr, RED "ERROR : Unable to open file %s\n" RESET, decInfo->output_fname);

        return e_failure;
    }

    printf(GREEN "DONE : Decoding files opened successfully.\n" RESET);

    return e_success;
}


/* Runs each decoding step in order and stops if a verification step fails.
 * Important: the magic string check must pass before the hidden message is decoded.
 */
Status do_decoding(DecodeInfo* decInfo)
{
    /* Perform the complete decoding process */
    printf(MAGENTA "INFO : Starting the decoding process...\n" RESET);
    usleep(300000);

    Status state = check_magic_string(decInfo);

    if(state == e_failure)
    {
        printf(RED "ERROR : Magic string verification failed.\n" RESET);
        return e_failure;
    }

    state = check_extn_file(decInfo);

    if(state == e_failure)
    {
        printf(RED "ERROR : Secret file extension verification failed.\n" RESET);
        return e_failure;
    }

    decode_file(decInfo);

    printf(GREEN "DONE : Decoding process completed successfully.\n" RESET);

    return e_success;
}


/* Reads the stored magic string from the image and checks the user's entry.
 * Important: strcmp confirms that the entered string matches the decoded one.
 */
Status check_magic_string(DecodeInfo* decInfo)
{
    /* Extract and verify the magic string from the stego image */
    printf(MAGENTA "INFO : Checking magic string...\n" RESET);
    usleep(300000);

    char buffer[32];
    fseek(decInfo->fptr_src_image,54,SEEK_SET);
    
    fread(buffer,1,32,decInfo->fptr_src_image);
    decoding_lsb_to_size(&decInfo->Magic_String_length,buffer);

    int i;

    for(i=0;i<decInfo->Magic_String_length;i++)
    {
        fread(decInfo->image_data,1,8,decInfo->fptr_src_image);
        decoding_lsb_to_byte(&decInfo->Magic_String[i],decInfo->image_data);
    }

    decInfo->Magic_String[i]='\0';

    printf(YELLOW "Enter the Magic String : " RESET);
    char str[5];
    scanf("%4s",str);

    if(strcmp(decInfo->Magic_String,str)==0)
    {
        usleep(300000);
        printf(GREEN "DONE : Magic string verified successfully.\n" RESET);

        return e_success;
    }

    usleep(300000);
    printf(RED "ERROR : Magic string is incorrect.\n" RESET);

    return e_failure;
}


/* Rebuilds a 32-bit number from the least significant bits in the buffer.
 * Important: the loop reads one stored bit at a time to reconstruct the value.
 */
Status decoding_lsb_to_size(int* data,char* buffer)
{
    /* Decode a 32-bit value from the least significant bits */

    *data=0;

    for(int i=0;i<32;i++)
    {
        *data = *data | ((buffer[i] & 1U) << (31-i));
    }

    return e_success;
}


/* Rebuilds one character from the least significant bits of eight image bytes.
 * Important: the loop combines those eight bits into the output byte.
 */
Status decoding_lsb_to_byte(char* data,char* buffer)
{
    /* Decode one byte from the least significant bits */

    *data=0;

    for(int i=0;i<8;i++)
    {
        *data = *data | ((buffer[i] & 1U) << (7-i));
    }


    return e_success;
}


/* Reads the hidden file extension and checks it against the output filename.
 * Important: strcmp verifies that both extensions match.
 */
Status check_extn_file(DecodeInfo* decInfo)
{
    /* Extract and verify the secret file extension */
    printf(MAGENTA "INFO : Checking secret file extension...\n" RESET);
    usleep(300000);

    char buffer[32];
    fread(buffer,1,32,decInfo->fptr_src_image);
    decoding_lsb_to_size(&decInfo->extn_secret_file_length,buffer);

    int i;

    for(i=0;i<decInfo->extn_secret_file_length;i++)
    {
        fread(decInfo->image_data,1,8,decInfo->fptr_src_image);
        decoding_lsb_to_byte(&decInfo->extn_secret_file[i],decInfo->image_data);
    }

    decInfo->extn_secret_file[i]='\0';

    int len = strlen(decInfo->output_fname);

    if(strcmp(decInfo->extn_secret_file,&decInfo->output_fname[len - decInfo->extn_secret_file_length])==0)
    {
        printf(GREEN "DONE : Secret file extension verified successfully.\n" RESET);

        return e_success;
    }

    printf(RED "ERROR : Secret file extension does not match.\n" RESET);

    return e_failure;
}


/* Reads the hidden file size and writes each decoded character to the output.
 * Important: fputc(ch, decInfo->fptr_output) saves each recovered character.
 */
Status decode_file(DecodeInfo* decInfo)
{
    /* Decode the secret file data from the stego image */
    printf(MAGENTA "INFO : Decoding secret file data...\n" RESET);
    usleep(300000);

    char buffer[32];
    fread(buffer,1,32,decInfo->fptr_src_image);

    decoding_lsb_to_size(&decInfo->size_secret_file,buffer);

    char ch;

    for(int i=0;i<decInfo->size_secret_file;i++)
    {
        fread(decInfo->output_data,1,8,decInfo->fptr_src_image);
        decoding_lsb_to_byte(&ch,decInfo->output_data);
        fputc(ch,decInfo->fptr_output);
    }

    printf(GREEN "DONE : Secret file decoded successfully.\n" RESET);

    return e_success;
}