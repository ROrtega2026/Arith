/*
 *     40image.c
 *     by Abhinav Mummameni (amumma01) and Rolando Ortega (rorteg02)
 *
 *     Driver main function for the 40image program. Handles commands from the 
 *     command lines and opens files from the command line or standard input.
 *     Produces errors if commands are not valid (are not compression or 
 *     decompression) or if the file provided by the client is invalid. Relies 
 *     on compress40 interface to run either the compression or decompression
 *     program, depending on the command. 
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "assert.h"
#include "compress40.h"

static void (*compress_or_decompress)(FILE *input) = compress40;

/***************************main**********************************
*
* Handles commands provided by clients on the command line and opens an image
* file, either from the command line or standard input.  
*
* Parameters: int argc: the number of arguments given
*             char *argv[]: the command line arguments given
*
* Expects: Expects that the commands are either '-c' or '-d' and that the image
*          file is a proper file. 
*
* Return: An int containing whether the program ran successfully 
*
* Notes: Relies on the compress40 interface to run either the compression or
*        decompression program. Exits if the commands are invalid and throws
*        checked runtime error if file cannot be open. 
*********************************************************************/
int main(int argc, char *argv[])
{
        int i;

        for (i = 1; i < argc; i++) {
                if (strcmp(argv[i], "-c") == 0) {
                        compress_or_decompress = compress40;
                } else if (strcmp(argv[i], "-d") == 0) {
                        compress_or_decompress = decompress40;
                } else if (*argv[i] == '-') {
                        fprintf(stderr, "%s: unknown option '%s'\n",
                                argv[0], argv[i]);
                        exit(1);
                } else if (argc - i > 2) {
                        fprintf(stderr, "Usage: %s -d [filename]\n"
                                "       %s -c [filename]\n",
                                argv[0], argv[0]);
                        exit(1);
                } else {
                        break;
                }
        }
        assert(argc - i <= 1);    /* at most one file on command line */
        if (i < argc) {
                FILE *fp = fopen(argv[i], "r");
                assert(fp != NULL);
                compress_or_decompress(fp);
                fclose(fp);
        } else {
                compress_or_decompress(stdin);
        }

        return EXIT_SUCCESS; 
}
