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


#include "saoc_const.h"
#include "error.h"

/* Hierarchical Agglomerative Channel ClustEring for Decorrelation - HACCED

   Agglomerative hierarchical clustering with complete linkage function for
   combining N loudspeaker channels into numGroups groups for decorrelation.

   INPUTS:
    chnlAzimuths[]   - array of loudspeaker azimuth angle in degrees (range: -180..+180)
    chnlElevations[] - array of loudspeaker elevation angles in degrees (range: -90..+90)
    numChnls         - number of channels
    numDecrltrs      - number of decorrelators => resulting groups. default=11
    noInterMed       - prohibit grouping across median plane (and on it). default=1
    heightScaler     - weighting factor for the height when determining the
                      3D-angle. default=0.5
 
   OUTPUTS:
    mergeMatrix[][]  - numDecrltrs*numChnls binary matrix defining the grouping
   
   ALGORITHM:
     (Prohibit grouping across the median plane, and the loudspeakers on the
     median plane are treated separately.)
     0. Each loudspeaker specifies an initial cluster.
     1. Merge clusters with the smallest 3D angular distance.
     2. If not unique, merge clusters with the smallest azimuth distance.
     3. If not unique, select merge in which the centroid is closer to the
        through-the-ears -axis.
     4. If not unique, select merge in which the centroid is further rear.
     5. If not unique, select merge further away from the horizontal plane.
     6. If not unique, select merge resulting into a smaller new cluster.
     7. If not unique, select merge with the new centroid more on right,
        then next time more on left, then again right, etc.
*/


int saoc_hierChnlGrouping(float az[], float elev[], int numChnls, int numDecrltrs, int noInterMed, float heightScalar, float mergeMatrix[][SAOC_MAX_RENDER_CHANNELS]);
