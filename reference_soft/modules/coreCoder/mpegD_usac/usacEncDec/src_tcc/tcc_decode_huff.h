/*
 * tcc_decode_huff.h
 *
 *  This file contains functions for Huffman decoding
 *
 *
 *	 This software module was originally developed by
 *
 *		Zylia Sp. z o.o.
 *
 *		Authors:
 *			Andrzej Ruminski ( andrzej.ruminski@zylia.pl )
 *			Lukasz Januszkiewicz ( lukasz.januszkiewicz@zylia.pl )
 *			Marzena Malczewska ( marzena.malczewska@zylia.pl )
 *
 *	 in the course of development of the ISO/IEC 23003-3 for reference purposes and its
 * 	 performance may not have been optimized. This software module is an implementation
 * 	 of one or more tools as specified by the ISO/IEC 23003-3 standard. ISO/IEC gives
 *	 you a royalty-free, worldwide, non-exclusive, copyright license to copy, distribute,
 *	 and make derivative works of this software module or modifications  thereof for use
 *	 in implementations or products claiming conformance to the ISO/IEC 23003-3 standard
 *	 and which satisfy any specified conformance criteria. Those intending to use this
 *	 software module in products are advised that its use may infringe existing patents.
 *	 ISO/IEC have no liability for use of this software module or modifications thereof.
 *	 Copyright is not released for products that do not conform to the ISO/IEC 23003-3
 *	 standard.
 *
 *	 Zylia Sp. z o.o. retains full right to modify and use the code for its
 *	 own purpose, assign or donate the code to a third party and to inhibit third parties
 *	 from using the code for products that do not conform to MPEG-related ITU Recommenda-
 *	 tions and/or ISO/IEC International Standards.
 *
 *	 This copyright notice must be included in all copies or derivative works.
 *
 *	 Copyright (c) ISO/IEC 2016.
 */
#ifndef HFSC_HUFF_H__
#define HFSC_HUFF_H__

#include "bitmux.h"
#include "ct_sbrbitb.h"

#include <stdio.h>
#include <stdlib.h>


#define NO_ESC   100000   /* large value to omit selection of ESC in the Huffman tables */
#define HUFF_SIGNED   1
#define HUFF_UNSIGNED 0

typedef struct STccHuffBook_t
{
            int index;      /* encoded value*/
   unsigned int length;     /* number of bits */
   unsigned int cw;         /* codeword */
} STccHuffBook;

typedef struct STccHuffTab_t
{
   unsigned int   escIndex;
   unsigned int   escLength;
   unsigned int   escCodeWord;
   unsigned int   runBits;       /* number of bits used to store PCM encoded value (after ESC code) */
   unsigned int   tabLength;     /* number of codewords */
   unsigned int   isTabSigned;   /* table can consists signed or unsigned values */
   unsigned int   maxLength;     /* maximal length of huff word */
   STccHuffBook* hcw;
} STccHuffTab;

unsigned int tccHuffmanDecode   (HANDLE_BIT_BUFFER bitStream, STccHuffTab* huffTab);
unsigned int tccHuffmanDecodeCW (HANDLE_BIT_BUFFER bitStream, STccHuffTab* huffTab, int tab[]);

#endif

