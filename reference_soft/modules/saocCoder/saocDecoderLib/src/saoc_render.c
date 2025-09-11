/*******************************************************************************
This software module was originally developed by

Coding Technologies, Fraunhofer IIS, Philips

and edited by

-

in the course of development of ISO/IEC 23003 for reference purposes and its
performance may not have been optimized. This software module is an
implementation of one or more tools as specified by ISO/IEC 23003. ISO/IEC gives
You a royalty-free, worldwide, non-exclusive, copyright license to copy,
distribute, and make derivative works of this software module or modifications
thereof for use in implementations of ISO/IEC 23003 in products that satisfy
conformance criteria (if any). Those intending to use this software module in
products are advised that its use may infringe existing patents. ISO/IEC have no
liability for use of this software module or modifications thereof. Copyright is
not released for products that do not conform to audiovisual and image-coding
related ITU Recommendations and/or ISO/IEC International Standards.

#ifdef NOT_PUBLISHED

Assurance that the originally developed software module can be used (1) in
ISO/IEC 23003 once ISO/IEC 23003 has been adopted; and (2) to develop ISO/IEC
23003:
Coding Technologies, Fraunhofer IIS, Philips grant(s) ISO/IEC all
rights necessary to include the originally developed software module or
modifications thereof in ISO/IEC 23003 and to permit ISO/IEC to offer You a
royalty-free, worldwide, non-exclusive, copyright license to copy, distribute,
and make derivative works for use in implementations of ISO/IEC 23003 in
products that satisfy conformance criteria (if any), and to the extent that such
originally developed software module or portions of it are included in ISO/IEC
23003. To the extent that Coding Technologies, Fraunhofer IIS,
Philips own(s) patent rights that would be required to make, use, or sell the
originally developed software module or portions thereof included in ISO/IEC
23003 in a conforming product, Coding Technologies, Fraunhofer
IIS, Philips will assure the ISO/IEC that it is (they are) willing to negotiate
licenses under reasonable and non-discriminatory terms and conditions with
applicants throughout the world. ISO/IEC gives You a free license to this
software module or modifications thereof for the sole purpose of developing
ISO/IEC 23003.

#endif

Coding Technologies, Fraunhofer IIS, Philips retain full right to
modify and use the code for its (their) own purpose, assign or donate the code
to a third party and to inhibit third parties from using the code for products
that do not conform to MPEG-related ITU Recommendations and/or ISO/IEC
International Standards. This copyright notice must be included in all copies or
derivative works.

Copyright (c) ISO/IEC 2007.
*******************************************************************************/

#include <stdio.h>
#include "saoc_render.h"

#include <string.h>
#include <stdlib.h>


float *SAOC_InitTimeInstances(char* pInName,int *num_time_instances)
{
  float *time_instances;
  int time_inst;
  char ren_str[] = ".reninfo";
  (*num_time_instances)=0;

	
  if(strstr(pInName,ren_str)==NULL)	{
    (*num_time_instances)=1;
  }
	
  else {
		
    FILE* fptr=fopen(pInName,"r");

    int iTmp;

    while(fscanf(fptr,"%d",&iTmp)!=EOF)	{
      (*num_time_instances)++;
    }

    fclose(fptr);
  }
	
  time_instances=(float *)malloc((*num_time_instances)*sizeof(float));
  memset(time_instances,0,(*num_time_instances)*sizeof(float));

  if((*num_time_instances)==1) {
    time_instances[0]=0.f;
  }
	
  else{
		
    FILE* fptr=fopen(pInName,"r");
    int iTmp;

    for(time_inst=0;time_inst<(*num_time_instances);time_inst++)	{
      fscanf(fptr,"%d",&iTmp);
      time_instances[time_inst]=(float)iTmp;
    }
    fclose(fptr);
  }
  return time_instances;
}


float ***SAOC_InitRenderMats(char* pInName,int num_time_instances,float *time_instances,int nChannelNumber)
{
  float ***pMats;
  int time_inst,ch;

  char ren_str[] = ".reninfo";

  pMats=(float ***)malloc(num_time_instances*sizeof(float **));
  for(time_inst=0;time_inst<num_time_instances;time_inst++)	{
    pMats[time_inst]=(float **)malloc(SAOC_MAX_RENDER_CHANNELS*sizeof(float *));
    for(ch=0;ch<SAOC_MAX_RENDER_CHANNELS;ch++) {
      pMats[time_inst][ch]=(float *)malloc(SAOC_MAX_OBJECTS*sizeof(float));
      memset(pMats[time_inst][ch],0,SAOC_MAX_OBJECTS*sizeof(float));
    }
  }

	
  if(strstr(pInName,ren_str)==NULL)	{
    FILE* fptr=fopen(pInName,"r");
    float fTmp;
    int nCnt=0;

    while(fscanf(fptr,"%f",&fTmp)!=EOF) {
      pMats[0][nCnt%nChannelNumber][nCnt/nChannelNumber] = fTmp;
      nCnt++;
    }
    fclose(fptr);
  }	else {
    char file_name[1000];
    char time_inst_str[1000];
    char tmp_str[1000];

    strncpy(file_name,pInName,strlen(pInName)-strlen(ren_str));
    file_name[strlen(pInName)-strlen(ren_str)]='\0';

    strcat(file_name,"_");

    for(time_inst=0;time_inst<num_time_instances;time_inst++)	{
      FILE* fptr;
      float fTmp;
      int nCnt=0;

      sprintf(time_inst_str,"%d",time_inst+1);
      strncpy(tmp_str,file_name,strlen(file_name));
      tmp_str[strlen(file_name)]='\0';
      strcat(tmp_str,time_inst_str);
      strcat(tmp_str,".saoc");

      fptr=fopen(tmp_str,"r");

      while(fscanf(fptr,"%f",&fTmp)!=EOF) {
        pMats[time_inst][nCnt%nChannelNumber][nCnt/nChannelNumber] = fTmp;
        nCnt++;
      }
      fclose(fptr);
    }
  }
  return pMats;
}

void SAOC_deleteTimeInstances(float *time_instances)
{
  if(time_instances!=NULL) {
    free(time_instances);
  }
}

void SAOC_deleteRenderMats(int num_time_instances,float ***pMats)
{
  int time_inst,ch;

  for(time_inst=0;time_inst<num_time_instances;time_inst++)	{
    for(ch=0;ch<SAOC_MAX_RENDER_CHANNELS;ch++) {
      if(pMats[time_inst][ch]!=NULL) {
        free(pMats[time_inst][ch]);
      }
    }
    if(pMats[time_inst]!=NULL){
      free(pMats[time_inst]);
    }
  }
  if(pMats!=NULL)	{
    free(pMats);
  }
}


void SAOC_Interpolate_oamRenMat(int numOamPerFrame,
																int	framePossOAM[32],
																float	frameRenderMat[32][SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_OBJECTS],
																float pRenderMatTmp[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_OBJECTS],
                                int env_p)
{
  int n,ch,obj;
  double p;

	for (n=1; n< numOamPerFrame; n++) {
		if(env_p == framePossOAM[n]) {
			for(ch=0;ch<SAOC_MAX_RENDER_CHANNELS;ch++) {
				for(obj=0;obj<SAOC_MAX_OBJECTS;obj++)	{
					pRenderMatTmp[ch][obj] = frameRenderMat[n][ch][obj];
				}
			}
		} else if (env_p < framePossOAM[n]) {
			p = (float)(env_p - framePossOAM[n-1])/(framePossOAM[n] - framePossOAM[n-1]);
			
			for(ch=0;ch<SAOC_MAX_RENDER_CHANNELS;ch++) {
				for(obj=0;obj<SAOC_MAX_OBJECTS;obj++)	{
					pRenderMatTmp[ch][obj] = (float)((1.0-p) * frameRenderMat[n-1][ch][obj] + p * frameRenderMat[n][ch][obj]);
				}
			}
		}
	}
}


void SAOC_getRenderMat(float pMat[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_OBJECTS],float ***RenderMats,int num_time_instances,float *time_instances,int env_p)
{
  int time_inst,ch,obj;
  double env_pos = 64*env_p;
  double p;

  if(num_time_instances==1)	{
    for(ch=0;ch<SAOC_MAX_RENDER_CHANNELS;ch++) {
      for(obj=0;obj<SAOC_MAX_OBJECTS;obj++)	{
        pMat[ch][obj]=RenderMats[0][ch][obj];
      }
    }
  }
	
  else{
    time_inst=0;

    while((time_inst<num_time_instances)&&(env_pos >= time_instances[time_inst]))	{

      time_inst++;
    }
    if(env_pos <= time_instances[0])	{

      for(ch=0;ch<SAOC_MAX_RENDER_CHANNELS;ch++) {
        for(obj=0;obj<SAOC_MAX_OBJECTS;obj++)	{
          pMat[ch][obj]=RenderMats[0][ch][obj];
        }
      }
    } else if(env_pos >= time_instances[num_time_instances-1]) {
      for(ch=0;ch<SAOC_MAX_RENDER_CHANNELS;ch++) {
        for(obj=0;obj<SAOC_MAX_OBJECTS;obj++)	{
          pMat[ch][obj]=RenderMats[num_time_instances-1][ch][obj];
        }
      }
    } else {
      p = (env_pos - time_instances[time_inst-1])/(time_instances[time_inst]-time_instances[time_inst-1]);

      for(ch=0;ch<SAOC_MAX_RENDER_CHANNELS;ch++) {
        for(obj=0;obj<SAOC_MAX_OBJECTS;obj++)	{
          pMat[ch][obj] = (float)((1.0-p)*RenderMats[time_inst-1][ch][obj] + p*RenderMats[time_inst][ch][obj]);
        }
      }
    }
  }
}


int saoc_get_RenMat_ChannelPath(int inSaocCICPIndex,
                                int outSaocCICPIndex,
                                int saocInputCh_Mapping[SAOC_MAX_RENDER_CHANNELS],
                                int saocInputLFE_Mapping[SAOC_MAX_LFE_CHANNELS],
                                int nInChannels,
                                int nInLFEs,
                                int saocOutputCh_Mapping[SAOC_MAX_RENDER_CHANNELS],
                                int saocOutputLFE_Mapping[SAOC_MAX_LFE_CHANNELS],
                                int nOutChannels,
                                int nOutLFEs,
                                char* pInName,
                                float RenMatCh[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_OBJECTS],
                                float RenderMatLfe[SAOC_MAX_LFE_CHANNELS][SAOC_MAX_LFE_CHANNELS])
{
  int i,j;
  FILE* fptr;
  float fTmp;
  int nCnt=0;
  float RenMatTmp[SAOC_MAX_RENDER_CHANNELS+SAOC_MAX_LFE_CHANNELS][SAOC_MAX_RENDER_CHANNELS+SAOC_MAX_LFE_CHANNELS] = {{ 0.0f }};

  int nTotalOutChannels = nOutChannels + nOutLFEs;

  if ((inSaocCICPIndex != -1) && (inSaocCICPIndex == outSaocCICPIndex)) {
    for (i=0; i<nInChannels ; i++) {
      RenMatCh[i][i] = 1.0f;
      for (j=i+1; j<nInChannels ; j++) {
        RenMatCh[j][i] = 0.0f;
        RenMatCh[i][j] = 0.0f;
      }
    }
    for (i=0; i<nInLFEs ; i++) {
      RenderMatLfe[i][i] = 1.0f;
      for (j=i+1; j<nInLFEs ; j++) {
        RenderMatLfe[j][i] = 0.0f;
        RenderMatLfe[i][j] = 0.0f;
      }
    }
  } else {
    fptr=fopen(pInName,"r");
    if (fptr == NULL) {
      fprintf(stderr, "saoc_render.c: Error opening rendering matrix file! \n");
      return -1;
    }
    while(fscanf(fptr,"%f",&fTmp)!=EOF) {
      RenMatTmp[nCnt/nTotalOutChannels][nCnt%nTotalOutChannels] = fTmp;
      nCnt++;
    }
    fclose(fptr);

    for (i=0; i<nOutChannels ; i++) {
        for (j=0; j<nInChannels; j++) {
          RenMatCh[i][j] = RenMatTmp[saocInputCh_Mapping[j]][saocOutputCh_Mapping[i]] ;
        }
    }
    for (i=0; i<nOutLFEs ; i++) {
        for (j=0; j<nInLFEs; j++) {
          RenderMatLfe[i][j] = RenMatTmp[saocInputLFE_Mapping[j]][saocOutputLFE_Mapping[i]] ;
        }
    }
  }
  return 0;
}
