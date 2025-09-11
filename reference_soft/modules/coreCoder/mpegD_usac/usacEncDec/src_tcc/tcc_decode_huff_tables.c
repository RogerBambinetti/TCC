/*
 * tcc_decode_huff_tables.c
 *
 *  This file contains huffman books
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
#include "tcc_decode_huff_tables.h"




STccHuffBook tccBook_AC[] =
{
    /* index, length/bits, deccode, bincode  */
    {       0,      6,       31},  /*           011111   */
    {       1,      3,        5},  /*              101   */
    {       2,      3,        1},  /*              001   */
    {       3,      3,        2},  /*              010   */
    {       4,      3,        4},  /*              100   */
    {       5,      3,        7},  /*              111   */
    {       6,      4,        6},  /*             0110   */
    {       7,      4,       13},  /*             1101   */
    {       8,      5,        2},  /*            00010   */
    {       9,      5,       14},  /*            01110   */
    {      10,      6,        0},  /*           000000   */
    {      11,      6,        2},  /*           000010   */
    {      12,      6,        7},  /*           000111   */
    {      13,      6,       30},  /*           011110   */
    {      14,      6,       50},  /*           110010   */
    {      15,      7,        2},  /*          0000010   */
    {      16,      7,        6},  /*          0000110   */
    {      17,      7,       96},  /*          1100000   */
    {      18,      7,       98},  /*          1100010   */
    {      19,      7,       99},  /*          1100011   */
    {      20,      8,        6},  /*         00000110   */
    {      21,      8,       27},  /*         00011011   */
    {      22,      8,        7},  /*         00000111   */
    {      23,      8,       15},  /*         00001111   */
    {      24,      8,       26},  /*         00011010   */
    {      25,      8,      206},  /*         11001110   */
    {      26,      9,       50},  /*        000110010   */
    {      27,      9,       49},  /*        000110001   */
    {      28,      9,       28},  /*        000011100   */
    {      29,      9,       48},  /*        000110000   */
    {      30,      9,      390},  /*        110000110   */
    {      31,      9,      389},  /*        110000101   */
    {      32,      9,       51},  /*        000110011   */
    {      33,     10,       59},  /*       0000111011   */
    {      34,     10,      783},  /*       1100001111   */
    {      35,      9,      408},  /*        110011000   */
    {      36,     10,      777},  /*       1100001001   */
    {      37,     10,       58},  /*       0000111010   */
    {      38,     10,      782},  /*       1100001110   */
    {      39,      8,      205},  /*         11001101   */
    {      40,      9,      415},  /*        110011111   */
    {      41,     10,      829},  /*       1100111101   */
    {      42,     10,      819},  /*       1100110011   */
    {      43,     10,      828},  /*       1100111100   */
    {      44,     11,     1553},  /*      11000010001   */
    {      45,     11,     1637},  /*      11001100101   */
    {      46,     12,     3105},  /*     110000100001   */
    {      47,     14,    12419},  /*   11000010000011   */
    {      48,     11,     1636},  /*      11001100100   */
    {      49,     14,    12418},  /*   11000010000010   */
    {      50,     13,     6208},  /*    1100001000000   */
};
/* escIndex, escLength, escCodeWord, runBits, tabLength, hcw */
STccHuffTab tccTab_AC = { 50, 13, 6208, BS_TCC_FREQ_COEFF, 51, HUFF_SIGNED, 14, tccBook_AC};


STccHuffBook tccBook_idx[] =
{
   /* index, length/bits, deccode, bincode  */
   {       0,      1,        0},  /*               0  */
   {       1,      3,        6},  /*             110  */
   {       2,      3,        7},  /*             111  */
   {       3,      4,        9},  /*            1001  */
   {       4,      4,       11},  /*            1011  */
   {       5,      5,       17},  /*           10001  */
   {       6,      6,       32},  /*          100000  */
   {       7,      6,       40},  /*          101000  */
   {       8,      6,       42},  /*          101010  */
   {       9,      7,       67},  /*         1000011  */
   {      10,      7,       83},  /*         1010011  */
   {      11,      8,      133},  /*        10000101  */
   {      12,      8,      132},  /*        10000100  */
   {      13,      8,      165},  /*        10100101  */
   {      14,      8,      173},  /*        10101101  */
   {      15,      8,      175},  /*        10101111  */
   {      16,      9,      329},  /*       101001001  */
   {      17,      9,      344},  /*       101011000  */
   {      18,      9,      348},  /*       101011100  */
   {      19,     10,      656},  /*      1010010000  */
   {      20,     10,      698},  /*      1010111010  */
   {      21,     10,      699},  /*      1010111011  */
   {      22,     11,     1380},  /*     10101100100  */
   {      23,     11,     1382},  /*     10101100110  */
   {      24,     11,     1383},  /*     10101100111  */
   {      25,     12,     2628},  /*    101001000100  */
   {      26,     12,     2763},  /*    101011001011  */
   {      27,     12,     2629},  /*    101001000101  */
   {      28,     12,     2631},  /*    101001000111  */
   {      29,     13,     5525},  /*   1010110010101  */
   {      30,     12,     2630},  /*    101001000110  */
   {      31,     13,     5524},  /*   1010110010100  */
};

/* escIndex, escLength, escCodeWord, runBits, tabLength, hcw */
STccHuffTab tccTab_idx = { NO_ESC, NO_ESC, NO_ESC, NO_ESC, 32, HUFF_UNSIGNED, 13, tccBook_idx};
