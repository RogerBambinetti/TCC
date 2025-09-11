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

Copyright (c) ISO/IEC 2014.

***********************************************************************************/

#ifndef REVERBGENLIB_HEADER
#define REVERBGENLIB_HEADER

#define REVERBGEN_BLOCK_SIZE 10
#define INPUTDELAY_MAX 20
#define REVERBGEN_BANDS_MAX 64

#ifdef __cplusplus
extern "C" {
#endif


typedef struct
    {
        float **circbuffer_real;
        float **circbuffer_imag;
        float *inputbuffer_real[INPUTDELAY_MAX+1];
        float *inputbuffer_imag[INPUTDELAY_MAX+1];
        int inputbuffer_pointer;
        int inputbuffer_length;
        int *taps[REVERBGEN_BANDS_MAX];
        int **tap_locations[REVERBGEN_BANDS_MAX];
        int **tap_phaseshift_type[REVERBGEN_BANDS_MAX];
        float ***tap_pointers_real[REVERBGEN_BANDS_MAX];
        float ***tap_pointers_imag[REVERBGEN_BANDS_MAX];
        float crossmix[REVERBGEN_BANDS_MAX];
        float directfeed[REVERBGEN_BANDS_MAX];
        float reverberation_times[REVERBGEN_BANDS_MAX];
        float gains[REVERBGEN_BANDS_MAX];
        float stationary_gain[REVERBGEN_BANDS_MAX];
        float attenuation_factor[REVERBGEN_BANDS_MAX];
        float loop_attenuation_factor[REVERBGEN_BANDS_MAX];
        int circbuffer_length[REVERBGEN_BANDS_MAX];
        int circbuffer_length_max[REVERBGEN_BANDS_MAX];
        float **block_buffer_real[REVERBGEN_BANDS_MAX];
        float **block_buffer_imag[REVERBGEN_BANDS_MAX];
        int block_sample;
        float fs;
        int use_correctCoh;
        int initialized;
        float wetGain,dryGain;
        int hybFlag;
        int inChannels,outChannels;
        unsigned long int myrand_next;
        int numQMFBands;
} IISrevdata;

 typedef struct ReverbStruct
  {
    float ***buffer_rev_re;
    float ***buffer_rev_im;
    float *scale;
    float *scale_old;
    int *startReverbQMF;
    int max_startReverbQMF;

  } REVERBSTRUCT, *H_REVERBSTRUCT;

/*
 * Functions
 */
 
unsigned int my_rand(IISrevdata* pData);

int ReverbGenerator_Init(void** ppData_v,
                  int hyb_flag,
                  int num_input_channels,
                  int num_output_channels,
                  int numQMFBands);

int ReverbGenerator_set_reverb_times(void* pData_v,
                              float fs,
                              int num_frequency_points, 
                              float *frequency_points, 
                              float *reverberation_times,
                              float *gains,
                              int seed);

int ReverbGenerator_process_sample(void* pData_v,
                            float **real_in, /* [channels][bands] */
                            float **imag_in,
                            float ***real_out,
                            float ***imag_out,
                            H_REVERBSTRUCT h_reverb_struct
                            );

int ReverbGenerator_set_correctCoh(void* pData_v,
                            int correctCoh_status);

int ReverbGenerator_set_predelay(void* pData_v,
                           int delay_QMF_samples);

int ReverbGenerator_set_drywet(void* pData_v,
                        float wetRatio);

void ReverbGenerator_close(void** ppData_v);

#ifdef __cplusplus
}
#endif

#endif /*#ifndef REVERBGENLIB_HEADER*/
