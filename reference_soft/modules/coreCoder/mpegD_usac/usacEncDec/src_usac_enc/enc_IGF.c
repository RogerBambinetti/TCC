/***********************************************************************************

 This software module was originally developed by

 Fraunhofer IIS

 in the course of development of the ISO/IEC 23008-3 for reference purposes and its
 performance may not have been optimized. This software module is an implementation
 of one or more tools as specified by the ISO/IEC 23008-3 standard. ISO/IEC gives
 you a royalty-free, worldwide, non-exclusive, copyright license to copy, distribute,
 and make derivative works of this software module or modifications  thereof for use
 in implementations or products claiming conformance to the ISO/IEC 23008-3 standard
 and which satisfy any specified conformance criteria. Those intending to use this
 software module in products are advised that its use may infringe existing patents.
 ISO/IEC have no liability for use of this software module or modifications thereof.
 Copyright is not released for products that do not conform to the ISO/IEC 23008-3
 standard.

 Fraunhofer IIS retains full right to modify and use the code for its own purpose,
 assign or donate the code to a third party and to inhibit third parties from using
 the code for products that do not conform to MPEG-related ITU Recommendations and/or
 ISO/IEC International Standards.

 This copyright notice must be included in all copies or derivative works.

 Copyright (c) ISO/IEC 2013.

 ***********************************************************************************/


#include <math.h>
#include <memory.h>
#include <float.h>

#include "usac_mainStruct.h"   /* structs */
#include "usac_mdct.h"
#include "usac_arith_dec.h"
#include "enc_IGF.h"


static unsigned short cf_se00[27] = {
 12240, 11867, 11562, 11189, 10826, 10468, 10124, 9727, 9325, 8981, 8589, 8085, 7533, 6996, 6401, 5854, 5215, 4639, 4053, 3453, 2920, 2426, 1830, 1283, 968, 629, 0
};

static unsigned short cf_se01[27] = {
 16370, 16360, 16350, 16336, 16326, 16283, 16215, 16065, 15799, 15417, 14875, 13795, 12038, 9704, 6736, 3918, 2054, 1066, 563, 311, 180, 98, 64, 20, 15, 5, 0
};

static unsigned short cf_se10[27] = {
 16218, 16145, 16013, 15754, 15426, 14663, 13563, 11627, 8894, 6220, 4333, 3223, 2680, 2347, 2058, 1887, 1638, 1472, 1306, 1154, 1012, 895, 758, 655, 562, 489, 0
};

static unsigned short cf_se02[ 7][27] = {
  { 16332, 16306, 16278, 16242, 16180, 16086, 15936, 15689, 15289, 14657, 13632, 12095, 9926, 6975, 4213, 2285, 1163, 637, 349, 196, 125, 82, 52, 28, 11, 2, 0},
  { 16370, 16367, 16364, 16358, 16350, 16330, 16284, 16170, 16030, 15647, 14840, 13094, 10364, 6833, 3742, 1639, 643, 282, 159, 85, 42, 22, 16, 15, 4, 1, 0},
  { 16373, 16371, 16367, 16363, 16354, 16336, 16290, 16204, 16047, 15735, 14940, 13159, 10171, 6377, 3044, 1212, 474, 208, 115, 60, 27, 14, 7, 6, 5, 1, 0},
  { 16382, 16377, 16367, 16357, 16334, 16281, 16213, 16035, 15613, 14694, 12898, 9720, 5747, 2506, 1030, 469, 251, 124, 58, 48, 35, 17, 12, 7, 6, 5, 0},
  { 16383, 16375, 16374, 16366, 16336, 16250, 16107, 15852, 15398, 14251, 12117, 8796, 5016, 2288, 998, 431, 236, 132, 89, 37, 16, 12, 4, 3, 2, 1, 0},
  { 16375, 16357, 16312, 16294, 16276, 16222, 16133, 15999, 15515, 14655, 13123, 10667, 7324, 4098, 2073, 1141, 630, 370, 209, 93, 48, 39, 12, 11, 10, 9, 0},
  { 16343, 16312, 16281, 16179, 16067, 15730, 15464, 15025, 14392, 13258, 11889, 10224, 7824, 5761, 3902, 2349, 1419, 837, 520, 285, 183, 122, 71, 61, 40, 20, 0}
};

static unsigned short cf_se20[ 7][27] = {
  { 16351, 16344, 16317, 16283, 16186, 16061, 15855, 15477, 14832, 13832, 12286, 10056, 7412, 4889, 2996, 1739, 1071, 716, 496, 383, 296, 212, 149, 109, 82, 59, 0},
  { 16368, 16352, 16325, 16291, 16224, 16081, 15788, 15228, 14074, 12059, 9253, 5952, 3161, 1655, 1006, 668, 479, 357, 254, 199, 154, 115, 88, 67, 51, 45, 0},
  { 16372, 16357, 16339, 16314, 16263, 16169, 15984, 15556, 14590, 12635, 9475, 5625, 2812, 1488, 913, 641, 467, 347, 250, 191, 155, 117, 89, 72, 59, 46, 0},
  { 16371, 16362, 16352, 16326, 16290, 16229, 16067, 15675, 14715, 12655, 9007, 5114, 2636, 1436, 914, 650, 477, 357, 287, 227, 182, 132, 105, 79, 58, 48, 0},
  { 16364, 16348, 16318, 16269, 16192, 16033, 15637, 14489, 12105, 8407, 4951, 2736, 1669, 1156, 827, 615, 465, 348, 269, 199, 162, 125, 99, 73, 51, 37, 0},
  { 16326, 16297, 16257, 16136, 15923, 15450, 14248, 11907, 8443, 5432, 3396, 2226, 1561, 1201, 909, 699, 520, 423, 323, 255, 221, 163, 121, 87, 71, 50, 0},
  { 16317, 16280, 16203, 16047, 15838, 15450, 14749, 13539, 11868, 9790, 7789, 5956, 4521, 3400, 2513, 1926, 1483, 1100, 816, 590, 431, 306, 214, 149, 105, 60, 0}
};

static unsigned short cf_se11[ 7][ 7][27] = {
  {
    { 16375, 16372, 16367, 16356, 16326, 16249, 16009, 15318, 13710, 10910, 7311, 3989, 1850, 840, 380, 187, 103, 66, 46, 36, 26, 20, 15, 12, 8, 6, 0},
    { 16383, 16382, 16381, 16380, 16379, 16378, 16377, 16374, 16363, 16323, 16171, 15649, 14281, 11398, 7299, 3581, 1336, 428, 135, 49, 17, 7, 4, 3, 2, 1, 0},
    { 16383, 16382, 16381, 16380, 16379, 16378, 16375, 16367, 16347, 16267, 15969, 15044, 12765, 9094, 5087, 2234, 787, 251, 89, 29, 13, 5, 4, 3, 2, 1, 0},
    { 16383, 16382, 16379, 16376, 16359, 16313, 16124, 15490, 13752, 10641, 6693, 3409, 1499, 567, 208, 76, 34, 17, 10, 7, 6, 5, 4, 3, 2, 1, 0},
    { 16383, 16382, 16381, 16380, 16375, 16367, 16336, 16220, 15772, 14485, 12105, 8736, 5367, 2833, 1387, 581, 239, 98, 46, 24, 12, 9, 7, 6, 5, 2, 0},
    { 16383, 16382, 16380, 16379, 16377, 16375, 16347, 16269, 16004, 15265, 13542, 10823, 7903, 5214, 3145, 1692, 847, 365, 139, 47, 14, 9, 8, 5, 4, 3, 0},
    { 16381, 16378, 16375, 16372, 16336, 16274, 16039, 15643, 14737, 13185, 11186, 8836, 6501, 4198, 2444, 1270, 615, 281, 153, 93, 63, 48, 42, 33, 24, 21, 0}
  },
  {
    { 16383, 16382, 16381, 16380, 16379, 16377, 16376, 16373, 16369, 16357, 16316, 16205, 15866, 14910, 12674, 8962, 4857, 1970, 632, 204, 75, 34, 15, 9, 5, 3, 0},
    { 16383, 16382, 16381, 16380, 16379, 16378, 16377, 16376, 16375, 16374, 16370, 16356, 16298, 16139, 15598, 14050, 10910, 6488, 2627, 701, 138, 38, 12, 6, 2, 1, 0},
    { 16383, 16382, 16381, 16380, 16379, 16378, 16377, 16375, 16358, 16292, 15999, 15070, 12735, 8772, 4549, 1595, 376, 95, 26, 10, 6, 5, 4, 3, 2, 1, 0},
    { 16383, 16382, 16381, 16380, 16379, 16378, 16377, 16376, 16375, 16373, 16361, 16309, 16153, 15563, 13983, 10829, 6716, 3004, 1002, 267, 74, 19, 5, 4, 2, 1, 0},
    { 16383, 16382, 16381, 16380, 16379, 16378, 16377, 16353, 16250, 15897, 14810, 12582, 9100, 5369, 2494, 884, 281, 87, 31, 12, 6, 5, 4, 3, 2, 1, 0},
    { 16383, 16382, 16379, 16378, 16377, 16371, 16348, 16282, 16042, 15416, 13942, 11431, 8296, 5101, 2586, 1035, 328, 68, 15, 9, 6, 5, 4, 3, 2, 1, 0},
    { 16383, 16380, 16379, 16373, 16340, 16267, 16130, 15773, 14969, 13751, 11722, 9172, 6092, 3329, 1507, 563, 186, 86, 26, 23, 10, 7, 6, 5, 4, 1, 0}
  },
  {
    { 16382, 16381, 16380, 16379, 16377, 16370, 16359, 16312, 16141, 15591, 14168, 11084, 6852, 3124, 1105, 354, 124, 48, 25, 14, 7, 6, 5, 4, 3, 1, 0},
    { 16383, 16382, 16381, 16380, 16379, 16378, 16377, 16374, 16357, 16301, 16076, 15343, 13341, 9379, 4693, 1476, 324, 67, 18, 9, 7, 5, 4, 3, 2, 1, 0},
    { 16383, 16382, 16381, 16380, 16379, 16378, 16377, 16369, 16349, 16265, 15937, 14834, 12076, 7587, 3123, 769, 152, 44, 13, 7, 6, 5, 4, 3, 2, 1, 0},
    { 16383, 16382, 16381, 16380, 16379, 16378, 16376, 16367, 16324, 16160, 15574, 13854, 10306, 5601, 1880, 436, 113, 34, 18, 9, 6, 5, 4, 3, 2, 1, 0},
    { 16383, 16382, 16381, 16380, 16379, 16378, 16372, 16348, 16267, 15929, 14858, 12426, 8315, 4098, 1412, 384, 112, 40, 16, 11, 6, 5, 4, 3, 2, 1, 0},
    { 16383, 16382, 16381, 16380, 16379, 16376, 16367, 16310, 16123, 15532, 13965, 11248, 7655, 3910, 1573, 491, 141, 43, 18, 9, 8, 5, 4, 3, 2, 1, 0},
    { 16383, 16381, 16379, 16378, 16377, 16373, 16371, 16367, 16347, 16280, 16132, 15778, 14963, 13688, 11380, 8072, 4680, 2140, 774, 193, 63, 33, 15, 7, 5, 4, 0}
  },
  {
    { 16382, 16381, 16380, 16379, 16378, 16377, 16373, 16360, 16339, 16250, 15927, 14873, 12393, 8549, 4645, 2000, 748, 271, 109, 48, 19, 9, 5, 4, 3, 1, 0},
    { 16383, 16382, 16381, 16380, 16379, 16378, 16377, 16371, 16351, 16244, 15876, 14627, 11604, 6836, 2711, 772, 210, 54, 21, 8, 6, 5, 4, 3, 2, 1, 0},
    { 16383, 16382, 16381, 16380, 16379, 16378, 16376, 16372, 16341, 16209, 15686, 13965, 10150, 5099, 1594, 333, 74, 27, 12, 8, 6, 5, 4, 3, 2, 1, 0},
    { 16383, 16382, 16381, 16380, 16379, 16378, 16376, 16369, 16321, 16091, 15261, 12834, 8160, 3248, 821, 187, 59, 22, 11, 7, 6, 5, 4, 3, 2, 1, 0},
    { 16383, 16382, 16381, 16380, 16379, 16378, 16372, 16350, 16249, 15838, 14425, 11097, 6138, 2238, 628, 180, 53, 21, 13, 7, 6, 5, 4, 3, 2, 1, 0},
    { 16383, 16382, 16381, 16380, 16379, 16377, 16365, 16308, 16026, 15269, 13352, 9583, 5246, 2223, 754, 202, 57, 26, 9, 8, 7, 6, 4, 3, 2, 1, 0},
    { 16379, 16378, 16377, 16376, 16375, 16370, 16365, 16338, 16270, 16120, 15723, 14760, 12783, 9474, 5727, 2713, 977, 296, 93, 39, 14, 12, 10, 7, 4, 3, 0}
  },
  {
    { 16383, 16382, 16379, 16378, 16377, 16370, 16364, 16342, 16267, 16032, 15272, 13475, 10375, 6652, 3685, 1813, 805, 358, 152, 61, 33, 20, 9, 7, 5, 3, 0},
    { 16383, 16382, 16381, 16380, 16379, 16378, 16376, 16361, 16311, 16096, 15280, 13085, 9315, 5003, 1992, 647, 170, 60, 25, 17, 7, 5, 4, 3, 2, 1, 0},
    { 16383, 16382, 16381, 16380, 16379, 16378, 16377, 16376, 16375, 16372, 16355, 16288, 15990, 14926, 12076, 7449, 3161, 981, 302, 78, 24, 7, 4, 3, 2, 1, 0},
    { 16383, 16382, 16381, 16380, 16379, 16377, 16373, 16351, 16264, 15836, 14299, 10534, 5358, 1777, 499, 145, 44, 17, 11, 8, 6, 5, 4, 3, 2, 1, 0},
    { 16383, 16382, 16381, 16380, 16379, 16377, 16366, 16324, 16155, 15416, 13055, 8332, 3423, 1080, 304, 97, 39, 16, 9, 7, 6, 5, 4, 3, 2, 1, 0},
    { 16383, 16382, 16381, 16380, 16377, 16373, 16359, 16258, 15905, 14720, 11631, 6834, 2911, 1022, 345, 116, 49, 24, 14, 7, 6, 5, 4, 3, 2, 1, 0},
    { 16383, 16380, 16379, 16378, 16377, 16376, 16375, 16370, 16365, 16338, 16236, 15960, 15302, 13685, 10788, 6853, 3314, 1213, 417, 149, 59, 25, 8, 3, 2, 1, 0}
  },
  {
    { 16378, 16377, 16376, 16374, 16373, 16368, 16349, 16303, 16149, 15653, 14445, 12326, 9581, 6707, 4156, 2251, 1062, 460, 202, 93, 53, 25, 12, 8, 3, 1, 0},
    { 16383, 16382, 16381, 16380, 16379, 16378, 16374, 16365, 16317, 16146, 15685, 14441, 11949, 8459, 4949, 2280, 874, 300, 86, 29, 20, 10, 7, 3, 2, 1, 0},
    { 16383, 16382, 16381, 16380, 16379, 16377, 16358, 16306, 16114, 15474, 13793, 10641, 6491, 3116, 1219, 382, 135, 62, 26, 17, 11, 6, 5, 3, 2, 1, 0},
    { 16383, 16382, 16381, 16380, 16379, 16376, 16361, 16305, 16051, 15112, 12593, 8234, 4130, 1583, 552, 182, 59, 25, 10, 9, 6, 5, 4, 3, 2, 1, 0},
    { 16383, 16382, 16381, 16380, 16379, 16376, 16346, 16245, 15837, 14409, 10881, 5964, 2333, 798, 279, 100, 41, 14, 9, 7, 6, 5, 4, 3, 2, 1, 0},
    { 16383, 16382, 16380, 16379, 16377, 16361, 16331, 16156, 15454, 13155, 8820, 4256, 1671, 610, 218, 84, 42, 14, 10, 9, 8, 6, 5, 4, 3, 2, 0},
    { 16382, 16380, 16378, 16377, 16367, 16352, 16241, 16077, 15536, 14352, 11787, 7926, 4119, 1726, 638, 233, 91, 28, 16, 9, 8, 6, 5, 4, 3, 1, 0}
  },
  {
    { 16369, 16361, 16352, 16340, 16315, 16284, 16223, 16091, 15848, 15385, 14573, 13396, 11681, 9316, 6613, 4037, 2144, 1033, 491, 213, 100, 55, 34, 18, 12, 6, 0},
    { 16382, 16381, 16379, 16376, 16371, 16359, 16345, 16306, 16198, 16002, 15534, 14580, 12881, 10271, 6793, 3572, 1467, 504, 152, 60, 23, 14, 5, 4, 2, 1, 0},
    { 16383, 16382, 16380, 16379, 16378, 16376, 16367, 16360, 16344, 16292, 16183, 15902, 15224, 13793, 11340, 7866, 4409, 1916, 689, 225, 80, 34, 16, 6, 3, 1, 0},
    { 16381, 16380, 16379, 16377, 16376, 16372, 16366, 16353, 16325, 16266, 16097, 15632, 14551, 12346, 9014, 5262, 2439, 920, 324, 126, 50, 20, 9, 6, 4, 1, 0},
    { 16383, 16380, 16379, 16377, 16375, 16373, 16369, 16360, 16338, 16283, 16183, 15892, 15109, 13313, 10173, 6308, 3103, 1264, 457, 169, 75, 30, 15, 5, 2, 1, 0},
    { 16379, 16377, 16372, 16370, 16365, 16347, 16296, 16186, 15988, 15448, 14083, 11465, 7678, 4215, 1961, 900, 431, 193, 87, 37, 21, 13, 8, 5, 2, 1, 0},
    { 16373, 16368, 16360, 16342, 16320, 16294, 16230, 16123, 15884, 15548, 14801, 13380, 11064, 7909, 4654, 2378, 1114, 490, 235, 135, 74, 40, 21, 11, 6, 1, 0}
  }
};

static short cf_off_se00 = -55;

static short cf_off_se01 =  +2;

static short cf_off_se10 =  -4;

static short cf_off_se02[ 7] = {
  +1,  +1,  +1,  +0,  +0,  +1,  +2
};

static short cf_off_se20[ 7] = {
  +0,  -2,  -2,  -2,  -3,  -4,  -3
};

static short cf_off_se11[ 7][ 7] = {
  {  -5,  +0,  +0,  -3,  -1,  +0,  -1},
  {  +1,  +3,  +0,  +3,  +0,  +0,  -1},
  {  -2,  +0,  +0,  +0,  +0,  +0,  +3},
  {  +0,  +0,  +0,  +0,  +0,  +0,  +2},
  {  +0,  +0,  +3,  +0,  +0,  +0,  +4},
  {  +0,  +1,  +0,  +0,  +0,  +0,  +1},
  {  +0,  +1,  +3,  +3,  +4,  +2,  +4}
};

void arith_encode_bits(
              unsigned char *buf,
              long  *bp,
              Tastat *stat,
              int x,
              int nBits
) {
  int i;
  static unsigned short cf_for_bit[2] = {8192, 0};

  for (i = nBits - 1; i >= 0; --i) {
    int bit = (x >> i) & 1;
    (*bp) = ari_encode(buf, *bp, stat, bit, cf_for_bit);
  }
}

#define MIN_ENC_SEPARATE -12
#define MAX_ENC_SEPARATE +12
#define SYMBOLS_IN_TABLE (1 + (MAX_ENC_SEPARATE - MIN_ENC_SEPARATE + 1) + 1)

void arith_encode_residual(
              unsigned char *buf,
              long  *bp,
              Tastat *stat,
              int x,
              unsigned short* cumulativeFrequencyTable,
              int tableOffset
) {
  x += tableOffset;
  if ((x >= MIN_ENC_SEPARATE) && (x <= MAX_ENC_SEPARATE)) {
    (*bp) = ari_encode(buf, *bp, stat, (x - MIN_ENC_SEPARATE) + 1, cumulativeFrequencyTable);
  } else if (x < MIN_ENC_SEPARATE) {
    int extra = (MIN_ENC_SEPARATE - 1) - x;
    (*bp) = ari_encode(buf, *bp, stat, 0, cumulativeFrequencyTable);
    if (extra < 15) {
      arith_encode_bits(buf, bp, stat, extra, 4);
    } else {
      arith_encode_bits(buf, bp, stat, 15, 4);
      extra -= 15;
      arith_encode_bits(buf, bp, stat, extra, 7);
    }
  } else {
    int extra = x - (MAX_ENC_SEPARATE + 1);
    (*bp) = ari_encode(buf, *bp, stat, SYMBOLS_IN_TABLE - 1, cumulativeFrequencyTable);
    if (extra < 15) {
      arith_encode_bits(buf, bp, stat, extra, 4);
    } else {
      arith_encode_bits(buf, bp, stat, 15, 4);
      extra -= 15;
      arith_encode_bits(buf, bp, stat, extra, 7);
    }
  }
}


static int quant_ctx(int ctx)
{
  if (abs(ctx) <= 3) {
    return ctx;
  } else if (ctx > 3) {
    return 3;
  } else {
    return -3;
  }
}


static int encode_sfe_vector(
              unsigned char                      *buf,
              long                               *bp,
              int                                 t,
              int                                 prev_prev_x0,
              int                                *prev_x,
              int                                *x,
              int                                 length
              )
{
  int f;
  static  int count =0;
  Tastat tstat={0,0,0},*stat;
  stat = &tstat;

  ari_start_encoding(stat);
  for (f = 0; f < length; f ++) {
    if (t == 0) {
      if (f == 0) {
        arith_encode_bits(buf, bp, stat, x[f], 7);
      }
      else if (f == 1) {
        int pred = x[f - 1];

        arith_encode_residual(buf, bp, stat, x[f] - pred, cf_se01, cf_off_se01);
      } else {
        int pred = x[f - 1];
        int ctx = quant_ctx(x[f - 1] - x[f - 2]);

        arith_encode_residual(buf, bp, stat, x[f] - pred, cf_se02[3 + ctx], cf_off_se02[3 + ctx]);
      }
    }
    else if (f == 0) {
      if (t == 1) {
        int pred = prev_x[f];

        arith_encode_residual(buf, bp, stat, x[f] - pred, cf_se10, cf_off_se10);
      } else {
        int pred = prev_x[f];
        int ctx = quant_ctx(prev_x[f] - prev_prev_x0);

        arith_encode_residual(buf, bp, stat, x[f] - pred, cf_se20[3 + ctx], cf_off_se20[3 + ctx]);
      }
    } else {
      int pred = prev_x[f] + x[f - 1] - prev_x[f - 1];
      int ctx_f = quant_ctx(prev_x[f] - prev_x[f - 1]);
      int ctx_t = quant_ctx(x[f - 1] - prev_x[f - 1]);

      arith_encode_residual(buf, bp, stat, x[f] - pred, cf_se11[3 + ctx_t][3 + ctx_f], cf_off_se11[3 + ctx_t][3 + ctx_f]);
    }
  }
  (*bp) = ari_done_encoding(buf,*bp,stat);

  return 0;
}


static int get_IGF_tile(
                 const int                                  igfMin,
#ifdef RM1_3D_BUGFIX_IGF_08
                 const int                                  sfbBgn,
                 const int                                  sfbEnd,
                 const int                                  bUH,
#else
                 const int                                  igfBgn,
                 const int                                  igfEnd,
#endif

                 const int                                 *swb_offset,
                 int                                       *tile
                 ) {
#ifdef RM1_3D_BUGFIX_IGF_08
  int igfBgn   = swb_offset[sfbBgn];
  int igfEnd   = swb_offset[sfbEnd];
  int mem      = sfbBgn;
#endif
  int igfRange = max( 8, igfEnd - igfBgn );
  int i,j;
  int sbs = igfBgn;
  int nTr = 0;
  for ( i = 0 ; i < 4; i++ ) {
    tile[ i ] = igfRange >> 2;
  }

  if(igfBgn > igfMin && igfBgn < igfEnd) {
    for ( i = 0 ; i < 4; i++ ) {
      j = 0;
      do {
        j++;
      } while( min( sbs + tile[ i ], igfEnd ) > swb_offset[ j ]);

#ifdef RM1_3D_BUGFIX_IGF_08
      if (!bUH) {
        if ((((j - sfbBgn) & 1) == 1) && ((j - mem) > 1) && (i < 3)) {
          j--;
        } else if (i == 3) {
          j = sfbEnd;
        }
        mem = j;
      }
#endif

      tile[ i ] = max( 2, min( swb_offset[ j ] - sbs, igfEnd - sbs));
      sbs += tile[ i ];
      nTr++;
      if(sbs == igfEnd) {
        break;
      }
    }
  }
  for ( i = nTr ; i < 4; i++ ) {
    tile[ i ] = 0;
  }
  return nTr;
}

#define INT(a) (int)(a)

#ifdef RM1_3D_BUGFIX_IGF_05
int IGF_getNTiles_enc(
          int               samplingRate,
          int               blockSize,
          int               igfSfbStart,
          int               igfSfbStop,
          int               igFUseHighRes,
          int               isShortWin,
          const int        *swb_offset
) {
  int tile[4];
  int igfBgn = swb_offset[igfSfbStart];
  int igfEnd = swb_offset[igfSfbStop];
  int bl = blockSize >> ((isShortWin)?3:0);
  int igfMin = INT( (1125* bl) / (samplingRate>>1) );

#ifdef RM1_3D_BUGFIX_IGF_04
  igfMin += igfMin%2;
#endif

#ifndef RM1_3D_BUGFIX_IGF_03
  if(igFUseHighRes) {
    if(isShortWin) igfEnd = 128; else igfEnd = 1024;
  }
#endif

#ifdef RM1_3D_BUGFIX_IGF_08
  return get_IGF_tile(igfMin, igfSfbStart, igfSfbStop, igFUseHighRes, swb_offset, tile);
#else
  return get_IGF_tile(igfMin, igfBgn, igfEnd, swb_offset, tile);
#endif
}
#endif

int IGF_level_enc(
          HANDLE_AACBITMUX  bitmux,
          int               indepFlag,
          int               igFAllZero,
          int              *igFLevel,
          int              *igFPrevLevel,
          int              *igFT,
          int              *igFD,
          int              *igFPrevWindow,
          int               isShortWindow,
          int               igFStartSfbLB,
          int               igFStopSfbLB
) {
  long  bp = 0;
  int write_flag = (bitmux!=NULL);
  HANDLE_BSBITSTREAM bs_IGF_LEVEL = aacBitMux_getBitstream(bitmux, IGF_LEVEL);
  static int saveLevel[NSFB_LONG] = {0};
  static int savePrevLevel[NSFB_LONG] = {0};
  static int saveD,saveT,saveW;
  int i;
  unsigned char buf[1024] = {0};

  if (write_flag && bs_IGF_LEVEL==NULL) {
    CommonWarning("usacBitMux: error writing igFLevel()");
    write_flag=0;
  }

#ifndef RM1_3D_BUGFIX_IGF_07
  if(isShortWindow) {
    igFAllZero = 1;
  }
  if (write_flag) {
    BsPutBit( bs_IGF_LEVEL, igFAllZero , LEN_ALL_ZERO );
  }
  bp += 1;
  if(!igFAllZero) {
#endif

    if(!write_flag) {
      saveD = (*igFD);
      saveT = (*igFT);
      saveW = (*igFPrevWindow);
      for(i=0;i<NSFB_LONG;i++) {
        saveLevel[i] = igFPrevLevel[i];
        savePrevLevel[i] = igFLevel[i];
      }
    }
#ifdef RM1_3D_BUGFIX_IGF_01
    if( igFAllZero || indepFlag || ((*igFPrevWindow)!=isShortWindow)) {
#else
    if(/*igFAllZero || */ indepFlag || ((*igFPrevWindow)!=isShortWindow)) {
#endif
      for(i=0;i<NSFB_LONG;i++) {
        igFPrevLevel[i] = 0;
        igFLevel[i] = 0;
      }
      (*igFD) = 0;
      (*igFT) = 0;
    }

    if(!igFAllZero) {
      encode_sfe_vector(buf, &bp, (*igFT), (*igFD), igFPrevLevel+igFStartSfbLB, igFLevel+igFStartSfbLB, (igFStopSfbLB - igFStartSfbLB));
      (*igFT)++;
      (*igFD) = igFPrevLevel[igFStartSfbLB];
      for(i=0;i<NSFB_LONG;i++) {
        igFPrevLevel[i] = igFLevel[i];
      }
      if(write_flag) {
#ifdef RM1_3D_BUGFIX_IGF_07
        for ( i = 0; i < bp ; i++) {
#else
        for ( i = 1; i < bp ; i++) {
#endif
          int bit = ((buf[i>>3]>>(7-(i&7))) & 1);
          BsPutBit( bs_IGF_LEVEL, bit , 1);
        }
      }
    }
#ifndef RM1_3D_BUGFIX_IGF_01

    (*igFPrevWindow) = isShortWindow;

    if(!write_flag) {
      (*igFD) = saveD;
      (*igFT) = saveT;
      (*igFPrevWindow) = saveW;
      for(i=0;i<NSFB_LONG;i++) {
        igFPrevLevel[i] = saveLevel[i];
        igFLevel[i] = savePrevLevel[i];
      }
    }
#endif
#ifndef RM1_3D_BUGFIX_IGF_07
  }
#endif
#ifdef RM1_3D_BUGFIX_IGF_01
  (*igFPrevWindow) = isShortWindow;

  if(!write_flag) {
    (*igFD) = saveD;
    (*igFT) = saveT;
    (*igFPrevWindow) = saveW;
    for(i=0;i<NSFB_LONG;i++) {
      igFPrevLevel[i] = saveLevel[i];
      igFLevel[i] = savePrevLevel[i];
    }
  }
#endif
  return bp;
}

int IGF_data_enc(
          HANDLE_AACBITMUX  bitmux,
          int              *igFTileIdx
#ifdef RM1_3D_BUGFIX_IGF_05
         ,int              nT
#endif
#ifdef RM1_3D_BUGFIX_IGF_02
         ,int               bUsacIndependencyFlag
#endif

) {
  int bit_count = 0;
  int write_flag = (bitmux!=NULL);
  int p;
  HANDLE_BSBITSTREAM bs_IGF_DATA = aacBitMux_getBitstream(bitmux, IGF_DATA);

  if (write_flag && bs_IGF_DATA==NULL) {
    CommonWarning("usacBitMux: error writing igFData()");
    write_flag=0;
  }
#ifdef RM1_3D_BUGFIX_IGF_02
  if(!bUsacIndependencyFlag) {
    if (write_flag) BsPutBit( bs_IGF_DATA, 0 , 1 );
    bit_count += 1;
  }
#else
  if (write_flag) BsPutBit( bs_IGF_DATA, 0 , 1 );
  bit_count += 1;
#endif

#ifdef RM1_3D_BUGFIX_IGF_05
  for(p=0;p<nT;p++) {
#else
  for(p=0;p<4;p++) {
#endif
    if (write_flag)  BsPutBit( bs_IGF_DATA, igFTileIdx[p] , 2 );
    bit_count += 2;
  }
#ifdef RM1_3D_BUGFIX_IGF_06
  for(   ;p<4;p++) {
    igFTileIdx[p] = 3;
  }
#endif
  return (bit_count);
}

#ifndef NINT
  #define NINT(a) ((int)((a)+0.5))
#endif
void IGF_encode(
          double           *pSpectrum,
          int              *swb_offset,
          int               isShortWindow,
          int               igFUseHighRes,
          int               igFStartSfbLB,
          int               igFStopSfbLB,
          int              *igFTileIdx,
          int              *igFAllZero,
          int              *igFLevel
) {
int sfb,width,sb,t;
double tmp;

  *igFAllZero = 1;
  if(!isShortWindow && igFUseHighRes) {
    for( sfb = igFStartSfbLB; sfb < igFStopSfbLB; sfb++) {
      width = swb_offset[sfb +1] - swb_offset[sfb];
      tmp = 0;
      for( sb = 0; sb < width; sb++ ) {
        tmp += pSpectrum[swb_offset[sfb]+sb]*pSpectrum[swb_offset[sfb]+sb];
      }
      tmp = sqrt(tmp / width);
      igFLevel[sfb] = NINT(4.0 * log(tmp) / log(2.0));
      igFLevel[sfb] = min(igFLevel[sfb],127);
      igFLevel[sfb] = max(igFLevel[sfb],  0);
      if(igFLevel[sfb]) *igFAllZero = 0;
    }
    for(t=0;t<4;t++) {
      igFTileIdx[t] = 3;
    }
    for( sfb = igFStartSfbLB; sfb < igFStopSfbLB; sfb++) {
      width = swb_offset[sfb +1] - swb_offset[sfb];
      for( sb = 0; sb < width; sb++ ) {
        tmp = sqrt(pSpectrum[sfb+sb]*pSpectrum[sfb+sb]);
        if(tmp > 1) tmp = 10*log10(tmp);
        if(tmp < 50) {
          pSpectrum[sfb+sb] = 0;
        }
      }
    }
  }
}
