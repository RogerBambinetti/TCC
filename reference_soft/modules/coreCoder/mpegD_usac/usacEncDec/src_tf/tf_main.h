/************************************************************************

This software module was originally developed by Fraunhofer IIS and 
University of Erlangen (UER) in the course of development of the ISO/IEC 23008-3 for 
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

Fraunhofer IIS and University of Erlangen (UER) retain full right to modify and use the 
code for its own purpose, assign or donate the code to a third party and 
to inhibit third parties from using the code for products that do not 
conform to MPEG-related ITU Recommendations and/or ISO/IEC International 
Standards.

This copyright notice must be included in all copies or derivative works.

Copyright (c) ISO/IEC 1996.

*************************************************************************/

/* CREATED BY :  Bernhard Grill -- June-96  */

/* 28-Aug-1996  NI: added "NO_SYNCWORD" to enum TRANSPORT_STREAM */

#ifndef _TF_MAIN_H_INCLUDED
#define _TF_MAIN_H_INCLUDED

#include "ntt_conf.h"
#include "block.h"

void buffer2freq( double           p_in_data[],      /* Input: Time signal              */   
                  double           p_out_mdct[],     /* Output: MDCT cofficients        */
                  double           p_overlap[],
                  WINDOW_SEQUENCE  windowSequence,
                  WINDOW_SHAPE     wfun_select,      /* offers the possibility to select different window functions */
                  WINDOW_SHAPE     wfun_select_prev,
                  int              nlong,            /* shift length for long windows   */
                  int              nshort,           /* shift length for short windows  */
                  Mdct_in          overlap_select,   /* select mdct input *TK*          */
                  int              previousMode,
                  int              nextMode,
                  int              num_short_win);   /* number of short windows to      
                                                        transform                       */
void freq2buffer( double           p_in_data[],      /* Input: MDCT coefficients                */
                  double           p_out_data[],     /* Output:time domain reconstructed signal */
                  double           p_overlap[],  
                  WINDOW_SEQUENCE  windowSequence,
                  int              nlong,            /* shift length for long windows   */
                  int              nshort,           /* shift length for short windows  */
                  WINDOW_SHAPE     wfun_select,      /* offers the possibility to select different window functions */
                  WINDOW_SHAPE     wfun_select_prev, /* YB : 971113 */
                  Imdct_out        overlap_select,   /* select imdct output *TK*        */
                  int              num_short_win );  /* number of short windows to  transform */

void mdct( double in_data[], double out_data[], int len);

void imdct(double in_data[], double out_data[], int len);

void fft( double in[], double out[], int len );

/* functions from tvqAUDecode.e */

void tvqInitDecoder (char *decPara, int layer,
                     FRAME_DATA *frameData,
                     TF_DATA *tfData);
                     

void tvqFreeDecoder (HANDLE_TVQDECODER nttDecoder);


void tvqAUDecode ( 
                FRAME_DATA*  fd, /* config data , obj descr. etc. */
                TF_DATA*     tfData,
                HANDLE_DECODER_GENERAL hFault,
                QC_MOD_SELECT      qc_select,
                ntt_INDEX*   index,
                ntt_INDEX*   index_scl,
                int * nextLayer);

#endif  /* #ifndef _TF_MAIN_H_INCLUDED */
