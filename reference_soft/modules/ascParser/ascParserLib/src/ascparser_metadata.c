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

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "ascparser_metadata.h"

static void Read_GroupDefinition ( ascparserBitStreamPtr hbitbuf, int numGroups, GroupDefinition* groups ) {

  int i, j;
  int dummyId = 0;

  for ( i = 0; i < numGroups; i++ ) {
    groups[i].groupID   = __getBits ( hbitbuf, dummyId, 7 );
    groups[i].allowOnOff = __getBits ( hbitbuf, dummyId, 1 );
    groups[i].defaultOnOff = __getBits ( hbitbuf, dummyId, 1 );

    groups[i].allowPositionInteractivity = __getBits ( hbitbuf, dummyId, 1 );
    if ( groups[i].allowPositionInteractivity ) {
      groups[i].interactivityMinAzOffset   = __getBits ( hbitbuf, dummyId, 7 );
      groups[i].interactivityMaxAzOffset   = __getBits ( hbitbuf, dummyId, 7 );
      groups[i].interactivityMinElOffset   = __getBits ( hbitbuf, dummyId, 5 );
      groups[i].interactivityMaxElOffset   = __getBits ( hbitbuf, dummyId, 5 );
      groups[i].interactivityMinDistFactor = __getBits ( hbitbuf, dummyId, 4 );
      groups[i].interactivityMaxDistFactor = __getBits ( hbitbuf, dummyId, 4 );
    }

    groups[i].allowGainInteractivity = __getBits ( hbitbuf, dummyId, 1 );

    if ( groups[i].allowGainInteractivity ) {
      groups[i].interactivityMinGain = __getBits ( hbitbuf, dummyId, 6 );
      groups[i].interactivityMaxGain = __getBits ( hbitbuf, dummyId, 5 );
    }

    /* correct syntax, remove no longer existing elements */
    /* groups[i].groupPriority         = __getBits ( hbitbuf, dummyId, 3 );
    groups[i].closestSpeakerPlayout = __getBits ( hbitbuf, dummyId, 1 ); */

    groups[i].groupNumMembers       = __getBits ( hbitbuf, dummyId, 7 ) + 1;
    groups[i].hasConjunctMembers    = __getBits ( hbitbuf, dummyId, 1 );

    if ( groups[i].hasConjunctMembers == 1 ) {
      groups[i].startID = __getBits ( hbitbuf, dummyId, 7 );
    } else {
      for ( j = 0; j < groups[i].groupNumMembers; j++ ) {
        groups[i].metaDataElementID[j] = __getBits ( hbitbuf, dummyId, 7 );
      }
    }
  }

  for (i = 0; i < numGroups; i++)
  {
    if (groups[i].allowPositionInteractivity == 1)
    {
      groups[i].f_interactivityMinAzOffset = -1.5f * (float)groups[i].interactivityMinAzOffset;
      groups[i].f_interactivityMaxAzOffset = +1.5f * (float)groups[i].interactivityMinAzOffset;
      groups[i].f_interactivityMinElOffset = -3.0f * (float)groups[i].interactivityMinElOffset;
      groups[i].f_interactivityMaxElOffset = +3.0f * (float)groups[i].interactivityMaxElOffset;
      groups[i].f_interactivityMinDistFactor = (float)pow(2.0f, groups[i].interactivityMinDistFactor - 12.0f);
      groups[i].f_interactivityMaxDistFactor = (float)pow(2.0f, groups[i].interactivityMaxDistFactor - 12.0f);
    }
    if (groups[i].allowGainInteractivity == 1)
    {
      groups[i].f_interactivityMinGain = (float)(groups[i].interactivityMinGain - 63.0f);
      groups[i].f_interactivityMaxGain = (float)groups[i].interactivityMaxGain;
    }
  }

}

static void Read_SwitchGroupDefinition ( ascparserBitStreamPtr hbitbuf, int numSwitchGroups, SwitchGroupDefinition* switchGroups ) {
 
  int i, j;
  int dummyId = 0;

  for ( i = 0; i < numSwitchGroups; i++ ) {
    switchGroups[i].switchGroupID         = __getBits ( hbitbuf, dummyId, 5 );
    switchGroups[i].allowOnOff            = __getBits ( hbitbuf, dummyId, 1 );
    if (switchGroups[i].allowOnOff == 1)
    {
      switchGroups[i].defaultOnOff        = __getBits ( hbitbuf, dummyId, 1 );
    }
    else
    {
      switchGroups[i].defaultOnOff = -1;
    }
    switchGroups[i].switchGroupNumMembers = __getBits ( hbitbuf, dummyId, 5 ) + 1;

    for ( j = 0; j < switchGroups[i].switchGroupNumMembers; j++ ) {
      switchGroups[i].switchGroupMemberID[j] = __getBits ( hbitbuf, dummyId, 7 );
    }

    switchGroups[i].switchGroupDefaultGroupID = __getBits ( hbitbuf, dummyId, 7 );
  }
}

static void Read_GroupPresetDefinition ( ascparserBitStreamPtr hbitbuf, int numGroupPresets, GroupPresetDefinition* groupPresets ) {

  int i, k;
  int dummyId = 0;

  for( i = 0; i < numGroupPresets; i++ )
  {
    groupPresets[i].presetID                          = __getBits ( hbitbuf, dummyId, 5);
    groupPresets[i].presetKind                        = __getBits ( hbitbuf, dummyId, 5);
    groupPresets[i].presetNumConditions               = __getBits ( hbitbuf, dummyId, 4);

    groupPresets[i].presetNumConditions++;
    for( k = 0; k < groupPresets[i].presetNumConditions; k++ )
    {
      groupPresets[i].presetGroupID[k]                = __getBits ( hbitbuf, dummyId, 7);
      groupPresets[i].presetConditionOnOff[k]         = __getBits ( hbitbuf, dummyId, 1);

      if(groupPresets[i].presetConditionOnOff[k])
      {
        groupPresets[i].groupPresetDisableGain[k]     = __getBits ( hbitbuf, dummyId, 1);
        groupPresets[i].groupPresetGainFlag[k]        = __getBits ( hbitbuf, dummyId, 1);

        if(groupPresets[i].groupPresetGainFlag[k])
        {
          groupPresets[i].groupPresetGain[k]          = __getBits ( hbitbuf, dummyId, 8);
        }

        groupPresets[i].groupPresetDisablePosition[k] = __getBits ( hbitbuf, dummyId, 1);
        groupPresets[i].groupPresetPositionFlag[k]    = __getBits ( hbitbuf, dummyId, 1);

        if(groupPresets[i].groupPresetPositionFlag[k])
        {
          groupPresets[i].groupPresetAzOffset[k]            = __getBits ( hbitbuf, dummyId, 8);
          groupPresets[i].groupPresetElOffset[k]            = __getBits ( hbitbuf, dummyId, 6);
          groupPresets[i].groupPresetDist[k]          = __getBits ( hbitbuf, dummyId, 4);
        }
      }
    }
  }
  for (i = 0; i < numGroupPresets; i++)
  {
    for (k = 0; k < groupPresets[i].presetNumConditions; k++)
    {
      groupPresets[i].f_groupPresetGain[k] = (0.5f* ((float)groupPresets[i].groupPresetGain[k] -255)) + 32.0f;
      groupPresets[i].f_groupPresetAzOffset[k] = 1.5f * ((float)groupPresets[i].groupPresetAzOffset[k] -127.0f);
      groupPresets[i].f_groupPresetElOffset[k] = 3.0f * ((float)groupPresets[i].groupPresetElOffset[k] -31.0f);
      groupPresets[i].f_groupPresetDist[k] = (float)pow(2.0f, groupPresets[i].groupPresetDist[k] - 12);
    }
  }
}

static void Read_ContentData ( ascparserBitStreamPtr hbitbuf, ContentData* content ) {
  
  int n;
  int dummyId = 0;
  unsigned int nBitsRead = __getValidBits ( hbitbuf );
 
  content->numContentDataBlocks = __getBits ( hbitbuf, dummyId, 7 ) + 1;

  for ( n = 0; n < content->numContentDataBlocks ; n++ ) {
    content->contentDataGroupID[n] = __getBits ( hbitbuf, dummyId, 7 ) ;
    content->contentKind[n]        = __getBits ( hbitbuf, dummyId, 4 ) ;
    content->hasContentLanguage[n] = __getBits ( hbitbuf, dummyId, 1 ) ;

    if ( content->hasContentLanguage[n] ) {
      content->contentLanguage[n] = __getBits ( hbitbuf, dummyId, 24 ) ;
    }
  }

  nBitsRead = nBitsRead - __getValidBits ( hbitbuf );

  if ( nBitsRead % 8 ) {
    __getBits ( hbitbuf, dummyId, 8 - ( nBitsRead % 8 ) );
  }
}

static void Read_DescriptionData ( ascparserBitStreamPtr hbitbuf, DescriptionData* description, int type ) {
 
  int i, n, d;
  int dummyId = 0;
  unsigned int nBitsRead = __getValidBits ( hbitbuf );
  
  description->numDescriptionBlocks = __getBits ( hbitbuf, dummyId, 7 ) + 1;

  for ( n = 0; n < description->numDescriptionBlocks ; n++ ) {
    if ( type == ID_MAE_GROUP_DESCRIPTION ) {
      description->descriptionGroupID[n] = __getBits ( hbitbuf, dummyId, 7 ) ;
    } else if ( type == ID_MAE_SWITCHGROUP_DESCRIPTION ||  type == ID_MAE_GROUPPRESET_DESCRIPTION ) {
      description->descriptionGroupID[n] = __getBits ( hbitbuf, dummyId, 5 ) ;
    } else {
      assert ( 0 );
    }

    description->numDescLanguages[n] = __getBits ( hbitbuf, dummyId, 4 ) + 1;

    for ( i = 0; i < description->numDescLanguages[n] ; i++ ) {

      description->descriptionLanguage[n][i] = __getBits ( hbitbuf, dummyId, 24 );

      description->descriptionDataLength[n][i] = __getBits ( hbitbuf, dummyId, 8 ) + 1;

      for ( d = 0; d < description->descriptionDataLength[n][i]; d++ ) {
        description->descriptionData[n][i][d] = __getBits ( hbitbuf, dummyId, 8 ) ;
      }
    }
  }

  nBitsRead = nBitsRead - __getValidBits ( hbitbuf );

  if ( nBitsRead % 8 ) {
    __getBits ( hbitbuf, dummyId, 8 - ( nBitsRead % 8 ) );
  }
}

static void Read_CompositeObjectPairData ( ascparserBitStreamPtr hbitbuf, CompositeObjectPairData* compositeObjectPair ) {

  int n;
  int dummyId = 0;
  unsigned int nBitsRead = __getValidBits ( hbitbuf );

  compositeObjectPair->numCompositePairs = __getBits ( hbitbuf, dummyId, 7 ) + 1;

  for ( n = 0; n < compositeObjectPair->numCompositePairs ; n++ ) {
    compositeObjectPair->compositeElementID0[n] = __getBits ( hbitbuf, dummyId, 7 ) ;
    compositeObjectPair->compositeElementID1[n] = __getBits ( hbitbuf, dummyId, 7 ) ;
  }

  nBitsRead = nBitsRead - __getValidBits ( hbitbuf );

  if ( nBitsRead % 8 ) {
    __getBits ( hbitbuf, dummyId, 8 - ( nBitsRead % 8 ) );
  }

}

static void Read_DrcUserInterfaceInfoData(ascparserBitStreamPtr hbitbuf,
                                          DrcUserInterfaceInfoData* drcUserInterfaceInfo
) {
  int dummyId = 0;
  unsigned int nBitsRead = __getValidBits(hbitbuf);
    
  drcUserInterfaceInfo->version = __getBits(hbitbuf, dummyId, 2);

  if ( drcUserInterfaceInfo->version == 0 ) {
    int c = 0;

    drcUserInterfaceInfo->numTargetLoudnessConditions = __getBits ( hbitbuf, dummyId, 3 ) + 1;
    drcUserInterfaceInfo->targetLoudnessValueLower[0] = -63.f;

    for (c = 0; c < drcUserInterfaceInfo->numTargetLoudnessConditions; c++) {
      drcUserInterfaceInfo->targetLoudnessValueUpper[c] = (float) __getBits ( hbitbuf, dummyId, 6 ) - 63.f;
      drcUserInterfaceInfo->drcSetEffectAvailable[c] = __getBits ( hbitbuf, dummyId, 16 );
            
      if (c < drcUserInterfaceInfo->numTargetLoudnessConditions - 1) {
          drcUserInterfaceInfo->targetLoudnessValueLower[c+1] = drcUserInterfaceInfo->targetLoudnessValueUpper[c];
      }
    }
  } else {
    /* discard remaining bits signaled by mae_dataLength */
  }

  nBitsRead = nBitsRead - __getValidBits ( hbitbuf );

  if ( nBitsRead % 8 ) {
    __getBits ( hbitbuf, dummyId, 8 - ( nBitsRead % 8 ) );
  }
}

static void Read_ProductionScreenSizeDataExtension ( ascparserBitStreamPtr hbitbuf, ProductionScreenExtensionData* productionScreenExtensions)
{
  int dummyId = 0;
  int sc = 0;
  unsigned int nBitsRead = __getValidBits ( hbitbuf );

  productionScreenExtensions->overwriteDefaultProductionScreenAzimuth = __getBits ( hbitbuf, dummyId, 1 );
  if (productionScreenExtensions->overwriteDefaultProductionScreenAzimuth == 1)
  {
    productionScreenExtensions->defaultScreenSizeLeftAz = __getBits ( hbitbuf, dummyId, 10 );
    productionScreenExtensions->defaultScreenSizeRightAz = __getBits ( hbitbuf, dummyId, 10 );
    productionScreenExtensions->f_defaultScreenSizeLeftAz = min(max(((float)productionScreenExtensions->defaultScreenSizeLeftAz - 511.0f) * 0.5f, -180.0f), 180.0f);
    productionScreenExtensions->f_defaultScreenSizeRightAz = min(max(((float)productionScreenExtensions->defaultScreenSizeRightAz - 511.0f) * 0.5f, -180.0f), 180.0f);
	
  }
  productionScreenExtensions->numPresetProductionScreens = __getBits ( hbitbuf, dummyId, 5 );
  for (sc = 0; sc < productionScreenExtensions->numPresetProductionScreens; sc++)
  {
    productionScreenExtensions->productionScreenPresetID[sc] = __getBits ( hbitbuf, dummyId, 5 );
    productionScreenExtensions->hasNonStandardScreenSize[sc] = __getBits ( hbitbuf, dummyId, 1 );
    if ( productionScreenExtensions->hasNonStandardScreenSize[sc] == 1 )
    {
      productionScreenExtensions->isCenteredInAzimuth[sc] = __getBits ( hbitbuf, dummyId, 1 );
      if (productionScreenExtensions->isCenteredInAzimuth[sc] == 1)
      {
        productionScreenExtensions->azimuth[sc] = __getBits ( hbitbuf, dummyId, 9 );
        productionScreenExtensions->f_azimuth[sc] = min(max(((float)productionScreenExtensions->azimuth[sc] * 0.5f), 0.0f), 180.0f);
        productionScreenExtensions->f_azimuth_left[sc] = productionScreenExtensions->f_azimuth[sc];
        productionScreenExtensions->f_azimuth_right[sc] = productionScreenExtensions->f_azimuth[sc] * -1.0f;
      }
      else
      {
        productionScreenExtensions->azimuth_left[sc] = __getBits ( hbitbuf, dummyId, 10 );
        productionScreenExtensions->azimuth_right[sc] = __getBits ( hbitbuf, dummyId, 10 );
        productionScreenExtensions->f_azimuth_left[sc] = min(max(((float)productionScreenExtensions->azimuth_left[sc] - 511.0f)* 0.5f, -180.0f), 180.0f);
        productionScreenExtensions->f_azimuth_right[sc] = min(max(((float)productionScreenExtensions->azimuth_right[sc] - 511.0f) * 0.5f, -180.0f), 180.0f);
      }
      productionScreenExtensions->elevation_top[sc] =  __getBits ( hbitbuf, dummyId, 9 );
      productionScreenExtensions->elevation_bottom[sc] =  __getBits ( hbitbuf, dummyId, 9 );
      productionScreenExtensions->f_elevation_top[sc] = min(max(((float)productionScreenExtensions->elevation_top[sc] - 255.0f) * 0.5f, -90.0f), 90.0f);
      productionScreenExtensions->f_elevation_bottom[sc] =  min(max(((float)productionScreenExtensions->elevation_bottom[sc]- 255.0f) * 0.5f, -90.0f), 90.0f);
    }
    else
    {
      productionScreenExtensions->isCenteredInAzimuth[sc] = -1;
      productionScreenExtensions->azimuth_left[sc] = 0;
      productionScreenExtensions->azimuth_right[sc] = 0;
      productionScreenExtensions->f_azimuth_left[sc] = 0.0f;
      productionScreenExtensions->f_azimuth_right[sc] = 0.0f;
      productionScreenExtensions->elevation_top[sc] = 0;
      productionScreenExtensions->elevation_bottom[sc] = 0;
      productionScreenExtensions->f_elevation_top[sc] = 0.0f;
      productionScreenExtensions->f_elevation_bottom[sc] = 0.0f;
    }
  }
  nBitsRead = nBitsRead - __getValidBits ( hbitbuf );

  if ( nBitsRead % 8 ) {
    __getBits ( hbitbuf, dummyId, 8 - ( nBitsRead % 8 ) );
  }
}


static void Read_GroupPresetDefinitionExtension ( ascparserBitStreamPtr hbitbuf, GroupPresetDefinitionExtensionData* groupPresetExtensions , AudioSceneInfo* asi)
{
  int dummyId = 0;
  unsigned int nBitsRead = __getValidBits ( hbitbuf );
  int gp = 0, cnd = 0, egp = 0;
  for (gp = 0; gp < asi->numGroupPresets; gp++)
  {
    int hasDownmixIdGroupExt;
    groupPresetExtensions[gp].hasSwitchGroupConditions = __getBits ( hbitbuf, dummyId, 1 ) ;
    if (groupPresetExtensions[gp].hasSwitchGroupConditions == 1)
    {
      int numCond = asi->groupPresets[gp].presetNumConditions;
      for (cnd = 0; cnd < numCond; cnd++)
      {
        groupPresetExtensions[gp].isSwitchGroupCondition[cnd] =  __getBits ( hbitbuf, dummyId, 1 ) ;
      }
    }
    hasDownmixIdGroupExt = __getBits ( hbitbuf, dummyId, 1 ) ;
    if (hasDownmixIdGroupExt)
    {
      groupPresetExtensions[gp].numDownmixIdPresetExtensions =  __getBits ( hbitbuf, dummyId, 5 ) ;
      for (egp = 0; egp < groupPresetExtensions[gp].numDownmixIdPresetExtensions; egp++)
      {
        groupPresetExtensions[gp].downmixId[egp] = __getBits ( hbitbuf, dummyId, 7 ) ;
        groupPresetExtensions[gp].downmixIdPresetNumConditions[egp] = __getBits ( hbitbuf, dummyId, 4 ) ;
        groupPresetExtensions[gp].downmixIdPresetNumConditions[egp]++;
        for (cnd = 0; cnd < groupPresetExtensions[gp].downmixIdPresetNumConditions[egp]; cnd++)
        {
          groupPresetExtensions[gp].downmixIdPresetIsSwitchGroupCondition[egp][cnd] = __getBits ( hbitbuf, dummyId, 1 ) ;
          if (groupPresetExtensions[gp].downmixIdPresetIsSwitchGroupCondition[egp][cnd] == 1)
          { 
            groupPresetExtensions[gp].downmixIdPresetReferenceID[egp][cnd] = __getBits ( hbitbuf, dummyId, 5 ) ; /* switchGroupID */
          }
          else
          {
            groupPresetExtensions[gp].downmixIdPresetReferenceID[egp][cnd] = __getBits ( hbitbuf, dummyId, 7 ) ; /* groupID */
          }
          groupPresetExtensions[gp].downmixIdPresetConditionOnOff[egp][cnd] = __getBits ( hbitbuf, dummyId, 1) ;
          if (groupPresetExtensions[gp].downmixIdPresetConditionOnOff[egp][cnd] == 1)
          {
            groupPresetExtensions[gp].downmixIdPresetDisableGain[egp][cnd] = __getBits ( hbitbuf, dummyId, 1) ;
            groupPresetExtensions[gp].downmixIdPresetGainFlag[egp][cnd] = __getBits ( hbitbuf, dummyId, 1) ;
            if (groupPresetExtensions[gp].downmixIdPresetGainFlag[egp][cnd] == 1)
            {
              groupPresetExtensions[gp].downmixIdPresetGain[egp][cnd] = __getBits ( hbitbuf, dummyId, 8) ;
              groupPresetExtensions[gp].f_downmixIdPresetGain[egp][cnd] = ((float)groupPresetExtensions[gp].downmixIdPresetGain[egp][cnd] - 255.0f) * 0.5f + 32.0f;
            }
            groupPresetExtensions[gp].downmixIdPresetDisablePosition[egp][cnd] = __getBits ( hbitbuf, dummyId, 1) ;
            groupPresetExtensions[gp].downmixIdPresetPositionFlag[egp][cnd] = __getBits ( hbitbuf, dummyId, 1) ;
            if (groupPresetExtensions[gp].downmixIdPresetPositionFlag[egp][cnd] == 1)
            {
              groupPresetExtensions[gp].downmixIdPresetAz[egp][cnd] = __getBits ( hbitbuf, dummyId, 8) ;
              groupPresetExtensions[gp].f_downmixIdPresetAz[egp][cnd] = min(max(((float)groupPresetExtensions[gp].downmixIdPresetAz[egp][cnd] * 1.5f), -180.0f), 180.0f);
              groupPresetExtensions[gp].downmixIdPresetEl[egp][cnd] = __getBits ( hbitbuf, dummyId, 6) ;
              groupPresetExtensions[gp].f_downmixIdPresetEl[egp][cnd] = min(max(((float)groupPresetExtensions[gp].downmixIdPresetEl[egp][cnd] * 3.0f), -90.0f), 90.0f);
              groupPresetExtensions[gp].downmixIdPresetDist[egp][cnd] = __getBits ( hbitbuf, dummyId, 4) ;
              groupPresetExtensions[gp].f_downmixIdPresetDist[egp][cnd] = (float)pow(2.0f, ((float)groupPresetExtensions[gp].downmixIdPresetDist[egp][cnd] -12.0f));
            }
          }
        }
      }
    }
    else
    {
      groupPresetExtensions[gp].numDownmixIdPresetExtensions = 0;
    }
  }
  nBitsRead = nBitsRead - __getValidBits ( hbitbuf );

  if ( nBitsRead % 8 ) {
    __getBits ( hbitbuf, dummyId, 8 - ( nBitsRead % 8 ) );
  }
}

static void Read_ProductionScreenSizeData ( ascparserBitStreamPtr hbitbuf, ProductionScreenData* productionScreen) 
{
  int dummyId = 0;
  unsigned int nBitsRead = __getValidBits ( hbitbuf );
  productionScreen->hasNonStandardScreenSize =  __getBits ( hbitbuf, dummyId, 1 ) ;
  if ( productionScreen->hasNonStandardScreenSize == 0)
  {
    productionScreen->azimuth = 58;           /* +29.0f */
    productionScreen->elevation_top = 35;     /* +17.5f */
    productionScreen->elevation_bottom = -35; /* -17.5f */
  }
  else
  {
    productionScreen->azimuth = __getBits ( hbitbuf, dummyId, 9 ) ;
    productionScreen->elevation_top = __getBits ( hbitbuf, dummyId, 9 ) ;
    productionScreen->elevation_bottom = __getBits ( hbitbuf, dummyId, 9 ) ;
  }

  /* dequantize data -> integer to float */
  productionScreen->f_azimuth_left = 0.5f*(float)productionScreen->azimuth;
  productionScreen->f_azimuth_right = 0.5f*(float)productionScreen->azimuth * -1.0f;
  productionScreen->f_elevation_top = 0.5f*((float)productionScreen->elevation_top - 255);
  productionScreen->f_elevation_bottom = 0.5f*((float)productionScreen->elevation_bottom - 255);

  nBitsRead = nBitsRead - __getValidBits ( hbitbuf );

  if ( nBitsRead % 8 ) {
    __getBits ( hbitbuf, dummyId, 8 - ( nBitsRead % 8 ) );
  }
}

static void Read_LoudnessCompensationData ( ascparserBitStreamPtr hbitbuf, LoudnessCompensationData* loudnessCompensation , int numGroups, int numGroupPresets) {

  int grp=0, grpPreset=0;
  int dummyId = 0;
  unsigned int nBitsRead = __getValidBits ( hbitbuf );
    
  loudnessCompensation->loudnessCompensationDataPresent = 1;
  
  loudnessCompensation->groupLoudnessPresent = __getBits ( hbitbuf, dummyId, 1 ) ;
  if ( loudnessCompensation->groupLoudnessPresent == 1 ) {
    for ( grp = 0; grp < numGroups; grp++ ) {
      loudnessCompensation->groupLoudness[grp] = (float) __getBits ( hbitbuf, dummyId, 8 ) * 0.25f - 57.75f ;
    }
  } else {
    /* not present or provided by mpegh3daLoudnessInfoSet() */
  }
  
  loudnessCompensation->defaultParamsPresent = __getBits ( hbitbuf, dummyId, 1 ) ;
  if ( loudnessCompensation->defaultParamsPresent == 1 ) {
    for ( grp = 0; grp < numGroups; grp++ ) {
      loudnessCompensation->defaultIncludeGroup[grp] = __getBits ( hbitbuf, dummyId, 1 ) ;
    }
    loudnessCompensation->defaultMinMaxGainPresent = __getBits ( hbitbuf, dummyId, 1 ) ;
    if ( loudnessCompensation->defaultMinMaxGainPresent == 1 ) {
      loudnessCompensation->defaultMinGain = (float) __getBits ( hbitbuf, dummyId, 4 ) * (-3.f) ;
      loudnessCompensation->defaultMaxGain = (float) __getBits ( hbitbuf, dummyId, 4 ) * 3.f ;
    }
  }
  
  for ( grpPreset = 0; grpPreset < numGroupPresets; grpPreset++ ) {
    loudnessCompensation->presetParamsPresent[grpPreset] = __getBits ( hbitbuf, dummyId, 1 ) ;
    if ( loudnessCompensation->presetParamsPresent[grpPreset] == 1 ) {
      for ( grp = 0; grp < numGroups; grp++ ) {
        loudnessCompensation->presetIncludeGroup[grpPreset][grp] = __getBits ( hbitbuf, dummyId, 1 ) ;
      }
      loudnessCompensation->presetMinMaxGainPresent[grpPreset] = __getBits ( hbitbuf, dummyId, 1 ) ;
      if ( loudnessCompensation->presetMinMaxGainPresent[grpPreset] == 1 ) {
        loudnessCompensation->presetMinGain[grpPreset] = (float) __getBits ( hbitbuf, dummyId, 4 ) * (-3.f) ;
        loudnessCompensation->presetMaxGain[grpPreset] = (float) __getBits ( hbitbuf, dummyId, 4 ) * 3.f ;
      }
    }
  }
  nBitsRead = nBitsRead - __getValidBits ( hbitbuf );

  if ( nBitsRead % 8 ) {
    __getBits ( hbitbuf, dummyId, 8 - ( nBitsRead % 8 ) );
  }
}

static void Read_Data ( ascparserBitStreamPtr hbitbuf, AudioSceneInfo* asi ) {
  
  int i, j;
  int dummyId = 0;
  int type;
  int lengthBytes;
  
  asi->numDataSets = __getBits ( hbitbuf, dummyId, 4 );

  for ( i = 0; i < asi->numDataSets; i++ ) {
    int nBitsRead;
    type = __getBits ( hbitbuf, dummyId, 4 );
    lengthBytes = __getBits ( hbitbuf, dummyId, 16 );

    nBitsRead = __getValidBits(hbitbuf);

    switch ( type ) {
    case ID_MAE_GROUP_CONTENT:
      Read_ContentData ( hbitbuf, & ( asi->content ) );
      break;

    case ID_MAE_GROUP_DESCRIPTION:
      Read_DescriptionData ( hbitbuf, & ( asi->groupDescription ), ID_MAE_GROUP_DESCRIPTION );
      break;

    case ID_MAE_SWITCHGROUP_DESCRIPTION:
      Read_DescriptionData ( hbitbuf, & ( asi->switchGroupDescription ), ID_MAE_SWITCHGROUP_DESCRIPTION );
      break;

    case ID_MAE_GROUPPRESET_DESCRIPTION:
      Read_DescriptionData ( hbitbuf, & ( asi->groupPresetDescription ), ID_MAE_GROUPPRESET_DESCRIPTION );
      break;

    case ID_MAE_GROUP_COMPOSITE:
      Read_CompositeObjectPairData ( hbitbuf, & ( asi->compositeObjectPair ) );
      break;

    case ID_MAE_LOUDNESS_COMPENSATION:
      Read_LoudnessCompensationData ( hbitbuf, & ( asi->loudnessCompensation ), asi->numGroups, asi->numGroupPresets );
      break;

    case ID_MAE_DRC_UI_INFO:
      Read_DrcUserInterfaceInfoData( hbitbuf, & ( asi->drcUserInterfaceInfo ) );
      break;

    case ID_MAE_SCREEN_SIZE:
      Read_ProductionScreenSizeData( hbitbuf, & ( asi->productionScreen ) );
      break;

    case ID_MAE_SCREEN_SIZE_EXTENSION:
      Read_ProductionScreenSizeDataExtension ( hbitbuf, & ( asi->productionScreenExtensions ) );
      break;

    case ID_MAE_GROUPPRESET_EXTENSION:
      Read_GroupPresetDefinitionExtension ( hbitbuf, asi->groupPresetExtensions, asi );
      break;

      
    default:
      for ( j = 0; j < lengthBytes; j++ ) {
        __getBits ( hbitbuf, dummyId, 8 );
      }
    }

    nBitsRead = nBitsRead - __getValidBits(hbitbuf);
    if ((nBitsRead % 8) || ((nBitsRead / 8) != lengthBytes)) {
      fprintf(stderr, "\n!!WARNING!!::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::");
      fprintf(stderr, "\n!!WARNING!!  mae_dataLength is %d for dscr %d but mae_Data() contains %d payload bytes!", lengthBytes, i, nBitsRead / 8);
      fprintf(stderr, "\n!!           The current stream is NOT conforming to ISO/IEC 23008-3!");
      fprintf(stderr, "\n!!           Decoders might refuse decoding this stream!");
      fprintf(stderr, "\n!!WARNING!!::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n\n");
    }
  }
}

int Read_AudioSceneInfo ( ascparserBitStreamPtr hbitbuf, AudioSceneInfo* asi ) {
  
  int dummyId = 0;
  int readBits = 0;

  unsigned int nBitsRead = __getValidBits ( hbitbuf );

  asi->isMainStream = __getBits ( hbitbuf, dummyId, 1 );

  if ( asi->isMainStream ) {
    
    asi->audioSceneInfoIDPresent = __getBits ( hbitbuf, dummyId, 1 );

    if( asi->audioSceneInfoIDPresent )
    {
      asi->audioSceneInfoID = __getBits ( hbitbuf, dummyId, 8 );
    }
    else
    {
      asi->audioSceneInfoID = 0; /* a value of 0 means that the ID shall not be evaluated */
    }

    asi->numGroups = __getBits ( hbitbuf, dummyId, 7 );
    Read_GroupDefinition ( hbitbuf, asi->numGroups, asi->groups );
    asi->numSwitchGroups = __getBits ( hbitbuf, dummyId, 5 );
    Read_SwitchGroupDefinition ( hbitbuf, asi->numSwitchGroups, asi->switchGroups );

    asi->numGroupPresets = __getBits ( hbitbuf, dummyId, 5 );
    Read_GroupPresetDefinition ( hbitbuf, asi->numGroupPresets, asi->groupPresets );

    Read_Data ( hbitbuf, asi );

    asi->bsMetaDataElementIDoffset = 0;
    asi->mae_metaDataElementIDmaxAvail = __getBits ( hbitbuf, dummyId, 7 );

  } else {

    asi->bsMetaDataElementIDoffset = __getBits ( hbitbuf, dummyId, 7 );
    asi->bsMetaDataElementIDoffset = asi->bsMetaDataElementIDoffset + 1;
    asi->mae_metaDataElementIDmaxAvail = __getBits ( hbitbuf, dummyId, 7 );

  }

  readBits = nBitsRead - __getValidBits ( hbitbuf );

  if ( readBits % 8 ) {
    __getBits ( hbitbuf, dummyId, 8 - ( readBits % 8 ) );
  }

  readBits = nBitsRead - __getValidBits ( hbitbuf );

  return readBits;
}
