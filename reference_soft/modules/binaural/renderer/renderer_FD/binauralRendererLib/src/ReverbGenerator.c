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

#define USE_DOWNMIX_ENERGY_NORMALIZATION 0
#define PI 3.14159265f

#include <math.h>
#include <stdlib.h>
#include <string.h>

/*declarations*/
#include "ReverbGenerator.h"


/* pseudo-random algorithm: linear congruential generator */
unsigned int my_rand(IISrevdata* pData) {
    pData->myrand_next=pData->myrand_next * 1103515245 + 12345;
    return (unsigned int)(pData->myrand_next/65536) % 32768; /* output bits 30..16 */
}


int ReverbGenerator_Init(void** ppData_v,
                  int hyb_flag,
                  int num_input_channels,
                  int num_output_channels,
                  int numQMFBands
                  )
{   
    int band,channel,ts;
    IISrevdata* pData = (IISrevdata*)calloc(1,sizeof(IISrevdata));
    *ppData_v = (void*)pData;
    
    /* Init defaults */
    pData->use_correctCoh=0;
    pData->inputbuffer_length=1;
    pData->inputbuffer_pointer=0;
    pData->dryGain=0;
    pData->wetGain=1;
    pData->block_sample=0;
    pData->hybFlag=hyb_flag;
    pData->inChannels=num_input_channels;
    pData->outChannels=num_output_channels;    
    pData->numQMFBands = numQMFBands;

    for (ts = 0; ts < INPUTDELAY_MAX+1; ts++)
    {
       /* Input buffer creates the predelay */
        pData->inputbuffer_real[ts]=(float*)calloc(pData->numQMFBands,sizeof(float));
        pData->inputbuffer_imag[ts]=(float*)calloc(pData->numQMFBands,sizeof(float));
    }

    pData->circbuffer_real = (float**)calloc(pData->numQMFBands,sizeof(float*));
    pData->circbuffer_imag = (float**)calloc(pData->numQMFBands,sizeof(float*));
    for (band=0;band<pData->numQMFBands;band++)
    {
        /* Circular buffer */
        pData->circbuffer_length_max[band] = 500/(1+band)+(REVERBGEN_BANDS_MAX-band); /* 1 over f */
        pData->circbuffer_real[band]=(float*)calloc(pData->circbuffer_length_max[band]+REVERBGEN_BLOCK_SIZE,sizeof(float));
        pData->circbuffer_imag[band]=(float*)calloc(pData->circbuffer_length_max[band]+REVERBGEN_BLOCK_SIZE,sizeof(float));
        
        /*  sparse filter tap locations */
        pData->taps[band] = (int*)calloc(pData->outChannels,sizeof(int));
        pData->tap_locations[band] = (int**)calloc(pData->outChannels,sizeof(int*));
        pData->tap_phaseshift_type[band] = (int**)calloc(pData->outChannels,sizeof(int*));
        pData->tap_pointers_real[band] = (float***)calloc(pData->outChannels,sizeof(float**));
        pData->tap_pointers_imag[band] = (float***)calloc(pData->outChannels,sizeof(float**));
        
        /* blockwise processing */
        pData->block_buffer_real[band]=(float**)calloc(pData->outChannels,sizeof(float*));
        pData->block_buffer_imag[band]=(float**)calloc(pData->outChannels,sizeof(float*));
        for(channel=0;channel<pData->outChannels;channel++)
        {
            pData->tap_locations[band][channel] = (int*)calloc(pData->circbuffer_length_max[band],sizeof(int));
            pData->tap_phaseshift_type[band][channel] = (int*)calloc(pData->circbuffer_length_max[band],sizeof(int));
            pData->tap_pointers_real[band][channel] = (float**)calloc(pData->circbuffer_length_max[band],sizeof(float*));
            pData->tap_pointers_imag[band][channel] = (float**)calloc(pData->circbuffer_length_max[band],sizeof(float*));
            pData->block_buffer_real[band][channel]=(float*)calloc(REVERBGEN_BLOCK_SIZE,sizeof(float));
            pData->block_buffer_imag[band][channel]=(float*)calloc(REVERBGEN_BLOCK_SIZE,sizeof(float));
        }
        
    }
    
    pData->initialized=1;
    
    return 0;
}


int ReverbGenerator_set_reverb_times(void* pData_v,
                              float fs, 
                              int num_frequency_points, 
                              float *frequency_points, 
                              float *reverberation_times,
                              float *gains,
                              int seed)
{
    float diffuse_field_ICC,temp_float;
    float attenuation_factor_square;
    float band_centerfreq, maxfreq;
    int band, index, channel;
    float prevfreq,nextfreq, fraction;
    float prevfreq_revtime, nextfreq_revtime;
    float prevfreq_gain, nextfreq_gain;
    float energy_buildup,current_energy;
    int tap_index,sample;
    float intended_energy, actualized_energy;
    int smart_buffersize;
    IISrevdata* pData = (IISrevdata*)pData_v;

    pData->fs=fs;

    pData->myrand_next = (unsigned long int)seed;
    for (band=0; band<pData->numQMFBands; band++) {
        band_centerfreq=((float)band+0.5f)/((float)pData->numQMFBands)*pData->fs/2.0f;
        if (band == 0) 
        {
            diffuse_field_ICC = 1.0f; /* force ICC = 1.0 for lowest band */
        } 
        else if (band_centerfreq < 2700.0f)
        {
            diffuse_field_ICC = (float) sin(PI*band_centerfreq/550.0f+1e-20f)/(PI*band_centerfreq/550.0f+1e-20f)*(1.0f-band_centerfreq/2700.0f);
        }
        else
        {
            diffuse_field_ICC=0.0f;
        }

        temp_float=(1.0f- (float) sqrt(1.0f-pow(diffuse_field_ICC,2.0f)))/2.0f;

        /* mixing gains */
        pData->crossmix[band] = (((diffuse_field_ICC > 0.0f) - (diffuse_field_ICC < 0.0f) )* (float) sqrt(fabs(temp_float)));
        pData->directfeed[band] = (float) sqrt(1.0f-fabs(temp_float));

        /* arbitrary set of frequency points defined -> interpolate the values linearly for the 64-band QMF. */
        prevfreq=-1.0f;
        nextfreq=1000000000.0f;

        if (frequency_points[num_frequency_points-1] > pData->fs/2.0f)
            maxfreq = frequency_points[num_frequency_points-1];
        else
            maxfreq = pData->fs/2.0f;
        
        for (index=0;index<num_frequency_points;index++)
        {
            if (frequency_points[index] <= band_centerfreq && frequency_points[index] > prevfreq)
            {
                prevfreq=frequency_points[index];
                prevfreq_revtime=reverberation_times[index];
                prevfreq_gain=gains[index];
            }
            if (frequency_points[index] >= band_centerfreq && frequency_points[index] < nextfreq)
            {
                nextfreq=frequency_points[index];
                nextfreq_revtime=reverberation_times[index];
                nextfreq_gain=gains[index];
            }
        }
        if (prevfreq < 0.0f)
        {
            prevfreq=0.0f;
            prevfreq_revtime=nextfreq_revtime;
            prevfreq_gain=nextfreq_gain;
        }
        if (nextfreq > maxfreq)
        {
            nextfreq=maxfreq;
            nextfreq_revtime=prevfreq_revtime;
            nextfreq_gain=prevfreq_gain;
        }
        fraction=(band_centerfreq-prevfreq)/(nextfreq-prevfreq+1e-12f);
        
        pData->reverberation_times[band]=(1.0f-fraction)*prevfreq_revtime + fraction*nextfreq_revtime;
        pData->stationary_gain[band]=(1.0f-fraction)*prevfreq_gain + fraction*nextfreq_gain;
        
        smart_buffersize = (int)(pData->reverberation_times[band]*150.0f)+1;
       
        if (pData->circbuffer_length_max[band]>smart_buffersize)
        {
            pData->circbuffer_length[band]=smart_buffersize;
        }
        else
        {
            pData->circbuffer_length[band] = pData->circbuffer_length_max[band];
        }
        
        /* Attenuation factor: energy decay according to reverberation time */
        pData->attenuation_factor[band] = (float) pow(10.0f, -3.0f*((float)REVERBGEN_BANDS_MAX)/(pData->fs*pData->reverberation_times[band]));
        pData->loop_attenuation_factor[band] = (float) pow(pData->attenuation_factor[band],pData->circbuffer_length[band]);
        attenuation_factor_square=pData->attenuation_factor[band]*pData->attenuation_factor[band];
        
        /* sparse decorrelation filters */
        intended_energy=0.0f;
        actualized_energy=0.0f;
        
        for(channel=0;channel<pData->outChannels;channel++)
        {
            energy_buildup=0.0f;
            current_energy=1.0f;
            tap_index=0;
            for(sample=0;sample<pData->circbuffer_length[band];sample++)
            {
                intended_energy += current_energy;
                energy_buildup += current_energy+ 0.1f*((float)my_rand(pData)/32768.0f-0.5f);
                if (energy_buildup >= 1.0f)
                {
                    pData->tap_locations[band][channel][tap_index]=sample;
                    /* four efficient phase operations: n*pi/2, n=0,1,2,3 */
                    pData->tap_phaseshift_type[band][channel][tap_index] = my_rand(pData) % 4;
                    pData->tap_pointers_real[band][channel][tap_index] = &(pData->circbuffer_real[band][sample]);
                    pData->tap_pointers_imag[band][channel][tap_index] = &(pData->circbuffer_imag[band][sample]);
                    energy_buildup -= 1.0f;
                    tap_index++;
                    actualized_energy +=1.0f;
                }
                current_energy *= attenuation_factor_square;
            }
            pData->taps[band][channel]=tap_index;
            
        }
        /* energy measure over all channels */
        pData->gains[band] = pData->stationary_gain[band];
        pData->gains[band] *= (float) sqrt(intended_energy/actualized_energy);
        pData->gains[band] *= (float) sqrt(0.5f*(1.0f-attenuation_factor_square));      
        
    }
    
    return 0;
}


int ReverbGenerator_process_sample(void* pData_v,
                            float **real_in,
                            float **imag_in,
                            float ***real_out,
                            float ***imag_out,
                            H_REVERBSTRUCT h_reverb_struct)
{
    IISrevdata* pData = (IISrevdata*)pData_v;
    float ***block_buffer_real = pData->block_buffer_real;
    float ***block_buffer_imag = pData->block_buffer_imag;
    int band,channel,sample,tap,band_ext,taps;
    int size,reverse_sample_index,inputbuffer_pointer;
    float *p_real_source, *p_real_target;
    float *p_imag_source, *p_imag_target;
    float gain,dryGain,wetGain;
    float equalizerTargetEnergy;
    
    /* 1. Delay the input in the input delay buffer and collect into the block buffer */
    sample=pData->block_sample;
    inputbuffer_pointer = pData->inputbuffer_pointer;
    reverse_sample_index=REVERBGEN_BLOCK_SIZE-sample-1;
    
    band_ext=0;
    band=0; 
    equalizerTargetEnergy=0.0f;
    
   
    
    for (channel=0; channel < pData->inChannels; channel++)
    {
        /* accumulate input channels and apply wetGain */
        p_real_source=real_in[channel];
        p_imag_source=imag_in[channel];
        if (channel % 2)
        {
            for (band=0;band<pData->numQMFBands;band++)
            {
                wetGain=pData->gains[band]*pData->wetGain;
                pData->inputbuffer_real[inputbuffer_pointer][band] += wetGain*p_real_source[band];
                pData->inputbuffer_imag[inputbuffer_pointer][band] += wetGain*p_imag_source[band];
            }
        }
        else
        {   
            /* every other channel is added up with 90?phase shift */
            for (band=0;band<pData->numQMFBands;band++)
            {
                wetGain=pData->gains[band]*pData->wetGain;
                pData->inputbuffer_real[inputbuffer_pointer][band] -= wetGain*p_imag_source[band];
                pData->inputbuffer_imag[inputbuffer_pointer][band] += wetGain*p_real_source[band];
            }
            
        }
    }
    dryGain=pData->dryGain;

    for (band=0;band<pData->numQMFBands;band++)
    {
      for (channel=0;channel < pData->outChannels;channel++)
      {
          p_real_target=real_out[h_reverb_struct->startReverbQMF[band]-1][channel];
          p_imag_target=imag_out[h_reverb_struct->startReverbQMF[band]-1][channel];
          if ((channel < pData->inChannels) && (dryGain>0.0f))
          {
              p_real_source=real_in[channel];
              p_imag_source=imag_in[channel];
              /* output block buffer contents and apply dryGain */
                  p_real_target[band] = block_buffer_real[band][channel][reverse_sample_index] + dryGain*p_real_source[band];
                  p_imag_target[band] = block_buffer_imag[band][channel][reverse_sample_index] + dryGain*p_imag_source[band];
                  block_buffer_real[band][channel][reverse_sample_index]=0.0f;
                  block_buffer_imag[band][channel][reverse_sample_index]=0.0f;
          }
          else {
              /* output block buffer contents */
                  p_real_target[band] = block_buffer_real[band][channel][reverse_sample_index];
                  p_imag_target[band] = block_buffer_imag[band][channel][reverse_sample_index];
                  block_buffer_real[band][channel][reverse_sample_index]=0.0f;
                  block_buffer_imag[band][channel][reverse_sample_index]=0.0f;
          }
      }
    }
    
    inputbuffer_pointer = (pData->inputbuffer_pointer + 1) % pData->inputbuffer_length;
    pData->inputbuffer_pointer = inputbuffer_pointer;
    
    /* Add data from the predelay buffer to the input block that is to be fed to the reverberator */
    for (band=0;band<pData->numQMFBands;band++)
    {
        block_buffer_real[band][0][reverse_sample_index] += pData->inputbuffer_real[inputbuffer_pointer][band];
        block_buffer_imag[band][0][reverse_sample_index] += pData->inputbuffer_imag[inputbuffer_pointer][band];
        pData->inputbuffer_real[inputbuffer_pointer][band]=0.0f;
        pData->inputbuffer_imag[inputbuffer_pointer][band]=0.0f;
    }
    
    pData->block_sample++;
    if (pData->block_sample < REVERBGEN_BLOCK_SIZE)
    {
        return 0;
    }
    pData->block_sample=0;
    
    /* 2. Rotate data in the buffer and insert the collected block */
    for (band=0;band<pData->numQMFBands;band++)
    {
        /* move data forwards */
        size=sizeof(float)*(pData->circbuffer_length[band]);
        memmove(pData->circbuffer_real[band]+REVERBGEN_BLOCK_SIZE,pData->circbuffer_real[band],size); 
        memmove(pData->circbuffer_imag[band]+REVERBGEN_BLOCK_SIZE,pData->circbuffer_imag[band],size);
        
        /* insert the new collected block to the beginning */
        size=sizeof(float)*(REVERBGEN_BLOCK_SIZE);
        memmove(pData->circbuffer_real[band],block_buffer_real[band][0],size);
        memmove(pData->circbuffer_imag[band],block_buffer_imag[band][0],size);
        
        p_real_target = pData->circbuffer_real[band];
        p_imag_target = pData->circbuffer_imag[band];
        p_real_source = pData->circbuffer_real[band]+pData->circbuffer_length[band];
        p_imag_source = pData->circbuffer_imag[band]+pData->circbuffer_length[band];
        gain = pData->loop_attenuation_factor[band];
        
        /* Add the circulating data from the end of the loop to 
           the beginning, with an attenuation factor according to RT60 */
        for (sample=0;sample<REVERBGEN_BLOCK_SIZE;sample++)
        {
            p_real_target[sample] += gain*p_real_source[sample];
            p_imag_target[sample] += gain*p_imag_source[sample];
        }
    }
    
    /* 3. Flush the block buffer for collecting the output audio */
    for (band=0;band<pData->numQMFBands;band++)
    {
        for(channel=0;channel<pData->outChannels;channel++)
        {
            memset(block_buffer_real[band][channel], 0, REVERBGEN_BLOCK_SIZE*sizeof(float));
            memset(block_buffer_imag[band][channel], 0, REVERBGEN_BLOCK_SIZE*sizeof(float));
        }
    }
    
    /* 4. Do the filtering for one block, in each band and each output channel: complex sparse FIR filtering */
    for (band=0;band<pData->numQMFBands;band++)
    {
        for(channel=0;channel<pData->outChannels;channel++)
        {
            float *p_real_out,*p_imag_out;
            float **pp_real_tap,**pp_imag_tap;
            float *p_real_tap,*p_imag_tap;
            int *p_phaseshift_type;
            p_real_out = block_buffer_real[band][channel];
            p_imag_out = block_buffer_imag[band][channel];
            pp_real_tap=pData->tap_pointers_real[band][channel];
            pp_imag_tap=pData->tap_pointers_imag[band][channel];
            taps=pData->taps[band][channel];
            p_phaseshift_type=pData->tap_phaseshift_type[band][channel];
            for(tap=0;tap<taps;tap++)
            {
                p_real_tap=pp_real_tap[tap];
                p_imag_tap=pp_imag_tap[tap];
                
                switch (p_phaseshift_type[tap])
                {
                    case 0:
                        for (sample=0;sample<REVERBGEN_BLOCK_SIZE;sample++)
                        {
                            p_real_out[sample] += p_real_tap[sample];
                            p_imag_out[sample] += p_imag_tap[sample];
                        }
                        break;
                    case 1:
                        for (sample=0;sample<REVERBGEN_BLOCK_SIZE;sample++)
                        {
                            p_real_out[sample] -= p_imag_tap[sample];
                            p_imag_out[sample] += p_real_tap[sample];
                        }
                        break;
                    case 2:
                        for (sample=0;sample<REVERBGEN_BLOCK_SIZE;sample++)
                        {
                            p_real_out[sample] -= p_real_tap[sample];
                            p_imag_out[sample] -= p_imag_tap[sample];
                        }
                        break;
                    default:
                        for (sample=0;sample<REVERBGEN_BLOCK_SIZE;sample++)
                        {
                            p_real_out[sample] += p_imag_tap[sample];
                            p_imag_out[sample] -= p_real_tap[sample];
                        }
                        break;
                }
            }
        }
        
        
        /* correct diffuse field binaural coherence */
        if (band < 10)
        {
            if (pData->use_correctCoh)
            {
                for (sample=0; sample<REVERBGEN_BLOCK_SIZE; sample++)
                {
                    float correctCoh_real_l, correctCoh_real_r;
                    float correctCoh_imag_l, correctCoh_imag_r;
                    correctCoh_real_l = pData->directfeed[band]*block_buffer_real[band][0][sample] + pData->crossmix[band]*block_buffer_real[band][1][sample];
                    correctCoh_real_r = pData->directfeed[band]*block_buffer_real[band][1][sample] + pData->crossmix[band]*block_buffer_real[band][0][sample];
                    correctCoh_imag_l = pData->directfeed[band]*block_buffer_imag[band][0][sample] + pData->crossmix[band]*block_buffer_imag[band][1][sample];
                    correctCoh_imag_r = pData->directfeed[band]*block_buffer_imag[band][1][sample] + pData->crossmix[band]*block_buffer_imag[band][0][sample];
                    block_buffer_real[band][0][sample]=correctCoh_real_l;
                    block_buffer_real[band][1][sample]=correctCoh_real_r;
                    block_buffer_imag[band][0][sample]=correctCoh_imag_l;
                    block_buffer_imag[band][1][sample]=correctCoh_imag_r;
                }
            }
        }
        
        
    }
    return 0;
}

int ReverbGenerator_set_correctCoh(void* pData_v,
                           int correctCoh_status)
{
    IISrevdata* pData = (IISrevdata*)pData_v;
    if (pData->outChannels == 2)
    {
        pData->use_correctCoh=correctCoh_status;
    }
    return 0;
}

int ReverbGenerator_set_predelay(void* pData_v,
                          int delay_QMF_samples)
{
    IISrevdata* pData = (IISrevdata*)pData_v;
    int new_predelay = delay_QMF_samples - REVERBGEN_BLOCK_SIZE;
    
    if (new_predelay<0)
    {
        pData->inputbuffer_length=1;
        return -2;
    }
    if (new_predelay>INPUTDELAY_MAX)
    {
        pData->inputbuffer_length=INPUTDELAY_MAX+1;
        return -1;
    }
    pData->inputbuffer_length=new_predelay+1;
    return 0;
    
}


int ReverbGenerator_set_drywet(void* pData_v, float wetRatio) {
    IISrevdata* pData = (IISrevdata*)pData_v;
    pData->wetGain = (float) sqrt(wetRatio);
    pData->dryGain = (float) sqrt(1.0f - wetRatio);
    return 0;
}


void ReverbGenerator_close(void** ppData_v)
{
    int band,channel,ts;
    IISrevdata* pData = (IISrevdata*)(*ppData_v);
    
    if (pData != NULL) {

        for (ts=0; ts<INPUTDELAY_MAX+1; ts++)
        {
            free(pData->inputbuffer_real[ts]);
            free(pData->inputbuffer_imag[ts]);
        }
        
        for (band=0;band<pData->numQMFBands;band++)
        {
            free(pData->circbuffer_real[band]);
            free(pData->circbuffer_imag[band]);
            
            for(channel=0;channel<pData->outChannels;channel++)
            {
                free(pData->tap_phaseshift_type[band][channel]);
                free(pData->tap_pointers_real[band][channel]);
                free(pData->tap_pointers_imag[band][channel]);
                free(pData->tap_locations[band][channel]);
                free(pData->block_buffer_real[band][channel]);
                free(pData->block_buffer_imag[band][channel]);
                
            }
            free(pData->tap_locations[band]);
            free(pData->tap_phaseshift_type[band]);
            free(pData->tap_pointers_real[band]);
            free(pData->tap_pointers_imag[band]);
            free(pData->taps[band]);
            free(pData->block_buffer_real[band]);
            free(pData->block_buffer_imag[band]);
            
        }
        free(pData->circbuffer_real);
        free(pData->circbuffer_imag);

        free(pData);
        *ppData_v=NULL;
    }
}

