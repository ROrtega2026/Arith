/*
 *     bitpack.c
 *     by Abhinav Mummameni (amumma01) and Rolando Ortega (rorteg02)
 *
 *     Implementation of the functions for the bitpack interface. Creates and 
 *     adds to words, throws exceptions for widths and/or least significant 
 *     bits that are out of bounds.  
 */

#include <math.h>
#include "bitpack.h"
#include <stdlib.h>
#include <stdio.h>
#include "assert.h"
#include "except.h"
#define WORDSIZE 64
/* Exceptions */
Except_T Bitpack_Overflow = { " Overflow packing bits "};

/***************************Bitpack_fitsu**********************************
*
* Determines whether the given value is valid for the provided width
*
* Parameters: uint64_t n: a value to be inserted into an unsigned word
*             unsigned width: a width for the given value in a unsigned word
*
* Expects: None
*
* Return: Returns true if the width is greater than 64 or if the width is 
*         greater than the value
*
*********************************************************************/
bool Bitpack_fitsu(uint64_t n, unsigned width)
{
        /* Check size of width */
        if (width >= WORDSIZE) {
                return true;
        }

        /* Compare maximum bit for width to the value n */
        uint64_t max = (uint64_t) 1 << width;
        return max > n;
}

/***************************Bitpack_fitss**********************************
*
* Determines whether the given value is valid for the provided width
*
* Parameters: int64_t n: a value to be inserted into a signed word
*             unsigned width: a width for the given value in a signed word
*
* Expects: None
*
* Return: Returns true if the width is greater than 64 or if the width is 
*         greater than the value
*
*********************************************************************/
bool Bitpack_fitss(int64_t n, unsigned width)
{
        /* Check if width is greater than 64 */
        if (width >= WORDSIZE) {
                return true;
        }
        if (width == 0) {
                return (n == -1);
        }
        uint64_t magnitude = (uint64_t) 1 << (width - 1);

        int64_t Min = -1 * magnitude;
        int64_t Max = magnitude;
        /* Compare width with the value n */
        return ((Min <= n) && (n < Max));
        
}

/***************************Bitpack_getu**********************************
*
* Get the unsigned value from the unsigned word that has the given width and 
* least significant bit
*
* Parameters: uint64_t word: the unsigned word to extract a value from 
*             unsigned width: a width for the a value in the word
*             unsigned lsb: the least significant bit fo the value 
*
* Expects: That width is between 0 and 64 and that width + lsb is less than or
*          equal to 64
*
* Return: A 64 bit unsigned integer containing the value with the given width
*         and least significant bit.
*
* Notes: Throws a checked runtime error if the width and least significant bit
*        are invalid. 
*
*********************************************************************/
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
        /* Check that width and lsb are valid */
        assert(width <= WORDSIZE);
        assert((width + lsb) <= WORDSIZE);

        /* Extract value from word and return*/
        uint64_t mask = ~0;
        mask = mask >> (WORDSIZE - width) << lsb;
        return (word & mask) >> lsb;
}

/***************************Bitpack_gets**********************************
*
* Get the signed value from the unsigned word that has the given width and 
* least significant bit
*
* Parameters: unint64_t word: the unsigned word to extract a value from 
*             unsigned width: a width for the a value in the word
*             unsigned lsb: the least significant bit fo the value 
*
* Expects: That width is between 0 and 64 and that width + lsb is less than or
*          equal to 64
*
* Return: A 64 bit signed integer containing the value with the given width
*         and least significant bit.
*
* Notes: Calls on Bitpack_getu to extract the unsigned representation of the 
*        value, and converts accordingly. 
*
*********************************************************************/
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
{
        /* Get the unsigned representation of the value */
        uint64_t getu = Bitpack_getu(word, width, lsb);
        

        /* Convert if the value should be negative */
        if (getu >= pow(2, width - 1) && width != 64) {
                return getu - ((uint64_t)1 << width); 
        }
        return (int64_t) getu;
}

/***************************Bitpack_newu**********************************
*
* Update a 64 bit unsigned word with a new unsigned value with the given width
* and least significant bit
*
* Parameters: uint64_t word: the unsigned word to extract a value from 
*             unsigned width: a width for the a value in the word
*             unsigned lsb: the least significant bit fo the value 
*             uint64_t value: a unsigned value to insert into the word 
*
* Expects: That the given value can fit in the given width, and that the width
*          is between 0 and 64 and width + lsb is less than 64
*
* Return: A 64 bit unsigned integer containing the updated word now containing
*         the given value
*
* Notes: Throws a checked runtime error if the width is unable to contain the 
*        value, which is checked using the Bitpack_fitsu function, or if the 
*        width and/or lsb are invalid.
*
*********************************************************************/
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb, 
                        uint64_t value)
{
        /* Check that given parameters are valid */
        assert(width <= WORDSIZE);
        assert((width + lsb) <= WORDSIZE);
        if (!Bitpack_fitsu(value, width)) {
                RAISE(Bitpack_Overflow);
        }

        /* Update word to contain the given value */
        uint64_t mask = ~0;
        mask = mask >> (WORDSIZE - width) << lsb;
        mask = ~mask;
        return ((word & mask) | (value << lsb));
}

/***************************Bitpack_news**********************************
*
* Update a 64 bit unsigned word with a new signed value with the given width
* and least significant bit
*
* Parameters: uint64_t word: the unsigned word to extract a value from 
*             unsigned width: a width for the a value in the word
*             unsigned lsb: the least significant bit fo the value 
*             int64_t value: a signed svalue to insert into the word 
*
* Expects: That the given value can fit in the given width
*
* Return: A 64 bit signed integer containing the updated word now containing
*         the given value
*
* Notes: Throws a checked runtime error if the width is unable to contain the 
*        value, which is checked using the Bitpack_fitsu function, or if the 
*        width and/or lsb are invalid
*
*********************************************************************/
uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb, 
                        int64_t value)
{
        /* Check that the given parameters are valid */
        assert(width <= WORDSIZE);
        assert((width + lsb) <= WORDSIZE);
        if (!Bitpack_fitss(value, width)) {
                RAISE(Bitpack_Overflow);
        }

        /* Update word to contain the given value */
        uint64_t mask = ~0;
        mask = mask >> (WORDSIZE - width) << lsb;
        uint64_t insertion = mask & (value << lsb);
        mask = ~mask;
        
        return ((word & mask) | insertion);
}
#undef WORDSIZE