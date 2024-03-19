/*
 *     conversion.c
 *     by Abhinav Mummameni (amumma01) and Rolando Ortega (rorteg02)
 *
 *     Implementation of the conversion functions for both compressing and 
 *     decompressing ppm images.
 */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "assert.h"
#include "except.h"
#include "pnm.h"
#include "a2plain.h"
#include "mem.h"
#include "conversion.h"
#define A2 A2Methods_UArray2
#define BlockWidth 2
#define BlockHeight 2
/* Definitions for indices of fields of rgb values */
#define RED  0
#define GREEN  1
#define BLUE  2
/* Definitions for indices of fields of Color Space values */
#define LUMA 0
#define PB 1
#define PR 2

floating RGB_TO_COLORSPACE[3][3] = {
        {0.299, 0.587, 0.114},
        {-0.168736, -0.331264, 0.5},
        {0.5, -0.418688, -0.081312},
};

floating COLORSPACE_TO_RGB[3][3] = {
        {1.0, 0.0, 1.402},
        {1.0, -0.344136, -0.714136},
        {1.0, 1.772, 0.0}
};

floating LUMAS_TO_DCT[4][4] = {
        {0.25, 0.25, 0.25, 0.25},
        {-0.25, -0.25, 0.25, 0.25},
        {-0.25, 0.25, -0.25, 0.25},
        {0.25, -0.25, -0.25, 0.25},
};

floating DCT_TO_LUMAS[4][4] = {
        {1.0, -1.0, -1.0, 1.0},
        {1.0, -1.0, 1.0, -1.0},
        {1.0, 1.0, -1.0, -1.0},
        {1.0, 1.0, 1.0, 1.0},
};
/***************************MatfMultiply**********************************
*
*  Performs a Matrix Vector multiplication and returns the result
*
* Parameters: floating (*mat)[]: A square matrix of floating type elements
*             Vecf vec: The vector to multiply
              int size: The size of the vector which should be the same as the 
                size of the matrix.
*
* Expects: The matrix and vector should both be of size, size.
           The caller must free the resulting vector.
*
* Return: Returns the result of the matrix vector multiplication, a new vector.
*
*********************************************************************/
Vecf MatfMultiply(floating (*mat)[], Vecf vec, int size) 
{
        floating (*matrix)[size] = mat;
        Vecf result = vec_new(size);
        for (int i = 0; i < size; i++) {
                result[i] = 0.0;
                for (int j = 0; j < size; j++) {
                        result[i] += (matrix[i][j] * vec[j]);
                }
        }
        return result;
}

/***************************pixel_to_rgb**********************************
*  Converts a Pnm_rgb pixel to a Vec3f rgb
* Parameters: Pnm_rgb pixel: The pixel that should be converted.
              unsigned denominator: The denominator of the image which the pixel
                belongs to.
* Expects: Denominator should be non-zero and pixel should be a valid Pnm_rgb
           The caller must free the resulting rgb Vec3f.
* Return: The converted floating number representation of the pixel as a 
           Vec3f rgb.
*********************************************************************/
Vec3f pixel_to_rgb(Pnm_rgb pixel, unsigned denominator) 
{
        Vec3f rgb = vec_new(3);
        rgb[RED] = ((floating)pixel->red / (floating) denominator);
        rgb[GREEN] = ((floating)pixel->green / (floating) denominator);
        rgb[BLUE] = ((floating)pixel->blue / (floating) denominator);

        return rgb;
}

/***************************************************************************
The following 4 functions are wrappers of MatfMultiply for specific use cases.
****************************************************************************/

/***************************rgb_to_cspace**********************************
* Converts a vector containing rgb values to a vector containing cspace values.
* Parameters: Vec3f rgb - a vector containing rgb values
* Expects: rgb should be a valid Vec3f representinng an rgb
* Return: A Vec3f of ColorSpace values.
*********************************************************************/
Vec3f rgb_to_cspace(Vec3f rgb) 
{
        return MatfMultiply(RGB_TO_COLORSPACE, rgb, 3);
}

/***************************lumas_to_dct**********************************
* Converts a vector containing 4 luma values to a vector containing the Discrete
 Cosine Transformation coefficients.
* Parameters: Vec4f lumas - a vector containing luma values
* Expects: lumas should be a valid Vec4f containing 4 luma values
* Return: A Vec4f of dct coefficients.
*********************************************************************/
Vec4f lumas_to_dct(Vec4f lumas) 
{
        return MatfMultiply(LUMAS_TO_DCT, lumas, 4);
}

/***************************dct_to_lumas**********************************
* Converts a vector containing 4 dct coefficients to a vector containing 4 luma
values
* Parameters: Vec4f dct - a vector containing dct coefficients
* Expects: dct should be a valid Vec4f representing dct values
* Return: A Vec4f of 4  luma values.
*********************************************************************/
Vec4f dct_to_lumas(Vec4f dct) 
{
        return MatfMultiply(DCT_TO_LUMAS, dct, 4);
}

/***************************cspace_to_rgb**********************************
* Converts a vector containing cspace values to a vector containing rgb values.
* Parameters: Vec3f cspace - a vector containing Color Space values
* Expects: cspace should be a valid Vec3f with Color Space values
* Return: A Vec3f of rgb values.
*********************************************************************/
Vec3f cspace_to_rgb(Vec3f cspace) 
{
        return MatfMultiply(COLORSPACE_TO_RGB, cspace, 3);
}

/***************************rgb_to_pixel**********************************
*  Updates a Pnm_rgb pixel to hold the scaled version of a Vec3f rgb
* Parameters: Vec3f rgb: A vector containing rgb vaues
              Pnm_rgb pixel: The pixel that should be converted.
              unsigned denominator: The denominator with which the pixel should 
              be scaled 
* Expects: Denominator should be non-zero and pixel should be a valid Pnm_rgb
* Return: void
*********************************************************************/
void rgb_to_pixel(Vec3f rgb, unsigned denominator, Pnm_rgb pixel)
{
        pixel->red = round(rgb[RED] * (floating) denominator);
        pixel->green = round(rgb[GREEN] * (floating) denominator);
        pixel->blue = round(rgb[BLUE] * (floating) denominator);
}

/***************************vec_new**********************************
* Creates a new Vecf object of size size
* Parameters: unsigned size: The size of the vector.
* Expects: CRE if size = 0. The caller must free the resulting Vecf.
* Return: A Vecf with memory allocated to hold size values.
*********************************************************************/
Vecf vec_new(unsigned size)
{
        assert(size > 0);
        return ALLOC(sizeof(floating) * size);
}

/***************************makeBlock**********************************
* Creates a CSBlock
* Parameters: none
* Expects: none
* Return: A pointer to a CSBlock
*********************************************************************/
CSBlock makeBlock() 
{
        return ALLOC(sizeof(Vecf) * BlockHeight * BlockWidth);
}

/***************************deleteBlock**********************************
* Deletes a CSBlock
* Parameters: CSBlock block: The block to be deleted
* Expects: block should be a valid block with valid Vecf elements
* Return: void
*********************************************************************/
void deleteBlock(CSBlock block) 
{
        for (int i = 0; i < BlockHeight * BlockWidth; i++) {
                FREE(block[i]);
        }
        FREE(block);

}

/***************************Compress_CSBlock**********************************
*
* Compresses CSBlock cs_block into Compressed compressed. Computes the dct
coefficients and the average pb and pr values.
*
* Parameters: CSBlock cs_block: The block to compress.
*
* Expects: cs_block should be a valid block
*
* Return: Returns a Compressed object which contains the dct coefficients and 
average pb and pr values.
*
*********************************************************************/
Compressed Compress_CSBlock(CSBlock cs_block)
{
        Vecf lumas = vec_new(BlockHeight * BlockWidth);
        floating total_pb = 0.0;
        floating total_pr = 0.0;
        for (int i = 0; i < BlockHeight * BlockWidth; i++) {
                lumas[i] = cs_block[i][LUMA];
                total_pb += cs_block[i][PB];
                total_pr += cs_block[i][PR];
        }
        Compressed compressed;
        compressed.dct_coeffs = lumas_to_dct(lumas); 
        FREE(lumas);

        compressed.avg_pb = total_pb / ((floating) (BlockHeight * BlockWidth));
        compressed.avg_pr = total_pr / ((floating) (BlockHeight * BlockWidth));
        return compressed;
}

/***************************getCompressed*********************************
*
* Converts the 2x2 block of pixels at the given column and row in the array 
* into color space values. 
*
* Parameters: A2 array: a UArray2 that stores the decompressed pixels
*             A2Methods_T methods: a methods suite that contains the functions 
*                               for a UArray2
*             int col: a column in the UArray2
*             int row: a column in the UArray2
*             unsigned denominator: the denominator for the decompressed image 
*
* Expects: None
* 
* Returns: A CSBlock that contains the information for each pixel in the block.
*
* Notes: Relies on dct_to_lumas to extract the luma values for each pixel and 
*               makeBlock and vec_new to create and update a CSBlock. Frees the 
*               memory needed for creating the vector of luma values. 
*
*********************************************************************/
Compressed getCompressed(A2 array, A2Methods_T methods, int col, int row, 
                        unsigned denominator) 
{
        assert(array != NULL);
        /* Create block to contain information about pixels */
        CSBlock cs_block = makeBlock();
        int i = 0;

        /* Traverse the gibenn block, convert RGB pixel values to color space */
        for (int block_row = 0; block_row < BlockHeight; block_row++) {
                for (int block_col = 0; block_col < BlockWidth; block_col++) {
                        Pnm_rgb pixel = methods->at(array, col + block_col, 
                                        row + block_row);
                        Vec3f rgb = pixel_to_rgb(pixel, denominator);
                        cs_block[i] = rgb_to_cspace(rgb);
                        FREE(rgb);
                        i++;
                }
        }

        /* Convert information to lumas and pb and pr averages */
        Compressed compressed = Compress_CSBlock(cs_block);
        deleteBlock(cs_block);
        return compressed;
}

/***************************Decompress_CSBlock*********************************
*
* Extracts the luma and average pb and pr values for each pixel in the given 
* compressed block. 
*
* Parameters: Compressed compressed: a block of compressed pixels thats contains
*                      data pertaining to the lumas and pb and pr values for
*                      each pixel in the block. 
*
* Expects: None
* 
* Returns: A CSBlock that contains the information for each pixel in the block.
*
* Notes: Relies on dct_to_lumas to extract the luma values for each pixel and 
*               makeBlock and vec_new to create and update a CSBlock. Frees the 
*               memory needed for creating the vector of luma values. 
*
*********************************************************************/
CSBlock Decompress_CSBlock(Compressed compressed) {
        /* Extract the luma values from the compressed block */
        Vec4f lumas = dct_to_lumas(compressed.dct_coeffs);
        CSBlock cs_block = makeBlock();

        /* Get the information for each pixel in the block */
        for (int i = 0; i < BlockHeight * BlockWidth; i++) {
                cs_block[i] = vec_new(3);
                cs_block[i][LUMA] = lumas[i];
                cs_block[i][PB] = compressed.avg_pb;
                cs_block[i][PR] = compressed.avg_pr;
        }
        FREE(lumas);
        return cs_block;
}

/***************************setPixels**********************************
*
* Decompresses a block of pixels at the given column and row of an image and 
* stores the decompressed pixels in a UArray2.
*
* Parameters: A2 array: a UArray2 that stores the decompressed pixels
*             A2Methods_T methods: a methods suite that contains the functions 
*                               for a UArray2
*             int col: a column in the UArray2
*             int row: a column in the UArray2
*             unsigned denominator: the denominator for the decompressed image
*             Compressed compressed: data from a code word that will be 
*                               further decompressed. 
*
* Expects: That array is not NULL
**
* Returns: None, but will update the given array to contain the pixels in the
*               2x2 block at the given index. 
*
* Notes: Checked runtime error if array is NULL. Relies on the Decompress_CSBl-
*               ock, cspace_to_rgb, and rgb_to_pixel functions to further
*               steps of decompressing the pixels. e
*
*********************************************************************/
void setPixels(A2 array, A2Methods_T methods, int col, int row, 
                        unsigned denominator, Compressed compressed)
{
        assert(array != NULL);
        /* Extract lumas and averages from the code word */
        CSBlock cs_block = Decompress_CSBlock(compressed);

        /* For each pixel in the block, decompress and add to array */
        int i = 0;
        for (int block_row = 0; block_row < BlockHeight; block_row++) {
                for (int block_col = 0; block_col < BlockWidth; block_col++) {
                        Vec3f rgb = cspace_to_rgb(cs_block[i]);
                        Pnm_rgb pixel = methods->at(array, col + block_col, 
                                        row + block_row);
                        rgb_to_pixel(rgb, denominator, pixel);
                        FREE(rgb);
                        i++;
                }
        }
        deleteBlock(cs_block);

}

#undef A2
#undef BlockWidth
#undef BlockHeight
#undef RED
#undef GREEN
#undef BLUE
#undef LUMA
#undef PB
#undef PR