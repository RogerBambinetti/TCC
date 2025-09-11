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

#include <string.h>

#include "gVBAPRenderer.h"

#include "speakersetup.hpp"
#include "vbap_ghost_wrapper.hpp"
#include "vbap_core.hpp"
#include "functions.hpp"
#include "types.hpp"

using namespace iosono::mpeg::referencerenderer;
using namespace iosono::mpeg;

#define GVBAPRENDERER_MAX_OBJECTS 32

typedef struct _GVBAPRENDERER
{
  VbapInterface* core;
  Speakersetup* speakerSetup;

  float* startGains[GVBAPRENDERER_MAX_OBJECTS]; /* Dim: [numObjects][numChannels] */
  float* endGains;                              /* Dim: [numChannels] */

  int numChannels;
  int numObjects;
  int frameLength;
  int frmCnt;
} GVBAPRENDERER;


static inline void rotateToCoreCoordinateSystem(OAM_SAMPLE lhs, geo::SphericalPosition& rhs)
{

  geo::SphericalPosition posAng(lhs.azimuth, lhs.elevation, lhs.radius);
 
  rhs = posAng;
}


int gVBAPRenderer_Open(HANDLE_GVBAPRENDERER* phgVBAPRenderer, int numObjects, int frameLength, int outCICPIndex, 
                       CICP2GEOMETRY_CHANNEL_GEOMETRY* outGeometryInfo, int outChannels)
{
  HANDLE_GVBAPRENDERER tmp;
  int i;

  tmp = (HANDLE_GVBAPRENDERER) calloc(1, sizeof(GVBAPRENDERER));
  if ( !tmp )
    return -1;

  *phgVBAPRenderer = tmp;
 
  (*phgVBAPRenderer)->speakerSetup = new Speakersetup();
 
  (*phgVBAPRenderer)->speakerSetup->init(outGeometryInfo, outChannels);

   std::vector<geo::Vector3d> speakers((*phgVBAPRenderer)->speakerSetup->getAllSpeakerPositions());
   std::vector<geo::VertexTriangle> triangles = (*phgVBAPRenderer)->speakerSetup->getSpeakerTriangles();

   std::auto_ptr<VbapInterface> core(new VbapCore());
   (*phgVBAPRenderer)->core = new VbapGhostWrapper((*phgVBAPRenderer)->speakerSetup->getDowmixMatrix(), core);


   (*phgVBAPRenderer)->core->init(speakers, triangles, true, (*phgVBAPRenderer)->speakerSetup->hasHeightSpeakers());

   (*phgVBAPRenderer)->numChannels =  (*phgVBAPRenderer)->speakerSetup->getNumSpeakers() + 
                                      (*phgVBAPRenderer)->speakerSetup->getNumSubwoofers();

   if ( (*phgVBAPRenderer)->numChannels != outChannels )
   {
     fprintf(stderr, "gVBAP num internal loudspeakers do not match intput number of loudspeakers\n");
     return -1;
   }

   for (i = 0; i < numObjects; i++) {
     (*phgVBAPRenderer)->startGains[i] = (float*) calloc((*phgVBAPRenderer)->numChannels , sizeof(float) );
   }

   (*phgVBAPRenderer)->endGains    = (float*) calloc((*phgVBAPRenderer)->numChannels , sizeof(float) );
   (*phgVBAPRenderer)->numObjects  = numObjects;
   (*phgVBAPRenderer)->frameLength = frameLength;
   (*phgVBAPRenderer)->frmCnt      = 0;

  return 0;
}

int gVBAPRenderer_RenderFrame_Time(HANDLE_GVBAPRENDERER hgVBAPRenderer, 
                                   float** inputBuffer, float** outputBuffer, OAM_SAMPLE* oamStartSample, OAM_SAMPLE* oamStopSample, int hasUniformSpread)
{
  int i = 0;
  int obj = 0;
  int channel = 0;

  /* Zero Out output buffer */
  for (i = 0; i < hgVBAPRenderer->numChannels; ++i)
  {
     memset(outputBuffer[i], 0, hgVBAPRenderer->frameLength * sizeof(float) );
  }

  /* Do the rendering for all objects */
  for ( obj = 0; obj < hgVBAPRenderer->numObjects; ++obj )
  {
    float spread[4];

    geo::SphericalPosition startPosAng, endPosAng;
    rotateToCoreCoordinateSystem(oamStartSample[obj], startPosAng);
    rotateToCoreCoordinateSystem(oamStopSample[obj], endPosAng);
    geo::Vector3d startPos(startPosAng), endPos(endPosAng);

    spread[0] = oamStopSample[obj].spread_angle;
    spread[1] = oamStopSample[obj].spread_angle;
    spread[2] = oamStopSample[obj].spread_angle_height;
    spread[3] = oamStopSample[obj].spread_depth;

    /* initialize the startGains of the very first frame: */
    if (0 == hgVBAPRenderer->frmCnt) {
      hgVBAPRenderer->core->calculcateVbap(startPos, hgVBAPRenderer->startGains[obj], oamStartSample[obj].gain, spread, hasUniformSpread);
    }

    hgVBAPRenderer->core->calculcateVbap(endPos, hgVBAPRenderer->endGains, oamStopSample[obj].gain, spread, hasUniformSpread);

    for ( channel = 0; channel < hgVBAPRenderer->numChannels; ++channel )
    {
      int mappedChannel = 0;
      int sample = 0;
      float step = (hgVBAPRenderer->endGains[channel] - hgVBAPRenderer->startGains[obj][channel]) / hgVBAPRenderer->frameLength;
      float scale = hgVBAPRenderer->startGains[obj][channel] + step;

      if (hgVBAPRenderer->speakerSetup->hasOutputMapping())
      {
        std::vector<int> outputMapping = hgVBAPRenderer->speakerSetup->getOutputMapping();
        mappedChannel = outputMapping[channel];
      }
      else
      {
        mappedChannel = channel;
      }

      for (sample = 0; sample < hgVBAPRenderer->frameLength; ++sample)
      {
        outputBuffer[mappedChannel][sample] += inputBuffer[obj][sample] * scale;
        scale += step;
      }

      hgVBAPRenderer->startGains[obj][channel] = hgVBAPRenderer->endGains[channel];
    }
  }

  hgVBAPRenderer->frmCnt++;

  return 0;
}


int gVBAPRenderer_RenderFrame_Frequency(HANDLE_GVBAPRENDERER hgVBAPRenderer, 
                                   float** inputBuffer, float** outputBuffer, OAM_SAMPLE* oamStartSample, 
                                   OAM_SAMPLE* oamStopSample, int numQmfBands, int hasUniformSpread)
{
  int i = 0;
  int obj = 0;
  int channel = 0;


  /* Zero Out output buffer */
  for (i = 0; i < hgVBAPRenderer->numChannels; ++i)
  {
     memset(outputBuffer[i], 0, hgVBAPRenderer->frameLength * sizeof(float) );
  }

  /* Do the rendering for all objects */
  for ( obj = 0; obj < hgVBAPRenderer->numObjects; ++obj )
  {
    float spread[4];
   
    geo::SphericalPosition startPosAng, endPosAng;
    rotateToCoreCoordinateSystem(oamStartSample[obj], startPosAng);
    rotateToCoreCoordinateSystem(oamStopSample[obj], endPosAng);
    geo::Vector3d startPos(startPosAng), endPos(endPosAng);

    spread[0] = oamStopSample[obj].spread_angle;
    spread[1] = oamStopSample[obj].spread_angle;
    spread[2] = oamStopSample[obj].spread_angle_height;
    spread[3] = oamStopSample[obj].spread_depth;

    /* initialize the startGains of the very first frame: */
    if (0 == hgVBAPRenderer->frmCnt) {
      hgVBAPRenderer->core->calculcateVbap(startPos, hgVBAPRenderer->startGains[obj], oamStartSample[obj].gain, spread, hasUniformSpread);
    }

    hgVBAPRenderer->core->calculcateVbap(endPos, hgVBAPRenderer->endGains, oamStopSample[obj].gain, spread, hasUniformSpread);

    for ( channel = 0; channel < hgVBAPRenderer->numChannels; ++channel )
    {
      int mappedChannel = 0;
      int timeslot = 0;
      int qmfband = 0;
      int noOfTimeslots = hgVBAPRenderer->frameLength / numQmfBands;
      float step = (hgVBAPRenderer->endGains[channel] - hgVBAPRenderer->startGains[obj][channel]) / noOfTimeslots;
      float scale = hgVBAPRenderer->startGains[obj][channel] + step;

      if (hgVBAPRenderer->speakerSetup->hasOutputMapping())
      {
        std::vector<int> outputMapping = hgVBAPRenderer->speakerSetup->getOutputMapping();
        mappedChannel = outputMapping[channel];
      }
      else
      {
        mappedChannel = channel;
      }

      for ( timeslot = 0; timeslot < noOfTimeslots; ++timeslot )
      {
        for ( qmfband = 0; qmfband < numQmfBands; qmfband++ ) {
          outputBuffer[mappedChannel][qmfband + (timeslot * numQmfBands) ] += inputBuffer[obj][qmfband + (timeslot * numQmfBands) ] * scale;
        }

        scale += step;
      }

      hgVBAPRenderer->startGains[obj][channel] = hgVBAPRenderer->endGains[channel];
    }
  }

  hgVBAPRenderer->frmCnt++;

  return 0;
}


int gVBAPRenderer_GetGains(HANDLE_GVBAPRENDERER hgVBAPRenderer, OAM_SAMPLE* oamSample, float** gainMatrix, int hasUniformSpread)
{
  int obj = 0;
  int channel = 0;

  /* Do the rendering for all objects */
  for ( obj = 0; obj < hgVBAPRenderer->numObjects; ++obj )
  {

    float spread[4];

    geo::SphericalPosition posAng;
    rotateToCoreCoordinateSystem(oamSample[obj], posAng);
    geo::Vector3d pos(posAng);

    spread[0] = oamSample[obj].spread_angle;
    spread[1] = oamSample[obj].spread_angle;
    spread[2] = oamSample[obj].spread_angle_height;
    spread[3] = oamSample[obj].spread_depth;

    hgVBAPRenderer->core->calculcateVbap(pos, hgVBAPRenderer->startGains[obj], oamSample[obj].gain, spread, hasUniformSpread);

    /* Return gains for all objects and channels */
    for ( channel = 0; channel < hgVBAPRenderer->numChannels; ++channel ) {
      int mappedChannel = 0;
      if (hgVBAPRenderer->speakerSetup->hasOutputMapping())
      {
        std::vector<int> outputMapping = hgVBAPRenderer->speakerSetup->getOutputMapping();
        mappedChannel = outputMapping[channel];
      }
      else
      {
        mappedChannel = channel;
      }

      gainMatrix[obj][mappedChannel] = hgVBAPRenderer->startGains[obj][channel];
    }
  }
  return 0;
}


int gVBAPRenderer_Close(HANDLE_GVBAPRENDERER hgVBAPRenderer)
{
  int i = 0;

  if ( hgVBAPRenderer )
  {
    if (hgVBAPRenderer->speakerSetup )
      delete hgVBAPRenderer->speakerSetup;

    if ( hgVBAPRenderer->core )
      delete hgVBAPRenderer->core;

    for (i = 0; i < hgVBAPRenderer->numObjects; i++) {
      if ( hgVBAPRenderer->startGains[i] )
        free (hgVBAPRenderer->startGains[i]);
    }

    if ( hgVBAPRenderer->endGains )
      free (hgVBAPRenderer->endGains);
 
    free( hgVBAPRenderer );
  }

  return 0;
}

int gVBAPRenderer_GetStaticGains(CICP2GEOMETRY_CHANNEL_GEOMETRY* inGeometryInfo, int numObjects, int outCICPIndex, 
                       CICP2GEOMETRY_CHANNEL_GEOMETRY* outGeometryInfo, int outChannels, float** staticGainsMatrix)
{


  Speakersetup* SpeakerSetup = new Speakersetup();

  SpeakerSetup->init(outGeometryInfo, outChannels);

  std::vector<geo::Vector3d> speakers(SpeakerSetup->getAllSpeakerPositions());
  std::vector<geo::VertexTriangle> triangles = SpeakerSetup->getSpeakerTriangles();

  std::auto_ptr<VbapInterface> _core(new VbapCore());
  VbapInterface* Core = new VbapGhostWrapper(SpeakerSetup->getDowmixMatrix(), _core);
  Core->init(speakers, triangles, true, SpeakerSetup->hasHeightSpeakers());

  float* gains = (float*) calloc( outChannels , sizeof(float) );

  for ( int i = 0; i < numObjects; ++i )
  {
    float spread[4];

    OAM_SAMPLE sample;
    sample.azimuth   = (float) inGeometryInfo[i].Az * (float)(M_PI / 180.0);
    sample.elevation = (float) inGeometryInfo[i].El * (float)(M_PI / 180.0);
    sample.gain      = 1.f;
    sample.radius    = 1.f;
    sample.spread_angle = 0.0f;
    sample.spread_angle_height = 0.0f;
    sample.spread_depth = 0.0f;

    spread[0] = sample.spread_angle;
    spread[1] = sample.spread_angle;
    spread[2] = sample.spread_angle_height;
    spread[3] = sample.spread_depth;
    
    geo::SphericalPosition posAng;
    rotateToCoreCoordinateSystem(sample, posAng);
    geo::Vector3d pos(posAng);
 
    Core->calculcateVbap(pos, gains, sample.gain, spread, 1);
   
    for ( int j = 0; j < outChannels; ++j ) {
      int mappedChannel = 0;
      if (SpeakerSetup->hasOutputMapping())
      {
            std::vector<int> outputMapping = SpeakerSetup->getOutputMapping();
            mappedChannel = outputMapping[j];
      }
      else
      {
            mappedChannel = j;
      }
          staticGainsMatrix[i][mappedChannel] = gains[j];
    } 
  }

  if ( gains )
    free(gains);

  if ( Core )
    delete Core;

  if ( SpeakerSetup )
    delete SpeakerSetup;

  return 0;
}



