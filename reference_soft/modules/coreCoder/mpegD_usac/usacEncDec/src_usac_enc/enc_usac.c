/************************************************************************

This software module was originally developed by Fraunhofer IIS and 
VoiceAge Corp. in the course of development of the ISO/IEC 23008-3 for 
reference purposes and its  performance may not have been optimized. 
This software module is an implementation of one or more tools as 
specified by the ISO/IEC 23008-3 standard. ISO/IEC gives you a 
royalty-free, worldwide, non-exclusive,copyright license to copy, 
distribute, and make derivative works of this software module or 
modifications thereof for use in implementations or products claiming 
conformance to the ISO/IEC 23008-3 standard and which satisfy any specified 
conformance criteria. 
Those intending to use this software module in products are advised that 
its use may infringe existing patents.

ISO/IEC have no liability for use of this software module or 
modifications thereof. Copyright is not released for products that do
not conform to the ISO/IEC 23008-3 standard.

Fraunhofer IIS and VoiceAge Corp. retain full right to modify and use the 
code for its own purpose, assign or donate the code to a third party and 
to inhibit third parties from using the code for products that do not 
conform to MPEG-related ITU Recommendations and/or ISO/IEC International 
Standards.

This copyright notice must be included in all copies or derivative works.

Copyright (c) ISO/IEC 2008.

*************************************************************************/


#include <limits.h>
#include <math.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "allHandles.h"
#include "encoder.h"

#include "block.h"
#include "ntt_win_sw.h"

#include "nok_ltp_common.h"      /* structs */
#include "obj_descr.h"           /* structs */


#include "enc_usac.h"
#include "enc_tf.h"
#include "usac_mainStruct.h"   /* structs */
/*#include "usac_main.h"*/
#include "usac_mdct.h"
#include "tf_main.h"

#include "common_m4a.h"	/* common module */

/*sbr*/
#include "ct_sbr.h"

#include "sac_enc.h"
#include "sac_stream_enc.h"

/* aac */
#include "ms.h"
#include "psych.h"
#include "tns.h"
#include "tns3.h"
#include "scal_enc_frame.h"
#include "usac_fd_enc.h"
#include "usac_fd_qc.h"


#include "int_dec.h"

/*bitmux*/
#include "bitstream.h"
#include "usac_bitmux.h"
#include "flex_mux.h"

#include "plotmtv.h"
#include "usac_tw_enc.h"

#include "../src_usac/acelp_plus.h"
#include "usac_tw_tools.h"

#include "signal_classifier.h"

#ifdef SONY_PVC
#include "sony_pvcenc_prepro.h"
#endif /* SONY_PVC */

#include "usac_config.h"
#include "enc_IGF.h"
#include "mct_enc.h"

#define FIX_USE_BYTE_BITRES
#define USAC_MAX_AUDIO_GROUPS 32

/* declarations */
static int EncUsac_data_free(HANDLE_ENCODER_DATA data);
static int EncUsac_tns_free(HANDLE_ENCODER_DATA data);

/* 20070530 SBR */
static SBR_CHAN sbrChan[MAX_TIME_CHANNELS];
extern int samplFreqIndex[];
extern long UsacSamplingFrequencyTable[32];

int g_useSplitTransform = 0;

#ifdef __RESTRICT
#define restrict _Restrict
#else
#define restrict
#endif


int gUseFractionalDelayDecor = 0;
static FILE* gExtSAOCMetadataBitrateFileHandle = NULL;
static FILE* gExtSAOCMetadataFileHandle  = NULL;
static FILE* gExtHOAMetadataBitrateFileHandle = NULL;
static FILE* gExtHOAMetadataFileHandle  = NULL;
#if IAR
static FILE* gIARBS = NULL;
#endif
static FILE* gProdMetadataConfigFileHandle = NULL;


/* encoded AU data */
struct mpegh3daFrame_data {
  unsigned int auLen;
  unsigned char au_data[USAC_MAX_EXTENSION_PAYLOAD_LEN];
};

/* encoded ASC data */
struct mpegh3daConfig_data {
  unsigned int configLen;
  unsigned char config_data[USAC_MAX_CONFIG_LEN];
};

/* audio pre-roll state */
typedef enum audioPreRoll_state {
  APR_IDLE,
  APR_PREPARE,
  APR_EMBED,
  APR_DELETE
} APR_STATE, *HANDLE_APR_STATE;

/* audio pre-roll bit distribution */
typedef struct audioPreRoll_bitbudget {
  int bits2Distribute;
  int bits2Save;
  int spreadFrames;
} APR_BITBUDGET, *HANDLE_APR_BITBUDGET;

/* audio pre-roll data */
typedef struct audioPreRollData {
  APR_STATE audioPreRollState;
  APR_BITBUDGET audioPreRollBitBudget;
  int audioPreRollExtID;
  MPEGH3DACONFIG_DATA mpegh3daConfig;
  int applyCrossfade;
  int reserved;
  int numPreRollFrames;
  MPEGH3DAFRAME_DATA mpegh3daFrame;
} APR_DATA, *HANDLE_APRDATA;

/*** the main container structure ***/
struct tagEncoderSpecificData {
  int     debugLevel;

  int     bitrate;
  int     channels;
  int     bw_limit;  /* bandwidth limit of the spectrum */
  int     sampling_rate;      /* core coder sampling rate */
  int     sampling_rate_out;  /* output (i.e. SBR) sampling rate */
  int     block_size_samples; /* nr of samples per block in one audio channel */
  int     window_size_samples[MAX_TIME_CHANNELS];
  int     delay_encoder_frames;

#ifdef USE_FILL_ELEMENT
  int     bUseFillElement;
#endif

  unsigned int receiverDelayCompensation;
  unsigned int coreQmfDelayCompensation;

  /*--- USAC independency flag ---*/
  int     usacIndependencyFlagInterval;
  int     usacIndependencyFlagCnt;

  /*--- TD/FD selection ---*/
  int frameCounter;
  int switchEveryNFrames;
  enum USAC_CODEC_MODE    codecMode;
  USAC_CORE_MODE   coreMode[MAX_TIME_CHANNELS];
  USAC_CORE_MODE   prev_coreMode[MAX_TIME_CHANNELS];
  USAC_CORE_MODE   next_coreMode[MAX_TIME_CHANNELS];

  /*---TD Data---*/
  float  tdBuffer[MAX_TIME_CHANNELS][L_FRAME_1024+L_NEXT_HIGH_RATE_1024];
  float  tdBuffer_past[MAX_TIME_CHANNELS][L_FRAME_1024+L_NEXT_HIGH_RATE_1024+L_LPC0_1024];
  HANDLE_USAC_TD_ENCODER tdenc[MAX_TIME_CHANNELS];
  int    total_nbbits[MAX_TIME_CHANNELS];   /* (o) : number of bits per superframe    */
  int    FD_nbbits_fac[MAX_TIME_CHANNELS];   /* (o) : number of bits used per FD FAC window    */
  int    TD_nbbits_fac[MAX_TIME_CHANNELS];   /* (o) : number of bits used per TD FAC window    */
  unsigned char TDfacData[MAX_TIME_CHANNELS][4*NBITS_MAX];
  int    td_bitrate[MAX_TIME_CHANNELS];
  int    acelp_core_mode[MAX_TIME_CHANNELS];

  /*---FD Data---*/
  int     flag_960;
  int     flag_768;
  int     ep_config;
  int     flag_twMdct;
  int     flag_noiseFilling;
  int     flag_harmonicBE;
  int     flag_winSwitchFD;
  int     layer_num;
  int     track_num;
  int     tracks_per_layer[MAX_TF_LAYER];
  int     max_bitreservoir_bits;
  int     available_bitreservoir_bits;
  TNS_COMPLEXITY        tns_select;
  int     ms_select;
  int     aacAllowScalefacs;
  int     max_sfb;
  float   warp_sum[MAX_TIME_CHANNELS][2];
  float  *warp_cont_mem[MAX_TIME_CHANNELS];

  MSInfo  msInfo[MAX_TIME_CHANNELS];
  int     predCoef[MAX_TIME_CHANNELS][MAX_SHORT_WINDOWS][SFB_NUM_MAX];
  TNS_INFO *tnsInfo[MAX_TIME_CHANNELS];
  UsacICSinfo icsInfo[MAX_TIME_CHANNELS];
  UsacToolsInfo toolsInfo[MAX_TIME_CHANNELS];
  UsacQuantInfo quantInfo[MAX_TIME_CHANNELS];

  WINDOW_SHAPE windowShapePrev[MAX_TIME_CHANNELS];
  WINDOW_SEQUENCE windowSequence[MAX_TIME_CHANNELS];
  HANDLE_NTT_WINSW win_sw_module;
  HANDLE_TFDEC hTfDec;

  int enhancedNoiseFilling;
  int igFStartFq;
  int igFStopFq;
  int igFStartIdx;
  int igFStopIdx;
  int igFStartSfbLB;
  int igFStopSfbLB;
  int igFStartSfbSB;
  int igFStopSfbSB;
  int igFUseHighRes;
  int igFUseWhitening;
  int igFIndependentTiling;
  int igFAfterTnsSynth;
#ifdef RM1_3D_BUGFIX_IGF_05
  int igFNTilesLB;
  int igFNTilesSB;
#endif
  /*--- SBR Data ---*/
  int sbrRatioIndex;
  int sbrenable;
  struct channelElementData chElmData[MAX_CHANNEL_ELEMENT];
  int ch_elm_total;

  /*--- MPEGS 212 Data ---*/
  HANDLE_SPATIAL_ENC hSpatialEnc[USAC_MAX_ELEMENTS];
  int usac212enable;
  unsigned char SpatialData[USAC_MAX_ELEMENTS][MAX_MPEGS_BITS/8];
  unsigned long SpatialDataLength[USAC_MAX_ELEMENTS];
  int         flag_ipd;
  int         flag_mpsres;
  int         flag_cplxPred;
  int         flag_mpsqce;
  int         noiseFillingMode;
  char        tsdInputFileName[FILENAME_MAX];
  int         hybridResidualMode;

  /*--- Data ---*/
  double *DTimeSigBuf[MAX_TIME_CHANNELS];
  double *DTimeSigLookAheadBuf[MAX_TIME_CHANNELS];
  double *spectral_line_vector[MAX_TIME_CHANNELS];
  double *twoFrame_DTimeSigBuf[MAX_TIME_CHANNELS]; /* temporary fix to the buffer size problem. */
  double *overlap_buffer[MAX_TIME_CHANNELS];
  short  tdOutStream[MAX_TIME_CHANNELS][NBITS_MAX];
  short  facOutStream[MAX_TIME_CHANNELS][NBITS_MAX];

  /*--- Synth ---*/
  double *reconstructed_spectrum[MAX_TIME_CHANNELS];

  /*--- 3D Audio Multi-Element Map */
  int channelElementType[MAX_TIME_CHANNELS];
  int channelElementIndex[MAX_TIME_CHANNELS];
  int isQceElement[MAX_USAC_ELEMENTS];


  /* --- 3D Audio Extension Data */
  char objFile[FILENAME_MAX];
  char saocFile[FILENAME_MAX];
  char saocBitrateFile[FILENAME_MAX];

#ifdef SPECIFY_INPUT_CONTENTTYPE
  INPUT_CONTENT_TYPE_3D contentType;
#endif

  char hoaFile[FILENAME_MAX];
  char hoaBitrateFile[FILENAME_MAX];
#if IAR
  char iarFile[FILENAME_MAX];
#endif
  char pmcFile[FILENAME_MAX];
  char pmfFile[FILENAME_MAX];

  int  hasDynamicObjectPriority;
  int  lowDelayMetadataCoding;
  int  OAMFrameLength;
  int  hasUniformSpread;

  int  cicpIn;

  int numExtElements;
  USAC_ID_EXT_ELE  extType[USAC_MAX_EXTENSION_PAYLOADS];
  unsigned char    extElementConfigPayload[USAC_MAX_EXTENSION_PAYLOADS][USAC_MAX_EXTENSION_PAYLOAD_LEN];
  unsigned int     extElementConfigLength[USAC_MAX_EXTENSION_PAYLOADS];

  int       mctMode;
  MCTData   mctData;

  APR_DATA  aprData;

  int useLcProfile;
  int useBlProfile;
  unsigned int numCompatibleSets;
  unsigned int CompatibleSetIndication[USAC_MAX_COMPATIBLE_PROFILE_LEVELS];
  unsigned int hasBlProfileCompatibility;
};

#define NUMBER_OF_ALLOWED_SAMPLING_RATES (12)
const int allowedSamplingRates[NUMBER_OF_ALLOWED_SAMPLING_RATES] = {14700, 16000, 22050, 24000, 29400, 32000, 44100, 48000, 58800, 64000, 88200, 96000};


static int EncUsacAudioPreRoll_reset(HANDLE_APRDATA       aprData,
                                     APR_STATE            resetConfig,
                                     APR_STATE            resetAU
) {
  int err = 0;

  aprData->audioPreRollState  = APR_IDLE;
  aprData->applyCrossfade     = 0;
  aprData->reserved           = 0;
  aprData->numPreRollFrames   = 1;

  if (APR_PREPARE == resetConfig) {
    aprData->mpegh3daConfig.configLen = 0;
    memset(aprData->mpegh3daConfig.config_data, 0, sizeof(MPEGH3DACONFIG_DATA));
  }

  if (APR_PREPARE == resetAU) {
    aprData->mpegh3daFrame.auLen = 0;
    memset(aprData->mpegh3daFrame.au_data, 0, sizeof(MPEGH3DAFRAME_DATA));
  }

  return err;
}

static int EncUsacAudioPreRoll_init(HANDLE_APRDATA       aprData,
                                    USAC_ID_EXT_ELE     *extType,
                                    unsigned int        *extElementConfigLength,
                                    int                 *numExtElements

) {
  int err = 0;

  /* reset internal data */
  EncUsacAudioPreRoll_reset(aprData,
                            APR_PREPARE,
                            APR_PREPARE);

  aprData->audioPreRollBitBudget.bits2Distribute = 0;
  aprData->audioPreRollBitBudget.bits2Save       = 0;
  aprData->audioPreRollBitBudget.spreadFrames    = 20;    /* spread AudioPreRoll bit demand over 20 frames */
  aprData->audioPreRollExtID                     = *numExtElements;

  /* register AudioPreRoll in the mpegh3daExtElementConfig */
  extType[aprData->audioPreRollExtID]                = USAC_ID_EXT_ELE_AUDIOPREROLL;
  extElementConfigLength[aprData->audioPreRollExtID] = 0;
  (*numExtElements)++;

  return err;
}

static int EncUsacAudioPreRoll_prepare(HANDLE_APRDATA       aprData,
                                       const int            nextIsIPF,
                                       int                 *bUsacIndependencyFlag,
                                       USAC_ID_EXT_ELE     *extType,
                                       int                 *numExtElements
) {
  int err = 0;

  /* don't write AudioPreRoll() extension any longer */
  if (APR_DELETE == aprData->audioPreRollState) {
    EncUsacAudioPreRoll_reset(aprData,
                              APR_IDLE,                 /* don't reset the config */
                              APR_PREPARE);

    /* idle APR chain */
    aprData->audioPreRollState = APR_IDLE;
  }

  /* add AudioPreRoll() to extension payload */
  if (APR_EMBED == aprData->audioPreRollState) {
    /* set the indep flag in the frame where the AudioPreRoll() is embedded */
    *bUsacIndependencyFlag = 1;
  }

  /* this frame will be the AU for the AudioPreRoll in the next IPF frame */
  if (1 == nextIsIPF) {
    if (APR_IDLE == aprData->audioPreRollState) {
      EncUsacAudioPreRoll_reset(aprData,
                                APR_IDLE,                 /* don't reset the config */
                                APR_PREPARE);
    }

    /* set the indep flag in the frame where the AU is buffered for the AudioPreRoll() */
    *bUsacIndependencyFlag = 1;

    /* (re-)start APR chain */
    aprData->audioPreRollState = APR_PREPARE;
  }

  return err;
}

static int EncUsacAudioPreRoll_setConfig(HANDLE_ENCODER_DATA     data,
                                         HANDLE_BSBITBUFFER     *asc,
                                         DEC_CONF_DESCRIPTOR    *dec_conf
) {
  int err                       = 0;
  int i                         = 0;
  int bitCount                  = 0;
  HANDLE_BSBITSTREAM output_asc = NULL;
  USAC_CONFIG* pUsacCfg         = &(dec_conf[0].audioSpecificConfig.specConf.usacConfig);

  /* open asc bs bitstream */
  output_asc = BsOpenBufferWrite(asc[i]);
  if (output_asc == NULL) {
    CommonWarning("EncUsacAudioPreRoll_setConfig: error opening output space");
    return -1;
  }

  /* encode usac config -> mpegh3daConfig() */
  UsacConfig_Init(pUsacCfg, 1);
  bitCount = UsacConfig_Advance(output_asc, pUsacCfg, 1);

  if (0 == bitCount) {
    CommonWarning("EncUsacAudioPreRoll_setConfig: no mpegh3daConfig present");
    return -1;
  }

  /* transport config to AudioPreRoll */
  data->aprData.mpegh3daConfig.configLen = ((bitCount + 7) >> 3);

  if (data->aprData.mpegh3daConfig.configLen > sizeof(MPEGH3DACONFIG_DATA)) {
    CommonWarning("EncUsacAudioPreRoll_setConfig: memory out of bounds");
    return -2;
  }
  memcpy(data->aprData.mpegh3daConfig.config_data, BsBufferGetDataBegin(asc[0]), sizeof(unsigned char) * data->aprData.mpegh3daConfig.configLen);

  /* close asc bs bitstream */
  BsClose(output_asc);

  return err;
}

static int EncUsacAudioPreRoll_setAU(HANDLE_APRDATA       aprData,
                                     const unsigned int   au_nBits,
#ifdef _DEBUG
                                     const long           au_Bits_debug,
#endif
                                     const unsigned char* au_data
) {
  int err = 0;

#ifdef _DEBUG
  assert(au_nBits == au_Bits_debug);
#endif

  if (APR_PREPARE == aprData->audioPreRollState) {
    aprData->audioPreRollState   = APR_EMBED;
    aprData->mpegh3daFrame.auLen = ((au_nBits + 7) >> 3);

    if (aprData->mpegh3daFrame.auLen > sizeof(MPEGH3DAFRAME_DATA)) {
      CommonWarning("EncUsacAudioPreRoll_setAU: memory out of bounds");
      return -1;
    }

    memcpy(aprData->mpegh3daFrame.au_data, au_data, sizeof(unsigned char) * aprData->mpegh3daFrame.auLen);
  }

  return err;
}

static int EncUsac_writeEscapedValue(HANDLE_BSBITSTREAM   bitStream,
                                     unsigned int         value,
                                     unsigned int         nBits1,
                                     unsigned int         nBits2,
                                     unsigned int         nBits3,
                                     unsigned int         WriteFlag
) {
  int bitCount             = 0;
  unsigned long valueLeft  = value;
  unsigned long valueWrite = 0;
  unsigned long maxValue1  = (1 << nBits1) - 1;
  unsigned long maxValue2  = (1 << nBits2) - 1;
  unsigned long maxValue3  = (1 << nBits3) - 1;

  valueWrite = min(valueLeft, maxValue1);
  bitCount += BsRWBitWrapper(bitStream, &valueWrite, nBits1, WriteFlag);

  if(valueWrite == maxValue1){
    valueLeft = valueLeft - valueWrite;

    valueWrite = min(valueLeft, maxValue2);
    bitCount += BsRWBitWrapper(bitStream, &valueWrite, nBits2, WriteFlag);

    if(valueWrite == maxValue2){
      valueLeft = valueLeft - valueWrite;

      valueWrite = min(valueLeft, maxValue3);
      bitCount += BsRWBitWrapper(bitStream, &valueWrite, nBits3, WriteFlag);
#ifdef _DEBUG
      assert((valueLeft - valueWrite) == 0);
#endif
    }
  }

  return bitCount;
}

static int EncUsac_writeBits(HANDLE_BSBITSTREAM   bitStream,
                             unsigned int         data,
                             int                  numBit
) {
  BsPutBit(bitStream, data, numBit);

  return numBit;
}

static int EncUsacAudioPreRoll_writeExtData(HANDLE_APRDATA       aprData,
                                            unsigned char       *extensionData,
                                            int                 *extensionDataSize,
                                            int                 *extensionDataPresent
) {
  int err                     = 0;
  unsigned int i              = 0;
  int frameIdx                = 0;
  int bitCount                = 0;
  const int maxNumBits        = ((USAC_MAX_EXTENSION_PAYLOAD_LEN + USAC_MAX_CONFIG_LEN) << 3) + 56;
  unsigned char *tmp          = NULL;
  HANDLE_BSBITSTREAM bs       = NULL;
  HANDLE_BSBITBUFFER bsBuffer = NULL;

  *extensionDataSize    = 0;
  *extensionDataPresent = 0;

  if (APR_EMBED == aprData->audioPreRollState) {
    if (0 != aprData->audioPreRollExtID) {
      CommonWarning("EncUsacAudioPreRoll_writeExtData: The first element of every frame shall be an extension element (mpegh3daExtElement) of type ID_EXT_ELE_AUDIOPREROLL.");
      return -1;
    }

    bsBuffer = BsAllocBuffer(maxNumBits);

    bs = BsOpenBufferWrite(bsBuffer);

    bitCount += EncUsac_writeEscapedValue(bs, aprData->mpegh3daConfig.configLen, 4, 4, 8, 1);

    for (i = 0; i < aprData->mpegh3daConfig.configLen; i++) {
      bitCount += EncUsac_writeBits(bs, aprData->mpegh3daConfig.config_data[i], 8);
    }

    bitCount += EncUsac_writeBits(bs, aprData->applyCrossfade, 1);
    bitCount += EncUsac_writeBits(bs, aprData->reserved, 1);

    bitCount += EncUsac_writeEscapedValue(bs, aprData->numPreRollFrames, 2, 4, 0, 1);

    for (frameIdx = 0; frameIdx < aprData->numPreRollFrames; frameIdx++) {
      bitCount += EncUsac_writeEscapedValue(bs, aprData->mpegh3daFrame.auLen, 16, 16, 0, 1);

      for (i = 0; i < aprData->mpegh3daFrame.auLen; i++) {
        bitCount += EncUsac_writeBits(bs, aprData->mpegh3daFrame.au_data[i], 8);
      }
    }

    if (bitCount % 8) {
      EncUsac_writeBits(bs, 0 , 8 - (bitCount % 8));
    }

    tmp = BsBufferGetDataBegin(bsBuffer);
    *extensionDataSize  = ((bitCount + 7) >> 3);

    /* extension data is present */
    if ((*extensionDataSize) > 0) {
      memcpy(extensionData, tmp, *extensionDataSize);
      *extensionDataPresent = 1;
    }

    BsClose(bs);
    BsFreeBuffer(bsBuffer);

    /* distribute AudioPreRoll() bits over the next frames */
    aprData->audioPreRollBitBudget.bits2Distribute += ((*extensionDataSize) << 3);
    aprData->audioPreRollBitBudget.bits2Save        = (aprData->audioPreRollBitBudget.bits2Distribute + aprData->audioPreRollBitBudget.spreadFrames - 1) / aprData->audioPreRollBitBudget.spreadFrames;

    /* clean-up APR chain */
    aprData->audioPreRollState = APR_DELETE;
  }

  return err;
}

static int EncUsacAudioPreRoll_bitDistribution(HANDLE_APRDATA       aprData,
                                               const int            extElementID
) {
  int savedBits = 0;

  if ((aprData->audioPreRollBitBudget.bits2Distribute > 0) && (aprData->audioPreRollExtID == extElementID)) {
    savedBits = aprData->audioPreRollBitBudget.bits2Save;

    if (aprData->audioPreRollBitBudget.bits2Distribute >= savedBits) {
      aprData->audioPreRollBitBudget.bits2Distribute -= savedBits;
    } else {
      savedBits = aprData->audioPreRollBitBudget.bits2Distribute;
      aprData->audioPreRollBitBudget.bits2Distribute = 0;
    }
  }

  return savedBits;
}

/********************************************************/
/* functions EncUsacInfo() and EncUsacFree()             */
/********************************************************/
#define PROGVER "Core Encoder Module"

/* EncUsacInfo() */
/* Get info about usac encoder core.*/
char *EncUsacInfo (FILE *helpStream, int extendedHelp)
{
  if ( helpStream != NULL ) {
    fprintf(helpStream,
      "\n"PROGVER "\n"
      "encoder parameter string format:\n\n"
      "possible options:\n");
  }

  if (helpStream != NULL && extendedHelp) {
    fprintf(helpStream,
	    "\t-usac_switched   USAC Switched Coding\n"
            "\t-usac_fd  	USAC Frequency Domain Coding\n"
            "\t-usac_td  	USAC Temporal Domain Coding\n"
	    "\t-br <Br1>        bitrate(s) in kbit/s\n"
	    "\t-bw <Bw1>        bandwidths in Hz\n");
    fprintf(helpStream,
            "\t-tns, -tns_lc	for TNS or TNS with low complexity\n"
	    "\t-ms <num>	set msMask to 0 or 2, instead of autodetect (1)\n"
            "\t-wshape  	use window shape WS_DOLBY instead of WS_FHG\n");
    fprintf(helpStream,
	    "\t-aac_nosfacs	all AAC-scalefacs will be equal\n");
    fprintf(helpStream,
	    "\t-ep      	create bitstreams with epConfig 1 syntax\n");
    fprintf(helpStream,
	    "\nobscure/not working options:\n"
	    "\t-length_960  	frame length flag (never tested)\n"
	    "\t-noSbr	use sbr for BSAC object types\n"
	    "\t-wlp <blksize>\n");
    fprintf(helpStream,
	    "\t-length_768  	     frame length of 768 samples\n"
            "\t-hSBR                 Harmonic SBR\n"
            "\t-nf                   Noise Filling\n"
            "\t-usac_tw              TW-MDCT\n"
            "\t-ipd                  IPD coding in MPEG Surround\n");
  }
  else {
    if ( helpStream != NULL ) {
      fprintf(helpStream,
	      "\nuse -xh for extended help!\n");
    }
  }

  if ( helpStream != NULL ) {
    fprintf(helpStream,
	    "\nimportant switches:\n"

          "\t-br <int>                      bitrate\n"
          "\t-sbrRatioIndex <int>           sbrRatioIndex\n"
          "\t-mps_res                       Allow MPEG Surround residual\n"
          "\t-cplx_pred                     Complex prediction\n"
          "\t-mps_qce                       use quad channel element\n"
          "\t-mps_hybridResidualMode <int>  enable hybrid residual coding\n"
          "\t-noiseFillingMode <int>        choose noise filling mode\n"
          "\t-enf                           use enhanced noisefilling\n"
          "\t-splitTransform                use transform splitting\n"
#ifdef SPECIFY_INPUT_CONTENTTYPE
          "\t-contentType <int>             type of input content (default: 0), possible options:\n"
          "\t                               [0 = channels | 1 = objects | 2 = SAOC objects | 3 = HOA]\n"
#endif
          "\t-objFile <string>              filename of file with object data\n"
          "\t-saocFile <string>             filename of file with saoc data\n"
          "\t-saocBitrateFile <string>      filename of file with saoc bitrate information\n"

          "\t-hoaFile <string>              filename of file with hoa data\n"
          "\t-hoaBitrateFile <string>       filename of file with hoa bitrate information\n"
#if IAR
          "\t-iarFile <string>              filename of file with rendering3DType data for immersive audio rendering\n"
#endif
          "\t-pmcFile <string>              filename of file with production metadata configuration dat\n"
          "\t-mctMode <int>                 set Multichannel Coding Tool mode (default: -1), possible options:\n"
          "\t                               [-1 = disable | 0 = prediction | 1 = rotation | 2 = pred+SF | 3 = rot+SF]\n"
          "\t-useLcProfile                  enforce Low Complexity Profile compliant bitstream\n"
          "\t                               [level 3 for < 16 channels, 48kHz; level 5 above ]\n"
          

          "\t-cicpIn                        specify the reference layout\n"

          "\t-hasDynamicObjectPriority <int> hasDynamicObjectPriority (default: 0)\n"
          "\t-lowDelayMetadataCoding <int>  lowDelayMetadataCoding (default: 0, low delay: 1, low delay high resolution: 2)\n"
          

          );
#ifdef USE_FILL_ELEMENT
    fprintf(helpStream,
          "\t-fillElem    	                write a FillElement or not. Note: if no FillElement is written the buffer requirements may be violated (dflt: 1)\n"
	      );
#endif
  }

  return PROGVER;
}


/* EncUscaFree() */
/* Free memory allocated by usac encoder core.*/
void EncUsacFree (HANDLE_ENCODER enc)
{
  if(enc){

    if(enc->data){
      int i_ch;

      for ( i_ch = 0 ; i_ch <MAX_TIME_CHANNELS; i_ch++) {
        if(enc->data->tdenc[i_ch]){
          close_coder_lf(enc->data->tdenc[i_ch]);
        }
      }

      EncUsac_data_free(enc->data);

      EncUsac_tns_free(enc->data);

      for ( i_ch = 0 ; i_ch <MAX_TIME_CHANNELS; i_ch++) {
        if(enc->data->tdenc[i_ch]){
          free(enc->data->tdenc[i_ch]);
          enc->data->tdenc[i_ch] = NULL;
        }
      }

      for ( i_ch = 0 ; i_ch < 64 /*MAX_USAC_ELEMENTS*/; i_ch++) {
        if(enc->data->hSpatialEnc[i_ch]){
          SpatialEncClose(enc->data->hSpatialEnc[i_ch]);
          enc->data->hSpatialEnc[i_ch] = NULL;
        }
      }

      if(enc->data->hTfDec){
        DeleteIntDec(&(enc->data->hTfDec));
      }

      free(enc->data);
      enc->data = NULL;
    }

  }

  if ( gExtSAOCMetadataBitrateFileHandle )
    fclose (gExtSAOCMetadataBitrateFileHandle );

  if ( gExtSAOCMetadataFileHandle )
    fclose(gExtSAOCMetadataFileHandle);


  if ( gExtHOAMetadataBitrateFileHandle )
    fclose (gExtHOAMetadataBitrateFileHandle );

  if ( gExtHOAMetadataFileHandle )
    fclose (gExtHOAMetadataFileHandle );

#if IAR
  if ( gIARBS )
    fclose ( gIARBS);
#endif
  if ( gProdMetadataConfigFileHandle )
    fclose ( gProdMetadataConfigFileHandle);
}


/**
 * Available as part of the HANDLE_ENCODER
 */
static int EncUsac_getNumChannels(HANDLE_ENCODER enc)
{ return enc->data->channels; }

static enum MP4Mode EncUsac_getEncoderMode(HANDLE_ENCODER enc)
{ enc=enc; return MODE_USAC; }

static int EncUsac_getSbrRatioIndex(HANDLE_ENCODER encoderStruct){
  return encoderStruct->data->sbrRatioIndex;
}

static int EncUsac_getBitrate(HANDLE_ENCODER encoderStruct){
  return encoderStruct->data->bitrate;
}

static int EncUsac_getSbrEnable(HANDLE_ENCODER encoderStruct, int *bitrate)
{
  int sbrenable;

  sbrenable = encoderStruct->data->sbrenable;
  *bitrate = encoderStruct->data->bitrate;

  return sbrenable;
}

static int EncUsac_getFlag768(HANDLE_ENCODER enc){
  return enc->data->flag_768;
}

/**
 * Helper functions
 */
static const char* getParam(const char* haystack, const char* needle)
{
  const char* ret = strstr(haystack, needle);
  if (ret) DebugPrintf(2, "EncUsacInit: accepted '%s'",needle);
  return ret;
}


/**
 * Initialise Encoder specific data structures:
 *   initialize window type and shape data
 */
static int EncUsac_window_init(HANDLE_ENCODER_DATA data, int wshape_flag)
{
  int i_ch;

  for (i_ch=0; i_ch<data->channels; i_ch++) {
    data->windowSequence[i_ch] = ONLY_LONG_SEQUENCE;
    if (wshape_flag == 1) {
      data->windowShapePrev[i_ch] = WS_DOLBY;
    } else {
      data->windowShapePrev[i_ch] = WS_FHG;
    }
  }

  return 0;
}


/**
 * Initialise Encoder specific data structures:
 *   allocate and initialize TNS
 */
static int EncUsac_tns_init(HANDLE_ENCODER_DATA data, TNS_COMPLEXITY tns_used)
{
  int i_ch;
  for (i_ch=0; i_ch<data->channels; i_ch++) {
    if (tns_used == NO_TNS) {
      data->tnsInfo[i_ch] = NULL;
    } else {
      data->tnsInfo[i_ch] = (TNS_INFO*)calloc(1, sizeof(TNS_INFO));
      if ( TnsInit(data->sampling_rate, data->tns_select, data->tnsInfo[i_ch]) )
        return -1;
    }
  }
  return 0;
}

static int EncUsac_tns_free(HANDLE_ENCODER_DATA data)
{

  if (data){
    int i_ch;

    for(i_ch = 0; i_ch < MAX_TIME_CHANNELS; i_ch++){

      if(data->tnsInfo[i_ch]){
        free(data->tnsInfo[i_ch]);
        data->tnsInfo[i_ch] = NULL;
      }
    }
  }

  return 0;
}




/**
 * Initialise Encoder specific data structures:
 *   calculate max. audio bandwidth
 */
static int EncUsac_bandwidth_init(HANDLE_ENCODER_DATA data)
{
  int i;

  if (data->bw_limit<=0) {
    /* no bandwidth is set at all, pick one from bitrate */
    /* table: audio_bandwidth( bit_rate ) */
    static const long bandwidth[8][2] =
      { {64000,20000},{56000,16000},{48000,14000},
        {40000,12000},{32000,9000},{24000,6000},
        {12000,6000},{-1,5000}
      };

    i = 0;
    while( bandwidth[i][0] > (data->bitrate/data->channels)) {
      if( bandwidth[i][0] < 0 ) {
	i--;
	break;
      }
      i++;
    }
    data->bw_limit = bandwidth[i][1];
  }

  return 0;
}


/**
 * Initialise Encoder specific data structures:
 *   some global initializations and memory allocations
 */
static int EncUsac_data_init(HANDLE_ENCODER_DATA data)
{
  int i_ch;
  int frameLength = data->block_size_samples;

  for (i_ch=0; i_ch<data->channels; i_ch++) {
    data->DTimeSigBuf[i_ch] =
      (double*)calloc(2*MAX_OSF*(frameLength), sizeof(double));
    {
      int jj;
      for (jj=0; jj<2*MAX_OSF*(frameLength); jj++)
        data->DTimeSigBuf[i_ch][jj] = 0.0;
    }
    data->DTimeSigLookAheadBuf[i_ch] =
      (double*)calloc(MAX_OSF*frameLength, sizeof(double));


    data->spectral_line_vector[i_ch] =
      (double*)calloc(MAX_OSF*(frameLength+128), sizeof(double));

    data->reconstructed_spectrum[i_ch] =
        (double*)calloc(MAX_OSF*(frameLength+128), sizeof(double));

    data->twoFrame_DTimeSigBuf[i_ch] =
      (double*)calloc(MAX_OSF*3*frameLength, sizeof(double));
    {
      int ii;
      for(ii=0; ii<MAX_OSF*3*frameLength; ii++)
        data->twoFrame_DTimeSigBuf[i_ch][ii] = 0.0;
    }

    /* initialize t/f mapping */
    data->overlap_buffer[i_ch] =
      (double*)calloc(frameLength*4, sizeof(double));
  {
       int ii;
       for(ii=0; ii<4*frameLength; ii++)
         data->overlap_buffer[i_ch][ii] = 0.0;
     }
    /*initialize TD buffer */
    {
      int jj;
      for (jj=0; jj<L_FRAME_1024+L_NEXT_HIGH_RATE_1024; jj++)
        data->tdBuffer[i_ch][jj] = 0.f;

    }

    /* initialize TW buffers */
    if ( data->flag_twMdct == 1 ) {
      int ii;
      data->warp_cont_mem[i_ch] = (float*) calloc(2*frameLength,sizeof(float));
      for ( ii = 0 ; ii < 2*frameLength ; ii++ ) {
        data->warp_cont_mem[i_ch][ii] = 1.0f;
      }
      data->warp_sum[i_ch][0] = data->warp_sum[i_ch][1] = (float) frameLength;
    }

  }
  return 0;
}


static int EncUsac_data_free(HANDLE_ENCODER_DATA data)
{
  int i_ch;

  if(data){
    for (i_ch=0; i_ch<MAX_TIME_CHANNELS; i_ch++) {
      if(data->DTimeSigBuf[i_ch]){
        free(data->DTimeSigBuf[i_ch]);
        data->DTimeSigBuf[i_ch] = NULL;
      }

      if(data->DTimeSigLookAheadBuf[i_ch]){
        free(data->DTimeSigLookAheadBuf[i_ch]);
        data->DTimeSigLookAheadBuf[i_ch] = NULL;
      }

      if(data->spectral_line_vector[i_ch]){
        free(data->spectral_line_vector[i_ch]);
        data->spectral_line_vector[i_ch] = NULL;
      }

      if(data->reconstructed_spectrum[i_ch]){
        free(data->reconstructed_spectrum[i_ch]);
        data->reconstructed_spectrum[i_ch] = NULL;
      }

      if(data->twoFrame_DTimeSigBuf[i_ch]){
        free(data->twoFrame_DTimeSigBuf[i_ch]);
        data->twoFrame_DTimeSigBuf[i_ch] = NULL;
      }

      if(data->overlap_buffer[i_ch]){
        free(data->overlap_buffer[i_ch]);
        data->overlap_buffer[i_ch] = NULL;
      }


      if(data->warp_cont_mem[i_ch]){
        free(data->warp_cont_mem[i_ch]);
        data->warp_cont_mem[i_ch] = NULL;
      }

    }
  }

  return 0;
}



static int getCoreSbrFrameLengthIndex(int outFrameLength, int sbrRatioIndex){

  int index = -1;

  switch(outFrameLength){
  case 768:
    if(sbrRatioIndex == 0){
      index = 0;
    }
    break;
  case 1024:
    if(sbrRatioIndex == 0){
      index = 1;
    }
    break;
  case 2048:
    if(sbrRatioIndex == 2){
      index = 2;
    } else if (sbrRatioIndex == 3){
      index = 3;
    }
    break;
  case 4096:
    if(sbrRatioIndex == 1){
      index = 4;
    }
    break;
  default:
    break;
  }

  assert(index > -1);

  return index;
}


static void __fillUsacMps212Config(USAC_MPS212_CONFIG *pUsacMps212Config, SAC_ENC_USAC_MPS212_CONFIG *pSacEncUsacMps212Config){

  pUsacMps212Config->bsFreqRes.value              = pSacEncUsacMps212Config->bsFreqRes;
  pUsacMps212Config->bsFixedGainDMX.value         = pSacEncUsacMps212Config->bsFixedGainDMX;
  pUsacMps212Config->bsTempShapeConfig.value      = pSacEncUsacMps212Config->bsTempShapeConfig;
  pUsacMps212Config->bsDecorrConfig.value         = pSacEncUsacMps212Config->bsDecorrConfig;
  pUsacMps212Config->bsHighRateMode.value         = pSacEncUsacMps212Config->bsHighRateMode;
  pUsacMps212Config->bsPhaseCoding.value          = pSacEncUsacMps212Config->bsPhaseCoding;
  pUsacMps212Config->bsOttBandsPhasePresent.value = pSacEncUsacMps212Config->bsOttBandsPhasePresent;
  pUsacMps212Config->bsOttBandsPhase.value        = pSacEncUsacMps212Config->bsOttBandsPhase;
  pUsacMps212Config->bsResidualBands.value        = pSacEncUsacMps212Config->bsResidualBands;
  pUsacMps212Config->bsPseudoLr.value             = pSacEncUsacMps212Config->bsPseudoLr;
  pUsacMps212Config->bsEnvQuantMode.value         = pSacEncUsacMps212Config->bsEnvQuantMode;

}

static void readObjectData(HANDLE_ENCODER_DATA        data,
                           unsigned char             *extensionData,
                           int                       *extensionDataSize,
                           int                       *extensionDataPresent
) {
  static int objFileCnt = 1;

  if (data->objFile) {
    FILE *extObjMetadataFileHandle;
    char *pFilename;
    char nextExtObjMetadataFilename[FILENAME_MAX];
    char suffix[5] = { '\0' };

    strncpy(nextExtObjMetadataFilename, data->objFile, FILENAME_MAX);

    pFilename = &(nextExtObjMetadataFilename[strlen(nextExtObjMetadataFilename)]);

    while (pFilename[0] != '.') {
      pFilename--;
    }

    strncpy(suffix, pFilename, 4);

    while (pFilename[0] != '_') {
      pFilename--;

    }

    sprintf(pFilename, "_%d%s", objFileCnt, suffix);

    /* Open next object file */
    extObjMetadataFileHandle = fopen(nextExtObjMetadataFilename,"rb");

    if (extObjMetadataFileHandle) {
      /* Get file length */

      fseek (extObjMetadataFileHandle, 0, SEEK_END);
      *extensionDataSize = ftell (extObjMetadataFileHandle);
      rewind (extObjMetadataFileHandle);

      /* Read data into buffer */
      if ((*extensionDataSize) > 0) {
        fread(extensionData, sizeof(unsigned char), *extensionDataSize, extObjMetadataFileHandle);
        *extensionDataPresent = 1;
      }

        objFileCnt++;  /* Use next obj file */

      fclose(extObjMetadataFileHandle);
    } else {
      *extensionDataSize    = 0;
      *extensionDataPresent = 0;
    }
  }
}

static void readPMFData(HANDLE_ENCODER_DATA        data,
                        unsigned char             *extensionData,
                        int                       *extensionDataSize,
                        int                       *extensionDataPresent
) {
  static int PMFFileCnt = 1;

  if (data->pmcFile[0] != '\0') {
    FILE *extProductionMetadataFrameFileHandle = NULL;
    int suffixlen = 0;
    char *pFilename = NULL;
    char nextExtProductionMetadataFrameFilename[FILENAME_MAX] = {'\0'};
    char suffix[5] = {'\0'};

    strncpy(nextExtProductionMetadataFrameFilename, data->pmcFile, FILENAME_MAX);

    pFilename = &(nextExtProductionMetadataFrameFilename[strlen(nextExtProductionMetadataFrameFilename)]);

    while (pFilename[0] != '.') {
      pFilename--;
    }

    sprintf(pFilename, "_%d.dat", PMFFileCnt);

    /* Open next object file */
    extProductionMetadataFrameFileHandle = fopen(nextExtProductionMetadataFrameFilename,"rb");

    if (extProductionMetadataFrameFileHandle) {
      /* Get file length */
      fseek (extProductionMetadataFrameFileHandle, 0, SEEK_END);
      *extensionDataSize = ftell (extProductionMetadataFrameFileHandle);
      rewind (extProductionMetadataFrameFileHandle);

      /* Read data into buffer */
      if ((*extensionDataSize) > 0) {
        fread(extensionData, sizeof(unsigned char), *extensionDataSize, extProductionMetadataFrameFileHandle);
        *extensionDataPresent = 1;
      }

      /* Use next obj file */
      PMFFileCnt++;

      fclose(extProductionMetadataFrameFileHandle);
    } else {
      *extensionDataSize    = 0;
      *extensionDataPresent = 0;
    }
  } else {
    *extensionDataSize    = 0;
    *extensionDataPresent = 0;
  }
}

static void readSaocData(HANDLE_ENCODER_DATA        data,
                         unsigned char             *extensionData,
                         int                       *extensionDataSize,
                         int                       *extensionDataPresent
) {
  int bitrate        = 0;
  char c_bitrate[14] = { '\0' };

  /* Get SAOC bitrate from file */
  fgets(c_bitrate, sizeof(c_bitrate), gExtSAOCMetadataBitrateFileHandle );
  bitrate = atoi(c_bitrate);

  *extensionDataSize = bitrate >> 3;

  /* Read data into buffer */
  if ((*extensionDataSize) > 0) {
    fread(extensionData, sizeof(unsigned char), *extensionDataSize, gExtSAOCMetadataFileHandle);
    *extensionDataPresent = 1;
  } else {
    *extensionDataSize    = 0;
    *extensionDataPresent = 0;
  }
}


static void readHoaData(HANDLE_ENCODER_DATA         data,
                        unsigned char              *extensionData,
                        int                        *extensionDataSize,
                        int                        *extensionDataPresent
) {
  int bitrate        = 0;
  char c_bitrate[14] = { '\0' };

  /* Get HOA bitrate from file */
  fgets(c_bitrate, sizeof(c_bitrate), gExtHOAMetadataBitrateFileHandle );
  bitrate = atoi(c_bitrate);

  *extensionDataSize = bitrate >> 3;

  /* Read data into buffer */
  if ((*extensionDataSize) > 0) {
    fread(extensionData, sizeof(unsigned char), *extensionDataSize, gExtHOAMetadataFileHandle);
    *extensionDataPresent = 1;
  } else {
    *extensionDataSize    = 0;
    *extensionDataPresent = 0;
  }
}

#if IAR
static void readIARData(HANDLE_ENCODER_DATA         data,
                        unsigned char              *extensionData,
                        int                        *extensionDataSize,
                        int                        *extensionDataPresent
) {
  if (gIARBS == NULL) {
    /* if there is no bitstream, it is assumed "rendering3DType" to be 1 */
    *extensionData = 1;
  } else {
    /* if it is unable to read the bitstream, it is also assumed "rendering3DType" to be 1 */
    if (!fread ( extensionData, 1, 1, gIARBS)) {
      *extensionData = 1;
    }
  }

  *extensionDataSize    = 1;
  *extensionDataPresent = 1;
}
#endif

static int checkBaselineProfileConfiguration(struct tagEncoderSpecificData const * const encData,      /* in: encoder specific data structure */
                                             DEC_CONF_DESCRIPTOR           const * const dec_conf,     /* in: space to write decoder configurations */
                                             int                                   const numChannel,   /* in: num audio channels */
                                             int                                   const profileLevel  /* in: mpegh3daProfileLevelIndication or CompatibleSetIndication to check against Baseline Profile restrictions */
){
  int err = 0;
  int i;

  /* check for valid configuration */
  if (numChannel > 28) {
    CommonWarning("Too many channels for Baseline profile! (>28)");
    return -1;
  }
  else if (numChannel > 16 && profileLevel != MPEG3DA_PLI_BASELINE_L4 && profileLevel != MPEG3DA_PLI_BASELINE_L5) {
    CommonWarning("Too many channels for Baseline Profile Level 3 (>16)");
    return -1;
  }
  if (encData->sampling_rate > 48000 && profileLevel != MPEG3DA_PLI_BASELINE_L5) {
    CommonWarning("Sampling rates >48kHz are only allowed with Baseline Profile Level 5");
    return -1;
  }
  for (i = 0; i < NUMBER_OF_ALLOWED_SAMPLING_RATES; i++) {
    if (encData->sampling_rate == allowedSamplingRates[i]) {
      break;
    }
  }
  if (encData->sampling_rate != allowedSamplingRates[i]) {
    CommonWarning("unsupported sampling rate!");
    return -1;
  }
  if ((encData->bitrate < 32000) || (encData->bitrate > 1536000)) {
    CommonWarning("unsopported bitrate for Baseline profile!");
    return -1;
  }
  if (encData->flag_twMdct == 1) {
    CommonWarning("time warped MDCT not supported in Baseline profile");
    return -1;
  }
  if (encData->flag_768 == 1) {
    CommonWarning("framelength 768 not supported in Baseline profile");
    return -1;
  }
  if ((encData->contentType == CONTENT_3D_SAOC_OBJECTS) || (encData->saocFile[0] != '\0') || (encData->saocBitrateFile[0] != '\0')) {
    CommonWarning("SAOC not supported in Baseline profile");
    return -1;
  }
  if ((encData->contentType == CONTENT_3D_HOA) || (encData->hoaFile[0] != '\0') || (encData->hoaBitrateFile[0] != '\0')) {
    CommonWarning("HOA not supported in Baseline profile");
    return -1;
  }
  if ((encData->objFile[0] != '\0') && (encData->lowDelayMetadataCoding == 0)) {
    CommonWarning("lowDelayMetadataCoding must be enabled for Baseline profile");
    return -1;
  }
  if (encData->receiverDelayCompensation == 0) {
    CommonWarning("receiverDelayCompensation must be enabled in Baseline profile");
    return -1;
  }


  /* ISO/IEC 23008-3, 4.8.2.6 Restrictions for the Baseline Profile and Levels */

  /* fullband LPD | LPD stereo | Time warped filterbank | Quad channel element (QCE) */
  if (0 == err) {
    unsigned int elemIdx;
    unsigned int numElements = dec_conf[0].audioSpecificConfig.specConf.usacConfig.usacDecoderConfig.numElements;

    for (elemIdx = 0; elemIdx < numElements; elemIdx++) {
      USAC_CONFIG * pUsacConfig = &(dec_conf[0].audioSpecificConfig.specConf.usacConfig);
      USAC_ELEMENT_TYPE usacElementType = UsacConfig_GetUsacElementType(pUsacConfig, elemIdx);
      USAC_CORE_CONFIG * pUsacCoreConfig = UsacConfig_GetUsacCoreConfig(pUsacConfig, elemIdx);
      
      int fullbandLpd = 0;
      int lpdStereoIndex = 0;
      int tw_mdct = 0;
      int qceIndex = 0;

      if (!pUsacCoreConfig ||
          (usacElementType != USAC_ELEMENT_TYPE_SCE && usacElementType != USAC_ELEMENT_TYPE_CPE && usacElementType != USAC_ELEMENT_TYPE_LFE)){
        continue;
      }

      tw_mdct = pUsacCoreConfig->tw_mdct.value;
      fullbandLpd = pUsacCoreConfig->fullbandLPD.value;

      if (usacElementType == USAC_ELEMENT_TYPE_CPE) {
        qceIndex = dec_conf[0].audioSpecificConfig.specConf.usacConfig.usacDecoderConfig.usacElementConfig[elemIdx].usacCpeConfig.qceFlag;
        lpdStereoIndex = dec_conf[0].audioSpecificConfig.specConf.usacConfig.usacDecoderConfig.usacElementConfig[elemIdx].usacCpeConfig.lpdStereoIndex.value;
      }

      if (qceIndex != 0) {
        CommonWarning("Quad channel element (QCE) not supported in Baseline Profile.");
        err = -1;
        break;
      }
      if (tw_mdct != 0) {
        CommonWarning("time warped MDCT not supported in Baseline profile.");
        err = -1;
        break;
      }
      if (fullbandLpd != 0) {
        CommonWarning("Fullband LPD not supported in Baseline profile.");
        err = -1;
        break;
      }
      if (lpdStereoIndex != 0) {
        CommonWarning("LPD stereo not supported in Baseline profile.");
        err = -1;
        break;
      }
    }
  }

  /* SignalGroupType */
  if (0 == err) {
    unsigned int numHOATransportChannels = dec_conf[0].audioSpecificConfig.specConf.usacConfig.frameworkConfig3d.signals3D.numHOATransportChannels;
    unsigned int numSAOCTransportChannels = dec_conf[0].audioSpecificConfig.specConf.usacConfig.frameworkConfig3d.signals3D.numSAOCTransportChannels;

    if (numHOATransportChannels != 0) {
      CommonWarning("HOA not allowed in Baseline Profile, i.e. SignalGroupType[grp] shall have the value \"SignalGroupTypeChannels\" or \"SignalGroupTypeObject\".");
      err = -1;
    } else if (numSAOCTransportChannels != 0){
      CommonWarning("SAOC not allowed in Baseline Profile, i.e. SignalGroupType[grp] shall have the value \"SignalGroupTypeChannels\" or \"SignalGroupTypeObject\".");
      err = -1;
    }
  }

  return err;
}

static int selectBaselineProfileLevel(struct tagEncoderSpecificData const * const encData,      /* in: encoder specific data structure */
                                      DEC_CONF_DESCRIPTOR                 * const dec_conf,     /* inout: space to write decoder configurations */
                                      int                                   const numChannel    /* in: num audio channels */
){
  /* select level based on channel number (assuming all encoded channels will be decoded, i.e. no switchgroups etc.) */
  if (numChannel > 28) {
    CommonWarning("Too many channels for Baseline profile! (>28)");
    return -1;
  }
  else if (numChannel > 16) {
    /* use Level 5 for higher channel numbers */
    CommonWarning("more than 16 channels => using Baseline Profile Level 5");
    dec_conf->MPEGHAudioProfileLevel.value = MPEG3DA_PLI_BASELINE_L5;
  }
  else {
    if (encData->sampling_rate > 48000) {
      /* use Level 5 for sampling rates > 48kHz */
      CommonWarning("sampling rate >48kHz => using Baseline Profile Level 5");
      dec_conf->MPEGHAudioProfileLevel.value = MPEG3DA_PLI_BASELINE_L5;
    }
    else {
      /* default: use Level 3 */
      dec_conf->MPEGHAudioProfileLevel.value = MPEG3DA_PLI_BASELINE_L3;
    }
  }

  if (dec_conf->MPEGHAudioProfileLevel.value == MPEG3DA_PLI_BASELINE_L5) {
    if (encData->sampling_rate == 48000 || encData->sampling_rate == 44100) {
      CommonWarning("two resampling ratios supported for Lvl 5 at this sampling rate. Value 1 assumed");
    }
    else if (encData->sampling_rate == 32000 || encData->sampling_rate == 29400) {
      CommonWarning("two resampling ratios supported for Lvl 5 at this sampling rate. Value 1.5 assumed");
    }
  }

  return 0;
}

static void disableUnsupportedToolsForBaseline(struct tagEncoderSpecificData * const encData,      /* inout: encoder specific data structure */
                                               int                             const numChannel    /* in: num audio channels */
){
  /* disable unsupported tools */
  if ((encData->sbrRatioIndex > 0) || (encData->sbrenable != 0)) {
    CommonWarning("Changing configuration to comply with Baseline profile: SBR disabled!");
    encData->sbrenable = 0;
    encData->sbrRatioIndex = 0;
  }
  if (encData->usac212enable) {
    CommonWarning("Changing configuration to comply with Baseline profile: MPS212 disabled!");
    encData->channels = numChannel;
    encData->usac212enable = 0;
    encData->flag_mpsres = 0;
  }
}

static int setupBaselineProfileEncoding(struct tagEncoderSpecificData * const encData,      /* inout: encoder specific data structure */
                                        DEC_CONF_DESCRIPTOR           * const dec_conf,     /* inout: space to write decoder configurations */
                                        int                             const numChannel    /* in: num audio channels */
){
  int err = 0;
  
  if (0 == err && encData->useBlProfile) {
    err = selectBaselineProfileLevel(encData, dec_conf, numChannel);
  }

  /* disable unsupported tools */
  if (0 == err && encData->useBlProfile) {
    disableUnsupportedToolsForBaseline(encData, numChannel);
  }

  return err;
}

/*****************************************************************************************
 ***
 *** Function: EncUsacInit
 ***
 *** Purpose:  Initialize USAC-part and the macro blocks of the USAC part of the VM
 ***
 *** Description:
 ***
 ***
 *** Parameters:
 ***
 ***
 *** Return Value:
 ***
 *** **** MPEG-4 VM ****
 ***
 ****************************************************************************************/

int EncUsacInit (
                int                 numChannel,             /* in: num audio channels */
                float               sampling_rate_f,        /* in: sampling frequancy [Hz] */
                HANDLE_ENCPARA      encPara,                /* in: encoder parameter struct */
                int                 *frameNumSample,        /* out: num samples per frame */
                int                 *frameMaxNumBit,        /* out: maximum num bits per frame */ /* SAMSUNG_2005-09-30 : added */
                DEC_CONF_DESCRIPTOR *dec_conf,              /* in: space to write decoder configurations */
                int                 *numTrack,              /* out: number of tracks */
                HANDLE_BSBITBUFFER  *asc,                   /* buffers to hold output Audio Specific Config */
                HANDLE_ENCODER      core,                   /* in:  core encoder or NULL */
                HANDLE_ENCODER      encoderStruct           /* in: space to put encoder data */
) {
  int err = 0;
  int wshape_flag = 0;
  int   i, i_dec_conf;
  int i_ch;
  int outputFrameLength;
  unsigned int elemIdx = 0;
  int samplingRateCore = (int)(sampling_rate_f+.5);
  unsigned int bs_pvc;

  HANDLE_ENCODER_DATA data = (HANDLE_ENCODER_DATA)calloc(1, sizeof(ENCODER_DATA));
  if (data == NULL) {
    CommonWarning("EncUsacInit: Allocation Error");
    return -1;
  }


  encoderStruct->data = data;
  encoderStruct->getNumChannels   = EncUsac_getNumChannels;
  encoderStruct->getEncoderMode   = EncUsac_getEncoderMode;
  encoderStruct->getSbrEnable     = EncUsac_getSbrEnable;
  encoderStruct->getFlag768       = EncUsac_getFlag768;
  encoderStruct->getSbrRatioIndex = EncUsac_getSbrRatioIndex;
  encoderStruct->getBitrate       = EncUsac_getBitrate;

  data->debugLevel=0;
  data->sampling_rate = (int)(sampling_rate_f+.5);
  data->sampling_rate_out = data->sampling_rate;
  data->tns_select = NO_TNS;
  data->ms_select = 1;
  data->flag_960 = 0;
  data->flag_768 = 0;
  data->ep_config = -1;
  data->track_num = -1;
  data->layer_num = 1;
  data->max_sfb = 0;
  data->usacIndependencyFlagCnt = 0;
  data->usacIndependencyFlagInterval = 25; /* send usacIndependencyFlag every 25th frame */
  data->enhancedNoiseFilling = 0;
  data->receiverDelayCompensation = 1;
  data->coreQmfDelayCompensation  = 0;

  {
    int i,j,k;
    for (j=0; j<MAX_TIME_CHANNELS; j++) {
    data->msInfo[j].ms_mask = 0;
    for (i=0; i<MAX_SHORT_WINDOWS; i++) {
      for (k=0; k<SFB_NUM_MAX; k++) {
        data->msInfo[j].ms_used[i][k] = 0;
        }
      }
    }
  }

  /* Core coding selection*/
  data->frameCounter       = 0;
  data->switchEveryNFrames = 50;

  /* initialize AudioPreRoll() */
  EncUsacAudioPreRoll_init(&data->aprData,
                           data->extType,
                           data->extElementConfigLength,
                           &data->numExtElements);

  /* ---- transfer encoder options ---- */
  if( ! encPara ){
    CommonWarning("EncUsacInit: invalid encoder parameter struct");
    return -1;
  } else {
    data->debugLevel                 = encPara->debugLevel;
    data->bitrate                    = encPara->bitrate;
    data->bw_limit                   = encPara->bw_limit;
#ifdef USE_FILL_ELEMENT
    data->bUseFillElement            = encPara->bUseFillElement;
#endif
    data->codecMode                  = encPara->codecMode;
    data->sbrenable                  = encPara->sbrenable;
    data->sbrRatioIndex              = encPara->sbrRatioIndex;
    data->flag_960                   = encPara->flag_960;
    data->flag_768                   = encPara->flag_768;
    data->flag_twMdct                = encPara->flag_twMdct;
    data->tns_select                 = encPara->tns_select;
    data->flag_harmonicBE            = encPara->flag_harmonicBE;
    data->flag_noiseFilling          = encPara->flag_noiseFilling;
    data->ms_select                  = encPara->ms_select;
    data->ep_config                  = encPara->ep_config;
    data->aacAllowScalefacs          = encPara->aacAllowScalefacs;
    data->flag_ipd                   = encPara->flag_ipd;
    data->flag_mpsres                = encPara->flag_mpsres;
    data->hybridResidualMode         = encPara->hybridResidualMode;
    data->flag_mpsqce                = encPara->flag_mpsqce;
    data->noiseFillingMode           = encPara->noiseFillingMode;
    data->enhancedNoiseFilling       = encPara->enhancedNoiseFilling;
    data->flag_cplxPred              = encPara->flag_cplxPred;
    data->mctMode                    = encPara->mctMode;
    data->useLcProfile               = encPara->useLcProfile;
    data->useBlProfile               = encPara->useBlProfile;
    data->hasBlProfileCompatibility  = encPara->useBlProfile;
    data->numCompatibleSets          = (encPara->compatibleProfileLevel >= SUPPORTED_COMPATIBLEPROFILELEVEL_MIN) ? 1 : 0;   /* the core encoder is currently restricted to allow only 1 CompatibleProfileLevelSet */
    data->CompatibleSetIndication[0] = encPara->compatibleProfileLevel;
    data->hasDynamicObjectPriority   = encPara->hasDynamicObjectPriority;
    data->lowDelayMetadataCoding     = encPara->lowDelayMetadataCoding;
    data->OAMFrameLength             = encPara->OAMFrameLength;
    data->cicpIn                     = encPara->cicpIn;
    data->contentType                = encPara->contentType;
    wshape_flag                      = encPara->wshape_flag;
    g_useSplitTransform              = encPara->g_useSplitTransform;
    memcpy( data->prev_coreMode,    encPara->prev_coreMode,    sizeof(encPara->prev_coreMode) );
    memcpy( data->coreMode,         encPara->coreMode,         sizeof(encPara->coreMode) );
    memcpy( data->tsdInputFileName, encPara->tsdInputFileName, sizeof(encPara->tsdInputFileName) );
    memcpy( data->objFile,          encPara->objFile,          sizeof(encPara->objFile) );
    memcpy( data->saocFile,         encPara->saocFile,         sizeof(encPara->saocFile) );
    memcpy( data->saocBitrateFile,  encPara->saocBitrateFile,  sizeof(encPara->saocBitrateFile) );
    memcpy( data->hoaFile,          encPara->hoaFile,          sizeof(encPara->hoaFile) );
    memcpy( data->hoaBitrateFile,   encPara->hoaBitrateFile,   sizeof(encPara->hoaBitrateFile) );
#if IAR
    memcpy( data->iarFile,          encPara->iarFile,          sizeof(encPara->iarFile) );
#endif
    memcpy( data->pmcFile,          encPara->pmcFile,          sizeof(encPara->pmcFile) );
    memcpy( data->pmfFile,          encPara->pmfFile,          sizeof(encPara->pmfFile) );
  }


#ifdef SPECIFY_INPUT_CONTENTTYPE
  if ( (data->contentType == CONTENT_3D_OBJECTS) ||
       (data->contentType == CONTENT_3D_SAOC_OBJECTS)) {
#endif
    /* Setup extension data handling */
    if ( data->objFile[0] != '\0' )
    {
      data->extType[data->numExtElements] = USAC_ID_EXT_ELE_OBJ;

      if ( data->OAMFrameLength < 0 ) {
        /* Default object config */
        data->extElementConfigLength[data->numExtElements] = 1;

        /* lowDelayMetadataCoding; */
        data->extElementConfigPayload[data->numExtElements][0] = data->lowDelayMetadataCoding << 7;

        /* hasCoreFramelength */
        data->extElementConfigPayload[data->numExtElements][0] += 1 << 6;

        /* hasScreenRelativeObjects << 5; */

        data->extElementConfigPayload[data->numExtElements][0] += (data->hasDynamicObjectPriority << 4);

        /* data->hasUniformSpread is fixed */
        data->hasUniformSpread = 1;
        data->extElementConfigPayload[data->numExtElements][0] += (data->hasUniformSpread << 3);
      }
      else {
        /* object metadata config for high resolution OAM */
        data->extElementConfigLength[data->numExtElements] = 2;

        /* lowDelayMetadataCoding; */
        data->extElementConfigPayload[data->numExtElements][0] = data->lowDelayMetadataCoding << 7;

        /* hasCoreFramelength */
        data->extElementConfigPayload[data->numExtElements][0] += 0 << 6;
        data->extElementConfigPayload[data->numExtElements][0] += (data->OAMFrameLength >> 6) - 1;

        /* hasScreenRelativeObjects << 7; */

        data->extElementConfigPayload[data->numExtElements][1] += (data->hasDynamicObjectPriority << 6);

        /* data->hasUniformSpread is fixed */
        data->hasUniformSpread = 1;
        data->extElementConfigPayload[data->numExtElements][1] += (data->hasUniformSpread << 5);
      }

      data->numExtElements++;
    }

#ifdef SPECIFY_INPUT_CONTENTTYPE
  }

  if ( (data->contentType == CONTENT_3D_SAOC_OBJECTS) ) {
#endif

    if ( data->saocFile[0] != '\0' && data->saocBitrateFile[0] != '\0' )
    {
      gExtSAOCMetadataBitrateFileHandle = fopen(data->saocBitrateFile,"rb");
      gExtSAOCMetadataFileHandle        = fopen(data->saocFile, "rb");

      if (gExtSAOCMetadataBitrateFileHandle == NULL) {
        CommonWarning("Could not open SAOC Metadata Bitrate File!");
        return -1;
      }
      if (gExtSAOCMetadataFileHandle == NULL) {
        CommonWarning("Could not open SAOC Metadata File!");
        return -1;
      }

      data->extType[data->numExtElements] = USAC_ID_EXT_ELE_SAOC_3D;

      /* Get SAOC Config */
      {
        int configLength = 0;
        int bitrate = 0;
        unsigned char config[6144/8];
        char c_bitrate[14] = { '\0' };
        fgets(c_bitrate, sizeof(c_bitrate), gExtSAOCMetadataBitrateFileHandle );
        bitrate = atoi(c_bitrate);

        configLength = bitrate >> 3;

        if ( configLength > 768 )
        {
          CommonWarning("EncUsacInit: SAOC Config Length too large!");
          return -1;
        }

        /* Read config into buffer */
        if ( configLength > 0 )
        {
          fread(config, sizeof(unsigned char), configLength, gExtSAOCMetadataFileHandle);

          data->extElementConfigLength[data->numExtElements] = configLength;
          memcpy(data->extElementConfigPayload[data->numExtElements], config, data->extElementConfigLength[data->numExtElements]);
        }
        else
        {
          CommonWarning("EncUsacInit: SAOC Config Length not found!");
          return -1;
        }
      }

      data->numExtElements++;
    }  /*  if ( data->saocFile[0] != '\0' && data->saocBitrateFile[0] != '\0' ) */
#ifdef SPECIFY_INPUT_CONTENTTYPE
    else {
      CommonWarning("EncUsacInit: input content is SAOC, but no SAOC data provided. Please use -saocFile and -saocBitrateFile");
      return -1;
    }
  }
#endif

#ifdef SPECIFY_INPUT_CONTENTTYPE
  if (data->contentType == CONTENT_3D_HOA) {
#endif
    if ( data->hoaFile[0] != '\0' && data->hoaBitrateFile[0] != '\0' )
    {
      gExtHOAMetadataBitrateFileHandle = fopen(data->hoaBitrateFile,"rb");
      gExtHOAMetadataFileHandle        = fopen(data->hoaFile, "rb");

      if (gExtHOAMetadataBitrateFileHandle == NULL) {
        CommonWarning("Could not open HOA Metadata Bitrate File!");
        return -1;
      }
      if (gExtHOAMetadataFileHandle == NULL) {
        CommonWarning("Could not open HOA Metadata File!");
        return -1;
      }

      data->extType[data->numExtElements] = USAC_ID_EXT_ELE_HOA;

      /* Get HOA Config */
      {
        int configLength = 0;
        int bitrate = 0;
        unsigned char config[6144/8];
        char c_bitrate[14] = { '\0' };
        fgets(c_bitrate, sizeof(c_bitrate), gExtHOAMetadataBitrateFileHandle );
        bitrate = atoi(c_bitrate);

        configLength = bitrate >> 3;

        if ( configLength > 768 )
        {
          CommonWarning("EncUsacInit: HOA Config Length too large!");
          return -1;
        }

        /* Read config into buffer */
        if ( configLength > 0 )
        {
          fread(config, sizeof(unsigned char), configLength, gExtHOAMetadataFileHandle);

          data->extElementConfigLength[data->numExtElements] = configLength;
          memcpy(data->extElementConfigPayload[data->numExtElements], config, data->extElementConfigLength[data->numExtElements]);
        }
        else
        {
          CommonWarning("EncUsacInit: HOA Config Length not found!");
          return -1;
        }
      }

      data->numExtElements++;
    }

#ifdef SPECIFY_INPUT_CONTENTTYPE
    else {
     CommonWarning("EncUsacInit: input content is HOA, but no HOA data provided. Please use -hoaFile and -hoaBitrateFile");
     return -1;
    }
  }
#endif


  if (data->contentType == CONTENT_3D_CHANNELS) {
    if (numChannel == 2 && data->bitrate < 64000){
      data->channels = 1;
      data->usac212enable = 1;
      data->flag_mpsres = 0;
    }else if(numChannel == 2 && data->flag_mpsres && data->bitrate == 64000){
      data->channels = 2;
      data->usac212enable = 1;
    }else if (numChannel > 2 && data->bitrate <= 512000) {
      data->channels = numChannel;
      data->usac212enable = 1;
    }else{
      data->channels = numChannel;
      data->usac212enable = 0;
      data->flag_mpsres = 0;
    }

    if (data->flag_cplxPred) {
      data->ms_select = 3;
    }
  }
  else {
    data->channels      = numChannel;
    data->usac212enable = 0;
    data->flag_mpsres   = 0;
    data->ms_select     = 0;
  }

  if (data->mctMode > -1) {
    int configLength = 0;
    unsigned char config[6144/8];
    int mctChanMask[MAX_CHANNELS];

    data->usac212enable = 0;
      
    data->extType[data->numExtElements] = USAC_ID_EXT_ELE_MC;
    memset(config, '\0', sizeof(char)*6144/8);
    memset(mctChanMask, 0, sizeof(int)*MAX_CHANNELS);

    /* set channel mask active for all channels */
    memset(mctChanMask, 1, sizeof(int)*numChannel);
    data->mctData.numChannels = numChannel;
    data->mctData.mctSignalingType = data->mctMode; 

    /* write bitwise channel mask to config */
    for (i = 0; i < numChannel; i++) {
      config[i / 8] |= 1 << (7 - i % 8);
    }
    configLength = (i % 8) ? i / 8 + 1 : i / 8;

    /* Read config into buffer */
    if ( configLength > 0 )
    {
      data->extElementConfigLength[data->numExtElements] = configLength;
      memcpy(data->extElementConfigPayload[data->numExtElements], config, data->extElementConfigLength[data->numExtElements]);
    }
    else
    {
      CommonWarning("EncUsacInit: Invalid MCT Config!");
      return -1;
    }
    data->numExtElements++;
  }

  /* baseline profile */
  if (data->useBlProfile == 1) {
    if (data->useLcProfile == 1) {
      CommonWarning("Low Complexity profile and Baseline profile must not be active the same time!");
      return -1;
    }
  }

  /* low complexity profile */
  if (data->useLcProfile == 1) {
    /* select level based on channel number (assuming all encoded channels will be decoded, i.e. no switchgroups etc.) */
    if (numChannel > 28 ){
      CommonWarning("Too many channels for LC profile! (>28)");
      return -1;
    }
    else if (numChannel > 16) {
      /* use Level 5 for higher channel numbers */
      CommonWarning("more than 16 channels => using LC Profile Level 5");
      dec_conf->MPEGHAudioProfileLevel.value = MPEG3DA_PLI_LC_L5;
    }
    else {
      if (data->sampling_rate > 48000) {
        /* use Level 5 for sampling rates > 48kHz */
        CommonWarning("sampling rate >48kHz => using LC Profile Level 5");
        dec_conf->MPEGHAudioProfileLevel.value = MPEG3DA_PLI_LC_L5;
      }
      else {
        /* default: use Level 3 */
        dec_conf->MPEGHAudioProfileLevel.value = MPEG3DA_PLI_LC_L3;
      }
    }

    if (dec_conf->MPEGHAudioProfileLevel.value == MPEG3DA_PLI_LC_L5) {
      if (data->sampling_rate == 48000 || data->sampling_rate == 44100){
        CommonWarning("two resampling ratios supported for Lvl 5 at this sampling rate. Value 1 assumed");
      } else if ( data->sampling_rate == 32000 || data->sampling_rate == 29400 ) {
          CommonWarning("two resampling ratios supported for Lvl 5 at this sampling rate. Value 1.5 assumed");
      }
    }

    /* check for valid configuration */  
    for (i = 0; i < NUMBER_OF_ALLOWED_SAMPLING_RATES; i++) if (data->sampling_rate == allowedSamplingRates[i]) break;
    if (data->sampling_rate != allowedSamplingRates[i]) {
      CommonWarning("unsupported sampling rate!");
      return -1;
    }    
    if ((data->bitrate < 32000) || (data->bitrate > 1536000)){
      CommonWarning("unsopported bitrate for LC profile!");
      return -1;
    }
    if (data->flag_twMdct == 1) {
      CommonWarning("time warped MDCT not supported in LC profile");
      return -1;
    }
    if (data->flag_768 == 1) {
      CommonWarning("framelength 768 not supported in LC profile");
      return -1;
    }
    if ( (data->contentType == CONTENT_3D_SAOC_OBJECTS) || (data->saocFile[0] != '\0') || (data->saocBitrateFile[0] != '\0') ) {
      CommonWarning("SAOC not supported in LC profile");
      return -1;
    }
    if ( (data->objFile[0] != '\0') && (data->lowDelayMetadataCoding == 0) ) {
      CommonWarning("lowDelayMetadataCoding must be enabled for LC profile");
      return -1;
    }
    if (data->receiverDelayCompensation == 0 ) {
      CommonWarning("receiverDelayCompensation must be enabled in LC profile");
      return -1;
    }

    /* disable unsupported tools,  */    
    if ((data->sbrRatioIndex > 0) || (data->sbrenable != 0)) {
      CommonWarning("Changing configuration to comply with LC profile: SBR disabled!");
      data->sbrenable     = 0;
      data->sbrRatioIndex = 0;
    }
    if (data->usac212enable) {
      CommonWarning("Changing configuration to comply with LC profile: MPS212 disabled!");
      data->channels      = numChannel;
      data->usac212enable = 0;
      data->flag_mpsres   = 0;
    }
  }

  {
    /* CompatibleProfileLevelSet */
    unsigned int idx;
    for (idx = 0; idx < data->numCompatibleSets; idx++) {
      if (data->CompatibleSetIndication[idx] >= SUPPORTED_COMPATIBLEPROFILELEVEL_MIN && data->CompatibleSetIndication[idx] > SUPPORTED_COMPATIBLEPROFILELEVEL_MAX) {
        CommonWarning("CompatibleSetIndication value is out of supported range!");
        return -1;
      }
      if (data->CompatibleSetIndication[idx] >= MPEG3DA_PLI_BASELINE_L1 && data->CompatibleSetIndication[idx] <= MPEG3DA_PLI_BASELINE_L5) {
        data->hasBlProfileCompatibility = 1;
      }
    }
  }

  /* baseline profile */
  if (data->useBlProfile == 1) {
    int err = setupBaselineProfileEncoding(data, dec_conf, numChannel);
    if (0 != err) {
      return err;
    }
  }

  dec_conf->profileAndLevelIndication.value = dec_conf->MPEGHAudioProfileLevel.value;

#if	IAR
  if (data->contentType == CONTENT_3D_CHANNELS) {
    /* for the 3d channel content, it is required to use "rendering3DType" bitstream
    if it is not available, "rendering3DType" is assumed to be 1
    see more @ the function readIARData */
    if (data->iarFile[0] != '\0') {
      gIARBS = fopen ( data->iarFile, "rb" );
      /* setting the USAC_ID_EXT_ELE_FMT_CNVTR (=8) */
      data->extType[data->numExtElements] = USAC_ID_EXT_ELE_FMT_CNVTR;
      data->numExtElements++;
    } else {
      gIARBS = NULL;
    }
  }
#endif
 
  if (data->pmcFile[0] != '\0') {
    FILE *extProductionMetadataConfigFileHandle = NULL;
    int extensionDataSize = 0;
    unsigned char config[6144/8] = {'\0'};

    /* Open next object file */
    extProductionMetadataConfigFileHandle = fopen(data->pmcFile, "rb");

    if (extProductionMetadataConfigFileHandle) {
      /* Get file length */
      fseek(extProductionMetadataConfigFileHandle, 0, SEEK_END);
      extensionDataSize = ftell(extProductionMetadataConfigFileHandle);
      rewind (extProductionMetadataConfigFileHandle);

      /* Read data into buffer */
      if (extensionDataSize > 0) {
        fread(config, sizeof(unsigned char), extensionDataSize, extProductionMetadataConfigFileHandle);
      }

      /* if production metadata file is not exists */
      data->extType[data->numExtElements] = USAC_ID_EXT_TYPE_PRODUCTION_METADATA;
      data->extElementConfigLength[data->numExtElements] = extensionDataSize;
      memcpy(data->extElementConfigPayload[data->numExtElements], config, data->extElementConfigLength[data->numExtElements]);
      data->numExtElements++;

      fclose(extProductionMetadataConfigFileHandle);
    }
  }

  /* init PVC dependent on number channels in SBR */
  bs_pvc = (data->channels == 1) ? 1 : 0; /* PVC not supported in CPEs */

  /* ---- Sanity check ---- */
  if (core != NULL) {
    CommonWarning("EncUsacInit: USAC doesn't support scalabity");
    return -1;
  }
  if (data->channels < 1) {
    CommonWarning("EncUsacInit: Channel number smaller than 1");
    return -1;
  }else if(data->channels > 32) {
    CommonWarning("EncUsacInit: Channel number bigger than 32");
    return -1;
  }
  if (data->layer_num!=1) {
    CommonWarning("EncUsacInit: Scalability not supported in USAC");
    return -1;
  }

  /* ======================= */
  /* Process Encoder Options */
  /* ======================= */
  if (data->flag_960) {
    data->block_size_samples = 960;
  } else if(data->flag_768){
    data->block_size_samples = 768;
  } else {
    data->block_size_samples = 1024;
  }


  /* ---- decide number of tracks for AAC ---- */
  {
    int tracks = 0;
    int t=0;
    if (data->ep_config==-1) {
      t=1;
    } else if (data->ep_config==1) {
      if (data->channels == 1) {
      t = 3;
      } else if (data->channels == 2) {
        t = 7;
      } else {
        CommonWarning("for USAC only mono or stereo configurations are supported");
        return -1;
      }
    } else {
      CommonWarning("only ep1 or non error resilient syntax are supported for AAC");
      return -1;
    }

    data->tracks_per_layer[0] = t;
    tracks += t;

    data->track_num = tracks;
  }
  if (data->track_num==-1) {
    CommonWarning("EncUsacInit: setup of track_count failed");
    return -1;
  }


  /*--- TW-MDCT ----*/
  if (data->flag_twMdct == 1 ) {
    tw_init_enc();
  }


  *frameNumSample = data->block_size_samples;
  *numTrack       = data->track_num;

  /* determine outputFrameLength */
  switch(data->sbrRatioIndex){
  case 0: /* no sbr */
    if(data->block_size_samples == 768){
      outputFrameLength = 768;
    } else {
      outputFrameLength = 1024;
    }
    break;
  case 1:
    outputFrameLength = 4096;
    break;
  case 2:
  case 3:
    outputFrameLength = 2048;
    break;
  default:
    assert(0);
    break;
  }

  /*--- IGF ----*/
  data->igFStartSfbLB         = -1;
  data->igFStopSfbLB          = -1;
  data->igFStartSfbSB         = -1;
  data->igFStopSfbSB          = -1;
  data->igFUseHighRes         = 0;
  data->igFUseWhitening       = 0;
  data->igFIndependentTiling  = 1;
  data->igFAfterTnsSynth      = 0;

  if(data->enhancedNoiseFilling){
    if((data->sbrenable == 1) || (data->sbrRatioIndex!=0) || (data->codecMode!=USAC_ONLY_FD)) {
      data->enhancedNoiseFilling = 0;
      CommonWarning("EncUsacInit: setup of IGF failed");
    } else {
      int swb_offset_long[(NSFB_LONG+1)],num_swb_long_window;
      int swb_offset_short[(NSFB_LONG+1)];
      double samplingRate = ((data->block_size_samples == 768) ? ((double)data->sampling_rate * 4 / 3) : (double)data->sampling_rate);
      int igfStartSubband,igfStopSubband;
      data->igFStartFq = data->sampling_rate >> 2;
      data->igFStopFq = data->sampling_rate >> 1;
      data->igFUseHighRes = 1;
      data->igFUseWhitening = 0;
      data->igFIndependentTiling = 1;
      data->igFAfterTnsSynth = 0;

      num_swb_long_window = EncTf_psycho_acoustic_long_sfb_offset(
              samplingRate,
              data->block_size_samples,
              data->block_size_samples,
              swb_offset_long
        );
      /*
      data->igFStartFq = 15000;
      data->igFStopFq = 18000;
      */
      if (data->igFStopFq > (int)(samplingRate / 2)) {
        data->igFStopFq = (int)(samplingRate / 2);
      }
      if (data->igFStopFq < data->igFStartFq) {
        data->igFStopFq = data->igFStartFq;
      }

      igfStartSubband = (data->igFStartFq * data->block_size_samples) / (int)(samplingRate / 2);
      igfStopSubband = (data->igFStopFq * data->block_size_samples) / (int)(samplingRate / 2);

      data->igFStartSfbLB = -1;
      for(i=0;i<num_swb_long_window;i++) {
        if( igfStartSubband >= swb_offset_long[i]) {
            data->igFStartSfbLB = i;
        }
      }
      data->igFStartSfbLB = min(num_swb_long_window-5, data->igFStartSfbLB);

      data->igFStopSfbLB = -1;
      for(i=0;i<=num_swb_long_window;i++) {
        if( igfStopSubband >= swb_offset_long[i]) {
            data->igFStopSfbLB = i;
        }
      }
      data->igFStopSfbLB = max(data->igFStopSfbLB, data->igFStartSfbLB +2);

      if( data->igFStopSfbLB < 0 || data->igFStopSfbLB < 0 ) {
        data->enhancedNoiseFilling = 0;
        CommonWarning("EncUsacInit: setup of IGF failed");
      }
      data->igFStartIdx = min(15,(data->igFStartSfbLB -11)/2);
      data->igFStopIdx = min(7,(int)(((data->igFStopSfbLB - data->igFStartSfbLB)<<3) /(float)(num_swb_long_window - (data->igFStartSfbLB+1)) -0.5));
#ifdef RM1_3D_BUGFIX_IGF_05
      data->igFNTilesLB = IGF_getNTiles_enc(data->sampling_rate,data->block_size_samples,data->igFStartSfbLB,data->igFStopSfbLB,1,0,swb_offset_long);
      data->igFNTilesSB = IGF_getNTiles_enc(data->sampling_rate,data->block_size_samples,data->igFStartSfbSB,data->igFStopSfbSB,1,1,swb_offset_short);
#endif
    }
  }


  /* ================================ */
  /* Write Header Information         */
  /* ================================ */
  for (i_dec_conf=0; i_dec_conf<data->track_num; i_dec_conf++) {
    int sx;
    int frameLenIdx = 0, sbrRatioIdx = 0;
    USAC_CONFIG *pUsacConfig = &(dec_conf[i_dec_conf].audioSpecificConfig.specConf.usacConfig);
    USAC_CORE_CONFIG   *pUsacCoreConfig = NULL;
    USAC_SBR_CONFIG    *pUsacSbrConfig  = NULL;
    USAC_MPS212_CONFIG *pUsacMps212Config = NULL;

    /* ----- common part ----- */
    dec_conf[i_dec_conf].avgBitrate.value = 0;
    dec_conf[i_dec_conf].maxBitrate.value = 0;
    dec_conf[i_dec_conf].avgBitrate.value = data->bitrate;
    dec_conf[i_dec_conf].bufferSizeDB.value = 6144*data->channels /8;
    dec_conf[i_dec_conf].audioSpecificConfig.samplingFrequency.value = data->sampling_rate;
    for (sx=0;sx<0xf;sx++) {
      if (dec_conf[i_dec_conf].audioSpecificConfig.samplingFrequency.value == samplFreqIndex[sx]) break;
    }
    dec_conf[i_dec_conf].audioSpecificConfig.samplingFreqencyIndex.value = sx;
    dec_conf[i_dec_conf].audioSpecificConfig.channelConfiguration.value = 0; /* data->channels; */
    dec_conf[i_dec_conf].audioSpecificConfig.audioDecoderType.value = USAC;
    dec_conf[i_dec_conf].audioSpecificConfig.epConfig.value =0;

    /* ----- USAC specific part ----- */
    pUsacConfig->usacSamplingFrequency.value = dec_conf[i_dec_conf].audioSpecificConfig.samplingFrequency.value;
    for(sx=0; sx<0x1f; sx++){
      if(pUsacConfig->usacSamplingFrequency.value == UsacSamplingFrequencyTable[sx]) break;
    }

    /* mpegh3daProfileLevelIndication = 0x0A -> High profile, level 5 */
    dec_conf[i_dec_conf].profileAndLevelIndication.value    = dec_conf[i_dec_conf].MPEGHAudioProfileLevel.value;
    pUsacConfig->mpegh3daProfileLevelIndication.value       = dec_conf[i_dec_conf].MPEGHAudioProfileLevel.value; /* Somehow this should be related to: dec_conf[i_dec_conf].MPEGHAudioLevel and dec_conf[i_dec_conf].MPEGHAudioProfile which is also written into MHA box */
    pUsacConfig->usacSamplingFrequencyIndex.value           = sx;
    pUsacConfig->coreSbrFrameLengthIndex.value              = getCoreSbrFrameLengthIndex(outputFrameLength, data->sbrRatioIndex);
    pUsacConfig->reservedCoreQmfDelayCompensation.value     = data->coreQmfDelayCompensation;
    pUsacConfig->receiverDelayCompensation.value            = data->receiverDelayCompensation;
    pUsacConfig->referenceLayout.CICPspeakerLayoutIdx.value = (data->contentType==CONTENT_3D_CHANNELS) ? UsacConfig_GetCICPIndex(numChannel) : data->cicpIn; /* 13: signal cicp  22.2 for objects only */
    pUsacConfig->usacDecoderConfig.numElements              = (data->contentType==CONTENT_3D_CHANNELS) ? UsacConfig_GetNumElements(numChannel) : numChannel;

    if (data->mctMode > -1) {
      pUsacConfig->usacDecoderConfig.numElements = numChannel;
    }

    pUsacConfig->frameLength.value = 0;


#ifndef SPECIFY_INPUT_CONTENTTYPE
#ifdef WD1_FRAMEWORK_CONFIG_3D
    pUsacConfig->frameworkConfig3d.signals3D.numAudioChannels = numChannel;
    pUsacConfig->frameworkConfig3d.signals3D.numAudioObjects = 0;
    pUsacConfig->frameworkConfig3d.signals3D.numSAOCTransportChannels = 0;
    pUsacConfig->frameworkConfig3d.signals3D.numHOATransportChannels = 0;
#endif
#endif

#ifdef SPECIFY_INPUT_CONTENTTYPE
    switch (data->contentType) {

      case CONTENT_3D_CHANNELS:
        pUsacConfig->frameworkConfig3d.signals3D.numAudioChannels = numChannel;
        pUsacConfig->frameworkConfig3d.signals3D.numAudioObjects = 0;
        pUsacConfig->frameworkConfig3d.signals3D.numSAOCTransportChannels = 0;
        pUsacConfig->frameworkConfig3d.signals3D.numHOATransportChannels = 0;
        break;

      case CONTENT_3D_OBJECTS:
        pUsacConfig->frameworkConfig3d.signals3D.numAudioChannels = 0;
        pUsacConfig->frameworkConfig3d.signals3D.numAudioObjects = numChannel;
        pUsacConfig->frameworkConfig3d.signals3D.numSAOCTransportChannels = 0;
        pUsacConfig->frameworkConfig3d.signals3D.numHOATransportChannels = 0;
        break;

      case CONTENT_3D_SAOC_OBJECTS:
        pUsacConfig->frameworkConfig3d.signals3D.numAudioChannels = 0;
        pUsacConfig->frameworkConfig3d.signals3D.numAudioObjects = 0;
        pUsacConfig->frameworkConfig3d.signals3D.numSAOCTransportChannels = numChannel;
        pUsacConfig->frameworkConfig3d.signals3D.numHOATransportChannels = 0;
        break;

      case CONTENT_3D_HOA:
        pUsacConfig->frameworkConfig3d.signals3D.numAudioChannels = 0;
        pUsacConfig->frameworkConfig3d.signals3D.numAudioObjects = 0;
        pUsacConfig->frameworkConfig3d.signals3D.numSAOCTransportChannels = 0;
        pUsacConfig->frameworkConfig3d.signals3D.numHOATransportChannels = numChannel;
        break;

      default:
        CommonWarning("EncUsacInit: Unknown Content Type index");
        return -1;
    }

#else
    if (pUsacConfig->usacDecoderConfig.numElements == numChannel) {
      /* SCE only channelmap: no usac212 for objects */
      data->usac212enable = 0;

#ifdef WD1_FRAMEWORK_CONFIG_3D


      if (data->hoaFile[0] != '\0')
      {
        pUsacConfig->frameworkConfig3d.signals3D.numAudioChannels = 0;
        pUsacConfig->frameworkConfig3d.signals3D.numAudioObjects = 0;
        pUsacConfig->numObjects.value = 0;
        pUsacConfig->frameworkConfig3d.signals3D.numSAOCTransportChannels = 0;
        pUsacConfig->frameworkConfig3d.signals3D.numHOATransportChannels  = numChannel;

        /* No resorting at decoder output */
        pUsacConfig->channelConfigurationIndex.value = 31;
      }
      else
      {
        if ((data->objFile[0] != '\0') && (data->saocFile[0] == '\0')) {
          pUsacConfig->frameworkConfig3d.signals3D.numAudioChannels = 0;
          pUsacConfig->frameworkConfig3d.signals3D.numAudioObjects = numChannel;
          pUsacConfig->numObjects.value = numChannel;
          pUsacConfig->frameworkConfig3d.signals3D.numSAOCTransportChannels = 0;
          pUsacConfig->frameworkConfig3d.signals3D.numHOATransportChannels = 0;

          /* No resorting at decoder output */
          pUsacConfig->channelConfigurationIndex.value = 31;
        }
        else if ( data->saocFile[0] != '\0' ) {
          pUsacConfig->frameworkConfig3d.signals3D.numAudioChannels = 0;
          pUsacConfig->frameworkConfig3d.signals3D.numAudioObjects = 0;
          pUsacConfig->frameworkConfig3d.signals3D.numSAOCTransportChannels = numChannel;
          pUsacConfig->frameworkConfig3d.signals3D.numHOATransportChannels = 0;

          /* No resorting at decoder output */
          pUsacConfig->channelConfigurationIndex.value = 31;
        }
      }
#else
      if ((data->objFile[0] != '\0') || (data->saocFile[0] == '\0')) {
        pUsacConfig->numObjects.value = numChannel;
        pUsacConfig->usacChannelConfig.numOutChannels = 0;
        pUsacConfig->channelConfigurationIndex.value = 31;
      }
#endif
    }
#endif

#if PMCNO
  if (data->pmcFile[0] != '\0') {
    unsigned char config[6144/8] = {'\0'};
    int i;
    int configbitlength = 0;
    unsigned int hasrefdistance = 0;
    unsigned int bsrefdistance = 0;
    unsigned int numObjectGroups = 0;
    unsigned int numChannelGroups = 0;
    unsigned int hasobjectdistance[USAC_MAX_AUDIO_GROUPS] = {0};
    unsigned int directHeadphone[USAC_MAX_AUDIO_GROUPS] = {0};
    float refdistance = 0.0f;

    gProdMetadataConfigFileHandle = fopen(data->pmcFile, "rb");
    if (gProdMetadataConfigFileHandle == NULL) {
      CommonWarning("Could not open Production Metadata Configuration File!");
      return -1;
    }

    /* Get Production Metadata Config */
    data->extType[data->numExtElements] = USAC_ID_EXT_TYPE_PRODUCTION_METADATA;

    /* Read high resolution reference distance */
    /* read has_reference_distance */
    if(feof(gProdMetadataConfigFileHandle)){
      CommonWarning("No has_reference_distance!");
      return -1;
    }
    fscanf(gProdMetadataConfigFileHandle, "%d", &hasrefdistance);

    /* if has_reference_distance is 1 */
    if (hasrefdistance == 1) {
      if (feof(gProdMetadataConfigFileHandle)) {
        CommonWarning("No reference distance!");
        return -1;
      }

      /* read reference_distance [m] */
      fscanf(gProdMetadataConfigFileHandle, "%f", &refdistance);

      /* reference distance [m] to bs_reference_distance */
      refdistance = log10(refdistance * 100.0f) / 0.014214299201363f - 119.0f; /* 0.014214299201363 = 0.0472188798661443*log10(2); */

      /* round up */
      bsrefdistance = (int)(refdistance + 0.5);

      /* bs_reference_distance = 57 */
      config[0] = (0x80) | bsrefdistance;
      configbitlength = configbitlength + 8;
    } else {
      /* if has_reference_distance is not 1 */
      config[0] = 0x00;
      configbitlength++;
    }
    data->extElementConfigLength[data->numExtElements] = 1;

    /* high resolution object distance */
    /* read numObjectGroups */
    if (feof(gProdMetadataConfigFileHandle)) {
      CommonWarning("No numObjectGroups!");
      return -1;
    }
    fscanf(gProdMetadataConfigFileHandle, "%d", &numObjectGroups);

    if (numObjectGroups > 0) {
      /* while hasobjectdistance is exists */
      for (i = 0 ; i < numObjectGroups; i++) {
        int temp = 0;
        if (feof(gProdMetadataConfigFileHandle)) {
          CommonWarning("No hasobjectdistance!");
          return -1;
        }
        /* read hasobjectdistance */
        fscanf(gProdMetadataConfigFileHandle, "%d", &temp);
        hasobjectdistance[i] = temp;

        if (temp == 1) {
          config[configbitlength / 8] |= 1 << (7 - configbitlength % 8);
        }
        configbitlength++;
      }
    } else {
      /* If numObjectGroups is none */
    }

    /* direct to headphone */
    /* read numChannelGroups */
    if (feof(gProdMetadataConfigFileHandle)) {
      CommonWarning("No numChannelGroups!");
      return -1;
    }
    fscanf(gProdMetadataConfigFileHandle, "%d", &numChannelGroups);
    if (numChannelGroups > 0) {
      /* while numChannelGroups is exists */
      for (i = 0; i < numChannelGroups; i++) {
        int temp = 0;
        if(feof(gProdMetadataConfigFileHandle)){
          CommonWarning("No directHeadphone!");
          return -1;
        }
        fscanf(gProdMetadataConfigFileHandle, "%d", &temp);
        directHeadphone[i] = temp;

        if (temp == 1) {
          config[configbitlength / 8] |= 1 << (7 - configbitlength % 8);
        }
        configbitlength++;
      }
    } else {
      /* If numChannelGroups is none */
    }

    data->extElementConfigLength[data->numExtElements] = configbitlength / 8 + 1;
    memcpy(data->extElementConfigPayload[data->numExtElements], config, data->extElementConfigLength[data->numExtElements]);
    data->numExtElements++;
  }else {
    unsigned char config[6144/8] = {'\0'};

    /* if production metadata file is not exists */
    data->extType[data->numExtElements] = USAC_ID_EXT_TYPE_PRODUCTION_METADATA;
    config[0] = 0x00;
    data->extElementConfigLength[data->numExtElements] = 1;
    memcpy(data->extElementConfigPayload[data->numExtElements], config, data->extElementConfigLength[data->numExtElements]);
    data->numExtElements++;
 }
#endif

    {
      int j = 0;

      pUsacConfig->usacDecoderConfig.numElements += data->numExtElements;
      
      for (j = 0; j < data->numExtElements; ++j ) 
      {
        pUsacConfig->usacDecoderConfig.usacElementType[elemIdx + j] = USAC_ELEMENT_TYPE_EXT;
        
        pUsacConfig->usacDecoderConfig.usacElementConfig[elemIdx +j].usacExtConfig.usacExtElementType = data->extType[j];
        
        switch (data->extType[j])
        {
        case USAC_ID_EXT_ELE_OBJ:
        case USAC_ID_EXT_ELE_SAOC_3D:
        case USAC_ID_EXT_ELE_HOA:
        case USAC_ID_EXT_ELE_MC:
        case USAC_ID_EXT_TYPE_PRODUCTION_METADATA:
        case USAC_ID_EXT_ELE_AUDIOPREROLL:
          pUsacConfig->usacDecoderConfig.usacElementConfig[elemIdx +j].usacExtConfig.usacExtElementDefaultLengthPresent.length = 1;
          pUsacConfig->usacDecoderConfig.usacElementConfig[elemIdx +j].usacExtConfig.usacExtElementDefaultLengthPresent.value  = 0;
          pUsacConfig->usacDecoderConfig.usacElementConfig[elemIdx +j].usacExtConfig.usacExtElementDefaultLength               = 0;
          pUsacConfig->usacDecoderConfig.usacElementConfig[elemIdx +j].usacExtConfig.usacExtElementPayloadFrag.length          = 1;
          pUsacConfig->usacDecoderConfig.usacElementConfig[elemIdx +j].usacExtConfig.usacExtElementPayloadFrag.value           = 0;
          pUsacConfig->usacDecoderConfig.usacElementConfig[elemIdx +j].usacExtConfig.usacExtElementConfigLength                = data->extElementConfigLength[j];
          memcpy(pUsacConfig->usacDecoderConfig.usacElementConfig[elemIdx +j].usacExtConfig.usacExtElementConfigPayload, data->extElementConfigPayload[j], data->extElementConfigLength[j]);
          break;
#if IAR
        case USAC_ID_EXT_ELE_FMT_CNVTR:
          /* no config for USAC_ID_EXT_ELE_FMT_CNVTR */
          break;
#endif
        default:
          fprintf(stderr, "Error: Unkown extension data type.\n");
          assert(0);
        }
      }
    }

    i_ch = 0;
    for (elemIdx = data->numExtElements;elemIdx < pUsacConfig->usacDecoderConfig.numElements; elemIdx++){

      pUsacConfig->usacDecoderConfig.usacElementType[elemIdx] = (data->contentType==CONTENT_3D_CHANNELS) ? UsacConfig_GetElementType(elemIdx-data->numExtElements, numChannel) : USAC_ELEMENT_TYPE_SCE;

      if (data->mctMode > -1) {
        pUsacConfig->usacDecoderConfig.usacElementType[elemIdx] = USAC_ELEMENT_TYPE_SCE;
      }

      pUsacCoreConfig   = UsacConfig_GetUsacCoreConfig(pUsacConfig, elemIdx);
      pUsacSbrConfig    = UsacConfig_GetUsacSbrConfig(pUsacConfig, elemIdx);
      pUsacMps212Config = UsacConfig_GetUsacMps212Config(pUsacConfig, elemIdx);

      pUsacCoreConfig->tw_mdct.value      = data->flag_twMdct;
      pUsacCoreConfig->fullbandLPD.value  = 0; 
      pUsacCoreConfig->noiseFilling.value = (pUsacConfig->usacDecoderConfig.usacElementType[elemIdx]==USAC_ELEMENT_TYPE_LFE)?0:data->flag_noiseFilling;
      pUsacCoreConfig->enhancedNoiseFilling.value = data->enhancedNoiseFilling;
      pUsacCoreConfig->reserved.value = 0;
      pUsacCoreConfig->igf_StartIdx.value = data->igFStartIdx;
      pUsacCoreConfig->igf_StopIdx.value = data->igFStopIdx;
      pUsacCoreConfig->igf_UseHighRes.value = data->igFUseHighRes;
      pUsacCoreConfig->igf_UseWhitening.value = data->igFUseWhitening;
      pUsacCoreConfig->igf_IndependentTiling.value = data->igFIndependentTiling;
      pUsacCoreConfig->igf_AfterTnsSynth.value = data->igFAfterTnsSynth;

      if ( NULL != pUsacSbrConfig) {
        pUsacSbrConfig->harmonicSBR.value   = data->flag_harmonicBE;
        pUsacSbrConfig->bs_interTes.value   = 1;
        pUsacSbrConfig->bs_pvc.value        = bs_pvc;
      }

      switch (pUsacConfig->usacDecoderConfig.usacElementType[elemIdx]) {
        case USAC_ELEMENT_TYPE_LFE:
          data->channelElementType[i_ch] = USAC_CHANNEL_ELEMENTTYPE_LFE;
          data->channelElementIndex[i_ch] = elemIdx;
          i_ch++;
          break;
        case USAC_ELEMENT_TYPE_SCE:
          data->channelElementType[i_ch] = USAC_CHANNEL_ELEMENTTYPE_SCE;
          data->channelElementIndex[i_ch] = elemIdx;
          i_ch++;
          break;
        case USAC_ELEMENT_TYPE_CPE:
          if ( data->usac212enable && !data->flag_mpsres && !UsacConfig_GetIsQceElement(elemIdx, numChannel, data->flag_mpsqce)) {
            data->channelElementType[i_ch]   = USAC_CHANNEL_ELEMENTTYPE_CPE_DMX;
            data->channelElementType[i_ch+1] = USAC_CHANNEL_ELEMENTTYPE_CPE_RES;
          }
          else {
            data->channelElementType[i_ch]   = USAC_CHANNEL_ELEMENTTYPE_CPE_L;
            data->channelElementType[i_ch+1] = USAC_CHANNEL_ELEMENTTYPE_CPE_R;
          }
          data->channelElementIndex[i_ch] = elemIdx;
          data->channelElementIndex[i_ch+1] = elemIdx;
          i_ch += 2;
          break;
        case USAC_ELEMENT_TYPE_EXT:  /* no ext element support yet */
        default:
          assert(0);  /* unknown element type */
          break;
      }
    }

#ifdef USE_FILL_ELEMENT
    /* Fill Element should be the last Element */
    if(data->bUseFillElement) {
      USAC_EXT_CONFIG * pFillElem = &(pUsacConfig->usacDecoderConfig.usacElementConfig[pUsacConfig->usacDecoderConfig.numElements].usacExtConfig);
      pUsacConfig->usacDecoderConfig.usacElementType[pUsacConfig->usacDecoderConfig.numElements] = USAC_ELEMENT_TYPE_EXT;

      pFillElem->usacExtElementType=USAC_ID_EXT_ELE_FILL;
      pFillElem->usacExtElementConfigLength=0;
      pFillElem->usacExtElementDefaultLengthPresent.length=1;
      pFillElem->usacExtElementDefaultLengthPresent.value=0;
      pFillElem->usacExtElementPayloadFrag.length=1;
      pFillElem->usacExtElementPayloadFrag.value=0;

      pUsacConfig->usacDecoderConfig.numElements++;
    }
#endif

    if (data->numCompatibleSets > 0) {
      unsigned int idx;
      COMPATIBLE_PROFILE_LEVEL_SET* CompatibleProfileLevelSet = &pUsacConfig->usacConfigExtension.usacCompatibleProfileLevelSet;

      CompatibleProfileLevelSet->numCompatibleSets.value = data->numCompatibleSets;
      CompatibleProfileLevelSet->reserved.value = 0;

      for (idx = 0; idx < data->numCompatibleSets; idx++) {
        CompatibleProfileLevelSet->CompatibleSetIndication[idx].value = data->CompatibleSetIndication[idx];
      }


      pUsacConfig->usacConfigExtension.usacConfigExtType[pUsacConfig->usacConfigExtension.numConfigExtensions] = USAC_CONFIG_EXT_COMPATIBLE_PROFILELVL_SET;
      pUsacConfig->usacConfigExtension.usacConfigExtLength[pUsacConfig->usacConfigExtension.numConfigExtensions] = 1 + data->numCompatibleSets;
      pUsacConfig->usacConfigExtension.numConfigExtensions++;
      pUsacConfig->usacConfigExtensionPresent.value = 1;
    } else {
      pUsacConfig->usacConfigExtensionPresent.value = 0;
    }
  }



  /* ================================ */
  /* Init coding tools                */
  /* ================================ */

  /* Initialization Classification */
  if ((data->codecMode == USAC_SWITCHED)&&(data->bitrate < 64000)) {
    init_classification(&classfyData, data->bitrate, 1);
  }


  /* Initilization SBR */
  if (data->sbrRatioIndex > 0) {
    USAC_CONFIG *pUsacConfig = &(dec_conf[0].audioSpecificConfig.specConf.usacConfig);
    int elem_start_ch = 0;

    switch(data->sbrRatioIndex){
      case 1:
        samplingRateCore = data->sampling_rate = data->sampling_rate/4;
        break;
      case 2:
        data->sampling_rate = (int)(3.0f * (float)data->sampling_rate / 8.0f);
        samplingRateCore = (int)((4.f/3.f) * (float)data->sampling_rate);
        break;
      case 3:
        samplingRateCore = data->sampling_rate = data->sampling_rate / 2;
        break;
      default:
        assert(0);
        break;
    }

    for (elemIdx = 0;elemIdx < pUsacConfig->usacDecoderConfig.numElements; elemIdx++){
      USAC_SBR_CONFIG * pUsacSbrConfig = UsacConfig_GetUsacSbrConfig(pUsacConfig, elemIdx);
      USAC_SBR_HEADER * pUsacSbrDfltHeader = &pUsacSbrConfig->sbrDfltHeader;

      int numChanInElem = 0;
      int numSbrChanInElem = 0;

      switch (pUsacConfig->usacDecoderConfig.usacElementType[elemIdx]) {
        case USAC_ELEMENT_TYPE_SCE:
          numChanInElem    = 1;
          numSbrChanInElem = 1;
          break;
        case USAC_ELEMENT_TYPE_CPE:
          numChanInElem    = 2;
          numSbrChanInElem = 2;
          break;
        case USAC_ELEMENT_TYPE_LFE:
          numChanInElem    = 1;
          numSbrChanInElem = 0;
        default:
          break;
      }

      if(data->debugLevel>0){
        fprintf(stderr, "EncUsacInit: SBR enable\n");
      }

      /* Init SBR */
      data->ch_elm_total = data->channels;

      for (i_ch= elem_start_ch ; i_ch < elem_start_ch + numSbrChanInElem ; i_ch++) {
        struct channelElementData *chData  = &data->chElmData[i_ch];
        chData->startChIdx = elem_start_ch;
        chData->endChIdx = elem_start_ch+1;
        sbrChan[chData->startChIdx].bs_pvc = bs_pvc;
        chData->chNum =  data->ch_elm_total;
        SbrInit(data->bitrate, data->sampling_rate_out, chData->chNum, (SBR_RATIO_INDEX)data->sbrRatioIndex, &sbrChan[chData->startChIdx]);
        chData->sbrChan[chData->startChIdx] = &sbrChan[chData->startChIdx];

        if(i_ch > elem_start_ch) {
          sbrChan[chData->endChIdx].bs_pvc = bs_pvc;
          SbrInit(data->bitrate, data->sampling_rate_out, chData->chNum, (SBR_RATIO_INDEX)data->sbrRatioIndex, &sbrChan[chData->endChIdx]);
          chData->sbrChan[chData->endChIdx] = &sbrChan[chData->endChIdx];
        }

        pUsacSbrDfltHeader->header_extra1.value = 0;
        pUsacSbrDfltHeader->header_extra2.value = 0;
        pUsacSbrDfltHeader->start_freq.value = sbrChan[chData->startChIdx].startFreq;
        pUsacSbrDfltHeader->stop_freq.value = sbrChan[chData->startChIdx].stopFreq;
      }

      elem_start_ch += numChanInElem;
    }
  }
  else {
    for(i=0; i<data->ch_elm_total; i++){
      struct channelElementData *chData  = &data->chElmData[i];
      chData->sbrChan[0] = chData->sbrChan[1] = NULL;
    }
  }


  /* Initilization MPEGS */
  if(data->usac212enable){
    USAC_CONFIG *pUsacConfig = &(dec_conf[0].audioSpecificConfig.specConf.usacConfig);
    for (elemIdx = 0;elemIdx < pUsacConfig->usacDecoderConfig.numElements; elemIdx++){
      int bufferSize = 0;
      int treeConfig;
      int timeSlots = (data->sbrRatioIndex == 1)?64:32;
      Stream bitstream;
      USAC_CPE_CONFIG *pUsacCpeConfig = &(pUsacConfig->usacDecoderConfig.usacElementConfig[elemIdx].usacCpeConfig);
      USAC_MPS212_CONFIG *pUsacMps212Config = UsacConfig_GetUsacMps212Config(pUsacConfig, elemIdx);
      SAC_ENC_USAC_MPS212_CONFIG sacEncUsacMps212Config;

      pUsacCpeConfig->qceFlag = 0;

      if (pUsacConfig->usacDecoderConfig.usacElementType[elemIdx] == USAC_ELEMENT_TYPE_CPE ) {

        if(data->debugLevel>0){
          fprintf(stderr, "EncUsacInit: USAC212 enable\n");
        }
        InitStream(&bitstream, NULL, STREAM_WRITE);

        if(data->flag_mpsres){
          treeConfig = 2122;
        }else{
          treeConfig = 2122;
        }

        pUsacCpeConfig->stereoConfigIndex.value = (data->flag_mpsres)?3:1;

        /* open MPS212 encoder */
        data->hSpatialEnc[elemIdx] = SpatialEncOpen(treeConfig, timeSlots, data->sampling_rate_out, &bufferSize, &bitstream, data->flag_ipd, data->flag_mpsres, data->tsdInputFileName, data->hybridResidualMode);

        /* get configuration for UsacConfig() */
        SpatialEncGetUsacMps212Config(data->hSpatialEnc[elemIdx], &sacEncUsacMps212Config);

        /* copy to struct */
        __fillUsacMps212Config(pUsacMps212Config, &sacEncUsacMps212Config);

        /* init delay compensation */
        SpaceEncInitDelayCompensation(data->hSpatialEnc[elemIdx], (2*2048+202));

        /* allow QCE if no residual coding */
        if ((UsacConfig_GetIsQceElement(elemIdx, numChannel, data->flag_mpsqce)>0)){
          pUsacCpeConfig->qceFlag = (data->flag_mpsres) ? 2 : 1;
          pUsacCpeConfig->stereoConfigIndex.value = 3;
          pUsacMps212Config->bsPseudoLr.value = 0;
          data->isQceElement[elemIdx] = UsacConfig_GetIsQceElement(elemIdx, numChannel, data->flag_mpsqce);
        }
        else {
          data->isQceElement[elemIdx] = 0;
        }

      }
    }
  } else {
    USAC_CONFIG *pUsacConfig = &(dec_conf[0].audioSpecificConfig.specConf.usacConfig);
    for (elemIdx = 0;elemIdx < pUsacConfig->usacDecoderConfig.numElements; elemIdx++){
      USAC_CONFIG *pUsacConfig = &(dec_conf[0].audioSpecificConfig.specConf.usacConfig);
      USAC_CPE_CONFIG *pUsacCpeConfig = &(pUsacConfig->usacDecoderConfig.usacElementConfig[elemIdx].usacCpeConfig);
      pUsacCpeConfig->stereoConfigIndex.value = 0;
      pUsacCpeConfig->qceFlag = 0;
    }
  }

  /* Initilization core coding */
  if (EncUsac_window_init(data, wshape_flag) != 0) return -1;
  if (EncUsac_bandwidth_init(data) != 0) return -1;
  if (EncUsac_data_init(data) != 0) return -1;
  /* bit reservoir*/
  data->max_bitreservoir_bits = 6144*data->channels;
  data->available_bitreservoir_bits = data->max_bitreservoir_bits;
  {
    int average_bits_total = (data->bitrate*data->block_size_samples) / data->sampling_rate;
    data->available_bitreservoir_bits -= average_bits_total;
  }
  *frameMaxNumBit = data->max_bitreservoir_bits;

  /* Initialization Fd coding  */
  if((data->codecMode==USAC_SWITCHED) || (data->codecMode==USAC_ONLY_FD)){
    if(data->debugLevel>0){
      fprintf(stderr, "EncUsacInit: FD coding enable\n");
    }
  }

  /* ---- initialize TNS ---- */
  if (EncUsac_tns_init(data, data->tns_select) != 0) return -1;

  /* initialize psychoacoustic module (does nothing at the moment) */
  EncTf_psycho_acoustic_init();

  /* initialize FD Qc and noiseless coding*/
  for (i = 0; i < MAX_TIME_CHANNELS; i++)
    usac_init_quantize_spectrum(&(data->quantInfo[i]), data->debugLevel );

  /* initialize window switching  */
  data->flag_winSwitchFD = 0;
  if (data->flag_winSwitchFD) {
    data->win_sw_module = ntt_win_sw_init(data->channels, data->block_size_samples);
  }


  /* Initialization Td coding  */
  switch (data->codecMode) {
  case USAC_SWITCHED:
  case USAC_ONLY_TD:

    if(data->debugLevel>0){
      fprintf(stderr, "EncUsacInit: TD coding enable\n");
    }

    for (i_ch=0; i_ch<data->channels; i_ch++) {

      data->tdenc[i_ch] = (USAC_TD_ENCODER *)calloc(1, sizeof(*data->tdenc[i_ch]));

      if ( (samplingRateCore) < SR_MIN || (samplingRateCore) > SR_MAX ) {
        CommonWarning("EncUsacInit():Sampling frequency outside of range");
        return -1;
      } else {

        data->tdenc[i_ch]->fscale = (int) ( ((float)FSCALE_DENOM * (samplingRateCore) / 12800.0F) + 0.5F );
        if (data->tdenc[i_ch]->fscale > FAC_FSCALE_MAX){
          data->tdenc[i_ch]->fscale = FAC_FSCALE_MAX;
        }
        if (data->tdenc[i_ch]->fscale < FAC_FSCALE_MIN){
          data->tdenc[i_ch]->fscale = FAC_FSCALE_MIN;
        }

        init_coder_lf(data->tdenc[i_ch], data->block_size_samples, data->tdenc[i_ch]->fscale);
      }

      data->td_bitrate[i_ch] = data->bitrate;
      if(data->sbrenable)
        data->td_bitrate[i_ch] -= 1500;
      if(data->usac212enable)
        data->td_bitrate[i_ch] -= 1500;
      data->td_bitrate[i_ch] /= data->channels;
      config_coder_lf(data->tdenc[i_ch], samplingRateCore, data->td_bitrate[i_ch]);
      if((data->tdenc[i_ch])->mode<0){
        CommonWarning("EncUsacInit: no td coding mode found for the user defined parameters" );
        return -1;
      }
      data->acelp_core_mode[i_ch] = (data->tdenc[i_ch])->mode;
      if(data->debugLevel>0){
        fprintf(stderr, "EncUsacInit: acelp core mode=%d\n", (data->tdenc[i_ch])->mode);
      }
    }
    break;
  default:
    data->acelp_core_mode[0] = 0;
    break;
  }

  /* transport ASC to AudioPreRoll */
  err = EncUsacAudioPreRoll_setConfig(data,
                                      asc,
                                      dec_conf);
  if (0 != err) {
    return err;
  }

  /*initialise internal decoder module */
  data->hTfDec = NULL;
  CreateIntDec(&data->hTfDec, data->channels, data->sampling_rate, data->block_size_samples);

  {
    /* check for valid Baseline profile configuration */
    int idx;
    int tmpProfileLevels[1 + USAC_MAX_COMPATIBLE_PROFILE_LEVELS];  /* Profile Level Indication + Maximum Number of Compatible Sets */

    tmpProfileLevels[0] = dec_conf->MPEGHAudioProfileLevel.value;
    for (idx = 0; idx < data->numCompatibleSets; idx++) {
      tmpProfileLevels[idx+1] = data->CompatibleSetIndication[idx];
    }

    for (idx = 0; idx < (1 + data->numCompatibleSets); idx++) {
      if (tmpProfileLevels[idx] >= MPEG3DA_PLI_BASELINE_L1 && tmpProfileLevels[idx] <= MPEG3DA_PLI_BASELINE_L5) {
        int err = checkBaselineProfileConfiguration(data, dec_conf, numChannel, tmpProfileLevels[idx]);
        if (0 != err) {
          return err;
        }
      }
    }
  }

  return 0;
}

static int __getIndependencyFlag(HANDLE_ENCODER_DATA data, int offset) {

  int bUsacIndependencyFlag = 0;

  if(data){
    int tmpCnt = (data->usacIndependencyFlagCnt + offset)%data->usacIndependencyFlagInterval;
    if(tmpCnt == 0) bUsacIndependencyFlag = 1;
  }

  return bUsacIndependencyFlag;
}

int EncUsac_getIndependencyFlag(HANDLE_ENCODER enc, int offset){

  HANDLE_ENCODER_DATA data = enc->data;

  return __getIndependencyFlag(data, offset);
}

static int EncUsac_writeMpegh3daExtElement(HANDLE_BSBITSTREAM     output_au,
                                           const int             *extensionDataPresent,
                                           const int             *extensionDataSize,
                                           const unsigned char    extensionData[USAC_MAX_EXTENSION_PAYLOADS][USAC_MAX_EXTENSION_PAYLOAD_LEN],
                                           const int              numExtElements
) {
  int bitCount = 0;
  int i        = 0;
  int j        = 0;

  for (i = 0; i < numExtElements; ++i) {
    int usacExtElementUseDefaultLength = 0;
    int usacExtElementPresent          = extensionDataPresent[i];
    int usacExtElementPayloadLength    = extensionDataSize[i];

    if (1 == usacExtElementPresent && 0 == usacExtElementPayloadLength) {
      CommonExit(-1, "EncUsac_writeMpegh3daExtElement: invalid extension payload!");
    }

    BsPutBit(output_au, usacExtElementPresent, 1);
    bitCount += 1;

    if (1 == usacExtElementPresent) {
      BsPutBit(output_au, usacExtElementUseDefaultLength, 1);
      bitCount += 1;

      if (usacExtElementUseDefaultLength) {
        CommonExit(-1, "EncUsac_writeMpegh3daExtElement: usacExtElementUseDefaultLength shall be 0!");
      } else {
        if (usacExtElementPayloadLength >= 255) {
          int valueAdd = usacExtElementPayloadLength - 255 + 2;
          BsPutBit(output_au, 255, 8);
          BsPutBit(output_au, valueAdd, 16);
          bitCount += 24;
        } else {
          BsPutBit(output_au, usacExtElementPayloadLength, 8);
          bitCount += 8;
        }
      }

      for (j = 0; j < usacExtElementPayloadLength; ++j) {
        BsPutBit(output_au, extensionData[i][j], 8);
        bitCount += 8;
      }
    }
  }

  return bitCount;
}

/*****************************************************************************************
 ***
 *** Function:    EncUsacFrame
 ***
 *** Purpose:     processes a block of time signal input samples into a bitstream
 ***              based on USAC encoding
 ***
 *** Description:
 ***
 ***
 *** Parameters:
 ***
 ***
 *** Return Value:  returns the number of used bits
 ***
 *** **** MPEG-4 VM ****
 ***
 ****************************************************************************************/
int EncUsacFrame(const ENCODER_DATA_TYPE  input,
                 HANDLE_BSBITBUFFER      *au,           /* buffers to hold output AccessUnits */
                 const int                nextIsIPF,    /* indicate that the next frame will be an IPF */
                 HANDLE_ENCODER           enc,
                 float                  **p_time_signal_orig
) {
  int err = 0;
  HANDLE_ENCODER_DATA data = enc->data;

  unsigned char extensionData[USAC_MAX_EXTENSION_PAYLOADS][USAC_MAX_EXTENSION_PAYLOAD_LEN] = {{ 0 }};
  int extensionDataSize[USAC_MAX_EXTENSION_PAYLOADS] = { 0 };
  int extensionDataPresent[USAC_MAX_EXTENSION_PAYLOADS] = { 0 };

  /*---FD---*/
  WINDOW_SHAPE windowShape[MAX_TIME_CHANNELS];
  WINDOW_SEQUENCE windowSequence[MAX_TIME_CHANNELS];
  WINDOW_SEQUENCE next_windowSequence[MAX_TIME_CHANNELS];
  WINDOW_SEQUENCE newWindowSequence;

  const double *p_ratio[MAX_TIME_CHANNELS];
  double allowed_distortion[MAX_TIME_CHANNELS][MAX_SCFAC_BANDS];
  double p_energy[MAX_TIME_CHANNELS][MAX_SCFAC_BANDS];
  int    nr_of_sfb[MAX_TIME_CHANNELS];
  int    max_sfb[MAX_TIME_CHANNELS];
  int    sfb_width_table[MAX_TIME_CHANNELS][MAX_SCFAC_BANDS];
  int    sfb_offset[MAX_TIME_CHANNELS][MAX_SCFAC_BANDS+1];

  int i_ch, i, k;

  int core_channels = 0;
  int core_max_sfb = 0;
  int isFirstTfLayer = 1;
  int osf = 1;

  /* structures holding the output of the psychoacoustic model */
  CH_PSYCH_OUTPUT chpo_long[MAX_TIME_CHANNELS];
  CH_PSYCH_OUTPUT chpo_short[MAX_TIME_CHANNELS][MAX_SHORT_WINDOWS];

  /* AAC window grouping information */
  int num_window_groups[MAX_TIME_CHANNELS] = {0};
  int window_group_length[MAX_TIME_CHANNELS][8];

  HANDLE_BSBITSTREAM* output_au;
  HANDLE_AACBITMUX aacmux;

  int lfe_chIdx = -1; /* SAMSUNG_2005-09-30 : LFE channel index */

  /*---Bitmux---*/
  int average_bits_total;
  int used_bits = 0;
  int sbr_bits = 0;
  int num_bits_available;
  int min_bits_needed;
  int bitCount;
  int padding_bits;
  int common_win[MAX_TIME_CHANNELS];
  int nb_bits_fac[MAX_TIME_CHANNELS];
  int bUsacIndependencyFlag = __getIndependencyFlag(data, -data->delay_encoder_frames);
  int AudioPreRollBits = 0;
  int   mod[MAX_TIME_CHANNELS][4] = {{0}}; /* LPD modes per frame */
  int tns_data_present[MAX_TIME_CHANNELS];
  int lFrame;
  int lLpc0;
  int lNextHighRate;
#ifdef SONY_PVC_ENC
  static int pvc_mode = 0;
#endif
  int bFacDataPresentFirst = 0;

  lFrame = data->block_size_samples;
  lLpc0 = (L_LPC0_1024 * lFrame)/L_FRAME_1024;
  lNextHighRate = (L_NEXT_HIGH_RATE_1024*lFrame)/L_FRAME_1024;

  /*-----------------------------------------------------------------------*/
  /* Initialization  */
  /*-----------------------------------------------------------------------*/
  if (data->debugLevel > 1) {
    fprintf(stderr,"EncUsacFrame entered\n");
  }

  /* pre-initialize the average number of bits available */
  average_bits_total = (data->bitrate * data->block_size_samples) / data->sampling_rate;

  /* prepare AudioPreRoll() */
  err = EncUsacAudioPreRoll_prepare(&data->aprData,
                                    nextIsIPF,
                                    &bUsacIndependencyFlag,
                                    data->extType,
                                    &data->numExtElements);
  if (0 != err) {
    return err;
  }

  /* Read extension data and adjust data rate */
  for (i = 0; i < data->numExtElements; ++i) {
    switch (data->extType[i]) {
      case USAC_ID_EXT_ELE_AUDIOPREROLL:
        err |= EncUsacAudioPreRoll_writeExtData(&data->aprData, extensionData[i], &extensionDataSize[i], &extensionDataPresent[i]);
        AudioPreRollBits = EncUsacAudioPreRoll_bitDistribution(&data->aprData, i);
        break;
      case USAC_ID_EXT_ELE_OBJ:
        readObjectData(data, extensionData[i], &extensionDataSize[i], &extensionDataPresent[i]);
        average_bits_total -= extensionDataSize[i] * 8;
        break;
      case USAC_ID_EXT_ELE_SAOC_3D:
        readSaocData(data, extensionData[i], &extensionDataSize[i], &extensionDataPresent[i]);
        average_bits_total -= extensionDataSize[i] * 8;
        break;
      case USAC_ID_EXT_ELE_HOA:
        readHoaData(data, extensionData[i], &extensionDataSize[i], &extensionDataPresent[i]);
        average_bits_total -= extensionDataSize[i] * 8;
        break;
      case USAC_ID_EXT_TYPE_PRODUCTION_METADATA:
        readPMFData (data, extensionData[i], &extensionDataSize[i], &extensionDataPresent[i]);
        average_bits_total -= extensionDataSize[i] * 8;
        break;
#if IAR
      case USAC_ID_EXT_ELE_FMT_CNVTR:
        readIARData (data, extensionData[i], &extensionDataSize[i], &extensionDataPresent[i]);
        average_bits_total -= extensionDataSize[i] * 8;
#endif
      case USAC_ID_EXT_ELE_MC:
        /* default payload */
        extensionData[i][0]        = '\0';
        extensionDataSize[i]       = 0;
        extensionDataPresent[i]    = 1;
        data->mctData.extElemIndex = i;
        average_bits_total -= extensionDataSize[i] * 8;
      default:
        break;
    }

    if (1 == extensionDataPresent[i]) {
      if (USAC_ID_EXT_ELE_AUDIOPREROLL == data->extType[i]) {
        used_bits += AudioPreRollBits;
        average_bits_total -= AudioPreRollBits;
      } else {
        used_bits += extensionDataSize[i] * 8;
      }

      used_bits += 10;                        /* 1 (usacExtElementPresent) + 1 (usacExtElementUseDefaultLength) + 8 (usacExtElementPayloadLength) */

      if (extensionDataSize[i] >= 255) {
        used_bits += 16;                      /* 16 (valueAdd) */
      }
    } else {
      /* no extension data present for this frame, but registered in mpegh3daExtElementConfig() */
      used_bits += 1;                         /* 1 (usacExtElementPresent) */
    }
  }
  if (0 != err) {
    return err;
  }

  output_au = (HANDLE_BSBITSTREAM*)calloc(1, data->track_num*sizeof(HANDLE_BSBITSTREAM));
  for (i = 0; i < data->track_num; i++) {
    output_au[i] = BsOpenBufferWrite(au[i]);
    if (output_au[i] == NULL) {
      CommonWarning("EncUsacFrame: error opening output space");
      return -1;
    }
  }

  for (i = 0; i < data->layer_num; i++){
    int t  = 0;
    aacmux = NULL;
    aacmux = aacBitMux_create(&(output_au[t]),
                              data->tracks_per_layer[t],
                              data->channels,
                              data->ep_config,
                              data->debugLevel);
    t += data->tracks_per_layer[t];
  }

  /* bit budget adjustment */
  num_bits_available = (long)(average_bits_total + MIN(average_bits_total, MAX(data->available_bitreservoir_bits-0.25*data->max_bitreservoir_bits,-0.1*average_bits_total)));
  min_bits_needed    = (long)(data->available_bitreservoir_bits + 2*average_bits_total - data->max_bitreservoir_bits);
  /* 2*average_bits_total, because one full frame always has to stay in buffer and one other frame is processed each frame */
  if (min_bits_needed < 0) {
    min_bits_needed = 0;
  }

  /* Print out info*/
  if (data->debugLevel > 1) {
    fprintf(stderr, "EncUsacFrame: init done\n");
    fprintf(stderr, "UsacFrame [%d]\n", data->frameCounter);
  }

#ifdef SONY_PVC_ENC
  /* Selection of the core coding*/
  if (isFirstTfLayer) {
    if (data->debugLevel>1)
      fprintf(stderr,"EncUsacFrame: select core coding\n");
    for ( i_ch = 0 ; i_ch < data->channels ; i_ch++ ) {
      switch( data->codecMode ) {
      case USAC_SWITCHED:
        if (classfyData.coding_mode == 2) {
          data->next_coreMode[i_ch]=CORE_MODE_FD;
        }
        else {
          data->next_coreMode[i_ch]=CORE_MODE_TD;
        }
        if (data->next_coreMode[i_ch] == CORE_MODE_TD) {
          pvc_mode = 2;
        } else {
          pvc_mode = 0;
        }

        /*           if((data->frameCounter%data->switchEveryNFrames)==0) */
        /*           { */
        /*              if(data->coreMode[i_ch]==CORE_MODE_FD) */
        /*                data->next_coreMode[i_ch] = CORE_MODE_TD; */
        /*             else */
        /*               data->next_coreMode[i_ch] = CORE_MODE_FD; */
        /*           } */
        /*           else */
        /*             data->next_coreMode[i_ch] = data->coreMode[i_ch]; */
        break;
      case USAC_ONLY_FD:
        data->next_coreMode[i_ch] = CORE_MODE_FD;
        pvc_mode = 0;
        break;
      case USAC_ONLY_TD:
        data->next_coreMode[i_ch] = CORE_MODE_TD;
        pvc_mode = 2;
        break;
      default:
        CommonWarning("EncUsacFrame: unsupported codec mode %d", data->codecMode);
        return(-1);
      }
    }
  }
#endif


  /***********************************************************************/
  /* SBR encoding */
  /***********************************************************************/

  if (p_time_signal_orig != NULL) {
    for (i_ch=0;i_ch<data->ch_elm_total;i_ch++) {

      if ((data->channelElementType[i_ch] == USAC_CHANNEL_ELEMENTTYPE_SCE) || (data->channelElementType[i_ch] == USAC_CHANNEL_ELEMENTTYPE_CPE_DMX)){
        /* single_channel_element */
        sbr_bits += SbrEncodeSingleChannel_BSAC(data->chElmData[i_ch].sbrChan[data->chElmData[i_ch].startChIdx],
                                                SBR_USAC,
                                                p_time_signal_orig[data->chElmData[i_ch].startChIdx],
#ifdef	PARAMETRICSTEREO
                                                NULL,  /* PS disable */
#endif
#ifdef SONY_PVC_ENC
                                                &pvc_mode,
#endif
                                                bUsacIndependencyFlag);

      } else if (data->channelElementType[i_ch]== USAC_CHANNEL_ELEMENTTYPE_CPE_R) {
        /* Write out cpe */
        sbr_bits +=
          SbrEncodeChannelPair_BSAC(data->chElmData[i_ch-1].sbrChan[data->chElmData[i_ch-1].startChIdx],     /* Left */
                                    data->chElmData[i_ch].sbrChan[data->chElmData[i_ch].endChIdx], /* Right */
                                    SBR_USAC,
                                    p_time_signal_orig[data->chElmData[i_ch-1].startChIdx],                       /* Left */
                                    p_time_signal_orig[data->chElmData[i_ch].endChIdx], /* Right */
                                    bUsacIndependencyFlag);

      } /* if (chann...*/
    } /* for (chanNum...*/
  }


  /***********************************************************************/
  /* Copy input buffer                                                   */
  /***********************************************************************/
  /* convert float input to double, which is the internal format */
  /* store input data in look ahead buffer which may be necessary for the window switching decision and AAC pre processing */

  if (data->debugLevel>1)
    fprintf(stderr,"EncUsacFrame: copy time buffers\n");

  for (i_ch=0; i_ch<data->channels; i_ch++){
    if ( data->flag_twMdct == 1 ) {
      /* temporary fix: a linear buffer for LTP containing the whole time frame */
      for( i=0; i<osf*data->block_size_samples/2; i++ ) {
        data->twoFrame_DTimeSigBuf[i_ch][i] =
          data->twoFrame_DTimeSigBuf[i_ch][osf*data->block_size_samples + i];
        data->twoFrame_DTimeSigBuf[i_ch][osf*data->block_size_samples/2+i] =
          data->DTimeSigBuf[i_ch][i];
        data->twoFrame_DTimeSigBuf[i_ch][osf*data->block_size_samples+i] =
          data->DTimeSigBuf[i_ch][osf*data->block_size_samples/2+i];
        data->twoFrame_DTimeSigBuf[i_ch][3*osf*data->block_size_samples/2+i] =
          data->DTimeSigLookAheadBuf[i_ch][osf*data->block_size_samples/2+i];
      }
      /* last frame input data are encoded now */
      for( i=0; i<osf*data->block_size_samples/2; i++ ) {
        data->DTimeSigBuf[i_ch][i]=data->DTimeSigLookAheadBuf[i_ch][osf*data->block_size_samples/2+i];
        data->DTimeSigBuf[i_ch][osf*data->block_size_samples/2+i] = (double) input[i_ch][i];
      }
      /* new data are stored here for use in window switching decision
           and AAC pre processing */
      for( i=0; i<osf*data->block_size_samples; i++ ) {
        data->DTimeSigLookAheadBuf[i_ch][i] = (double)input[i_ch][i];
      }

    }
    else {
      for( i=0; i<osf*data->block_size_samples; i++ ) {
        /* temporary fix: a linear buffer for LTP containing the whole time frame */
        data->twoFrame_DTimeSigBuf[i_ch][i] = data->DTimeSigBuf[i_ch][i];
        data->twoFrame_DTimeSigBuf[i_ch][data->block_size_samples + i] = data->DTimeSigLookAheadBuf[i_ch][i];
        /* last frame input data are encoded now */
        data->DTimeSigBuf[i_ch][i]          = data->DTimeSigLookAheadBuf[i_ch][i];
        /* new data are stored here for use in window switching decision
	 and AAC pre processing */
        data->DTimeSigLookAheadBuf[i_ch][i] = (double)input[i_ch][i];

      }
    }

    /* Copy TD coding input samples*/
    for (i=0; i<lFrame+lNextHighRate; i++){
      data->tdBuffer[i_ch][i] = (float)(data->twoFrame_DTimeSigBuf[i_ch][i+TD_BUFFER_OFFSET]);
    }
    for (i=0; i<lFrame+lNextHighRate+lLpc0; i++){
      data->tdBuffer_past[i_ch][i] = (float)(data->twoFrame_DTimeSigBuf[i_ch][i+TD_BUFFER_OFFSET-lLpc0]);
    }
  }

  /***********************************************************************/
  /* Pre-processing                                                      */
  /***********************************************************************/
#ifndef SONY_PVC_ENC
  /* Selection of the core coding*/
  if (isFirstTfLayer) {
    if (data->debugLevel>1)
      fprintf(stderr,"EncUsacFrame: select core coding\n");
    for ( i_ch = 0 ; i_ch < data->channels ; i_ch++ ) {
      switch( data->codecMode ) {
      case USAC_SWITCHED:
        if (classfyData.coding_mode == 2) {
          data->next_coreMode[i_ch]=CORE_MODE_FD;
        }
        else {
          data->next_coreMode[i_ch]=CORE_MODE_TD;
        }

        /*           if((data->frameCounter%data->switchEveryNFrames)==0) */
        /*           { */
        /*              if(data->coreMode[i_ch]==CORE_MODE_FD) */
        /*                data->next_coreMode[i_ch] = CORE_MODE_TD; */
        /*             else */
        /*               data->next_coreMode[i_ch] = CORE_MODE_FD; */
        /*           } */
        /*           else */
        /*             data->next_coreMode[i_ch] = data->coreMode[i_ch]; */
        break;
      case USAC_ONLY_FD:
        data->next_coreMode[i_ch] = CORE_MODE_FD;
        break;
      case USAC_ONLY_TD:
        data->next_coreMode[i_ch] = CORE_MODE_TD;
        break;
      default:
        CommonWarning("EncUsacFrame: unsupported codec mode %d", data->codecMode);
        return(-1);
      }
    }
  }
#endif


  for ( i_ch = 0 ; i_ch < data->channels ; i_ch++ ) {
    data->window_size_samples[i_ch]=data->block_size_samples;
    windowSequence[i_ch] =  data->windowSequence[i_ch];
  }


  /* Selection of the window sequence*/
  if (data->debugLevel>1)
    fprintf(stderr,"EncUsacFrame: select window sequence and shape\n");

  if(data->flag_winSwitchFD){
    newWindowSequence =
        ntt_win_sw(data->DTimeSigLookAheadBuf,
                   data->channels,
                   data->sampling_rate,
                   data->block_size_samples,
                   data->windowSequence[0],
                   data->win_sw_module);
  }
  else{
    newWindowSequence =  ONLY_LONG_SEQUENCE;
  }

  for ( i_ch = 0 ; i_ch < data->channels ; i_ch++ ) {
    next_windowSequence[i_ch] = newWindowSequence;

    if (data->next_coreMode[i_ch]==CORE_MODE_TD ) {
      next_windowSequence[i_ch] = (WINDOW_SEQUENCE)EIGHT_SHORT_SEQUENCE;
    }

    if (data->coreMode[i_ch]==CORE_MODE_TD && data->next_coreMode[i_ch] != CORE_MODE_TD) {
      next_windowSequence[i_ch] = LONG_STOP_SEQUENCE;
    }

    /* Adjust to allowed Window Sequence */
    if(next_windowSequence[i_ch] == EIGHT_SHORT_SEQUENCE) {
      if(windowSequence[i_ch] == ONLY_LONG_SEQUENCE){
        windowSequence[i_ch] = LONG_START_SEQUENCE;
      }
      if(windowSequence[i_ch] == LONG_STOP_SEQUENCE) {
        windowSequence[i_ch] = STOP_START_SEQUENCE;
      }
    }

    if(next_windowSequence[i_ch] == ONLY_LONG_SEQUENCE) {
      if(windowSequence[i_ch] == EIGHT_SHORT_SEQUENCE){
        next_windowSequence[i_ch] = LONG_STOP_SEQUENCE;
      }
    }

    if (g_useSplitTransform) {
      next_windowSequence[i_ch] = STOP_START_SEQUENCE;
    }

    if (data->debugLevel>1)
      fprintf(stderr,"EncUsacFrame: Blocktype   %d \n",windowSequence[0] );

    /* Perform window shape adaptation depending on the selected mode */
    windowShape[i_ch] = data->windowShapePrev[i_ch];

    if (data->debugLevel>1)
      fprintf(stderr,"EncUsacFrame: windowshape %d \n",windowShape[i_ch] );
  }

  used_bits += 1; /* usac independency flag */
  used_bits += sbr_bits;
  if(data->usac212enable){
    for ( i_ch = 0 ; i_ch < data->channels ; i_ch++ ) {
      if ((data->channelElementType[i_ch]==USAC_CHANNEL_ELEMENTTYPE_CPE_R) || (data->channelElementType[i_ch]==USAC_CHANNEL_ELEMENTTYPE_CPE_DMX)) {
        int elemIdx = data->channelElementIndex[i_ch];
        used_bits += data->SpatialDataLength[elemIdx];
      }
    }
  }

  for ( i_ch = 0 ; i_ch < data->channels ; i_ch++ ) {
    /***********************************************************************/
    /* Core-coding                                                         */
    /***********************************************************************/
    if(data->coreMode[i_ch]==CORE_MODE_TD){
      if (data->debugLevel>1)
        fprintf(stderr,"EncUsacFrame: TD coding\n");

      if(data->prev_coreMode[i_ch]==CORE_MODE_FD){
        reset_coder_lf(data->tdenc[i_ch]);

        coder_amrwb_plus_first(&data->tdBuffer_past[i_ch][0],
                               lNextHighRate+lLpc0,
                               data->tdenc[i_ch]
        );

      }

     coder_amrwb_plus_mono(&data->tdBuffer[i_ch][lNextHighRate],
                           data->tdOutStream[i_ch],
                           data->facOutStream[i_ch],
                           &data->TD_nbbits_fac[i_ch],
                           data->tdenc[i_ch],
                           data->tdenc[i_ch]->fscale,
                           (data->prev_coreMode[i_ch]==CORE_MODE_FD),
                           &data->total_nbbits[i_ch],
                           mod[i_ch],
                           data->channelElementType[i_ch] == ID_LFE ? 0 : data->flag_noiseFilling,
                           bUsacIndependencyFlag
                           );

     /* count bits */
     used_bits += data->total_nbbits[i_ch];

     /* write FAC data */
     if( (data->prev_coreMode[i_ch]==CORE_MODE_FD) && (mod[i_ch][0] == 0) ){
       for(i=0; i<(data->TD_nbbits_fac[i_ch]+7)/8; i++){
         data->TDfacData[i_ch][i] = (unsigned char) (
                                                     (data->facOutStream[i_ch][8*i+0] & 0x1) << 7 |
                                                     (data->facOutStream[i_ch][8*i+1] & 0x1) << 6 |
                                                     (data->facOutStream[i_ch][8*i+2] & 0x1) << 5 |
                                                     (data->facOutStream[i_ch][8*i+3] & 0x1) << 4 |
                                                     (data->facOutStream[i_ch][8*i+4] & 0x1) << 3 |
                                                     (data->facOutStream[i_ch][8*i+5] & 0x1) << 2 |
                                                     (data->facOutStream[i_ch][8*i+6] & 0x1) << 1 |
                                                     (data->facOutStream[i_ch][8*i+7] & 0x1) << 0);
       }
       used_bits += data->TD_nbbits_fac[i_ch];
     }

     used_bits += LEN_ACELP_CORE_MODE_IDX;
     if ( data->flag_twMdct == 1 ) {
       for(i=0;i<osf*data->block_size_samples;i++){
         data->overlap_buffer[i_ch][i]=data->overlap_buffer[i_ch][osf*data->block_size_samples+i];
       }
       for(i=0;i<osf*data->block_size_samples;i++){
         data->overlap_buffer[i_ch][osf*data->block_size_samples+i]=data->DTimeSigBuf[i_ch][i];
       }
     }
     else {
       for(i=0;i<osf*data->block_size_samples;i++){
         data->overlap_buffer[i_ch][i]=data->DTimeSigBuf[i_ch][i];
       }
     }
    }
    else{
      float sample_pos[MAX_TIME_CHANNELS][1024*3];
      float tw_trans_len[MAX_TIME_CHANNELS][2];
      int   tw_start_stop[MAX_TIME_CHANNELS][2];

      if (data->debugLevel>1)
        fprintf(stderr,"EncUsacFrame: FD coding\n");


      /*--- time - warping ---*/

      if ( data->flag_twMdct ) {

        if (data->prev_coreMode[i_ch]==CORE_MODE_TD) {
          tw_reset(data->block_size_samples,
                   data->warp_cont_mem[i_ch],
                   data->warp_sum[i_ch]);
        }

        EncUsac_TwEncode (data->DTimeSigBuf[i_ch],
                          (data->prev_coreMode[i_ch]==CORE_MODE_TD),
                          windowSequence[i_ch],
                          &data->toolsInfo[i_ch],
                          data->warp_cont_mem[i_ch],
                          data->warp_sum[i_ch],
                          sample_pos[i_ch],
                          tw_trans_len[i_ch],
                          tw_start_stop[i_ch],
                          data->block_size_samples);
      }

      buffer2fd(data->DTimeSigBuf[i_ch],
                data->spectral_line_vector[i_ch],
                data->overlap_buffer[i_ch],
                windowSequence[i_ch],
                windowShape[i_ch],
                data->windowShapePrev[i_ch],
                data->block_size_samples,
                data->block_size_samples/NSHORT,
                OVERLAPPED_MODE,
                (data->prev_coreMode[i_ch]==CORE_MODE_TD),
                (data->next_coreMode[i_ch]==CORE_MODE_TD),
                NSHORT,
                data->flag_twMdct,
                sample_pos[i_ch],
                tw_trans_len[i_ch],
                tw_start_stop[i_ch]);


      /*---  psychoacoustic ---*/
      if (data->debugLevel>1)
        fprintf(stderr,"EncUsacFrame: psychoacoustic initialisation\n");

      EncTf_psycho_acoustic( ((data->block_size_samples == 768) ? ((double)data->sampling_rate * 4 / 3) : (double)data->sampling_rate),
                             data->channels,
                             data->block_size_samples,
                             data->window_size_samples[i_ch],
                             chpo_long, chpo_short );


      switch( windowSequence[i_ch] ) {
        case ONLY_LONG_SEQUENCE :
          memcpy( (char*)sfb_width_table[i_ch], (char*)chpo_long[i_ch].cb_width, (NSFB_LONG+1)*sizeof(int) );
          nr_of_sfb[i_ch] = chpo_long[i_ch].no_of_cb;
          p_ratio[i_ch]   = chpo_long[i_ch].p_ratio;
          break;
        case LONG_START_SEQUENCE :
        case STOP_START_SEQUENCE :
          memcpy( (char*)sfb_width_table[i_ch], (char*)chpo_long[i_ch].cb_width, (NSFB_LONG+1)*sizeof(int) );
          nr_of_sfb[i_ch] = chpo_long[i_ch].no_of_cb;
          p_ratio[i_ch]   = chpo_long[i_ch].p_ratio;
          break;
        case EIGHT_SHORT_SEQUENCE :
          memcpy( (char*)sfb_width_table[i_ch], (char*)chpo_short[i_ch][0].cb_width, (NSFB_SHORT+1)*sizeof(int) );
          nr_of_sfb[i_ch] = chpo_short[i_ch][0].no_of_cb;
          p_ratio[i_ch]   = chpo_short[i_ch][0].p_ratio;
          break;
        case LONG_STOP_SEQUENCE :
          memcpy( (char*)sfb_width_table[i_ch], (char*)chpo_long[i_ch].cb_width, (NSFB_LONG+1)*sizeof(int) );
          nr_of_sfb[i_ch] = chpo_long[i_ch].no_of_cb;
          p_ratio[i_ch]   = chpo_long[i_ch].p_ratio;
          break;
        default:
          /* to prevent undefined ptr just use a meaningful value to allow for experiments with the meachanism above */
          memcpy( (char*)sfb_width_table[i_ch], (char*)chpo_long[i_ch].cb_width, (NSFB_LONG+1)*sizeof(int) );
          nr_of_sfb[i_ch]  = chpo_long[i_ch].no_of_cb;
          p_ratio[i_ch] = chpo_long[i_ch].p_ratio;
          break;
      }


      /* Set the AAC grouping information.  */
      if (windowSequence[i_ch] == EIGHT_SHORT_SEQUENCE) {
        num_window_groups[i_ch]=8;
        window_group_length[i_ch][0] = 1;
        window_group_length[i_ch][1] = 1;
        window_group_length[i_ch][2] = 1;
        window_group_length[i_ch][3] = 1;
        window_group_length[i_ch][4] = 1;
        window_group_length[i_ch][5] = 1;
        window_group_length[i_ch][6] = 1;
        window_group_length[i_ch][7] = 1;
      } else {
        num_window_groups[i_ch] = 1;
        window_group_length[i_ch][0] = 1;
      }

      /* Calculate sfb-offset table. Needed by (at least) TNS */
      sfb_offset[i_ch][0] = 0;
      k=0;
      for(i = 0; i < nr_of_sfb[i_ch]; i++ ) {
        sfb_offset[i_ch][i] = k;
        k += sfb_width_table[i_ch][i];
      }
      sfb_offset[i_ch][i] = k;

      /* Limit the bandwidth and calculate the max sfb*/
      if (data->debugLevel>3)
        fprintf(stderr,"EncUsacFrame: apply bandwidth limitation\n");
      if (data->debugLevel>5)
        fprintf(stderr,"            bandwidth limit %i Hz\n", data->bw_limit);

      if(data->enhancedNoiseFilling) {
        data->bw_limit = data->igFStopFq;
      }

      usac_bandwidth_limit_spectrum(data->spectral_line_vector[i_ch],
                                    data->spectral_line_vector[i_ch],
                                    windowSequence[i_ch],
                                    data->window_size_samples[i_ch],
                                    data->sampling_rate,
                                    data->bw_limit,
                                    sfb_offset[i_ch],
                                    nr_of_sfb[i_ch],
                                    &(max_sfb[i_ch]));

      /*--- TNS ---*/
      if (data->tns_select!=NO_TNS) {
        if (data->debugLevel>1)
          fprintf(stderr,"EncUsacFrame: TNS\n");


        if(i_ch != lfe_chIdx) /* SAMSUNG_2005-09-30 */
          TnsEncode(nr_of_sfb[i_ch],                /* Number of bands per window */
                    max_sfb[i_ch],                /* max_sfb */
                    windowSequence[i_ch],           /* block type */
                    sfb_offset[i_ch],               /* Scalefactor band offset table */
                    data->spectral_line_vector[i_ch], /* Spectral data array */
                    data->tnsInfo[i_ch]);           /* TNS info */

      }
      /* IGF */
      if(data->enhancedNoiseFilling && !data->igFAfterTnsSynth) {

        data->toolsInfo[i_ch].igFStartSfbLB = data->igFStartSfbLB;
        data->toolsInfo[i_ch].igFStopSfbLB = data->igFStopSfbLB;
#ifdef RM1_3D_BUGFIX_IGF_05
        data->toolsInfo[i_ch].igFNTilesLB = data->igFNTilesLB;
        data->toolsInfo[i_ch].igFNTilesSB = data->igFNTilesSB;
#endif
        IGF_encode(
          data->spectral_line_vector[i_ch],
          sfb_offset[i_ch],
          (windowSequence[i_ch] == EIGHT_SHORT_SEQUENCE),
          data->igFUseHighRes,
          data->igFStartSfbLB,
          data->igFStopSfbLB,
          data->toolsInfo[i_ch].igFTileIdx,
         &data->toolsInfo[i_ch].igFAllZero,
          data->toolsInfo[i_ch].igFLevel
          );

      }

      {

        int group, index, j, sfb;
        double dtmp;

        if (data->debugLevel>1)
          fprintf(stderr,"EncUsacFrame: compute allowed distorsion\n");

        data->max_sfb = nr_of_sfb[MONO_CHAN];

        index = 0;
        /* calculate the scale factor band energy in window groups  */
        for (group = 0; group < num_window_groups[i_ch]; group++) {
          for (sfb = 0; sfb < nr_of_sfb[i_ch]; sfb++)
            p_energy[i_ch][sfb + group * nr_of_sfb[i_ch]] = 0.0;
          for (i = 0; i < window_group_length[i_ch][group]; i++) {
            for (sfb = 0; sfb < nr_of_sfb[i_ch]; sfb++) {
              for (j = 0; j < sfb_width_table[i_ch][sfb]; j++) {
                dtmp = data->spectral_line_vector[i_ch][index++];
                p_energy[i_ch][sfb + group * nr_of_sfb[i_ch]] += dtmp * dtmp;

                if (data->debugLevel > 8 )
                  fprintf(stderr,"enc_tf.c: spectral_line_vector index %d; %d %d %d %d %d\n", index, i_ch, group, i, sfb, j);
              }
            }
          }
        }
        /* calculate the allowed distortion */
        /* Currently the ratios from psychoacoustic model
	   are common for all subwindows, that is, there is only
	   num_of_sfb[i_ch] elements in p_ratio[i_ch]. However, the
	   distortion has to be specified for scalefactor bands in all
	   window groups. */
        for (group = 0; group < num_window_groups[i_ch]; group++) {
          for (sfb = 0; sfb < nr_of_sfb[i_ch]; sfb++) {
            index = sfb + group * nr_of_sfb[i_ch];
            if ((10 * log10(p_energy[i_ch][index] + 1e-15)) > 120)
              allowed_distortion[i_ch][index] = p_energy[i_ch][index] * p_ratio[i_ch][sfb];
            else
              allowed_distortion[i_ch][index] = p_energy[i_ch][index] * 1.1;
          }
        }
      }
    }
  } /* end of first channel loop */

  /* common window */
  if ( (data->channels == 2) &&
       (windowShape[0] == windowShape[1]) &&
       (windowSequence[0] == windowSequence[1]) &&
       (data->coreMode[0] == data->coreMode[1]) &&
       (max_sfb[0] == max_sfb[1]) ) {
    common_win[0] = common_win[1] = 1;
  }
  else if ( data->channels > 2) {
    for (i = 0; i < data->channels; i++) {
      /* enable common window only for CPE */
      if (data->channelElementType[i] == USAC_CHANNEL_ELEMENTTYPE_CPE_L) {
        if ((windowShape[i] == windowShape[i+1]) &&
            (windowSequence[i] == windowSequence[i+1]) &&
            (data->coreMode[i] == data->coreMode[i+1]) &&
            (max_sfb[i] == max_sfb[i+1]) &&
            (g_useSplitTransform == 0) ) {
          common_win[i] = 1;
          common_win[i+1] = 1;
          i++;
        }
        else {
          common_win[i] = 0;
          common_win[i+1] = 0;
          i++;
        }
      }
      else {
        common_win[i] = 0;
      }
    }
  }
  else {
    common_win[0] = 0;
  }


  /*--- M/S selection ---*/
  if (data->channels == 2 &&
      data->coreMode[0] == CORE_MODE_FD && data->coreMode[1] == CORE_MODE_FD &&
      common_win[0] == 1) {
    select_ms(data->spectral_line_vector,
              data->block_size_samples,
              num_window_groups[0],
              window_group_length[0],
              sfb_offset[0],
              (core_channels==2)?core_max_sfb:0,
              nr_of_sfb[0],
              &data->msInfo[0],
              data->ms_select,
              data->debugLevel,
              0);
  }
  else if (data->channels > 2) {
    if (data->mctMode > -1) {
      mct_encode(data->spectral_line_vector,
              data->block_size_samples,
              windowSequence,
              num_window_groups[0],
              window_group_length[0],
              sfb_offset[0],
              nr_of_sfb[0],
              &data->mctData,
              data->debugLevel,
              0);
      mct_write_bitstream(&data->mctData, 
                          bUsacIndependencyFlag,
                          extensionData[data->mctData.extElemIndex], 
                          &extensionDataSize[data->mctData.extElemIndex],
                          &extensionDataPresent[data->mctData.extElemIndex]);
      used_bits +=extensionDataSize[data->mctData.extElemIndex] * 8;
    }
    
    for ( i_ch = 0 ; i_ch < data->channels ; i_ch++ ) {
      if ((data->channelElementType[i_ch] == USAC_CHANNEL_ELEMENTTYPE_CPE_L) && (common_win[i_ch] == 1)) {
        select_ms(data->spectral_line_vector,
              data->block_size_samples,
              num_window_groups[0],
              window_group_length[0],
              sfb_offset[0],
              core_max_sfb,
              nr_of_sfb[0],
              &data->msInfo[i_ch],
              data->ms_select,
              data->debugLevel,
              i_ch);
        i_ch++;
      }
    }
  }



  used_bits += usac_fd_encode(data->twoFrame_DTimeSigBuf,
                              data->spectral_line_vector,
                              data->reconstructed_spectrum,
                              data->tdenc,
                              data->hTfDec,
                              p_energy,
                              allowed_distortion,
                              sfb_width_table,
                              sfb_offset,
                              max_sfb,
                              nr_of_sfb,
                              num_bits_available-used_bits,
                              min_bits_needed-used_bits,
                              aacmux,
                              (data->ep_config!=-1),
                              0/*ld_mode*/,
                              windowSequence,
                              windowShape,
                              data->windowShapePrev,
                              data->aacAllowScalefacs,
                              data->window_size_samples,
                              data->channels,
                              data->sampling_rate,
                              &data->msInfo[0],
                              data->predCoef,
                              data->tnsInfo,
                              data->icsInfo,
                              data->toolsInfo,
                              data->quantInfo,
                              num_window_groups,
                              window_group_length,
                              data->bw_limit,
                              data->flag_noiseFilling,
                              data->flag_twMdct,
                              common_win,
                              data->facOutStream,
                              nb_bits_fac,
                              data->coreMode,
                              data->prev_coreMode,
                              data->next_coreMode,
                              bUsacIndependencyFlag,
                              data->debugLevel,
                              data->channelElementType,
                              data->enhancedNoiseFilling);

  for (i = 0; i < data->channels; i++) {
    tns_data_present[i] = data->tnsInfo[i] != NULL;

    if (tns_data_present[i]) {
      tns_data_present[i] = data->tnsInfo[i]->tnsDataPresent;
    }
  }

  if (data->hasBlProfileCompatibility == 1) {
    for (i_ch = 0; i_ch < data->channels; i_ch++) {
      if (data->coreMode[i_ch] != CORE_MODE_FD) {
        CommonWarning("core_more[ch] shall have the value \"0\" for Baseline Profile.");
        return -1;
      }

      if (data->toolsInfo[i_ch].common_tw != 0) {
        CommonWarning("common_tw shall have the value \"0\" for Baseline Profile.");
        return -1;
      }

      if (nb_bits_fac[i_ch] > 0) {
        CommonWarning("fac_data_present shall have the value \"0\" for Baseline Profile.");
        return -1;
      }
    }
  }

  /* count channel element bits */
  switch (data->channels) {
    case 1:
      used_bits += usac_write_sce(NULL, data->coreMode[0],
                                  tns_data_present[0]);
      break;
    case 2:
      used_bits += usac_write_cpe(NULL,
                                  data->coreMode,
                                  tns_data_present,
                                  data->predCoef[0],
                                  data->quantInfo[0].huffTable,
                                  common_win[0],
                                  data->toolsInfo[0].common_tw,
                                  data->icsInfo[0].max_sfb,
                                  data->icsInfo[0].windowSequence,
                                  data->icsInfo[0].window_shape,
                                  num_window_groups[0],
                                  window_group_length[0],
                                  data->msInfo[0].ms_mask,
                                  data->msInfo[0].ms_used,
                                  data->flag_twMdct,
                                  &(data->toolsInfo[0]),
                                  bUsacIndependencyFlag,
                                  data->hasBlProfileCompatibility);

      break;
    default:
      /*
       for (i = 0; i < data->channels; i++) {
          used_bits += usac_write_sce(NULL, data->coreMode[i],
                                      tns_data_present[i]);
       }
       */
      for (i = 0; i < data->channels; i++) {
        if (data->channelElementType[i] == USAC_CHANNEL_ELEMENTTYPE_LFE) {
          ; /* no config bits for LFE */
        }
        else if ((data->channelElementType[i] == USAC_CHANNEL_ELEMENTTYPE_SCE)  || (data->channelElementType[i] == USAC_CHANNEL_ELEMENTTYPE_CPE_DMX)) {
          used_bits += usac_write_sce(NULL, data->coreMode[i],
            tns_data_present[i]);
        }
        else if (data->channelElementType[i] == USAC_CHANNEL_ELEMENTTYPE_CPE_L) {
              used_bits += usac_write_cpe(NULL,
                                          data->coreMode,
                                          tns_data_present,
                                          data->predCoef[i],
                                          data->quantInfo[i].huffTable,
                                          common_win[i],
                                          data->toolsInfo[i].common_tw,
                                          data->icsInfo[i].max_sfb,
                                          data->icsInfo[i].windowSequence,
                                          data->icsInfo[i].window_shape,
                                          num_window_groups[i],
                                          window_group_length[i],
                                          data->msInfo[i].ms_mask,
                                          data->msInfo[i].ms_used,
                                          data->flag_twMdct,
                                          &(data->toolsInfo[i]),
                                          bUsacIndependencyFlag,
                                          data->hasBlProfileCompatibility);
        }
      }

  }




  /***********************************************************************/
  /* Write AUs                                                           */
  /***********************************************************************/
  
  bitCount = 0;

  /* usac independency flag */
  BsPutBit(output_au[0], bUsacIndependencyFlag, 1);
  bitCount++;

  /* Extension data - mpegh3daExtElement */
  bitCount += EncUsac_writeMpegh3daExtElement(output_au[0],
                                              extensionDataPresent,
                                              extensionDataSize,
                                              extensionData,
                                              data->numExtElements);


  /*Header*/
  switch (data->channels) {
    case 1:
      aacBitMux_setAssignmentScheme(aacmux, data->channels, 0);
      aacBitMux_setCurrentChannel(aacmux, 0);
      bitCount+=usac_write_sce(aacmux, data->coreMode[0],
                               tns_data_present[0]);
      break;
    case 2:
      aacBitMux_setAssignmentScheme(aacmux, data->channels, 1);
      aacBitMux_setCurrentChannel(aacmux, 0);
      bitCount+=usac_write_cpe(aacmux,
                               data->coreMode,
                               tns_data_present,
                               data->predCoef[0],
                               data->quantInfo[0].huffTable,
                               common_win[0],
                               data->toolsInfo[0].common_tw,
                               max_sfb[0],
                               windowSequence[0],
                               windowShape[0],
                               num_window_groups[0],
                               window_group_length[0],
                               data->msInfo[0].ms_mask,
                               data->msInfo[0].ms_used,
                               data->flag_twMdct,
                               &data->toolsInfo[0],
                               bUsacIndependencyFlag,
                               data->hasBlProfileCompatibility);
      break;
    default:
      break;
  }

  /*Channel stream*/
  for (i_ch=0; i_ch<data->channels; i_ch++){
    if(data->coreMode[i_ch]==CORE_MODE_TD){
      HANDLE_BSBITSTREAM bs_ACELP_CORE_MODE_INDEX;
      aacBitMux_setCurrentChannel(aacmux, 0);
      bs_ACELP_CORE_MODE_INDEX = aacBitMux_getBitstream(aacmux, TD_CORE_MODE_IDX);

      if((data->tdenc[i_ch])->mode > 7) CommonExit(-1,"\n invalid acelp_core_mode : %d", (data->tdenc[i_ch])->mode);
      BsPutBit(bs_ACELP_CORE_MODE_INDEX,(data->tdenc[i_ch])->mode,LEN_ACELP_CORE_MODE_IDX);
      bitCount += LEN_ACELP_CORE_MODE_IDX;

      for(i=0; i<data->total_nbbits[i_ch]; i++){
        BsPutBit(output_au[0], data->tdOutStream[i_ch][i], 1);
        bitCount++;
      }
      if( (data->prev_coreMode[i_ch] == CORE_MODE_FD) && (mod[i_ch][0] == 0) ){
        for(i=0; i<data->TD_nbbits_fac[i_ch]; i++){
          BsPutBit(output_au[0], data->facOutStream[i_ch][i], 1);
          bitCount++;
        }
      }
    }
    else{
      int used_bits_fd_cs = 0;

      if (data->channels > 2) {
        /*
        aacBitMux_setCurrentChannel(aacmux, i_ch);
        bitCount+=usac_write_sce(aacmux, data->coreMode[i_ch],
                                 tns_data_present[i_ch]);
                                 */

        if ( data->channelElementType[i_ch] == USAC_CHANNEL_ELEMENTTYPE_LFE)  {
          ; /* no config bits for LFE */
        }
        if ((data->channelElementType[i_ch] == USAC_CHANNEL_ELEMENTTYPE_SCE) || ( data->channelElementType[i_ch] == USAC_CHANNEL_ELEMENTTYPE_CPE_DMX))  {
          bitCount+=usac_write_sce(aacmux, data->coreMode[i_ch],
                                 tns_data_present[i_ch]);
        }
        else if ( data->channelElementType[i_ch] == USAC_CHANNEL_ELEMENTTYPE_CPE_L) {
          aacBitMux_setAssignmentScheme(aacmux, data->channels, 1);
          aacBitMux_setCurrentChannel(aacmux, i_ch);
          bitCount+=usac_write_cpe(aacmux,
                                   data->coreMode,
                                   tns_data_present,
                                   data->predCoef[i_ch],
                                   data->quantInfo[i_ch].huffTable,
                                   common_win[i_ch],
                                   data->toolsInfo[i_ch].common_tw,
                                   max_sfb[i_ch],
                                   windowSequence[i_ch],
                                   windowShape[i_ch],
                                   num_window_groups[i_ch],
                                   window_group_length[i_ch],
                                   data->msInfo[i_ch].ms_mask,
                                   data->msInfo[i_ch].ms_used,
                                   data->flag_twMdct,
                                   &data->toolsInfo[i_ch],
                                   bUsacIndependencyFlag,
                                   data->hasBlProfileCompatibility);

        }
       }

       if ( data->channelElementType[i_ch] != USAC_CHANNEL_ELEMENTTYPE_CPE_RES) {
         aacBitMux_setCurrentChannel(aacmux, i_ch);
         used_bits_fd_cs += usac_fd_cs(aacmux,
                                       windowSequence[i_ch],
                                       windowShape[i_ch],
                                       data->quantInfo[i_ch].scale_factor[0],
                                       data->quantInfo[i_ch].huffTable,
                                       max_sfb[i_ch],
                                       nr_of_sfb[i_ch],
                                       num_window_groups[i_ch],
                                       window_group_length[i_ch],
                                       NULL, /*No PNS*/
                                       &(data->icsInfo[i_ch]),
                                       &(data->toolsInfo[i_ch]),
                                       data->tnsInfo[i_ch],
                                       &(data->quantInfo[i_ch]),
                                       data->icsInfo[i_ch].common_window,
                                       data->toolsInfo[i_ch].common_tw,
                                       data->flag_twMdct,
                                       (data->channelElementType[i_ch]==USAC_CHANNEL_ELEMENTTYPE_LFE)?0:data->flag_noiseFilling,
                                       data->facOutStream[i_ch],
                                       nb_bits_fac[i_ch],
                                       bUsacIndependencyFlag,
                                       (data->channelElementType[i_ch]==USAC_CHANNEL_ELEMENTTYPE_LFE)?0:data->enhancedNoiseFilling,
                                       data->debugLevel);
        bitCount += used_bits_fd_cs;
     }
    }

    /* write SBR and USAC212 data */
    if (data->sbrenable)
    {
      int used_bits_sbr=0;
      if (( data->channelElementType[i_ch] == USAC_CHANNEL_ELEMENTTYPE_SCE) || ( data->channelElementType[i_ch] == USAC_CHANNEL_ELEMENTTYPE_CPE_DMX))  {
        used_bits_sbr = WriteSbrSCE(data->chElmData[i_ch].sbrChan[data->chElmData[i_ch].startChIdx],
                                  data->flag_harmonicBE?SBR_USAC_HARM:SBR_USAC,
                                  output_au[0],
#ifdef SONY_PVC_ENC
                                  pvc_mode,
#endif
                                  bUsacIndependencyFlag );
      }
      else if (data->channelElementType[i_ch] == USAC_CHANNEL_ELEMENTTYPE_CPE_R) { /* write after second CPE channel */
        assert(i_ch>0);
        used_bits_sbr = WriteSbrCPE(data->chElmData[i_ch-1].sbrChan[data->chElmData[i_ch-1].startChIdx], /* left */
                                    data->chElmData[i_ch].sbrChan[data->chElmData[i_ch].endChIdx],       /* right */
                                    data->flag_harmonicBE?SBR_USAC_HARM:SBR_USAC,
                                    output_au[0],
                                    bUsacIndependencyFlag);
      }

      bitCount += used_bits_sbr;

      /* write USAC212 data */
      if ((data->channelElementType[i_ch] == USAC_CHANNEL_ELEMENTTYPE_CPE_R) || (data->channelElementType[i_ch] == USAC_CHANNEL_ELEMENTTYPE_CPE_DMX)) {
        if(data->usac212enable){
          int used_bits_usac212=0;
          int elemIdx = data->channelElementIndex[i_ch];

          for(i=0;i<(int)(data->SpatialDataLength[elemIdx])-7;i+=8){
            BsPutBit(output_au[0], data->SpatialData[elemIdx][i/8], 8);
            used_bits_usac212+=8;
          }

          /* take care of remaining bits... */
          BsPutBit(output_au[0], data->SpatialData[elemIdx][i/8]>>(8-(data->SpatialDataLength[elemIdx]%8)), data->SpatialDataLength[elemIdx]%8);
          used_bits_usac212+=data->SpatialDataLength[elemIdx]%8;

          bitCount += used_bits_usac212;
        }
      }
    }
  }

#ifdef USE_FILL_ELEMENT
  if(data->bUseFillElement) {
    int used_bits_fillElem = 0;
    padding_bits = min_bits_needed - bitCount;
    used_bits_fillElem = usac_write_fillElem(output_au[0], padding_bits);
    bitCount += used_bits_fillElem;
    used_bits += used_bits_fillElem;
  }
#else
  /*Fill up*/
  padding_bits = min_bits_needed - bitCount;
  if (padding_bits>0) {
    bitCount += write_padding_bits(aacmux, padding_bits);
    used_bits += padding_bits;
  }
#endif

  /* store current AU as AudioPreRoll-AU for the next frame */
  err = EncUsacAudioPreRoll_setAU(&data->aprData,
                                  bitCount,
#ifdef _DEBUG
                                  BsBufferNumBit(au[0]),
#endif
                                  BsBufferGetDataBegin(au[0]));
  if (0 != err) {
    return err;
  }

  /* correct the number of calculated bits if an AudioPreRoll is written in the current frame */
  for (i = 0; i < data->numExtElements; ++i) {
    if (1 == extensionDataPresent[i]) {
      if (USAC_ID_EXT_ELE_AUDIOPREROLL == data->extType[i]) {
        used_bits = used_bits - AudioPreRollBits + extensionDataSize[i] * 8;
      }
    }
  }


  /*Update bit reservoir*/
  if (data->debugLevel>2) {
    fprintf(stderr,"\nEncUsacFrame: bitbuffer: having %i bit reservoir, produced: %i bits, consumed %i bits\n",
            data->available_bitreservoir_bits,
            bitCount,
            average_bits_total);
  }
  data->available_bitreservoir_bits -= bitCount;
#ifdef FIX_USE_BYTE_BITRES
  if(bitCount%8) {
    data->available_bitreservoir_bits -= 8-(bitCount%8);
  }
#endif
  data->available_bitreservoir_bits += average_bits_total;

  if ( used_bits != bitCount ) {
    fprintf(stderr, "Warning: the number of calculated and written bits are not equal!\n");
  }

  if (data->available_bitreservoir_bits > data->max_bitreservoir_bits) {
    fprintf(stderr,"WARNING: bit reservoir got too few bits\n");
    data->available_bitreservoir_bits = data->max_bitreservoir_bits;
  }

  if (data->available_bitreservoir_bits < 0) {
    fprintf(stderr,"WARNING: no bits in bit reservoir remaining: %d\n", data->available_bitreservoir_bits);
  }

  if (data->debugLevel>2) {
    fprintf(stderr,"EncUsacFrame: bitbuffer: => now %i bit reservoir\n", data->available_bitreservoir_bits);
  }

  if (data->debugLevel>2)
    fprintf(stdout,"EncUsacFrame: %d bits written\n", bitCount);

  /***********************************************************************/
  /* End of frame                                                        */
  /***********************************************************************/
  if (data->debugLevel>1)
    fprintf(stderr,"EncUsacFrame: prepare next frame\n");

  /* save the actual window shape for next frame */
  for(i_ch=0; i_ch<data->channels; i_ch++) {
    data->windowShapePrev[i_ch] = windowShape[i_ch];
    data->windowSequence[i_ch] = next_windowSequence[i_ch];
  }

  /* save the core coding mode */
  for(i_ch=0; i_ch<data->channels; i_ch++) {
    data->prev_coreMode[i_ch] = data->coreMode[i_ch];
    data->coreMode[i_ch] = data->next_coreMode[i_ch];
  }

  /* update usac independency flag */
  data->usacIndependencyFlagCnt = (data->usacIndependencyFlagCnt + 1) % data->usacIndependencyFlagInterval;

  data->frameCounter++;
  for (i=0; i<data->track_num; i++) {
    BsClose(output_au[i]);
  }
  free(output_au);

  if (aacmux!=NULL)
    aacBitMux_free(aacmux);


  if (data->debugLevel>1)
    fprintf(stderr,"EncUsacFrame: successful return\n");

  return 0;
}

int EncUsac_getUsacDelay(HANDLE_ENCODER encoderStruct) {
  int addlDelay = 0;

  if (encoderStruct->data->flag_twMdct) {
    addlDelay = 1024;
  }

  return (addlDelay);

}

int EncUsac_getUsacEncoderDelay(HANDLE_ENCODER encoderStruct) {
  int encDelaySamples = 0;

  if (1 == encoderStruct->data->sbrenable){
    encoderStruct->data->delay_encoder_frames = 0;
  }
  else {
    encoderStruct->data->delay_encoder_frames = DELAY_ENC_FRAMES;
  }

  /* Add one frame delay required by the resampler */
  if (1 == encoderStruct->data->sbrenable) {
    encoderStruct->data->delay_encoder_frames += 1;
  }

  encDelaySamples = encoderStruct->data->delay_encoder_frames * encoderStruct->data->block_size_samples;

  return (encDelaySamples);
}

int EncUsac_getusac212enable(HANDLE_ENCODER encoderStruct)
{
  return(encoderStruct->data->usac212enable);
}

HANDLE_SPATIAL_ENC EncUsac_getSpatialEnc(HANDLE_ENCODER encoderStruct, int elementIndex)
{
  return(encoderStruct->data->hSpatialEnc[elementIndex]);
}

int EncUsac_getChannelElementType(HANDLE_ENCODER encoderStruct, int ch) {
  return(encoderStruct->data->channelElementType[ch]);
}

int EncUsac_getChannelElementIndex(HANDLE_ENCODER encoderStruct, int ch) {
  return(encoderStruct->data->channelElementIndex[ch]);
}


int EncUsac_getIsQceElement(HANDLE_ENCODER encoderStruct, int elemIdx) {
  return(encoderStruct->data->isQceElement[elemIdx]);
}

int EncUsac_getFlagMpsRes(HANDLE_ENCODER encoderStruct) {
  return(encoderStruct->data->flag_mpsres);
}

COMPATIBLE_PROFILE_LEVEL_SET EncUsac_getCompatibleProfileLevelSet(HANDLE_ENCODER encoderStruct) {
  COMPATIBLE_PROFILE_LEVEL_SET compatibleProfileLevelSet;
  unsigned int i;

  compatibleProfileLevelSet.numCompatibleSets.length = 4;
  compatibleProfileLevelSet.numCompatibleSets.value = encoderStruct->data->numCompatibleSets;
  compatibleProfileLevelSet.reserved.length = 4;

  for (i = 0; i < encoderStruct->data->numCompatibleSets; i++) {
    compatibleProfileLevelSet.CompatibleSetIndication[i].length = 8;
    compatibleProfileLevelSet.CompatibleSetIndication[i].value = encoderStruct->data->CompatibleSetIndication[i];
  }
  return compatibleProfileLevelSet;
}

void EncUsac_setSpatialData(HANDLE_ENCODER encoderStruct, unsigned char *databuf, unsigned long size, int elementIndex)
{
  unsigned long int i;
  unsigned int j, currentSize;
  HANDLE_SPATIAL_ENC enc = encoderStruct->data->hSpatialEnc[elementIndex];

  enc->pnOutputBits[enc->nBitstreamBufferWrite] = size;
  for (i=0; i<(size + 7)/8; i++) {
    enc->ppBitstreamDelayBuffer[enc->nBitstreamBufferWrite][i] = databuf[i];
  }

  encoderStruct->data->SpatialDataLength[elementIndex] = enc->pnOutputBits[enc->nBitstreamBufferRead];
  currentSize = encoderStruct->data->SpatialDataLength[elementIndex];
  for (j=0; j<(currentSize + 7)/8; j++) {
    encoderStruct->data->SpatialData[elementIndex][j] = enc->ppBitstreamDelayBuffer[enc->nBitstreamBufferRead][j];
  }

  enc->nBitstreamBufferRead  = (enc->nBitstreamBufferRead  + 1)%enc->nBitstreamDelayBuffer;
  enc->nBitstreamBufferWrite = (enc->nBitstreamBufferWrite + 1)%enc->nBitstreamDelayBuffer;
}

int EncUsac_getSpatialOutputBufferDelay(HANDLE_ENCODER encoderStruct)
{
  int elementIndex;

  for (elementIndex = 0; elementIndex < USAC_MAX_ELEMENTS; elementIndex++) {
    if (encoderStruct->data->hSpatialEnc[elementIndex] != NULL) {
      return (encoderStruct->data->hSpatialEnc[elementIndex]->nOutputBufferDelay);
    }
  }

  return 0;
}


float * acelpGetZir(HANDLE_USAC_TD_ENCODER hAcelp) {
  return hAcelp->LPDmem.Txnq;
}
float * acelpGetLastLPC(HANDLE_USAC_TD_ENCODER hAcelp) {
  return hAcelp->LPDmem.Aq;
}

void acelpUpdatePastFDSynthesis(HANDLE_USAC_TD_ENCODER hAcelp, double* pPastFDSynthesis, double* pPastFDOrig, int lowpassLine) {

  int i;
  if ( hAcelp != NULL ) {
    for(i=0; i<1024/2+1+M; i++){
      hAcelp->fdSynth[i] = (float)pPastFDSynthesis[i-M];
    }

    for (i=0; i<1024/2+1+M; i++){
      hAcelp->fdOrig[i] = (float)pPastFDOrig[-M+i];
    }

    hAcelp->lowpassLine = lowpassLine;
  }

  return;
}


int acelpLastSubFrameWasAcelp(HANDLE_USAC_TD_ENCODER hAcelp) {

  int result = 0;
  if ( hAcelp != NULL ) {
    result = (hAcelp->prev_mod== 0);

  }
  return result;
}


