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


#ifndef _usac_arith_dec_h_
#define _usac_arith_dec_h_

#include "bitmux.h"

#ifndef uchar
#define uchar	unsigned char
#endif

/* types */
typedef struct {
  int	a,b;
  int   c, c_prev;
} Tqi2;

/* function prototypes */
void acSpecFrame(HANDLE_USAC_DECODER      hDec,
                 Info*                    info,
                 float*                   coef,
                 int                      max_spec_coefficients,
                 int                      nlong,
                 int                      noise_level,
                 short*                   factors,
                 int                      arithSize,
                 int                     *arithPreviousSize,
                 Tqi2                     arithQ[],
                 HANDLE_BUFFER            hVm,
                 byte                     max_sfb,
                 byte                     max_noise_sfb,
                 int                      reset,
                 unsigned int            *nfSeed,
                 int                      bUseNoiseFilling,
                 int                      stereoFilling,
                 const int                fdp_spacing_index, /* -1: no FDP */
                 const int                ch, /* overall channel index */
                 const int                bUsacIndependencyFlag,
                 int                      igFilling,
                 int                      igFNT,
                 int                      igFUseINF,
                 int                      igFAllZeroFlag,
                 float*                   igf_infSpec,
                 float                   *dmx_re_prev);

int aencSpecFrame(HANDLE_BSBITSTREAM       bs_data,
                  WINDOW_SEQUENCE          windowSequence,
                  int                      nlong,
                  int                      *quantSpectrum,
                  int                      max_spec_coefficients,
                  Tqi2                     arithQ[],
                  int                      *arithPreviousSize,
                  int                      reset);

int tcxArithDec(int                      tcx_size,
                int*                     quant,
                int                      *arithPreviousSize,
                Tqi2                     arithQ[],
                HANDLE_BUFFER            hVm,
                int 			 reset);

int tcxArithEnc(int      tcx_size,
                int      max_tcx_size,
		int      *quantSpectrum,
		Tqi2     arithQ[],
		int      update_flag,
		unsigned char bitBuffer[]);

int tcxArithReset(Tqi2 arithQ[]);

typedef struct {
  int	low,high,vobf;
} Tastat;
long ari_start_decoding(unsigned char *buf,long bp,Tastat *s);
long ari_decode(unsigned char *buf,
                       long bp,
                       int *res,
                       Tastat *s,
                       unsigned short const *cum_freq,
                       long cfl);
long ari_done_decoding(long bp);
long ari_encode(unsigned char *buf,
                       long bp,
                       Tastat *s,
                       long symbol,
                       unsigned short const *cum_freq);
long ari_done_encoding(
                unsigned char *buf,
                       long bp,
                       Tastat *s);
void ari_start_encoding(Tastat *s);

#endif  /* _usac_arith_h_ */
