/*
 *     a2plain.c
 *     by Abhinav Mummameni (amumma01) and Rolando Ortega (rorteg02)
 *
 *     Implementation of private methods on plain UArray2s for the A2Methods 
 *     methods suite 
 */

#include <string.h>

#include <a2plain.h>
#include "uarray2.h"
#include <stdio.h>
typedef A2Methods_UArray2 A2;

/*************new*******************************************
*
* Function that initializes an empty UArray2 with the specified         
* dimensions
* 
* Parameters: int witdth: the width of the UArray2
*             int height: the height of the UArray2
*             int size: the size of an element in the UArray2
*
* Return: an new UArray2
*
* Expects: there is space for ELEMENT_SIZE amount of memory can be 
* allocated and that the dimensions are greater than 0.
*      
*********************************************************************/
static A2Methods_UArray2 new(int width, int height, int size)
{
        return UArray2_new(width, height, size);
}

/*************new_with_blocksize****************************
*
* Function that initializes an empty UArray2 with the specified         
* dimensions
* 
* Parameters: int DIM1: the width of the UArray2
*             int DIM2: the height of the UArray2
*             int ELEMENT_SIZE: the size of an element in the UArray2
*
* Return: an new UArray2
*
* Expects: there is space for ELEMENT_SIZE amount of memory can be 
* allocated and that the dimensions are greater than 0.
*      
*********************************************************************/
static A2Methods_UArray2 new_with_blocksize(int width, int height, int size,
                                            int blocksize)
{
        (void) blocksize;
        return UArray2_new(width, height, size);
}

/*************a2free******************************************************
*
* Function that frees the memory of an initialized UArray2
*
* Parameters: A2* : array2p a pointer to a UArray2 that has been initialized
*
* Return: void
*
* Expects: UArray2 a is not NULL
*      
* Notes: does not free the memory of the individual elements inside   
* the array
*         
*********************************************************************/
static void a2free(A2 * array2p)
{
        UArray2_free((UArray2_T *) array2p);
}

/*************width******************************************************
*
* Function that gets the width of a given UArray2
*
* Parameters: A2* : array2p a UArray2 that has been initialized
*
* Return: the width of the array
*
* Expects: array2 is not NULL
*      
*
*********************************************************************/
static int width(A2 array2)
{
        return UArray2_width(array2);
}

/*************height******************************************************
*
* Function that gets the height of a given UArray2
*
* Parameters: A2* : array2p a UArray2 that has been initialized
*
* Return: the height of the array
*
* Expects: array2 is not NULL
*      
*
*********************************************************************/
static int height(A2 array2) 
{
        return UArray2_height(array2);
}

/*************size******************************************************
*
* Function that gets the size of the elements in a UArray2
*
* Parameters: A2* : array2p a UArray2 that has been initialized
*
* Return: the size of the array's elements
*
* Expects: array2  is not NULL
*      
*
*********************************************************************/
static int size(A2 array2)
{
        return UArray2_size(array2);
}

/*************blocksize******************************************************
*
* Function that gets the blocksize of the elements in a UArray2
*
* Parameters: A2* : array2p a UArray2 that has been initialized
*
* Return: the size of the array's blocksize
*
* Expects: array2  is not NULL
*      
* Notes: blocksize of standard UArray2 is always 1, so always returns 1
*********************************************************************/
static int blocksize(A2 array2)
{
        (void) array2;
        return 1;
}

/*************at******************************************************
*
* Function that gets an element at specified indicies in the given uarray2
*
* Parameters: A2* : array2p a UArray2 that has been initialized
*          int col: col index of a UArray2
*          int row: row index of a UArray2
*
* Return: void pointer of element at given index
*
* Expects: array2 is not NULL, indicies are within bounds of array
*      
*
*********************************************************************/
static A2Methods_Object *at(A2 array2, int col, int row)
{
        return UArray2_at(array2, col, row);
}

typedef void applyfun(int i, int j, UArray2_T array2, void *elem, void *cl);

/*************map_row_major***********************************
*
* Traverses the elements in the UArray2 by row and calls the apply         
* function for each element. 
*
* Parameters: A2Methods_UArray2 uarray2: a UArray2 that has been initialized 
*          void apply: a function pointer for a function that will 
*          be called for each element that the map function         
*          accesses. The apply function should take at least a row  
*          i, a column j and the UArray2_T. 
*          void *cl: a closure statement for the map function 
*
* Return: nothing, but apply function affects elements of array and closure
*
*********************************************************************/
static void map_row_major(A2Methods_UArray2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
        UArray2_map_row_major(uarray2, (UArray2_applyfun *)apply, cl);
}

/*************map_col_major***********************************
*
* Traverses the elements in the UArray2 by col and calls the apply         
* function for each element. 
*
* Parameters: A2Methods_UArray2 uarray2: a UArray2 that has been initialized 
*          void apply: a function pointer for a function that will 
*          be called for each element that the map function         
*          accesses. The apply function should take at least a row  
*          i, a column j and the UArray2_T. 
*          void *cl: a closure statement for the map function 
*
* Return: nothing, but apply function affects elements of array and closure
*
*********************************************************************/
static void map_col_major(A2Methods_UArray2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
        UArray2_map_col_major(uarray2, (UArray2_applyfun*)apply, cl);
}

struct small_closure {
        A2Methods_smallapplyfun *apply; 
        void                    *cl;
};

static void apply_small(int i, int j, UArray2_T uarray2,
                        void *elem, void *vcl)
{
        struct small_closure *cl = vcl;
        (void)i;
        (void)j;
        (void)uarray2;
        cl->apply(elem, cl->cl);
}

static void small_map_row_major(A2Methods_UArray2        a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
        struct small_closure mycl = { apply, cl };
        UArray2_map_row_major(a2, apply_small, &mycl);
}

static void small_map_col_major(A2Methods_UArray2        a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
        struct small_closure mycl = { apply, cl };
        UArray2_map_col_major(a2, apply_small, &mycl);
}

/*
Struct storing the functions defined by the A2Methods interface for 2D plain
UArray2s. 
*/
static struct A2Methods_T uarray2_methods_plain_struct = {
        new,
        new_with_blocksize,
        a2free,
        width,
        height,
        size,
        blocksize,
        at, 
        map_row_major,
        map_col_major,
        NULL,
        map_row_major, /*default mapping*/
        small_map_row_major,
        small_map_col_major,
        NULL,
        small_map_row_major, /*small map default*/
};

// finally the payoff: here is the exported pointer to the struct

A2Methods_T uarray2_methods_plain = &uarray2_methods_plain_struct;
