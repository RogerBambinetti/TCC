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

#define SLOPE1_WGHT_LSF (float)((3.347-1.8)/(450.0-0.0))
#define SLOPE2_WGHT_LSF (float)((1.8-1.0)/(1500.0-450.0))

void lsf_wt(float *lsf, float *wf);

void lsf_wt(float *lsf, float *wf)
{
   float temp;
   int i;

   wf[0] = lsf[1];

   for (i=1; i<14; i++) {
      wf[i] = lsf[i+1] -lsf[i-1];
   }
   wf[14] = 6400.0f - lsf[14];

   for (i=0; i<15; i++) {
      if (wf[i]<450.0f) {
         temp = 3.347f - SLOPE1_WGHT_LSF * wf[i];
      }
      else {
         temp = 1.8f - SLOPE2_WGHT_LSF * (wf[i]-450.0f);
      }
      wf[i] = temp*temp;
   }

   wf[15] = wf[0];
   for (i=1; i<15; i++) wf[15] += wf[i];
   wf[15] = wf[15]/15;
/*   wf[15] = 1.0f;*/

   return;

}

