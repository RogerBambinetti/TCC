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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "oam_encode_ld.h"
#include "oam_read.h"

#ifndef ROUND
#define ROUND(x) (((x)<(0))?(int)((x)-0.5f):(int)((x)+0.5f))
#endif
#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#define NUM_OAM_COMPONENTS_LD (5)

typedef enum
{
  AZIMUTH                 = 0,
  ELEVATION               = 1,
  RADIUS                  = 2,
  GAIN                    = 3,
  SPREAD                  = 4,
  DYNAMIC_OBJECT_PRIORITY = 5,
  SPREAD_HEIGHT           = 6,
  SPREAD_DEPTH            = 7
} OAM_COMPONENT;


/*
** Quantizes a sultidata structure containing one oam frame to int values
*/
static int roundMultidataFrame( StructOamMultidata* oamFrame ){

  int num_objects = 0;
  int n = 0;

  /* Check if  multidata structure contains only one frame. */
  if( oamFrame->size2 > 1 )
  {
    return -1;
  }

  /* quantize the floating point values */
  num_objects = oamFrame->size1;

  for (n = 0; n < num_objects; n++)
  {
    oamFrame->azimuth[n]                    = (float)ROUND(oamFrame->azimuth[n]);
    oamFrame->elevation[n]                  = (float)ROUND(oamFrame->elevation[n]);
    oamFrame->radius[n]                     = (float)ROUND(oamFrame->radius[n]);
    oamFrame->gain[n]                       = (float)ROUND(oamFrame->gain[n]);
    oamFrame->spread[n]                     = (float)ROUND(oamFrame->spread[n]);
    oamFrame->spread_height[n]              = (float)ROUND(oamFrame->spread_height[n]);
    oamFrame->spread_depth[n]               = (float)ROUND(oamFrame->spread_depth[n]);
    oamFrame->dynamic_object_priority[n]    = (float)ROUND(oamFrame->dynamic_object_priority[n]);
  }

  return 0;
}

/*
** calculates a dcpm encoded frame
*/
static int calcDPCMFrame( StructOamMultidata* inputSignal,
                          StructOamMultidata* outputSignal,
                          StructOamMultidata* last)
{
  int count;
  
  /* Check for valid input parameters */
  if( inputSignal == NULL || outputSignal == NULL || last == NULL )
  {
    return -1;
  }
 
  /* Only multidata structures with one frame are supported */
  if( inputSignal->size2 > 1 )
  {
    return -1;
  }

  for( count = 0; count < inputSignal->size1 /* num_obj */; count++)
  { /* Here we process a dframe, which is a differentially coded frame */

    /* Calculate output azimuth vaule */   
    outputSignal->azimuth[count]     = inputSignal->azimuth[count] - last->azimuth[count];
    /* Calculate output elevation vaule */   
    outputSignal->elevation[count]   = inputSignal->elevation[count] - last->elevation[count];
    /* Calculate output radius valuse */
    outputSignal->radius[count]      = inputSignal->radius[count] - last->radius[count];
    /* Calculate output gain value */
    outputSignal->gain[count]        = inputSignal->gain[count] - last->gain[count];
    /* Calculate output spread value */
    outputSignal->spread[count]         = inputSignal->spread[count] - last->spread[count];
    outputSignal->spread_height[count]  = inputSignal->spread_height[count] - last->spread_height[count];
    outputSignal->spread_depth[count]   = inputSignal->spread_depth[count] - last->spread_depth[count];
    /* Calculate output DOP value */
    outputSignal->dynamic_object_priority[count]  = inputSignal->dynamic_object_priority[count] - last->dynamic_object_priority[count];
  }

  return 0;
}


static int requiredSignedBits(const int* data_row, int hasDynObjPrio, int hasUniformSpread, int* pBits)
{
  int xmin, xmax;
  int minval, maxval;
  int value;
  int k, kmax;

  /* Check, if input pointers/handlers are present. */
  if((data_row == NULL)||(pBits == NULL))
  {
    return -1;
  }

  /* determine the minimum and maximum values of data */
  value = data_row[0];
  xmin = value;
  xmax = value;
  kmax = NUM_OAM_COMPONENTS_LD+hasDynObjPrio; 
  for (k = 1; k < kmax; k++)
  {
    value = data_row[k];
    xmin = MIN(xmin, value);
    xmax = MAX(xmax, value);
  }

  if(!hasUniformSpread){ /*if sread is not uniform, check additional values */
    for (k = SPREAD_HEIGHT; k <= SPREAD_DEPTH; k++)
    {
      value = data_row[k];
      xmin = MIN(xmin, value);
      xmax = MAX(xmax, value);
    }
  }

  /* determine the required wordsize */
  *pBits = 2;
  minval = - (1<<((*pBits)-1));
  maxval = ((1<<(*pBits)) - 1) + minval;
  while (xmin < minval || xmax > maxval)
  {
    (*pBits)++;
    minval = - (1<<((*pBits)-1));
    maxval = ((1<<(*pBits)) - 1) + minval;
  }

  return 0;
}


static int singleDynamicObjectData(OamPBS* pbs, int* single_data, int* fixed_values,
                                   int flag_absolute, int hasDynObjPrio, int hasUniformSpread)
{
  int retVal = 0; /* Return value */
  int num_bits;
  int count;
  int flag[8] = {0,0,0,0,0,0,0,0};
  int wordsize[6] = {OAM_BITS_AZI, OAM_BITS_ELE, OAM_BITS_RAD, OAM_BITS_GAIN, OAM_BITS_SPREAD, OAM_BITS_PRIORITY};


   /* Check, if input pointers/handlers are present. */
  if((pbs == NULL)||(single_data == NULL)||(fixed_values == NULL))
  {
    return -1;
  }

  if(flag_absolute)
  {
    for(count = 0; count < NUM_OAM_COMPONENTS_LD+hasDynObjPrio; count++) /* Number of components increases by one when DOP is present */
    {
      if(!fixed_values[count])
      {
        if(retVal == 0)
        {
          oam_bitbuf_add(pbs, single_data[count], wordsize[count]);
          if( count == SPREAD && !hasUniformSpread){ /* if non uniform spread add additional values */
            oam_bitbuf_add(pbs, single_data[SPREAD_HEIGHT], wordsize[count]);
            oam_bitbuf_add(pbs, single_data[SPREAD_DEPTH], wordsize[count]);
          }
        }
      }
    } 
  }
  else
  {
    if(retVal == 0)
    {
      retVal = requiredSignedBits(single_data, hasDynObjPrio, hasUniformSpread, &num_bits);
    }

    if(retVal ==  0)
    {
      oam_bitbuf_add(pbs, (num_bits-2) , 3);
    }

    for(count = 0; count < NUM_OAM_COMPONENTS_LD+hasDynObjPrio; count++)
    {
      if(!fixed_values[count])
      {
        if(single_data[count])
        {
          flag[count] = 1;
        }
        if(retVal == 0)
        {
          oam_bitbuf_add(pbs, flag[count], 1); /*flag*/
        }
        if(flag[count])
        {
          if( count == 0)
          {
            if(retVal == 0)
            {
              oam_bitbuf_add(pbs, single_data[count], num_bits); /*position_difference*/
            }
          }
          else
          {
            if(retVal == 0)
            {
              oam_bitbuf_add(pbs, single_data[count], MIN(num_bits, (wordsize[count]+1)));
            }
          }
        }
        /* add additional spread values if needed*/
        if( count == SPREAD && !hasUniformSpread ){
          if(single_data[SPREAD_HEIGHT]){
            flag[SPREAD_HEIGHT] = 1;
            if(retVal == 0)
            {
              oam_bitbuf_add(pbs, flag[SPREAD_HEIGHT], 1); /*flag*/
              oam_bitbuf_add(pbs, single_data[SPREAD_HEIGHT], MIN(num_bits, (OAM_BITS_SPREAD_HEIGHT+1)));
            }
          }
          else{
            if(retVal == 0)
            {
              oam_bitbuf_add(pbs, flag[SPREAD_HEIGHT], 1); /*flag*/
            }
          }
          if(single_data[SPREAD_DEPTH]){
            flag[SPREAD_DEPTH] = 1;
            if(retVal == 0)
            {
              oam_bitbuf_add(pbs, flag[SPREAD_DEPTH], 1); /*flag*/
              oam_bitbuf_add(pbs, single_data[SPREAD_DEPTH], MIN(num_bits, (OAM_BITS_SPREAD_DEPTH+1)));
            }
          }
          else{
            if(retVal == 0)
            {
              oam_bitbuf_add(pbs, flag[SPREAD_DEPTH], 1); /*flag*/
            }
          }
        }
      }
    }
  }

  return retVal;
}


static int dynamicObjectData(OamPBS* pbs, StructOamMultidata* multidata, StructOamMultidata* multidata_diff, int* fixed_values, int hasDynObjPrio, int hasUniformSpread) 
{
  int retVal = 0; /* Return value */
  int count, count2nd; /* Counter variables. count2nd is used in nested loops */

  int num_objects = multidata->size1;
  int word_size = 0;
  int flag_absolute = 0;

  OamPBS singlePayload;

  int data_row[8];
  int data_diff_row[8];

  int size_absolute;
  int sum = 0;

  int wordsize[6] = {OAM_BITS_AZI, OAM_BITS_ELE, OAM_BITS_RAD, OAM_BITS_GAIN, OAM_BITS_SPREAD, OAM_BITS_PRIORITY};

  /*init arrays*/
  int needs_transmission[OAM_MAX_NUM_OBJECTS];

   /* Check, if input pointers/handlers are present. */
  if((pbs == NULL)||(multidata == NULL)||(multidata_diff == NULL)||(fixed_values == NULL))
  {
    return -1;
  }

  for(count = 0; count < NUM_OAM_COMPONENTS_LD+hasDynObjPrio; count++)
  {  
    if(!fixed_values[count])
    {
      word_size += wordsize[count];
      if( count == SPREAD && !hasUniformSpread ){
        word_size += OAM_BITS_SPREAD_HEIGHT;
        word_size += OAM_BITS_SPREAD_DEPTH;
      }
    }
  }

  /* Look for differences that need to be transmitted */
  for(count = 0; count < multidata->size1; count++)
  {
    sum = 0;

    if(!fixed_values[AZIMUTH])
    {
      /* Differences in azimuth */
      sum += abs(ROUND(multidata_diff->azimuth[count]));
    }
    if(!fixed_values[ELEVATION])
    {
      /* Differences in elevation */
      sum += abs(ROUND(multidata_diff->elevation[count]));
    }
    if(!fixed_values[RADIUS])
    {
      /* Differences in radius */
      sum += abs(ROUND(multidata_diff->radius[count]));
    }
    if(!fixed_values[GAIN])
    {
      /* Differences in gain */
      sum += abs(ROUND(multidata_diff->gain[count]));
    }
    if(!fixed_values[SPREAD])
    {
      /* Differences in spread */
      sum += abs(ROUND(multidata_diff->spread[count]));
      if(!hasUniformSpread){
        sum += abs(ROUND(multidata_diff->spread_height[count]));
        sum += abs(ROUND(multidata_diff->spread_depth[count]));
      }
    }
    if(!fixed_values[DYNAMIC_OBJECT_PRIORITY] && hasDynObjPrio)
    {
      /* Differences in DOP */
      sum += abs(ROUND(multidata_diff->dynamic_object_priority[count]));
    }

    /* Signal data available for transmission */
    if(sum > 0)
    {
      needs_transmission[count] = 1;
    }
    else
    {
      needs_transmission[count] = 0;
    }
  }
  
  pbs->num = 0;
  oam_bitbuf_add(pbs, flag_absolute, 1);  /*flag_absolute*/

  for(count = 0; count < multidata->size1; count++)
  {
    if(retVal == 0)
    {
      oam_bitbuf_add(pbs, needs_transmission[count], 1); /*has_object_data*/
    }
      
    if( needs_transmission[count] == 1) /*has_object_data*/
    { 
      data_diff_row[AZIMUTH]    = ROUND(multidata_diff->azimuth[count]);
      data_diff_row[ELEVATION]  = ROUND(multidata_diff->elevation[count]);
      data_diff_row[RADIUS]     = ROUND(multidata_diff->radius[count]);
      data_diff_row[GAIN]       = ROUND(multidata_diff->gain[count]);
      data_diff_row[SPREAD]     = ROUND(multidata_diff->spread[count]);
      if(!hasUniformSpread){
        data_diff_row[SPREAD_HEIGHT]    = ROUND(multidata_diff->spread_height[count]);
        data_diff_row[SPREAD_DEPTH]     = ROUND(multidata_diff->spread_depth[count]);
      }
      if(hasDynObjPrio)
      {
        data_diff_row[DYNAMIC_OBJECT_PRIORITY] = ROUND(multidata_diff->dynamic_object_priority[count]);
      }

      singlePayload.num = 0;
 
      if(retVal == 0)
      {
        retVal = singleDynamicObjectData(&singlePayload, data_diff_row, fixed_values, flag_absolute, hasDynObjPrio, hasUniformSpread);
      }

      for(count2nd = 0; count2nd < singlePayload.num; count2nd++)
      {
        if(retVal == 0)
        {
          oam_bitbuf_add(pbs, singlePayload.data[count2nd], singlePayload.size[count2nd]); 
        }
      }
    }
  }

  /*check if absolute encoding is more efficient*/
  sum = 0;
  for( count = 0; count < multidata->size1; count++ ){
    sum += needs_transmission[count];
  }

  size_absolute = 1 + multidata->size1 + sum * word_size;

  sum = 0;
  for( count = 0; count < pbs->num; count++ ){
    sum += pbs->size[count];
  }

  if(size_absolute <= sum){
    pbs->num = 0;
    flag_absolute = 1;

    if(retVal == 0)
    {
      oam_bitbuf_add(pbs, flag_absolute, 1);  /*flag_absolute*/
    }

    for(count = 0; count < num_objects; count++)
    {
      if(retVal == 0)
      {
        oam_bitbuf_add(pbs, needs_transmission[count], 1); /*has_object_data*/
      }

      if( needs_transmission[count] == 1){ /*has_object_data*/
        data_row[AZIMUTH]     = ROUND(multidata->azimuth[count]);
        data_row[ELEVATION]   = ROUND(multidata->elevation[count]);
        data_row[RADIUS]      = ROUND(multidata->radius[count]);
        data_row[GAIN]        = ROUND(multidata->gain[count]);
        data_row[SPREAD]      = ROUND(multidata->spread[count]);
        if(!hasUniformSpread){
          data_row[SPREAD_HEIGHT]    = ROUND(multidata_diff->spread_height[count]);
          data_row[SPREAD_DEPTH]     = ROUND(multidata_diff->spread_depth[count]);
        }
        if( hasDynObjPrio ){
          data_row[DYNAMIC_OBJECT_PRIORITY] = ROUND(multidata->dynamic_object_priority[count]);
        }

        singlePayload.num = 0;
  
        if(retVal == 0)
        {
          retVal = singleDynamicObjectData(&singlePayload, data_row, fixed_values, flag_absolute, hasDynObjPrio, hasUniformSpread); 
        }

        for( count2nd = 0; count2nd < singlePayload.num; count2nd++ )
        {
          if(retVal == 0)
          {
            oam_bitbuf_add(pbs, singlePayload.data[count2nd], singlePayload.size[count2nd]); 
          }
        }
      }
    }
  }

  return retVal;
}

static int intracodedObjectData(OamPBS* pbs, StructOamMultidata* multidata, int* fixed_values, int hasDynObjPrio, int hasUniformSpread)
{
  int count; /* Counter variable */

  /*common values - azi - ele - rad - gain - spread - dynobjprio*/
  int common_values[6] = {1,1,1,1,1,1};  /* common_values is set to 1 and will be set to 0 if differences are found */
  
  /* Check, if multidata structure uses common values for azimuth, elevation etc. */
  for(count = 1; count < multidata->size1; count++)
  {
    /* Check for common azimuth values. */
    if(common_values[AZIMUTH] && (multidata->azimuth[count] != multidata->azimuth[0]))
    {
      common_values[AZIMUTH] = 0;
    }
    /* Check for common elevation values. */
    if(common_values[ELEVATION] && (multidata->elevation[count] != multidata->elevation[0]))
    {
      common_values[ELEVATION] = 0;
    }
    /* Check for common radius values. */
    if(common_values[RADIUS] && (multidata->radius[count] != multidata->radius[0]))
    {
      common_values[RADIUS] = 0;
    }
    /* Check for common gain values. */
    if(common_values[GAIN] && (multidata->gain[count] != multidata->gain[0]))
    {
      common_values[GAIN] = 0;
    }
    /* Check for common spread values. */
    if(hasUniformSpread){
      if(common_values[SPREAD] && (multidata->spread[count] != multidata->spread[0]))
      {
        common_values[SPREAD] = 0;
      }
    }
    else{
      if(common_values[SPREAD] && (multidata->spread[count]        != multidata->spread[0]        || 
                                   multidata->spread_height[count] != multidata->spread_height[0] || 
                                   multidata->spread_depth[count]  != multidata->spread_depth[0])    )
      {
        common_values[SPREAD] = 0;
      }
    }
    if(hasDynObjPrio) /* We only look for common DOP values, if DOP is present. */
    {
      /* Check for common DOP values. */
      if(common_values[DYNAMIC_OBJECT_PRIORITY] && (multidata->dynamic_object_priority[count] != multidata->dynamic_object_priority[0]))
      {
        common_values[DYNAMIC_OBJECT_PRIORITY] = 0;
      }
    }
  }

  if(multidata->size1 > 1) /* We only use fixed values if we have more than one object. */
  {
    /* Fixed_Value / Common_Values : Azimuth */
    /* Add fixed_values flag to bitstream */
    oam_bitbuf_add(pbs, ROUND(fixed_values[AZIMUTH]), 1);
    if (fixed_values[AZIMUTH])
    {
      /* If fixed_values is set, add default azimuth */
      oam_bitbuf_add(pbs, ROUND(multidata->azimuth[0]), OAM_BITS_AZI);
    }
    else
    {
      /* Fixed_values ist not set. So we go on with the common values flag the same way. */
      oam_bitbuf_add(pbs, ROUND(common_values[AZIMUTH]), 1);
      if(common_values[AZIMUTH])
      {
        /* Since common_values is set, add default azimuth */
        oam_bitbuf_add(pbs, ROUND(multidata->azimuth[0]), OAM_BITS_AZI); 
      }
      else
      {
        /* Add individual values for all objects, since no fixed_values or common_values flag is set. */
        for (count = 0; count < multidata->size1; count++)
        {
          oam_bitbuf_add(pbs, ROUND(multidata->azimuth[count]), OAM_BITS_AZI);
        }
      }
    }

    /* Fixed_Value / Common_Values : Elevation */
    /* Add fixed_values flag to bitstream */
    oam_bitbuf_add(pbs, fixed_values[ELEVATION], 1);
    if(fixed_values[ELEVATION])
    {
      /* If fixed_values is set, add default elevation */
      oam_bitbuf_add(pbs, ROUND(multidata->elevation[0]), OAM_BITS_ELE);			
    }
    else
    {
      /* Fixed_values ist not set. So we go on with the common values flag the same way. */
      oam_bitbuf_add(pbs, common_values[ELEVATION], 1);
      if(common_values[ELEVATION])
      {
        /* Since common_values is set, add default elevation */
        oam_bitbuf_add(pbs, ROUND(multidata->elevation[0]), OAM_BITS_ELE );
      }
      else
      {
        /* Add individual values for all objects, since no fixed_values or common_values flag is set. */
        for(count = 0; count < multidata->size1; count++)
        {
          oam_bitbuf_add(pbs, ROUND(multidata->elevation[count]), OAM_BITS_ELE);
        }
      }
    }

    /* Fixed_Value / Common_Values : Radius */
    /* Add fixed_values flag to bitstream */
    oam_bitbuf_add(pbs, fixed_values[RADIUS], 1);
    if(fixed_values[RADIUS])
    {
      /* If fixed_values is set, add default radius */
      oam_bitbuf_add(pbs, ROUND(multidata->radius[0]), OAM_BITS_RAD);
    }
    else
    {
      /* Fixed_values ist not set. So we go on with the common values flag the same way. */
      oam_bitbuf_add(pbs, common_values[RADIUS], 1);
      if(common_values[RADIUS])
      {
        /* Since common_values is set, add default radius */
        oam_bitbuf_add(pbs, ROUND(multidata->radius[0]), OAM_BITS_RAD);
      }
      else
      {
        /* Add individual values for all objects, since no fixed_values or common_values flag is set. */
        for(count = 0; count < multidata->size1; count++)
        {
          oam_bitbuf_add(pbs, ROUND(multidata->radius[count]), OAM_BITS_RAD);
        }
      }
    }

    /* Fixed_Value / Common_Values : Gain */
    /* Add fixed_values flag to bitstream */
    oam_bitbuf_add(pbs, fixed_values[GAIN], 1);
    if(fixed_values[GAIN])
    {
      /* If fixed_values is set, add default gain */
      oam_bitbuf_add(pbs, ROUND(multidata->gain[0]), OAM_BITS_GAIN);
    }
    else
    {
      /* Fixed_values ist not set. So we go on with the common values flag the same way. */
      oam_bitbuf_add(pbs, common_values[GAIN], 1);
      if(common_values[GAIN])
      {
        /* Since common_values is set, add default gain */
        oam_bitbuf_add(pbs, ROUND(multidata->gain[0]), OAM_BITS_GAIN ); /* default_gain */
      }
      else
      {
        /* Add individual values for all objects, since no fixed_values or common_values flag is set. */
        for(count = 0; count < multidata->size1; count++)
        {
          oam_bitbuf_add(pbs, ROUND(multidata->gain[count]), OAM_BITS_GAIN);
        }
      }
    }

    /* Fixed_Value / Common_Values : Spread */
    /* Add fixed_values flag to bitstream */
    oam_bitbuf_add(pbs, fixed_values[SPREAD], 1);
    if(fixed_values[SPREAD])
    {
      /* If fixed_values is set, add default spread */
      oam_bitbuf_add(pbs, ROUND(multidata->spread[0]), OAM_BITS_SPREAD);
      if(!hasUniformSpread){ /* If spread is not uniform, add additional values */
        oam_bitbuf_add(pbs, ROUND(multidata->spread_height[0]), OAM_BITS_SPREAD_HEIGHT);
        oam_bitbuf_add(pbs, ROUND(multidata->spread_depth[0]), OAM_BITS_SPREAD_DEPTH);
      }
    }
    else
    {
      /* Fixed_values ist not set. So we go on with the common values flag the same way. */
      oam_bitbuf_add(pbs, common_values[SPREAD], 1);
      if(common_values[SPREAD])
      {
        /* Since common_values is set, add default spread */
        oam_bitbuf_add(pbs, ROUND(multidata->spread[0]), OAM_BITS_SPREAD);
        if(!hasUniformSpread){ /* If spread is not uniform, add additional values */
          oam_bitbuf_add(pbs, ROUND(multidata->spread_height[0]), OAM_BITS_SPREAD_HEIGHT);
          oam_bitbuf_add(pbs, ROUND(multidata->spread_depth[0]), OAM_BITS_SPREAD_DEPTH);
        }
      }
      else
      {
        /* Add individual values for all objects, since no fixed_values or common_values flag is set. */
        for (count = 0; count < multidata->size1; count++)
        {
          oam_bitbuf_add(pbs, ROUND(multidata->spread[count]), OAM_BITS_SPREAD);
          if(!hasUniformSpread){ /* If spread is not uniform, add additional values */
            oam_bitbuf_add(pbs, ROUND(multidata->spread_height[count]), OAM_BITS_SPREAD_HEIGHT);
            oam_bitbuf_add(pbs, ROUND(multidata->spread_depth[count]), OAM_BITS_SPREAD_DEPTH);
          }
        }
      }
    }

    /* If DOP is supported, we evaluate both flags for DOP, too. */
    if(hasDynObjPrio)
    { 
      /* Fixed_Value / Common_Values : DOP */
      /* Add fixed_values flag to bitstream */ 
      oam_bitbuf_add(pbs, fixed_values[DYNAMIC_OBJECT_PRIORITY], 1);
      if(fixed_values[DYNAMIC_OBJECT_PRIORITY])
      {
        /* If fixed_values is set, add default DOP */
        oam_bitbuf_add(pbs, ROUND(multidata->dynamic_object_priority[0]), OAM_BITS_PRIORITY);
      }
      else
      {
        /* Fixed_values ist not set. So we go on with the common values flag the same way. */
        oam_bitbuf_add(pbs, common_values[DYNAMIC_OBJECT_PRIORITY], 1);
        if (common_values[DYNAMIC_OBJECT_PRIORITY])
        {
          /* Since common_values is set, add default DOP */
          oam_bitbuf_add(pbs, ROUND(multidata->dynamic_object_priority[0]), OAM_BITS_PRIORITY);
        }
        else
        {
          /* Add individual values for all objects, since no fixed_values or common_values flag is set. */
          for (count = 0; count < multidata->size1; count++)
          {
            oam_bitbuf_add(pbs, ROUND(multidata->dynamic_object_priority[count]), OAM_BITS_PRIORITY);	/* dyn_obj_prio */
          }
        }
      }
    }
  }
  else
  {
    /* If we have one object only, we just add the values. */
    oam_bitbuf_add(pbs, ROUND(multidata->azimuth[0]),  OAM_BITS_AZI);    /* position_azimuth */
    oam_bitbuf_add(pbs, ROUND(multidata->elevation[0]),  OAM_BITS_ELE);	/* position_elevation */
    oam_bitbuf_add(pbs, ROUND(multidata->radius[0]),  OAM_BITS_RAD);		  /* position_radius */
    oam_bitbuf_add(pbs, ROUND(multidata->gain[0]), OAM_BITS_GAIN);	      /* gain_factor */
    oam_bitbuf_add(pbs, ROUND(multidata->spread[0]), OAM_BITS_SPREAD);	  /* spread */
    if(!hasUniformSpread){ /* If spread is not uniform, add additional values */
      oam_bitbuf_add(pbs, ROUND(multidata->spread_height[0]), OAM_BITS_SPREAD_HEIGHT);
      oam_bitbuf_add(pbs, ROUND(multidata->spread_depth[0]), OAM_BITS_SPREAD_DEPTH);
    }
    if(hasDynObjPrio)
    {
      oam_bitbuf_add(pbs, ROUND(multidata->dynamic_object_priority[0]), OAM_BITS_PRIORITY);	/* dyn_obj_prio */
    }
  }

  return 0;
}

static int updateLastSample( StructOamMultidata* inputSignal,
                             StructOamMultidata* last)
{
  int count;
  
  /* Check for valid input parameters */
  if((inputSignal == NULL)||(last == NULL))
  {
    return -1;
  }
 
  /* Only multidata structures with one frame are supported */
  if( inputSignal->size2 > 1 )
  {
    return -1;
  }
  
  for( count = 0; count < inputSignal->size1 /* num_obj */; count++)
  {
    /* Set history azimuth value */
    last->azimuth[count]            = inputSignal->azimuth[count];
    /* Set history elevation value */
    last->elevation[count]          = inputSignal->elevation[count];
    /* Set history radius value */
    last->radius[count]             = inputSignal->radius[count];
    /* Set history gain value */
    last->gain[count]               = inputSignal->gain[count];
    /* Set history spread value */
    last->spread[count]             = inputSignal->spread[count];
    last->spread_height[count]      = inputSignal->spread_height[count];
    last->spread_depth[count]       = inputSignal->spread_depth[count];
    /* Set history DOP value */
    last->dynamic_object_priority[count]         = inputSignal->dynamic_object_priority[count];
  }

  return 0;
}

static int compressOamData(OamPBS* pbs, 
                           StructOamMultidata* multidata,
                           StructOamMultidata* multidata_diff, 
                           StructOamMultidata* multidata_last,
                           int is_iframe, int *fixed_values, int hasDynObjPrio, int hasUniformSpread)
{

  int count = 0; /* Counter variable */
  int ret = 0;

  OamPBS payloadIntra; /* Buffer for payload coded as intracoded object */
  OamPBS payloadDiff; /* Buffer for payload coded as differentially coded object */
  
  int dataSize = 0;         /* Final size of the coded payload */
  int dataSizeDiff = 0;     /* Size of the payload (intracoded) */
  int dataSizeIntra = 0;    /* Size of the payload (diferentially coded) */
 
  /* Check, if input structures are valid */
  if((multidata == NULL) || (multidata_diff == NULL) || (fixed_values == NULL) || (pbs == NULL))
  {
    return -1;
  }

  /*reset pbs*/
  pbs->num = 0;
  payloadIntra.num = 0;
  payloadDiff.num = 0;


  if(is_iframe)
  {
    /* We can only use intracoding if is_iframe is set */
    oam_bitbuf_add(pbs, is_iframe, 1);/* has_intracoded_object_data */
    ret = intracodedObjectData(&payloadIntra, multidata, fixed_values, hasDynObjPrio, hasUniformSpread);
    if(ret) return -1;

    for( count = 0; count < payloadIntra.num; count++){
      oam_bitbuf_add(pbs, payloadIntra.data[count], payloadIntra.size[count]); /*intracoded_object_data*/
    }
  }
  else
  {
    /* We can use intracoding or differential coding. We try both... */
    ret = intracodedObjectData(&payloadIntra, multidata, fixed_values, hasDynObjPrio, hasUniformSpread);
    if(ret) return -1;

    ret = calcDPCMFrame( multidata, multidata_diff, multidata_last);
    if(ret) return -1;

    ret = dynamicObjectData(&payloadDiff, multidata, multidata_diff, fixed_values, hasDynObjPrio, hasUniformSpread);
    if(ret) return -1;

    /* Calculate payload size for using intracoding */
    for( count = 0; count < payloadIntra.num; count++ )
    {
      dataSizeIntra += payloadIntra.size[count];
    }
    /* Calculate payload size for using differential coding */
    for( count = 0; count < payloadDiff.num; count++ )
    {
      dataSizeDiff += payloadDiff.size[count];
    }

    /* ...and take the better (=smaller) alternative. */ 
    if(dataSizeDiff < dataSizeIntra)
    {
      oam_bitbuf_add(pbs, is_iframe, 1);      /*has_intracoded_object_data*/
      for( count = 0; count < payloadDiff.num; count++)
      {
        oam_bitbuf_add(pbs, payloadDiff.data[count], payloadDiff.size[count]);      /*dynamic_object_data*/
      }
    }
    else
    {
      is_iframe = 1;
      oam_bitbuf_add(pbs, is_iframe, 1);      /*has_intracoded_object_data*/
      for( count = 0; count < payloadIntra.num; count++)
      {
        oam_bitbuf_add(pbs, payloadIntra.data[count], payloadIntra.size[count]);      /*dynamic_object_data*/
      }
    }
  }

  ret = updateLastSample( multidata, multidata_last);
  if(ret) return -1;
  
  dataSize = 0;
  for( count = 0; count < pbs->num; count++ )
  {
      dataSize += pbs->size[count];
  }

  /*clear empty frames*/ 
  if(dataSize == 1)
  {
    pbs->num = 0;
  }

  return 0;
}


/*
**ecnodes one oam frame in low delay mode
*/
int encode_frame_ld(StructOamMultidata* oamInputData,
                    StructOamMultidata* oamPreviousData,
                    StructOamMultidata* oamDiffValues,
                    int *fixed_values,
                    int is_iframe,
                    const DYNAMIC_OBJECT_PRIO hasDynamicObjectPriority,
                    const unsigned int hasUniformSpread,
                    OamPBS* pbs
){
  int ret = 0;

  /* some sanity checks on input data: */
  if( oamInputData == NULL || oamPreviousData == NULL || oamDiffValues == NULL ){
    return -1;
  }

  /* round oam multidata - (only single frame) */
  if( roundMultidataFrame(oamInputData) ) return -1;

  /* encode current frame */
  ret = compressOamData( pbs, oamInputData, oamDiffValues, oamPreviousData, is_iframe, fixed_values, hasDynamicObjectPriority, hasUniformSpread );

  return ret;

}





