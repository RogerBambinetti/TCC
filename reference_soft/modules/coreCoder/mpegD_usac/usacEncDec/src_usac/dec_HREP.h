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

 Copyright (c) ISO/IEC 2016.

 ***********************************************************************************/

#ifndef _DEC_HREP_
#define _DEC_HREP_

#include "usac_arith_dec.h"
#include "mod_buf.h"
#include <assert.h>

#define HREP_HALF_BLOCK_SIZE         64
#define HREP_MAX_BLOCKS         64

typedef struct hrep_group_data {
  unsigned int isHREPActive[MAX_TIME_CHANNELS];
  unsigned int idxMinAdj[MAX_TIME_CHANNELS];
  unsigned int defaultBeta[MAX_TIME_CHANNELS];
  unsigned int transWidth[MAX_TIME_CHANNELS];
  unsigned int extGainRange;
  unsigned int extBetaPrec;
} HREP_GROUP_DATA;

typedef struct hrep_data {
  unsigned int useHREP;
  HREP_GROUP_DATA hrepGroupData[10];

  int startChannels[10];

  int gain_idx[HREP_MAX_BLOCKS][MAX_TIME_CHANNELS];
  int betaFactorIdx[MAX_TIME_CHANNELS];
  int numHrepBlocks;
  int numHrepChannels;
  int hrepDelay;

  float delayedGains[2][MAX_TIME_CHANNELS];
  float delay_buf[HREP_HALF_BLOCK_SIZE][MAX_TIME_CHANNELS];
  int sampleCnt[MAX_TIME_CHANNELS];
} HREP_DATA;

/*copied from proto_func.h*/
int CFFTN_NI(float *InRealData,
             float *InImagData,
             float *OutRealData,
             float *OutImagData,
             int len, int isign);

void hrepConfigInit(unsigned char*   buf,
                    unsigned int     bufLen,
                    HREP_GROUP_DATA* hrepGroupData,
                    int              startChannel,
                    int              stopChannel,
                    int*             lfe_channels
                    );

void decodeHrepSideInfo(unsigned char *bitbuf,
                        int bufLen,
                        HREP_DATA *hrepData,
                        int sigGroup,
                        int lfe_channels[2]
                        ); 

void decode_HREP(float* time_samples, 
                int    blockSize, 
                int    currCh,
                HANDLE_MODULO_BUF_VM* hrepInBuffer,
                HREP_DATA *hrepData,
                int    sigGroup
                ); 

#endif