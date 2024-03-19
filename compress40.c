/*
 *     compress40.c
 *     by Abhinav Mummameni (amumma01) and Rolando Ortega (rorteg02)
 *
 *     Implementation of the compress40 functions for both compressing and 
 *     decompressing images 
 */

#include <stdlib.h>
#include <stdio.h>
#include <mem.h>
#include <assert.h>
#include <except.h>
#include <math.h>
#include "compress40.h"
#include "pnm.h"
#include "a2methods.h"
#include "a2plain.h"
#include "arith40.h"
#include "bitpack.h"
#include "conversion.h"

#define A2 A2Methods_UArray2
typedef void (*MapFunc) (A2 arr, A2Methods_T methods, unsigned denominator, 
                        int col, int row, void *cl);
/****************** Helper functions and exceptions *******************/
void map_2by2(A2 arr, A2Methods_T methods, unsigned denominator, MapFunc func, 
                void *cl);
void encode_2by2(A2 arr, A2Methods_T methods, unsigned denominator, int col, 
                                int row, void *cl);
void pack_block(Compressed compressed);
int scale_DCT(double coefficient);
uint64_t make_codeword(unsigned pb_bar, unsigned pr_bar, unsigned int a, int b,
                        int c, int d);
Compressed decode_codeword(uint64_t codeword);
void decode_2by2(A2 arr, A2Methods_T methods, unsigned denominator, int col, 
                                int row, void *cl);
double unscale_DCT(int scaled);

Except_T SHORT_FILE = { "Supplied file is too short" };

/***************************compress40**********************************
*
* Function that reads in a file provided by the client and begins in the 
* compression process. 
*
* Parameters: FILE *fp: a file containing data for an image
*
* Expects: Expects that the file pointer is not NULL, which is checked in the 
*          main function of 40image
*
* Return: nothing
*
* Notes: Relies on the pnm.h interface to read in the contents of the file and
*        the map_2by_2 function to compress images. Is called on by the main
*        function in 40image to handle compression
*********************************************************************/
void compress40(FILE *input)
{
        /* Build Pnm_ppm and handle odd-numbered dimensions*/
        A2Methods_T methods = uarray2_methods_plain;
        Pnm_ppm source = Pnm_ppmread(input, methods);
        int trimmed_width = source->width - source->width % 2;
        int trimmed_height = source->height - source->height % 2; 

        /* Print the compressed file header and compress the given file */
        printf("COMP40 Compressed image format 2\n%u %u\n", trimmed_width, 
                trimmed_height);
        map_2by2(source->pixels, methods, source->denominator, encode_2by2, 
                        NULL);

        Pnm_ppmfree(&source);
}

/******************************decompress40**********************************
*
* Function that takes the given input file and begins the decompression process

* Parameters: FILE *fp: an input file containing the data for a given image
*
* Expects: That the given input file is not NULL, which is already checked in 
*          the main function for 40image. 
*
* Return: nothing
*
* Notes: Calls on decode_2by2 to decompress the image and relies on the pnm.h
*        interface to write the decompressed file to standard output
*********************************************************************/
void decompress40(FILE *input)
{
        /* Read in the file header */
        unsigned height, width;
        int read = fscanf(input, "COMP40 Compressed image format 2\n%u %u", 
                                &width, &height);
        assert(read == 2);
        int c = getc(input);
        assert(c == '\n');

        /* Decode the image and store in UArray2 */
        A2Methods_T methods = uarray2_methods_plain;
        A2 decompressed = methods->new(width, height, sizeof(struct Pnm_rgb));
        assert(decompressed != NULL);
        map_2by2(decompressed, methods, 255, decode_2by2, input);

        /* Create new Pnm_ppm & write decompressed image to standard output */
        Pnm_ppm output;
        NEW(output);
        output->denominator = 255;
        output->width = width;
        output->height = height;
        output->pixels = decompressed;
        output->methods = methods;

        Pnm_ppmwrite(stdout, output);
        Pnm_ppmfree(&output);
}

/*****************************map_2by2**********************************
*
* Function that maps each two by two pixel block and either compressed or 
* decompresses, depending on the passed in apply function
*
* Parameters: A2 arr: a UArray2 that contains all of the pixels from the input
*                     file
*             A2Methods_T methods: a methods suite for the UArray2
*             unsigned denominator: the denominator for the rgb values of each
*                     pixel, for calculating decompression
*             MapFunc func: an apply function
*             void *cl: a closure for the mapping function 
*
* Expects: Expects that arr is not NULL and will throw a checked runtime error
*                   if it is. 
*
* Return: nothing, but will map the array and call the given functionfor
*         either compression or decompression
*
* Notes: Apply functions for this function will either by encode_2by2 or 
*        decode_2by2
*********************************************************************/
void map_2by2(A2 arr, A2Methods_T methods, unsigned denominator, MapFunc func, 
                void *cl)
{
        /* Check array is not NULL */
        assert(arr != NULL);
        int width = methods->width(arr);
        int height = methods->height(arr);

        /* Map through each 2x2 block in the array in row-major order */
        for (int row = 0; row < height - 1; row += 2) {
                for (int col = 0; col < width - 1; col += 2 ) {
                        func(arr, methods, denominator, col, row, cl);
                }
        }
}

/*****************************encode_2by2**********************************
*
* Function that compressed and packs the block for the pixel at the given row
* and column 
*
* Parameters: A2 arr: a UArray2 that contains all of the pixels from the input
*                     file
*             A2Methods_T methods: a methods suite for the UArray2
*             unsigned denominator: the denominator for the rgb values of each
                      pixel, for calculating decompression
              int col: a column in the array
              int row: a row in the array
*
* Expects: Expects that arr is not NULL and will throw a checked runtime error
*                   if it is. 
*
* Return: nothing, but will result in a compressed image
*
* Notes: relies on the getCompressed function in the conversion.h interface to
*                   create a compressed block of pixels. Calls on pack_block to
*                   pack the values in the block and print them. Used as an 
*                   apply function for map_2by2 when undergoing compression
*********************************************************************/
void encode_2by2(A2 arr, A2Methods_T methods, unsigned denominator, int col, 
                                int row, void *cl)
{
        /* Check array is not NULL */
        (void) cl;
        assert(arr != NULL);
        Compressed compressed = getCompressed(arr, methods, col, row, 
                        denominator);
        pack_block(compressed);
}

/*****************************pack_block**********************************
*
* Creates the code word for the given pixel block and prints the code out
*
* Parameters: Compressed compressed: a 2x2 block of pixels 
*
* Expects: None
*
* Return: nothing, but prints code word to standard output 
*
* Notes: relies on the Arith40 interface to create the indices for the chroma, 
*        and the scale_DCT and make_codeword functons to scale the DCT 
*        coefficients and make the code word, respectively.
*
*********************************************************************/
void pack_block(Compressed compressed)
{
        /* Create indices for chroma */
        unsigned pb_bar = Arith40_index_of_chroma(compressed.avg_pb);
        unsigned pr_bar = Arith40_index_of_chroma(compressed.avg_pr);

        /* Scale coefficients to discrete cosine */
        Vec4f dct_coeffs = compressed.dct_coeffs;
        unsigned a_scaled = round(dct_coeffs[0] * 63.0);
        int b_scaled = scale_DCT(dct_coeffs[1]);
        int c_scaled = scale_DCT(dct_coeffs[2]);
        int d_scaled = scale_DCT(dct_coeffs[3]);
        FREE(dct_coeffs);

        /* Create code word and print in big endian order */
        uint64_t codeword = make_codeword(pb_bar, pr_bar, a_scaled, b_scaled, 
                                                c_scaled, d_scaled);
        for (int i = 3; i >= 0; i--) {
                putchar(((unsigned char *) (&codeword))[i]);
        }
}

/*****************************scale_DCT**********************************
*
* Converts the given coefficient to a DCT value
*
* Parameters: double coefficient: the coefficient for a particular pixel 
*
* Expects: None
*
* Return: an integer containing the scaled value of the coefficient  
*
*********************************************************************/
int scale_DCT(double coefficient)
{
        /* If coefficient is range, set it to 0.3 or -0.3*/
        if (coefficient > 0.3) {
                coefficient = 0.3;
        }

        else if (coefficient < -0.3) {
                coefficient = -0.3;
        }

        /* Scale coefficient and return */
        double scaled = coefficient * 103.33;
        
        return round(scaled);
}

/*****************************make_codeword**********************************
*
* Creates the code word for the given pixel block and prints the code out
*
* Parameters: unsigned pb_bar: the average pb value for a pixel block
*             unsigned pr_bar: the average pr value for a pixel block
*             unsigned int a: the DCT value for the a coefficient
*             unsigned int b: the DCT value for the b coefficient
*             unsigned int c: the DCT value for the c coefficient
*             unsigned int d: the DCT value for the d coefficient
*
* Expects: None
*
* Return: a 64-bit unsigned int that contains the code word 
*
* Notes: relies on bitpack interface to create the code word from the given
*        values
*
*********************************************************************/
uint64_t make_codeword(unsigned pb_bar, unsigned pr_bar, unsigned int a, int b,
                        int c, int d)
{
        /* Create code word and add values to the word */
        uint64_t codeword = 0;

        codeword = Bitpack_newu(codeword, 6, 26, a);
        codeword = Bitpack_news(codeword, 6, 20, b);
        codeword = Bitpack_news(codeword, 6, 14, c);
        codeword = Bitpack_news(codeword, 6, 8, d);
        codeword = Bitpack_newu(codeword, 4, 4, pb_bar);
        codeword = Bitpack_newu(codeword, 4, 0, pr_bar);

        return codeword;
}

/*****************************decode_2by2**********************************
*
* Decompresses the pixels from the provided file and stores the results in the
* given UArray2
*
* Parameters: A2 arr: an UArray2 to store the decompressed values
*             A2Methods_T methods: a methods suite for the UArray2
*             unsigned denominator: the denominator for the new image
*             int col: a column for a pixel in the array
*             int row: a row for a pixel in the array
*             FILE *fp: a file that contains a compressed image
*
* Expects: Expects that the UArray2 is not NULL, which is checked in the 
*          decompress40 function
*
* Return: none, but updates the UArray2 to contain the decompressed values
*
* Notes: Relies on the conversion interface to unpack the code word and store
*        the values in the array. Calls decode_codeword to decode the code word 
*        beforehand. 
*
*********************************************************************/
void decode_2by2(A2 arr, A2Methods_T methods, unsigned denominator, 
        int col, int row, void *cl)
{
        /* Map through each 2x2 block in the UArray2 and get code words*/
        assert(arr != NULL);
        FILE *fp = cl;
        uint32_t codeword = 0;
        for (int i = 3 ; i >= 0; i--) {
                int read = fgetc(fp);
                if (read == EOF) {
                        RAISE(SHORT_FILE);
                }
                ((unsigned char *) (&codeword))[i] = (unsigned char) read;
        }

        /* Decode code word and add pixels to array */
        Compressed compressed = decode_codeword(codeword);

        setPixels(arr, methods, col, row, denominator, compressed);
        FREE(compressed.dct_coeffs);

}

/*****************************decode_codeword**********************************
*
* Unpacks the given code word, unscales the DCT values, and converts the indic-
* es back to chroma
*
* Parameters: uint64_t codeword: a codeword to decode
*
* Expects: None
*
* Return: a "Compressed" vector of doubles that contains the decompressed values
*
* Notes: Relies on the bitpack interface to get the values from the code word
*        and the Arith40 interface to get the average chroma from the indices
*
*********************************************************************/
Compressed decode_codeword(uint64_t codeword)
{
        /* Get values from the code word */
        uint64_t a_scaled = Bitpack_getu(codeword, 6, 26);
        int64_t b_scaled = Bitpack_gets(codeword, 6, 20);
        int64_t c_scaled = Bitpack_gets(codeword, 6, 14);
        int64_t d_scaled = Bitpack_gets(codeword, 6, 8);
        uint64_t pb_bar = Bitpack_getu(codeword, 4, 4);
        uint64_t pr_bar = Bitpack_getu(codeword, 4, 0);

        /* Unscale the coefficients and place in new compressed vector */
        Compressed compressed;
        compressed.dct_coeffs = vec_new(4);
        compressed.dct_coeffs[0] = a_scaled / 63.0;
        compressed.dct_coeffs[1] = unscale_DCT(b_scaled);
        compressed.dct_coeffs[2] = unscale_DCT(c_scaled);
        compressed.dct_coeffs[3] = unscale_DCT(d_scaled);

        compressed.avg_pb = Arith40_chroma_of_index(pb_bar);
        compressed.avg_pr = Arith40_chroma_of_index(pr_bar);

        return compressed;
}

/*****************************unscale_DCT**********************************
*
* Reverses the scaling of the DCT value 
*
* Parameters: int scaled: a scale DCT value to scale back down
*
* Expects: None
*
* Return: a double containing the unscaled coefficient of the DCT
*
*********************************************************************/
double unscale_DCT(int scaled) 
{
        /* Unscale value and return */
        return ((double) scaled / 103.33);
}

#undef A2