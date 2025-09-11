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


#include <math.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "saoc_decode.h"
#include "saoc_kernels.h"
#include "saoc_svd_tool.h"

float RG_Interp[SAOC_MAX_TIME_SLOTS][SAOC_MAX_HYBRID_BANDS][SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS];
float P1_Interp[SAOC_MAX_TIME_SLOTS][SAOC_MAX_HYBRID_BANDS][SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS];
float P2_Interp[SAOC_MAX_TIME_SLOTS][SAOC_MAX_HYBRID_BANDS][SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS];


static void saoc_pinv(float in[SAOC_MAX_GROUP_DMX_CHANNELS][SAOC_MAX_GROUP_DMX_CHANNELS], 
                      float pinv[SAOC_MAX_GROUP_DMX_CHANNELS][SAOC_MAX_GROUP_DMX_CHANNELS],
                      int numDmx,
						          float relative_Thresh,
                      float absolute_Thresh,
                      int omiteigvecs);

static void LimitedCovarianceAdjustment( float CovRenTarget[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS],
                                         float CovRenDry[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS],
                                         float CovRenWet[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS],
                                         int numOutCh,
                                         float relative_Thresh,
                                         float absolute_Thresh,
                                         float P_wet[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS]);


static int getSaocCoreGrouping(float Dmx[][SAOC_MAX_OBJECTS],
							   int RelatedToMatrix[SAOC_MAX_OBJECTS][SAOC_MAX_OBJECTS],
                               int numChannels,
                               int numObjects,
                               int numDmxChannels,
                               int numDmxObjects,
                               int combinedFlag,
                               saocCoreGroup *channelsGroup,
                               saocCoreGroup *objectsGroup);

static void computeEnergyMatrix(float pOld[SAOC_MAX_OBJECTS][SAOC_MAX_PARAM_BANDS],
                                float pIoc[SAOC_MAX_OBJECTS][SAOC_MAX_OBJECTS][SAOC_MAX_PARAM_BANDS],
                                int objectOffset,
                                int paramBand,
                                float E[SAOC_MAX_OBJECTS][SAOC_MAX_OBJECTS],
                                saocCoreGroup objectsGroup)
{
  int iObj, jObj, iGrp, objIdx, objIdx2;

  for (iGrp = 0; iGrp < objectsGroup.numGroups; iGrp++){  
    for (objIdx = 0; objIdx < objectsGroup.saocGroup[iGrp].numObj; objIdx++) {
      iObj = objectOffset + objectsGroup.saocGroup[iGrp].objList[objIdx];
      
      E[iObj][iObj] = pOld[iObj][paramBand] * pIoc[iObj][iObj][paramBand];
      
      for (objIdx2 = 0; objIdx2 < objectsGroup.saocGroup[iGrp].numObj; objIdx2++) {
        jObj = objectOffset + objectsGroup.saocGroup[iGrp].objList[objIdx2];
        
        E[iObj][jObj] = (float)sqrt(pOld[iObj][paramBand] * pOld[jObj][paramBand]) * pIoc[iObj][jObj][paramBand];
        E[jObj][iObj] = E[iObj][jObj];
      }
    }
  }
}

static void computeUnmixMatrix( float E[SAOC_MAX_OBJECTS][SAOC_MAX_OBJECTS],
                                float Dmx[SAOC_MAX_DMX_CHANNELS][SAOC_MAX_OBJECTS],
                                float pRenderMat[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_OBJECTS][SAOC_MAX_PARAMETER_BANDS],
                                int objectOffset,
                                int dmxChannOffset,
                                int numOutCh,
                                float RG[SAOC_MAX_PARAM_SETS][SAOC_MAX_PARAMETER_BANDS][SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS], 
                                float CovRenDry[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS],
                                int iset,
                                int paramBand,
                                saocCoreGroup objectsGroup,
                                float relative_Thresh,
                                float absolute_Thresh,
                                int omiteigvecs,
                                int dualModeFlag)
{
  int iGrp, iObj, jObj, iDmx, jDmx, dmxIdx, dmxIdx2, iUpmx, jUpmx, objIdx, objIdx2;
  float pinvDmxCovMat[SAOC_MAX_GROUP_DMX_CHANNELS][SAOC_MAX_GROUP_DMX_CHANNELS] = {{0}};
  float DmxCovMat[SAOC_MAX_GROUP_DMX_CHANNELS][SAOC_MAX_GROUP_DMX_CHANNELS] = {{0}};   /* DmxCovMat = D*E*D' */
  float DmxCovMat_DualMode[SAOC_MAX_GROUP_DMX_CHANNELS][SAOC_MAX_GROUP_DMX_CHANNELS] = {{0}};
  float G[SAOC_MAX_GROUP_OBJECTS][SAOC_MAX_GROUP_DMX_CHANNELS] = {{0}};
	float ED[SAOC_MAX_GROUP_OBJECTS][SAOC_MAX_GROUP_DMX_CHANNELS] = {{0}};

  for (iGrp = 0; iGrp < objectsGroup.numGroups; iGrp++){

    for (dmxIdx = 0; dmxIdx < objectsGroup.saocGroup[iGrp].numDmx; dmxIdx++){
      iDmx = dmxChannOffset + objectsGroup.saocGroup[iGrp].dmxList[dmxIdx];

      for (dmxIdx2 = dmxIdx; dmxIdx2 < objectsGroup.saocGroup[iGrp].numDmx; dmxIdx2++){   
        jDmx = dmxChannOffset + objectsGroup.saocGroup[iGrp].dmxList[dmxIdx2];
        
        DmxCovMat[dmxIdx][dmxIdx2] = 0.0f;      
			  for (objIdx = 0; objIdx < objectsGroup.saocGroup[iGrp].numObj; objIdx++) {
          iObj = objectOffset + objectsGroup.saocGroup[iGrp].objList[objIdx];
			    
					ED[objIdx][dmxIdx2] = 0.0f;
          for (objIdx2 = 0; objIdx2 < objectsGroup.saocGroup[iGrp].numObj; objIdx2++) {
            jObj = objectOffset + objectsGroup.saocGroup[iGrp].objList[objIdx2];

						ED[objIdx][dmxIdx2] += E[iObj][jObj] * Dmx[jDmx][jObj];
				  }  
					
          DmxCovMat[dmxIdx][dmxIdx2] += Dmx[iDmx][iObj] * ED[objIdx][dmxIdx2]; 						
        }
        DmxCovMat[dmxIdx2][dmxIdx] = DmxCovMat[dmxIdx][dmxIdx2];
      }
    }

    /*  the unmixing matrix G = E * D' * pinvDmxCovMat */
    if (dualModeFlag == 0) {
      
      saoc_pinv(DmxCovMat, pinvDmxCovMat, objectsGroup.saocGroup[iGrp].numDmx, relative_Thresh, absolute_Thresh, omiteigvecs);

	    for (objIdx = 0; objIdx < objectsGroup.saocGroup[iGrp].numObj; objIdx++) {
        iObj = objectOffset + objectsGroup.saocGroup[iGrp].objList[objIdx];

		    for (dmxIdx2 = 0; dmxIdx2 < objectsGroup.saocGroup[iGrp].numDmx; dmxIdx2++){
          jDmx = dmxChannOffset + objectsGroup.saocGroup[iGrp].dmxList[dmxIdx2];

			    G[objIdx][dmxIdx2] = 0.0f;
					
					for (dmxIdx = 0; dmxIdx < objectsGroup.saocGroup[iGrp].numDmx; dmxIdx++){
						iDmx = dmxChannOffset + objectsGroup.saocGroup[iGrp].dmxList[dmxIdx];

						G[objIdx][dmxIdx2] += ED[objIdx][dmxIdx] * pinvDmxCovMat[dmxIdx][dmxIdx2]; 					

			    }
		    }
	    }
    } else {
      for (dmxIdx = 0; dmxIdx < objectsGroup.saocGroup[iGrp].numDmx; dmxIdx++){
        iDmx = dmxChannOffset + objectsGroup.saocGroup[iGrp].dmxList[dmxIdx];
        
        for (dmxIdx2 = 0; dmxIdx2 < objectsGroup.saocGroup[iGrp].numDmx; dmxIdx2++){
          jDmx = dmxChannOffset + objectsGroup.saocGroup[iGrp].dmxList[dmxIdx2];
          
          DmxCovMat_DualMode[dmxIdx][dmxIdx2] = 0.0f;

	        for (objIdx = 0; objIdx < objectsGroup.saocGroup[iGrp].numObj; objIdx++) {
            iObj = objectOffset + objectsGroup.saocGroup[iGrp].objList[objIdx];
				      
            DmxCovMat_DualMode[dmxIdx][dmxIdx2] += Dmx[iDmx][iObj] * Dmx[jDmx][iObj]; 						
          }
          DmxCovMat_DualMode[dmxIdx2][dmxIdx] = DmxCovMat_DualMode[dmxIdx][dmxIdx2];
        }
      }

      saoc_pinv(DmxCovMat_DualMode, pinvDmxCovMat, objectsGroup.saocGroup[iGrp].numDmx, relative_Thresh, absolute_Thresh, omiteigvecs);
	    
      for (objIdx = 0; objIdx < objectsGroup.saocGroup[iGrp].numObj; objIdx++) {
        iObj = objectOffset + objectsGroup.saocGroup[iGrp].objList[objIdx];

		    for (dmxIdx2 = 0; dmxIdx2 < objectsGroup.saocGroup[iGrp].numDmx; dmxIdx2++){
          jDmx = dmxChannOffset + objectsGroup.saocGroup[iGrp].dmxList[dmxIdx2];

			    G[objIdx][dmxIdx2] = 0.0f;

				  for (dmxIdx = 0; dmxIdx < objectsGroup.saocGroup[iGrp].numDmx; dmxIdx++){
            iDmx = dmxChannOffset + objectsGroup.saocGroup[iGrp].dmxList[dmxIdx];

				    G[objIdx][dmxIdx2] += Dmx[iDmx][iObj] * pinvDmxCovMat[dmxIdx][dmxIdx2]; 						
			    }
		    }
	    }
    }

    /* Compute R*G in order to apply interpolation in GSAOCproc */
    for (iUpmx = 0; iUpmx < numOutCh ; iUpmx++){
      for (dmxIdx = 0; dmxIdx < objectsGroup.saocGroup[iGrp].numDmx; dmxIdx++){
        iDmx = dmxChannOffset + objectsGroup.saocGroup[iGrp].dmxList[dmxIdx];
        
        RG[iset][paramBand][iUpmx][iDmx] = 0.0f;
				
        for (objIdx = 0; objIdx < objectsGroup.saocGroup[iGrp].numObj; objIdx++) {
          iObj = objectOffset + objectsGroup.saocGroup[iGrp].objList[objIdx];
				  
          RG[iset][paramBand][iUpmx][iDmx] += pRenderMat[iUpmx][iObj][paramBand] * G[objIdx][dmxIdx]; 						
				}
      }
    }

		/* CovRenDry = R*G*DmxCovMat*G'*R' */
		if (dualModeFlag == 1) {
      for (iUpmx = 0; iUpmx < numOutCh ; iUpmx++){
        for (jUpmx = 0; jUpmx < numOutCh ; jUpmx++){
					for (dmxIdx = 0; dmxIdx < objectsGroup.saocGroup[iGrp].numDmx; dmxIdx++){
            iDmx = dmxChannOffset + objectsGroup.saocGroup[iGrp].dmxList[dmxIdx];
						
            CovRenDry[iUpmx][jUpmx] += RG[iset][paramBand][iUpmx][iDmx] * DmxCovMat[dmxIdx][dmxIdx] * RG[iset][paramBand][jUpmx][iDmx]; 						
					}
				}
			} 
		} else {
			for (iUpmx = 0; iUpmx < numOutCh ; iUpmx++){
        for (jUpmx = 0; jUpmx < numOutCh ; jUpmx++){
					for (dmxIdx = 0; dmxIdx < objectsGroup.saocGroup[iGrp].numDmx; dmxIdx++){
            iDmx = dmxChannOffset + objectsGroup.saocGroup[iGrp].dmxList[dmxIdx];
						
		        for (dmxIdx2 = 0; dmxIdx2 < objectsGroup.saocGroup[iGrp].numDmx; dmxIdx2++){
              jDmx = dmxChannOffset + objectsGroup.saocGroup[iGrp].dmxList[dmxIdx2];
							
              CovRenDry[iUpmx][jUpmx] += RG[iset][paramBand][iUpmx][iDmx] * DmxCovMat[dmxIdx][dmxIdx2] * RG[iset][paramBand][jUpmx][jDmx]; 
						}
					}
				}
			}  
		}
  }
}

static void computeTargetCovariance(saocCoreGroup objectsGroup,
                                    int objectOffset,
                                    int numOutCh,
                                    int paramBand,
                                    float E[SAOC_MAX_OBJECTS][SAOC_MAX_OBJECTS],
                                    float pRenderMat[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_OBJECTS][SAOC_MAX_PARAMETER_BANDS],
                                    float CovRenTarget[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS]) {

  int iGrp, iUpmx, jUpmx, iObj, jObj,  objIdx, objIdx2;

  for (iGrp = 0; iGrp < objectsGroup.numGroups; iGrp++){

    /* CovRenTarget = R*E*R' */
    for (iUpmx = 0; iUpmx < numOutCh ; iUpmx++){
      for (jUpmx = iUpmx; jUpmx < numOutCh ; jUpmx++){
				
        for (objIdx = 0; objIdx < objectsGroup.saocGroup[iGrp].numObj; objIdx++) {
          iObj = objectOffset + objectsGroup.saocGroup[iGrp].objList[objIdx];
					
          for (objIdx2 = 0; objIdx2 < objectsGroup.saocGroup[iGrp].numObj; objIdx2++) {
            jObj = objectOffset + objectsGroup.saocGroup[iGrp].objList[objIdx2];
						
            CovRenTarget[iUpmx][jUpmx] += pRenderMat[iUpmx][iObj][paramBand] * E[iObj][jObj] * pRenderMat[jUpmx][jObj][paramBand];
					}
				}
        CovRenTarget[jUpmx][iUpmx] = CovRenTarget[iUpmx][jUpmx];
      }
    }
  }
}

void saoc_ParametersEstimation( float pOld[SAOC_MAX_OBJECTS][SAOC_MAX_PARAM_BANDS],
                                float pIoc[SAOC_MAX_OBJECTS][SAOC_MAX_OBJECTS][SAOC_MAX_PARAM_BANDS],
                                float Dmx[SAOC_MAX_DMX_CHANNELS][SAOC_MAX_OBJECTS],
                                float Mpre[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS],
                                float Mpost[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS],
                                float Hproto[SAOC_MAX_RENDER_CHANNELS][2*SAOC_MAX_RENDER_CHANNELS],
                                float pRenderMat[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_OBJECTS][SAOC_MAX_PARAMETER_BANDS],
                                int RelatedToMatrix[SAOC_MAX_OBJECTS][SAOC_MAX_OBJECTS],
                                int DecorrMethod,
                                int numDecorrelators,
                                int numOutCh,
                                int nSaocChannels,
                                int nSaocObjects,
                                int nSaocDmxChannels,
                                int nSaocDmxObjects,   
                                int nNumBins,
                                int iset,
                                INVERSION_PARAMETERS inversionParam,
                                int dualMode,
                                int bandsLow,
                                float RG[SAOC_MAX_PARAM_SETS][SAOC_MAX_PARAMETER_BANDS][SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS], 
                                float pP_dry[SAOC_MAX_PARAM_SETS][SAOC_MAX_PARAMETER_BANDS][SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS],
                                float pP_wet[SAOC_MAX_PARAM_SETS][SAOC_MAX_PARAMETER_BANDS][SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS]
                                )
{
  int iObj,iDmx,iUpmx,jUpmx,k;
  int numDmx,totalNumObjects, dualModeFlag;
  unsigned int combinedModeFlag = 0;

	float gainValueDualMode;
  saocCoreGroup channelsGroup;
  saocCoreGroup objectsGroup;

	float R[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_OBJECTS] = { {0} };
  float G[SAOC_MAX_OBJECTS][SAOC_MAX_DMX_CHANNELS] = { {0} };

  float CovRenTarget[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS] = { {0} };   /* CovRenTarget = R*E*R' */
  float CovRenDry[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS] = {{0}};   /* CovRenDry = R*G*D*E*D'*G'*R' */
  float CovRenWet[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS] = {{0}};   /* CovRenWet = Mpost*diag(Mpre*diag(CovRenDry)*Mpre.')*Mpost.' */
  float TmpDiagMat[SAOC_MAX_RENDER_CHANNELS] = {0}; 

  float E[SAOC_MAX_OBJECTS][SAOC_MAX_OBJECTS] = {{0}};  /* covariance matrix of input objects E */
  float TmpMat[SAOC_MAX_OBJECTS][SAOC_MAX_OBJECTS] = {{0}};  /* Estimated covariance matrix of input objects G * D * E * D.' * G.' */
  float P_wet[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS];

  numDmx = nSaocDmxChannels + nSaocDmxObjects, 
   
  totalNumObjects = nSaocChannels + nSaocObjects;
  if ( nSaocChannels>0 && nSaocObjects>0 && nSaocDmxObjects>0 ) {
    combinedModeFlag = 0;
	} else {
		combinedModeFlag = 1;
	}

	getSaocCoreGrouping(Dmx, RelatedToMatrix, nSaocChannels, nSaocObjects, nSaocDmxChannels, nSaocDmxObjects,
                      combinedModeFlag, &channelsGroup, &objectsGroup);

	for (k=0; k < nNumBins; k++) {
		if (dualMode==1 && k>=bandsLow) {
      dualModeFlag = 1;
    } else {
      dualModeFlag = 0;
    }

    for (iUpmx = 0; iUpmx < numOutCh ; iUpmx++){
       memset(CovRenDry[iUpmx],0,numOutCh*sizeof(float));
       memset(CovRenWet[iUpmx],0,numOutCh*sizeof(float));
       memset(CovRenTarget[iUpmx],0,numOutCh*sizeof(float));
    }
    
		if (combinedModeFlag) {

      computeEnergyMatrix(pOld, pIoc, 0, k, E, objectsGroup);

      /*  the unmixing matrix G = E * D' * pinvDmxCovMat */
			computeUnmixMatrix(E, Dmx, pRenderMat, 0, 0, numOutCh, RG, CovRenDry, iset, k, objectsGroup,
                         inversionParam.inversion_reltol, inversionParam.inversion_abstol, inversionParam.omiteigvecs, dualModeFlag);

      computeTargetCovariance(objectsGroup, 0, numOutCh, k, E, pRenderMat, CovRenTarget);

		} else {

      computeEnergyMatrix(pOld, pIoc, 0, k, E, channelsGroup);
      computeEnergyMatrix(pOld, pIoc, nSaocChannels, k, E, objectsGroup);

			/*  the unmixing matrix G = E * D' * pinvDmxCovMat */
      computeUnmixMatrix(E, Dmx, pRenderMat, 0, 0, numOutCh, RG, CovRenDry, iset, k, channelsGroup, 
                         inversionParam.inversion_reltol, inversionParam.inversion_abstol, inversionParam.omiteigvecs, dualModeFlag);
      
      computeUnmixMatrix(E, Dmx, pRenderMat, nSaocChannels, nSaocDmxChannels, numOutCh, RG, CovRenDry, iset, k, objectsGroup, 
                         inversionParam.inversion_reltol, inversionParam.inversion_abstol, inversionParam.omiteigvecs, dualModeFlag);
      
      computeTargetCovariance(channelsGroup, 0, numOutCh, k, E, pRenderMat, CovRenTarget);
      
      computeTargetCovariance(objectsGroup, nSaocChannels, numOutCh, k, E, pRenderMat, CovRenTarget);
		}


    /* Reduce number of decorrelators CovRenWet = Mpost*diag(Mpre*diag(CovRenDry)*Mpre.')*Mpost.' */
    if (numDecorrelators < numOutCh) {
      /* TmpDiagMat = diag(Mpre*diag(CovRenDry)*Mpre.') */
      for (iObj = 0; iObj < numDecorrelators ; iObj++){
        TmpDiagMat[iObj] = 0.0f;
        for (iUpmx = 0; iUpmx < numOutCh ; iUpmx++){
          for (jUpmx = 0; jUpmx < numOutCh ; jUpmx++){
						TmpDiagMat[iObj] += Mpre[iObj][iUpmx] * CovRenDry[iUpmx][jUpmx] * Mpre[iObj][jUpmx]; 	
            pP_dry[iset][k][iUpmx][jUpmx] = 0.0f;
            pP_wet[iset][k][iUpmx][jUpmx] = 0.0f;
					}
				}
      }
      /* CovRenWet = Mpost*diag(TmpDiagMat)*Mpost.' */
      for (iUpmx = 0; iUpmx < numOutCh ; iUpmx++){
        for (jUpmx = 0; jUpmx < numOutCh ; jUpmx++){
          for (iObj = 0; iObj < numDecorrelators ; iObj++){
					  CovRenWet[iUpmx][jUpmx] += Mpost[iUpmx][iObj] * TmpDiagMat[iObj] * Mpost[jUpmx][iObj]; 						
				  }
        }
      }
    } else {
      for (iUpmx = 0; iUpmx < numOutCh ; iUpmx++){
        for (jUpmx = 0; jUpmx < numOutCh ; jUpmx++){
          CovRenWet[iUpmx][jUpmx] = 0.0f;				
        }
        CovRenWet[iUpmx][iUpmx] = CovRenDry[iUpmx][iUpmx];	
      }
    }

		if (dualMode==1 && k>=bandsLow) {
				for (iUpmx = 0; iUpmx < numOutCh ; iUpmx++){
					gainValueDualMode = (float) sqrt(CovRenTarget[iUpmx][iUpmx]/max(CovRenDry[iUpmx][iUpmx],SAOC_EPSILON));
					pP_dry[iset][k][iUpmx][iUpmx] = 1.0f;
					pP_wet[iset][k][iUpmx][iUpmx] = 0.0f;

					/* U = GT = GRD^{T}J */
          for (iDmx = 0; iDmx < numDmx; iDmx++){
            RG[iset][k][iUpmx][iDmx] = gainValueDualMode * RG[iset][k][iUpmx][iDmx];
          }
          
				}

		} else  {
			if (DecorrMethod == 0) 
			{     /* Decorrelation Method I: Energy Compensation */
				for (iUpmx = 0; iUpmx < numOutCh ; iUpmx++)
				{
					pP_dry[iset][k][iUpmx][iUpmx] = 1.0f;
					if (CovRenWet[iUpmx][iUpmx] <= 0.0f) 
					{
						pP_wet[iset][k][iUpmx][iUpmx] = 0.0f;
					} 
					else 
					{
            pP_wet[iset][k][iUpmx][iUpmx] = (float) sqrt( max( 0.f, CovRenTarget[iUpmx][iUpmx]-CovRenDry[iUpmx][iUpmx] )  /  max(CovRenWet[iUpmx][iUpmx],SAOC_EPSILON) );
            TmpDiagMat[iUpmx] = max(SAOC_EPSILON, pP_wet[iset][k][iUpmx][iUpmx] * CovRenWet[iUpmx][iUpmx] * pP_wet[iset][k][iUpmx][iUpmx]) ;
            pP_wet[iset][k][iUpmx][iUpmx] = (float) min( 1.f, sqrt( max(0.0f, SAOC_MAX_DECORRELATION * ( CovRenDry[iUpmx][iUpmx] / TmpDiagMat[iUpmx] ) )) ) * pP_wet[iset][k][iUpmx][iUpmx];

          }
				}

			} 
			else if (DecorrMethod == 1) 
			{     /* Decorrelation Method II: Covariance Addjustment */
				LimitedCovarianceAdjustment(CovRenTarget,
																		CovRenDry,
																		CovRenWet,
																		numOutCh,
																		inversionParam.inversion_reltol,
																		inversionParam.inversion_abstol,
																		P_wet);

				for (iUpmx = 0; iUpmx < numOutCh ; iUpmx++)
				{
          for (jUpmx = 0; jUpmx < numOutCh ; jUpmx++)
					{
            TmpMat[iUpmx][jUpmx] = max(SAOC_EPSILON , P_wet[iUpmx][iUpmx] * CovRenWet[iUpmx][jUpmx] * P_wet[jUpmx][jUpmx]);
          }
        }

        for (iUpmx = 0; iUpmx < numOutCh ; iUpmx++)
				{
					TmpDiagMat[iUpmx] = (float) min( 1, sqrt( max(0.0f , SAOC_MAX_DECORRELATION * (CovRenDry[iUpmx][iUpmx] /TmpMat[iUpmx][iUpmx]) ) ) );
          pP_dry[iset][k][iUpmx][iUpmx] = 1.0f;
					for (jUpmx = 0; jUpmx < numOutCh ; jUpmx++)
					{
						pP_wet[iset][k][iUpmx][jUpmx] = (float) TmpDiagMat[iUpmx] * P_wet[iUpmx][jUpmx];
					}
				}  
 			}
		}
  }
}

    
void saoc_Processing(float Dmx[SAOC_MAX_DMX_CHANNELS][SAOC_MAX_OBJECTS],
               float RG[SAOC_MAX_PARAM_SETS][SAOC_MAX_PARAMETER_BANDS][SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS],      
               float pP_dry[SAOC_MAX_PARAM_SETS][SAOC_MAX_PARAMETER_BANDS][SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS],
               float pP_wet[SAOC_MAX_PARAM_SETS][SAOC_MAX_PARAMETER_BANDS][SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS],
               float Mpre[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS],
               float Mpost[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS],
               SAOC_HANDLE_DECORR_DEC hDecorrDec[SAOC_MAX_RENDER_CHANNELS],
               int DecorrMethod,
               int numDecorrelators,
               int startHybBand,
               int numUpmx,
               int lowDelay,
               float pHybReal[SAOC_MAX_CHANNELS][SAOC_MAX_TIME_SLOTS][SAOC_MAX_HYBRID_BANDS],
               float pHybImag[SAOC_MAX_CHANNELS][SAOC_MAX_TIME_SLOTS][SAOC_MAX_HYBRID_BANDS],
               int numDmx,
               int numObj,
               int nNumBins,
               int nHybridBands,
               int nParamSets,
               int pParamSlot[SAOC_MAX_PARAM_SETS],
               int frameLength)
{
  int kernels[SAOC_MAX_HYBRID_BANDS][2];
  int nParamSetsMod;
  int i, islot, ihyb, ipar, j;
  int  jDmx, iUpmx, jUpmx;

  float Gtmp, Ptmp;
  float objtmpReal[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_HYBRID_BANDS];
  float objtmpImag[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_HYBRID_BANDS];

  float realDec[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_HYBRID_BANDS]={{0}};
  float imagDec[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_HYBRID_BANDS]={{0}};
  float tmpReal[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_HYBRID_BANDS]={{0}};
  float tmpImag[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_HYBRID_BANDS]={{0}};

  SAOC_GetKernels(nNumBins, nHybridBands, lowDelay,  kernels);

  if (pParamSlot[nParamSets] != frameLength - 1){
    nParamSetsMod= nParamSets + 1;
    pParamSlot[nParamSetsMod] = frameLength - 1;
  } else{
    nParamSetsMod = nParamSets;
  }

  saoc_interpolate(numUpmx, numDmx, nNumBins, nParamSets, nParamSetsMod, nHybridBands, lowDelay, pParamSlot, RG, RG_Interp);
  saoc_interpolate(numUpmx, numUpmx, nNumBins, nParamSets, nParamSetsMod, nHybridBands, lowDelay, pParamSlot, pP_dry, P1_Interp);
  saoc_interpolate(numUpmx, numUpmx, nNumBins, nParamSets, nParamSetsMod, nHybridBands, lowDelay, pParamSlot, pP_wet, P2_Interp);

  for (islot = pParamSlot[0]+1; islot <= pParamSlot[nParamSetsMod]; islot++) {
    for (ihyb = 0; ihyb < nHybridBands; ihyb++){

      for (iUpmx=0; iUpmx<numUpmx; iUpmx++){
        objtmpReal[iUpmx][ihyb] = 0.0f;
        objtmpImag[iUpmx][ihyb] = 0.0f;
        for (jDmx=0; jDmx<numDmx; jDmx++){
          Gtmp = RG_Interp[islot][ihyb][iUpmx][jDmx] ;
          objtmpReal[iUpmx][ihyb] += Gtmp * pHybReal[jDmx][islot][ihyb];
          objtmpImag[iUpmx][ihyb] += Gtmp * pHybImag[jDmx][islot][ihyb];
        }
      }

      /*Apply Decorrelation*/
      if (numDecorrelators < numUpmx) {  
        for (i=0; i < numDecorrelators; i++){
          tmpReal[i][ihyb] = 0.0f;
          tmpImag[i][ihyb] = 0.0f;
          for (iUpmx=0; iUpmx < numUpmx; iUpmx++){
            tmpReal[i][ihyb] += Mpre[i][iUpmx] * objtmpReal[iUpmx][ihyb];
            tmpImag[i][ihyb] += Mpre[i][iUpmx] * objtmpImag[iUpmx][ihyb];
          }
        }
      } else {
        for (i=0; i<numDecorrelators; i++){
          tmpReal[i][ihyb] = objtmpReal[i][ihyb];
          tmpImag[i][ihyb] = objtmpImag[i][ihyb];
        }
      }
    }

    for (j=0; j<numDecorrelators; j++)
		{
			SAOC_SpatialDecDecorrelateApply(hDecorrDec[j],
                                      tmpReal[j],
                                      tmpImag[j],
                                      realDec[j],
                                      imagDec[j],
                                      startHybBand); 

    } 

    for (ihyb = 0; ihyb < nHybridBands; ihyb++){
      if (numDecorrelators < numUpmx) {
        for (i=0; i<numUpmx; i++){
          tmpReal[i][ihyb] = 0.0f;
          tmpImag[i][ihyb] = 0.0f;
          for (j=0; j<numDecorrelators; j++){
            tmpReal[i][ihyb] += Mpost[i][j] * realDec[j][ihyb];
            tmpImag[i][ihyb] += Mpost[i][j] * imagDec[j][ihyb];
          }
        }
      } else {
        for (i=0; i<numUpmx; i++){
            tmpReal[i][ihyb]  = realDec[i][ihyb];
            tmpImag[i][ihyb]  = imagDec[i][ihyb];
        }
      }

      if (DecorrMethod == 0) {
        for (iUpmx=0; iUpmx<numUpmx; iUpmx++){
          Gtmp = P1_Interp[islot][ihyb][iUpmx][iUpmx];
          Ptmp = P2_Interp[islot][ihyb][iUpmx][iUpmx];
          pHybReal[iUpmx][islot][ihyb] = Gtmp * objtmpReal[iUpmx][ihyb] + Ptmp * tmpReal[iUpmx][ihyb];
          pHybImag[iUpmx][islot][ihyb] = Gtmp * objtmpImag[iUpmx][ihyb] + Ptmp * tmpImag[iUpmx][ihyb];
        }
      } else if (DecorrMethod == 1) {
        for (iUpmx=0; iUpmx<numUpmx; iUpmx++){
          Gtmp = P1_Interp[islot][ihyb][iUpmx][iUpmx];
          pHybReal[iUpmx][islot][ihyb] = Gtmp * objtmpReal[iUpmx][ihyb];
          pHybImag[iUpmx][islot][ihyb] = Gtmp * objtmpImag[iUpmx][ihyb];
          for (i=0; i<numUpmx; i++){ 
            Ptmp = P2_Interp[islot][ihyb][iUpmx][i];
            pHybReal[iUpmx][islot][ihyb] += Ptmp * tmpReal[i][ihyb];
            pHybImag[iUpmx][islot][ihyb] += Ptmp * tmpImag[i][ihyb];
          }        
        }
      }   
    }
  }

  /* hold last parameter position to end of frame: */
  for (ipar = 0; ipar < nNumBins; ipar++) {
    for (i=0; i<numUpmx; i++){
      for (j=0; j<numDmx; j++){
        RG[0][ipar][i][j]  = RG[nParamSets][ipar][i][j];
      }
    }

    for (iUpmx=0; iUpmx<numUpmx; iUpmx++){
      for (jUpmx=0; jUpmx<numUpmx; jUpmx++){
        pP_dry[0][ipar][iUpmx][jUpmx]= pP_dry[nParamSets][ipar][iUpmx][jUpmx];
        pP_wet[0][ipar][iUpmx][jUpmx]= pP_wet[nParamSets][ipar][iUpmx][jUpmx];
      }
    }
  }
}


static void saoc_pinv(float in[SAOC_MAX_GROUP_DMX_CHANNELS][SAOC_MAX_GROUP_DMX_CHANNELS], 
                      float pinv[SAOC_MAX_GROUP_DMX_CHANNELS][SAOC_MAX_GROUP_DMX_CHANNELS],
                      int numDmx,
						          float relative_Thresh,
                      float absolute_Thresh,
                      int omiteigvecs)
{
  /* Suggestions for default parameters: pinv_mod(A,1e-2,0, 1e+5);  (=-20dB) 
     Relative threshold : 1e-2 (-20dB)
     Absolute_Thresh    : not given, default value of 0 is used 
  */
  float in2svd[SAOC_MAX_CHANNELS][SAOC_MAX_CHANNELS] = {{0}};
  float svd_vectors_L[SAOC_MAX_CHANNELS][SAOC_MAX_CHANNELS] = {{0}};
  float svd_vectors_R[SAOC_MAX_CHANNELS][SAOC_MAX_CHANNELS] = {{0}};
  float svd_values[SAOC_MAX_CHANNELS] = {0};

	float off_nrg = 0.0f, f;
	int i,j,l,k,rank,condition;



  /* Compute the singular value decomposition of the in[][] matrix */

	/* if the matrix is empty avoid computation */
		  condition = 0;
		  for (i = 0; i < numDmx; i++){
        for (j = 0; j < numDmx; j++){
				  if (in[i][j] > 0.0f) {
					  condition = 1;
					  break;
				  }
					pinv[i][j] = 0.0f;
			  }
			  if (condition == 1) {
				  break;
			  }
		  }

	if (condition == 1) {

		/* If matrix is diagonal avoid computation ---------------------------------------------- */
		off_nrg = 0.0f;  /* Energy of the off diagonal elements */
		for (i = 0; i< numDmx; i++) {
			for (j=i+1;  j<numDmx; j++) { 
				off_nrg = off_nrg + in[i][j]*in[i][j];
			}
		}
	  
		if (off_nrg == 0.0f) {  
			if (numDmx == 1) {
				svd_values[0] = in[0][0];     svd_vectors_L[0][0] = 1;     svd_vectors_R[0][0] = 1;
			} 
			else{
				for (i = 0; i< numDmx; i++) {
					svd_values[i] = in[i][i];     svd_vectors_L[i][i] = 1;     svd_vectors_R[i][i] = 1;
					for (j=i+1;  j<numDmx; j++) { 
						svd_vectors_L[i][j] = svd_vectors_L[j][i] = 0;
						svd_vectors_R[i][j] = svd_vectors_R[j][i] = 0;
					}
				}
	    
				do{
					condition = 0;  
					for (i = 0; i<numDmx-1; i++) {
						if (svd_values[i] < svd_values[i+1]) {
							condition = 1;
							f = svd_values[i];   svd_values[i] = svd_values[i+1];   svd_values[i+1] = f;
							for (k = 0; k<numDmx; k++) {
								f = svd_vectors_L[k][i];  svd_vectors_L[k][i] = svd_vectors_L[k][i+1];  svd_vectors_L[k][i+1] = f;
								f = svd_vectors_R[k][i];  svd_vectors_R[k][i] = svd_vectors_R[k][i+1];  svd_vectors_R[k][i+1] = f;
							}
						}  
					}
				} while (condition == 1);
			}
		} 
		else {
			for (i = 0; i< numDmx; i++) {
				for (j=0;  j<numDmx; j++) { 
					in2svd[i][j] = in[i][j];
				}
			}
			saoc_SVD(in2svd, svd_vectors_L, svd_values, svd_vectors_R, numDmx);
		} /*------------------------------------------------------------------------------------ */

		if (!relative_Thresh){
		 relative_Thresh = numDmx * SAOC_EPSILON;
		} else {
			relative_Thresh = svd_values[0]* relative_Thresh;
		}

		rank = numDmx;
		for (i=0; i < numDmx; i++ ){
			if ((svd_values[i] < absolute_Thresh) || (svd_values[i] < relative_Thresh)){
				rank = i;
				break;
			}
		}  
		if (!omiteigvecs) {
			for (i = rank; i < numDmx; i++) {
				svd_values[i] = relative_Thresh;
			}
			rank = numDmx;
		}

		/* Invert singular values */
		for (l = 0; l < rank; l++) {
			svd_values[l] = 1.0f / svd_values[l]; /*svd_values[l]>SAOC_EPSILON */
		}

		/* Compute the pseudoinverse */
		for (i = 0; i < numDmx; i++) {
			for (j = i; j < numDmx; j++) {
				pinv[i][j] = 0.0f;
				for (l = 0; l < rank; l++) {
					/* A = U S V' -> pinv_A = V S^-1 U' */
					pinv[i][j] += (svd_vectors_R[i][l] * svd_values[l] * svd_vectors_L[j][l]);  
				}
				pinv[j][i] = pinv[i][j];
			}
		}
	}
}

static void LimitedCovarianceAdjustment( float CovRenTarget[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS],
                                         float CovRenDry[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS],
                                         float CovRenWet[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS],
                                         int numOutCh,
                                         float relative_Thresh,
                                         float absolute_Thresh,
                                         float P_wet[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS])
{
  int iUpmx, jUpmx, kUpmx, rank_S;
  float rel_Thresh;

  float delta_E[SAOC_MAX_CHANNELS][SAOC_MAX_CHANNELS] = {{0}};   /* CovRenDiff = CovRenTarget - CovRenDry */
  float E_wet[SAOC_MAX_CHANNELS][SAOC_MAX_CHANNELS] = {{0}};

  float TmpMat1[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS] = {{0}}; 
  float TmpMat2[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS] = {{0}}; 

  float eigen_vectors_UL[SAOC_MAX_CHANNELS][SAOC_MAX_CHANNELS] = {{0}};
  float eigen_vectors_UR[SAOC_MAX_CHANNELS][SAOC_MAX_CHANNELS] = {{0}};
  float eigen_values_S[SAOC_MAX_CHANNELS] = {0}; 

  float eigen_vectors_VL[SAOC_MAX_CHANNELS][SAOC_MAX_CHANNELS] = {{0}};
  float eigen_vectors_VR[SAOC_MAX_CHANNELS][SAOC_MAX_CHANNELS] = {{0}};
  float eigen_values_Q[SAOC_MAX_CHANNELS] = {0}; 

  for (iUpmx = 0; iUpmx < numOutCh ; iUpmx++){
    for (jUpmx = 0; jUpmx < numOutCh ; jUpmx++){
      delta_E[iUpmx][jUpmx] = CovRenTarget[iUpmx][jUpmx] - CovRenDry[iUpmx][jUpmx];
      E_wet[iUpmx][jUpmx] = CovRenWet[iUpmx][jUpmx]; 
      P_wet[iUpmx][jUpmx] = 0.0f;
   }
   delta_E[iUpmx][iUpmx] = max(0.0f, delta_E[iUpmx][iUpmx]);
 }

  saoc_SVD(delta_E, 
           eigen_vectors_VL,
           eigen_values_Q,
           eigen_vectors_VR,
           numOutCh);

  saoc_SVD(E_wet, 
           eigen_vectors_UL,
           eigen_values_S,
           eigen_vectors_UR,
           numOutCh);
  
  rel_Thresh = eigen_values_S[0]* relative_Thresh ;
  rank_S = numOutCh;
	for (iUpmx = 0; iUpmx < numOutCh ; iUpmx++){
	  if ((eigen_values_S[iUpmx] < absolute_Thresh) || (eigen_values_S[iUpmx] < rel_Thresh)){
		  rank_S = iUpmx;
			break;
	  }
  }
  
  if (rank_S > 0) {
    for (iUpmx = 0; iUpmx < numOutCh ; iUpmx++){
      for (jUpmx = 0; jUpmx < numOutCh ; jUpmx++){
        TmpMat1[iUpmx][jUpmx] = 0.0f;
        TmpMat2[iUpmx][jUpmx] = 0.0f;
        for (kUpmx = 0; kUpmx < numOutCh; kUpmx++){
          TmpMat1[iUpmx][jUpmx] += (float) (eigen_vectors_VL[iUpmx][kUpmx] * sqrt(eigen_values_Q[kUpmx]) * eigen_vectors_VR[jUpmx][kUpmx]);
        }
        for (kUpmx = 0; kUpmx < rank_S  ; kUpmx++){
          TmpMat2[iUpmx][jUpmx] += (float) (eigen_vectors_UR[iUpmx][kUpmx] * 1/sqrt(eigen_values_S[kUpmx] + SAOC_EPSILON) * eigen_vectors_UL[jUpmx][kUpmx]);
        }
      }
    }
    for (iUpmx = 0; iUpmx < numOutCh ; iUpmx++){
      for (jUpmx = 0; jUpmx < numOutCh ; jUpmx++){
        P_wet[iUpmx][jUpmx] = 0.0f;
        for (kUpmx = 0; kUpmx < numOutCh  ; kUpmx++){
          P_wet[iUpmx][jUpmx] +=  TmpMat1[iUpmx][kUpmx] * TmpMat2[kUpmx][jUpmx];
        }
      }
    }
	}
}

static void getSaocDmxPossitions(float Dmx[SAOC_MAX_DMX_CHANNELS][SAOC_MAX_OBJECTS],
																 int RelatedToMatrix[SAOC_MAX_OBJECTS][SAOC_MAX_OBJECTS],
                                 int CrossCov[SAOC_MAX_DMX_CHANNELS][SAOC_MAX_OBJECTS],
                                 int numObjects,
                                 int firstObject,
                                 int numDmxChannels,
                                 int firstDmxChannel)
{
  int ch, dCh, obj, obj2, objIdx, objIdx2;
	int D[SAOC_MAX_DMX_CHANNELS][SAOC_MAX_OBJECTS] = {{0}};
  float minVal = (float)pow(10.0f,-150.0f/20.0f) + SAOC_EPSILON;
  
	for (ch=0; ch<numDmxChannels; ch++) {
    dCh = firstDmxChannel + ch;
    for (obj=0; obj<numObjects; obj++) {
      D[ch][obj] = (minVal<Dmx[dCh][firstObject + obj] ? 1 : 0);
    }
  }

	for (ch=0; ch<numDmxChannels; ch++) {
    dCh = firstDmxChannel + ch;
		for (obj=0; obj<numObjects; obj++) {
			objIdx = firstObject + obj;
			CrossCov[ch][obj] = 0;
			for (obj2=0; obj2<numObjects; obj2++) {
				objIdx2 = firstObject + obj2;
				CrossCov[ch][obj] += D[ch][obj2] * RelatedToMatrix[objIdx][objIdx2];
			}
		}
  }
}

static int getSaocCoreGroups(int CrossCov[SAOC_MAX_DMX_CHANNELS][SAOC_MAX_OBJECTS],
                             int numObjects,
                             int numDmxChannels,
                             saocCoreGroup *groupInfo)
{
  int iGr, iObj, jObj, iDmx, jDmx, indexObj, indexChan, groupIndex, foundGroup;
  
  int channelsFlag[SAOC_MAX_DMX_CHANNELS];
  int objectsFlag[SAOC_MAX_OBJECTS];

	/* Init empty grouping info */
  for (iGr = 0; iGr<SAOC_MAX_DMX_CHANNELS; iGr++) {
    for (iObj = 0; iObj < numObjects; iObj++){ 
      groupInfo->saocGroup[iGr].objList[iObj] = -1;
    }
    for (iDmx = 0; iDmx <SAOC_MAX_DMX_CHANNELS; iDmx++) {
      groupInfo->saocGroup[iGr].dmxList[iDmx] = -1;
    }
    groupInfo->saocGroup[iGr].numDmx = 0;
    groupInfo->saocGroup[iGr].numObj = 0;
  }
  groupInfo->numGroups = 0;

  for (iObj = 0; iObj < numObjects; iObj++){
    objectsFlag[iObj] = -1;
  }
  for (jDmx = 0; jDmx <numDmxChannels; jDmx++) {
    channelsFlag[jDmx] = jDmx;
  }
 
	for (iDmx = 0; iDmx <numDmxChannels; iDmx++) {         
		for (iObj = 0; iObj < numObjects; iObj++) {          /* search for not assigned objects */  
			if (CrossCov[iDmx][iObj]>0) {                             
				if (objectsFlag[iObj] == -1) {                   /* iObj not assigned to any group */
					objectsFlag[iObj] = iDmx;                      /* assign iObj to iDmx group*/
				} else {                                         /* iObj already assigned to a group*/
					foundGroup = objectsFlag[iObj];                         
					for (jObj = 0; jObj < numObjects; jObj++) {    /* assign all objects from this existing group to the new one */
						if (objectsFlag[jObj] == foundGroup) {
							objectsFlag[jObj] = iDmx;
						}
					}
					for (jDmx = 0; jDmx <iDmx; jDmx++) {           /* assign all channels from this existing group to the new one */
						if (channelsFlag[jDmx] == foundGroup) {
							channelsFlag[jDmx] = iDmx;
						}
					}
				}
			}
		}
	}

	for (iObj = 0; iObj < numObjects; iObj++) {
		iGr = groupInfo->numGroups;
		if (objectsFlag[iObj] > -1) {											   /* object not assigned to any group */
			groupIndex = objectsFlag[iObj];
			for (jObj = 0; jObj < numObjects; jObj++) {        /* assign all objects from this group into the object list */
				if (objectsFlag[jObj] == groupIndex) {
					indexObj = groupInfo->saocGroup[iGr].numObj++; 
					groupInfo->saocGroup[iGr].objList[indexObj] = jObj;
					objectsFlag[jObj] = -1;                        /* mark object as assigned to an object list */
				}
			}
			for (iDmx = 0; iDmx <numDmxChannels; iDmx++) {     /* assign all chnls from this group into the list */
				if (channelsFlag[iDmx] == groupIndex) {
					indexChan = groupInfo->saocGroup[iGr].numDmx++; 
					groupInfo->saocGroup[iGr].dmxList[indexChan] = iDmx;
					channelsFlag[iDmx] = -1;                       /* mark channel as assigned to a channel list */
				}
			}
			groupInfo->numGroups++;
		}
	}
   
	for (iGr = 0; iGr<groupInfo->numGroups; iGr++) {
		assert(groupInfo->saocGroup[iGr].numDmx < SAOC_MAX_GROUP_DMX_CHANNELS);
		assert(groupInfo->saocGroup[iGr].numObj < SAOC_MAX_GROUP_OBJECTS);
	}

  return 0;
}

static int getSaocCoreGrouping(float Dmx[][SAOC_MAX_OBJECTS],
															 int RelatedToMatrix[SAOC_MAX_OBJECTS][SAOC_MAX_OBJECTS],
                               int numChannels,
                               int numObjects,
                               int numDmxChannels,
                               int numDmxObjects,
                               int combinedFlag,
                               saocCoreGroup *channelsGroup,
                               saocCoreGroup *objectsGroup)

{
  int CrossCov_ch[SAOC_MAX_DMX_CHANNELS][SAOC_MAX_OBJECTS] = {{0}};
  int CrossCov_obj[SAOC_MAX_DMX_CHANNELS][SAOC_MAX_OBJECTS] = {{0}};

  if (combinedFlag == 0) {

    getSaocDmxPossitions(Dmx, RelatedToMatrix, CrossCov_ch, numChannels, 0, numDmxChannels, 0);
    getSaocCoreGroups(CrossCov_ch, numChannels, numDmxChannels, channelsGroup);

    getSaocDmxPossitions(Dmx, RelatedToMatrix, CrossCov_obj, numObjects, numChannels, numDmxObjects, numDmxChannels);
    getSaocCoreGroups(CrossCov_obj, numObjects, numDmxObjects, objectsGroup);

  } else {

    getSaocDmxPossitions(Dmx, RelatedToMatrix, CrossCov_obj, numChannels+numObjects, 0, numDmxChannels+numDmxObjects, 0);
    getSaocCoreGroups(CrossCov_obj, numChannels+numObjects, numDmxChannels+numDmxObjects, objectsGroup);

  }

  return 0;
}