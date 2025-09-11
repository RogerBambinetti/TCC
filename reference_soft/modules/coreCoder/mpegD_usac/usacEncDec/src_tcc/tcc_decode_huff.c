/*
 * tcc_decode_huff.c
 *
 *  This file contains implementation of Huffman decoding
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

#include "tcc_decode_huff.h"

/*
 * core huffman decoding
 */
unsigned int tccHuffmanDecodeCW(HANDLE_BIT_BUFFER bitStream, STccHuffTab* huffTab, int tab[])
{
   unsigned int index;
   unsigned long cw, bit, len;

   STccHuffBook* huffBook = huffTab->hcw;

   len = 0;
   cw  = 0;
   while( len <= huffTab->maxLength )
   {
      /* BsGetBit(bitStream, &bit, 1); */
      bit = BufGetBits(bitStream, 1);
      cw <<= 1;
      cw |= bit;
      len += 1;
	  /*printf( "\nBit %d\n", bit );*/
      for(index=0; index < huffTab->tabLength; index++)
      {
         if ( (cw == huffBook[index].cw) && (len == huffBook[index].length ) )
         {
            tab[1] = huffBook[index].index;
            tab[0] = huffBook[index].length;             /*// huuftable can store values different then table index*/
            return len;
         }
      }

   }

   return 0;
}

/**
   Decode single Huffman code word
   out: decode value
        -1: error
*/
unsigned int tccHuffmanDecode(HANDLE_BIT_BUFFER bitStream, STccHuffTab* huffTab)
{
   /*printf( "Huffman\n" );*/
   int index, sign, numBits;

   int bvTab[] = {-1, -1}; /* {bits, value}*/

   numBits = tccHuffmanDecodeCW(bitStream, huffTab, bvTab);

   if( numBits == 0 ) {
		fprintf( stderr, "CoreDecoder: Invalid TCC bitstream\n" );
   }

   if (bvTab[1] == huffTab->escIndex)
   {
      /*BsGetBit(bitStream, &index, huffTab->runBits);*/
      index = BufGetBits(bitStream, huffTab->runBits);
      bvTab[1]  = index;
      bvTab[0] += huffTab->runBits;
   }

   if( huffTab->isTabSigned )
   {
      /*BsGetBit(bitStream,&sign,1);*/
      sign = BufGetBits(bitStream, 1);

      if(sign) {
         bvTab[1] *= -1;
      }

      bvTab[0] += 1;
   }

   /*printf("(%d,%d) ", bvTab[0], bvTab[1]);*/
   return bvTab[1];
}

