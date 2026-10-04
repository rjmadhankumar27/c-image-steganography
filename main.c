#include <stdio.h>
#include "encode.h"
#include "decode.h"
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

/*
 * Runs the requested encode or decode operation.
 * Important: check_operation_type(argv) selects which operation to perform.
 */
int main(int argc,char* argv[])
{
    

if(argc<3)
{
    printf(RED "ERROR : Insufficient arguments.\n" RESET);
    return e_failure;
}

OperationType check = check_operation_type(argv);

if(check == e_encode)
{
    if(argc<4)
    {
        printf(RED "ERROR : Insufficient arguments.\n" RESET);
        return e_failure;
    }

    EncodeInfo encInfo;
    Status state = read_and_validate_encode_args(argv, &encInfo);
    
    if(state == e_success)
    {
        state = do_encoding(&encInfo); 

        if(state == e_success)
        {
            printf(GREEN "DONE : Encoding completed successfully.\n" RESET);
            return 0;
        }
    }

    printf(RED "ERROR : Encoding failed.\n" RESET);  
}
else if(check == e_decode)
{
    DecodeInfo decInfo;
    Status state = read_and_validate_decode_args(argv, &decInfo);

    if(state == e_success)
    {
        state = do_decoding(&decInfo);

        if(state == e_success)
        {
            printf(GREEN "DONE : Decoding completed successfully.\n" RESET);

            char ch;
            rewind(decInfo.fptr_output);

            printf(ORANGE "Secret Message : \n" RESET);
            while((ch=fgetc(decInfo.fptr_output))!=EOF)
            {
                fputc(ch,stdout);
            }
            
            printf("\n");
            
            return 0;
        } 
    }
    
    printf(RED "ERROR : Decoding failed.\n" RESET);  
}
else
{
    printf(RED "ERROR : Please enter a valid operation type.\n" RESET);
    return 0;
}

return 0;
}

/*
 * Checks the command-line flag and returns the selected operation.
 * Important: strcmp(argv[1], "-e") / strcmp(argv[1], "-d") detects the flag.
 */
OperationType check_operation_type(char *argv[])
{
    /* Check whether the requested operation is encoding or decoding */
    printf(MAGENTA "INFO : Checking operation type...\n" RESET);
    usleep(300000);

    if(strcmp(argv[1],"-e")==0)
    {
        printf(GREEN "DONE : Encode operation selected.\n" RESET);
        return e_encode;
    }
    else if(strcmp(argv[1],"-d")==0)
    {
        printf(GREEN "DONE : Decode operation selected.\n" RESET);
        return e_decode;
    }
    else
    {
        printf(RED "ERROR : Unsupported operation type.\n" RESET);
        return e_unsupported;
    }
}