/***********************************************************************************

This software module was originally developed by 

Fraunhofer IIS, ETRI, Yonsei University, WILUS Institute

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

Fraunhofer IIS, ETRI, Yonsei University, WILUS Institute retain full right to modify 
and use the code for its own purpose, assign or donate the code to a third party and
to inhibit third parties from using the code for products that do not conform to 
MPEG-related ITU Recommendations and/or ISO/IEC International Standards.

This copyright notice must be included in all copies or derivative works. 

Copyright (c) ISO/IEC 2013.

***********************************************************************************/

#ifndef _BINAURALIZER_H

  #define _BINAURALIZER_H

  #include "qmflib.h"
  #include "fftlib.h"  
  #include "wavIO.h"
  #include "ReverbGenerator.h"

  #ifndef NULL
  #define NULL 0
  #endif

  #ifndef PI
  #define PI 3.14159265358979323846f
  #endif

  #define BINAURAL_NUM_OUTCHANNELS 2
  #define QMFLIB_NUMBANDS 64

  /*--------------------------------*/
#ifndef _BRIRMETADATASTRUCT_
#define _BRIRMETADATASTRUCT_
  typedef struct BRIRmetadataStruct 
  {
    int LateReverb_AnalysisBands;
    int noBands_transition;
    float *LateReverb_fc; 
    float *LateReverb_RT60; 
    float *LateReverb_energy; 
    int initDelay;	
    int *transition;
    int channels;
    float *azimuth;
    float *elevation;
    int numLFE;
    int *filterlength_fft; 
    int *N_FFT;
    int *N_BLK;
    int *N_FRM;
    float ****FFTdomain_left_real;
    float ****FFTdomain_left_imag;
    float ****FFTdomain_right_real;
    float ****FFTdomain_right_imag;
    int numband_conv;
    float **TDL_gain_left_real;
    float **TDL_gain_left_imag;
    float **TDL_gain_right_real;
    float **TDL_gain_right_imag;
    int **TDL_delay_left;
    int **TDL_delay_right;
    int *LFEchannels;
    unsigned int fs_BRIRs;
  } BRIRMETADATASTRUCT, *H_BRIRMETADATASTRUCT; 
#endif

  /*--------------------------------*/


  int binaural_getChannelPosData(int CICPIndex, char *filename, int nInChannels, float *Audio_azimuth, float *Audio_elevation, int **LFE_channels, int *num_LFE);

  void binaural_close(H_BRIRMETADATASTRUCT h_BRIRmetadata, 
                    int max_numblocks,
                    int handle_error); 

int binaural_readBRIRmetadata(FILE *fBRIRmetadata, H_BRIRMETADATASTRUCT h_BRIRmetadata);


  int binaural_qmftransform_frame(float **inBuffer, 
                                  int binaural_numTimeslots, 
                                  int binaural_hopsize, 
                                  int nInChannels, 
                                  QMFLIB_POLYPHASE_ANA_FILTERBANK **pAnalysisQmf, 
                                  int ldMode, 
                                  float ***inputBufferFreqQMF_real, 
                                  float ***inputBufferFreqQMF_imag); 

  int binaural_iqmftransform_frame(float **outBuffer, 
                                  int binaural_numTimeslots, 
                                  int binaural_hopsize, 
                                  int nOutChannels, 
                                  QMFLIB_POLYPHASE_SYN_FILTERBANK **pSynthesisQmf, 
                                  int ldMode, 
                                  float ***outputBufferFreqQMF_real, 
                                  float ***outputBufferFreqQMF_imag); 


  int binaural_downmix_frame_qmfdomain(int timeslots, 
                                      float ***input_real, 
                                      float ***input_imag, 
                                      float ***output_real, 
                                      float ***output_imag, 
                                      H_REVERBSTRUCT h_reverb_struct,
                                      int fs,
                                      float **dmxmtx,
                                      int nInChannels,
                                      int kmax); 

  int binaural_get_dmxmtx(int ChannelConfig, 
                         float **dmxmtx, 
                         int *inc_left, 
                         int *inc_right); 

  int binaural_calculate_reverb(void *reverb, H_REVERBSTRUCT h_reverb_struct, float ***dmxOutputQMF_real, float ***dmxOutputQMF_imag, float ***outputBufferQMF_real, float ***outputBufferQMF_imag, int maxband, int blocklength);
  
  int binaural_get_scaling(float ***input_real, 
                          float ***input_imag, 
                          float **dmxmtx, 
                          int *inc_left, 
                          int *inc_right, 
                          int timeslots, 
                          int channels, 
                          float *scale); 

int binaural_init_scaling(H_REVERBSTRUCT h_reverb_struct, float **dmxmtx, int nInChannels);

  int binaural_getBRIRConfig(float *Audio_azimuth, float *Audio_elevation, H_BRIRMETADATASTRUCT h_BRIRmetadata, int *m_conv, int nInChannels, int num_LFE, int *LFE_channels); 

  int binaural_FFTtransform_QMFframe(float ***input_audio_QMF_real, 
                                     float ***input_audio_QMF_imag, 
                                     float ****input_audio_FFT_real, 
                                     float ****input_audio_FFT_imag, 
                                     int numband_conv, 
                                     int *N_FFT, 
                                     int *N_FRM, 
                                     int binaural_numTimeslots,
                                     int nInChannels);

void binaural_VOFF(float*** output_DE_QMF_real, float*** output_DE_QMF_imag, float**** input_audio_FFT_real, float**** input_audio_FFT_imag,
           float*** buf_DE_BLK_left_real, float*** buf_DE_BLK_left_imag, float*** buf_DE_BLK_right_real, float*** buf_DE_BLK_right_imag, 
           float** buf_DE_FRM_left_real, float** buf_DE_FRM_left_imag, float** buf_DE_FRM_right_real, float** buf_DE_FRM_right_imag, 
           H_BRIRMETADATASTRUCT h_BRIRmetadata, int* m_conv, int binaural_numTimeslots, unsigned int nInChannels, int numband_conv);

void binaural_QTDL(float*** output_DE_QMF_real, float*** output_DE_QMF_imag, float*** input_audio_QMF_real, float*** input_audio_QMF_imag,
          float*** buf_TDL_left_real, float*** buf_TDL_left_imag, float*** buf_TDL_right_real, float*** buf_TDL_right_imag, 
          H_BRIRMETADATASTRUCT h_BRIRmetadata, int* m_conv, int binaural_numTimeslots, unsigned int nInChannels, int numband_conv, int numband_proc);

#endif
