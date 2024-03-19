/*
 *     conversion.h
 *     by Abhinav Mummameni (amumma01) and Rolando Ortega (rorteg02)
 *
 *     Functions for converting 2x2 blocks of RGB pixels to DCT values and 
 *     averages, as well as converting DCT values and averages to RGB pixels. 
 *     Relies on the UArray2 interface and a2plain methods suite to get pixels
 *     to compress or store pixels once they are decompressed. Called by
 *     compress40 to handle the calculations behind compressing and decompress-
 *     ing pixels. 
 *
 */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "assert.h"
#include "except.h"
#include "pnm.h"
#include "a2plain.h"
#include "mem.h"
#define A2 A2Methods_UArray2

/* typedefs
* Floating is a floating point number, Vecf is a vector of floating point 
* numbers. Vec3f and Vec4f represent vectors of 3 and 4 numbers respectively.
* CSBlock, short for ColorSpaceBlock, is a block of Vec3f's each of
which represents a color space representation of a pixel. Our implementation 
uses a 2by2 block.
*/
typedef double floating;
typedef floating *Vecf;
typedef floating *Vec3f;
typedef floating *Vec4f;
typedef Vec3f *CSBlock;

/* struct Compressed - Contains the compressed representation of a CSBlock
* dct_coeffs - A Vec4f of the "a, b, c, d" values produced by the discrete 
cosine transform of 4 luma values
* avg_pb - The average Pb value of the CSBlock
* avg_pr - The average Pr value of the CSBlock
*/
typedef struct Compressed {
        Vec4f dct_coeffs;
        floating avg_pb;
        floating avg_pr;
} Compressed;




Vecf vec_new(unsigned size);
void setPixels(A2 array, A2Methods_T methods, int col, int row, 
                        unsigned denominator, Compressed compressed);

Compressed getCompressed(A2 array, A2Methods_T methods, int col, int row, 
                        unsigned denominator);

#undef A2