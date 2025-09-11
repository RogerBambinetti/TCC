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

 Copyright (c) ISO/IEC 2015.

 ***********************************************************************************/
#ifndef USAC_MULTICHANNEL_H_
#define USAC_MULTICHANNEL_H_ 1

#include "tf_mainStruct.h"

#define MCT_ANGLE_65_FIX

#define MAX_NUM_MC_CHANNELS 32
#define MAX_NUM_MC_BOXES 120
#define MAX_NUM_MC_BANDS 64
#ifdef MCT_ANGLE_65_FIX
#define CODE_BOOK_BETA_LAV 65
#else
#define CODE_BOOK_BETA_LAV 64
#endif
#define CODE_BOOK_ALPHA_LAV 121
#define DEFAULT_BETA (48)  /*equals 45 degrees */
#define DEFAULT_ALPHA (0)

#ifdef MCT_ANGLE_65_FIX
/* huffman tables */
static const int huff_ctabAngle[CODE_BOOK_BETA_LAV] = {
0x00000000, 0x0000000B, 0x00000012, 0x0000001B, 0x0000001F, 0x00000031, 0x0000003A, 0x00000043,
0x00000065, 0x00000073, 0x00000082, 0x0000009A, 0x000000CE, 0x000000EE, 0x00000106, 0x0000013A,
0x000001D9, 0x000001DE, 0x00000202, 0x00000261, 0x0000020F, 0x0000020E, 0x00000263, 0x00000266,
0x00000272, 0x00000271, 0x00000277, 0x00000276, 0x00000334, 0x00000325, 0x00000326, 0x00000327,
0x00000324, 0x00000323, 0x00000335, 0x00000322, 0x00000320, 0x00000321, 0x00000273, 0x00000270,
0x00000267, 0x00000260, 0x000004C4, 0x000004C5, 0x00000203, 0x000001DF, 0x000001DA, 0x000001D8, 0x0000019B,
0x000001DB, 0x00000132, 0x00000100, 0x000000CF, 0x000000CC, 0x0000009B, 0x00000081, 0x00000072,
0x0000004F, 0x00000042, 0x00000038, 0x00000030, 0x0000001E, 0x0000001A, 0x00000011, 0x0000000A
};

static const int huff_ltabAngle[CODE_BOOK_BETA_LAV] = {
0x00000001, 0x00000004, 0x00000005, 0x00000005, 0x00000005, 0x00000006, 0x00000006, 0x00000007,
0x00000007, 0x00000007, 0x00000008, 0x00000008, 0x00000008, 0x00000008, 0x00000009, 0x00000009,
0x00000009, 0x00000009, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A,
0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A,
0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A,
0x0000000A, 0x0000000A, 0x0000000B, 0x0000000B, 0x0000000A, 0x00000009, 0x00000009, 0x00000009, 0x00000009,
0x00000009, 0x00000009, 0x00000009, 0x00000008, 0x00000008, 0x00000008, 0x00000008, 0x00000007,
0x00000007, 0x00000007, 0x00000006, 0x00000006, 0x00000005, 0x00000005, 0x00000005, 0x00000004
};
#else
/* huffman tables for angles */
static const int huff_ctabAngle[CODE_BOOK_BETA_LAV] = {
0x00000000, 0x0000000B, 0x00000012, 0x0000001B, 0x0000001F, 0x00000031, 0x0000003A, 0x00000043,
0x00000065, 0x00000073, 0x00000082, 0x0000009A, 0x000000CE, 0x000000EE, 0x00000106, 0x0000013A,
0x000001D9, 0x000001DE, 0x00000202, 0x00000261, 0x0000020F, 0x0000020E, 0x00000263, 0x00000266,
0x00000272, 0x00000271, 0x00000277, 0x00000276, 0x00000334, 0x00000325, 0x00000326, 0x00000327,
0x00000324, 0x00000323, 0x00000335, 0x00000322, 0x00000320, 0x00000321, 0x00000273, 0x00000270,
0x00000267, 0x00000260, 0x00000262, 0x00000203, 0x000001DF, 0x000001DA, 0x000001D8, 0x0000019B,
0x000001DB, 0x00000132, 0x00000100, 0x000000CF, 0x000000CC, 0x0000009B, 0x00000081, 0x00000072,
0x0000004F, 0x00000042, 0x00000038, 0x00000030, 0x0000001E, 0x0000001A, 0x00000011, 0x0000000A
};

static const int huff_ltabAngle[CODE_BOOK_BETA_LAV] = {
0x00000001, 0x00000004, 0x00000005, 0x00000005, 0x00000005, 0x00000006, 0x00000006, 0x00000007,
0x00000007, 0x00000007, 0x00000008, 0x00000008, 0x00000008, 0x00000008, 0x00000009, 0x00000009,
0x00000009, 0x00000009, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A,
0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A,
0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A,
0x0000000A, 0x0000000A, 0x0000000A, 0x0000000A, 0x00000009, 0x00000009, 0x00000009, 0x00000009,
0x00000009, 0x00000009, 0x00000009, 0x00000008, 0x00000008, 0x00000008, 0x00000008, 0x00000007,
0x00000007, 0x00000007, 0x00000006, 0x00000006, 0x00000005, 0x00000005, 0x00000005, 0x00000004
};
#endif

static const int huff_ctabscf[CODE_BOOK_ALPHA_LAV]=
{
0x0003ffe8, 0x0003ffe6, 0x0003ffe7, 0x0003ffe5, 0x0007fff5, 0x0007fff1, 0x0007ffed, 0x0007fff6,
0x0007ffee, 0x0007ffef, 0x0007fff0, 0x0007fffc, 0x0007fffd, 0x0007ffff, 0x0007fffe, 0x0007fff7,
0x0007fff8, 0x0007fffb, 0x0007fff9, 0x0003ffe4, 0x0007fffa, 0x0003ffe3, 0x0001ffef, 0x0001fff0,
0x0000fff5, 0x0001ffee, 0x0000fff2, 0x0000fff3, 0x0000fff4, 0x0000fff1, 0x00007ff6, 0x00007ff7,
0x00003ff9, 0x00003ff5, 0x00003ff7, 0x00003ff3, 0x00003ff6, 0x00003ff2, 0x00001ff7, 0x00001ff5,
0x00000ff9, 0x00000ff7, 0x00000ff6, 0x000007f9, 0x00000ff4, 0x000007f8, 0x000003f9, 0x000003f7,
0x000003f5, 0x000001f8, 0x000001f7, 0x000000fa, 0x000000f8, 0x000000f6, 0x00000079, 0x0000003a,
0x00000038, 0x0000001a, 0x0000000b, 0x00000004, 0x00000000, 0x0000000a, 0x0000000c, 0x0000001b,
0x00000039, 0x0000003b, 0x00000078, 0x0000007a, 0x000000f7, 0x000000f9, 0x000001f6, 0x000001f9,
0x000003f4, 0x000003f6, 0x000003f8, 0x000007f5, 0x000007f4, 0x000007f6, 0x000007f7, 0x00000ff5,
0x00000ff8, 0x00001ff4, 0x00001ff6, 0x00001ff8, 0x00003ff8, 0x00003ff4, 0x0000fff0, 0x00007ff4,
0x0000fff6, 0x00007ff5, 0x0003ffe2, 0x0007ffd9, 0x0007ffda, 0x0007ffdb, 0x0007ffdc, 0x0007ffdd,
0x0007ffde, 0x0007ffd8, 0x0007ffd2, 0x0007ffd3, 0x0007ffd4, 0x0007ffd5, 0x0007ffd6, 0x0007fff2,
0x0007ffdf, 0x0007ffe7, 0x0007ffe8, 0x0007ffe9, 0x0007ffea, 0x0007ffeb, 0x0007ffe6, 0x0007ffe0,
0x0007ffe1, 0x0007ffe2, 0x0007ffe3, 0x0007ffe4, 0x0007ffe5, 0x0007ffd7, 0x0007ffec, 0x0007fff4,
0x0007fff3
};

static int huff_ltabscf[CODE_BOOK_ALPHA_LAV]=
{
0x00000012, 0x00000012, 0x00000012, 0x00000012, 0x00000013, 0x00000013, 0x00000013, 0x00000013,
0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013,
0x00000013, 0x00000013, 0x00000013, 0x00000012, 0x00000013, 0x00000012, 0x00000011, 0x00000011,
0x00000010, 0x00000011, 0x00000010, 0x00000010, 0x00000010, 0x00000010, 0x0000000f, 0x0000000f,
0x0000000e, 0x0000000e, 0x0000000e, 0x0000000e, 0x0000000e, 0x0000000e, 0x0000000d, 0x0000000d,
0x0000000c, 0x0000000c, 0x0000000c, 0x0000000b, 0x0000000c, 0x0000000b, 0x0000000a, 0x0000000a,
0x0000000a, 0x00000009, 0x00000009, 0x00000008, 0x00000008, 0x00000008, 0x00000007, 0x00000006,
0x00000006, 0x00000005, 0x00000004, 0x00000003, 0x00000001, 0x00000004, 0x00000004, 0x00000005,
0x00000006, 0x00000006, 0x00000007, 0x00000007, 0x00000008, 0x00000008, 0x00000009, 0x00000009,
0x0000000a, 0x0000000a, 0x0000000a, 0x0000000b, 0x0000000b, 0x0000000b, 0x0000000b, 0x0000000c,
0x0000000c, 0x0000000d, 0x0000000d, 0x0000000d, 0x0000000e, 0x0000000e, 0x00000010, 0x0000000f,
0x00000010, 0x0000000f, 0x00000012, 0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013,
0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013,
0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013,
0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013,
0x00000013
};

/* quantized sin/cos tables for index */
static const float tabIndexToSinAlpha[CODE_BOOK_BETA_LAV] = {
-1.000000f, -0.998795f, -0.995185f, -0.989177f, -0.980785f, -0.970031f, -0.956940f, -0.941544f,
-0.923880f, -0.903989f, -0.881921f, -0.857729f, -0.831470f, -0.803208f, -0.773010f, -0.740951f,
-0.707107f, -0.671559f, -0.634393f, -0.595699f, -0.555570f, -0.514103f, -0.471397f, -0.427555f,
-0.382683f, -0.336890f, -0.290285f, -0.242980f, -0.195090f, -0.146730f, -0.098017f, -0.049068f,
 0.000000f,  0.049068f,  0.098017f,  0.146730f,  0.195090f,  0.242980f,  0.290285f,  0.336890f,
 0.382683f,  0.427555f,  0.471397f,  0.514103f,  0.555570f,  0.595699f,  0.634393f,  0.671559f,
 0.707107f,  0.740951f,  0.773010f,  0.803208f,  0.831470f,  0.857729f,  0.881921f,  0.903989f,
 0.923880f,  0.941544f,  0.956940f,  0.970031f,  0.980785f,  0.989177f,  0.995185f,  0.998795f
#ifdef MCT_ANGLE_65_FIX
,1.000000f
#endif
};

static const float tabIndexToCosAlpha[CODE_BOOK_BETA_LAV] = {
0.000000f, 0.049068f, 0.098017f, 0.146730f, 0.195090f, 0.242980f, 0.290285f, 0.336890f,
0.382683f, 0.427555f, 0.471397f, 0.514103f, 0.555570f, 0.595699f, 0.634393f, 0.671559f,
0.707107f, 0.740951f, 0.773010f, 0.803208f, 0.831470f, 0.857729f, 0.881921f, 0.903989f,
0.923880f, 0.941544f, 0.956940f, 0.970031f, 0.980785f, 0.989177f, 0.995185f, 0.998795f,
1.000000f, 0.998795f, 0.995185f, 0.989177f, 0.980785f, 0.970031f, 0.956940f, 0.941544f,
0.923880f, 0.903989f, 0.881921f, 0.857729f, 0.831470f, 0.803208f, 0.773010f, 0.740951f,
0.707107f, 0.671559f, 0.634393f, 0.595699f, 0.555570f, 0.514103f, 0.471397f, 0.427555f,
0.382683f, 0.336890f, 0.290285f, 0.242980f, 0.195090f, 0.146730f, 0.098017f, 0.049068f
#ifdef MCT_ANGLE_65_FIX
,0.000000f
#endif
};

typedef struct multichannel_data_tag {
  int stereoFilling;
  int signalingType;
  int useTool;
  int keepTree;
  int numPairs;
  int startElement;
  int startChannel;
  int windowSequenceIsLongPrev[MAX_NUM_MC_BOXES];
  int channelMask[MAX_NUM_MC_CHANNELS];
  int channelMap[MAX_NUM_MC_CHANNELS];
  int numChannelsToApply;
  int codePairs[MAX_NUM_MC_BOXES][2];
  int pairCoeffQSfbPrev[MAX_NUM_MC_BOXES][MAX_NUM_MC_BANDS];
  int pairCoeffQFbPrev[MAX_NUM_MC_BOXES]; /* fullband angle */
  int bDeltaTime[MAX_NUM_MC_BOXES];
  int predDir[MAX_NUM_MC_BOXES];
  int pairCoeffQSfb[MAX_NUM_MC_BOXES][MAX_NUM_MC_BANDS];
  int numMaskBands[MAX_NUM_MC_BOXES];
  int mask[MAX_NUM_MC_BOXES][MAX_NUM_MC_BANDS];
  int bHasMask[MAX_NUM_MC_BOXES];
  int bHasBandwiseCoeffs[MAX_NUM_MC_BOXES];
  int bHasStereoFilling[MAX_NUM_MC_BOXES];
  float prevOutSpec[MAX_NUM_MC_CHANNELS][LN2];
} MULTICHANNEL_DATA;

int usac_multichannel_init(MULTICHANNEL_DATA **mcData,
                           unsigned char *bitbuf,
                           unsigned char nBitbufLen,
                           int nChannels,
                           int startChannel,
                           int startElement);

int usac_multichannel_parse(MULTICHANNEL_DATA *mcData,
                            unsigned char *bitbuf,
                            unsigned int nBitBufLen,
                            const int indepFlag,
                            Info *sfbInfo);

void usac_multichannel_save_prev(MULTICHANNEL_DATA *mcData,
                                 float *spec,
                                 const int ch,
                                 const int num_groups,
                                 const int *bins_per_sbk,
                                 const int zeroSpecSaves);

void usac_multichannel_get_prev_dmx(MULTICHANNEL_DATA *mcData,
                                    float *prevSpec1,
                                    float *prevSpec2,
                                    float *prevDmx,
                                    const int bandsPerWindow,
                                    const int *mask,
                                    const int *coeffSfbIdx,
                                    const int nSamples,
                                    const int pair,
                                    const int totalSfb,
                                    const short *sfbOffset);

void usac_multichannel_stereofilling_add(float *coef, float *dmx_prev, float *scaleFactors, const int total_sfb,
                                         const int group_len, const int bins_per_sbk, const short *sbk_sfb_top);

void usac_multichannel_process(MULTICHANNEL_DATA *mcData,
                               float *dmx,
                               float *res,
                               int *coeffSfbIdx,
                               int *mask,
                               int bandsPerWindow,
                               int totalSfb,
                               int pair,
                               const short *sfbOffset,
                               int nSamples);

#endif /* USAC_MULTICHANNEL_H_*/
