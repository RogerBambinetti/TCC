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


#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#include "hierChnlGrouping.h"

#define SAOC_HACCED_EPS 0.01f

#ifndef SAOC_INT_MAX
#define SAOC_INT_MAX       2147483647    /* maximum (signed) int value */
#endif

#ifndef M_PI 
#define M_PI           3.14159265358979323846
#endif

#ifndef RM3_3D_BUGFIX_SAOC_1
#define RM3_3D_BUGFIX_SAOC_1
#endif

static int saoc_sign(float v);

/* Converting spherical coordinates to Cartesian Coordinates. 
   chnlAzimuths[]      - array of azimuth angles (in degrees)
   chnlElevations[]    - array of elevation angles (in degrees)
   cart[][3] - 2D array containing the calculated x,y,z coordinates 
   numElem   - the number of coordinates to be converted
*/
static void sph2cart(float chnlAzimuths[], float chnlElevations[], int r, float cart[][3], int numChnls)
{
  int i;
  float rcoselev;
  float deg2rad = (float) (M_PI/180);

  for (i=0; i<numChnls; i++) {
    cart[i][2] = r * ((float) sin(chnlElevations[i] * deg2rad));
    rcoselev = r * ((float) cos(chnlElevations[i] * deg2rad));
    cart[i][0] = rcoselev * ((float) cos(chnlAzimuths[i] * deg2rad));
    cart[i][1] = rcoselev * ((float) sin(chnlAzimuths[i] * deg2rad));
  }
  return;
}

/* Converting Cartesian coordinates to spherical Coordinates. 
   chnlAzimuths[]      - array of calculated azimuth angles (in degrees)
   chnlElevations[]    - array of calculated elevation angles (in degrees)
   cart[][3] - 2D array containing the x,y,z coordinates 
   numElem   - the number of coordinates to be converted
*/
static void cart2sph(float chnlAzimuths[], float chnlElevations[], float cart[][3], int numChnls) 
{
  int i;
  float hypotxy;
  float rad2deg = (float) (180/M_PI);

  for (i=0; i<numChnls; i++) {
    hypotxy = (float) sqrt(cart[i][0] * cart[i][0] + cart[i][1] * cart[i][1]);
    chnlElevations[i] = ((float) atan2(cart[i][2], hypotxy)) * rad2deg;
    chnlAzimuths[i] = ((float) atan2(cart[i][1], cart[i][0])) * rad2deg;
  }
  return;
}

static int saoc_sign(float v)
{
return v > 0 ? 1 : (v < 0 ? -1 : 0);
}

/* Checks whether element is in the groupingList. Returns 1 if found and
   0 if not found 
   groupingList[][2] - 2D array containing the grouping elements
   groupingListSize  - number of groups in groupingList
   element - element to look for
*/
static int isInGroupingList(int groupingList[][2], int groupingListSize, int element) {
  int i=0;

  for (i=0; i<groupingListSize; i++) {
    if (groupingList[i][0] == element || groupingList[i][1] == element)
      return 1;
    else
      return 0;
  }
  return 0;
}


int saoc_hierChnlGrouping(float chnlAzimuths[], float chnlElevations[], int numChnls, int numDecrltrs, 
                          int noInterMed, float heightScalar, float mergeMatrix[][SAOC_MAX_RENDER_CHANNELS])
{
  int i=0, j=0, k=0, a=0, b=0, index=0;
  int nMerges = 0;
  int nClusters = 0;
  int clustCount = 0;
  int tiePrefSide = 1; /* 0:left, 1:right */
  int nMinsAngle = 0;
  int nMinsAz = 0;
  int divider = 0;
  int nMinTteDelta = 0;
  int lengthRearGroups = 0;
  int numCandGroups = 0;
  int nEleMaxs = 0;
  int clustOne = 0;
  int clustTwo = 0;
  int minMemCount = 0;
  int nMinMemPoints = 0;
  int tmpIdx = 0;
  int groupingListSize = 0;
  int row_index = 0;
  int col_index = 0;
  int minAngleRowIn = 0;
  int minAngleColIn = 0;
  int minAngleRowOut = 0;
  int minAngleColOut = 0;
  int numNewMembers = 0;
  
  float minDistAngle = 0.f;
  float minDistAz = 0.f;
  float minTteDelta = 0.f;
  float maxEleDelta = 0.f;
  float tmpVal = 0.f;

  int aziSign[SAOC_MAX_RENDER_CHANNELS] = {0};
  int rearMedianChnls[SAOC_MAX_RENDER_CHANNELS] = {0};
  int minMaskAngle[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS] = {{0}};
  int clustLabels[SAOC_MAX_RENDER_CHANNELS] = {0};
  int uniLabels[SAOC_MAX_RENDER_CHANNELS] = {0};
  int cOneMembers[SAOC_MAX_RENDER_CHANNELS*SAOC_MAX_NUM_DECORRELATORS] = {0};
  int cTwoMembers[SAOC_MAX_RENDER_CHANNELS*SAOC_MAX_NUM_DECORRELATORS] = {0};
  int newMembers[SAOC_MAX_RENDER_CHANNELS*SAOC_MAX_NUM_DECORRELATORS*2] = {0};
  int thisMembers[SAOC_MAX_RENDER_CHANNELS] = {0};
  int minRowsAngle[SAOC_MAX_RENDER_CHANNELS] = {0};
  int minColsAngle[SAOC_MAX_RENDER_CHANNELS] = {0};
  int minRowsAz[SAOC_MAX_RENDER_CHANNELS] = {0};
  int minColsAz[SAOC_MAX_RENDER_CHANNELS] = {0};
  int groupingList[SAOC_MAX_RENDER_CHANNELS*SAOC_MAX_NUM_DECORRELATORS][2] = {{0}}; 
  int minPoints[SAOC_MAX_RENDER_CHANNELS] = {0};
  int azSide[SAOC_MAX_RENDER_CHANNELS] = {0};
  int rearGroups[SAOC_MAX_RENDER_CHANNELS] = {0};
  int candGroups[SAOC_MAX_RENDER_CHANNELS] = {0};
  int maxPoints[SAOC_MAX_RENDER_CHANNELS] = {0};
  int afterMergeMemberCount[SAOC_MAX_RENDER_CHANNELS] = {0};
  int minMemPoints[SAOC_MAX_RENDER_CHANNELS] = {0};

  float cart[SAOC_MAX_RENDER_CHANNELS][3] = {0.f};
  float xMat[SAOC_MAX_RENDER_CHANNELS][3] = {0.f};
  float xNorm[SAOC_MAX_RENDER_CHANNELS] = {0.f};
  float xProd[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS] = {{0.f}};
  float chnlAngleDists[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS] = {{0.f}};
  float azDiff[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS] = {{0.f}};
  float chnlAzDists[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS] = {{0.f}};
  float clustAngleDists[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS] = {{0.f}};
  float clustAzDists[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS] = {{0.f}};
  float newAngleCol[SAOC_MAX_RENDER_CHANNELS] = {0.f};
  float newAzCol[SAOC_MAX_RENDER_CHANNELS] = {0.f};
  float newCentroids[SAOC_MAX_RENDER_CHANNELS][3] = {0.f};
  float centroidAzDeg[SAOC_MAX_RENDER_CHANNELS] = {0.f};
  float centroidEleDeg[SAOC_MAX_RENDER_CHANNELS] = {0.f};
  float tteDeltaAngle[SAOC_MAX_RENDER_CHANNELS] = {0.f};
  float newAngleDists[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS] = {{0.f}};
  float newAzDists[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS] = {{0.f}};
  float xMatSum[1][3] = {0.f};
  
#ifdef RM3_3D_BUGFIX_SAOC_1
  for (i=0; i<numChnls; i++) {
    if (fabs(chnlAzimuths[i] - 180) < SAOC_HACCED_EPS) {
      chnlAzimuths[i]   = 0.0f;
      chnlElevations[i] = 180 - chnlElevations[i];
    }
  }
#endif

  /* Converting spherical coordinates to cartesian */
  sph2cart(chnlAzimuths, chnlElevations, 1, cart, numChnls);

  for (i=0; i<numChnls; i++) {
    xMat[i][0] = cart[i][0];
    xMat[i][1] = cart[i][1];
    xMat[i][2] = heightScalar * cart[i][2]; /* lower importance for the height-axis */

    xNorm[i] = (float) sqrt(xMat[i][0]*xMat[i][0] + xMat[i][1]*xMat[i][1] + xMat[i][2]*xMat[i][2]);
  }

  /* xMat*xMat' */
  for (i=0; i<numChnls; i++) {
    for (b=0; b<numChnls; b++) {
      for (a=0; a<3; a++){
        xProd[i][b] += xMat[i][a] * xMat[b][a];
      }
    }
  }

  /* length normalization */ 
  for (i=0; i<numChnls; i++) {
    for (j=0; j<numChnls; j++) {
      xProd[i][j] /= xNorm[j];
      xProd[j][i] /= xNorm[j];
    }
  }

  for (i=0; i<numChnls; i++) {
    for (j=0; j<numChnls; j++) {
      /* matlab: chnlAngleDists = max(0, 1 - xProd); */
      /*chnlAngleDists[i][j] = max(0, 1 - xProd[i][j]);*/

      /* matlab: chnlAngleDists = acos(min(1, xProd)) / pi * 180; */
      chnlAngleDists[i][j] = (float) (acos(min(1, xProd[i][j])) / M_PI * 180);

      /* matlab: azDiff = mod( repmat(chnlAzimuths, 1, nChnls) - repmat(chnlAzimuths.', nChnls, 1) , 360); */
      azDiff[i][j] = (chnlAzimuths[i] - chnlAzimuths[j]) - ((float) floor((chnlAzimuths[i] - chnlAzimuths[j]) / 360) * 360);
    }
  }

  /* agnostic of the direction
     matlab: chnlAzDists = min(azDiff, azDiff.'); */
  for (i=0; i<numChnls; i++) {
    for (j=0; j<numChnls; j++) {
      chnlAzDists[i][j] = min(azDiff[i][j], azDiff[j][i]);      
    }
  }

  /* noInterMed == 1 => prohibit mixing across the median plane */
  if (noInterMed == 1)
  {
    /* make the "azi=180" -channels to have azi=0 and add elevation */
    for (i=0; i<numChnls; i++) {
      if (chnlAzimuths[i] - 180 == 0) {
        aziSign[i] = 0;
        chnlAzimuths[i] = 0;
        chnlElevations[i] += 90;
        rearMedianChnls[index++] = i;
      }
      else
        aziSign[i] = saoc_sign(chnlAzimuths[i]);
    }

    for (i=0; i<index; i++) {
      for (j=0; j<index; j++) {
        chnlAzDists[rearMedianChnls[i]][rearMedianChnls[j]] = 0;
      }
    }

    for (i=0; i<numChnls; i++) {
      for (j=0; j<numChnls; j++) {
        if (aziSign[i] != aziSign[j]) {
          chnlAngleDists[j][i] = FLT_MAX; /* infinity */
	        chnlAzDists[j][i] = FLT_MAX;    /* infinity */
        }
      }
    }
  }

  /* number of cluster merges required */
  nMerges = numChnls - numDecrltrs;

  /* initialization */
  nClusters = numChnls;

  for (i=0; i<nClusters; i++) {
    uniLabels[i] = i+1;
    clustLabels[i] = uniLabels[i];
  }

  memcpy(clustAngleDists, chnlAngleDists, sizeof(chnlAngleDists));
  memcpy(clustAzDists, chnlAzDists, sizeof(chnlAzDists));

  /* retain only unique true pairs */
  index = 1;
  for (i=0; i<numChnls; i++) {
    for (j=0; j<index; j++) {
      clustAngleDists[i][j] = FLT_MAX;
      clustAzDists[i][j] = FLT_MAX;
    }
    index++;
  }

  clustCount = nClusters;

  for (k=0; k<nMerges; k++) {
    
    minDistAngle = FLT_MAX;
    nMinsAngle = 0;
 
    for (i=0; i<clustCount; i++) {
      for (j=0; j<clustCount; j++) {
        if (clustAngleDists[i][j] < minDistAngle)
          minDistAngle = clustAngleDists[i][j];
      }
    }

    if (minDistAngle == FLT_MAX)
      return -1;

    memset(minRowsAngle, 0, sizeof(minRowsAngle));
    memset(minColsAngle, 0, sizeof(minColsAngle));

    for (i=0; i<clustCount; i++) {
      for (j=0; j<clustCount; j++) {
        if (clustAngleDists[i][j] - minDistAngle < SAOC_HACCED_EPS) {
          minRowsAngle[nMinsAngle] = i;
          minColsAngle[nMinsAngle] = j;
          groupingList[nMinsAngle][0] = i;
          groupingList[nMinsAngle++][1] = j;
        }
      }
    }
    groupingListSize = nMinsAngle;

    if (nMinsAngle > 1) {
      /* heuristics to solve competing clusterings */
      minDistAz = FLT_MAX;
      for (i=0; i<nMinsAngle; i++) {
        if (clustAzDists[minRowsAngle[i]][minColsAngle[i]] < minDistAz)
          minDistAz = clustAzDists[minRowsAngle[i]][minColsAngle[i]];
      }

      nMinsAz = 0;
      memset(groupingList, 0, sizeof(groupingList));
      memset(minRowsAz, 0, sizeof(minRowsAz));
      memset(minColsAz, 0, sizeof(minColsAz));
      for (i=0; i<nMinsAngle; i++) {
        if (clustAzDists[minRowsAngle[i]][minColsAngle[i]] - minDistAz < SAOC_HACCED_EPS) {
            minRowsAz[nMinsAz] = minRowsAngle[i];
            minColsAz[nMinsAz] = minColsAngle[i];
            groupingList[nMinsAz][0] = minRowsAz[nMinsAz];
            groupingList[nMinsAz][1] = minColsAz[nMinsAz++];
        }
      }
      groupingListSize = nMinsAz;

      if (nMinsAz > 1) {
        /* non-unique decision from angular distance, non-unique
			     decision from added azimuth distance */

        /* candidate merges: calculate the resulting cluster centroids */
        memset(newCentroids, 0, sizeof(newCentroids));
        for (i=0; i<nMinsAz; i++) {
          for (j=0; j<nClusters; j++) {
            if (clustLabels[j] == uniLabels[groupingList[i][0]] || 
                clustLabels[j] == uniLabels[groupingList[i][1]]) {
              xMatSum[0][0] += xMat[j][0];
              xMatSum[0][1] += xMat[j][1];
              xMatSum[0][2] += xMat[j][2];
              divider++;
            }
          }
          newCentroids[i][0] = xMatSum[0][0]/divider;
          newCentroids[i][1] = xMatSum[0][1]/divider;
          newCentroids[i][2] = xMatSum[0][2]/divider;

          xMatSum[0][0] = 0;
          xMatSum[0][1] = 0;
          xMatSum[0][2] = 0;
          divider = 0;
        } 

        /* Converting cartesian coordinates to spherical */
        cart2sph(centroidAzDeg, centroidEleDeg, newCentroids, nMinsAz);

        /* difference of the new centroid from the through-the-ears-axis */
        minTteDelta = FLT_MAX;
        memset(tteDeltaAngle, 0, sizeof(tteDeltaAngle));
        for (i=0; i<nMinsAz; i++) {
          tteDeltaAngle[i] = min((float) fabs(centroidAzDeg[i] - 90.0f), (float) fabs(centroidAzDeg[i] + 90.0f));
          if (tteDeltaAngle[i] < minTteDelta)
            minTteDelta = tteDeltaAngle[i];
        }

        index = 0;
        nMinTteDelta = 0;
        memset(minPoints, 0, sizeof(minPoints));
        for (i=0; i<nMinsAz; i++) {
          if ((float) fabs(tteDeltaAngle[i] - minTteDelta) < SAOC_HACCED_EPS) {
              nMinTteDelta++;
              minPoints[index++] = i;
          }
        }

        if (nMinTteDelta > 1) {
          for (i=nMinTteDelta-1; i>0; i--) {
            memmove(groupingList[minPoints[i-1] +1], groupingList[minPoints[i]], (sizeof(int) * 2 * (SAOC_MAX_RENDER_CHANNELS - minPoints[i] - 1)));
          }
        }
        
        memmove(groupingList, groupingList[minPoints[0]], (sizeof(int) * 2 * (SAOC_MAX_RENDER_CHANNELS - minPoints[0])));
        memset(groupingList[nMinTteDelta], 0, sizeof(int)*2*(SAOC_MAX_RENDER_CHANNELS-nMinTteDelta-1));

        groupingListSize = nMinTteDelta;

        if (nMinTteDelta > 1) {

          /* not unique decision */
          index = 0;
          lengthRearGroups = 0;
          memset(rearGroups, 0, sizeof(rearGroups));
          for (i=0; i<nMinTteDelta; i++) {
            if ((centroidAzDeg[minPoints[i]] < 0 && centroidAzDeg[minPoints[i]] < -90) ||
              (centroidAzDeg[minPoints[i]] >= 0 && centroidAzDeg[minPoints[i]] > 90)) {
                azSide[i] = 1;
                rearGroups[index++] = i;
                lengthRearGroups++;
            } 
          }

          if (lengthRearGroups == 1) {
            /* unique decision based on the rear/front -preference */
            
            memmove(groupingList, groupingList[rearGroups[0]], (sizeof(int)*2*(SAOC_MAX_RENDER_CHANNELS-rearGroups[0])));
            memset(groupingList[rearGroups[0]+1], 0, sizeof(int)*2*(SAOC_MAX_RENDER_CHANNELS-rearGroups[0]-1));

            groupingListSize = lengthRearGroups;
          }
          else {
            /* not unique decision */
            memset(candGroups, 0, sizeof(candGroups));
            if (lengthRearGroups == 0) {
              /* all candidates in front */
              memcpy(candGroups, minPoints, sizeof(minPoints));
              numCandGroups = nMinTteDelta;
            }
            else {
              /* select the rear candidates */
              index=0;
              for (i=0; i<nMinTteDelta; i++) {
                candGroups[index++] = rearGroups[minPoints[i]];
              }
              numCandGroups = index;

              for (i=lengthRearGroups-1; i>0; i--) {
                memmove(groupingList[rearGroups[i-1]+1], groupingList[rearGroups[i]], (sizeof(int) * 2 * (SAOC_MAX_RENDER_CHANNELS - rearGroups[i]-1)));
              }
              
              memmove(groupingList, groupingList[rearGroups[0]], (sizeof(int) * 2 * (SAOC_MAX_RENDER_CHANNELS - rearGroups[0])));
              memset(groupingList[lengthRearGroups], 0, sizeof(int)*2*(SAOC_MAX_RENDER_CHANNELS-lengthRearGroups-1));

              groupingListSize = lengthRearGroups;
            }

            maxEleDelta = 0;
            for (i=0; i<numCandGroups; i++) {
              if ((float) fabs(centroidEleDeg[candGroups[i]]) > maxEleDelta)
                maxEleDelta = (float) fabs(centroidEleDeg[candGroups[i]]);
            }

            index = 0;
            memset(maxPoints, 0, sizeof(maxPoints));
            for (i=0; i<numCandGroups; i++) {
              if ((float) fabs((float) fabs(centroidEleDeg[candGroups[i]]) - maxEleDelta) < SAOC_HACCED_EPS)
                maxPoints[index++] = i;
            }
            nEleMaxs = index;

            for (i=nEleMaxs-1; i>0; i--) {
                memmove(groupingList[maxPoints[i-1]+1], groupingList[maxPoints[i]], (sizeof(int) * 2 * (SAOC_MAX_RENDER_CHANNELS - maxPoints[i]-1)));
            }
            
            memmove(groupingList, groupingList[maxPoints[0]], (sizeof(int) * 2 * (SAOC_MAX_RENDER_CHANNELS - maxPoints[0])));
            memset(groupingList[nEleMaxs], 0, sizeof(int)*2*(SAOC_MAX_RENDER_CHANNELS-nEleMaxs-1));

            groupingListSize = nEleMaxs;

            if (nEleMaxs > 1) {
              /* not unique decision -> decision based on cluster
						     member count */

              minMemCount = SAOC_INT_MAX;
              memset(afterMergeMemberCount, 0, sizeof(afterMergeMemberCount));
              for (i=0; i<nEleMaxs; i++) {
                clustOne = uniLabels[groupingList[i][0]];
							  clustTwo = uniLabels[groupingList[i][1]];

                for (j=0; j<nClusters; j++) {
                  if (clustLabels[j] == clustOne || clustLabels[j] == clustTwo)
                    afterMergeMemberCount[i]++;
                }

                if (afterMergeMemberCount[i] < minMemCount)
                  minMemCount = afterMergeMemberCount[i];
              }

              index = 0;
              memset(minMemPoints, 0, sizeof(minMemPoints));
              for (i=0; i<nEleMaxs; i++) {
                if (abs(afterMergeMemberCount[i] - minMemCount) < SAOC_HACCED_EPS)
                  minMemPoints[index++] = i;
              }
              nMinMemPoints = index;

              if (nMinMemPoints > 1) {
                for (i=nMinMemPoints-1; i>0; i--) {
                  memmove(groupingList[minMemPoints[i-1]+1], groupingList[minMemPoints[i]], (sizeof(int) * 2 * (SAOC_MAX_RENDER_CHANNELS - minMemPoints[i]-1)));
                }
              }
              
              memmove(groupingList, groupingList[minMemPoints[0]], (sizeof(int) * 2 * (SAOC_MAX_RENDER_CHANNELS - minMemPoints[0])));
              memset(groupingList[nMinMemPoints], 0, sizeof(int)*2*(SAOC_MAX_RENDER_CHANNELS-nMinMemPoints-1));

              groupingListSize = nMinMemPoints;

              if (nMinMemPoints > 1) {
                /* not unique decision
							     select the merge right/left/right/left/etc */
                tmpVal = 0;
                if (tiePrefSide == 1) {
                  for (i=0; i<nEleMaxs; i++) {
                    if (centroidAzDeg[candGroups[maxPoints[i]]] > tmpVal) {
                      tmpVal = centroidAzDeg[candGroups[maxPoints[i]]];
                      tmpIdx = i;
                    }
                  }
                  tiePrefSide = 0;
                }
                else
                {
                  tmpVal = FLT_MAX;
                  for (i=0; i<nEleMaxs; i++) {
                    if (centroidAzDeg[candGroups[maxPoints[i]]] < tmpVal) {
                      tmpVal = centroidAzDeg[candGroups[maxPoints[i]]];
                      tmpIdx = i;
                    }
                  }
                  tiePrefSide = 1;
                }

                memmove(groupingList, groupingList[tmpIdx], (sizeof(int)*2*(SAOC_MAX_RENDER_CHANNELS-tmpIdx)));
                memset(groupingList[tmpIdx+1], 0, sizeof(int)*2*(SAOC_MAX_RENDER_CHANNELS-tmpIdx-1));

                groupingListSize = 1;
              }
            }
          }
        }
      }
    }

    row_index = 0;
    col_index = 0;
    memset (cOneMembers, 0, sizeof(cOneMembers));
    memset (cTwoMembers, 0, sizeof(cTwoMembers));
    for (i=0; i<nClusters; i++) { 
      for (j=0; j<groupingListSize; j++) {
        if (clustLabels[i] == uniLabels[groupingList[j][0]])
            cOneMembers[row_index++] = i;
        if (clustLabels[i] == uniLabels[groupingList[j][1]])
            cTwoMembers[col_index++] = i;
      }
    }

    memset (newMembers, 0, sizeof(newMembers));
    for (i=0; i<row_index; i++) {
      newMembers[i] = cOneMembers[i];
      clustLabels[cOneMembers[i]] = nClusters + k+1;
    }
    for(i=0; i<col_index; i++) {
      newMembers[i+row_index] = cTwoMembers[i];
      clustLabels[cTwoMembers[i]] = nClusters + k+1;
    }

    numNewMembers = row_index + col_index;

    /* matlab: uniLabels = unique(clustLabels);*/
    index = 0;
    a = 0;

    /* finding the unique values */
    for (i=0; i<nClusters; i++) {
      uniLabels[i] = 0;
      a = 0;
      for (j=0; j<index; j++) {
        if (clustLabels[i] == uniLabels[j])
          a++;
      }
      if (a == 0)
        uniLabels[index++] = clustLabels[i];
    }

    /* bubble sort */
    for (i=0; i<(index-1); i++) {
      for (j=0; j<(index-i-1); j++) {
        if (uniLabels[j] > uniLabels[j+1])
        {
          a = uniLabels[j];
          uniLabels[j] = uniLabels[j+1];
          uniLabels[j+1] = a;
        }
      }
    }


    row_index = 0;
    col_index = 0;
    minAngleRowOut = 0;
    minAngleColOut = 0;
    minAngleRowIn = 0;
    minAngleRowOut = 0;

    memset(newAngleDists, 0, sizeof(newAngleDists));
    memset(newAzDists, 0, sizeof(newAzDists));
    for (i=0; i<numChnls; i++) {
      if (isInGroupingList(groupingList, groupingListSize, i) == 0) {
        for (j=0; j<numChnls; j++) {
          if (isInGroupingList(groupingList, groupingListSize, j) == 0){
            newAngleDists[row_index][col_index] = clustAngleDists[i][j];
            newAzDists[row_index][col_index] = clustAzDists[i][j];
            col_index++;
          }
          else {
            if (j == groupingList[minAngleRowIn][minAngleColIn])
              minAngleColIn++;
            else
              minAngleRowIn++;
          }
        }
        row_index++;
      }
      else {
        if (i == groupingList[minAngleRowOut][minAngleColOut])
          minAngleRowOut++;
        else
          minAngleColOut++;
      }

      minAngleColIn = 0;
      minAngleRowIn = 0;
      col_index = 0;
    }

    clustCount--;

    for (j=0; j<clustCount - 1; j++) {
      newAngleCol[j] = FLT_MAX;
      newAzCol[j] = FLT_MAX;
    }

    /* update the distance matrices */
    index = 0;
    for (i=0; i<clustCount - 2; i++) {
      newAngleCol[i] = 0;
      newAzCol[i] = 0;
      for (j=0; j<nClusters; j++) {
        if (clustLabels[j] == uniLabels[i])
          thisMembers[index++] = j;
      }
      for (j=0; j<index; j++) {
        row_index = thisMembers[j];
        for (a=0; a<index; a++) {
          col_index = thisMembers[a];
          if (chnlAngleDists[row_index][col_index] > newAngleCol[i])
            newAngleCol[i] = chnlAngleDists[row_index][col_index];
          if (chnlAzDists[row_index][col_index] > newAzCol[i])
            newAzCol[i] = chnlAzDists[row_index][col_index];
        }
        for (a=0; a<numNewMembers; a++) {
          col_index = newMembers[a];
          if (chnlAngleDists[row_index][col_index] > newAngleCol[i])
            newAngleCol[i] = chnlAngleDists[row_index][col_index];
          if (chnlAzDists[row_index][col_index] > newAzCol[i])
            newAzCol[i] = chnlAzDists[row_index][col_index];
        }
      }
      for (j=0; j<numNewMembers; j++) {
        row_index = newMembers[j];
        for (a=0; a<index; a++) {
          col_index = thisMembers[a];
          if (chnlAngleDists[row_index][col_index] > newAngleCol[i])
            newAngleCol[i] = chnlAngleDists[row_index][col_index];
          if (chnlAzDists[row_index][col_index] > newAzCol[i])
            newAzCol[i] = chnlAzDists[row_index][col_index];
        }
        for (a=0; a<numNewMembers; a++) {
          col_index = newMembers[a];
          if (chnlAngleDists[row_index][col_index] > newAngleCol[i])
            newAngleCol[i] = chnlAngleDists[row_index][col_index];
          if (chnlAzDists[row_index][col_index] > newAzCol[i])
            newAzCol[i] = chnlAzDists[row_index][col_index];
         }
      }
      index = 0;
    }

    for (i=0; i<clustCount-1; i++) {
      newAngleDists[i][clustCount-1] = newAngleCol[i];
      newAngleDists[clustCount-1][i] = FLT_MAX;
      newAzDists[i][clustCount-1] = newAzCol[i];
      newAzDists[clustCount-1][i] = FLT_MAX;
    }
    newAngleDists[clustCount-1][i] = FLT_MAX;
    newAzDists[clustCount-1][i] = FLT_MAX;

    memcpy(clustAngleDists, newAngleDists, sizeof(newAngleDists));
	  memcpy(clustAzDists, newAzDists, sizeof(newAzDists));
  }

  index = 0;
  for (i=0; i<numDecrltrs; i++) {
    for (j=0; j<nClusters; j++) {
      if (clustLabels[j] == uniLabels[i]) {
        thisMembers[index++] = j;
        mergeMatrix[i][j] = 1.0f;
      }
    }
  }
  return 0;
}
