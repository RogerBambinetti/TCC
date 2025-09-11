/***********************************************************************************

This software module was originally developed by

VoiceAge Corp.

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

VoiceAge Corp. retains full right to modify and use the code for its own purpose,
assign or donate the code to a third party and to inhibit third parties from using
the code for products that do not conform to MPEG-related ITU Recommendations and/or
ISO/IEC International Standards.

This copyright notice must be included in all copies or derivative works.

Copyright (c) ISO/IEC 2014.

***********************************************************************************/

#ifndef re8_h
#define re8_h

/* RE8 lattice quantiser constants */
#define NB_SPHERE 32
#define NB_LEADER 37
#define NB_LDSIGN 226
#define NB_LDQ3   9
#define NB_LDQ4   28

/* RE8 lattice quantiser functions in re8_*.c */
void RE8_PPV(float x[], int y[]);
void RE8_cod(int *y, int *n, long *I, int *k);
void RE8_dec(int nq, long I, int kv[], int y[]);
void RE8_vor(int y[], int *n, int k[], int c[], int *ka);
void re8_coord(int y[], int k[]);
void re8_k2y(int k[], int m, int y[]);

/* RE8 lattice quantiser tables */
extern const int tab_pow2[8];
extern const int tab_factorial[8];
extern const int Ia[NB_LEADER];
extern const unsigned char Ds[NB_LDSIGN];
extern const unsigned int Is[NB_LDSIGN];
extern const int Ns[], A3[], A4[];
extern const unsigned char Da[][8];
extern const unsigned int I3[], I4[];
extern const int Da_nq[], Da_pos[], Da_nb[];
extern const unsigned int Da_id[];

#endif
