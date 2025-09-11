
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
 
 Copyright (c) ISO/IEC 2013.
 
 ***********************************************************************************/


#include "saoc_svd_tool.h"
#include "error.h"

#define SAOC_SVD_EPS        1e-32f /* 1e-16f */

#ifndef FLT_EPSILON 
#define FLT_EPSILON         1.19209290e-07f /* decimal constant */
#endif

#define SAOC_MAX_NUM_ITERATION       75

static float GivensRotation(float x, float z);

static int biDiagonalReduction( float singularVecors[][SAOC_MAX_CHANNELS],
                                float singularValues[SAOC_MAX_CHANNELS],
                                float secDiag[SAOC_MAX_CHANNELS],
                                int nChannels,
                                int currChannel,
                                float *sig_x,
                                float *g,
                                int leftHand);

static int singularVecorsAccumulation( float singularVecors_Left[][SAOC_MAX_CHANNELS],
                                       float singularVecors_Right[][SAOC_MAX_CHANNELS],
                                       float singularValues[SAOC_MAX_CHANNELS],
                                       float secDiag[SAOC_MAX_CHANNELS],
                                       int nChannels,
                                       int leftHand);

static int HouseholderReduction( float singularVecors_Left[][SAOC_MAX_CHANNELS],
                                 float singularValues[SAOC_MAX_CHANNELS],
                                 float singularVecors_Right[][SAOC_MAX_CHANNELS],
                                 float secDiag[SAOC_MAX_CHANNELS],
                                 int nChannels,
                                 float *anorm);

static int BidagonalDiagonalisation( float singularVectors_Left[][SAOC_MAX_CHANNELS],
                                     float singularValues[SAOC_MAX_CHANNELS],
                                     float singularVectors_Right[][SAOC_MAX_CHANNELS],
                                     float secDiag[SAOC_MAX_CHANNELS],
                                     int nChannels,
                                     float eps_x);

static void ApplyQRTransform( float singularVectors_Left[][SAOC_MAX_CHANNELS],
                             float singularValues[SAOC_MAX_CHANNELS],
                             float singularVectors_Right[][SAOC_MAX_CHANNELS],
                             float secDiag[SAOC_MAX_CHANNELS],
                             int startIndex, 
                             int currentIndex, 
                             int nChannels);

static void ApplyRotation(float singularVector[][SAOC_MAX_CHANNELS],
                          float c,
                          float s,
                          float x11,
                          float x12,
                          float *f,
                          float *g,
                          int currentIndex1,
                          int currentIndex2,
                          int nChannels);

static float maxWithSign(float a);

int saoc_SVD(float InputMatrix[][SAOC_MAX_CHANNELS], 
             float singularVectors_Left[][SAOC_MAX_CHANNELS],
             float singularValues[SAOC_MAX_CHANNELS],
             float singularVectors_Right[][SAOC_MAX_CHANNELS],
             int nChannels)

{
    int iCh,jCh;
    float secDiag[SAOC_MAX_CHANNELS]={0.0f};
    float eps_x = 0.0f, temp;
    int errorMessage, condition;
    
    for (iCh = 0; iCh < nChannels; iCh++) {
      for (jCh = 0; jCh < nChannels; jCh++)  {
        singularVectors_Left[iCh][jCh] = InputMatrix[iCh][jCh];
      }
    }

    HouseholderReduction(singularVectors_Left,singularValues,singularVectors_Right,secDiag,nChannels,&eps_x);

    errorMessage = BidagonalDiagonalisation(singularVectors_Left, singularValues, singularVectors_Right, secDiag, nChannels, eps_x);

    /* Sort the singular values descending order */
    do{
      condition = 0;  
      for (iCh = 0; iCh<nChannels-1; iCh++){
        if (singularValues[iCh] < singularValues[iCh+1]){
          condition = 1;
          temp = singularValues[iCh]; singularValues[iCh] = singularValues[iCh+1]; singularValues[iCh+1] = temp;
					for (jCh = 0; jCh<nChannels; jCh++)
					{
						temp = singularVectors_Left[jCh][iCh]; singularVectors_Left[jCh][iCh] = singularVectors_Left[jCh][iCh+1]; singularVectors_Left[jCh][iCh+1]=temp;
						temp = singularVectors_Right[jCh][iCh]; singularVectors_Right[jCh][iCh] = singularVectors_Right[jCh][iCh+1]; singularVectors_Right[jCh][iCh+1]=temp;
					}
        }
      }
    } while (condition == 1);
    return(errorMessage);
}

static int BidagonalDiagonalisation( float singularVectors_Left[][SAOC_MAX_CHANNELS],
                                     float singularValues[SAOC_MAX_CHANNELS],
                                     float singularVectors_Right[][SAOC_MAX_CHANNELS],
                                     float secDiag[SAOC_MAX_CHANNELS],
                                     int nChannels,
                                     float eps_x)
{
  int  kCh,  nCh, iCh, jCh, split;
  float c, s, f1, f2;
  float g = 0.0f;
  int convergence, iteration, found_split;
  int error = ERROR_NONE;

  for (iCh = nChannels - 1; iCh >= 0; iCh--)   { 
    convergence = 0;
    iteration = 0;
    while (convergence == 0) {
      iteration++;
                    
      found_split = 1;
      for (jCh = iCh; jCh >= 0; jCh--) {  
          split = jCh - 1;
          if (fabs(secDiag[jCh]) <= FLT_EPSILON * eps_x ){ /* is secDiag[ch] vanishing compared to eps_x */
              found_split = 0;
              break;
          }
          if (fabs(singularValues[split]) <= FLT_EPSILON * eps_x) { /* is singularValues[split] vanishing compared to eps_x */
              break;
          }
      }

      convergence = (jCh == iCh)? 1: 0;

      if (found_split) {
        s = 1.0f;
        c = 0.0f;
        for (kCh = jCh; kCh <= iCh; kCh++) {
          g = s * secDiag[kCh]; 
          secDiag[kCh] = c * secDiag[kCh];
          if (fabs(g) <= FLT_EPSILON *eps_x) {
            break;
          } 
          c = singularValues[kCh];
          singularValues[kCh] = GivensRotation(g, singularValues[kCh]);
          c = c /maxWithSign(singularValues[kCh]);
          s = - g / maxWithSign(singularValues[kCh]);
          ApplyRotation(singularVectors_Left, c, s, 0, 0, &f1, &f2, kCh, split, nChannels);
        }
      }

      if (convergence) { 
        singularValues[iCh] = (float) singularValues[iCh];
        if (singularValues[iCh] < 0.0f)  {
          singularValues[iCh] = -singularValues[iCh];
          for (nCh = 0; nCh < nChannels; nCh++) {
            singularVectors_Right[nCh][iCh] = -singularVectors_Right[nCh][iCh];
          }
        }
      } else {
        if (iteration >= SAOC_MAX_NUM_ITERATION) {
          if (singularValues[iCh] < 0.0f)  {
            singularValues[iCh] = -singularValues[iCh];
            for (nCh = 0; nCh < nChannels; nCh++) {
              singularVectors_Right[nCh][iCh] = -singularVectors_Right[nCh][iCh];
            }
          }
		      error = ERROR_NO_CONVERGENCE;
          convergence = 1;
        } else {
          ApplyQRTransform(singularVectors_Left,singularValues,singularVectors_Right,secDiag,jCh,iCh,nChannels);
        }
      }
    }
  }
  return(error);
}

static void ApplyQRTransform( float singularVectors_Left[][SAOC_MAX_CHANNELS],
                             float singularValues[SAOC_MAX_CHANNELS],
                             float singularVectors_Right[][SAOC_MAX_CHANNELS],
                             float secDiag[SAOC_MAX_CHANNELS],
                             int startIndex, 
                             int currentIndex, 
                             int nChannels)
{

  int ch, split;
  float  d, g, r, x_ii, x_split, x_kk, mu, aux;
  float c = 1.0f;
  float s = 1.0f;

  x_kk = singularValues[currentIndex];
  x_ii = singularValues[startIndex];
  split = currentIndex - 1;

  x_split = singularValues[split];
  g = secDiag[split];
  r = secDiag[currentIndex];

  d = (x_split + x_kk) * (x_split - x_kk) + (g + r) * (g - r);
  d /= maxWithSign((r + r) * x_split);

  g = GivensRotation(1.0f, d);
  mu =  (float) (x_split / maxWithSign( d + (d >= 0.0f ? 1 : (-1)) * (float) fabs(g)) - r);
  d = (  (x_ii + x_kk) * (x_ii - x_kk) + r * mu ) / maxWithSign(x_ii);  

  /*QR transformation*/
  for (ch = startIndex; ch <= split; ch++) {
    r = s * secDiag[ch + 1];
    g = c * secDiag[ch + 1];
    
    secDiag[ch] = GivensRotation(d, r);
    c = d / maxWithSign(secDiag[ch]);
    s = r / maxWithSign(secDiag[ch]);

    r = s * singularValues[ch + 1];
    x_split = c *  singularValues[ch + 1] ;
    aux = g;
    ApplyRotation(singularVectors_Right, c, s, x_ii, aux, &d, &g, ch + 1, ch, nChannels);

    singularValues[ch] = GivensRotation(d, r);
    if (fabs(singularValues[ch]) > FLT_EPSILON * fabs(singularValues[ch])) {
      aux = 1.0f / singularValues[ch];  
      c = d * aux; 
      s = r * aux;
    }
    ApplyRotation(singularVectors_Left, c, s, g, x_split, &d, &x_ii, ch + 1, ch, nChannels);
  }
  secDiag[startIndex] = 0.0f;
  secDiag[currentIndex] = d;
  singularValues[currentIndex] = x_ii;
}


static void ApplyRotation(float singularVector[][SAOC_MAX_CHANNELS],
                          float c,
                          float s,
                          float x11,
                          float x12,
                          float *d,
                          float *g,
                          int currentIndex1,
                          int currentIndex2,
                          int nChannels)
{
  int ch;
  *d = c * x11 + s * x12;
  *g = c * x12 - s * x11;
  for (ch = 0; ch < nChannels; ch++)  {
    x11 = singularVector[ch][currentIndex2];
    x12 = singularVector[ch][currentIndex1];
    singularVector[ch][currentIndex2] =  (c * x11 + s * x12);
    singularVector[ch][currentIndex1] =  (c * x12 - s * x11);
  }
}

static int HouseholderReduction( float singularVecors_Left[][SAOC_MAX_CHANNELS],
                                 float singularValues[SAOC_MAX_CHANNELS],
                                 float singularVecors_Right[][SAOC_MAX_CHANNELS],
                                 float secDiag[SAOC_MAX_CHANNELS],
                                 int nChannels,
                                 float *eps_x)
{
    int nCh;
    float g = 0.0f, sig_x = 0.0f;

    for (nCh = 0; nCh < nChannels; nCh++) {
      biDiagonalReduction(singularVecors_Left, singularValues, secDiag, nChannels, nCh, &sig_x, &g, 1);
      biDiagonalReduction(singularVecors_Left, singularValues, secDiag, nChannels, nCh, &sig_x, &g, 0);
      *eps_x = (float) max(*eps_x, (fabs(singularValues[nCh]) + fabs(secDiag[nCh])));
    }
  
    singularVecorsAccumulation(singularVecors_Left, singularVecors_Right, singularValues, secDiag, nChannels, 0);
    singularVecorsAccumulation(singularVecors_Left, singularVecors_Right, singularValues, secDiag, nChannels, 1);

  return 1;
}

static int biDiagonalReduction( float singularVecors[][SAOC_MAX_CHANNELS],
                                float singularValues[SAOC_MAX_CHANNELS],
                                float secDiag[SAOC_MAX_CHANNELS],
                                int nChannels,
                                int currChannel,
                                float *sig_x,
                                float *g,
                                int leftHand)
{
  int iCh, jCh, idx;
  float norm_x, f, r;

  if(leftHand) {
    secDiag[currChannel] = (*sig_x) * (*g);
  }
  (*sig_x) = 0.0f;
  (*g) = 0.0f;
  if(leftHand || currChannel < nChannels - 1) {
    idx = currChannel;
    if(!leftHand) {
      idx++;
    }
    for (jCh = idx; jCh < nChannels; jCh++) { 
      if(leftHand) {
        (*sig_x) += (float)fabs(singularVecors[jCh][currChannel]);
      }
      else {
        (*sig_x) += (float)fabs(singularVecors[currChannel][jCh]);
      }
    }
    if ((*sig_x)) { /*(fabs(*sig_x) > FLT_EPSILON * fabs(*sig_x)) { */ 
      norm_x = 0.0f;
      for (jCh = idx; jCh < nChannels; jCh++) {
        if(leftHand) {
          singularVecors[jCh][currChannel] = (singularVecors[jCh][currChannel]/maxWithSign((*sig_x)));
          norm_x += (singularVecors[jCh][currChannel] * singularVecors[jCh][currChannel]);
        }
        else {
          singularVecors[currChannel][jCh] = (singularVecors[currChannel][jCh]/maxWithSign((*sig_x)));
          norm_x += (singularVecors[currChannel][jCh] * singularVecors[currChannel][jCh]);
        }
      }
      (*g) =   (float) (-  ( singularVecors[currChannel][idx] >=0? 1:(-1)) * sqrt(norm_x)  );
      r = (*g) * singularVecors[currChannel][idx] - norm_x;
      singularVecors[currChannel][idx] = (singularVecors[currChannel][idx] - (*g));

      if(!leftHand) {
        for (jCh = idx; jCh < nChannels; jCh++) { 
          secDiag[jCh] = singularVecors[currChannel][jCh] / maxWithSign(r);
        }
      }
      if(currChannel < nChannels - 1) {
        for (iCh = currChannel + 1; iCh < nChannels; iCh++) {
          norm_x = 0.0f;
          for (jCh = idx; jCh < nChannels; jCh++) {
            if(leftHand) {
              norm_x += (singularVecors[jCh][currChannel] * singularVecors[jCh][iCh]);
            }
            else {
              norm_x += (singularVecors[iCh][jCh] * singularVecors[currChannel][jCh]);
            }
          }
          if(leftHand) {
            f = norm_x / maxWithSign(r);
          }
          for (jCh = idx; jCh < nChannels; jCh++) { 
            if(leftHand) {
              singularVecors[jCh][iCh] += (f * singularVecors[jCh][currChannel]);
            }
            else {
              singularVecors[iCh][jCh] += (norm_x * secDiag[jCh]);
            }
          }
        }
      }
      for (jCh = idx; jCh < nChannels; jCh++) {
        if(leftHand) {
          singularVecors[jCh][currChannel] = (singularVecors[jCh][currChannel]*(*sig_x));
        }
        else {
          singularVecors[currChannel][jCh] = (singularVecors[currChannel][jCh]*(*sig_x));
        }
      }
    }
    if(leftHand) {
      singularValues[currChannel] = ((*sig_x) * (*g));
    }
  }
  return 1;
}


static int singularVecorsAccumulation( float singularVecors_Left[][SAOC_MAX_CHANNELS],
                                       float singularVecors_Right[][SAOC_MAX_CHANNELS],
                                       float singularValues[SAOC_MAX_CHANNELS],
                                       float secDiag[SAOC_MAX_CHANNELS],
                                       int nChannels,
                                       int leftHand)
{
  int nCh, iCh, k;
  float norm_y, t_jj, t_ii, ratio;

  for (nCh = nChannels - 1; nCh >= 0; nCh--) { 
    if ( !leftHand) {
      if (nCh < nChannels - 1) {
        if (t_ii) {
          for (iCh = nCh + 1; iCh < nChannels; iCh++) {
            ratio = singularVecors_Left[nCh][iCh] / maxWithSign(singularVecors_Left[nCh][nCh + 1]);
            singularVecors_Right[iCh][nCh] = ratio / maxWithSign(t_ii);
          }
          for (iCh = nCh + 1; iCh < nChannels; iCh++) {
            norm_y = 0.0f;
            for (k = nCh + 1; k < nChannels; k++) {
              norm_y += (singularVecors_Left[nCh][k] * singularVecors_Right[k][iCh]);
            }
            for (k = nCh + 1; k < nChannels; k++) {
              singularVecors_Right[k][iCh] += (norm_y * singularVecors_Right[k][nCh]);
            }
          }
        }
        for (iCh = nCh + 1; iCh < nChannels; iCh++) {
          singularVecors_Right[nCh][iCh] = singularVecors_Right[iCh][nCh] = 0.0f;
        }
      }
      singularVecors_Right[nCh][nCh] = 1.0f;
      t_ii = secDiag[nCh];
    }  else {
      t_ii = singularValues[nCh];
      if (nCh < nChannels - 1) 
        for (iCh = nCh + 1; iCh < nChannels; iCh++) {
          singularVecors_Left[nCh][iCh] = 0.0f;
        }
      if (t_ii) { 
        t_ii = 1.0f / maxWithSign(t_ii);
        if (nCh != nChannels - 1) {
          for (iCh = nCh + 1; iCh < nChannels; iCh++) {
            norm_y = 0.0f;
            for (k = nCh + 1; k < nChannels; k++) {
              norm_y += (singularVecors_Left[k][nCh] * singularVecors_Left[k][iCh]);
            }
            t_jj = t_ii * norm_y / maxWithSign(singularVecors_Left[nCh][nCh]) ;
            for (k = nCh; k < nChannels; k++) {
              singularVecors_Left[k][iCh] += (t_jj * singularVecors_Left[k][nCh]);
            }
          }
        }
        for (iCh = nCh; iCh < nChannels; iCh++) {
          singularVecors_Left[iCh][nCh] = (singularVecors_Left[iCh][nCh]*t_ii);
        }
      } else {
        for (iCh = nCh; iCh < nChannels; iCh++) {
          singularVecors_Left[iCh][nCh] = 0.0f;
        }
      }
      ++singularVecors_Left[nCh][nCh];
    }
  }
  return 1;
}


static float GivensRotation(float x, float z)
{
    float x_abs = (float) fabs(x); 
    float z_abs = (float) fabs(z); 
    float cotan, tan, r;

    if (x_abs <= FLT_EPSILON*x_abs && z_abs <= FLT_EPSILON*z_abs){ 
      r = 0.0f;
    } else if (x_abs >= z_abs) { 
      if (x_abs <= SAOC_SVD_EPS && z_abs <SAOC_SVD_EPS)
          r = 0.0f;
      cotan = z_abs / (x_abs); 
      r = x_abs * (float)sqrt(1.0f + cotan * cotan); 
    }
    else {
      tan = x_abs / (z_abs);  
      r = z_abs * (float)sqrt(1.0f + tan * tan); 
    }

    return(r);
}

static float maxWithSign(float a)
{
  if (fabs(a) > SAOC_SVD_EPS)
    return a;
  else
    if (a < 0.0f){
			return -SAOC_SVD_EPS;
		} else {
			return SAOC_SVD_EPS;
		}
}