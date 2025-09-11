/*******************************************************************************
This software module was originally developed by

AT&T, Dolby Laboratories, Fraunhofer IIS, VoiceAge Corp.

and edited by
Yoshiaki Oikawa     (Sony Corporation),
Mitsuyuki Hatanaka  (Sony Corporation),
Ralph Sperschneider (Fraunhofer Gesellschaft IIS)
Markus Werner       (SEED / Software Development Karlsruhe)
Eugen Dodenhoeft    (Fraunhofer IIS)

-

in the course of development of ISO/IEC 23003 for reference purposes and its
performance may not have been optimized. This software module is an
implementation of one or more tools as specified by ISO/IEC 23003. ISO/IEC gives
You a royalty-free, worldwide, non-exclusive, copyright license to copy,
distribute, and make derivative works of this software module or modifications
thereof for use in implementations of ISO/IEC 23003 in products that satisfy
conformance criteria (if any). Those intending to use this software module in
products are advised that its use may infringe existing patents. ISO/IEC have no
liability for use of this software module or modifications thereof. Copyright is
not released for products that do not conform to audiovisual and image-coding
related ITU Recommendations and/or ISO/IEC International Standards.

Assurance that the originally developed software module can be used (1) in
ISO/IEC 23003 once ISO/IEC 23003 has been adopted; and (2) to develop ISO/IEC
23003:s
Fraunhofer IIS, VoiceAge Corp. grant(s) ISO/IEC all
rights necessary to include the originally developed software module or
modifications thereof in ISO/IEC 23003 and to permit ISO/IEC to offer You a
royalty-free, worldwide, non-exclusive, copyright license to copy, distribute,
and make derivative works for use in implementations of ISO/IEC 23003 in
products that satisfy conformance criteria (if any), and to the extent that such
originally developed software module or portions of it are included in ISO/IEC
23003. To the extent that Fraunhofer IIS, VoiceAge Corp.
own(s) patent rights that would be required to make, use, or sell the
originally developed software module or portions thereof included in ISO/IEC
23003 in a conforming product, Fraunhofer IIS, VoiceAge Corp.
will assure the ISO/IEC that it is (they are) willing to negotiate
licenses under reasonable and non-discriminatory terms and conditions with
applicants throughout the world. ISO/IEC gives You a free license to this
software module or modifications thereof for the sole purpose of developing
ISO/IEC 23003.

Fraunhofer IIS, VoiceAge Corp. retain full right to
modify and use the code for its (their) own purpose, assign or donate the code
to a third party and to inhibit third parties from using the code for products
that do not conform to MPEG-related ITU Recommendations and/or ISO/IEC
International Standards. This copyright notice must be included in all copies or
derivative works.

Copyright (c) ISO/IEC 2008.
$Id: decoder_usac.c,v 1.21.2.1 2012-04-19 09:15:33 frd Exp $
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "allHandles.h"
#include "tf_mainHandle.h"
#include "block.h"
#include "buffer.h"
#include "coupling.h"

#ifdef SONY_PVC
#include "../src_tf/sony_pvcprepro.h"
#endif  /* SONY_PVC */

#include "../src_tf/ct_sbrdec.h"

#include "all.h"                 /* structs */
#include "obj_descr.h"           /* structs */
#include "usac_config.h"
#include "usac_mainStruct.h"       /* structs */
#include "tns.h"                 /* structs */

#include "usac_port.h"           /* channel mapping */

#include "allVariables.h"        /* variables */
#include "usac_allVariables.h"
#include "spatial_bitstreamreader.h"

#ifdef CT_SBR
#include "ct_sbrdecoder.h"
#endif

#include "usac_multichannel.h"
#include "usac.h"
#include "dec_tf.h"

#ifdef DRC
#include "drc.h"
#endif

#include "tf_main.h"
#include "common_m4a.h"
#include "usac_port.h"
#include "buffers.h"
#include "bitstream.h"

#include "resilience.h"
#include "concealment.h"

#include "allVariables.h"
#include "obj_descr.h"

#include "extension_type.h"

#include "usac_tw_tools.h"

#include "acelp_plus.h"
#include "sac_dec_interface.h"
#include "sac_polyphase.h"

#include "dec_IGF.h"
#include "dec_HREP.h"

#ifdef PDAM3_PHASE2_TCC
#include "../src_tcc/tcc.h"
#endif

#include "../src_ic/ic_rom.h"

#define MAXNRSBRELEMENTS 64

#ifndef DEFAULT_NUM_TIMESLOTS
#define DEFAULT_NUM_TIMESLOTS (64)
#endif

#ifndef DEFAULT_NUM_QMFBANDS
#define DEFAULT_NUM_QMFBANDS  (64)
#endif

/*
---     Global variables from usac-decoder      ---
 */

int dec_debug[256];
static int* last_rstgrp_num = NULL;

/* global variables */
spatialDec* hSpatialDec[MAX_NUM_ELEMENTS] = {NULL};
extern int gUseFractionalDelayDecor;
extern char* g_mixMatrixFileName;
extern int   g_mixMatrixFileNameUsed;

static void rearrangeSampleBufferForQCE(float** p_pcm, int qceChIdx );
static int getChannelIndexFromElementIndex(USAC_ELEMENT_TYPE* self, int idx);
static int readFormatConverterMixmatrix( char* f, int fcMixMatrix[160][160] );  /* 32 ch + 128 obj */

extern int ch_assign[][100];


/* TCC DEBUG */
#ifdef TCC_DEBUG
#include "../src_frame/bitstream.h"
static HANDLE_BSBITSTREAM tccOutputBistream;
#endif


/* -------------------------------------- */
/*       module-private functions         */
/* -------------------------------------- */

static void predinit (HANDLE_AACDECODER hDec)
{
  int i, ch;

  if (hDec->pred_type == MONOPRED)
  {
    for (ch = 0; ch < Chans; ch++)
    {
      for (i = 0; i < LN2; i++)
      {
        init_pred_stat(&hDec->sp_status[ch][i],
                       PRED_ORDER,PRED_ALPHA,PRED_A,PRED_B);
      }
    }
  }
}

static int getdata (int*              tag,
                    int*              dt_cnt,
                    byte*             data_bytes,
                    HANDLE_RESILIENCE hResilience,
                    HANDLE_ESC_INSTANCE_DATA    hEscInstanceData,
                    HANDLE_BUFFER     hVm )
{
  int i, align_flag, cnt;

  if (dec_debug['V']) fprintf(stderr, "# DSE detected\n");
  *tag = GetBits ( LEN_TAG,
                   ELEMENT_INSTANCE_TAG,
                   hResilience,
                   hEscInstanceData,
                   hVm );

  align_flag = GetBits ( LEN_D_ALIGN,
                         MAX_ELEMENTS, /* not supported, no error resilient bitstream payload syntax exists */
                         hResilience,
                         hEscInstanceData,
                         hVm );

  if ((cnt = GetBits ( LEN_D_CNT,
                       MAX_ELEMENTS, /* not supported, no error resilient bitstream payload syntax exists */
                       hResilience,
                       hEscInstanceData,
                       hVm)) == (1<<LEN_D_CNT)-1)

    cnt +=  GetBits ( LEN_D_ESC,
                      MAX_ELEMENTS, /* not supported, no error resilient bitstream payload syntax exists */
                      hResilience,
                      hEscInstanceData,
                      hVm);

  *dt_cnt = cnt;
  if (dec_debug['x'])
    fprintf(stderr, "data element %d has %d bytes\n", *tag, cnt);
  if (align_flag)
    byte_align();

  for (i=0; i<cnt; i++){
    data_bytes[i] = (byte) GetBits ( LEN_BYTE,
                                     MAX_ELEMENTS, /* not supported, no error resilient bitstream payload syntax exists */
                                     hResilience,
                                     hEscInstanceData,
                                     hVm);
    if (dec_debug['X'])
      fprintf(stderr, "%6d %2x\n", i, data_bytes[i]);
  }
  return 0;
}

static int
getSbrExtensionData(
                       HANDLE_RESILIENCE        hResilience,
                       HANDLE_ESC_INSTANCE_DATA hEscInstanceData,
                       HANDLE_BUFFER            hVm
                       ,SBRBITSTREAM*           ct_sbrBitStr
                       , int                    instance
                       ,int*                    payloadLength
)
{
  int i;
  int count = 0;
  int ReadBits = 0;
  int Extension_Type = 0;
  int unalignedBits = 0;

  /* mul: workaround, to not read too many bits */
  HANDLE_BSBITBUFFER bsBitBuffer = GetRemainingBufferBits(hVm);
  HANDLE_BSBITSTREAM bsBitStream = BsOpenBufferRead(bsBitBuffer);

  /* get Byte count of the payload data */
  count = (*payloadLength) >> 3; /* divide by 8 and floor */
  if ( count > 0 ) {
    /* push sbr payload */
    if((count < MAXSBRBYTES) && (ct_sbrBitStr->NrElements < MAXNRSBRELEMENTS)) {
      ct_sbrBitStr->sbrElement[instance].ExtensionType = EXT_SBR_DATA;
      ct_sbrBitStr->sbrElement[instance].Payload       = count;

      for (i = 0 ; i < count ; i++) {
        BsGetBitChar(bsBitStream,
                     &ct_sbrBitStr->sbrElement[instance].Data[i],
                     8);
        ReadBits+=8;
      }

      /* read unaligned bits */
      unalignedBits = (*payloadLength-ReadBits);
      if ( unalignedBits > 0 && unalignedBits < 8) {
        BsGetBitChar(bsBitStream, &ct_sbrBitStr->sbrElement[instance].Data[count], unalignedBits);

        ct_sbrBitStr->sbrElement[instance].Data[count] =
          ct_sbrBitStr->sbrElement[instance].Data[count] << (8-unalignedBits);

        ReadBits += (unalignedBits);
        count++;
        ct_sbrBitStr->sbrElement[instance].Payload = count; /* incr Payload cnt */
      }
      ct_sbrBitStr->NrElements +=1;
      ct_sbrBitStr->NrElementsCore += 1;
    }

    *payloadLength -= ReadBits;
  }

  BsClose(bsBitStream);
  BsFreeBuffer(bsBitBuffer);

  return ReadBits;
}

#define USAC_EXTENSION_PAYLOAD_INTEGRATION
#ifdef USAC_EXTENSION_PAYLOAD_INTEGRATION


static unsigned int __ReadUsacExtElement(USAC_EXT_CONFIG * pUsacExtConfig,
                                         USAC_EXT_ELEMENT * pUsacExtElement,
                                         HANDLE_BUFFER hVm){
  unsigned int nBitsRead = 0;

  HANDLE_BSBITBUFFER bsBitBuffer = GetRemainingBufferBits(hVm);
  HANDLE_BSBITSTREAM bsBitStream = BsOpenBufferRead(bsBitBuffer);

  if(pUsacExtConfig){
    unsigned int i = 0;

    nBitsRead += BsRWBitWrapper(bsBitStream, &pUsacExtElement->usacExtElementPresent, 1, 0);
    if(pUsacExtElement->usacExtElementPresent){
      UINT32 usacExtElementPayloadLength = 0;

      nBitsRead += BsRWBitWrapper(bsBitStream, &pUsacExtElement->usacExtElementUseDefaultLength, 1, 0);
      if(pUsacExtElement->usacExtElementUseDefaultLength){
        usacExtElementPayloadLength = pUsacExtConfig->usacExtElementDefaultLength;
      } else {

       /* Reset size info if last frame was a stop frame */
        if ( pUsacExtElement->usacExtElementStop )
          {
            pUsacExtElement->usacExtElementPayloadLength = 0;
          }

#ifdef USAC_EXTENSION_ELEMENT_NOFIX
        nBitsRead += UsacConfig_ReadEscapedValue(bsBitStream, &pUsacExtElement->usacExtElementPayloadLength, 8, 16, 0);
#else
        nBitsRead += BsRWBitWrapper(bsBitStream, &usacExtElementPayloadLength, 8, 0);
        if(usacExtElementPayloadLength == 255){
          unsigned long valueAdd = 0;
          nBitsRead += BsRWBitWrapper(bsBitStream, &valueAdd, 16, 0);
          usacExtElementPayloadLength += (valueAdd - 2);
        }
#endif
      }

      if(usacExtElementPayloadLength > 0){
        if(pUsacExtConfig->usacExtElementPayloadFrag.value){
          nBitsRead += BsRWBitWrapper(bsBitStream, &pUsacExtElement->usacExtElementStart, 1, 0);
          nBitsRead += BsRWBitWrapper(bsBitStream, &pUsacExtElement->usacExtElementStop, 1, 0);
        } else {
          pUsacExtElement->usacExtElementStart = 1;
          pUsacExtElement->usacExtElementStop = 1;
        }

        for(i = 0; i < usacExtElementPayloadLength; i++){
          unsigned long tmp;
          nBitsRead += BsRWBitWrapper(bsBitStream, &tmp, 8, 0);
          pUsacExtElement->usacExtElementSegmentData[pUsacExtElement->usacExtElementPayloadLength + i] = (unsigned char) tmp & 0xFF;
        }

        pUsacExtElement->usacExtElementPayloadLength += usacExtElementPayloadLength;
      }
    }
    else {
      /* reset size info if no payload is received */
      pUsacExtElement->usacExtElementPayloadLength = 0;
    }



  }

  if(bsBitStream) BsClose(bsBitStream);
  if(bsBitBuffer) BsFreeBuffer(bsBitBuffer);
  SkipBits(hVm, nBitsRead);

  return nBitsRead;
}


#endif



#ifdef CT_SBR
static void
ResetSbrBitstream( SBRBITSTREAM *ct_sbrBitStr )
{

  ct_sbrBitStr->NrElements     = 0;
  ct_sbrBitStr->NrElementsCore = 0;

}


static void
sbrSetElementType( SBRBITSTREAM *ct_sbrBitStr, SBR_ELEMENT_ID Type )
{

  ct_sbrBitStr->sbrElement[ct_sbrBitStr->NrElements].ElementID = Type;

}


#endif /* CT_SBR */

static void fillSacDecUsacMps212Config(SAC_DEC_USAC_MPS212_CONFIG *pSacDecUsacMps212Config, USAC_MPS212_CONFIG *pUsacMps212Config){

  pSacDecUsacMps212Config->bsFreqRes              = pUsacMps212Config->bsFreqRes.value;
  pSacDecUsacMps212Config->bsFixedGainDMX         = pUsacMps212Config->bsFixedGainDMX.value;
  pSacDecUsacMps212Config->bsTempShapeConfig      = pUsacMps212Config->bsTempShapeConfig.value;
  pSacDecUsacMps212Config->bsDecorrConfig         = pUsacMps212Config->bsDecorrConfig.value;
  pSacDecUsacMps212Config->bsHighRateMode         = pUsacMps212Config->bsHighRateMode.value;
  pSacDecUsacMps212Config->bsPhaseCoding          = pUsacMps212Config->bsPhaseCoding.value;
  pSacDecUsacMps212Config->bsOttBandsPhasePresent = pUsacMps212Config->bsOttBandsPhasePresent.value;
  pSacDecUsacMps212Config->bsOttBandsPhase        = pUsacMps212Config->bsOttBandsPhase.value;
  pSacDecUsacMps212Config->bsResidualBands        = pUsacMps212Config->bsResidualBands.value;
  pSacDecUsacMps212Config->bsPseudoLr             = pUsacMps212Config->bsPseudoLr.value;
  pSacDecUsacMps212Config->bsEnvQuantMode         = pUsacMps212Config->bsEnvQuantMode.value;

  return;
}


/* -------------------------------------- */
/*           Init USAC-Decoder             */
/* -------------------------------------- */

HANDLE_USAC_DECODER USACDecodeInit(int samplingRate,
                                   char *usacDebugStr,
                                   const int  block_size_samples,
                                   Info ***sfbInfo,
                                   int  profile,
                                   AUDIO_SPECIFIC_CONFIG *streamConfig
) {
  int i;

  HANDLE_USAC_DECODER hDec = (HANDLE_USAC_DECODER) calloc (1,  sizeof(T_USAC_DECODER));
  int elemIdx = 0;
  unsigned int chIdx = 0;
  USAC_CONFIG *pUsacConfig = &(streamConfig->specConf.usacConfig);
  USAC_DECODER_CONFIG * pUsacDecoderConfig = &(streamConfig->specConf.usacConfig.usacDecoderConfig);
  int const nElements = pUsacDecoderConfig->numElements;
  int outputFrameLength = UsacConfig_GetOutputFrameLength(streamConfig->specConf.usacConfig.coreSbrFrameLengthIndex.value);
  int const samplingRateOrig = samplingRate;

  int format_converter_mix_matrix[160][160]= {{ 0 }};

  if (!hDec) {
    return hDec ;
  }

  hDec->blockSize                 = block_size_samples;
  hDec->mip                       = &mc_info;
  hDec->short_win_in_long         = 8;
  hDec->receiverDelayCompensation = pUsacConfig->receiverDelayCompensation.value;

#ifdef CT_SBR
  hDec->ct_sbrBitStr[0].NrElements     =  0;
  hDec->ct_sbrBitStr[0].NrElementsCore =  0;
  hDec->ct_sbrBitStr[1].NrElements     =  0;
  hDec->ct_sbrBitStr[1].NrElementsCore =  0;
#endif

  for(i=0; i<Chans; i++)
  {
    hDec->coef[i] = (float *)mal1((LN2 + LN2/8)*sizeof(*hDec->coef[0]));
    hDec->coefSave[i] = (float *)mal1((LN2 + LN2/8)*sizeof(*hDec->coefSave[0]));
    memset( hDec->coefSave[i], 0, (LN2 + LN2/8)*sizeof(*hDec->coefSave[0]));
    hDec->data[i] = (float *)mal1(LN2*sizeof(*hDec->data[0]));
    hDec->factors[i] = (short *)mal1(MAXBANDS*sizeof(*hDec->factors[0]));
    hDec->state[i] = (float *)mal1(LN*sizeof(*hDec->state[0]));	/* changed LN4 to LN 1/97 mfd */
    fltclrs(hDec->state[i], LN);
    hDec->cb_map[i] = (byte *)mal1(MAXBANDS*sizeof(*hDec->cb_map[0]));
    hDec->group[i] = (byte *)mal1(NSHORT*sizeof(unsigned char));
    hDec->tns[i] = (TNS_frame_info *)mal1(sizeof(*hDec->tns[0]));
    hDec->wnd_shape[i].prev_bk = WS_FHG;
    hDec->tw_ratio[i] = (int *) mal1(NUM_TW_NODES*sizeof(*hDec->tw_ratio[0]));

#ifndef RANDOMSIGN_NO_BUGFIX
    hDec->nfSeed[i] = 0x0;
#else /* RANDOMSIGN_NO_BUGFIX */

#ifdef UNIFIED_RANDOMSIGN
    if (i%2 == 0) {
      hDec->nfSeed[i] = 0x3039;
    } else {
      hDec->nfSeed[i] = 0x10932;
    }
#else
    hDec->nfSeed[i] = 12345;
#endif

#endif /* RANDOMSIGN_NO_BUGFIX */

    hDec->ApplyFAC[i]=0;
  }

  for(i=0; i<Winds; i++)
  {
    hDec->mask[i] = (byte *)mal1(MAXBANDS*sizeof(hDec->mask[0]));
  }

  /* set defaults */

  current_program = -1;

  /* multi-channel info is invalid so far */

  mc_info.mcInfoCorrectFlag = 0;
  mc_info.profile = profile ;
  mc_info.sampling_rate_idx = Fs_48;

  /* reset all debug options */

  for (i=0;i<256;i++)
    dec_debug[i] = 0;

  if (usacDebugStr)
  {
    for (i=0;i<(int)strlen(usacDebugStr);i++){
      dec_debug[(int)usacDebugStr[i]] = 1;
      fprintf(stderr,"   !!! USAC debug option: %c recognized.\n", usacDebugStr[i]);
    }
  }

  /* Set sampling rate */
  samplingRate = (outputFrameLength == 768) ? ((samplingRateOrig * 4) / 3) : samplingRateOrig;

  for (i = 0; i < (1 << LEN_SAMP_IDX); i++) {
    if (sampling_boundaries[i] <= samplingRate) {
      break;
    }
  }

  if (i == (1<<LEN_SAMP_IDX)) {
    CommonExit(1,"Unsupported sampling frequency %d", samplingRate);
  }

  prog_config.sampling_rate_idx = mc_info.sampling_rate_idx = hDec->samplingRateIdx = i;

  hDec->samplingRate = samplingRate;
  hDec->fscale       = (int)((double)hDec->samplingRate * (double)FSCALE_DENOM / 12800.0f);

  for (i = 0; i < Chans; i++) {
    hDec->wnd_shape[i].prev_bk = (WINDOW_SHAPE)0;
    hDec->wnd_shape[i].this_bk = (WINDOW_SHAPE)0;
  }

  huffbookinit(block_size_samples, mc_info.sampling_rate_idx);
  usac_infoinit(&usac_samp_rate_info[mc_info.sampling_rate_idx], block_size_samples);
  usac_winmap[0] = usac_win_seq_info[ONLY_LONG_SEQUENCE];
  usac_winmap[1] = usac_win_seq_info[ONLY_LONG_SEQUENCE];
  usac_winmap[2] = usac_win_seq_info[EIGHT_SHORT_SEQUENCE];
  usac_winmap[3] = usac_win_seq_info[ONLY_LONG_SEQUENCE];
  usac_winmap[4] = usac_win_seq_info[ONLY_LONG_SEQUENCE]; /* STOP_START_SEQUENCE */

  *sfbInfo = usac_winmap;

  for(elemIdx = 0; elemIdx < nElements; elemIdx++){
    USAC_CORE_CONFIG *pUsacCoreConfig = UsacConfig_GetUsacCoreConfig(pUsacConfig, elemIdx);
    USAC_ELEMENT_TYPE elemType = UsacConfig_GetUsacElementType(pUsacConfig, elemIdx);
    if (pUsacCoreConfig){
      hDec->tdconfig[elemIdx].fullbandLPD       = pUsacCoreConfig->fullbandLPD.value;
      hDec->tdconfig[elemIdx].lpdStereoIndex    = (elemType == USAC_ELEMENT_TYPE_CPE) ? pUsacDecoderConfig->usacElementConfig[elemIdx].usacCpeConfig.lpdStereoIndex.value : 0;
      hDec->tdconfig[elemIdx].bUseNoiseFilling  = pUsacCoreConfig->noiseFilling.value;
      hDec->tdconfig[elemIdx].lFrameFB          = block_size_samples;
      hDec->tdconfig[elemIdx].fscaleFB          = samplingRateOrig;
      if (hDec->tdconfig[elemIdx].fullbandLPD) {
        hDec->tdconfig[elemIdx].igf_active      = ((elemType == USAC_ELEMENT_TYPE_SCE || (elemType == USAC_ELEMENT_TYPE_CPE))) ? pUsacCoreConfig->enhancedNoiseFilling.value : 0;
        hDec->tdconfig[elemIdx].fac_FB          = 2;
        hDec->tdconfig[elemIdx].nbDiv           = NB_DIV/2;
        hDec->tdconfig[elemIdx].lFrame          = block_size_samples/2;
        hDec->tdconfig[elemIdx].lDiv            = (block_size_samples/2)/hDec->tdconfig[elemIdx].nbDiv;
        hDec->tdconfig[elemIdx].nbSubfr         = (NB_SUBFR_1024*hDec->tdconfig[elemIdx].lDiv) / L_DIV_1024;
        hDec->tdconfig[elemIdx].fscale          = (samplingRateOrig * block_size_samples/2) / L_FRAME_1024;
      } else {
        hDec->tdconfig[elemIdx].igf_active      = 0;
        hDec->tdconfig[elemIdx].fac_FB          = 1;
        hDec->tdconfig[elemIdx].nbDiv           = NB_DIV;
        hDec->tdconfig[elemIdx].lFrame          = block_size_samples;
        hDec->tdconfig[elemIdx].lDiv            = block_size_samples/hDec->tdconfig[elemIdx].nbDiv;
        hDec->tdconfig[elemIdx].nbSubfr         = (NB_SUBFR_1024*hDec->tdconfig[elemIdx].lDiv) / L_DIV_1024;
        if(outputFrameLength == 768){
          hDec->tdconfig[elemIdx].fscale        = samplingRateOrig;
        } else {
          hDec->tdconfig[elemIdx].fscale        = ((hDec->fscale) * block_size_samples) / L_FRAME_1024;
        }
      }
    }
  }

  if (g_mixMatrixFileNameUsed) {
    int err = 0;
    format_converter_mix_matrix[0][0] = -1;  /* Indicate no external mix matrix */
    err = readFormatConverterMixmatrix( g_mixMatrixFileName, format_converter_mix_matrix );
    if(err){
      CommonExit(1, "Invalid mix matrix!\n");
    }
  }

  {
    int i;
    int lfe_channels[2] = {-1, -1};
    int hrepCnt = 0;
    hDec->hrepData.useHREP = 0;
    hDec->hrepData.startChannels[0] = 0;

    for(elemIdx = 0; elemIdx < nElements; elemIdx++){
      if (pUsacDecoderConfig->usacElementType[elemIdx] == USAC_ELEMENT_TYPE_LFE) {
        if (lfe_channels[0] == -1) {
          lfe_channels[0]=getChannelIndexFromElementIndex(pUsacDecoderConfig->usacElementType,elemIdx);
        } else {
          lfe_channels[1]=getChannelIndexFromElementIndex(pUsacDecoderConfig->usacElementType,elemIdx);
        }
      }
    }

    for(elemIdx = 0; elemIdx < nElements; elemIdx++){
      if (pUsacDecoderConfig->usacElementType[elemIdx] == USAC_ELEMENT_TYPE_EXT) {
        if (pUsacDecoderConfig->usacElementConfig[elemIdx].usacExtConfig.usacExtElementType == USAC_ID_EXT_ELE_HREP) {
          const int nChannels = streamConfig->specConf.usacConfig.frameworkConfig3d.signals3D.m_numberOfSignals[0]+1;
          int startChannel=0, stopChannel=0;
          hDec->hrepData.useHREP = 1;

          /* obtain start channel of the signal group */
          for (i=0;i<hrepCnt;i++) {
            startChannel += streamConfig->specConf.usacConfig.frameworkConfig3d.signals3D.m_numberOfSignals[i]+1;
          }
          stopChannel = startChannel + streamConfig->specConf.usacConfig.frameworkConfig3d.signals3D.m_numberOfSignals[i]+1;

          hrepConfigInit(pUsacDecoderConfig->usacElementConfig[elemIdx].usacExtConfig.usacExtElementConfigPayload,
                         pUsacDecoderConfig->usacElementConfig[elemIdx].usacExtConfig.usacExtElementConfigLength,
                         &hDec->hrepData.hrepGroupData[hrepCnt],
                         startChannel,
                         stopChannel,
                         lfe_channels);

          hrepCnt++;
          hDec->hrepData.startChannels[hrepCnt] = stopChannel;
        }
      }
    }
  }

  if (1) {
    int err = 0,i,j,k;
    int mctCnt=0;
    int chCnt=0;
    int sigGroup=0;
    for (elemIdx = 0; elemIdx < nElements; elemIdx++) {
      switch(pUsacDecoderConfig->usacElementType[elemIdx]) {
        case USAC_ELEMENT_TYPE_SCE:
        case USAC_ELEMENT_TYPE_LFE:
          chCnt++;
          break;
        case USAC_ELEMENT_TYPE_CPE:
          chCnt+=2;
          break;
        default: break;
      }
      k=0;
      for (j=0;j<MAX_NUM_SIGNALGROUPS;j++) {
        /* obtain the corresponding signal group of the current element */
        k += streamConfig->specConf.usacConfig.frameworkConfig3d.signals3D.m_numberOfSignals[j]+1;
        if(k > chCnt) {
          sigGroup = j;
          break;
        }
      }
      if (pUsacDecoderConfig->usacElementType[elemIdx] == USAC_ELEMENT_TYPE_EXT) {
        if (pUsacDecoderConfig->usacElementConfig[elemIdx].usacExtConfig.usacExtElementType == USAC_ID_EXT_ELE_MC) {
          const int nChannels = streamConfig->specConf.usacConfig.frameworkConfig3d.signals3D.m_numberOfSignals[sigGroup]+1;
          int startChannel=0;

          /* obtain start channel of the signal group */
          for (i = 0; i < sigGroup; i++) {
            startChannel += streamConfig->specConf.usacConfig.frameworkConfig3d.signals3D.m_numberOfSignals[i]+1;
          }

          err = usac_multichannel_init(&hDec->mcData[mctCnt],
                                       pUsacDecoderConfig->usacElementConfig[elemIdx].usacExtConfig.usacExtElementConfigPayload,
                                       pUsacDecoderConfig->usacElementConfig[elemIdx].usacExtConfig.usacExtElementConfigLength,
                                       nChannels, startChannel, elemIdx);

          mctCnt++;
          /*break;*/
        }
      }
    }
    if (err){
      CommonExit(1, "Invalid multichannel initialization!\n");
    }
  }

  for (i = 0 ; i < Chans ; i++) {
    /* init LTPF for FD-core */
    usac_ltpf_init(&hDec->ltpfData[i],
                   hDec->samplingRate,
                   block_size_samples);
  }

  for (elemIdx = 0; elemIdx < nElements; elemIdx++) {
    USAC_CORE_CONFIG *pUsacCoreConfig = UsacConfig_GetUsacCoreConfig(pUsacConfig, elemIdx);
    USAC_SBR_CONFIG  *pUsacSbrConfig  = UsacConfig_GetUsacSbrConfig(pUsacConfig, elemIdx);
    USAC_ELEMENT_TYPE elemType = UsacConfig_GetUsacElementType(pUsacConfig, elemIdx);
    int stereoConfigIndex = UsacConfig_GetStereoConfigIndex(pUsacConfig, elemIdx);

    if (pUsacCoreConfig){
      hDec->twMdct[elemIdx] = pUsacCoreConfig->tw_mdct.value;
      if (hDec->twMdct[elemIdx]) {
         tw_init();
      }

      /* fulbbandLPD */
      switch(elemType){
        case USAC_ELEMENT_TYPE_SCE:
        case USAC_ELEMENT_TYPE_CPE:
          hDec->fullbandLPD[elemIdx] = pUsacCoreConfig->fullbandLPD.value;
        break;
        default:
          hDec->fullbandLPD[elemIdx] = 0;
        break;
      }

      /* Noise Filling */
      hDec->bUseNoiseFilling[elemIdx] = pUsacCoreConfig->noiseFilling.value;

      /* Enhanced Noise Filling */
      IGF_init(&(hDec->igfDecData[chIdx]),
               (elemType == USAC_ELEMENT_TYPE_CPE) ? &(hDec->igfDecData[chIdx + 1]) : NULL,
               &hDec->igfConfig[elemIdx],
               ((elemType == USAC_ELEMENT_TYPE_SCE || (elemType == USAC_ELEMENT_TYPE_CPE))) ? pUsacCoreConfig->enhancedNoiseFilling.value : 0,
               pUsacCoreConfig->igf_UseHighRes.value,
               pUsacCoreConfig->igf_UseENF.value,
               pUsacCoreConfig->igf_UseWhitening.value,
               pUsacCoreConfig->igf_AfterTnsSynth.value,
               pUsacCoreConfig->igf_IndependentTiling.value,
               (int)pUsacCoreConfig->igf_StartIdx.value,
               (int)pUsacCoreConfig->igf_StopIdx.value,
               block_size_samples,
               samplingRateOrig);
    }

    SacOpenAnaFilterbank(&hDec->filterbank[elemIdx]);

    hDec->lpdStereoIndex[elemIdx] = 0;

    switch (elemType) {
      case USAC_ELEMENT_TYPE_SCE:
        hDec->tddec[chIdx] = (USAC_TD_DECODER *)mal1(sizeof(*hDec->tddec[chIdx]));

        if (hDec->tddec[chIdx]) {

          hDec->tddec[chIdx]->igfDecData = &(hDec->igfDecData[chIdx]);

          map_decoder_lf_config(hDec->tddec[chIdx],
                               &hDec->tdconfig[elemIdx]);
          init_decoder_lf(hDec->tddec[chIdx]);
        }

        /* Noise Filling */
        hDec->tddec[chIdx]->seed_tcx = 0x3039;
        hDec->nfSeed[chIdx] = 0x3039;
        chIdx++;
        break;

      case USAC_ELEMENT_TYPE_CPE:
        for (i = 0; i < 2; i++) {
          hDec->tddec[chIdx + i] = (USAC_TD_DECODER *)mal1(sizeof(*hDec->tddec[chIdx + i]));

          if (hDec->tddec[chIdx + i]) {

            hDec->tddec[chIdx + i]->igfDecData = &(hDec->igfDecData[chIdx + i]);

            map_decoder_lf_config(hDec->tddec[chIdx + i],
                                 &hDec->tdconfig[elemIdx]);
            init_decoder_lf(hDec->tddec[chIdx + i]);
          }
        }

        /* Noise Filling */
        hDec->tddec[chIdx]->seed_tcx = 0x3039;
        hDec->nfSeed[chIdx] = 0x3039;
        hDec->tddec[chIdx+1]->seed_tcx = 0x10932;
        hDec->nfSeed[chIdx+1] = 0x10932;
        chIdx += 2;

        /* LPD joint channels*/
        hDec->lpdStereoIndex[elemIdx] = pUsacDecoderConfig->usacElementConfig[elemIdx].usacCpeConfig.lpdStereoIndex.value;
        if (hDec->lpdStereoIndex[elemIdx] == 1) {
          stereoLpd_init(&(hDec->stereolpdDecData[elemIdx]),
                         hDec->samplingRate,
                         hDec->fullbandLPD[elemIdx],
                         hDec->blockSize);
        }

        /* MPS212 */
        if (stereoConfigIndex > 0) {
          int nTimeSlots = 0;
          int bsFrameLength =  UsacConfig_GetMps212NumSlots(pUsacConfig->coreSbrFrameLengthIndex.value)-1;
          int bsResidualCoding = (stereoConfigIndex > 1)?1:0;
          int nBitsRead = 0;
          const int qmfBands = 64;
          int idx1, idx2;

          SAC_DEC_USAC_MPS212_CONFIG sacDecUsacMps212Config;

          if(pUsacDecoderConfig->usacElementConfig[elemIdx].usacCpeConfig.qceFlag == 1) {/* QCE not using residual coding */
            bsResidualCoding = 0;
          }

          fillSacDecUsacMps212Config(&sacDecUsacMps212Config, &(pUsacConfig->usacDecoderConfig.usacElementConfig[elemIdx].usacCpeConfig.usacMps212Config));

          if(hSpatialDec[elemIdx]){
            SpatialDecClose(hSpatialDec[elemIdx]);
            hSpatialDec[elemIdx] = NULL;
          }

          hSpatialDec[elemIdx] = SpatialDecOpen(NULL,
                                       "",
                                       pUsacConfig->usacSamplingFrequency.value,
                                       1+((bsResidualCoding)?1:0), /* asc->channelConfiguration.value */
                                       &nTimeSlots,
                                       qmfBands,
                                       bsFrameLength,
                                       bsResidualCoding,
                                       gUseFractionalDelayDecor,
                                       0,
                                       0,
                                       1,
                                       68,
                                       &nBitsRead,
                                       &sacDecUsacMps212Config);

          if(g_mixMatrixFileNameUsed){
#ifdef WD1_ELEMENT_MAPPING_3D
            idx1 = pUsacConfig->usacDecoderConfig.usacChannelIndex[chIdx-2];   /* Get mapped output channel */
            idx2 = pUsacConfig->usacDecoderConfig.usacChannelIndex[chIdx-1];   /* Get mapped output channel */
#else
            idx1 = ch_assign[pUsacConfig->channelConfigurationIndex.value][chIdx-2];   /* Get mapped output channel */
            idx2 = ch_assign[pUsacConfig->channelConfigurationIndex.value][chIdx-1];   /* Get mapped output channel */
#endif
            if ( format_converter_mix_matrix[idx1][idx2] > 0 ){
              int dis=1;
              dis=dis;
              SpatialDecDisableDecorrelators( hSpatialDec[elemIdx], 1);
            }
          }
        }
        break;

      case USAC_ELEMENT_TYPE_LFE:
        chIdx++;
        break;
      case USAC_ELEMENT_TYPE_EXT:

#ifdef PDAM3_PHASE2_TCC
        if (pUsacDecoderConfig->usacElementConfig[elemIdx].usacExtConfig.usacExtElementType == USAC_ID_EXT_ELE_TCC) {
          HANDLE_BSBITBUFFER bitBuffer = BsAllocPlainDirtyBuffer(
                  pUsacDecoderConfig->usacElementConfig[elemIdx].usacExtConfig.usacExtElementConfigPayload,
                  pUsacDecoderConfig->usacElementConfig[elemIdx].usacExtConfig.usacExtElementConfigLength * 8 );

            HANDLE_BSBITSTREAM stream = BsOpenBufferRead ( bitBuffer );
            int idx;
            int chIdx = 0;
#ifdef TCC_DEBUG
            int i;
            printf( "FOUND TCC CONF %d\n", pUsacDecoderConfig->usacElementConfig[elemIdx].usacExtConfig.usacExtElementConfigLength );
            for( i = 0 ; i < pUsacDecoderConfig->usacElementConfig[elemIdx].usacExtConfig.usacExtElementConfigLength; i++) {
              printf( "%x\n", pUsacDecoderConfig->usacElementConfig[elemIdx].usacExtConfig.usacExtElementConfigPayload[i] );
            }

            tccOutputBistream = BsOpenFileWrite( "tccData.bit", NULL, NULL );
#endif
            TCC_CONFIG config;

            memset( &config, 0, sizeof( TCC_CONFIG ) );

            config.samplingRate = pUsacConfig->usacSamplingFrequency.value;
            config.outFrameLenght = UsacConfig_GetOutputFrameLength(pUsacConfig->coreSbrFrameLengthIndex.value);

            for ( idx = 0; idx < nElements; idx++ ) {
        USAC_ELEMENT_TYPE elem_type_local = UsacConfig_GetUsacElementType(pUsacConfig, idx);
        unsigned long affected;
        switch ( elem_type_local  ) {

        case USAC_ELEMENT_TYPE_SCE:

          BsGetBit( stream, &affected, 2 );
          if( affected == 1 ) {
          config.channelMap[ config.numAffectedChannel ] = chIdx;
          config.stereoConfigIndex[ config.numAffectedChannel ] = 0;

          config.tccMode[ config.numAffectedChannel ] = TCC_MODE_CPE_NOT_CPE;
          config.numAffectedChannel++;
          }
          chIdx++;

          break;
        case USAC_ELEMENT_TYPE_CPE:
          BsGetBit( stream, &affected, 2 );
          if(( affected == 2 ) || (affected == 1)){
          config.channelMap[ config.numAffectedChannel ]    = chIdx;
          config.channelMap[ config.numAffectedChannel + 1] = chIdx + 1;

          config.stereoConfigIndex[ config.numAffectedChannel ] =
              UsacConfig_GetStereoConfigIndex(pUsacConfig, idx);

          config.stereoConfigIndex[ config.numAffectedChannel + 1] =
              UsacConfig_GetStereoConfigIndex(pUsacConfig, idx);

          /* assign mode for element
           *
           * for CPE frame can be:
           *     a) one TCC frame for both channel
           *     b) two TCC frame for each channel separately
           * */

          config.tccMode[ config.numAffectedChannel ] = affected;
          config.tccMode[ config.numAffectedChannel + 1 ] = affected;

          config.numAffectedChannel+=2;

          }
          chIdx += 2;
          break;
        case USAC_ELEMENT_TYPE_LFE:
          chIdx++;
          break;
        default:
          break;
        }
      }

      hDec->tccAplied = 1;

      initTccDecoder( config );

      BsFreePlainDirtyBuffer( stream );
      BsClose( stream );
    }
#endif
        break;
      default:
        CommonExit(1, "Invalid usac element type: %d", elemType);
        break;
    }
  }

  return hDec ;
}

/* ------------------------------------------ */
/*  Release data allocated by AACDecoderInit  */
/* ------------------------------------------ */

void USACDecodeFree (HANDLE_USAC_DECODER hDec)
{
  int i ;
  for(i=0; i<Chans; i++)
  {

    free (hDec->state[i]);
    free (hDec->coef[i]) ;
    free (hDec->coefSave[i]) ;
    free (hDec->data[i]) ;
    free (hDec->factors[i]) ;
    free (hDec->cb_map[i]) ;
    free (hDec->group[i]) ;

    free (hDec->tns[i]) ;

    free (hDec->tw_ratio[i]);
    close_decoder_lf(hDec->tddec[i]);
    if(hDec->tddec[i]) {
      free (hDec->tddec[i]);
    }
    usac_ltpf_free(&hDec->ltpfData[i]);
  }

  if (NULL != last_rstgrp_num) {
    free (last_rstgrp_num);
  }
  for(i=0; i<Winds; i++)
    free (hDec->mask[i]) ;

  for(i = 0; i < MAX_NUM_ELEMENTS; i++){
    if(hSpatialDec[i]){
      SpatialDecClose(hSpatialDec[i]);
      hSpatialDec[i] = NULL;

    }
  }
  for(i=0; i<sizeof(hDec->filterbank)/sizeof(hDec->filterbank[0]); i++){
    if(hDec->filterbank[i]){
      SacCloseAnaFilterbank(hDec->filterbank[i]);
    }
  }

#ifdef PDAM3_PHASE2_TCC
  if( hDec->tccAplied ){
     tccClean();
  }
#ifdef TCC_DEBUG
  BsClose( tccOutputBistream );
#endif
#endif

  free (hDec) ;
}

/* -------------------------------------- */
/*           DECODE 1 frame               */
/* -------------------------------------- */

int USACDecodeFrame (USAC_DATA               *usacData,
                     HANDLE_BSBITSTREAM       fixed_stream,
                     float*                   spectral_line_vector[MAX_TIME_CHANNELS],
                     WINDOW_SEQUENCE          windowSequence[MAX_TIME_CHANNELS],
                     WINDOW_SHAPE             window_shape[MAX_TIME_CHANNELS],
                     byte                     max_sfb[Winds],
                     int                      numChannels,
                     int*                     numOutChannels,
                     Info**                   sfbInfo,
                     byte                     sfbCbMap[MAX_TIME_CHANNELS][MAXBANDS],
                     short*                   pFactors[MAX_TIME_CHANNELS],
                     HANDLE_DECODER_GENERAL   hFault,
                     QC_MOD_SELECT            qc_select,
                     FRAME_DATA*              fd,
                     float***                 qmfRealOutBuffer,
                     float***                 qmfImagOutBuffer
) {
  MPEG3DACORE_RETURN_CODE error_3da = MPEG3DACORE_OK;
  HANDLE_USAC_DECODER hDec = usacData->usacDecoder;

  byte hasmask [Winds] ;

  int           left=0, right=0, ele_id=0, wn=0, ch=0 ;
  int           i=0,sigGroup;
  int           chCnt=0;
  int           outCh=0;
  int           sfb=0;
  Info*         sfbInfoP;
  Info          sfbInfoNScale;
  unsigned char nrOfSignificantElements = 0; /* SCE (mono) and CPE (stereo) only */
  int           instance=0;

  int           numOutChannelsTotal=0;
  float**       mpsQmfBuffer[2*4] = { NULL }; /* enough memory for real + imag of a QCE */

#ifdef SONY_PVC_DEC
  int	core_mode;
#endif /* SONY_PVC_DEC */

  USAC_CONFIG *pUsacConfig = &(fd->od->ESDescriptor [fd->od->streamCount.value-1]->DecConfigDescr.audioSpecificConfig.specConf.usacConfig);


  HANDLE_BUFFER            hVm              = hFault[0].hVm;
  HANDLE_RESILIENCE        hResilience      = hFault[0].hResilience;
  HANDLE_ESC_INSTANCE_DATA hEscInstanceData = hFault[0].hEscInstanceData;
  HANDLE_CONCEALMENT       hConcealment     = hFault[0].hConcealment;
  float **sbrQmfBufferReal[2] = { NULL };
  float **sbrQmfBufferImag[2] = { NULL };

  USAC_DECODER_CONFIG * pUsacDecoderConfig = &(fd->od->ESDescriptor[fd->od->streamCount.value-1]->DecConfigDescr.audioSpecificConfig.specConf.usacConfig.usacDecoderConfig);
  int nInstances = pUsacDecoderConfig->numElements;
  int elementTag[8] = {0};
  int nQCEpart = 0; /* needed for element skipping */
  int bSkipThisQce = 0; /* needed for element skipping */
  int bSetSkippedBufferToZero = 0; /* needed for element skipping */
  int mctCnt=0;
  int lfe_channels[2] = {-1, -1};
  int hrepCnt=0;


  usacData->usacExtNumElements = 0;

  if(dec_debug['n']) {
    /*     fprintf(stderr, "\rblock %ld", bno); */
    fprintf(stderr,"\n-------\nBlock: %ld\n", bno);
  }

  /* Set the current bitStream for usac-decoder */
  setHuffdec2BitBuffer(fixed_stream);


  nrOfSignificantElements++;
  sfbInfoP = &sfbInfoNScale;

  if(dec_debug['v'])
    fprintf(stderr, "\nele_id %d\n", ele_id);

  BsSetSynElemId (ele_id) ;

  /* read usac independency flag */
  usacData->usacIndependencyFlag = GetBits( LEN_USAC_INDEP_FLAG,
                                            USAC_INDEP_FLAG,
                                            hFault[0].hResilience,
                                            hFault[0].hEscInstanceData,
                                            hFault[0].hVm );

  if (usacData->usacDecoder->hrepData.useHREP) {
    int j;
    for (i = 0; i < usacData->usacDecoder->hrepData.numHrepBlocks; i++) {
      for (j = 0; j < usacData->usacDecoder->hrepData.numHrepChannels; j++) {
        usacData->usacDecoder->hrepData.gain_idx[i][j] = 0;
      }
    }
  }

#ifdef CT_SBR
    ResetSbrBitstream( &hDec->ct_sbrBitStr[0] );
    ResetSbrBitstream( &hDec->ct_sbrBitStr[1] );
#endif

/* parse loop */
  for(instance = 0; instance < nInstances; instance++){
    int stereoConfigIndex = UsacConfig_GetStereoConfigIndex(pUsacConfig, instance);
    int numChannelsPerElement = 0;
    int nBitsReadSBR = 0;

    /* get audio syntactic element */

    chCnt=getChannelIndexFromElementIndex(pUsacDecoderConfig->usacElementType,instance);

    switch(pUsacDecoderConfig->usacElementType[instance]){
    case USAC_ELEMENT_TYPE_SCE:
      ele_id = ID_SCE;

      if ( 1 == pUsacDecoderConfig->usacElementLengthPresent ) {
        usacData->usacElementLength[instance] = GetBits( LEN_ELEMENT_LENGTH,
                                                         ELEMENT_LENGTH,
                                                         hFault[0].hResilience,
                                                         hFault[0].hEscInstanceData,
                                                         hFault[0].hVm );
      }

      numChannelsPerElement = 1;
      break;
    case USAC_ELEMENT_TYPE_CPE:
      ele_id = (stereoConfigIndex == 1) ? ID_SCE : ID_CPE;

      if ( 1 == pUsacDecoderConfig->usacElementLengthPresent ) {
        usacData->usacElementLength[instance] = GetBits( LEN_ELEMENT_LENGTH,
                                                         ELEMENT_LENGTH,
                                                         hFault[0].hResilience,
                                                         hFault[0].hEscInstanceData,
                                                         hFault[0].hVm );
      }

      numChannelsPerElement = (stereoConfigIndex == 1) ? 1 : 2;
      break;
    case USAC_ELEMENT_TYPE_LFE:
      ele_id = ID_LFE;

      if ( 1 == pUsacDecoderConfig->usacElementLengthPresent ) {
        usacData->usacElementLength[instance] = GetBits( LEN_ELEMENT_LENGTH,
                                                         ELEMENT_LENGTH,
                                                         hFault[0].hResilience,
                                                         hFault[0].hEscInstanceData,
                                                         hFault[0].hVm );
      }
      numChannelsPerElement = 1;
      break;
    case USAC_ELEMENT_TYPE_EXT:
      hDec->ct_sbrBitStr[0].NrElements +=1;
      hDec->ct_sbrBitStr[0].NrElementsCore += 1;
      ele_id = ID_FIL;
      break;
    default:
      CommonExit(1,"Unknown usac element type (decoder_usac.c)");
      break;
    }

    switch (ele_id) {
    case ID_SCE:                /* single channel */
    case ID_CPE:                /* channel pair */
    case ID_LFE:                /* low frequency enhancement */

      /* reset variables for parse and process of elements */
      usacData->tns_on_lr[instance] = 1;
      usacData->core_mode_element[instance][0] = usacData->core_mode_element[instance][1] = 0;
      usacData->max_sfb_ste_clear[instance] = SFB_NUM_MAX;
      usacData->max_spec_coefficients[instance][0] = usacData->max_spec_coefficients[instance][1] = -1;
      usacData->complex_coef[instance] = 0;
      usacData->use_prev_frame[instance] = 0;
      usacData->igf_pred_dir[instance] = 0;
      usacData->pred_dir[instance] = 0;
      usacData->common_window[instance] = 0;
      usacData->stereoConfigIndexElement[instance] = stereoConfigIndex;
      usacData->sfbInfoElement[chCnt] = *sfbInfoP;

      error_3da = parseChannelElement(ele_id,
                                      NULL,
                                      usacData,
                                      hDec->wnd,
                                      hDec->wnd_shape,
                                      hDec->cb_map,
                                      hDec->factors,
                                      hDec->group,
                                      hasmask,
                                      hDec->mask,
                                      max_sfb,
                                      hDec->tns,
                                      hDec->coef,
                                      &usacData->sfbInfoElement[chCnt],
                                      qc_select,
                                      hFault,
                                      elementTag[ele_id],
                                      hDec->bUseNoiseFilling[instance],
                                      &hDec->stereolpdDecData[instance],
                                      hDec->lpdStereoIndex[instance],
                                      &hDec->igfConfig[instance],
                                      chCnt,
                                      &usacData->stereoFilling[instance],
                                      &usacData->tns_on_lr[instance],
                                      usacData->core_mode_element[instance],
                                      &usacData->max_sfb1[instance],
                                      &usacData->max_sfb_ste[instance],
                                      &usacData->max_sfb_ste_clear[instance],
                                      usacData->max_spec_coefficients[instance],
                                      &usacData->complex_coef[instance],
                                      &usacData->use_prev_frame[instance],
                                      usacData->igf_hasmask[instance],
                                      &usacData->igf_pred_dir[instance],
                                      &usacData->igf_SfbStart[instance],
                                      &usacData->igf_SfbStop[instance],
                                      &usacData->pred_dir[instance],
                                      &usacData->common_window[instance]
#ifdef SONY_PVC_DEC                      
                                    ,&core_mode
#endif /* SONY_PVC_DEC */                
#ifndef CPLX_PRED_NOFIX                  
                                    ,&usacData->stereoConfigIndexElement[instance]
#endif /* CPLX_PRED_NOFIX */             
                                    );
      if (MPEG3DACORE_OK != error_3da) {
        CommonExit((int)error_3da, "parseChannelElement (decoder_usac.c)");
      }

      windowSequence[chCnt] = hDec->wnd[chCnt];
      window_shape[chCnt] = hDec->wnd_shape[chCnt].this_bk;
      if(numChannelsPerElement == 2){
        windowSequence[chCnt+1] = hDec->wnd[chCnt+1];
        window_shape[chCnt+1] = hDec->wnd_shape[chCnt+1].this_bk;
      }

      elementTag[ele_id]++;
#ifdef CT_SBR

      if ( ele_id == ID_SCE ) {
        sbrSetElementType( &hDec->ct_sbrBitStr[0], SBR_ID_SCE );
        sbrSetElementType( &hDec->ct_sbrBitStr[1], SBR_ID_SCE );
      }
      if ( ele_id == ID_CPE ) {
        if ( (stereoConfigIndex == 0) || (stereoConfigIndex == 3) ) {
          sbrSetElementType( &hDec->ct_sbrBitStr[0], SBR_ID_CPE );
          sbrSetElementType( &hDec->ct_sbrBitStr[1], SBR_ID_CPE );
        }
        else {
          sbrSetElementType( &hDec->ct_sbrBitStr[0], SBR_ID_SCE );
          sbrSetElementType( &hDec->ct_sbrBitStr[1], SBR_ID_SCE );
        }
      }
      if( ele_id == ID_LFE) {
        sbrSetElementType( &hDec->ct_sbrBitStr[0], SBR_ID_SCE );
        sbrSetElementType( &hDec->ct_sbrBitStr[1], SBR_ID_SCE );

        hDec->ct_sbrBitStr[0].sbrElement[hDec->ct_sbrBitStr[0].NrElements].Payload = 0;
        hDec->ct_sbrBitStr[0].NrElements++;
        hDec->ct_sbrBitStr[0].NrElementsCore++;
        /* hDec->ct_sbrBitStr[0]->sbrElement[0].ExtensionType = */
      }

      if(ele_id != ID_LFE){
      /* sbr_extension_data() */
        if(usacData->sbrRatioIndex > 0){
          unsigned int au;
          int payloadLength = 0;
          int AuLength = 0;
          int temp;
          int sbr_modeTmp = usacData->sbr_mode;

          int AacPayloadLength = BsCurrentBit(fixed_stream);
          for (au = 0; au < fd->layer[0].NoAUInBuffer; au++) {
            AuLength += (fd->layer[0].AULength[au] );
            if(AuLength>=AacPayloadLength)
              break;
          }
          payloadLength = AuLength - AacPayloadLength;

          getSbrExtensionData(hResilience,
              hEscInstanceData,
              hVm,
              &hDec->ct_sbrBitStr[0],
              instance,
              &payloadLength );

          usac_readSBR(usacData->ct_sbrDecoder,
                       &hDec->ct_sbrBitStr[0],
                       numChannelsPerElement,
                       instance,
                       &payloadLength,
#ifdef SONY_PVC_DEC
                       core_mode,
                       &sbr_modeTmp,
                       &temp,
#endif /* SONY_PVC_DEC */
                       usacData->usacIndependencyFlag);
          SkipBits(hVm, payloadLength);
        }
      }

      if (stereoConfigIndex > 0){

        int tmpChanPerElement = 0;
        int numTimeSlots = 0;
        int nBitsRead;
        HANDLE_BSBITBUFFER bsBitBuffer = GetRemainingBufferBits(hVm);
        int nBitsInBuffer = BsBufferNumBit(bsBitBuffer);
        HANDLE_BSBITSTREAM bsBitStream;
        HANDLE_BITSTREAM_READER hBitstreamReader;
        HANDLE_BYTE_READER hByteReader;

        /* bit-reader inside MPEG-Surround reads byte-wise */
        BsBufferManipulateSetNumBit((((int)(nBitsInBuffer+7)/8)*8), bsBitBuffer);

        bsBitStream = BsOpenBufferRead(bsBitBuffer);
        hBitstreamReader = BitstreamReaderOpen(bsBitStream);
        hByteReader = BitstreamReaderGetByteReader(hBitstreamReader);


        *numOutChannels = SpatialDecGetNumOutputChannels(hSpatialDec[instance]);

        numOutChannelsTotal+=*numOutChannels;

        numTimeSlots    = SpatialDecGetNumTimeSlots(hSpatialDec[instance]);

        SpatialDecResetBitstream(hSpatialDec[instance],
            hByteReader);

        SpatialDecParseFrame(hSpatialDec[instance], &nBitsRead, usacData->usacIndependencyFlag);
        SkipBits(hVm, nBitsRead);
      }


#endif
      chCnt += numChannelsPerElement;
      break;

    case ID_FIL:
      /* usacExtElement extension payload */
      {
        USAC_EXT_CONFIG * pUsacExtConfig = &pUsacDecoderConfig->usacElementConfig[instance].usacExtConfig;

        usacData->usacExtElementComplete[usacData->usacExtNumElements] = 0;

        __ReadUsacExtElement(pUsacExtConfig, &usacData->usacExtElement[usacData->usacExtNumElements], hVm);


        switch ( pUsacExtConfig->usacExtElementType )
        {
        case USAC_ID_EXT_ELE_OBJ:
        case USAC_ID_EXT_ELE_SAOC_3D:
        case USAC_ID_EXT_ELE_HOA:
        case USAC_ID_EXT_ELE_UNI_DRC:
#if IAR
        case USAC_ID_EXT_ELE_FMT_CNVTR:
#endif
        case USAC_ID_EXT_TYPE_PRODUCTION_METADATA:
        case USAC_ID_EXT_ELE_MC:
        case 128:
        case USAC_ID_EXT_ELE_HREP:
          usacData->usacExtElementType[usacData->usacExtNumElements] = (USAC_EXT_TYPE)pUsacExtConfig->usacExtElementType;
          if ( usacData->usacExtElement[usacData->usacExtNumElements].usacExtElementStop == 1 )
          {
            usacData->usacExtElementComplete[usacData->usacExtNumElements] = 1;
          }
          break;
        case USAC_ID_EXT_ELE_AUDIOPREROLL:
          usacData->usacExtElementType[usacData->usacExtNumElements] = (USAC_EXT_TYPE)pUsacExtConfig->usacExtElementType;
          break;
#ifdef PDAM3_PHASE2_TCC
        case USAC_ID_EXT_ELE_TCC:
        {
            usacData->usacExtElementType[usacData->usacExtNumElements] = (USAC_EXT_TYPE)pUsacExtConfig->usacExtElementType;
#ifdef TCC_DEBUG
          int i;
      for( i = 0; i < usacData->usacExtElement[usacData->usacExtNumElements].usacExtElementPayloadLength; i++ ) {
        BsPutBit( tccOutputBistream, usacData->usacExtElement[usacData->usacExtNumElements].usacExtElementSegmentData[i], 8 );
      }
#endif
      if( usacData->usacExtElement[usacData->usacExtNumElements].usacExtElementPresent == 1 ) {
        parseTccExtension( usacData->usacExtElement[usacData->usacExtNumElements].usacExtElementSegmentData,
                       usacData->usacExtElement[usacData->usacExtNumElements].usacExtElementPayloadLength);
      } else {
        parseTccExtension( NULL, 0 );
      }

      break;
        }
#endif
        } /* switch */

        usacData->usacExtNumElements++;
      }
      break;


    default:
      CommonExit(1, "Element not supported: %d\n", ele_id);

      break;
    }

    if ( ele_id == ID_SCE || ele_id == ID_CPE ) {
      nrOfSignificantElements++;
    }


    if(dec_debug['v']) {
      fprintf(stderr, "\nele_id %d\n", ele_id);
    }
  }

  for(i=0;i<usacData->usacExtNumElements;i++) {
    if((usacData->usacExtElementType[i] == USAC_ID_EXT_ELE_HREP) && (usacData->usacExtElement[i].usacExtElementPayloadLength != 0)) {

      for(instance = 0; instance < nInstances; instance++){
        if (pUsacDecoderConfig->usacElementType[instance] == USAC_ELEMENT_TYPE_LFE) {
          if (lfe_channels[0] == -1) {
            lfe_channels[0]=getChannelIndexFromElementIndex(pUsacDecoderConfig->usacElementType,instance);
          } else {
            lfe_channels[1]=getChannelIndexFromElementIndex(pUsacDecoderConfig->usacElementType,instance);
          }
        }
      }

      decodeHrepSideInfo(usacData->usacExtElement[i].usacExtElementSegmentData,
                         usacData->usacExtElement[i].usacExtElementPayloadLength,
                         &(usacData->usacDecoder->hrepData),
                         hrepCnt,
                         lfe_channels);

      hrepCnt++;
    }
  }

  for(i=0;i<usacData->usacExtNumElements;i++) {
    if(usacData->usacExtElementType[i] == USAC_ID_EXT_ELE_MC) {
      usac_multichannel_parse(usacData->usacDecoder->mcData[mctCnt],
                              usacData->usacExtElement[i].usacExtElementSegmentData,
                              usacData->usacExtElement[i].usacExtElementPayloadLength,
                              usacData->usacIndependencyFlag,
                              usacData->sfbInfoElement);
      mctCnt++;
    }
  }

  for(sigGroup=0; sigGroup < mctCnt; sigGroup++) {
    for(i=0;i<usacData->usacDecoder->mcData[sigGroup]->numPairs;i++) {
      int mctBandOffset = 0, mctBandsPerWindow;
      int win, group, groupwin, ch1, ch2;
      float *dmx, *res;
      int igf_Cnt;
      MULTICHANNEL_DATA *mct = usacData->usacDecoder->mcData[sigGroup];
      int win_tot = 0, zeroPrevOutSpec1, zeroPrevOutSpec2, bNoFrameMemory;

      ch1 = mct->channelMap[mct->codePairs[i][0] + mct->startChannel];
      ch2 = mct->channelMap[mct->codePairs[i][1] + mct->startChannel];
      mctBandsPerWindow = usacData->sfbInfoElement[ch1].islong ? mct->numMaskBands[i] : mct->numMaskBands[i]/8;

      if (usacData->sfbInfoElement[ch1].islong != usacData->sfbInfoElement[ch2].islong) {
        CommonExit(1, "MCT: Window sequence for current pair not equal!\n");
        return -1;
      }

      
      
      
      
      zeroPrevOutSpec1 = ((usacData->windowSequence[ch1] == EIGHT_SHORT_SEQUENCE) != (usacData->windowSequenceLast[ch1] == EIGHT_SHORT_SEQUENCE));
      zeroPrevOutSpec2 = ((usacData->windowSequence[ch2] == EIGHT_SHORT_SEQUENCE) != (usacData->windowSequenceLast[ch2] == EIGHT_SHORT_SEQUENCE));
      bNoFrameMemory   = ( usacData->usacIndependencyFlag || usacData->FrameWasTD[ch1] || usacData->FrameWasTD[ch2]);

      if (mct->bHasStereoFilling[i]) {
        /* calculate downmix for stereo filling within MCT */
        float prevDmx[LN2] = {0.0f};
        float *prevSpec1 = mct->prevOutSpec[mct->codePairs[i][0] + mct->startChannel];
        float *prevSpec2 = mct->prevOutSpec[mct->codePairs[i][1] + mct->startChannel];

        for (win = 0, group = 0; group < usacData->sfbInfoElement[ch2].num_groups; group++) {
          const int band_offset = win_tot*usacData->sfbInfoElement[ch2].sfb_per_sbk [win_tot];

          for (groupwin = 0; groupwin < usacData->sfbInfoElement[ch2].group_len[group]; groupwin++, win++) {
            usac_multichannel_get_prev_dmx(mct, /* depending on the MCT signaling type (rotation or prediction) */
                                           (zeroPrevOutSpec1 || bNoFrameMemory) ? NULL : prevSpec1,
                                           (zeroPrevOutSpec2 || bNoFrameMemory) ? NULL : prevSpec2,
                                           prevDmx + win*usacData->sfbInfoElement[ch2].bins_per_sbk[win],
                                           mctBandsPerWindow,
                                           mct->mask[i] + mctBandOffset,
                                           mct->pairCoeffQSfb[i] + mctBandOffset,
                                           usacData->sfbInfoElement[ch2].bins_per_sbk[win],
                                           i,
                                           usacData->sfbInfoElement[ch2].sfb_per_sbk[win],
                                           usacData->sfbInfoElement[ch2].sbk_sfb_top[win]);

            mctBandOffset += mctBandsPerWindow;
            prevSpec1 += usacData->sfbInfoElement[ch2].bins_per_sbk[win];
            prevSpec2 += usacData->sfbInfoElement[ch2].bins_per_sbk[win];
          }
          usac_multichannel_stereofilling_add(hDec->coef[ch2] + win_tot*usacData->sfbInfoElement[ch2].bins_per_sbk[win_tot],
                                              &prevDmx[win_tot*usacData->sfbInfoElement[ch2].bins_per_sbk[win_tot]],
                                              &hDec->scaleFactors[ch2][band_offset],
                                              usacData->sfbInfoElement[ch2].sfb_per_sbk[win_tot],
                                              usacData->sfbInfoElement[ch2].group_len[group],
                                              usacData->sfbInfoElement[ch2].bins_per_sbk[win_tot],
                                              usacData->sfbInfoElement[ch2].sbk_sfb_top[win_tot]);

          for (igf_Cnt = 0; igf_Cnt < 4; igf_Cnt++) {
            usac_multichannel_stereofilling_add(&hDec->igfDecData[ch2].igf_infSpec[2048 * igf_Cnt] + win_tot*usacData->sfbInfoElement[ch2].bins_per_sbk[win_tot],
                                                &prevDmx[win_tot*usacData->sfbInfoElement[ch2].bins_per_sbk[win_tot]],
                                                &hDec->scaleFactors[ch2][band_offset],
                                                usacData->sfbInfoElement[ch2].sfb_per_sbk[win_tot],
                                                usacData->sfbInfoElement[ch2].group_len[group],
                                                usacData->sfbInfoElement[ch2].bins_per_sbk[win_tot],
                                                usacData->sfbInfoElement[ch2].sbk_sfb_top[win_tot]);
          }
          for (sfb = 0; sfb < usacData->sfbInfoElement[ch2].sfb_per_sbk[win_tot]; sfb++) {
            hDec->scaleFactors[ch2][band_offset+sfb] = 0.0f;   /* zero factor, prevents cascaded stereo filling */
          }
          win_tot += usacData->sfbInfoElement[ch2].group_len[group];
        } /* loop over groups */

        if (usacData->sfbInfoElement[ch1].islong && usacData->sfbInfoElement[ch2].islong &&
           (usacData->usacDecoder->igfDecData[ch2].igf_bitstream[0].igf_WhiteningLevel[0] == IGF_FLAT_STRONG ||
            usacData->usacDecoder->igfDecData[ch2].igf_bitstream[0].igf_WhiteningLevel[1] == IGF_FLAT_STRONG ||
            usacData->usacDecoder->igfDecData[ch2].igf_bitstream[0].igf_WhiteningLevel[2] == IGF_FLAT_STRONG ||
            usacData->usacDecoder->igfDecData[ch2].igf_bitstream[0].igf_WhiteningLevel[3] == IGF_FLAT_STRONG)) {
          int bin;
          float enDmx = 0.0f, enRes = 0.0f;
          assert((usacData->sfbInfoElement[ch1].num_groups == 1) && (usacData->sfbInfoElement[ch2].num_groups == 1));

          /* compute spectral energies of downmix (ch1) and residual (ch2) at and above noiseFillingStartOffset */
          for (bin = (160 * usacData->usacDecoder->blockSize) / 1024; bin < (usacData->sfbInfoElement[ch1].bins_per_sbk[0] >> 1); bin++) {
            enDmx += hDec->coef[ch1][bin] * hDec->coef[ch1][bin];
            enRes += hDec->coef[ch2][bin] * hDec->coef[ch2][bin];
          }
          if (enDmx * 0.125f > enRes) { /* synchronize noise-fill seeds for strong whitening inside IGF decoder */
            usacData->usacDecoder->nfSeed[ch2] = usacData->usacDecoder->nfSeed[ch1];
          }
        }
      } /* mct->stereoFilling */

      mctBandOffset = 0;

      /* inverse MCT rotation */
      for (win = 0, group = 0; group < usacData->sfbInfoElement[ch1].num_groups; group++) {
        for (groupwin = 0; groupwin < usacData->sfbInfoElement[ch1].group_len[group]; groupwin++, win++) {
          dmx = hDec->coef[ch1] + win*usacData->sfbInfoElement[ch1].bins_per_sbk[win];
          res = hDec->coef[ch2] + win*usacData->sfbInfoElement[ch2].bins_per_sbk[win];

          usac_multichannel_process(mct,
                                    dmx,
                                    res,
                                    mct->pairCoeffQSfb[i] + mctBandOffset,
                                    mct->mask[i] + mctBandOffset,
                                    mctBandsPerWindow,
                                    usacData->sfbInfoElement[ch1].sfb_per_sbk[win],
                                    i,
                                    usacData->sfbInfoElement[ch1].sbk_sfb_top[win],
                                    usacData->sfbInfoElement[ch1].bins_per_sbk[win]);

          for (igf_Cnt = 0; igf_Cnt < 4; igf_Cnt++) {
            dmx = &hDec->igfDecData[ch1].igf_infSpec[2048 * igf_Cnt] + win*usacData->sfbInfoElement[ch1].bins_per_sbk[win];
            res = &hDec->igfDecData[ch2].igf_infSpec[2048 * igf_Cnt] + win*usacData->sfbInfoElement[ch2].bins_per_sbk[win];

            usac_multichannel_process(mct,
                                      dmx,
                                      res,
                                      mct->pairCoeffQSfb[i] + mctBandOffset,
                                      mct->mask[i] + mctBandOffset,
                                      mctBandsPerWindow,
                                      usacData->sfbInfoElement[ch1].sfb_per_sbk[win],
                                      i,
                                      usacData->sfbInfoElement[ch1].sbk_sfb_top[win],
                                      usacData->sfbInfoElement[ch1].bins_per_sbk[win]);
          }
          mctBandOffset += mctBandsPerWindow;
        }
      }
    }
  }

  for (sigGroup = 0; sigGroup < mctCnt; sigGroup++) {
    MULTICHANNEL_DATA *mct = usacData->usacDecoder->mcData[sigGroup];
    int ch;
    for (ch = 0; ch < mct->numChannelsToApply; ch++) {
      const int chIdx = mct->channelMap[ch + mct->startChannel];
      const Info sfbInfoElement = usacData->sfbInfoElement[chIdx];

      usac_multichannel_save_prev(mct,
                                  hDec->coef[chIdx],
                                  ch,
                                  sfbInfoElement.nsbk,
                                  sfbInfoElement.bins_per_sbk,
                                  0 /*zeroCoefSave*/);
    }
  }

  for (i=0;i<8;i++){
    elementTag[i] = 0;
  }
  chCnt=0;
  /* process elements */
  for(instance = 0; instance < nInstances; instance++){
    int stereoConfigIndex = UsacConfig_GetStereoConfigIndex(pUsacConfig, instance);
    int numChannelsPerElement = 0;
    int nBitsReadSBR = 0;

    /* get audio syntactic element */

    chCnt=getChannelIndexFromElementIndex(pUsacDecoderConfig->usacElementType,instance);

    switch(pUsacDecoderConfig->usacElementType[instance]){
    case USAC_ELEMENT_TYPE_SCE:
      ele_id = ID_SCE;
      numChannelsPerElement = 1;
      break;
    case USAC_ELEMENT_TYPE_CPE:
      ele_id = (stereoConfigIndex == 1) ? ID_SCE : ID_CPE;
      numChannelsPerElement = (stereoConfigIndex == 1) ? 1 : 2;
      break;
    case USAC_ELEMENT_TYPE_LFE:
      ele_id = ID_LFE;
      numChannelsPerElement = 1;
      break;
    case USAC_ELEMENT_TYPE_EXT:
      ele_id = ID_FIL;
      break;
    default:
      CommonExit(1,"Unknown usac element type (decoder_usac.c)");
      break;
    }

    switch(ele_id){
    case ID_SCE:                /* single channel */
    case ID_CPE:                /* channel pair */
    case ID_LFE:                /* low frequency enhancement */

            if (0 > processChannelElement(ele_id,
                                          NULL,
                                          usacData,
                                          hDec->wnd,
                                          hDec->wnd_shape, 
                                          hDec->cb_map,
                                          hDec->factors,
                                          hDec->group,
                                          hasmask,
                                          hDec->mask,
                                          max_sfb,
                                          hDec->tns,
                                          hDec->coef,
                                         &usacData->sfbInfoElement[chCnt],
                                          qc_select,
                                          hFault,
                                          elementTag[ele_id],
                                          hDec->bUseNoiseFilling[instance],
                                         &hDec->stereolpdDecData[instance],
                                          hDec->lpdStereoIndex[instance],
                                         &hDec->igfConfig[instance],
                                          chCnt,
                                          usacData->stereoFilling[instance],
                                          usacData->tns_on_lr[instance],
                                          usacData->core_mode_element[instance],
                                          usacData->max_sfb1[instance],
                                          usacData->max_sfb_ste[instance],
                                          usacData->max_sfb_ste_clear[instance],
                                          usacData->max_spec_coefficients[instance],
                                          usacData->complex_coef[instance],
                                          usacData->use_prev_frame[instance],
                                          usacData->igf_hasmask[instance],
                                          usacData->igf_pred_dir[instance],
                                          usacData->igf_SfbStart[instance],
                                          usacData->igf_SfbStop[instance],
                                          usacData->pred_dir[instance],
                                          usacData->common_window[instance]
#ifdef SONY_PVC_DEC
                                        ,&core_mode
#endif /* SONY_PVC_DEC */
#ifndef CPLX_PRED_NOFIX
                                         ,usacData->stereoConfigIndexElement[instance]
#endif /* CPLX_PRED_NOFIX */
                                     ) ) {
        CommonExit(1,"processChannelElement (decoder.c)");
      }
      elementTag[ele_id]++;
#ifdef CT_SBR

      if (usacData->bPseudoLr[instance] && (stereoConfigIndex > 1) ) {
        /* Pseudo L/R to Dmx/Res rotation */
        float tmp;
        const int l = chCnt;
        const int r = chCnt+1;
        for (i = 0; i < usacData->block_size_samples; i++) {
          tmp = (usacData->time_sample_vector[l][i] + usacData->time_sample_vector[r][i])/(float)sqrt(2.0f);
          usacData->time_sample_vector[r][i] =
              (usacData->time_sample_vector[l][i] - usacData->time_sample_vector[r][i])/(float)sqrt(2.0f);
          usacData->time_sample_vector[l][i] = tmp;
        }
      }

      /* convert time data from double to float */
      {
        int osf=1;
        for(i = 0; i < osf * (int)usacData->block_size_samples; i++){
          usacData->sampleBuf[chCnt][i] = usacData->time_sample_vector[chCnt][i];
          if(numChannelsPerElement == 2){
            usacData->sampleBuf[chCnt+1][i] = usacData->time_sample_vector[chCnt+1][i];
          }
        }
      }
#endif
      /**********************************************************/
      /* Functions for SBR Decoder                              */
      /**********************************************************/

      {
        SBRBITSTREAM* hCt_sbrBitStr;

        usacData->runSbr = 0;
        if(usacData->sbrRatioIndex > 0) {
          hCt_sbrBitStr = &hDec->ct_sbrBitStr[0];
          usacData->runSbr = 1;

          if ( hDec != NULL ) {
            if (hCt_sbrBitStr[0].NrElements != 0 ) {

              {
#ifdef SBR_SCALABLE
                /* just useful for scalable SBR */
                int maxSfbFreqLine = getMaxSfbFreqLine( usacData->aacDecoder );
#endif

#ifdef SONY_PVC_DEC
                int sbr_modeTmp = usacData->sbr_mode;
#endif /* SONY_PVC_DEC */

                if ( usac_applySBR(usacData->ct_sbrDecoder,
                    hCt_sbrBitStr,
                    usacData->sampleBuf,
                    usacData->sampleBuf,
                    sbrQmfBufferReal,/* qmfBufferReal[64][128] */
                    sbrQmfBufferImag,/* qmfBufferImag[64][128] */
#ifdef SBR_SCALABLE
                    maxSfbFreqLine,
#endif
                    usacData->bDownSampleSbr,
#ifdef PARAMETRICSTEREO
                    usacData->sbrEnablePS,
#endif
                    stereoConfigIndex,
                    numChannelsPerElement, instance, chCnt
                    ,0 /* core_bandwidth */
                    ,0 /* bUsedBSAC */
                    ,&nBitsReadSBR
#ifdef SONY_PVC_DEC
                    ,core_mode
                    ,&sbr_modeTmp
#endif /* SONY_PVC_DEC */
                    ,usacData->usacIndependencyFlag
                    ,usacData->usacDecoder->receiverDelayCompensation
                ) != SBRDEC_OK ) {
                  CommonExit(1,"invalid sbr bitstream\n");
                }

#ifdef SONY_PVC_DEC
                usacData->sbr_mode = sbr_modeTmp;
#endif /* SONY_PVC_DEC */

#ifdef PARAMETRICSTEREO
                if ( usacData->sbrEnablePS == 1 ){
                  *numOutChannels = 2;
                }
#endif
                /*            framecnt++; */
              }
            }
            else{
              usacData->runSbr = 0;
            }
          }/* usacDecoder != NULL */
        } /* sbrPresentFlag */
      } /* SBR processing block */

      /**********************************************************/
      /* MPEGS 212                                              */
      /**********************************************************/
      if (stereoConfigIndex > 0){

        int tmpChanPerElement = 0;
        int numTimeSlots = 0;
        int ch,ts,i;
        int qmfBands;

        if ( stereoConfigIndex == 1 ) {
          tmpChanPerElement = numChannelsPerElement + 1;
        } else
        {
          tmpChanPerElement = numChannelsPerElement;
        }


        /*           for(ch=0; ch<numChannels; ch++){ */
        for(ch=0; ch<tmpChanPerElement; ch++){
          mpsQmfBuffer[2*ch]   = sbrQmfBufferReal[ch];
          mpsQmfBuffer[2*ch+1] = sbrQmfBufferImag[ch];
        }

        qmfBands = SpatialGetQmfBands(hSpatialDec[instance]);
        if ( qmfBands != 64 ){
          float *inSamplesDeinterleaved = (float*)malloc(qmfBands*sizeof(float));
          /* for (ch=0; ch<numChannels; ch++) { */
          for (ch=0; ch<numChannelsPerElement; ch++) {
            for (ts=0; ts<numTimeSlots; ts++) {
              for (i=0; i<qmfBands; i++) {
                inSamplesDeinterleaved[i] = usacData->sampleBuf[0][numChannelsPerElement* (ts*qmfBands+i)+ch];
              }

              SacCalculateAnaFilterbank( usacData->usacDecoder->filterbank[ch],
                  inSamplesDeinterleaved,
                  mpsQmfBuffer[2*ch][ts],
                  mpsQmfBuffer[2*ch+1][ts] );
            }
          }
        }


         if(((((*pUsacConfig).usacDecoderConfig).usacElementConfig[instance]).usacCpeConfig).qceFlag==0)
         {
           int tmpch=getChannelIndexFromElementIndex(pUsacDecoderConfig->usacElementType,instance);
           numTimeSlots    = SpatialDecGetNumTimeSlots(hSpatialDec[instance]);
#ifdef RM6_INTERNAL_CHANNEL
           {
             int chA, chB;
              chA  = pUsacDecoderConfig->usacChannelIndex[tmpch];
              chB  = pUsacDecoderConfig->usacChannelIndex[tmpch+1];
              loadEQnGain4CPE ( chA, chB );
              setICConfig (instance, 0 );
            }
#endif 
           SpatialDecApplyFrame(hSpatialDec[instance], numTimeSlots, mpsQmfBuffer, &usacData->sampleBuf[tmpch], 1.0f, instance, 0, pUsacConfig->receiverDelayCompensation.value);

#ifdef RM6_INTERNAL_CHANNEL
          resetICConfig ();
#endif        
         }
      }

      if (usacData->usacDecoder->hrepData.useHREP) {
        int sigGroup = 0;
        for(ch=0; ch<numChannelsPerElement; ch++){
          while ((chCnt+ch) >= usacData->usacDecoder->hrepData.startChannels[sigGroup+1]) sigGroup++;
          decode_HREP(usacData->sampleBuf[chCnt+ch], 
            (usacData->runSbr) ? UsacConfig_GetOutputFrameLength(pUsacConfig->coreSbrFrameLengthIndex.value) : usacData->usacDecoder->blockSize,
            chCnt+ch,
            usacData->hrepInBuffer,
            &(usacData->usacDecoder->hrepData),
            sigGroup
            );
        }
      }

      /* copy sbrQmfBuffers to "mpsBuffers" in case of sce and lfe
         OR
         in case of regular CPE with no mps - when sbr is present */
      if ( (ele_id == ID_SCE || ele_id == ID_LFE) ||
           ( usacData->sbrRatioIndex > 0 && stereoConfigIndex == 0 ) ) {
        for(ch=0; ch < numChannelsPerElement; ch++){
          mpsQmfBuffer[2*ch]   = sbrQmfBufferReal[ch];
          mpsQmfBuffer[2*ch+1] = sbrQmfBufferImag[ch];
        }
      }

      if(((((*pUsacConfig).usacDecoderConfig).usacElementConfig[instance]).usacCpeConfig).qceFlag==0)
      {
        int ts                = 0;
        int channel           = 0;
        int band              = 0;
        int tmpch             = getChannelIndexFromElementIndex(pUsacDecoderConfig->usacElementType,instance);
        int tmpChanPerElement = 0;
        int numTimeSlots      = SpatialDecGetNumTimeSlots(hSpatialDec[instance]);
        int qmfBands          = SpatialGetQmfBands(hSpatialDec[instance]);

        if ( stereoConfigIndex == 1 ) {
          tmpChanPerElement = numChannelsPerElement + 1;
        }
        else {
          tmpChanPerElement = numChannelsPerElement;
        }

        /* copy relevant qmf buffers to a special qmfOutBuffer for wavIO */

        if ( mpsQmfBuffer[0] )
          {
            for ( channel = 0; channel < tmpChanPerElement; ++channel ) {
              for ( ts = 0; ts < DEFAULT_NUM_TIMESLOTS; ++ts ) {
                for ( band = 0; band < DEFAULT_NUM_QMFBANDS; ++band ) {
                  qmfRealOutBuffer[ts][tmpch][band] = mpsQmfBuffer[2*channel][ts][band] / 32768.f;
                  qmfImagOutBuffer[ts][tmpch][band] = mpsQmfBuffer[2*channel+1][ts][band] / 32768.f;
                }
              }
              tmpch++;
            }
          }
      }

      break;
    default: assert(1);
    }

    switch(ele_id){
      case ID_SCE:                /* single channel */
        chCnt+=1;
        break;
      case ID_CPE:                /* channel pair */
        chCnt+=2;
        break;
      case ID_LFE:                /* low frequency enhancement */
        chCnt+=1;
        break;
      default:
        break;
    }
  }

  instance=0;
  while(instance < nInstances) {

    if(pUsacDecoderConfig->usacElementType[instance] == ID_CPE){

      if(((((*pUsacConfig).usacDecoderConfig).usacElementConfig[instance]).usacCpeConfig).qceFlag>0){

        int numTimeSlots = 0;

        int ch=getChannelIndexFromElementIndex(pUsacDecoderConfig->usacElementType,instance);

#ifdef RM6_INTERNAL_CHANNEL
        int ch1A, ch1B, ch2A, ch2B, chCPE1, chCPE2;

        chCPE1 = getChannelIndexFromElementIndex(pUsacDecoderConfig->usacElementType, instance);
        chCPE2 = getChannelIndexFromElementIndex(pUsacDecoderConfig->usacElementType, instance+1);

        ch1A = pUsacDecoderConfig->usacChannelIndex[chCPE1];
        ch1B = pUsacDecoderConfig->usacChannelIndex[chCPE1+1];
        ch2A = pUsacDecoderConfig->usacChannelIndex[chCPE2];
        ch2B = pUsacDecoderConfig->usacChannelIndex[chCPE2+1];
#endif

        numTimeSlots = SpatialDecGetNumTimeSlots(hSpatialDec[instance]);

        sbrDecGetQmfSamplesChannelElement(0,instance,&mpsQmfBuffer[0],&mpsQmfBuffer[1]);
        sbrDecGetQmfSamplesChannelElement(0,instance+1,&mpsQmfBuffer[2],&mpsQmfBuffer[3]);

#ifdef RM6_INTERNAL_CHANNEL
        loadEQnGain4CPE ( ch1A, ch1B );
        setICConfig ( instance, 1 );
#endif

        SpatialDecApplyFrame(hSpatialDec[instance], numTimeSlots, mpsQmfBuffer, &usacData->sampleBuf[ch], 1.0f, instance, 1, pUsacConfig->receiverDelayCompensation.value);

        sbrDecGetQmfSamplesChannelElement(1,instance,&mpsQmfBuffer[4],&mpsQmfBuffer[5]);
        sbrDecGetQmfSamplesChannelElement(1,instance+1,&mpsQmfBuffer[6],&mpsQmfBuffer[7]);

#ifdef RM6_INTERNAL_CHANNEL
        loadEQnGain4CPE ( ch2A, ch2B );
        setICConfig ( instance, 2 );
#endif
        SpatialDecApplyFrame(hSpatialDec[instance+1], numTimeSlots, &(mpsQmfBuffer[4]), &usacData->sampleBuf[ch+2], 1.0f, instance+1, 1, pUsacConfig->receiverDelayCompensation.value);

        SpatialDecResortDataSbr(hSpatialDec[instance], hSpatialDec[instance+1]);

#ifdef RM6_INTERNAL_CHANNEL
        /* loadEQnGain4CPE ( ch1A, ch2A  ); */
        setQCEs ( ch1A, ch1B, ch2A, ch2B );
        setICConfig    ( instance, 1 );
        if ( ICConfig.isICON == IC_POST_STR || ICConfig.isICON == IC_PRE_STR )
        {
          sbrProcess(hSpatialDec[instance], instance, &mpsQmfBuffer[0]);
          SpatialDecPostProcessFrame(hSpatialDec[instance], &usacData->sampleBuf[ch], &mpsQmfBuffer[0], 1.0f);
        }
        else
#endif
        {
        sbrProcess(hSpatialDec[instance], instance, &mpsQmfBuffer[0]);
        sbrProcess(hSpatialDec[instance+1], instance+1, &mpsQmfBuffer[4]);

        SpatialDecPostProcessFrame(hSpatialDec[instance], &usacData->sampleBuf[ch], &mpsQmfBuffer[0], 1.0f);
        SpatialDecPostProcessFrame(hSpatialDec[instance+1], &usacData->sampleBuf[ch+2], &mpsQmfBuffer[4], 1.0f);
        }

#ifdef RM6_INTERNAL_CHANNEL
        resetICConfig ();
#endif


        SpatialDecResortQmfOutData(mpsQmfBuffer, hSpatialDec[instance]);

        rearrangeSampleBufferForQCE(usacData->sampleBuf, ch);

        {
          /* here, all qmf values are known */
          int channel           = 0;
          int ts                = 0;
          int band              = 0;
          int numChannelsPerQce = 4;
          int tmpChannel        = getChannelIndexFromElementIndex(pUsacDecoderConfig->usacElementType,instance);
          int numQmfBands       = SpatialGetQmfBands(hSpatialDec[instance]);

          /* copy relevant qmf buffers to a special qmfOutBuffer for wavIO */
          for ( channel = 0; channel < numChannelsPerQce; ++channel ) {
            for ( ts = 0; ts <  DEFAULT_NUM_TIMESLOTS; ++ts ) {
              for ( band = 0; band < DEFAULT_NUM_QMFBANDS; ++band ) {
                qmfRealOutBuffer[ts][tmpChannel][band] = mpsQmfBuffer[2*channel][ts][band]   / 32768.f;
                qmfImagOutBuffer[ts][tmpChannel][band] = mpsQmfBuffer[2*channel+1][ts][band] / 32768.f;
              }
            }
            ++tmpChannel;
          }
        }


        instance++;
      }
    }
    instance++;

 } /* mps instances / elements / channels */


#ifdef PDAM3_PHASE2_TCC
  if ( hDec->tccAplied == 1 ) {
  if( usacData->runSbr ) {
      updateOutputBuffer( usacData->sampleBuf, qmfRealOutBuffer, qmfImagOutBuffer );
  } else {
      updateOutputBuffer( usacData->sampleBuf, NULL, NULL );
  }

  }
#endif
   
  *numOutChannels=numChannels;







#ifdef CT_SBR
#ifndef SBR_SCALABLE
  if ( hDec->ct_sbrBitStr->NrElements ) {
    if ( hDec->ct_sbrBitStr->NrElements != hDec->ct_sbrBitStr->NrElementsCore ) {
      CommonExit(1,"number of SBR elements does not match number of SCE/CPEs");
    }
  }
#else
  if ( hDec->ct_sbrBitStr[0].NrElements ) {
    if ( hDec->ct_sbrBitStr[0].NrElements != hDec->ct_sbrBitStr[0].NrElementsCore ) {
      CommonExit(1,"number of SBR elements does not match number of SCE/CPEs");
    }
  }
#endif
#endif


  ConcealmentCheckClassBufferFullnessEP(GetReadBitCnt(hVm),
                                        hResilience,
                                        hEscInstanceData,
                                        hConcealment);

  if (ConcealmentGetEPprematurely(hConcealment))
    hEscInstanceData = ConcealmentGetEPprematurely(hConcealment);



  /* moved to processChannelElement():
     m/s stereo, intensity, prediction, tns
  */


  bno++;


  return GetReadBitCnt ( hVm );
}


void usacAUDecode ( int numChannels,
                    FRAME_DATA* frameData,
                    USAC_DATA* usacData,
                    HANDLE_DECODER_GENERAL hFault,
                    int *numOutChannels,
                    float*** qmfRealOutBuffer,
                    float*** qmfImagOutBuffer
) {
  int i_ch, decoded_bits = 0;
  short* pFactors[MAX_TIME_CHANNELS] = {NULL};
  int i;
  int unreadBits = 0;

  HANDLE_BSBITSTREAM fixed_stream;
  HANDLE_USAC_DECODER  hDec = usacData->usacDecoder;


  int short_win_in_long = 8;

  byte max_sfb[Winds];
  byte dummy[MAX_TIME_CHANNELS][MAXBANDS];
  QC_MOD_SELECT qc_select;

  qc_select = USAC_AC;

  for (i=0; i<MAX_TIME_CHANNELS; i++) {
    pFactors[i] = (short *)mal1(MAXBANDS*sizeof(*pFactors[0]));
  }

  for ( i_ch = 0 ; i_ch < numChannels ; i_ch++ ) {
    if ( ! hFault->hConcealment || ! ConcealmentAvailable ( 0, hFault->hConcealment ) ) {
      usacData->windowSequence[i_ch] = ONLY_LONG_SEQUENCE;
    }
  }

  fixed_stream = BsOpenBufferRead (frameData->layer[0].bitBuf);
  /* inverse Q&C */

  decoded_bits += USACDecodeFrame ( usacData,
                                    fixed_stream,
                                    usacData->spectral_line_vector,
                                    usacData->windowSequence,
                                    usacData->windowShape,
                                    max_sfb,
                                    numChannels,
                                    numOutChannels,
                                    usacData->sfbInfo,
                                    dummy,/*sfbCbMap*/
                                    pFactors, /*pFactors */
                                    hFault,
                                    qc_select,
                                    frameData,
                                    qmfRealOutBuffer,
                                    qmfImagOutBuffer
  );

  unreadBits = (int)(*((frameData->layer)[0]).AULength) - decoded_bits;

  if (unreadBits < 0) {
    fprintf(stderr, "\nWarning:\n\tRead too many bits (bitsleft %d)\n", unreadBits);
    /*CommonExit(1, "Read too many bits (bitsleft %d)\n", unreadBits);*/
  }
  if (unreadBits > 7) {
    fprintf(stderr, "\nWarning:\n\tToo many bits left after parsing (bitsleft %d)\n", unreadBits);
    /*CommonExit(1, "Too many bits left after parsing (bitsleft %d)\n", unreadBits);*/
  }

  removeAU(fixed_stream,decoded_bits,frameData,0);
  BsCloseRemove(fixed_stream,1);

  for (i=0; i<MAX_TIME_CHANNELS; i++) {
    if(pFactors[i]) free (pFactors[i]);
  }
}




static void rearrangeSampleBufferForQCE(float** p_pcm, int qceChIdx )
{
  float tmpBuffer[2048];
  int nSamplesPerChannel = 2048;
  int i = 0;

  int qce0Ch = qceChIdx;
  int qce1Ch = qceChIdx + 2;  /* QCE is channel pair */

  for ( i=0; i < nSamplesPerChannel; i++ )
    {
      /* Save second channel of first QCE */
      tmpBuffer[i] = p_pcm[(qce0Ch + 1)][i];

      /* Write first channel of second QCE into second channel of first QCE */
      p_pcm[(qce0Ch + 1)][i] = p_pcm[qce1Ch][i];

      /* Write saved channel into first channel of second QCE */
      p_pcm[qce1Ch][i] =  tmpBuffer[i];
    }
}


static int getChannelIndexFromElementIndex(USAC_ELEMENT_TYPE* self, int idx)
{
  int eleIdx = 0;
  int chIndx = 0;


  for(eleIdx = 0; eleIdx < idx; eleIdx++)
    {
      switch (self[eleIdx] )
        {
        case USAC_ELEMENT_TYPE_SCE:
        case USAC_ELEMENT_TYPE_LFE:
          chIndx++;
          break;
        case USAC_ELEMENT_TYPE_CPE:
          chIndx += 2;
          break;
        default:
          break;

        }
    }

  return chIndx;
}

static int readFormatConverterMixmatrix( char* f, int fcMixMatrix[160][160] )
{

  FILE* fMixMatrix;
  char line[512] = {0};
  int column = 0;
  int row = 0;


  fMixMatrix = fopen(f, "r");

  if ( !fMixMatrix )
    {
      fprintf(stderr,"Unable to open format converter mix matrix from file\n");

      return -1;
    }

  /* Get new line */
  while ( fgets(line, 512, fMixMatrix) != NULL )
    {
      int i = 0;
      char* pC = line;

      /* Add newline at eof line */
      line[strlen(line)] = '\n';

      /* Replace all white spaces with new lines for easier parsing */
      while ( pC[i] != '\0')
        {
          if ( pC[i] == ' ' || pC[i] == '\t')
            pC[i] = '\n';

          i++;
        }

      pC = line;
      row = 0;

      /* Parse line */

      while  ( (*pC) != '\0')
        {
          while (  (*pC) == '\n' || (*pC) == '\r'  )
            pC++;

          fcMixMatrix[column][row] = (int)atof(pC);

          /* Jump over parsed float value */
          while (  (*pC) != '\n' )
            pC++;

          /* Jump over new lines */
          while (  (*pC) == '\n' || (*pC) == '\r'  )
            pC++;

          row++;

        }

      column++;

    }

  if ( row == column )
    return 0;
  else
    return -1;


}

