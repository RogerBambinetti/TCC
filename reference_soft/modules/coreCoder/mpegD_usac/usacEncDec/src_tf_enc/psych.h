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

/* CREATED BY :  Bernhard Grill -- August-96  */

typedef struct { 
  long   sampling_rate;                   /* the following entries are for this sampling rate */
  int    num_cb_long;
  int    num_cb_short;
  const short* cb_offset_long;
  const short* cb_offset_short;
  double fixed_ratio_long[NSFB_LONG];
  double fixed_ratio_short[NSFB_SHORT];
  int    cb_width_long[NSFB_LONG];
  int    cb_width_short[NSFB_LONG];

} SR_INFO;

 
typedef struct {
  double *p_ratio;
  int    *cb_width;
  int    no_of_cb;
} CH_PSYCH_OUTPUT;

#ifdef __cplusplus
extern "C" {
#endif

void EncTf_psycho_acoustic_init( void );
int EncTf_psycho_acoustic ( double          sampling_rate,
                            int             no_of_chan,         
                            int             frameLength,
                            int             windowLength,
                            CH_PSYCH_OUTPUT p_chpo_long[],
                            CH_PSYCH_OUTPUT p_chpo_short[][MAX_SHORT_WINDOWS] );

int EncTf_psycho_acoustic_long_sfb_offset ( double          sampling_rate, 
                            int             frameLength, 
                            int             windowLength,
                            int            *swb_offset
);


#ifdef __cplusplus
}
#endif
