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
#include <float.h>
#ifdef _DEBUG
#include <assert.h>
#endif
#include "usac_mainStruct.h"
#include "usac_allVariables.h"
#include "buffers.h"
#include "huffdec3.h"
#include "dec_IGF.h"

#define INT(a) (int)(a)

/*#define IGF_COMPUTE_REQUIREMENTS*/

#define IGF_SPECTRUM_LEN  1024
#define SFE_GROUP_SIZE    2
#define SFE_QUANT_EXTRA   2
#define CTX_OFFSET        3
#define MIN_ENC_SEPARATE -12
#define MAX_ENC_SEPARATE +12
#define SYMBOLS_IN_TABLE (1 + (MAX_ENC_SEPARATE - MIN_ENC_SEPARATE + 1) + 1)

static float const tnfAcfWindow[8] = {
  0.997803f, 0.991211f, 0.980225f, 0.964844f, 0.945068f, 0.920898f, 0.892334f, 0.859375f
};

static const unsigned short cf_se01[27] = {
  16370, 16360, 16350, 16336, 16326, 16283, 16215, 16065, 15799, 15417, 14875, 13795, 12038, 9704, 6736, 3918, 2054, 1066, 563, 311, 180, 98, 64, 20, 15, 5, 0
};

static const unsigned short cf_se10[27] = {
  16218, 16145, 16013, 15754, 15426, 14663, 13563, 11627, 8894, 6220, 4333, 3223, 2680, 2347, 2058, 1887, 1638, 1472, 1306, 1154, 1012, 895, 758, 655, 562, 489, 0
};

static const unsigned short cf_se02[7][27] = {
  { 16332, 16306, 16278, 16242, 16180, 16086, 15936, 15689, 15289, 14657, 13632, 12095, 9926, 6975, 4213, 2285, 1163, 637, 349, 196, 125, 82, 52, 28, 11, 2, 0},
  { 16370, 16367, 16364, 16358, 16350, 16330, 16284, 16170, 16030, 15647, 14840, 13094, 10364, 6833, 3742, 1639, 643, 282, 159, 85, 42, 22, 16, 15, 4, 1, 0},
  { 16373, 16371, 16367, 16363, 16354, 16336, 16290, 16204, 16047, 15735, 14940, 13159, 10171, 6377, 3044, 1212, 474, 208, 115, 60, 27, 14, 7, 6, 5, 1, 0},
  { 16382, 16377, 16367, 16357, 16334, 16281, 16213, 16035, 15613, 14694, 12898, 9720, 5747, 2506, 1030, 469, 251, 124, 58, 48, 35, 17, 12, 7, 6, 5, 0},
  { 16383, 16375, 16374, 16366, 16336, 16250, 16107, 15852, 15398, 14251, 12117, 8796, 5016, 2288, 998, 431, 236, 132, 89, 37, 16, 12, 4, 3, 2, 1, 0},
  { 16375, 16357, 16312, 16294, 16276, 16222, 16133, 15999, 15515, 14655, 13123, 10667, 7324, 4098, 2073, 1141, 630, 370, 209, 93, 48, 39, 12, 11, 10, 9, 0},
  { 16343, 16312, 16281, 16179, 16067, 15730, 15464, 15025, 14392, 13258, 11889, 10224, 7824, 5761, 3902, 2349, 1419, 837, 520, 285, 183, 122, 71, 61, 40, 20, 0}
};

static const unsigned short cf_se20[7][27] = {
  { 16351, 16344, 16317, 16283, 16186, 16061, 15855, 15477, 14832, 13832, 12286, 10056, 7412, 4889, 2996, 1739, 1071, 716, 496, 383, 296, 212, 149, 109, 82, 59, 0},
  { 16368, 16352, 16325, 16291, 16224, 16081, 15788, 15228, 14074, 12059, 9253, 5952, 3161, 1655, 1006, 668, 479, 357, 254, 199, 154, 115, 88, 67, 51, 45, 0},
  { 16372, 16357, 16339, 16314, 16263, 16169, 15984, 15556, 14590, 12635, 9475, 5625, 2812, 1488, 913, 641, 467, 347, 250, 191, 155, 117, 89, 72, 59, 46, 0},
  { 16371, 16362, 16352, 16326, 16290, 16229, 16067, 15675, 14715, 12655, 9007, 5114, 2636, 1436, 914, 650, 477, 357, 287, 227, 182, 132, 105, 79, 58, 48, 0},
  { 16364, 16348, 16318, 16269, 16192, 16033, 15637, 14489, 12105, 8407, 4951, 2736, 1669, 1156, 827, 615, 465, 348, 269, 199, 162, 125, 99, 73, 51, 37, 0},
  { 16326, 16297, 16257, 16136, 15923, 15450, 14248, 11907, 8443, 5432, 3396, 2226, 1561, 1201, 909, 699, 520, 423, 323, 255, 221, 163, 121, 87, 71, 50, 0},
  { 16317, 16280, 16203, 16047, 15838, 15450, 14749, 13539, 11868, 9790, 7789, 5956, 4521, 3400, 2513, 1926, 1483, 1100, 816, 590, 431, 306, 214, 149, 105, 60, 0}
};

static const unsigned short cf_se11[7][7][27] = {
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

static const short cf_off_se02[7] = {
  +1,  +1,  +1,  +0,  +0,  +1,  +2
};

static const short cf_off_se20[7] = {
  +0,  -2,  -2,  -2,  -3,  -4,  -3
};

static const short cf_off_se11[7][7] = {
  {  -5,  +0,  +0,  -3,  -1,  +0,  -1},
  {  +1,  +3,  +0,  +3,  +0,  +0,  -1},
  {  -2,  +0,  +0,  +0,  +0,  +0,  +3},
  {  +0,  +0,  +0,  +0,  +0,  +0,  +2},
  {  +0,  +0,  +3,  +0,  +0,  +0,  +4},
  {  +0,  +1,  +0,  +0,  +0,  +0,  +1},
  {  +0,  +1,  +3,  +3,  +4,  +2,  +4}
};

static const unsigned short cf_for_bit[2] = {
  8192, 0
};

static const short cf_off_se01 = +2;
static const short cf_off_se10 = -4;

static int _IGF_quant_ctx(
                    const int                 ctx
) {
  if (abs(ctx) <= 3) {
    return ctx;
  } else if (ctx > 3) {
    return 3;
  } else {
    return -3;
  }
}

static int _IGF_arith_decode_bits(
                    unsigned char            *bitBuf,
                    int                      *bitOffset,
                    Tastat                   *stat,
                    const int                 nBits
) {
  int i;
  int x = 0;
  int bit;
  for (i = nBits - 1; i >= 0; --i) {
    *bitOffset = (int)ari_decode(bitBuf,
                                 *bitOffset,
                                 &bit,
                                 stat,
                                 cf_for_bit,
                                 2);
    x = x + (bit << i);
  }
  return x;
}

static int _IGF_arith_decode_residual(
                    unsigned char            *bitBuf,
                    int                      *bitOffset,
                    Tastat                   *stat,
                    const unsigned short     *cumulativeFrequencyTable,
                    const int                 tableOffset
) {
  int val;
  int x;

  *bitOffset = ari_decode(bitBuf,
                          *bitOffset,
                          &val,
                          stat,
                          cumulativeFrequencyTable,
                          SYMBOLS_IN_TABLE);

  if ((val != 0) && (val != SYMBOLS_IN_TABLE - 1)) {
    int x = (val - 1) + MIN_ENC_SEPARATE;
    x -= tableOffset;
    return x;
  } else if (val == 0) {
    int extra = _IGF_arith_decode_bits(bitBuf,
                                       bitOffset,
                                       stat,
                                       4);
    if (extra == 15) {
      extra = _IGF_arith_decode_bits(bitBuf,
                                     bitOffset,
                                     stat,
                                     7);
      extra = 15 + extra;
    }
    x = (MIN_ENC_SEPARATE - 1) - extra;
  } else {
    int extra = _IGF_arith_decode_bits(bitBuf,
                                       bitOffset,
                                       stat,
                                       4);
    if (extra == 15) {
      extra = _IGF_arith_decode_bits(bitBuf,
                                     bitOffset,
                                     stat,
                                     7);
      extra = 15 + extra;
    }
    x = (MAX_ENC_SEPARATE + 1) + extra;
  }
  x -= tableOffset;
  return x;
}

static void _IGF_arith_decode(
                    unsigned char            *bitBuf,
                    int                      *bitOffset,
                    Tastat                   *stat,
                    const IGF_WIN_TYPE        igfWinType,
                    const int                 m_igfSfbStart,
                    const int                 m_igfSfbStop,
                    const int                *prev_x,
                    int                      *x,
                    const int                 prev_prev_x0,
                    const int                 t,
                    const int                 igf_UseHighRes
) {
  int f;
  int sfe_group_enabled = (!igf_UseHighRes && (IGF_LONG == igfWinType));
  int increment = 1;
  int length = m_igfSfbStop - m_igfSfbStart;

  if (sfe_group_enabled) {
    increment = SFE_GROUP_SIZE;
  }

  for (f = 0; f < length; f += increment) {
    int z;
    if (t == 0) {
      if (f == 0) {
        x[f] = _IGF_arith_decode_bits(bitBuf,
                                      bitOffset,
                                      stat,
                                      7);
      }
      else if (f == increment) {
        int pred = x[f - increment];
        x[f] = pred + _IGF_arith_decode_residual(bitBuf,
                                                 bitOffset,
                                                 stat,
                                                 cf_se01,
                                                 cf_off_se01);
      } else {
        int pred = x[f - increment];
        int ctx = _IGF_quant_ctx(x[f - increment] - x[f - 2 * increment]);
        x[f] = pred + _IGF_arith_decode_residual(bitBuf,
                                                 bitOffset,
                                                 stat,
                                                 cf_se02[CTX_OFFSET + ctx],
                                                 cf_off_se02[CTX_OFFSET + ctx]);
      }
    }
    else if (f == 0) {
      if (t == 1) {
        int pred = prev_x[f];
        x[f] = pred + _IGF_arith_decode_residual(bitBuf,
                                                 bitOffset,
                                                 stat,
                                                 cf_se10,
                                                 cf_off_se10);
      } else {
        int pred = prev_x[f];
        int ctx = _IGF_quant_ctx(prev_x[f] - prev_prev_x0);
        x[f] = pred + _IGF_arith_decode_residual(bitBuf,
                                                 bitOffset,
                                                 stat,
                                                 cf_se20[CTX_OFFSET + ctx],
                                                 cf_off_se20[CTX_OFFSET + ctx]);
      }
    } else {
      int pred = prev_x[f] + x[f - increment] - prev_x[f - increment];
      int ctx_f = _IGF_quant_ctx(prev_x[f] - prev_x[f - increment]);
      int ctx_t = _IGF_quant_ctx(x[f - increment] - prev_x[f - increment]);
      x[f] = pred + _IGF_arith_decode_residual(bitBuf,
                                               bitOffset,
                                               stat,
                                               cf_se11[CTX_OFFSET + ctx_t][CTX_OFFSET + ctx_f],
                                               cf_off_se11[CTX_OFFSET + ctx_t][CTX_OFFSET + ctx_f]);
    }
    for (z = 1; z < increment; ++z) {
      if (f + z < length) {
        x[f + z] = x[f];
      }
    }
  }
}

static void _TNF_convertLPC(
                    const float              *input,
                    const int                 order,
                    float                    *predCoef,
                    float                    *predGain
) {
  int i;
  int j;
  float tmp;
  float tmp2;
  float memory[32]       = {0.0f};
  float parCoeff[16]     = {0.0f};
  float* const pMemory   = &memory[order];
  const float threshold  = 1.0f / 65536.0f;

  for (i = 0; i < order; i++) {
    memory[i]  = input[i];
    pMemory[i] = input[i+1];
  }

  for (i = 0; i < order; i++) {
    tmp = 0;
    if (memory[0] >= threshold) {
      tmp = -pMemory[i] / memory[0];
    }

    tmp = MIN(0.999f, MAX(-0.999f, tmp));
    parCoeff[i] = tmp;

    for (j = i; j < order; j++) {
      tmp2           = pMemory[j] + tmp * memory[j - i];
      memory[j - i] += tmp * pMemory[j];
      pMemory[j]     = tmp2;
    }
  }

  predCoef[0] = 1.0f;
  predCoef[1] = parCoeff[0];

  for (i = 1; i < order; i++) {
    for (j = 0; j < (i >> 1); j++) {
      tmp                      = predCoef[j + 1];
      predCoef[j + 1]         += parCoeff[i] * predCoef[i - 1 - j + 1];
      predCoef[i - 1 - j + 1] += parCoeff[i] * tmp;
    }
    if (i & 1) {
      predCoef[j + 1] += parCoeff[i] * predCoef[j + 1];
    }
    predCoef[i + 1] = parCoeff[i];
  }

  *predGain = ((input[0] + 1e-30f) / (memory[0] + 1e-30f));
}

static void _TNF_filter(
                    float                    *spectrum,
                    const int                 tnfLength,
                    const float              *predCoef,
                    const int                 order
) {
  int i;
  int j;
  float buf[16 + 2048] = {0.0f};
  float *pbuf          = buf + 16;

  if (order > 0) {
    for (i = 0; i < tnfLength; i++) {
      pbuf[i] = spectrum[i];
    }

    for (j = 0; j < tnfLength; j++, pbuf++) {
      spectrum[j] = pbuf[0];

      for (i = 1; i < order; i++){
        spectrum[j] += predCoef[i] * pbuf[-i];
      }
    }
  }
}

static float _TNF_norm(
                    const float              *X,
                    const int                 n
) {
  int i;
  float acc = 0.0f;

  for (i = 0; i < n; i++) {
    acc += X[i] * X[i];
  }
  return acc;
}

static float _TNF_autocorrelation(
                    const float              *x,
                    const int                 n,
                    const int                 lag
) {
  int i;
  float acc = 0.0f;

  if (n - lag) {
    acc = x[0] * x[lag];
  }
  for (i = 1; i < n - lag; i++) {
    acc += x[i] * x[i + lag];
  }
  return acc;
}

static void _TNF_detect(
                    const float              *pSpectrum,
                    const int                 igfBgn,
                    const int                 igfEnd,
                    float                    *predCoef,
                    float                    *predGain,
                    int                      *order
) {
  int i;
  int lag;
  int startLine;
  int stopLine;
  int tnfRange;
  float fac;
  float norms[3]        = {0.0f};
  float rxx[16 + 1]     = {0.0f};
  const int nSubDiv     = 3;
  const int igfRange    = igfEnd - igfBgn;
  const float threshold = 0.0000000037252902984619140625f;

  for (i = 0; i < nSubDiv; i++) {
    startLine = igfBgn + (igfEnd - igfBgn) *  i      / nSubDiv;
    stopLine  = igfBgn + (igfEnd - igfBgn) * (i + 1) / nSubDiv;
    tnfRange  = stopLine - startLine;

    norms[i] = _TNF_norm(pSpectrum + startLine,
                         tnfRange);
  }

  for (i = 0; (i < nSubDiv) && (norms[i] > threshold); i++) {
    fac       = 1.0f / norms[i];
    startLine = igfBgn + igfRange *  i      / nSubDiv;
    stopLine  = igfBgn + igfRange * (i + 1) / nSubDiv;
    tnfRange  = stopLine - startLine;

    for (lag = 1; lag <= 8; lag++) {
      rxx[lag] += fac * tnfAcfWindow[lag - 1] * _TNF_autocorrelation(pSpectrum + startLine,
                                                                     tnfRange,
                                                                     lag);
    }
  }

  if (i == nSubDiv) {
    *order = 8;
    rxx[0] = (float)nSubDiv;
    _TNF_convertLPC(rxx,
                    MIN(8, igfRange >> 2),
                    predCoef,
                    predGain);
  }
}

static void _TNF_apply(
                    HIGF_DEC_DATA             igfDecData,
                    const int                 igfBgn,
                    const int                 igfEnd
) {
  int order = 0;
  float predGain = 0;
  float predCoef[16+1] = {0.0f};

  _TNF_detect(igfDecData->igf_tnfSpec,
              igfBgn,
              igfEnd,
              predCoef,
              &predGain,
              &order);

  _TNF_filter(&igfDecData->igf_tnfSpec[igfBgn],
              igfEnd - igfBgn,
              predCoef,
              order);
}

static int _IGF_get_igfMin(
                    const IGF_WIN_TYPE        igfWinType,
                    const int                 ccfl,
                    const int                 sampleRate
) {
  int igfMin;
  int bl;

  switch (igfWinType) {
    case IGF_SHORT:
      bl = ccfl >> 3;
      break;
    case IGF_TCX_MEDIUM:
      bl = ccfl >> 1;
      break;
    default:
      bl = ccfl;
  }

  igfMin  = INT((1125 * bl) / (sampleRate >> 1));
  igfMin += igfMin % 2;

  return igfMin;
}

static void _IGF_decode_whitening(
                    HANDLE_BUFFER             hVm,
                    HANDLE_RESILIENCE         hResilience,
                    HANDLE_ESC_INSTANCE_DATA  hEscInstanceData,
                    HIGF_BITSTREAM            igfBitStream,
                    const int                 p
) {
  int tmp;

  tmp = (int)GetBits(1,
                     IGF_WHITENING,
                     hResilience,
                     hEscInstanceData,
                     hVm);
  if (tmp == 1) {
    tmp = (int)GetBits(1,
                       IGF_WHITENING,
                       hResilience,
                       hEscInstanceData,
                       hVm);
    if (tmp == 1) {
      igfBitStream->igf_WhiteningLevel[p] = IGF_FLAT_STRONG;
    } else {
      igfBitStream->igf_WhiteningLevel[p] = IGF_FLAT_OFF;
    }
  } else {
    igfBitStream->igf_WhiteningLevel[p] = IGF_FLAT_MEDIUM;
  }
}

static void _IGF_reset_level(
                    HIGF_ENVELOPE             igfEnvelope
) {
  int sfb;

  for (sfb = 0; sfb < NSFB_LONG; sfb++) {
    igfEnvelope->igf_sN[sfb] = 0.0f;
    igfEnvelope->igf_pN[sfb] = 0.0f;
  }
}

static void _IGF_reset_data(
                    HIGF_DEC_DATA             igfDecData,
                    const IGF_FRAME_ID        igfFrmID
) {
  int p;
  const int sFidx = (IGF_ID_TCX_MEDIUM_SECOND == igfFrmID) ? 1 : 0;

  for (p = 0; p < 4; p++) {
    igfDecData->igf_memory.igf_prevTileNum[p]              = 3;
    igfDecData->igf_memory.igf_prevWhiteningLevel[p]       = IGF_FLAT_MEDIUM;
    igfDecData->igf_bitstream[sFidx].igf_TileNum[p]        = 3;
    igfDecData->igf_bitstream[sFidx].igf_WhiteningLevel[p] = IGF_FLAT_MEDIUM;
  }

  igfDecData->igf_bitstream[sFidx].igf_isTnfFlat = IGF_TNF_OFF;
}

static void _IGF_reset(
                    HIGF_DEC_DATA             igfDecData
) {
  if (NULL != igfDecData) {
    _IGF_reset_level(&igfDecData->igf_envelope);
    _IGF_reset_data(igfDecData,
                    IGF_ID_TCX_MEDIUM_FIRST);
    _IGF_reset_data(igfDecData,
                    IGF_ID_TCX_MEDIUM_SECOND);
  }
}

static int _IGF_get_tile_ix(
                    const int                 tb,
                    const int                 igfMin,
                    const int                 nT,
                    const int                *tile
) {
  int p = 0;
  int sbs = igfMin;

  for (p = 0; p < nT; p++) {
    sbs += tile[p];
    if (tb < sbs) {
      break;
    }
  }
  return p;
}

static int _IGF_get_tile(
                    const int                 igfMin,
                    const int                 sfbBgn,
                    const int                 sfbEnd,
                    const int                 bUH,
                    const short              *swb_offset,
#ifdef IGF_COMPUTE_REQUIREMENTS
                    int                      *nST,
#endif
                    int                      *tile
) {
  int i;
  int j;
  int igfBgn   = swb_offset[sfbBgn];
  int igfEnd   = swb_offset[sfbEnd];
  int mem      = sfbBgn;
  int igfRange = MAX(8, igfEnd - igfBgn);
  int sbs      = igfBgn;
  int nTr      = 0;

#ifdef IGF_COMPUTE_REQUIREMENTS
  int src = igfBgn - igfMin;
#endif
  for (i = 0 ; i < 4; i++) {
    tile[i] = igfRange >> 2;

#ifdef IGF_COMPUTE_REQUIREMENTS
    nST[i] = 0;
#endif
  }

  if (igfBgn > igfMin && igfBgn < igfEnd) {
    for (i = 0 ; i < 4; i++) {
      j = 0;
      do {
        j++;
      } while (MIN(sbs + tile[i], igfEnd) > swb_offset[j]);

      if (!bUH) {
        if ((((j - sfbBgn) & 1) == 1) && ((j - mem) > 1) && (i < 3)) {
          j--;
        } else if (i == 3) {
          j = sfbEnd;
        }
        mem = j;
      }

      tile[i] = MAX(2, MIN(swb_offset[j] - sbs, igfEnd - sbs));

#ifdef IGF_COMPUTE_REQUIREMENTS
      nST[i]  = MAX(1, MIN((src / (tile[i] >> 1)) - 1, 4));
#endif

      sbs += tile[i];
      nTr++;
      if (sbs == igfEnd) {
        break;
      }
    }
  }
  for (i = nTr ; i < 4; i++) {
    tile[i] = 0;
  }
  return nTr;
}

static int _IGF_get_sb(
                    const int                 igfMin,
                    const int                 igfBgn,
                    const int                 tb,
                    const int                 nT,
#ifdef IGF_COMPUTE_REQUIREMENTS
                    const int                *nST,
#endif
                    const int                *tile,
                    const int                *tileIdx
) {
  int i;
  int selTile;
  int sbs    = igfBgn;
  int offset = 0;
  int sb     = tb;
  int src    = igfBgn - igfMin;

  if (src > 0) {
    for (i = 0; i < nT; i++) {

      if ((tb >= sbs) && (tb < (sbs + tile[i]))) {
        selTile = tileIdx[i];

#ifdef IGF_COMPUTE_REQUIREMENTS
        if ((selTile - (4 - nST[i]) < 0) || (selTile < 0) || (selTile > 3)) {
          /*selTile = 3;*/
          assert(selTile + nST[i] >= 4);
        }
#endif
        offset = INT((-0.5f * selTile + 2.5f) * tile[i] + sbs - igfBgn);
        if ((offset & 1) == 1) {
          offset++;
        }
        break;
      }
      sbs += tile[i];
    }
    sb = tb - offset;
    if (sb < igfMin) {
      sb = (igfMin + tb % src);
    }
  }
  return sb;
}

static float _IGF_randomSign(
                    unsigned int             *seed
) {
  float sign = +1.f;

  *seed = ((*seed) * 69069) + 5;
  if (((*seed) & 0x10000) > 0) {
    sign = -1.f;
  }
  return sign;
}

static void _IGF_apply_whitening(
                    const float              *pMDCT,
                    float                    *pMDCT_flat,
                    const int                 igfMin,
                    const int                 stop
) {
  int i;
  int j;
  int n;
  float env;
  float fac;

  for (i = igfMin; i < stop - 3; i++) {
    env = 1e-3f;
    for (j = i - 3; j <= i + 3; j++) {
      env += pMDCT[j] * pMDCT[j];
    }

#ifdef _MSC_VER
    frexpf(env, &n); n--;         /* see NOTE (C99) */
#else
    frexp((double)env, &n); n--;  /* see NOTE (C90) */
#endif
    /* NOTE: In ISO/IEC 23008-3 subclause "5.5.5.4.7 Spectral whitening in IGF"        */
    /*       the estimation kernel is defined as follows:                              */
    /*       n = FLOOR(log(env)/log(2));                                               */
    /*       This pseudo code translates to the following C/C++ code:                  */
    /*       n = (int)floor(log((double)env) / log(2.0));                              */
    /*       ISO/IEC 9899 defines frexp() and frexpf() which are well suited functions */
    /*       allowing for an optimized implementation of the given calculation.        */

    fac = (float)pow(2.0f, 21.0f - 0.5f * n);
    pMDCT_flat[i] = pMDCT[i] * fac;
  }

  for (; i < stop; i++) {
    pMDCT_flat[i] = pMDCT[i] * fac;
  }
}

static void _IGF_store_whitening(
                    float                    *out,
                    const float              *in,
                    const int                 igfMin,
                    const int                 stop
) {
  int bin;

  for (bin = igfMin; bin < stop; bin++) {
    out[bin] = in[bin];
  }
}

static void _IGF_spectral_whitening(
                    const HIGF_CONFIG         igfConfig,
                    const HIGF_BITSTREAM      igfBitStream,
                    float                    *pINFSpectralData,
                    const short              *swb_offset,
                    const IGF_WIN_TYPE        igfWinType,
                    const int                 blockSize,
                    const int                 samplingRate
) {
  int ix;
  const int nT          = igfConfig->igf_grid[igfWinType].igf_NTiles;
  const int igfSfbStart = igfConfig->igf_grid[igfWinType].igf_SfbStart;
  const int igfMin      = igfConfig->igf_grid[igfWinType].igf_Min;
  const int igfBgn      = swb_offset[igfSfbStart];

  if (IGF_SHORT != igfWinType) {
    for (ix = 0; ix < nT; ix++) {
      if (IGF_FLAT_MEDIUM == igfBitStream->igf_WhiteningLevel[ix]) {
        float igf_spec_flat[2048] = {0.0f};

        _IGF_apply_whitening(&pINFSpectralData[ix * 2048],
                             igf_spec_flat,
                             igfMin,
                             igfBgn);

        _IGF_store_whitening(&pINFSpectralData[ix * 2048],
                             igf_spec_flat,
                             igfMin,
                             igfBgn);
      }
    }
  }
}

static void _IGF_norm(
                    const HIGF_CONFIG         igfConfig,
                    HIGF_ENVELOPE             igfEnvelope,
                    const IGF_WIN_TYPE        igfWinType,
                    const int                 group_len
) {
  int sfb;
  const int igfSfbStart = igfConfig->igf_grid[igfWinType].igf_SfbStart;
  const int igfSfbStop  = igfConfig->igf_grid[igfWinType].igf_SfbStop;

  for (sfb = igfSfbStart; sfb < igfSfbStop; sfb++) {
    igfEnvelope->igf_sN[sfb] /= group_len;
    igfEnvelope->igf_pN[sfb] /= group_len;
  }
}

static void _IGF_rescaling(
                    const HIGF_CONFIG         igfConfig,
                    const IGF_WIN_TYPE        igfWinType,
                    float                    *pSpectralData,
                    const short              *swb_offset,
                    const float              *igf_curr
) {
  int sfb;
  int bin;
  int width;
  int sfbStep;
  int sfbLow;
  int sfbHigh;
  const int igfSfbStart = igfConfig->igf_grid[igfWinType].igf_SfbStart;
  const int igfSfbStop  = igfConfig->igf_grid[igfWinType].igf_SfbStop;

  if (igfConfig->igf_UseHighRes || igfWinType >= IGF_SHORT) {
    sfbStep = 1;
  } else {
    sfbStep = 2;
  }

  if (igfWinType > IGF_SHORT) {
    for (sfb = igfSfbStart; sfb < igfSfbStop; sfb += sfbStep) {
      sfbLow  = sfb;
      sfbHigh = MIN(sfb + sfbStep, igfSfbStop);
      width   = (swb_offset[sfbHigh] - swb_offset[sfbLow]);

      for (bin = 0; bin < width; bin++) {
        pSpectralData[swb_offset[sfb] + bin] *= igf_curr[sfb];
      }
    }
  }
}

static void _IGF_calc(
                    const HIGF_CONFIG         igfConfig,
                    HIGF_ENVELOPE             igfEnvelope,
                    const HIGF_BITSTREAM      igfBitStream,
                    const float              *pSpectralData,
                    const float              *pINFSpectralData,
                    const short              *swb_offset,
                    unsigned int             *nfSeed,
                    const IGF_WIN_TYPE        igfWinType,
                    const int                 blockSize,
                    const int                 samplingRate
) {
  int sfb;
  int bin;
  int width;
  int IGF_sb;
  float val;
  float E;
  int sfbStep;
  int sfbLow;
  int sfbHigh;
  int tile[4] = {0};
#ifdef IGF_COMPUTE_REQUIREMENTS
  int nST[4] = {0};
#endif
  int nT;
  const int isShortWin  = (IGF_SHORT == igfWinType) ? 1 : 0;
  const int igfSfbStart = igfConfig->igf_grid[igfWinType].igf_SfbStart;
  const int igfSfbStop  = igfConfig->igf_grid[igfWinType].igf_SfbStop;
  const int igfMin      = igfConfig->igf_grid[igfWinType].igf_Min;
  const int igfBgn      = swb_offset[igfSfbStart];

  if (igfConfig->igf_UseHighRes || igfWinType >= IGF_SHORT) {
    sfbStep = 1;
  } else {
    sfbStep = 2;
  }

  for (sfb = igfSfbStart; sfb < igfSfbStop; sfb += sfbStep) {
    sfbLow  = sfb;
    sfbHigh = MIN(sfb + sfbStep, igfSfbStop);
    width   = (swb_offset[sfbHigh] - swb_offset[sfbLow]);

    E = 0.0f;
    for (bin = 0; bin < width; bin++) {
      IGF_sb = swb_offset[sfb] + bin;
      val    = pSpectralData[IGF_sb];
      E     += val * val;
    }

    igfEnvelope->igf_sN[sfb] += E;

    E = 0.0f;
    nT = _IGF_get_tile(igfMin,
                       igfSfbStart,
                       igfSfbStop,
                       igfConfig->igf_UseHighRes,
                       swb_offset,
#ifdef IGF_COMPUTE_REQUIREMENTS
                       nST,
#endif
                       tile);

    for (bin = 0; bin < width; bin++) {
      if (pSpectralData[swb_offset[sfb] + bin] == 0.0f) {
        int ix = _IGF_get_tile_ix(swb_offset[sfb] + bin, swb_offset[igfSfbStart], nT, tile);
        IGF_sb = _IGF_get_sb(igfMin,
                             igfBgn,
                             swb_offset[sfb] + bin,
                             nT,
#ifdef IGF_COMPUTE_REQUIREMENTS
                             nST,
#endif
                             tile,
                             igfBitStream->igf_TileNum);
        val = pINFSpectralData[ix * 2048 + IGF_sb];

        if (!isShortWin && igfConfig->igf_UseWhitening) {
          if (IGF_FLAT_STRONG == igfBitStream->igf_WhiteningLevel[ix]) {
            val  = _IGF_randomSign(nfSeed);
            val *= 2097152.f;
          }
        }
        E += val * val;
      }
    }
    igfEnvelope->igf_pN[sfb] += E;
  }
}

static void _IGF_calc_s(
                    const HIGF_CONFIG         igfConfig,
                    HIGF_ENVELOPE             igfEnvelopeL,
                    HIGF_ENVELOPE             igfEnvelopeR,
                    const HIGF_BITSTREAM      igfBitStreamL,
                    const HIGF_BITSTREAM      igfBitStreamR,
                    const float              *pSpectralDataL,
                    const float              *pSpectralDataR,
                    const float              *pINFSpectralDataL,
                    const float              *pINFSpectralDataR,
                    const int                *mask,
                    const short              *swb_offset,
                    unsigned int             *nfSeedL,
                    unsigned int             *nfSeedR,
                    const IGF_WIN_TYPE        igfWinType,
                    const int                 blockSize,
                    const int                 samplingRate
) {
  int sfb;
  int bin;
  int width;
  int IGF_sb;
  int IGF_sb_l;
  int IGF_sb_r;
  float valL;
  float valR;
  float EL;
  float ER;
  int sfbStep;
  int sfbLow;
  int sfbHigh;
  int tileL[4] = {0};
  int tileR[4] = {0};
#ifdef IGF_COMPUTE_REQUIREMENTS
  int nSTL[4] = {0};
  int nSTR[4] = {0};
#endif
  int nTL;
  int nTR;
  const int isShortWin  = (IGF_SHORT == igfWinType) ? 1 : 0;
  const int igfSfbStart = igfConfig->igf_grid[igfWinType].igf_SfbStart;
  const int igfSfbStop  = igfConfig->igf_grid[igfWinType].igf_SfbStop;
  const int igfMin      = igfConfig->igf_grid[igfWinType].igf_Min;
  const int igfBgn      = swb_offset[igfSfbStart];

  if (igfConfig->igf_UseHighRes || igfWinType >= IGF_SHORT) {
    sfbStep = 1;
  } else {
    sfbStep = 2;
  }

  for (sfb = igfSfbStart; sfb < igfSfbStop; sfb += sfbStep) {
    sfbLow  = sfb;
    sfbHigh = MIN(sfb + sfbStep, igfSfbStop);
    width   = swb_offset[sfbHigh] - swb_offset[sfbLow];
    EL = ER = 0.0f;

    for (bin = 0; bin < width; bin++) {
      IGF_sb = swb_offset[sfb] + bin;
      valL   = pSpectralDataL[IGF_sb];
      EL    += valL * valL;
      valR   = pSpectralDataR[IGF_sb];
      ER    += valR * valR;
    }
    igfEnvelopeL->igf_sN[sfb] += EL;
    igfEnvelopeR->igf_sN[sfb] += ER;

    EL  = ER = 0.0f;
    nTL = _IGF_get_tile(igfMin,
                        igfSfbStart,
                        igfSfbStop,
                        igfConfig->igf_UseHighRes,
                        swb_offset,
#ifdef IGF_COMPUTE_REQUIREMENTS
                        nSTL,
#endif
                        tileL);
    nTR = _IGF_get_tile(igfMin,
                        igfSfbStart,
                        igfSfbStop,
                        igfConfig->igf_UseHighRes,
                        swb_offset,
#ifdef IGF_COMPUTE_REQUIREMENTS
                        nSTR,
#endif
                        tileR);

    for (bin = 0; bin < width; bin++) {
      int ix_l = _IGF_get_tile_ix(swb_offset[sfb] + bin, swb_offset[igfSfbStart], nTL, tileL);
      int ix_r = _IGF_get_tile_ix(swb_offset[sfb] + bin, swb_offset[igfSfbStart], nTR, tileR);
      IGF_sb_l = _IGF_get_sb(igfMin,
                             igfBgn,
                             swb_offset[sfb] + bin,
                             nTL,
#ifdef IGF_COMPUTE_REQUIREMENTS
                             nSTL,
#endif
                             tileL,
                             igfBitStreamL->igf_TileNum);
      IGF_sb_r = _IGF_get_sb(igfMin, 
                             igfBgn,
                             swb_offset[sfb] + bin,
                             nTR,
#ifdef IGF_COMPUTE_REQUIREMENTS
                             nSTR,
#endif
                             tileR,
                             igfBitStreamR->igf_TileNum);

      valL = pINFSpectralDataL[ix_l * 2048 + IGF_sb_l];
      valR = pINFSpectralDataR[ix_r * 2048 + IGF_sb_r];

      if (!isShortWin && igfConfig->igf_UseWhitening) {
        if (IGF_FLAT_STRONG == igfBitStreamL->igf_WhiteningLevel[ix_l]) {
          valL = _IGF_randomSign(nfSeedL);
          valL *= 2097152.f;
        }
        if (IGF_FLAT_STRONG == igfBitStreamR->igf_WhiteningLevel[ix_r]) {
          valR = _IGF_randomSign(nfSeedR);
          valR *= 2097152.f;
        }
      }

      if (mask[sfb]) {
        float tmp = valL;
        valL = 0.5f * (tmp + valR);
        valR = 0.5f * (tmp - valR);
      }
      if (pSpectralDataL[swb_offset[sfb] + bin] == 0.0f) {
        EL += valL * valL;
      }
      if (pSpectralDataR[swb_offset[sfb] + bin] == 0.0f) {
        ER += valR * valR;
      }
    }
    igfEnvelopeL->igf_pN[sfb] += EL;
    igfEnvelopeR->igf_pN[sfb] += ER;
  }
}

static void _IGF_apply_m(
                    HIGF_DEC_DATA             igfDecData,
                    const HIGF_CONFIG         igfConfig,
                    const HIGF_ENVELOPE       igfEnvelope,
                    const HIGF_BITSTREAM      igfBitStream,
                    float                    *pSpectralData,
                    const float              *pINFSpectralData,
                    const short              *swb_offset,
                    unsigned int             *nfSeed,
                    const IGF_WIN_TYPE        igfWinType,
                    const int                 blockSize,
                    const int                 samplingRate,
                    const int                 g
) {
  int sfb;
  int bin;
  float dE;
  float mN;
  float sN;
  float pN;
  int width;
  int IGF_sb;
  float val;
  float gn;
  int sfbStep;
  int sfbLow;
  int sfbHigh;
  int tile[4] = {0};
#ifdef IGF_COMPUTE_REQUIREMENTS
  int nST[4] = {0};
#endif
  int nT;
  const int isShortWin  = (IGF_SHORT == igfWinType) ? 1 : 0;
  const int igfSfbStart = igfConfig->igf_grid[igfWinType].igf_SfbStart;
  const int igfSfbStop  = igfConfig->igf_grid[igfWinType].igf_SfbStop;
  const int igfMin      = igfConfig->igf_grid[igfWinType].igf_Min;
  const int igfBgn      = swb_offset[igfSfbStart];;

  if (igfConfig->igf_UseTNF && IGF_SHORT != igfWinType) {
    for (bin = 0; bin < 2048; bin++) {
      igfDecData->igf_tnfSpec[bin] = 0.0f;
      igfDecData->igf_tnfMask[bin] = IGF_TNF_EXCLUDE;
    }
  }

  _IGF_rescaling(igfConfig,
                 igfWinType,
                 pSpectralData,
                 swb_offset,
                 igfBitStream->igf_curr[g]);

  if (igfConfig->igf_UseHighRes || igfWinType >= IGF_SHORT) {
    sfbStep = 1;
  } else {
    sfbStep = 2;
  }

  for (sfb = igfSfbStart; sfb < igfSfbStop; sfb += sfbStep) {
    sfbLow  = sfb;
    sfbHigh = MIN(sfb + sfbStep, igfSfbStop);
    width   = (swb_offset[sfbHigh] - swb_offset[sfbLow]);

    dE = igfBitStream->igf_curr[g][sfb];
    sN = igfEnvelope->igf_sN[sfb];
    pN = igfEnvelope->igf_pN[sfb];
    mN = (dE * dE) * width - sN;

    if (mN > 0.0f && pN > 0.0f) {
      gn = MIN(10.f, (float)sqrt(mN / pN));
    } else {
      gn = 0.0f;
    }

    nT = _IGF_get_tile(igfMin,
                       igfSfbStart,
                       igfSfbStop,
                       igfConfig->igf_UseHighRes,
                       swb_offset,
#ifdef IGF_COMPUTE_REQUIREMENTS
                       nST,
#endif
                       tile);

    for (bin = 0; bin < width; bin++) {
      int ix = _IGF_get_tile_ix(swb_offset[sfb] + bin, swb_offset[igfSfbStart], nT, tile);
      IGF_sb = _IGF_get_sb(igfMin,
                           igfBgn,
                           swb_offset[sfb] + bin,
                           nT,
#ifdef IGF_COMPUTE_REQUIREMENTS
                           nST,
#endif
                           tile,
                           igfBitStream->igf_TileNum);
      val    = pINFSpectralData[ix * 2048 + IGF_sb];

      if (pSpectralData[swb_offset[sfb] + bin] == 0.0f) {
        if (!isShortWin && igfConfig->igf_UseWhitening) {
          if (IGF_FLAT_STRONG == igfBitStream->igf_WhiteningLevel[ix]) {
            val = _IGF_randomSign(nfSeed);
            val *= 2097152.f;
          }
        }
      }

      if (igfConfig->igf_UseTNF && IGF_SHORT != igfWinType) {
        igfDecData->igf_tnfSpec[swb_offset[sfb] + bin] = gn * val;

        if (pSpectralData[swb_offset[sfb] + bin] == 0.0f) {
          igfDecData->igf_tnfMask[swb_offset[sfb] + bin] = IGF_TNF_INCLUDE;
        }
      }

      if (pSpectralData[swb_offset[sfb] + bin] == 0.0f) {
        pSpectralData[swb_offset[sfb] + bin] = gn * val;
      }
    }
  }
}

static void _IGF_apply_s(
                    HIGF_DEC_DATA             igfDecDataL,
                    HIGF_DEC_DATA             igfDecDataR,
                    HIGF_CONFIG               igfConfig,
                    HIGF_ENVELOPE             igfEnvelopeL,
                    HIGF_ENVELOPE             igfEnvelopeR,
                    HIGF_BITSTREAM            igfBitStreamL,
                    HIGF_BITSTREAM            igfBitStreamR,
                    float                    *pSpectralDataL,
                    float                    *pSpectralDataR,
                    const float              *pINFSpectralDataL,
                    const float              *pINFSpectralDataR,
                    const short              *swb_offset,
                    unsigned int             *nfSeedL,
                    unsigned int             *nfSeedR,
                    const int                *mask,
                    const IGF_WIN_TYPE        igfWinType,
                    const int                 blockSize,
                    const int                 samplingRate,
                    const int                 g
) {
  int sfb;
  int bin;
  float dEL;
  float dER;
  float mNL;
  float mNR;
  float sNL;
  float sNR;
  float pNL;
  float pNR;
  int width;
  int IGF_sb_l;
  int IGF_sb_r;
  float gnL;
  float gnR;
  float valL;
  float valR;
  int sfbStep;
  int sfbLow;
  int sfbHigh;
  int tileL[4] = {0};
  int tileR[4] = {0};
#ifdef IGF_COMPUTE_REQUIREMENTS
  int nSTL[4] = {0};
  int nSTR[4] = {0};
#endif
  int nTTL;
  int nTTR;
  const int isShortWin  = (IGF_SHORT == igfWinType) ? 1 : 0;
  const int igfSfbStart = igfConfig->igf_grid[igfWinType].igf_SfbStart;
  const int igfSfbStop  = igfConfig->igf_grid[igfWinType].igf_SfbStop;
  const int igfMin      = igfConfig->igf_grid[igfWinType].igf_Min;
  const int igfBgn      = swb_offset[igfSfbStart];

  if (igfConfig->igf_UseTNF && IGF_SHORT != igfWinType) {
    for (bin = 0; bin < 2048; bin++) {
      igfDecDataL->igf_tnfSpec[bin] = 0.0f;
      igfDecDataR->igf_tnfSpec[bin] = 0.0f;
      igfDecDataL->igf_tnfMask[bin] = IGF_TNF_EXCLUDE;
      igfDecDataR->igf_tnfMask[bin] = IGF_TNF_EXCLUDE;
    }
  }

  _IGF_rescaling(igfConfig,
                 igfWinType,
                 pSpectralDataL,
                 swb_offset,
                 igfBitStreamL->igf_curr[g]);
  _IGF_rescaling(igfConfig,
                 igfWinType,
                 pSpectralDataR,
                 swb_offset,
                 igfBitStreamR->igf_curr[g]);

  if (igfConfig->igf_UseHighRes || igfWinType >= IGF_SHORT) {
    sfbStep = 1;
  } else {
    sfbStep = 2;
  }

  for (sfb = igfSfbStart; sfb < igfSfbStop; sfb += sfbStep) {
    sfbLow  = sfb;
    sfbHigh = MIN(sfb + sfbStep, igfSfbStop);
    width   = (swb_offset[sfbHigh] - swb_offset[sfbLow]);

    dEL = igfBitStreamL->igf_curr[g][sfb];
    sNL = igfEnvelopeL->igf_sN[sfb];
    pNL = igfEnvelopeL->igf_pN[sfb];
    mNL = (dEL * dEL) * width - sNL;
    dER = igfBitStreamR->igf_curr[g][sfb];
    sNR = igfEnvelopeR->igf_sN[sfb];
    pNR = igfEnvelopeR->igf_pN[sfb];
    mNR = (dER * dER) * width - sNR;

    if (mNL > 0.0f && pNL > 0.0f) {
      gnL = MIN(10.f, (float)sqrt(mNL / pNL));
    } else {
      gnL = 0.0f;
    }

    if (mNR > 0.0f && pNR > 0.0f) {
      gnR = MIN(10.f, (float)sqrt(mNR / pNR));
    } else {
      gnR = 0.0f;
    }

    nTTL = _IGF_get_tile(igfMin,
                         igfSfbStart,
                         igfSfbStop,
                         igfConfig->igf_UseHighRes,
                         swb_offset,
#ifdef IGF_COMPUTE_REQUIREMENTS
                         nSTL,
#endif
                         tileL);
    nTTR = _IGF_get_tile(igfMin,
                         igfSfbStart,
                         igfSfbStop,
                         igfConfig->igf_UseHighRes,
                         swb_offset,
#ifdef IGF_COMPUTE_REQUIREMENTS
                         nSTR,
#endif
                         tileR);

    for (bin = 0; bin < width; bin++) {
      int ix_l = _IGF_get_tile_ix(swb_offset[sfb] + bin, swb_offset[igfSfbStart], nTTL, tileL);
      int ix_r = _IGF_get_tile_ix(swb_offset[sfb] + bin, swb_offset[igfSfbStart], nTTR, tileR);
      IGF_sb_l = _IGF_get_sb(igfMin,
                             igfBgn,
                             swb_offset[sfb] + bin,
                             nTTL,
#ifdef IGF_COMPUTE_REQUIREMENTS
                             nSTL,
#endif
                             tileL,
                             igfBitStreamL->igf_TileNum);
      IGF_sb_r = _IGF_get_sb(igfMin,
                             igfBgn,
                             swb_offset[sfb] + bin,
                             nTTR,
#ifdef IGF_COMPUTE_REQUIREMENTS
                             nSTR,
#endif
                             tileR,
                             igfBitStreamR->igf_TileNum);
      valL     =  pINFSpectralDataL[ix_l * 2048 + IGF_sb_l];
      valR     =  pINFSpectralDataR[ix_r * 2048 + IGF_sb_r];

      if (!isShortWin && igfConfig->igf_UseWhitening) {
        if (IGF_FLAT_STRONG == igfBitStreamL->igf_WhiteningLevel[ix_l]) {
          valL  = _IGF_randomSign(nfSeedL);
          valL *= 2097152.f;
        }
        if (IGF_FLAT_STRONG == igfBitStreamR->igf_WhiteningLevel[ix_r]) {
          valR  = _IGF_randomSign(nfSeedR);
          valR *= 2097152.f;
        }
      }

      if (mask[sfb]) {
        float tmp = valL;
        valL = 0.5f * (tmp + valR);
        valR = 0.5f * (tmp - valR);
      }

      if (igfConfig->igf_UseTNF && IGF_SHORT != igfWinType) {
        igfDecDataL->igf_tnfSpec[swb_offset[sfb] + bin] = gnL * valL;
        igfDecDataR->igf_tnfSpec[swb_offset[sfb] + bin] = gnR * valR;

        if (pSpectralDataL[swb_offset[sfb] + bin] == 0.0f) {
          igfDecDataL->igf_tnfMask[swb_offset[sfb] + bin] = IGF_TNF_INCLUDE;
        }
        if (pSpectralDataR[swb_offset[sfb] + bin] == 0.0f) {
          igfDecDataR->igf_tnfMask[swb_offset[sfb] + bin] = IGF_TNF_INCLUDE;
        }
        if (mask[sfb]) {
          igfDecDataL->igf_tnfMask[swb_offset[sfb] + bin] = (IGF_TNF_MASK)(igfDecDataL->igf_tnfMask[swb_offset[sfb] + bin] || igfDecDataR->igf_tnfMask[swb_offset[sfb] + bin]);
          igfDecDataR->igf_tnfMask[swb_offset[sfb] + bin] = igfDecDataL->igf_tnfMask[swb_offset[sfb] + bin];
        }
      }

      if (pSpectralDataL[swb_offset[sfb] + bin] == 0.0f) {
        pSpectralDataL[swb_offset[sfb] + bin] = gnL * valL;
      }
      if (pSpectralDataR[swb_offset[sfb] + bin] == 0.0f) {
        pSpectralDataR[swb_offset[sfb] + bin] = gnR * valR;
      }
    }
  }
}

static void _IGF_sync_Data(
                    HIGF_BITSTREAM            igfBitStreamL,
                    HIGF_BITSTREAM            igfBitStreamR
) {
  int t;
  int igf_allZeroL;
  int igf_allZeroR;

  igf_allZeroL = igfBitStreamL->igf_allZero;
  igf_allZeroR = igfBitStreamR->igf_allZero;

  if (1 == igf_allZeroL && 0 == igf_allZeroR) {
    for (t = 0; t < 4; t++) {
      igfBitStreamL->igf_TileNum[t]        = igfBitStreamR->igf_TileNum[t];
      igfBitStreamL->igf_WhiteningLevel[t] = igfBitStreamR->igf_WhiteningLevel[t];
    }
    igfBitStreamL->igf_isTnfFlat = igfBitStreamR->igf_isTnfFlat;
  } else if (0 == igf_allZeroL && 1 == igf_allZeroR) {
    for (t = 0; t < 4; t++) {
      igfBitStreamR->igf_TileNum[t]        = igfBitStreamL->igf_TileNum[t];
      igfBitStreamR->igf_WhiteningLevel[t] = igfBitStreamL->igf_WhiteningLevel[t];
    }
    igfBitStreamR->igf_isTnfFlat = igfBitStreamL->igf_isTnfFlat;
  }
}

static int _IGF_get_NTiles(
                    const HIGF_GRID_CONFIG    igf_grid,
                    const int                 igfUseHighRes,
                    const short              *swb_offset
) {
  int tile[4] = {0};
#ifdef IGF_COMPUTE_REQUIREMENTS
  int nST[4] = {0};
#endif

  int nT = _IGF_get_tile(igf_grid->igf_Min,
                         igf_grid->igf_SfbStart,
                         igf_grid->igf_SfbStop,
                         igfUseHighRes,
                         swb_offset,
#ifdef IGF_COMPUTE_REQUIREMENTS
                         nST,
#endif
                         tile);
  return nT;
}

static void _IGF_get_swb_offset(
                    const WINDOW_SEQUENCE     win,
                    const byte               *group,
                    const IGF_WIN_TYPE        igfWinType,
                    const int                 winOffset,
                    short                    *swb_offset
) {
  int sfb;
  int swb_shift    = 0;
  Info *IGFsfbInfo = usac_winmap[win];

  switch (igfWinType) {
    case IGF_LONG:
    case IGF_SHORT:
    case IGF_CODEC_TRANSITION:
      swb_shift = 0;
      break;
    case IGF_TCX_MEDIUM:
    case IGF_TCX_LONG:
      swb_shift = (int)igfWinType;
      break;
  }

  for (sfb = 0; sfb < IGFsfbInfo->sfb_per_sbk[0]; sfb++) {
    swb_offset[sfb + 1] = (*(IGFsfbInfo->sbk_sfb_top[winOffset] + sfb)) << swb_shift;
#ifdef _DEBUG
    assert((sfb + 1) < 64);
#endif
  }
}

int IGF_get_igfBgn(
                    const HIGF_CONFIG         igfConfig,
                    const IGF_WIN_TYPE        igfWinType,
                    const WINDOW_SEQUENCE     win,
                    const byte               *group,
                    const int                 winOffset
) {
  short swb_offset[64]  = {0};
  int swb_shift         = 0;
  int igfBgn            = 0;
  const int igfSfbStart = igfConfig->igf_grid[igfWinType].igf_SfbStart;
  Info *IGFsfbInfo      = usac_winmap[win];

  _IGF_get_swb_offset(win,
                      group,
                      igfWinType,
                      winOffset,
                      swb_offset);

  igfBgn = swb_offset[igfSfbStart];

  return igfBgn;
}

int IGF_get_mask(
                    HANDLE_BUFFER             hVm,
                    HANDLE_RESILIENCE         hResilience,
                    HANDLE_ESC_INSTANCE_DATA  hEscInstanceData,
                    const HIGF_CONFIG         igfConfig,
                    const IGF_WIN_TYPE        igfWinType,
                    const Info               *info,
                    const byte               *group,
                    byte                     *mask
) {
  int b;
  int i;
  int mp;
  int sfbPerMsBand;
  const int igfSfbStart = igfConfig->igf_grid[igfWinType].igf_SfbStart;
  const int igfSfbStop  = igfConfig->igf_grid[igfWinType].igf_SfbStop;

  if (igfConfig->igf_UseHighRes || igfWinType > IGF_SHORT) {
    sfbPerMsBand = 1;
  } else {
    sfbPerMsBand = 2;
  }

  mp = GetBits(LEN_MASK_PRES,
               MS_MASK_PRESENT,
               hResilience,
               hEscInstanceData,
               hVm);

  if (mp == 0) {
    return 0;
  }

  if (mp == 2) {
    for (b = 0; b < info->nsbk; b = *group++) {
      mask += igfSfbStart;
      for (i = igfSfbStart; i < igfSfbStop; i++) {
        *mask++ = 1;
      }
      for (     ; i < info->sfb_per_sbk[b]; i++) {
        *mask++ = 0;
      }
    }
    return 2;
  }

  if (mp == 3) {
    return 3;
  }

  for (b = 0; b < info->nsbk; b = *group++) {
    mask += igfSfbStart;
    for (i = igfSfbStart; i < igfSfbStop; i += sfbPerMsBand) {
      *mask = (byte) GetBits(LEN_MASK,
                             MS_USED,
                             hResilience,
                             hEscInstanceData,
                             hVm);
      mask++;
      if (i + 1 < igfSfbStop && sfbPerMsBand == 2) {
        *mask = *(mask -1);
        mask++;
      }
    }
    for (i = igfSfbStop ; i < info->sfb_per_sbk[b]; i++) {
      *mask = 0;
      mask++;
    }
  }
  return 1;
}

void IGF_get_cplx_pred_data(
                    HANDLE_BUFFER             hVm,
                    HANDLE_RESILIENCE         hResilience,
                    HANDLE_ESC_INSTANCE_DATA  hEscInstanceData,
                    const HIGF_CONFIG         igfConfig,
                    const IGF_WIN_TYPE        igfWinType,
                    int                     **alpha_q_re,
                    int                     **alpha_q_im,
                    int                      *alpha_q_re_prev,
                    int                      *alpha_q_im_prev,
                    int                     **cplx_pred_used,
                    int                      *pred_dir,
                    const int                 num_window_groups,
                    const int                 bUsacIndependenceFlag
) {
  const int sfb_per_pred_band = 2;
  Hcb*      hcb = &book[BOOKSCL];
  Huffman*  hcw = hcb->hcw;
  int cplx_pred_all;
  int delta_code_time;
  int g;
  int sfb;
  int dpcm_alpha;
  int last_alpha_q_re;
  int last_alpha_q_im;
  const int igfSfbStart = igfConfig->igf_grid[igfWinType].igf_SfbStart;
  const int igfSfbStop  = igfConfig->igf_grid[igfWinType].igf_SfbStop;

  cplx_pred_all = GetBits(LEN_CPLX_PRED_ALL,
                          CPLX_PRED_ALL,
                          hResilience,
                          hEscInstanceData,
                          hVm);

  if (cplx_pred_all == 0) {
    for (g = 0; g < num_window_groups; g++) {
      for (sfb = igfSfbStart; sfb < igfSfbStop; sfb += sfb_per_pred_band) {
        cplx_pred_used[g][sfb] = GetBits(LEN_CPLX_PRED_USED,
                                         CPLX_PRED_USED,
                                         hResilience,
                                         hEscInstanceData,
                                         hVm);
        if (sfb + 1 < igfSfbStop) {
          cplx_pred_used[g][sfb + 1] = cplx_pred_used[g][sfb];
        }
      }
      for (sfb = igfSfbStop; sfb < SFB_NUM_MAX; sfb++) {
        cplx_pred_used[g][sfb] = 0;
      }
    }
  } else {
    for (g = 0; g < num_window_groups; g++) {
      for (sfb = igfSfbStart; sfb < igfSfbStop; sfb++) {
        cplx_pred_used[g][sfb] = 1;
      }
      for (sfb = igfSfbStop; sfb < SFB_NUM_MAX; sfb++) {
        cplx_pred_used[g][sfb] = 0;
      }
    }
  }

  *pred_dir = GetBits(LEN_PRED_DIR,
                      PRED_DIR,
                      hResilience,
                      hEscInstanceData,
                      hVm);

  if (bUsacIndependenceFlag){
    delta_code_time = 0;
  } else {
    delta_code_time = GetBits(LEN_DELTA_CODE_TIME,
                              DELTA_CODE_TIME,
                              hResilience,
                              hEscInstanceData,
                              hVm);
  }

  for (g = 0; g < num_window_groups; g++) {
    for (sfb = igfSfbStart; sfb < igfSfbStop; sfb += sfb_per_pred_band) {
      if (delta_code_time == 1) {
        last_alpha_q_re = alpha_q_re_prev[sfb];
        last_alpha_q_im = alpha_q_im_prev[sfb];
      } else {
        if (sfb > igfSfbStart) {
          last_alpha_q_re = alpha_q_re[g][sfb - 1];
          last_alpha_q_im = alpha_q_im[g][sfb - 1];
        } else {
          last_alpha_q_re = last_alpha_q_im = 0;
        }
      }

      if (cplx_pred_used[g][sfb] == 1) {
        dpcm_alpha = -decode_huff_cw(hcw, 0, hResilience, hEscInstanceData, hVm) + 60;
        alpha_q_re[g][sfb] = dpcm_alpha + last_alpha_q_re;
        alpha_q_im[g][sfb] = 0;
      }
      else {
        alpha_q_re[g][sfb] = 0;
        alpha_q_im[g][sfb] = 0;
      }

      if ((sfb + 1) < igfSfbStop) {
        alpha_q_re[g][sfb+1] = alpha_q_re[g][sfb];
        alpha_q_im[g][sfb+1] = alpha_q_im[g][sfb];
      }

      alpha_q_re_prev[sfb] = alpha_q_re[g][sfb];
      alpha_q_im_prev[sfb] = alpha_q_im[g][sfb];
    }
    for (sfb = igfSfbStop; sfb < SFB_NUM_MAX; sfb++) {
      alpha_q_re[g][sfb]   = 0;
      alpha_q_im[g][sfb]   = 0;
      alpha_q_re_prev[sfb] = 0;
      alpha_q_im_prev[sfb] = 0;
    }
  }
}

int IGF_level(
                    HANDLE_BUFFER             hVm,
                    HANDLE_RESILIENCE         hResilience,
                    HANDLE_ESC_INSTANCE_DATA  hEscInstanceData,
                    HIGF_DEC_DATA             igfDecData,
                    const HIGF_CONFIG         igfConfig,
                    const IGF_WIN_TYPE        igfWinType,
                    const IGF_FRAME_ID        igfFrmID,
                    const int                 num_window_groups,
                    const int                 bUsacIndependencyFlag
) {
  int g;
  int sfb;
  int igf_AllZero = 0;
  int factor = 0;
  int bitOffset = 0;
  HANDLE_BSBITBUFFER h_arith_bb;
  unsigned char *bitBuf = NULL;
  Tastat stat = {0, 0, 0};
  float igf_emphasis = 0.0f;
  int igf_curr_q[MAX_SHORT_WINDOWS][NSFB_LONG] = {0};
  const int sFidx = (IGF_ID_TCX_MEDIUM_SECOND == igfFrmID) ? 1 : 0;
  const int igfSfbStart = igfConfig->igf_grid[igfWinType].igf_SfbStart;
  const int igfSfbStop  = igfConfig->igf_grid[igfWinType].igf_SfbStop;

  igf_AllZero = (int) GetBits(1,
                              IGF_ALLZERO,
                              hResilience,
                              hEscInstanceData,
                              hVm);

  if (igfConfig->igf_UseHighRes || igfWinType > IGF_SHORT) {
    factor = 1;
  } else {
    factor = 2;
  }

  /* NOTE: In ISO/IEC 23008-3 subclause "5.5.5.4.3.2 Decoding Process"               */
  /*        the given pseudo:                                                        */
  /*       if (   igf_AllZero                                                        */
  /*           || indepFlag                                                          */
  /*           || ((num_windows[ch] != prev_num_windows[ch]) && core_mode[ch] == 0)  */
  /*           || ((lpd_mode[ch]    != last_lpd_mode[ch])    && core_mode[ch] == 1)  */
  /*           || (core_mode[ch]    != core_mode_prev[ch]))                          */
  /*       relates to the following if-statement:                                    */
  if (igf_AllZero || bUsacIndependencyFlag || (igfWinType != igfDecData->igf_memory.igf_prevWinType)) {
    for (sfb = 0; sfb < NSFB_LONG; sfb++) {
      igfDecData->igf_envelope.igf_prev[sfb] = 0;
      for (g = 0; g < MAX_SHORT_WINDOWS; g++) {
        igfDecData->igf_bitstream[sFidx].igf_curr[g][sfb] = 0;
      }
    }
    igfDecData->igf_envelope.igf_arith_t = 0;
    igfDecData->igf_envelope.igf_PrevD   = 0;
  }

  if (!igf_AllZero) {
    h_arith_bb = GetRemainingBufferBits (hVm);
    bitBuf     = BsBufferGetDataBegin(h_arith_bb);
    bitOffset  = 0;
    bitOffset  = ari_start_decoding(bitBuf, bitOffset, &stat);

    igf_emphasis = 0.0f;
    if (igfWinType > IGF_SHORT) {
      igf_emphasis = 40.0f;
    }

    for (g = 0; g < num_window_groups; g++) {
      _IGF_arith_decode(bitBuf,
                        &bitOffset,
                        &stat,
                        igfWinType,
                        igfSfbStart,
                        igfSfbStop,
                        igfDecData->igf_envelope.igf_prev + igfSfbStart,
                        igf_curr_q[g] + igfSfbStart,
                        igfDecData->igf_envelope.igf_PrevD,
                        igfDecData->igf_envelope.igf_arith_t,
                        igfConfig->igf_UseHighRes);
      igfDecData->igf_envelope.igf_PrevD = igfDecData->igf_envelope.igf_prev[igfSfbStart];

      for (sfb = igfSfbStart; sfb < igfSfbStop; sfb++) {
        igfDecData->igf_envelope.igf_prev[sfb] = igf_curr_q[g][sfb];
        igf_curr_q[g][sfb] *= factor;
        igfDecData->igf_bitstream[sFidx].igf_curr[g][sfb] = (float)pow(2.0, 0.25 * (igf_curr_q[g][sfb] - igf_emphasis));
      }
      igfDecData->igf_envelope.igf_arith_t++;
    }
    SkipBits(hVm, bitOffset - 14);
    BsFreeBuffer(h_arith_bb);
  } else {
    _IGF_reset_data(igfDecData,
                    igfFrmID);
  }

  igfDecData->igf_memory.igf_prevWinType = igfWinType;
  igfDecData->igf_bitstream[sFidx].igf_allZero = igf_AllZero;

  return igf_AllZero;
}

void IGF_data(
                    HANDLE_BUFFER             hVm,
                    HANDLE_RESILIENCE         hResilience,
                    HANDLE_ESC_INSTANCE_DATA  hEscInstanceData,
                    HIGF_DEC_DATA             igfDecData,
                    const HIGF_CONFIG         igfConfig,
                    const IGF_WIN_TYPE        igfWinType,
                    const IGF_FRAME_ID        igfFrmID,
                    const int                 bUsacIndependencyFlag
) {
  int p;
  int igf_UsePrevTile;
  int igf_UsePrevWhiteningLevel;
  const int isShortWin = (IGF_SHORT == igfWinType) ? 1 : 0;
  const int sFidx = (IGF_ID_TCX_MEDIUM_SECOND == igfFrmID) ? 1 : 0;
  const int nT = igfConfig->igf_grid[igfWinType].igf_NTiles;

  if (!bUsacIndependencyFlag) {
    igf_UsePrevTile = (int) GetBits(1,
                                    IGF_USEPREVTILE,
                                    hResilience,
                                    hEscInstanceData,
                                    hVm);
  } else {
    igf_UsePrevTile = 0;
  }
  if (igf_UsePrevTile) {
    for (p = 0; p < nT; p++) {
      igfDecData->igf_bitstream[sFidx].igf_TileNum[p] = igfDecData->igf_memory.igf_prevTileNum[p];
    }
  } else {
    for (p = 0; p < nT; p++) {
      igfDecData->igf_bitstream[sFidx].igf_TileNum[p] = (int) GetBits(2,
                                                                      IGF_TILENUM,
                                                                      hResilience,
                                                                      hEscInstanceData,
                                                                      hVm);
    }
  }

  for (p = 0; p < nT; p++) {
    igfDecData->igf_memory.igf_prevTileNum[p] = igfDecData->igf_bitstream[sFidx].igf_TileNum[p];
    igfDecData->igf_bitstream[sFidx].igf_WhiteningLevel[p] = IGF_FLAT_MEDIUM;
  }
  for (     ; p < 4; p++) {
    igfDecData->igf_memory.igf_prevTileNum[p] = 3;
    igfDecData->igf_memory.igf_prevWhiteningLevel[p] = IGF_FLAT_MEDIUM;
    igfDecData->igf_bitstream[sFidx].igf_TileNum[p] = 3;
    igfDecData->igf_bitstream[sFidx].igf_WhiteningLevel[p] = IGF_FLAT_MEDIUM;
  }

  if (igfConfig->igf_UseWhitening && !isShortWin) {
    if (bUsacIndependencyFlag) {
      igf_UsePrevWhiteningLevel = 0;
    } else {
      igf_UsePrevWhiteningLevel = (int) GetBits(1,
                                                IGF_WHITENING,
                                                hResilience,
                                                hEscInstanceData,
                                                hVm);
    }
    if (igf_UsePrevWhiteningLevel == 1) {
      for (p = 0; p < nT; p++) {
        igfDecData->igf_bitstream[sFidx].igf_WhiteningLevel[p] = igfDecData->igf_memory.igf_prevWhiteningLevel[p];
      }
    } else {
      int remainingTilesDifferent = 0;
      p = 0;
      _IGF_decode_whitening(hVm,
                            hResilience,
                            hEscInstanceData,
                            &igfDecData->igf_bitstream[sFidx],
                            p);
      remainingTilesDifferent = (int) GetBits(1,
                                              IGF_WHITENING,
                                              hResilience,
                                              hEscInstanceData,
                                              hVm);
      if (remainingTilesDifferent == 1) {
        for (p = 1; p < nT; p++) {
          _IGF_decode_whitening(hVm,
                                hResilience,
                                hEscInstanceData,
                                &igfDecData->igf_bitstream[sFidx],
                                p);
        }
      } else {
        for (p = 1; p < nT; p++) {
          igfDecData->igf_bitstream[sFidx].igf_WhiteningLevel[p] = igfDecData->igf_bitstream[sFidx].igf_WhiteningLevel[0];
        }
      }
    }
  }

  for (p = 0; p < 4; p++) {
    igfDecData->igf_memory.igf_prevWhiteningLevel[p] = igfDecData->igf_bitstream[sFidx].igf_WhiteningLevel[p];
  }

  if (igfConfig->igf_UseTNF && !isShortWin) {
    int tmp = (int)GetBits(1,
                           IGF_FLATTRIGER,
                           hResilience,
                           hEscInstanceData,
                           hVm);
    if (tmp == 1) {
      igfDecData->igf_bitstream[sFidx].igf_isTnfFlat = IGF_TNF_ON;
    } else {
      igfDecData->igf_bitstream[sFidx].igf_isTnfFlat = IGF_TNF_OFF;
    }
  }
}

void IGF_init(
                    HIGF_DEC_DATA             igfDecDataL,
                    HIGF_DEC_DATA             igfDecDataR,
                    HIGF_CONFIG               igfConfig,
                    const int                 igf_active,
                    const int                 igf_UseHighRes,
                    const int                 igf_UseENF,
                    const int                 igf_UseWhitening,
                    const int                 igf_AfterTnsSynth,
                    const int                 igf_IndependentTiling,
                    const int                 igf_StartIdx,
                    const int                 igf_StopIdx,
                    const int                 ccfl,
                    const int                 sampleRate
) {
  Info *IGFsfbInfoLB = usac_winmap[ONLY_LONG_SEQUENCE];
  Info *IGFsfbInfoSB = usac_winmap[EIGHT_SHORT_SEQUENCE];
  int sfb;
  int igf_SfbStartLB = 0;
  int igf_SfbStartSB = 0;
  int igf_SfbStopLB  = 0;
  int igf_SfbStopSB  = 0;

  igfConfig->igf_active = igf_active;

  if (igfConfig->igf_active) {
    igfConfig->igf_UseHighRes        = igf_UseHighRes;
    igfConfig->igf_UseINF            = igf_UseENF;
    igfConfig->igf_UseTNF            = igf_UseENF;
    igfConfig->igf_UseWhitening      = igf_UseWhitening;
    igfConfig->igf_AfterTnsSynth     = igf_AfterTnsSynth;
    igfConfig->igf_IndependentTiling = (IGF_CPE_MODE)((igf_IndependentTiling == 1) ? IGF_DUALMONO : IGF_STEREO);

    igf_SfbStartLB = MIN(11 + igf_StartIdx, IGFsfbInfoLB->sfb_per_bk - 5);
    if (igf_StopIdx != 15) {
      igf_SfbStopLB = MIN(IGFsfbInfoLB->sfb_per_bk, MAX(igf_SfbStartLB + (((IGFsfbInfoLB->sfb_per_bk - (igf_SfbStartLB + 1)) * (igf_StopIdx + 2)) >> 4), igf_SfbStartLB + 1));
    } else {
      igf_SfbStopLB = IGFsfbInfoLB->sfb_per_bk;
    }

    igf_SfbStartSB = -1;
    for (sfb = 0; sfb < IGFsfbInfoSB->sfb_per_sbk[0]; sfb++) {
      if (IGFsfbInfoSB->sbk_sfb_top[0][sfb] >= (IGFsfbInfoLB->sbk_sfb_top[0][igf_SfbStartLB - 1] >> 3)) {
        if (igf_SfbStartSB < 0) {
          igf_SfbStartSB = sfb + 1;
        }
      }
    }

    igf_SfbStopSB = -1;
    for (sfb = 0; sfb < IGFsfbInfoSB->sfb_per_sbk[0]; sfb++) {
      if (IGFsfbInfoSB->sbk_sfb_top[0][sfb] >= (IGFsfbInfoLB->sbk_sfb_top[0][igf_SfbStopLB - 1] >> 3)) {
        if (igf_SfbStopSB < 0) {
          igf_SfbStopSB = sfb + 1;
        }
      }
    }

    igfConfig->igf_grid[IGF_LONG].igf_SfbStart       = igf_SfbStartLB;
    igfConfig->igf_grid[IGF_LONG].igf_SfbStop        = igf_SfbStopLB;
    igfConfig->igf_grid[IGF_SHORT].igf_SfbStart      = igf_SfbStartSB;
    igfConfig->igf_grid[IGF_SHORT].igf_SfbStop       = igf_SfbStopSB;
    igfConfig->igf_grid[IGF_TCX_MEDIUM].igf_SfbStart = igfConfig->igf_grid[IGF_SHORT].igf_SfbStart;
    igfConfig->igf_grid[IGF_TCX_MEDIUM].igf_SfbStop  = igfConfig->igf_grid[IGF_SHORT].igf_SfbStop;
    igfConfig->igf_grid[IGF_TCX_LONG].igf_SfbStart   = igfConfig->igf_grid[IGF_SHORT].igf_SfbStart;
    igfConfig->igf_grid[IGF_TCX_LONG].igf_SfbStop    = igfConfig->igf_grid[IGF_SHORT].igf_SfbStop;

    igfConfig->igf_grid[IGF_LONG].igf_Min            = _IGF_get_igfMin(IGF_LONG,
                                                                       ccfl,
                                                                       sampleRate);
    igfConfig->igf_grid[IGF_SHORT].igf_Min           = _IGF_get_igfMin(IGF_SHORT,
                                                                       ccfl,
                                                                       sampleRate);
    igfConfig->igf_grid[IGF_TCX_MEDIUM].igf_Min      = _IGF_get_igfMin(IGF_TCX_MEDIUM,
                                                                       ccfl,
                                                                       sampleRate);
    igfConfig->igf_grid[IGF_TCX_LONG].igf_Min        = _IGF_get_igfMin(IGF_TCX_LONG,
                                                                       ccfl,
                                                                       sampleRate);

    igfConfig->igf_grid[IGF_LONG].igf_NTiles         = _IGF_get_NTiles(&igfConfig->igf_grid[IGF_LONG],
                                                                       igfConfig->igf_UseHighRes,
                                                                       IGFsfbInfoLB->sbk_sfb_top[0] - 1);
    igfConfig->igf_grid[IGF_SHORT].igf_NTiles        = _IGF_get_NTiles(&igfConfig->igf_grid[IGF_SHORT],
                                                                       igfConfig->igf_UseHighRes,
                                                                       IGFsfbInfoSB->sbk_sfb_top[0] - 1);
    igfConfig->igf_grid[IGF_TCX_MEDIUM].igf_NTiles   = igfConfig->igf_grid[IGF_SHORT].igf_NTiles;
    igfConfig->igf_grid[IGF_TCX_LONG].igf_NTiles     = igfConfig->igf_grid[IGF_SHORT].igf_NTiles;

    _IGF_reset(igfDecDataL);
    _IGF_reset(igfDecDataR);
  }
}

void IGF_apply_tnf(
                    HIGF_DEC_DATA             igfDecData,
                    const HIGF_CONFIG         igfConfig,
                    const IGF_WIN_TYPE        igfWinType,
                    const IGF_FRAME_ID        igfFrmID,
                    float                    *coef,
                    const WINDOW_SEQUENCE     win,
                    const byte               *group
) {
  int j;
  int winOffset = 0;
  short swb_offset[64] = {0};
  const int sFidx = (IGF_ID_TCX_MEDIUM_SECOND == igfFrmID) ? 1 : 0;
  const int igfSfbStart = igfConfig->igf_grid[igfWinType].igf_SfbStart;
  const int igfSfbStop  = igfConfig->igf_grid[igfWinType].igf_SfbStop;

  _IGF_get_swb_offset(win,
                      group,
                      igfWinType,
                      winOffset,
                      swb_offset);

  if ((IGF_SHORT != igfWinType) && igfConfig->igf_UseTNF) {
    if (igfDecData->igf_bitstream[sFidx].igf_isTnfFlat == IGF_TNF_ON) {
      _TNF_apply(igfDecData,
                 swb_offset[igfSfbStart],
                 swb_offset[igfSfbStop]);

      for (j = swb_offset[igfSfbStart]; j < swb_offset[igfSfbStop]; j++) {
        if (IGF_TNF_INCLUDE == igfDecData->igf_tnfMask[j]) {
          coef[j] = igfDecData->igf_tnfSpec[j];
        }
      }
    }
  }
}

void IGF_apply_mono(
                    HIGF_DEC_DATA             igfDecData,
                    const HIGF_CONFIG         igfConfig,
                    const IGF_WIN_TYPE        igfWinType,
                    const IGF_FRAME_ID        igfFrmID,
                    float                    *coef,
                    unsigned int             *nfSeed,
                    const WINDOW_SEQUENCE     win,
                    const byte               *group,
                    const int                 num_groups,
                    const short              *group_len,
                    const int                *bins_per_sbk,
                    const int                 blockSize,
                    const int                 samplingRate
) {
  int g;
  int w;
  int startWin;
  int winOffset = 0;
  short swb_offset[64] = {0};
  const int sFidx = (IGF_ID_TCX_MEDIUM_SECOND == igfFrmID) ? 1 : 0;
  unsigned int igf_nfSeed[2] = {*nfSeed, *nfSeed};

  if (0 == igfDecData->igf_bitstream[sFidx].igf_allZero) {
    _IGF_get_swb_offset(win,
                        group,
                        igfWinType,
                        winOffset,
                        swb_offset);

    if ((IGF_SHORT != igfWinType) && igfConfig->igf_UseWhitening) {
      _IGF_spectral_whitening(igfConfig,
                              &igfDecData->igf_bitstream[sFidx],
                              igfDecData->igf_infSpec,
                              swb_offset,
                              igfWinType,
                              blockSize,
                              samplingRate);
    }

    for (g = 0; g < num_groups; g++) {
      startWin = winOffset;
      _IGF_reset_level(&igfDecData->igf_envelope);

      for (w = 0; w < group_len[g]; w++) {
        _IGF_calc(igfConfig,
                  &igfDecData->igf_envelope,
                  &igfDecData->igf_bitstream[sFidx],
                  &coef[winOffset * bins_per_sbk[winOffset]],
                  &igfDecData->igf_infSpec[winOffset * bins_per_sbk[winOffset]],
                  swb_offset,
                  &igf_nfSeed[0],
                  igfWinType,
                  blockSize,
                  samplingRate);
        winOffset++;
      }

      _IGF_norm(igfConfig,
                &igfDecData->igf_envelope,
                igfWinType,
                group_len[g]);
      winOffset = startWin;

      for (w = 0; w < group_len[g]; w++) {
        _IGF_apply_m(igfDecData,
                     igfConfig,
                     &igfDecData->igf_envelope,
                     &igfDecData->igf_bitstream[sFidx],
                     &coef[winOffset * bins_per_sbk[winOffset]],
                     &igfDecData->igf_infSpec[winOffset * bins_per_sbk[winOffset]],
                     swb_offset,
                     &igf_nfSeed[1],
                     igfWinType,
                     blockSize,
                     samplingRate,
                     g);
        winOffset++;
      }
    }
#ifdef _DEBUG
    assert(igf_nfSeed[0] == igf_nfSeed[1]);
#endif
    *nfSeed = igf_nfSeed[0];
  }
}

void IGF_apply_stereo(
                    HIGF_DEC_DATA             igfDecDataL,
                    HIGF_DEC_DATA             igfDecDataR,
                    const HIGF_CONFIG         igfConfig,
                    const IGF_WIN_TYPE        igfWinType,
                    const IGF_FRAME_ID        igfFrmID,
                    float                    *coefL,
                    float                    *coefR,
                    unsigned int             *nfSeedL,
                    unsigned int             *nfSeedR,
                    const WINDOW_SEQUENCE     win,
                    const byte               *group,
                    const byte               *mask,
                    const int               **cplx_pred_used,
                    const int                 hasmask,
                    const int                 num_groups,
                    const short              *group_len,
                    const int                *bins_per_sbk,
                    const int                *sfb_per_sbk,
                    const int                 blockSize,
                    const int                 samplingRate
) {
  int g;
  int w;
  int b;
  int startWin;
  int winOffset = 0;
  short swb_offset[64] = {0};
  int stereoMask[SFB_NUM_MAX] = {0};
  const int sFidx = (IGF_ID_TCX_MEDIUM_SECOND == igfFrmID) ? 1 : 0;
  unsigned int igf_nfSeedL[2] = {*nfSeedL, *nfSeedL};
  unsigned int igf_nfSeedR[2] = {*nfSeedR, *nfSeedR};

  if ((0 == igfDecDataL->igf_bitstream[sFidx].igf_allZero) || (0 == igfDecDataR->igf_bitstream[sFidx].igf_allZero)) {
    _IGF_get_swb_offset(win,
                        group,
                        igfWinType,
                        winOffset,
                        swb_offset);

    _IGF_sync_Data(&igfDecDataL->igf_bitstream[sFidx],
                   &igfDecDataR->igf_bitstream[sFidx]);

    if ((IGF_SHORT != igfWinType) && igfConfig->igf_UseWhitening) {
      _IGF_spectral_whitening(igfConfig,
                              &igfDecDataL->igf_bitstream[sFidx],
                              igfDecDataL->igf_infSpec,
                              swb_offset,
                              igfWinType,
                              blockSize,
                              samplingRate);

      _IGF_spectral_whitening(igfConfig,
                              &igfDecDataR->igf_bitstream[sFidx],
                              igfDecDataR->igf_infSpec,
                              swb_offset,
                              igfWinType,
                              blockSize,
                              samplingRate);
    }

    for (g = 0; g < num_groups; g++) {
      startWin = winOffset;
      _IGF_reset_level(&igfDecDataL->igf_envelope);
      _IGF_reset_level(&igfDecDataR->igf_envelope);

      for (b = 0; b < sfb_per_sbk[g]; b++) {
        stereoMask[b] = 0;
      }

      if (hasmask == 3) {
        for (b = 0; b < sfb_per_sbk[g]; b++) {
          stereoMask[b] = cplx_pred_used[g][b];
        }
      }
      else if (hasmask > 0) {
        for (b = 0; b < sfb_per_sbk[g]; b++) {
          stereoMask[b] = mask[g * sfb_per_sbk[g] + b];
        }
      }

      for (w = 0; w < group_len[g]; w++) {
        _IGF_calc_s(igfConfig,
                    &igfDecDataL->igf_envelope,
                    &igfDecDataR->igf_envelope,
                    &igfDecDataL->igf_bitstream[sFidx],
                    &igfDecDataR->igf_bitstream[sFidx],
                    &coefL[winOffset * bins_per_sbk[winOffset]],
                    &coefR[winOffset * bins_per_sbk[winOffset]],
                    &igfDecDataL->igf_infSpec[winOffset*bins_per_sbk[winOffset]],
                    &igfDecDataR->igf_infSpec[winOffset*bins_per_sbk[winOffset]],
                    stereoMask,
                    swb_offset,
                    &igf_nfSeedL[0],
                    &igf_nfSeedR[0],
                    igfWinType,
                    blockSize,
                    samplingRate);
        winOffset++;
      }

      _IGF_norm(igfConfig,
                &igfDecDataL->igf_envelope,
                igfWinType,
                group_len[g]);

      _IGF_norm(igfConfig,
                &igfDecDataR->igf_envelope,
                igfWinType,
                group_len[g]);
      winOffset = startWin;

      for (w = 0; w < group_len[g]; w++) {
        _IGF_apply_s(igfDecDataL,
                     igfDecDataR,
                     igfConfig,
                     &igfDecDataL->igf_envelope,
                     &igfDecDataR->igf_envelope,
                     &igfDecDataL->igf_bitstream[sFidx],
                     &igfDecDataR->igf_bitstream[sFidx],
                     &coefL[winOffset * bins_per_sbk[winOffset]],
                     &coefR[winOffset * bins_per_sbk[winOffset]],
                     &igfDecDataL->igf_infSpec[winOffset * bins_per_sbk[winOffset]],
                     &igfDecDataR->igf_infSpec[winOffset * bins_per_sbk[winOffset]],
                     swb_offset,
                     &igf_nfSeedL[1],
                     &igf_nfSeedR[1],
                     stereoMask,
                     igfWinType,
                     blockSize,
                     samplingRate,
                     g);
        winOffset++;
      }
    }
#ifdef _DEBUG
    assert(igf_nfSeedL[0] == igf_nfSeedL[1]);
    assert(igf_nfSeedR[0] == igf_nfSeedR[1]);
#endif
    *nfSeedL = igf_nfSeedL[0];
    *nfSeedR = igf_nfSeedR[0];
  }
}
