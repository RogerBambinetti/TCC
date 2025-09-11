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

#include "localSetupInformationInterfaceLib.h"
#include "localSetupInformationInterfaceLib_BitstreamIO.h"
#include "bitstreamBinaural.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const int brirSamplingFrequencyTable[32] = {
	96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 
	16000, 12000, 11025, 8000, 7350, -1, -1, 57600,
	51200, 40000, 38400, 34150, 28800, 25600, 20000, 19200,
	17075, 14400, 12800, 9600, -1, -1, -1, 0
};

static int getBrirSamplingFrequencyIdx(int fs)
{
	short ii;
	for (ii=0; ii<32; ++ii)
	{
		if ( brirSamplingFrequencyTable[ii] == fs )
			return ii;
	}

	return 0x1F; /* escape code */
}

int LS_setLoudspeakerLayout_fromFile(LOCAL_SETUP_HANDLE h_LocalSetupHandle, char *filename, int *numSpeakers, int *extDistComp, int **LSdistance, float **LscalibGain)
{
  /**
	*   Read and set the loudspeaker layout from a dedicated file
	*   @param    h_LocalSetupHandle    : local setup handle
	*   @param    filename		          : path+filename of the file
  *   @param    numSpeakers		        : output parameter: number of speakers
  *   @param    extDistComp		        : output parameter: flag that signals if an external distance compensation is applied
  *   @param    LSdistance		        : output parameter: array of distances of speakers
  *   @param    LscalibGain		        : output parameter: array of calibration gains of speakers
	*
	*   @return   ERROR_CODE
	*
  *   FILE HAS TO BE STRUCTURED IN THE FOLLOWING FORM 
  *
  *   line 1: numSpeakers, CICPLayoutIdx (-1 if none), externalDistanceCompensation (0|1)
  *
  *     if CICPLayoutIdx == -1:
  *         line 2 to Line 2+numSpeakers-1 either:
  *         g, az, el, isLfe, LSdist, LScalibGain
  *         c, cicpSpeakerIdx, knownAz, knownEl, LSdist, LScalibGain
  *
  *     if CICPLayoutIdx > 0:
  *       either no more lines following, or
  *         line 2 to Line 2+numSpeakers-1:
  *         knownAz, knownEl, LSdist, LScalibGain
  *
  */

  CHANNEL_GEOMETRY myTable[32] = {{ 0 }};
  CICP2GEOMETRY_CHANNEL_GEOMETRY myTableCICP[32] = {{ 0 }};
  int numChannels = 0;
  int numLFEs = 0;
  int error = 0;
  int cicpIndex = 0;
  int k;

  int *LSdist = NULL;
  float *LScalibGain = NULL;
  int *hasKnownPositions = NULL;
  float *Azimuth = NULL;
  float *Elevation = NULL;

  float *Azimuth2 = NULL;
  float *Elevation2 = NULL;

  int *isLFE = NULL;
  int *cicpSpIdx = NULL;
  int *alsoAddSymmetricPair = NULL;

  int hasLoudspeakerDistance = 1;
  int hasCalibrationGain = 1;
  int anyCICPspeaker = 0;
  int allCICPspeaker = 1;
  int anyKnownPos = 0;
  int knownIdx = -1;

  int externalDistComp = -1;

  int numExplicitelySignaledSpeakers = 0;


  if ( filename != NULL ) 
  {
    error = LS_getGeometryFromFile( filename, myTable, &numChannels, &externalDistComp, &cicpIndex, 2);
    *numSpeakers = numChannels;
    /* the following part is needed for being also able to read geometry files as defined in cicp2geometry */
    if (error == 2)
    {
      cicp2geometry_get_geometry_from_file( filename, myTableCICP, &numChannels, &numLFEs);
      externalDistComp = 1;
      cicpIndex = -1;
      *numSpeakers = numChannels + numLFEs;
      for (k = 0; k < *numSpeakers; k++)
      {
        myTable[k].Az = myTableCICP[k].Az;
        myTable[k].cicpLoudspeakerIndex = myTableCICP[k].cicpLoudspeakerIndex;
        myTable[k].El = myTableCICP[k].El;
        myTable[k].LFE = myTableCICP[k].LFE;
        myTable[k].dist = -1;
        myTable[k].calibGain = -33.0f;
      }
    }
  }

  *extDistComp = externalDistComp;

  if (cicpIndex > -1)
  {

    LSdist = (int*)calloc(numChannels,sizeof(int));
    LScalibGain = (float*)calloc(numChannels,sizeof(float));
    hasKnownPositions = (int*)calloc(numChannels,sizeof(int));
    Azimuth = (float*)calloc(numChannels,sizeof(float));
    Elevation = (float*)calloc(numChannels,sizeof(float));

    Azimuth2 = (float*)calloc(numChannels,sizeof(float));
    Elevation2 = (float*)calloc(numChannels,sizeof(float));

    for (k = 0; k < numChannels; k++)
    {
      LSdist[k] = myTable[k].dist;
      LScalibGain[k] = myTable[k].calibGain;
      Azimuth[k] = (float)myTable[k].Az;
      Elevation[k] = (float)myTable[k].El;
      if (Azimuth[k] != -181 && Elevation[k] != -91)
      {
        knownIdx++;
        hasKnownPositions[k] = 1;
        anyKnownPos = 1;
        Azimuth2[knownIdx] = Azimuth[k];
        Elevation2[knownIdx] = Elevation[k];
      }
      if (LSdist[k] == -1) 
      {
        hasLoudspeakerDistance = 0;
      }
      if (LScalibGain[k] == -33.0f)
      {
        hasCalibrationGain = 0;
      }
    }

    if (hasLoudspeakerDistance == 0)
    {
      free(LSdist); LSdist = NULL;
    }
    if (hasCalibrationGain == 0)
    {
      free(LScalibGain); LScalibGain = NULL;
    }
    if (anyKnownPos == 0)
    {
      free(hasKnownPositions); hasKnownPositions = NULL;
      free(Azimuth); Azimuth = NULL;
      free(Elevation); Elevation = NULL;

      free(Azimuth2); Azimuth2 = NULL;
      free(Elevation2); Elevation2 = NULL;
    }

    error = LS_setLoudspeakerRenderingConfig(h_LocalSetupHandle,externalDistComp, numChannels, LSdist,LScalibGain, 0);
    error = LS_setFixedLoudspeakerLayout(h_LocalSetupHandle, cicpIndex, NULL, hasKnownPositions, Azimuth2, Elevation2);
  
    if (LSdist != NULL) { free(LSdist); LSdist = NULL;}
    if (LScalibGain != NULL) {free(LScalibGain); LScalibGain = NULL;}
    if (anyKnownPos == 1)
    {
      free(hasKnownPositions); hasKnownPositions = NULL;
      free(Azimuth); Azimuth = NULL;
      free(Elevation); Elevation = NULL;

      free(Azimuth2); Azimuth2 = NULL;
      free(Elevation2); Elevation2 = NULL;
    }
  }

  else
  {
    LSdist = (int*)calloc(numChannels,sizeof(int));
    LScalibGain = (float*)calloc(numChannels,sizeof(float));
    Azimuth = (float*)calloc(numChannels,sizeof(float));
    Elevation = (float*)calloc(numChannels,sizeof(float));
    isLFE = (int*)calloc(numChannels,sizeof(int));
    cicpSpIdx = (int*)calloc(numChannels,sizeof(int));
    alsoAddSymmetricPair = (int*)calloc(numChannels,sizeof(int));
    hasKnownPositions = (int*)calloc(numChannels,sizeof(int));

    Azimuth2 = (float*)calloc(numChannels,sizeof(float));
    Elevation2 = (float*)calloc(numChannels,sizeof(float));


    for (k = 0; k < numChannels; k++)
    {
      LSdist[k] = myTable[k].dist;
      LScalibGain[k] = myTable[k].calibGain;
      Azimuth[k] = (float)myTable[k].Az;
      Elevation[k] = (float)myTable[k].El;
      isLFE[k] = myTable[k].LFE;
      cicpSpIdx[k] = myTable[k].cicpLoudspeakerIndex;
      alsoAddSymmetricPair[k] = 0;

      if (Azimuth[k] != -181 && Elevation[k] != -91)
      {
        knownIdx++;
        hasKnownPositions[k] = 1;
        anyKnownPos = 1;
        Azimuth2[knownIdx] = Azimuth[k];
        Elevation2[knownIdx] = Elevation[k];
      }
      if (LSdist[k] == -1) 
      {
        hasLoudspeakerDistance = 0;
      }
      if (LScalibGain[k] == -33.0f)
      {
        hasCalibrationGain = 0;
      }
      if (cicpSpIdx[k] >= 0)
      {
        anyCICPspeaker = 1;
      }
      if (cicpSpIdx[k] < 0)
      {
        allCICPspeaker = 0;
      }
    }

    if (hasLoudspeakerDistance == 0)
    {
      free(LSdist); LSdist = NULL;
    }
    if (hasCalibrationGain == 0)
    {
      free(LScalibGain); LScalibGain = NULL;
    }
    if (anyCICPspeaker == 0)
    {
      free(cicpSpIdx); cicpSpIdx = NULL;
    }
    if (anyKnownPos == 0)
    {
      free(hasKnownPositions); hasKnownPositions = NULL;
      free(Azimuth); Azimuth = NULL;
      free(Elevation); Elevation = NULL;

      free(Azimuth2); Azimuth2 = NULL;
      free(Elevation2); Elevation2 = NULL;
    }

    error = LS_setLoudspeakerRenderingConfig(h_LocalSetupHandle,externalDistComp, numChannels, LSdist,LScalibGain, 0);
    
    numExplicitelySignaledSpeakers = numChannels;
    if (allCICPspeaker == 1)
    {
      error = LS_setFixedLoudspeakerLayout(h_LocalSetupHandle,-1,cicpSpIdx,hasKnownPositions,Azimuth2,Elevation2);
    }
    else
    {
      error = LS_setFlexibleLoudspeakerLayout(h_LocalSetupHandle,cicpSpIdx,Azimuth2,Elevation2,isLFE,alsoAddSymmetricPair,numExplicitelySignaledSpeakers);
    }

    if (hasLoudspeakerDistance == 0)
    {
      *LSdistance = NULL;
    }
    else
    {
      *LSdistance = LSdist;
    }

    if (hasCalibrationGain == 0)
    {
      *LscalibGain = NULL;
    }
    else
    {
      *LscalibGain = LScalibGain;
    }
    

    if (anyKnownPos == 1)
    {
      free(hasKnownPositions); hasKnownPositions = NULL;
      free(Azimuth); Azimuth = NULL;
      free(Elevation); Elevation = NULL;

      free(Azimuth2); Azimuth2 = NULL;
      free(Elevation2); Elevation2 = NULL;
    }
    free(isLFE); isLFE = NULL;
    if (cicpSpIdx != NULL) {free(cicpSpIdx); cicpSpIdx = NULL;}
    free(alsoAddSymmetricPair); alsoAddSymmetricPair = NULL;
  
  }


  return error;
}

int LS_getGeometryFromFile(char *filename, CHANNEL_GEOMETRY AzElLfe[LS_MAX_LOUDSPEAKERS], int *numChannels, int *extDistComp, int *CICPLayoutIdx, const int verboseLvl)
{
  /**
  *   Reading the loudspeaker geometry from file
  *   @param    filename      : path+filename of the dedicated file
  *   @param    AzElLfe       : channel geometry struct
  *   @param    numChannels   : output parameter, number of channels
  *   @param    extDistComp   : output parameter, flag to signal if  external distance compensation is applied 
  *   @param    CICPLayoutIdx : output parameter, CICPindex of loudspeaker layout (returns - 1 if not a CICPlayout)
  *   @param    verboseLvl    : input parameter,  verbose level
  *
  *   @return   ERROR_CODE
  */

  FILE *fileHandle;
  char line[512] = {0};
  int i, ch;
  int error = 0;
  int CicpLayoutIdx = 0;
  int k = 0;
  int chNo = -1;

  if ( AzElLfe == NULL ) {
    if (verboseLvl >= 1) {
      fprintf( stderr, "Bad pointer in LS_getGeometryFromFile()!\n");
    }
    return -1;
  }

  /* open file */
  fileHandle = fopen(filename, "r");    
  if ( !fileHandle )
  {
    if (verboseLvl >= 1) {
      fprintf(stderr,"Error: Unable to open loudspeaker geometry file (%s).\n",filename);        
    }
    return -1;
  } 
  else 
  {
    if (verboseLvl >= 2) {
      fprintf(stdout, "\nInfo: Found loudspeaker geometry file: %s.\n", filename );
    }
  }

  /* Init channel counters */
  *numChannels = 0;

  /* Get new line */
  if (verboseLvl >= 2) {
    fprintf(stdout, "Info: loudspeaker geometry file ... \n");    
  }
  ch = 0;
  while ( fgets(line, 511, fileHandle) != NULL )
  {
    char* pChar = line;
    i = 0;
      
    /* Add newline at end of line (for eof line), terminate string after newline */
    line[strlen(line)+1] = '\0';
    line[strlen(line)] = '\n';      
    
    /* Replace all white spaces with new lines for easier parsing */
    while ( pChar[i] != '\0') {

      if ( pChar[i] == ' ' || pChar[i] == '\t') {
        pChar[i] = '\n';            
      }

      i++;
    }
                
    /* Parse line */        
    pChar = line;
    while ( (*pChar) != '\0') {

      /****************************************************  
        SANITY CHECK: break when all loudspeakers are read
      *****************************************************/
      if( ( *numChannels > 0 ) && ( ch >= LS_MAX_LOUDSPEAKERS ) ) {
        if (verboseLvl >= 1) {
          fprintf(stderr,"Error: Wrong channel list file format.\n");
        }
        error = -1;
        break;
      }

      while (  (*pChar) == '\n' || (*pChar) == '\r'  )
        pChar++;

      /***********************************************  
        FIRST ENTRY OF FILE: read number of channels
      ***********************************************/
      if (*numChannels == 0) 
      {     
        int digits = 0;
        int j;

        *numChannels = atoi(pChar);

        if ( ( *numChannels < 1 ) || ( *numChannels > LS_MAX_LOUDSPEAKERS ) ) 
        { /* sanity check for reasonable number of channels */
          if (verboseLvl >= 1) {
            fprintf(stderr,"Error: Read wrong number of channels: %d. Valid range is [1 ... 32].\n",*numChannels);
          }
          *numChannels = LS_MAX_LOUDSPEAKERS;
          error = -1;
        }
        else 
        {
          if (verboseLvl >= 2) {
            fprintf(stdout,"Reading geometry for %d channels.\n",*numChannels);
          }
        }
        digits = max(1, (int)floor(log((float)*numChannels)));

        for (j = 0; j < digits; j++)
        {
          pChar++;
        }
        if ( *pChar != ',')
        {
          return 2;
          /* read geometric file according to cicp2geometryLib*/
        }

        while ( *(pChar) != ',' )
            pChar++;

        /* jump over semicolon */
        pChar++;

        while ( (*pChar) == '\n' || (*pChar) == '\r' )
          pChar++;

        CicpLayoutIdx = atoi(pChar);
        if (CicpLayoutIdx == 0)
        {
          CicpLayoutIdx = -1;
        }
        *CICPLayoutIdx = CicpLayoutIdx;

        while ( *(pChar) != ',' )
            pChar++;

        /* jump over semicolon */
        pChar++;

        while ( (*pChar) == '\n' || (*pChar) == '\r' )
          pChar++;

        *extDistComp = atoi(pChar);

        for (k = 0; k < *numChannels; k++)
        {
              AzElLfe[k].cicpLoudspeakerIndex = -1;
              AzElLfe[k].Az = -181;
              AzElLfe[k].El = -90;
              AzElLfe[k].LFE = -1;
              AzElLfe[k].dist = -1;
              AzElLfe[k].calibGain = -33.0f;
        }
      } 

      else 
      { /* read channel definition */

        if (CicpLayoutIdx == -1)
        {

          if( (*pChar) == 'c' )
          { 

            int CicpChannelIndex = -1;

            pChar++; pChar++;

            while ( (*pChar) == '\n' || (*pChar) == '\r' )
              pChar++;

            CicpChannelIndex = atoi(pChar);

            if( (*pChar) != '\n' && (*pChar) != '\r' )
            {

              AzElLfe[ch].cicpLoudspeakerIndex = CicpChannelIndex;
              AzElLfe[ch].Az = -181;
              AzElLfe[ch].El = -90;
              AzElLfe[ch].LFE = -1;
              AzElLfe[ch].dist = -1;
              AzElLfe[ch].calibGain = -33.0f;

              while (  (*pChar) == '\n' || (*pChar) == '\r'  )
                pChar++;

              while ( *(pChar) != ',' )
                pChar++;

              /* jump over semicolon */
              pChar++;

              /* skip new lines and carriage return */
              while (  (*pChar) == '\n' || (*pChar) == '\r'  )
                pChar++;

              if (*(pChar) != ',')
              {
                AzElLfe[ch].Az = atoi(pChar);
              }

              while ( *(pChar) != ',' )
                    pChar++;

              /* jump over semicolon */
              pChar++;

              /* skip new lines and carriage return */
              while (  (*pChar) == '\n' || (*pChar) == '\r'  )
                pChar++;


              /* known elevation */
              if (*(pChar) != ',')
              {
                AzElLfe[ch].El = atoi(pChar);
              }

              while ( *(pChar) != ',' )
                    pChar++;

              /* jump over semicolon */
              pChar++;

              /* skip new lines and carriage return */
              while (  (*pChar) == '\n' || (*pChar) == '\r'  )
                pChar++;

              if (*(pChar) != ',')
              {
                AzElLfe[ch].dist = atoi(pChar);
              }

              while ( *(pChar) != ',' )
                    pChar++;

              /* jump over semicolon */
              pChar++;

              {
                char *temp = pChar;
                temp++;
                if ((*temp) != '\n')
                {
                  while (  (*pChar) == '\n' || (*pChar) == '\r'  )
                    pChar++;
                  AzElLfe[ch].calibGain = (float)atof(pChar);
                }
              }
            }
          }
          else if ( (*pChar) == 'g')
          {
            pChar++;

            if( (*pChar) != '\n' && (*pChar) != '\r' )
            {

              AzElLfe[ch].cicpLoudspeakerIndex = -1;
              AzElLfe[ch].Az = -181;
              AzElLfe[ch].El = -90;
              AzElLfe[ch].LFE = -1;
              AzElLfe[ch].dist = -1;
              AzElLfe[ch].calibGain = -33.0f;

              while (  (*pChar) == '\n' || (*pChar) == '\r'  )
                pChar++;

              while ( *(pChar) != ',' )
                pChar++;

              /* jump over semicolon */
              pChar++;

              /* skip new lines and carriage return */
              while (  (*pChar) == '\n' || (*pChar) == '\r'  )
                pChar++;

              if (*(pChar) != ',')
              {
                AzElLfe[ch].Az = atoi(pChar);
              }

              while ( *(pChar) != ',' )
                    pChar++;

              /* jump over semicolon */
              pChar++;

              /* skip new lines and carriage return */
              while (  (*pChar) == '\n' || (*pChar) == '\r'  )
                pChar++;


              /* known elevation */
              if (*(pChar) != ',')
              {
                AzElLfe[ch].El = atoi(pChar);
              }

              while ( *(pChar) != ',' )
                    pChar++;

              /* jump over semicolon */
              pChar++;

              /* skip new lines and carriage return */
              while (  (*pChar) == '\n' || (*pChar) == '\r'  )
                pChar++;

              AzElLfe[ch].LFE = atoi(pChar);

              while ( *(pChar) != ',' )
                    pChar++;

              /* jump over semicolon */
              pChar++;

              /* skip new lines and carriage return */
              while (  (*pChar) == '\n' || (*pChar) == '\r'  )
                pChar++;

              if (*(pChar) != ',')
              {
                AzElLfe[ch].dist = atoi(pChar);
              }

              while ( *(pChar) != ',' )
                    pChar++;

              /* jump over semicolon */
              pChar++;

              {
                char *temp = pChar;
                temp++;
                if ((*temp) != '\n')
                {
                  while (  (*pChar) == '\n' || (*pChar) == '\r'  )
                    pChar++;
                  AzElLfe[ch].calibGain = (float)atof(pChar);
                }
              }
            }
          }
          else
          {
            /*************************************
            INVALID CICP / GEOMETRY FILE SYNTAX
            *************************************/         
            if (verboseLvl >= 1) {
              fprintf(stderr,"Error: Wrong channel list file format.\n");
            }
            break;
            error = -1;
          }
        }
        else 
        {

          if( (*pChar) != '\n' && (*pChar) != '\r' )
          {
            chNo++;

            AzElLfe[chNo].cicpLoudspeakerIndex = -1;
            AzElLfe[chNo].Az = -181;
            AzElLfe[chNo].El = -90;
            AzElLfe[chNo].LFE = -1;
            AzElLfe[chNo].dist = -1;
            AzElLfe[chNo].calibGain = -33.0f;

            while (  (*pChar) == '\n' || (*pChar) == '\r'  )
              pChar++;

            /* skip new lines and carriage return */
            while (  (*pChar) == '\n' || (*pChar) == '\r'  )
              pChar++;

            if (*(pChar) != ',')
            {
              AzElLfe[chNo].Az = atoi(pChar);
            }

            while ( *(pChar) != ',' )
                  pChar++;

            /* jump over semicolon */
            pChar++;

            /* skip new lines and carriage return */
            while (  (*pChar) == '\n' || (*pChar) == '\r'  )
              pChar++;


            /* known elevation */
            if (*(pChar) != ',')
            {
              AzElLfe[chNo].El = atoi(pChar);
            }

            while ( *(pChar) != ',' )
                  pChar++;

            /* jump over semicolon */
            pChar++;

            /* skip new lines and carriage return */
            while (  (*pChar) == '\n' || (*pChar) == '\r'  )
              pChar++;

            if (*(pChar) != ',')
            {
              AzElLfe[chNo].dist = atoi(pChar);
            }

            while ( *(pChar) != ',' )
                      pChar++;

            /* jump over semicolon */
            pChar++;
            {
              char *temp = pChar;
              temp++;
              if ((*temp) != '\n')
              {
                while (  (*pChar) == '\n' || (*pChar) == '\r'  )
                  pChar++;
                AzElLfe[chNo].calibGain = (float)atof(pChar);
              }
            }
          }
        }
        ++ch;
      }

      /* Jump over parsed value */
      while (  (*pChar) != '\n' )
        pChar++;            

      /* Jump over new lines */
      while (  (*pChar) == '\n' || (*pChar) == '\r'  )
        pChar++;
    }
  }
 
  return  error;
}

int LS_getAzimuthFromCICPspeakerIdx(int CICPspeakerIdx)
{
  /**
	*   Get azimuth angle of a CICPspeakerIdx
	*   @param    CICPspeakerIdx  : CICPspeakerIdx of the wanted speaker
	*
	*   @return   azimuthAngle  
	*/

  int cicpspeaker_az[43] = { 30, -30, 0, 0, 110, -110, 22, -22, 135, -135, 180, -1, -1, 90, -90, 60, -60, 30, -30, 0, 135, -135, 180, 90, -90, 0, 45, 45, -45, 0, 110, -110, 45, -45, 45, -45, -45, 60, -60, 30, -30, 150, -150 };
  if ((CICPspeakerIdx < 0) || (CICPspeakerIdx > 42) || (CICPspeakerIdx == 3))
  {
    return -1;
  }
  else
  {
    return cicpspeaker_az[CICPspeakerIdx];
  }
}

int LS_writeLocalSetupBitstream(LOCAL_SETUP_HANDLE h_LocalSetupHandle, H_LS_BITSTREAM_ENCODER h_bitstream, unsigned long *noBitsWritten, char * filename)
{
  /**
	*   Writing of the local setup data (mpeg3daLocalSetupInformation() to the bitstream)
	*   @param    h_LocalSetupHandle  : local setup handle
	*   @param    h_bitstream		      : bitstream handle
  *   @param    noBitsWritten		    : output parameter, number of written bits
	*
	*   @return   ERROR_CODE
	*/
  unsigned long noBitsWritten2 = 0;
  unsigned long noBitsWrittenTotal = 0;
  int error = 0;
  int k = 0;

  if (h_LocalSetupHandle->rendering_type == 0)
  {  
    noBitsWrittenTotal += LS_DecInt_writeBits(h_bitstream,(unsigned int)h_LocalSetupHandle->rendering_type, 1);
    /* quantize Loudspeaker setup data */
    error = LS_quantize_LSRendering_Data(h_LocalSetupHandle);
    /* write data */
    error = LS_writeBits_LSRendering(h_LocalSetupHandle,h_bitstream, &noBitsWritten2,&noBitsWrittenTotal);
    
    noBitsWrittenTotal += LS_DecInt_writeBits(h_bitstream,(unsigned int)h_LocalSetupHandle->numWireOutputs, 16);
    if (h_LocalSetupHandle->numWireOutputs > 0)
    {
      for (k = 0; k < h_LocalSetupHandle->numWireOutputs; k++)
      {
        noBitsWrittenTotal += LS_DecInt_writeBits(h_bitstream,(unsigned int)h_LocalSetupHandle->WireIDs[k], 16);
      }
    }
    if (h_LocalSetupHandle->LocalScreenInfo->hasLocalScreenInfo == 1)
    {
      error = LS_quantize_LocalScreen_Data(h_LocalSetupHandle);
      noBitsWrittenTotal += LS_DecInt_writeBits(h_bitstream,1,1);

      if (fabs(h_LocalSetupHandle->LocalScreenInfo->az_left) == fabs(h_LocalSetupHandle->LocalScreenInfo->az_right))
      {
        noBitsWrittenTotal += LS_DecInt_writeBits(h_bitstream,1,1);
        noBitsWrittenTotal += LS_DecInt_writeBits(h_bitstream,(unsigned int)h_LocalSetupHandle->LocalScreenInfoQuant->az_left,9);

      }
      else
      {
        noBitsWrittenTotal += LS_DecInt_writeBits(h_bitstream,0,1);
        noBitsWrittenTotal += LS_DecInt_writeBits(h_bitstream,(unsigned int)h_LocalSetupHandle->LocalScreenInfoQuant->az_left,10);
        noBitsWrittenTotal += LS_DecInt_writeBits(h_bitstream,(unsigned int)h_LocalSetupHandle->LocalScreenInfoQuant->az_right,10);
      }
      noBitsWrittenTotal += LS_DecInt_writeBits(h_bitstream,h_LocalSetupHandle->LocalScreenInfo->hasElevationInfo,1);
      if (h_LocalSetupHandle->LocalScreenInfo->hasElevationInfo == 1)
      {
        noBitsWrittenTotal += LS_DecInt_writeBits(h_bitstream,(unsigned int)h_LocalSetupHandle->LocalScreenInfoQuant->el_top,9);
        noBitsWrittenTotal += LS_DecInt_writeBits(h_bitstream,(unsigned int)h_LocalSetupHandle->LocalScreenInfoQuant->el_bottom,9);
      }
    }
    else
    {
      noBitsWrittenTotal += LS_DecInt_writeBits(h_bitstream,0,1);
    }
  }
  else if (h_LocalSetupHandle->rendering_type == 1)
  {
    error = LS_writeBits_BinauralRendering(h_LocalSetupHandle, h_bitstream, &noBitsWritten2,&noBitsWrittenTotal, filename);
  }
  else
  {
    error = -1;
    return error;
  }

  *noBitsWritten = noBitsWrittenTotal;
  h_LocalSetupHandle->noBits = noBitsWrittenTotal;

  return error;
}

int LS_getSymmetricCICPspeakerIdx(int CICPspeakerIdx)
{
  /**
	*   Get the CICPspeakerIdx of a speaker that is symmetric (same elevation, same absolute azimuth) to a given speaker
	*   @param    CICPspeakerIdx  : index of the given speaker
	*
	*   @return   CICPspeakerIdx of the symmetric speaker (if any, else the function returns -1)
	*/
  int tempCICP;
  int checkCICP;

  if ((LS_getAzimuthFromCICPspeakerIdx(CICPspeakerIdx) == 0) ||(LS_getAzimuthFromCICPspeakerIdx(CICPspeakerIdx) == 180))
  {
    return -1;
  }
  else
  {
    if (CICPspeakerIdx == 26)
    {
      return 36;
    }
    if (CICPspeakerIdx == 36)
    {
      return 26;
    }

    if (LS_getAzimuthFromCICPspeakerIdx(CICPspeakerIdx) > 0)
    {
      tempCICP = CICPspeakerIdx + 1;
    }
    else
    {
      tempCICP = CICPspeakerIdx - 1;
    }
    checkCICP = (abs(LS_getAzimuthFromCICPspeakerIdx(CICPspeakerIdx)) == abs(LS_getAzimuthFromCICPspeakerIdx(tempCICP)));
    if (checkCICP == 1)
    {
      return tempCICP;
    }
    else
    {
      return -1;
    }
  }
}

int LS_invquantizeLoudspeakerRenderingData(LOCAL_SETUP_HANDLE LocalSetup)
{
  /**
	*   Inverse quantization of the loudspeaker rendering data read from the bitstream
	*   @param    LocalSetup  : local setup handle
	*
	*   @return   ERROR_CODE
	*/
  float tempfloat;
  int tempint; 

  int k = 0;
  if (LocalSetup->LoudspeakerRenderingQuant->loudspeakerCalibrationGain != NULL)
  {
    for (k = 0; k < LocalSetup->LoudspeakerRendering->numSpeakers; k++)
    {
      tempint = LocalSetup->LoudspeakerRenderingQuant->loudspeakerCalibrationGain[k];
      tempfloat = (float)(tempint - 64)*0.5f;
      tempfloat = max(min(tempfloat,31.5f),-32.0f);
      LocalSetup->LoudspeakerRendering->loudspeakerCalibrationGain[k] = tempfloat;
    }
  }
  if ((LocalSetup->LoudspeakerRenderingQuant->knownAzimuth != NULL) && (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType <= 1))
  {
    for (k = 0; k < LocalSetup->LoudspeakerRendering->numSpeakers; k++)
    {
      if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[0][k] == 1)
      {
        tempint = LocalSetup->LoudspeakerRenderingQuant->knownAzimuth[k];
        tempfloat = (float)(tempint - 256);
        tempfloat = max(min(tempfloat,180.0f),-180.0f);
        LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[1][k] = tempfloat;
      }
    }
  }
  if ((LocalSetup->LoudspeakerRenderingQuant->knownElevation != NULL) && (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType <= 1))
  {
    for (k = 0; k < LocalSetup->LoudspeakerRendering->numSpeakers; k++)
    {
      if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[0][k] == 1)
      {
        tempint = LocalSetup->LoudspeakerRenderingQuant->knownElevation[k];
        tempfloat = (float)(tempint - 128);
        tempfloat = max(min(tempfloat,90.0f),-90.0f);
        LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[2][k] = tempfloat;
      }
    }
  }

  if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType == 2)
  {
    for (k = 0; k < LocalSetup->LoudspeakerRendering->numSpeakers; k++)
    {
      if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthDirection[k] == 0)
      {
        LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Azimuth[k] = (float)(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx[k] * LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->angularPrecision);
      }
      else if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthDirection[k] == 1)
      {
        LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Azimuth[k] = LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx[k] * LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->angularPrecision * -1.0f;;
      }

      if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationDirection[k] == 0)
      {
        LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Elevation[k] = (float)(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationAngleIdx[k] * LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->angularPrecision);
      }
      else if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationDirection[k] == 1)
      {
        LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Elevation[k] = LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationAngleIdx[k] * LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->angularPrecision * -1.0f;;
      }
    }
  }

  return 0;
}

int LS_invquantizeLocalScreenData(LOCAL_SETUP_HANDLE LocalSetup, int centeredInAz)
{
  /**
	*   Inverse quantization of the local screen size data data read from the bitstream
	*   @param    LocalSetup    : local setup handle
  *   @param    centeredInAz  : flag to signal if the local screen is centered in azimuth
	*
	*   @return   ERROR_CODE
	*/
  float tempfloat;
  int tempint;

  if (LocalSetup->LocalScreenInfo->hasLocalScreenInfo == 1)
  {
    if (centeredInAz)
    {
      float tempfloat2;
        tempint = LocalSetup->LocalScreenInfoQuant->az_left;
        tempfloat2 = (float)(tempint)*0.5f;
        tempfloat2 = max(min(tempfloat2,180.0f),0.0f);
        LocalSetup->LocalScreenInfo->az_left = tempfloat2;

        tempfloat = (float)(tempint)*0.5f*-1.0f;
        tempfloat = max(min(tempfloat,0.0f),-180.0f);
        LocalSetup->LocalScreenInfo->az_right = tempfloat;
    }
    else
    {
        tempint = LocalSetup->LocalScreenInfoQuant->az_left;
        tempfloat = (float)(tempint - 511)*0.5f;
        tempfloat = max(min(tempfloat,180.0f),-180.0f);
        LocalSetup->LocalScreenInfo->az_left = tempfloat;

        tempint = LocalSetup->LocalScreenInfoQuant->az_right;
        tempfloat = (float)(tempint - 511)*0.5f;
        tempfloat = max(min(tempfloat,180.0f),-180.0f);
        LocalSetup->LocalScreenInfo->az_right = tempfloat;
    }
    if (LocalSetup->LocalScreenInfo->hasElevationInfo == 1)
    {
      tempint = LocalSetup->LocalScreenInfoQuant->el_top;
      tempfloat = (float)(tempint - 255)*0.5f;
      tempfloat = max(min(tempfloat,90.0f),-90.0f);
      LocalSetup->LocalScreenInfo->el_top = tempfloat;

      tempint = LocalSetup->LocalScreenInfoQuant->el_bottom;
      tempfloat = (float)(tempint - 255)*0.5f;
      tempfloat = max(min(tempfloat,90.0f),-90.0f);
      LocalSetup->LocalScreenInfo->el_bottom = tempfloat;
    }
  }

  return 0;
}

int LS_readLocalSetupBitstream(LOCAL_SETUP_HANDLE LocalSetup, H_LS_BITSTREAM_DECODER h_bitstream, unsigned long *no_BitsRead)
{
  /**
	*   Reading the local setup data (mpegh3daLocalSetupInformation() syntax) from the bitstream
	*   @param    LocalSetup    : local setup handle
	*   @param    h_bitstream	  : bitstream handle
  *   @param    no_BitsRead		: output parameter, number of read bits
	*
	*   @return   ERROR_CODE
	*/
  unsigned long noBitsRead = 0;
  unsigned long noBitsReadTotal = 0;
  int k = 0;
  int error = 0;
  int centeredInAz = 0;
  unsigned int temp;

  unsigned char *bitbuf;
	long	nBytes, nBits;
	robitbuf readbitBuffer;
	robitbufHandle hBitstream = &readbitBuffer;

  fseek(h_bitstream->bsFile, 0L, SEEK_SET); /* rewind */
  fseek(h_bitstream->bsFile, 0L, SEEK_END);
  nBytes = ftell(h_bitstream->bsFile);
  nBits = 8 * nBytes;
  fseek(h_bitstream->bsFile, 0L, SEEK_SET); /* rewind */
  bitbuf = (unsigned char*)malloc(nBytes * sizeof(unsigned char));
  fread(bitbuf, 1, nBytes, h_bitstream->bsFile);
  robitbuf_Init(hBitstream, bitbuf, nBits, 0);
  fseek(h_bitstream->bsFile, 0L, SEEK_SET); /* rewind */


  /* flush data structure */
  if (LocalSetup->WireIDs != NULL)
  {
    free(LocalSetup->WireIDs); LocalSetup->WireIDs = NULL;
  }

  noBitsReadTotal += LS_DecInt_readBits(h_bitstream,&temp,1);
  LocalSetup->rendering_type = temp;

  if (LocalSetup->rendering_type == 0)
  {
    /* loudspeaker rendering */
    error = LS_readLoudspeakerRenderingBits(LocalSetup, h_bitstream, &noBitsRead, &noBitsReadTotal);
    error = LS_invquantizeLoudspeakerRenderingData(LocalSetup);
    noBitsReadTotal += LS_DecInt_readBits(h_bitstream,&temp,16);
    LocalSetup->numWireOutputs = (int)temp;
    if (LocalSetup->numWireOutputs > 0)
    {
      LocalSetup->WireIDs = (int*)calloc(LocalSetup->numWireOutputs, sizeof(int));
      for (k = 0; k < LocalSetup->numWireOutputs; k++)
      {
        noBitsReadTotal += LS_DecInt_readBits(h_bitstream,&temp,16);
        LocalSetup->WireIDs[k] = (int)temp;
      }
    }
    noBitsReadTotal += LS_DecInt_readBits(h_bitstream, &temp,1);
    LocalSetup->LocalScreenInfo->hasLocalScreenInfo = (int)temp;
    if (LocalSetup->LocalScreenInfo->hasLocalScreenInfo == 1)
    {
      unsigned int temp2;
      /* isCenteredInAzimuth */
      noBitsReadTotal += LS_DecInt_readBits(h_bitstream,&temp2,1);
      if (temp2 == 1)
      {
        noBitsReadTotal += LS_DecInt_readBits(h_bitstream, &temp,9);
        LocalSetup->LocalScreenInfoQuant->az_left = (int)temp;
        centeredInAz = 1;
      }
      else
      {
        noBitsReadTotal += LS_DecInt_readBits(h_bitstream, &temp,10);
        LocalSetup->LocalScreenInfoQuant->az_left = (int)temp;
        noBitsReadTotal += LS_DecInt_readBits(h_bitstream, &temp,10);
        LocalSetup->LocalScreenInfoQuant->az_right = (int)temp;
      }
      noBitsReadTotal += LS_DecInt_readBits(h_bitstream,&temp,1);
      LocalSetup->LocalScreenInfo->hasElevationInfo = (int)temp;
      if (LocalSetup->LocalScreenInfo->hasElevationInfo == 1)
      {
        noBitsReadTotal += LS_DecInt_readBits(h_bitstream, &temp,9);
        LocalSetup->LocalScreenInfoQuant->el_top = (int)temp;
        noBitsReadTotal += LS_DecInt_readBits(h_bitstream, &temp,9);
        LocalSetup->LocalScreenInfoQuant->el_bottom = (int)temp;
      }
      error = LS_invquantizeLocalScreenData(LocalSetup, centeredInAz);
    }

    LocalSetup->noBits = noBitsReadTotal;
    *no_BitsRead = noBitsReadTotal;
  }
  else
  {
    error = LS_readBinauralRenderingBits(LocalSetup, hBitstream, &noBitsRead, &noBitsReadTotal);

    LocalSetup->numWireOutputs = (int)robitbuf_ReadBits(hBitstream,16);
    if (LocalSetup->numWireOutputs > 0)
    {
      LocalSetup->WireIDs = (int*)calloc(LocalSetup->numWireOutputs, sizeof(int));
      for (k = 0; k < LocalSetup->numWireOutputs; k++)
      {
        LocalSetup->WireIDs[k] = (int)robitbuf_ReadBits(hBitstream,16);
      }
    }
    LocalSetup->LocalScreenInfo->hasLocalScreenInfo = (int)robitbuf_ReadBits(hBitstream,1);
    if (LocalSetup->LocalScreenInfo->hasLocalScreenInfo == 1)
    {
      unsigned int temp2;
      /* isCenteredInAzimuth */
      temp2 = robitbuf_ReadBits(hBitstream,1);
      if (temp2 == 1)
      {
        LocalSetup->LocalScreenInfoQuant->az_left = (int)robitbuf_ReadBits(hBitstream,9);
        centeredInAz = 1;
      }
      else
      {
        LocalSetup->LocalScreenInfoQuant->az_left = (int)robitbuf_ReadBits(hBitstream,10);
        LocalSetup->LocalScreenInfoQuant->az_right = (int)robitbuf_ReadBits(hBitstream,10);
      }
      LocalSetup->LocalScreenInfo->hasElevationInfo = (int)robitbuf_ReadBits(hBitstream,1);
      if (LocalSetup->LocalScreenInfo->hasElevationInfo == 1)
      {
        LocalSetup->LocalScreenInfoQuant->el_top = (int)robitbuf_ReadBits(hBitstream,9);
        LocalSetup->LocalScreenInfoQuant->el_bottom = (int)robitbuf_ReadBits(hBitstream,9);
      }
      error = LS_invquantizeLocalScreenData(LocalSetup, centeredInAz);
    }
    noBitsReadTotal = hBitstream->bitsRead;
    LocalSetup->noBits = noBitsReadTotal;
    *no_BitsRead = noBitsReadTotal;
  }
  
  free(bitbuf);
  return error;
}

int getBinauralFirData(BinauralFirData *pBinauralFirData, short RepresentationIndex, BinauralRendering *pBR)
{
  int i, j;
  int nBrirPairs;

  memcpy(pBinauralFirData,pBR->ppBinauralRepresentation[RepresentationIndex]->pBinauralFirData, sizeof(BinauralFirData));

  nBrirPairs = pBR->ppBinauralRepresentation[RepresentationIndex]->nBrirPairs;

  pBinauralFirData->Ntaps = pBR->ppBinauralRepresentation[RepresentationIndex]->pBinauralFirData->Ntaps;
   
	if (pBinauralFirData->Ntaps > MAX_BRIR_SIZE)
	{
		fprintf(stderr,"\nError : BRIR size should be <= %d", MAX_BRIR_SIZE);
		return -1;
	}

  for (i=0; i<nBrirPairs; i++)
		{
	  for(j=0; j<pBinauralFirData->Ntaps; j++)
	  {
      pBinauralFirData->pppTaps[0][i][j] = pBR->ppBinauralRepresentation[RepresentationIndex]->pBinauralFirData->pppTaps[0][i][j];
		  pBinauralFirData->pppTaps[1][i][j] = pBR->ppBinauralRepresentation[RepresentationIndex]->pBinauralFirData->pppTaps[1][i][j];
	  }
  }

  pBinauralFirData->allCutFreq = pBR->ppBinauralRepresentation[RepresentationIndex]->pBinauralFirData->allCutFreq; /* bsAllCutFreq */

	if (pBinauralFirData->allCutFreq == 0)
	{
		for (i=0; i<nBrirPairs; i++)
		{
			pBinauralFirData->ppCutFreq[0][i] = pBR->ppBinauralRepresentation[RepresentationIndex]->pBinauralFirData->ppCutFreq[0][i];
      pBinauralFirData->ppCutFreq[1][i] = pBR->ppBinauralRepresentation[RepresentationIndex]->pBinauralFirData->ppCutFreq[1][i];
		}
	}
	else
	{
		for (i=0; i<nBrirPairs; i++)
		{
			pBinauralFirData->ppCutFreq[0][i] = (float)pBR->ppBinauralRepresentation[RepresentationIndex]->pBinauralFirData->allCutFreq;
			pBinauralFirData->ppCutFreq[1][i] = (float)pBR->ppBinauralRepresentation[RepresentationIndex]->pBinauralFirData->allCutFreq;
		}
	}
	return 0;
}

int getFdBinauralRendererParam(FdBinauralRendererParam *pFdBinauralRendererParam, short RepresentationIndex, BinauralRendering *pBR)
{
  unsigned int k, b, nr, v;
  unsigned int fftlength;
  unsigned int nBrirPairs;


  pFdBinauralRendererParam->flagHRIR = pBR->ppBinauralRepresentation[RepresentationIndex]->pFdBinauralRendererParam->flagHRIR;
	pFdBinauralRendererParam->dInit	   = pBR->ppBinauralRepresentation[RepresentationIndex]->pFdBinauralRendererParam->dInit;
	pFdBinauralRendererParam->kMax     = pBR->ppBinauralRepresentation[RepresentationIndex]->pFdBinauralRendererParam->kMax;
	pFdBinauralRendererParam->kConv    = pBR->ppBinauralRepresentation[RepresentationIndex]->pFdBinauralRendererParam->kConv;
	pFdBinauralRendererParam->kAna     = pBR->ppBinauralRepresentation[RepresentationIndex]->pFdBinauralRendererParam->kAna;

  nBrirPairs = pBR->ppBinauralRepresentation[RepresentationIndex]->nBrirPairs;

	/***************************** VoffBrirParam *****************************/

	/* Memory Allocation */
	pFdBinauralRendererParam->nFilter = (unsigned int*)calloc(pFdBinauralRendererParam->kMax, sizeof(unsigned int));
	pFdBinauralRendererParam->nFft = (unsigned int*)calloc(pFdBinauralRendererParam->kMax, sizeof(unsigned int));
	pFdBinauralRendererParam->nBlk = (unsigned int*)calloc(pFdBinauralRendererParam->kMax, sizeof(unsigned int));

	pFdBinauralRendererParam->ppppVoffCoeffLeftReal = (float****)calloc(pFdBinauralRendererParam->kMax, sizeof(float***));
	pFdBinauralRendererParam->ppppVoffCoeffLeftImag = (float****)calloc(pFdBinauralRendererParam->kMax, sizeof(float***));
	pFdBinauralRendererParam->ppppVoffCoeffRightReal = (float****)calloc(pFdBinauralRendererParam->kMax, sizeof(float***));
	pFdBinauralRendererParam->ppppVoffCoeffRightImag = (float****)calloc(pFdBinauralRendererParam->kMax, sizeof(float***));

	for(k=0; k<pFdBinauralRendererParam->kMax; k++)
	{
		pFdBinauralRendererParam->nFilter[k] = pBR->ppBinauralRepresentation[RepresentationIndex]->pFdBinauralRendererParam->nFilter[k];
		pFdBinauralRendererParam->nFft[k] = pBR->ppBinauralRepresentation[RepresentationIndex]->pFdBinauralRendererParam->nFft[k];
		pFdBinauralRendererParam->nBlk[k] = pBR->ppBinauralRepresentation[RepresentationIndex]->pFdBinauralRendererParam->nBlk[k];

		/* Memory Allocation */
		pFdBinauralRendererParam->ppppVoffCoeffLeftReal[k] = (float***)calloc(pFdBinauralRendererParam->nBlk[k], sizeof(float**));
		pFdBinauralRendererParam->ppppVoffCoeffLeftImag[k] = (float***)calloc(pFdBinauralRendererParam->nBlk[k], sizeof(float**));
		pFdBinauralRendererParam->ppppVoffCoeffRightReal[k] = (float***)calloc(pFdBinauralRendererParam->nBlk[k], sizeof(float**));
		pFdBinauralRendererParam->ppppVoffCoeffRightImag[k] = (float***)calloc(pFdBinauralRendererParam->nBlk[k], sizeof(float**));
		fftlength = (int)(pow(2.0f, pFdBinauralRendererParam->nFft[k]));
		for( b=0; b<pFdBinauralRendererParam->nBlk[k]; b++)
		{		
			/* Memory Allocation */
			pFdBinauralRendererParam->ppppVoffCoeffLeftReal[k][b] = (float**)calloc(nBrirPairs, sizeof(float*));
			pFdBinauralRendererParam->ppppVoffCoeffLeftImag[k][b] = (float**)calloc(nBrirPairs, sizeof(float*)); 
			pFdBinauralRendererParam->ppppVoffCoeffRightReal[k][b] = (float**)calloc(nBrirPairs, sizeof(float*));
			pFdBinauralRendererParam->ppppVoffCoeffRightImag[k][b] = (float**)calloc(nBrirPairs, sizeof(float*));
			for( nr=0; nr < nBrirPairs; nr++)
			{
				/* Memory Allocation */
				pFdBinauralRendererParam->ppppVoffCoeffLeftReal[k][b][nr] = (float*)calloc(fftlength, sizeof(float));
				pFdBinauralRendererParam->ppppVoffCoeffLeftImag[k][b][nr] = (float*)calloc(fftlength, sizeof(float));
				pFdBinauralRendererParam->ppppVoffCoeffRightReal[k][b][nr] = (float*)calloc(fftlength, sizeof(float));
				pFdBinauralRendererParam->ppppVoffCoeffRightImag[k][b][nr] = (float*)calloc(fftlength, sizeof(float));
				for( v=0; v< fftlength; v++)
				{
					pFdBinauralRendererParam->ppppVoffCoeffLeftReal[k][b][nr][v] = pBR->ppBinauralRepresentation[RepresentationIndex]->pFdBinauralRendererParam->ppppVoffCoeffLeftReal[k][b][nr][v];
					pFdBinauralRendererParam->ppppVoffCoeffLeftImag[k][b][nr][v] = pBR->ppBinauralRepresentation[RepresentationIndex]->pFdBinauralRendererParam->ppppVoffCoeffLeftImag[k][b][nr][v];
					pFdBinauralRendererParam->ppppVoffCoeffRightReal[k][b][nr][v] = pBR->ppBinauralRepresentation[RepresentationIndex]->pFdBinauralRendererParam->ppppVoffCoeffRightReal[k][b][nr][v];
					pFdBinauralRendererParam->ppppVoffCoeffRightImag[k][b][nr][v] = pBR->ppBinauralRepresentation[RepresentationIndex]->pFdBinauralRendererParam->ppppVoffCoeffRightImag[k][b][nr][v];
				}
			}
		}
	}

	/***************************** SfrBrirParam *****************************/
	pFdBinauralRendererParam->fcAna = (float*)calloc(pFdBinauralRendererParam->kAna, sizeof(float));
	pFdBinauralRendererParam->rt60 = (float*)calloc(pFdBinauralRendererParam->kAna, sizeof(float));
	pFdBinauralRendererParam->nrgLr = (float*)calloc(pFdBinauralRendererParam->kAna, sizeof(float));
	for(k=0; k<pFdBinauralRendererParam->kAna; k++)
	{
    pFdBinauralRendererParam->fcAna[k] = pBR->ppBinauralRepresentation[RepresentationIndex]->pFdBinauralRendererParam->fcAna[k];
		pFdBinauralRendererParam->rt60[k] = pBR->ppBinauralRepresentation[RepresentationIndex]->pFdBinauralRendererParam->rt60[k];
		pFdBinauralRendererParam->nrgLr[k] = pBR->ppBinauralRepresentation[RepresentationIndex]->pFdBinauralRendererParam->nrgLr[k];
	}
	
	/***************************** QtdlBrirParam *****************************/
	/* Memory Allocation */
	pFdBinauralRendererParam->ppQtdlGainLeftReal = (float**)calloc(pFdBinauralRendererParam->kMax-pFdBinauralRendererParam->kConv, sizeof(float*));
	pFdBinauralRendererParam->ppQtdlGainLeftImag = (float**)calloc(pFdBinauralRendererParam->kMax-pFdBinauralRendererParam->kConv, sizeof(float*));
	pFdBinauralRendererParam->ppQtdlGainRightReal = (float**)calloc(pFdBinauralRendererParam->kMax-pFdBinauralRendererParam->kConv, sizeof(float*));
	pFdBinauralRendererParam->ppQtdlGainRightImag = (float**)calloc(pFdBinauralRendererParam->kMax-pFdBinauralRendererParam->kConv, sizeof(float*));
	pFdBinauralRendererParam->ppQtdlLagLeft = (unsigned int**)calloc(pFdBinauralRendererParam->kMax-pFdBinauralRendererParam->kConv, sizeof(unsigned int*));
	pFdBinauralRendererParam->ppQtdlLagRight = (unsigned int**)calloc(pFdBinauralRendererParam->kMax-pFdBinauralRendererParam->kConv, sizeof(unsigned int*));
	for(k=0; k<pFdBinauralRendererParam->kMax-pFdBinauralRendererParam->kConv; k++)
	{
		/* Memory Allocation */
		pFdBinauralRendererParam->ppQtdlGainLeftReal[k] = (float*)calloc(nBrirPairs, sizeof(float));
		pFdBinauralRendererParam->ppQtdlGainLeftImag[k] = (float*)calloc(nBrirPairs, sizeof(float));
		pFdBinauralRendererParam->ppQtdlGainRightReal[k] = (float*)calloc(nBrirPairs, sizeof(float));
		pFdBinauralRendererParam->ppQtdlGainRightImag[k] = (float*)calloc(nBrirPairs, sizeof(float));
		pFdBinauralRendererParam->ppQtdlLagLeft[k] = (unsigned int*)calloc(nBrirPairs, sizeof(unsigned int));
		pFdBinauralRendererParam->ppQtdlLagRight[k] = (unsigned int*)calloc(nBrirPairs, sizeof(unsigned int));
		for( nr=0; nr < nBrirPairs; nr++)
		{
			pFdBinauralRendererParam->ppQtdlGainLeftReal[k][nr] = pBR->ppBinauralRepresentation[RepresentationIndex]->pFdBinauralRendererParam->ppQtdlGainLeftReal[k][nr];
			pFdBinauralRendererParam->ppQtdlGainLeftImag[k][nr] = pBR->ppBinauralRepresentation[RepresentationIndex]->pFdBinauralRendererParam->ppQtdlGainLeftImag[k][nr];
			pFdBinauralRendererParam->ppQtdlGainRightReal[k][nr] = pBR->ppBinauralRepresentation[RepresentationIndex]->pFdBinauralRendererParam->ppQtdlGainRightReal[k][nr];
			pFdBinauralRendererParam->ppQtdlGainRightImag[k][nr] = pBR->ppBinauralRepresentation[RepresentationIndex]->pFdBinauralRendererParam->ppQtdlGainRightImag[k][nr];
			pFdBinauralRendererParam->ppQtdlLagLeft[k][nr] = pBR->ppBinauralRepresentation[RepresentationIndex]->pFdBinauralRendererParam->ppQtdlLagLeft[k][nr];
			pFdBinauralRendererParam->ppQtdlLagRight[k][nr] = pBR->ppBinauralRepresentation[RepresentationIndex]->pFdBinauralRendererParam->ppQtdlLagRight[k][nr];
		}
	}
  return 0;
}

int getTdBinauralRendererParam(TdBinauralRendererParam *pTdBinauralRendererParam, short RepresentationIndex, BinauralRendering *pBR)
{
  int i, j;
  int nBrirPairs;

  nBrirPairs = pBR->ppBinauralRepresentation[RepresentationIndex]->nBrirPairs;

	pTdBinauralRendererParam->numChannel = nBrirPairs;

  pTdBinauralRendererParam->beginDelay = pBR->ppBinauralRepresentation[RepresentationIndex]->pTdBinauralRendererParam->beginDelay;

	pTdBinauralRendererParam->lenDirect = pBR->ppBinauralRepresentation[RepresentationIndex]->pTdBinauralRendererParam->lenDirect;
	if (pTdBinauralRendererParam->lenDirect > MAX_LENGTH_DIRECT_FILTER)
	{
		fprintf(stderr,"\nError : bsDirectLen should be <= %d", MAX_LENGTH_DIRECT_FILTER);
		return -1;
	}

  pTdBinauralRendererParam->numDiffuseBlock = pBR->ppBinauralRepresentation[RepresentationIndex]->pTdBinauralRendererParam->numDiffuseBlock;
	if (pTdBinauralRendererParam->numDiffuseBlock > MAX_NUM_DIFFUSE_BLOCKS)
	{
		fprintf(stderr,"\nError : bsNbDiffuseBlocks should be <= %d", MAX_NUM_DIFFUSE_BLOCKS);
		return -1;
	}

	for (i=0; i<nBrirPairs; i++)
	{
		pTdBinauralRendererParam->ppDirectFc[0][i] = pBR->ppBinauralRepresentation[RepresentationIndex]->pTdBinauralRendererParam->ppDirectFc[0][i];
    pTdBinauralRendererParam->ppDirectFc[1][i] = pBR->ppBinauralRepresentation[RepresentationIndex]->pTdBinauralRendererParam->ppDirectFc[1][i];
	}

	for (i=0; i<pTdBinauralRendererParam->numDiffuseBlock; i++)
	{
		pTdBinauralRendererParam->ppDiffuseFc[0][i] = pBR->ppBinauralRepresentation[RepresentationIndex]->pTdBinauralRendererParam->ppDiffuseFc[0][i];
		pTdBinauralRendererParam->ppDiffuseFc[1][i] = pBR->ppBinauralRepresentation[RepresentationIndex]->pTdBinauralRendererParam->ppDiffuseFc[1][i];
	}

	for (i=0; i<nBrirPairs; i++)
	{
		pTdBinauralRendererParam->pInvDiffuseWeight[i] = pBR->ppBinauralRepresentation[RepresentationIndex]->pTdBinauralRendererParam->pInvDiffuseWeight[i];
	}

	for (i=0; i<nBrirPairs; i++)
	{
		for(j=0; j<pTdBinauralRendererParam->lenDirect; j++)
		{
      pTdBinauralRendererParam->pppTaps_direct[0][i][j] = pBR->ppBinauralRepresentation[RepresentationIndex]->pTdBinauralRendererParam->pppTaps_direct[0][i][j];
      pTdBinauralRendererParam->pppTaps_direct[1][i][j] = pBR->ppBinauralRepresentation[RepresentationIndex]->pTdBinauralRendererParam->pppTaps_direct[1][i][j];
		}
	}

	for (i=0; i<pTdBinauralRendererParam->numDiffuseBlock; i++)
	{
		for(j=0; j<pTdBinauralRendererParam->lenDirect; j++)
		{
      pTdBinauralRendererParam->pppTaps_diffuse[i][0][j] = pBR->ppBinauralRepresentation[RepresentationIndex]->pTdBinauralRendererParam->pppTaps_diffuse[i][0][j];
      pTdBinauralRendererParam->pppTaps_diffuse[i][1][j] = pBR->ppBinauralRepresentation[RepresentationIndex]->pTdBinauralRendererParam->pppTaps_diffuse[i][1][j];
		}
	}
	return 0;
}

int LS_getLocalSetupData(LOCAL_SETUP_HANDLE h_LocalSetupHandle, H_LOCAL_SETUP_DATA *LocalSetupData)
{
  /**
	*   Transfering the local setup data to a public structure
	*   @param    h_LocalSetupHandle    : local setup handle
	*   @param    LocalSetupData	      : pointer to public structure, needs to point to NULL or to non-allocated memory before the function is called
	*
	*   @return   ERROR_CODE
	*/
  int k = 0;
  int numSp;
  H_LOCAL_SETUP_DATA temp = NULL;
  int error = 0;

  H_LOUDSPEAKER_RENDERING LR = NULL;
  H_BINAURAL_RENDERING binauralRendering_temp = NULL;

  temp = (H_LOCAL_SETUP_DATA)calloc(1, sizeof(struct _LocalSetupData));

  temp->dmxID = -1;

  if (h_LocalSetupHandle->rendering_type == 0)
  {
    LR = (H_LOUDSPEAKER_RENDERING)calloc(1, sizeof(struct _LoudspeakerRendering));
    numSp = h_LocalSetupHandle->LoudspeakerRendering->numSpeakers;
    LR->numSpeakers = numSp;
    LR->externalDistanceCompensation = h_LocalSetupHandle->LoudspeakerRendering->externalDistanceCompensation;
    LR->useTrackingMode = h_LocalSetupHandle->LoudspeakerRendering->useTrackingMode;
    if (h_LocalSetupHandle->LoudspeakerRendering->loudspeakerCalibrationGain != NULL)
    {
      LR->loudspeakerCalibrationGain = (float*)calloc(numSp,sizeof(float));
      for (k = 0; k < numSp; k++)
      {
        LR->loudspeakerCalibrationGain[k] = h_LocalSetupHandle->LoudspeakerRendering->loudspeakerCalibrationGain[k];
      }
    }
    if (h_LocalSetupHandle->LoudspeakerRendering->loudspeakerDistance != NULL)
    {
      LR->loudspeakerDistance= (int*)calloc(numSp,sizeof(int));
      for (k = 0; k < numSp; k++)
      {
        LR->loudspeakerDistance[k] = h_LocalSetupHandle->LoudspeakerRendering->loudspeakerDistance[k];
      }
    }
    LR->Setup_SpeakerConfig3D = (H_LOUDSPEAKER_SETUP)calloc(1, sizeof(struct _LoudspeakerSetup));
    
    LR->Setup_SpeakerConfig3D->speakerLayoutType = h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType;
    
    LR->Setup_SpeakerConfig3D->numSpeakers = h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->numSpeakers;
    LR->Setup_SpeakerConfig3D->numExplicitelySignaledSpeakers = h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->numExplicitelySignaledSpeakers;
    
    if (h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType <= 1)
    {
      LR->Setup_SpeakerConfig3D->CICPspeakerLayoutIdx = h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerLayoutIdx;
      if (h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerIdx != NULL)
      {
        LR->Setup_SpeakerConfig3D->CICPspeakerIdx = (int*)calloc(LR->Setup_SpeakerConfig3D->numSpeakers, sizeof(int));
        for (k = 0; k < numSp; k++)
        {
          LR->Setup_SpeakerConfig3D->CICPspeakerIdx[k] = h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerIdx[k];
        }
      }

      { int m = 0;
        LR->Setup_SpeakerConfig3D->knownPositions = (float**)calloc(3, sizeof(float*));
        for (k = 0; k < 3; k++)
        {
          LR->Setup_SpeakerConfig3D->knownPositions[k] = (float*)calloc(numSp, sizeof(float));
          for (m = 0; m < numSp; m++)
          {
            LR->Setup_SpeakerConfig3D->knownPositions[k][m] = h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[k][m];
          }
        }
      }
    }
    else
    {
      LR->Setup_SpeakerConfig3D->CICPspeakerLayoutIdx = -1;

      LR->Setup_SpeakerConfig3D->FlexibleSpeakerConfig = (H_FLEXIBLE_SPEAKER_CONFIG)calloc(1,sizeof(struct _FlexibleSpeakerConfig));
      LR->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->angularPrecision = h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->angularPrecision;
      
      LR->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->alsoAddSymmetricPair = (int*)calloc(numSp,sizeof(int));
      LR->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Azimuth = (float*)calloc(numSp,sizeof(float));
      LR->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx = (int*)calloc(numSp,sizeof(int));
      LR->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthDirection = (int*)calloc(numSp,sizeof(int));
      LR->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->CICPspeakerIdx = (int*)calloc(numSp,sizeof(int));
      LR->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Elevation = (float*)calloc(numSp,sizeof(float));;
      LR->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationAngleIdx = (int*)calloc(numSp,sizeof(int));
      LR->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationClass = (int*)calloc(numSp,sizeof(int));
      LR->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationDirection = (int*)calloc(numSp,sizeof(int));
      LR->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isCICPspeakerIdx = (int*)calloc(numSp,sizeof(int));
      LR->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isLFE = (int*)calloc(numSp,sizeof(int));

      for (k = 0; k < numSp; k++)
      {
        LR->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->alsoAddSymmetricPair[k] = h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->alsoAddSymmetricPair[k];
        LR->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Azimuth[k] = h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Azimuth[k];
        LR->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx[k] = h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx[k];
        LR->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthDirection[k] = h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthDirection[k];
        LR->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->CICPspeakerIdx[k] = h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->CICPspeakerIdx[k];
        LR->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Elevation[k] = h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Elevation[k];
        LR->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationAngleIdx[k] = h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationAngleIdx[k];
        LR->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationClass[k] = h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationClass[k];
        LR->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationDirection[k] = h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationDirection[k];
        LR->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isCICPspeakerIdx[k] = h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isCICPspeakerIdx[k];
        LR->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isLFE[k] = h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isLFE[k];
      }
    }

    temp->LoudspeakerRendering = LR;
  }
  else
  {
    BinauralRendering *pBinauralRendering;

    binauralRendering_temp = (H_BINAURAL_RENDERING)calloc(1, sizeof(struct _BinauralRendering));
    pBinauralRendering = BinauralRendering_initAlloc();
    binauralRendering_temp->pBR = pBinauralRendering;
    
    binauralRendering_temp->pBR->fileSignature = h_LocalSetupHandle->BinauralRendering->pBR->fileSignature;
    binauralRendering_temp->pBR->fileVersion = h_LocalSetupHandle->BinauralRendering->pBR->fileVersion;
    for (k = 0; k < 257; k++)
    {
      binauralRendering_temp->pBR->name[k] = h_LocalSetupHandle->BinauralRendering->pBR->name[k];
    }

    binauralRendering_temp->pBR->useTrackingMode = h_LocalSetupHandle->BinauralRendering->pBR->useTrackingMode;
    binauralRendering_temp->pBR->numBinauralRepresentation = h_LocalSetupHandle->BinauralRendering->pBR->numBinauralRepresentation;

    for (k = 0; k < binauralRendering_temp->pBR->numBinauralRepresentation; k++)
    {
      
      BinauralRepresentation *pNewBinauralRepresentation = (BinauralRepresentation *)calloc(1,sizeof(BinauralRepresentation));
	   
      if (k >= MAX_BINAURAL_REPRESENTATION)
      {
        free(pNewBinauralRepresentation);
	      fprintf(stderr,"\nError : MAX_BINAURAL_REPRESENTATION reached");
	      return -1;
      }

      pNewBinauralRepresentation->brirSamplingFrequency = h_LocalSetupHandle->BinauralRendering->pBR->ppBinauralRepresentation[k]->brirSamplingFrequency;
      pNewBinauralRepresentation->isHoaData = h_LocalSetupHandle->BinauralRendering->pBR->ppBinauralRepresentation[k]->isHoaData;
      pNewBinauralRepresentation->hoaOrderBinaural = h_LocalSetupHandle->BinauralRendering->pBR->ppBinauralRepresentation[k]->hoaOrderBinaural;
      pNewBinauralRepresentation->nBrirPairs = h_LocalSetupHandle->BinauralRendering->pBR->ppBinauralRepresentation[k]->nBrirPairs;
      pNewBinauralRepresentation->binauralDataFormatID = h_LocalSetupHandle->BinauralRendering->pBR->ppBinauralRepresentation[k]->binauralDataFormatID;
      
      switch (pNewBinauralRepresentation->binauralDataFormatID)
      {
        case 0:  /* 0 : raw FIR */  

          {
            BinauralFirData *pBinauralFirData = (BinauralFirData *)calloc(1,sizeof(BinauralFirData));
            pNewBinauralRepresentation->pBinauralFirData = pBinauralFirData;

	          if ( getBinauralFirData(
		          pNewBinauralRepresentation->pBinauralFirData, 
		          k,
		          h_LocalSetupHandle->BinauralRendering->pBR) != 0) return -1;
	          break;
          }

        case 1: /* 1 : FD parameters */

          {
            FdBinauralRendererParam *pFdBinauralRendererParam = (FdBinauralRendererParam *)calloc(1,sizeof(FdBinauralRendererParam));
            pNewBinauralRepresentation->pFdBinauralRendererParam = pFdBinauralRendererParam;

	          if ( getFdBinauralRendererParam(
		          pNewBinauralRepresentation->pFdBinauralRendererParam, 
		          k,
		          h_LocalSetupHandle->BinauralRendering->pBR) != 0) return -1;  
	          break;
          }

        case 2: /* 2 : TD parameters*/
          {
            TdBinauralRendererParam *pTdBinauralRendererParam = (TdBinauralRendererParam *)calloc(1,sizeof(TdBinauralRendererParam));
            pNewBinauralRepresentation->pTdBinauralRendererParam = pTdBinauralRendererParam;

	          if ( getTdBinauralRendererParam(
		          pNewBinauralRepresentation->pTdBinauralRendererParam, 
		          k,
		          h_LocalSetupHandle->BinauralRendering->pBR) != 0) return -1;
	          break;
          }

        default:
	        fprintf(stderr,"\nError : Unknown binauralDataFormatID");
	        return -1;
      }

      error = getSpeakerConfig3d(&pNewBinauralRepresentation->Setup_SpeakerConfig3d, h_LocalSetupHandle->BinauralRendering->pBR, k);

      /* set representation in BinauralRendering */
		  if ( BinauralRendering_setBinauralRepresentation(binauralRendering_temp->pBR, pNewBinauralRepresentation, k) != 0) 
      {
        return -1;
      }
    }
    temp->BinauralRendering = binauralRendering_temp;
  }

  if (h_LocalSetupHandle->numWireOutputs > 0)
  {
    temp->WireIDs = (int*)calloc(h_LocalSetupHandle->numWireOutputs, sizeof(int));
    temp->numWireOutputs = h_LocalSetupHandle->numWireOutputs;
    for (k = 0; k < temp->numWireOutputs; k++)
    {
      temp->WireIDs[k] = h_LocalSetupHandle->WireIDs[k];
    }
  }
  temp->LocalScreenInfo = (H_LOCAL_SCREEN_INFO)calloc(1, sizeof(struct _LocalScreenInfo));
  if (h_LocalSetupHandle->LocalScreenInfo->hasLocalScreenInfo)
  {
    temp->LocalScreenInfo->hasLocalScreenInfo = 1;
    temp->LocalScreenInfo->az_left = h_LocalSetupHandle->LocalScreenInfo->az_left;
    temp->LocalScreenInfo->az_right = h_LocalSetupHandle->LocalScreenInfo->az_right;
    temp->LocalScreenInfo->hasElevationInfo = h_LocalSetupHandle->LocalScreenInfo->hasElevationInfo;
    temp->LocalScreenInfo->el_bottom = h_LocalSetupHandle->LocalScreenInfo->el_bottom;
    temp->LocalScreenInfo->el_top = h_LocalSetupHandle->LocalScreenInfo->el_top;
  }
  else
  {
    temp->LocalScreenInfo->hasLocalScreenInfo = 0;
    temp->LocalScreenInfo->az_left = 0;
    temp->LocalScreenInfo->az_right = 0;
    temp->LocalScreenInfo->hasElevationInfo = 0;
    temp->LocalScreenInfo->el_bottom = 0;
    temp->LocalScreenInfo->el_top = 0;
  }

  temp->rendering_type = h_LocalSetupHandle->rendering_type;
  *LocalSetupData = temp;

  return error;
}

int getSpeakerConfig3d(Setup_SpeakerConfig3d *pSpeakerConfig3d, BinauralRendering *pBR, int RepresentationIndex)
{

  unsigned int i;
  int numSpeakers = 0;

  pSpeakerConfig3d->speakerLayoutType = pBR->ppBinauralRepresentation[RepresentationIndex]->Setup_SpeakerConfig3d.speakerLayoutType;
  numSpeakers = pBR->ppBinauralRepresentation[RepresentationIndex]->Setup_SpeakerConfig3d.numSpeakers;

	if ( pSpeakerConfig3d->speakerLayoutType == 0 ) 
	{
    pSpeakerConfig3d->CICPspeakerLayoutIdx = pBR->ppBinauralRepresentation[RepresentationIndex]->Setup_SpeakerConfig3d.CICPspeakerLayoutIdx;
	} 
	else if (pSpeakerConfig3d->speakerLayoutType < 3)
	{
    pSpeakerConfig3d->CICPspeakerLayoutIdx = -1;
		pSpeakerConfig3d->numSpeakers =  pBR->ppBinauralRepresentation[RepresentationIndex]->Setup_SpeakerConfig3d.numSpeakers;
		if ( pSpeakerConfig3d->speakerLayoutType == 1 ) 
		{
			for ( i = 0; i < pSpeakerConfig3d->numSpeakers; i++ ) 
			{
				pSpeakerConfig3d->CICPspeakerIdx[i] =  pBR->ppBinauralRepresentation[RepresentationIndex]->Setup_SpeakerConfig3d.CICPspeakerIdx[i];
			}
		}
		if ( pSpeakerConfig3d->speakerLayoutType == 2 ) 
		{
      int i = 0;

      pSpeakerConfig3d->flexibleSpeakerConfig.angularPrecision = pBR->ppBinauralRepresentation[RepresentationIndex]->Setup_SpeakerConfig3d.flexibleSpeakerConfig.angularPrecision;

	    while ( i < numSpeakers ) {

        int azimuth = 0;
        pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].isCICPspeakerIdx = pBR->ppBinauralRepresentation[RepresentationIndex]->Setup_SpeakerConfig3d.flexibleSpeakerConfig.speakerDescription[i].isCICPspeakerIdx;
	      if ( pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].isCICPspeakerIdx ) 
	      {
		      pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].CICPspeakerIdx = pBR->ppBinauralRepresentation[RepresentationIndex]->Setup_SpeakerConfig3d.flexibleSpeakerConfig.speakerDescription[i].CICPspeakerIdx;
	      } 
	      else 
	      {
		      pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].ElevationClass = pBR->ppBinauralRepresentation[RepresentationIndex]->Setup_SpeakerConfig3d.flexibleSpeakerConfig.speakerDescription[i].ElevationClass;
		      if ( pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].ElevationClass == 3 ) 
		      {
				    pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].ElevationAngleIdx = pBR->ppBinauralRepresentation[RepresentationIndex]->Setup_SpeakerConfig3d.flexibleSpeakerConfig.speakerDescription[i].ElevationAngleIdx;
            pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].ElevationDirection = pBR->ppBinauralRepresentation[RepresentationIndex]->Setup_SpeakerConfig3d.flexibleSpeakerConfig.speakerDescription[i].ElevationDirection;
		      }

			    pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].AzimuthAngleIdx = pBR->ppBinauralRepresentation[RepresentationIndex]->Setup_SpeakerConfig3d.flexibleSpeakerConfig.speakerDescription[i].AzimuthAngleIdx;
			    pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].AzimuthDirection = pBR->ppBinauralRepresentation[RepresentationIndex]->Setup_SpeakerConfig3d.flexibleSpeakerConfig.speakerDescription[i].AzimuthDirection;
		      pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].isLFE = pBR->ppBinauralRepresentation[RepresentationIndex]->Setup_SpeakerConfig3d.flexibleSpeakerConfig.speakerDescription[i].isLFE;

	      }

		    if(pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].isCICPspeakerIdx) 
		    {
			    CICP2GEOMETRY_CHANNEL_GEOMETRY AzElLfe; 
			    cicp2geometry_get_geometry_from_cicp_loudspeaker_index( 
				    pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].CICPspeakerIdx, 
				    &AzElLfe);
			    azimuth = AzElLfe.Az;
		    }
		    else 
		    {
			    azimuth = saoc_AzimuthAngleIdxToDegrees ( 
				    pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].AzimuthAngleIdx, 
				    pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].AzimuthDirection, 
				    pSpeakerConfig3d->flexibleSpeakerConfig.angularPrecision );
		    }

		    i++;
	    }
		}
	}

	/* Fill geometry, numChannels and numLFEs ... */
	get_geometry_from_speakerConfig3d(
		pSpeakerConfig3d,
		pSpeakerConfig3d->pGeometry,
		&pSpeakerConfig3d->numChannels,
		&pSpeakerConfig3d->numLFEs);

	/* ... and set numSpeakers */
	pSpeakerConfig3d->numSpeakers = pSpeakerConfig3d->numChannels + pSpeakerConfig3d->numLFEs;
      
  return 0;
}


int LS_quantize_LSRendering_Data(LOCAL_SETUP_HANDLE LocalSetup)
{
  /**
	*   Quantization of the known positions and calibration gain data
	*   @param    LocalSetup    : local setup handle
	*
	*   @return   ERROR_CODE
	*/
  float tempfloat = 0.0f;
  int tempint = 0;
  int k = 0;


  if ((LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType <= 1) && (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions != NULL))
  {
    for (k = 0; k < LocalSetup->LoudspeakerRendering->numSpeakers; k++)
    {
      if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[0][k] == 1)
      {
        tempfloat = LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[1][k];
        tempfloat = max(min(tempfloat,180.0f), -180.0f);
        tempint = (int)((floor(tempfloat + 0.5)) + 256.0f);
        LocalSetup->LoudspeakerRenderingQuant->knownAzimuth[k] = tempint;

        tempfloat = LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[2][k];
        tempfloat = max(min(tempfloat,90.0f), -90.0f);
        tempint = (int)((floor(tempfloat + 0.5)) + 128.0f);
        LocalSetup->LoudspeakerRenderingQuant->knownElevation[k] = tempint;
      }
      else
      {
        LocalSetup->LoudspeakerRenderingQuant->knownAzimuth[k] = -181;
        LocalSetup->LoudspeakerRenderingQuant->knownElevation[k] = -91;
      }
    }
  }
  
  if (LocalSetup->LoudspeakerRendering->loudspeakerCalibrationGain != NULL)
  {
    for (k = 0; k < LocalSetup->LoudspeakerRendering->numSpeakers; k++)
    {
        tempfloat = LocalSetup->LoudspeakerRendering->loudspeakerCalibrationGain[k];
        tempfloat = max(min(tempfloat,31.5f), -32.0f);
        tempint = (int)((floor(tempfloat*2.0f + 0.5) / 2.0f) * 2.0f + 64.0f);
        LocalSetup->LoudspeakerRenderingQuant->loudspeakerCalibrationGain[k] = tempint;
    }
  }
  return 0;
}

int LS_quantize_LocalScreen_Data(LOCAL_SETUP_HANDLE LocalSetup)
{
  /**
	*   Quantization of the local screen size information
	*   @param    LocalSetup    : local setup handle
	*
	*   @return   ERROR_CODE
	*/
  float tempfloat = 0.0f;
  int tempint = 0;

  /* quantize local screen size information */
  if (LocalSetup->LocalScreenInfo->hasLocalScreenInfo)
  {
    if (fabs(LocalSetup->LocalScreenInfo->az_left) == fabs(LocalSetup->LocalScreenInfo->az_right))
    {
      tempfloat = (float)fabs(LocalSetup->LocalScreenInfo->az_left);
      tempfloat = max(min(tempfloat,180.0f), 0.0f);
      tempint = (int)((floor(tempfloat*2.0f + 0.5) / 2.0f) * 2.0f);
      LocalSetup->LocalScreenInfoQuant->az_left = tempint;
    }
    else
    {
      tempfloat = LocalSetup->LocalScreenInfo->az_left;
      tempfloat = max(min(tempfloat,180.0f), -180.0f);
      tempint = (int)((floor(tempfloat*2.0f + 0.5) / 2.0f) * 2.0f + 511);
      LocalSetup->LocalScreenInfoQuant->az_left = tempint;

      tempfloat = LocalSetup->LocalScreenInfo->az_right;
      tempfloat = max(min(tempfloat,180.0f), -180.0f);
      tempint = (int)((floor(tempfloat*2.0f + 0.5) / 2.0f) * 2.0f + 511);
      LocalSetup->LocalScreenInfoQuant->az_right = tempint;
    }

    if (LocalSetup->LocalScreenInfo->hasElevationInfo)
    {
      tempfloat = LocalSetup->LocalScreenInfo->el_top;
      tempfloat = max(min(tempfloat,90.0f), -90.0f);
      tempint = (int)((floor(tempfloat*2.0f + 0.5) / 2.0f) * 2.0f + 255);
      LocalSetup->LocalScreenInfoQuant->el_top = tempint;

      tempfloat = LocalSetup->LocalScreenInfo->el_bottom;
      tempfloat = max(min(tempfloat,90.0f), -90.0f);
      tempint = (int)((floor(tempfloat*2.0f + 0.5) / 2.0f) * 2.0f + 255);
      LocalSetup->LocalScreenInfoQuant->el_bottom = tempint;
    }
  }

  return 0;
}

int LS_freeLocalSetupData(H_LOCAL_SETUP_DATA LocalSetupData)
{
  /**
	*   De-allocation of memory of the public local setup data
	*   @param    LocalSetupData  : local setup data struct
	*
	*   @return   ERROR_CODE
	*/
  unsigned int k = 0;
  unsigned int k2, b, nr;


  if (LocalSetupData->rendering_type == 0)
  {
    if (LocalSetupData->LoudspeakerRendering->loudspeakerCalibrationGain != NULL)
    {
      free(LocalSetupData->LoudspeakerRendering->loudspeakerCalibrationGain); LocalSetupData->LoudspeakerRendering->loudspeakerCalibrationGain = NULL;
    }
    if (LocalSetupData->LoudspeakerRendering->loudspeakerDistance != NULL)
    {
      free(LocalSetupData->LoudspeakerRendering->loudspeakerDistance); LocalSetupData->LoudspeakerRendering->loudspeakerDistance = NULL;
    }
    
    if (LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType <= 1)
    {
      if (LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerIdx != NULL)
      {
        free(LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerIdx);
      }

      if (LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions != NULL)
      {
        for (k = 0; k < 3; k++)
        {
          free(LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[k]); LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[k] = NULL;
        }
        free(LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions); LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions = NULL;
      }
    }
    else
    {
      
      free(LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->alsoAddSymmetricPair); LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->alsoAddSymmetricPair = NULL;
      free(LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Azimuth); LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Azimuth = NULL;
      free(LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx); LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx = NULL;
      free(LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthDirection); LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthDirection = NULL;
      free(LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->CICPspeakerIdx); LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->CICPspeakerIdx = NULL;
      free(LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Elevation); LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Elevation = NULL;
      free(LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationAngleIdx); LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationAngleIdx = NULL;
      free(LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationDirection); LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationDirection = NULL;
      free(LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationClass); LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationClass = NULL;
      free(LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isCICPspeakerIdx); LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isCICPspeakerIdx = NULL;
      free(LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isLFE); LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isLFE = NULL;
    }
    free(LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig); LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig = NULL;
    free(LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D); LocalSetupData->LoudspeakerRendering->Setup_SpeakerConfig3D = NULL;
    free(LocalSetupData->LoudspeakerRendering); LocalSetupData->LoudspeakerRendering = NULL;
  }
  else
  {
    for (k = 0; k < LocalSetupData->BinauralRendering->pBR->numBinauralRepresentation; k++)
    {
      unsigned int nBrirPairs = LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->nBrirPairs;

      switch (LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->binauralDataFormatID)
      {
        case 0:  /* 0 : raw FIR */ 
          {
            break;
          }
        case 1: /* 1 : FD parameters */  
          {
            for(k2=0; k2<LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->kMax; k2++)
	          {
              for( b=0; b<LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->nBlk[k2]; b++)
		          {	
                for( nr=0; nr < nBrirPairs; nr++)
			          {
				          free(LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffLeftReal[k2][b][nr]);
				          free(LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffLeftImag[k2][b][nr]);
				          free(LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffRightReal[k2][b][nr]);
				          free(LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffRightImag[k2][b][nr]);
                }	
			          free(LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffLeftReal[k2][b]);
			          free(LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffLeftImag[k2][b]);
			          free(LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffRightReal[k2][b]);
			          free(LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffRightImag[k2][b]);
              }
              free(LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffLeftReal[k2]);
              free(LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffLeftImag[k2]);
              free(LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffRightReal[k2]);
              free(LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffRightImag[k2]);
            }

            for(k2=0; k2<LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->kMax - LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->kConv; k2++)
	          {
              free(LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppQtdlGainLeftReal[k2]);
		          free(LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppQtdlGainLeftImag[k2]);
		          free(LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppQtdlGainRightReal[k2]);
		          free(LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppQtdlGainRightImag[k2]);
		          free(LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppQtdlLagLeft[k2]);
		          free(LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppQtdlLagRight[k2]);
            }

            free(LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->pBinauralFirData);
            free(LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->fcAna);
            free(LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->nBlk);
            free(LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->nFft);
            free(LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->nFilter);
            free(LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->nrgLr);
            free(LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->rt60);


            free(LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam);
            
            break;
          }
        case 2: /* 2 : TD parameters*/
          {
            free(LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]->pTdBinauralRendererParam);
          }
        default:
          {
            fprintf(stderr,"\nError : Unknown binauralDataFormatID");
		        return -1;
          }
      }
      free(LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k]); LocalSetupData->BinauralRendering->pBR->ppBinauralRepresentation[k] = NULL;
    }
    free(LocalSetupData->BinauralRendering->pBR); LocalSetupData->BinauralRendering->pBR = NULL;
    free(LocalSetupData->BinauralRendering); LocalSetupData->BinauralRendering = NULL;
  }

  if (LocalSetupData->numWireOutputs > 0)
  {
    free(LocalSetupData->WireIDs);
  }
  if (LocalSetupData->LocalScreenInfo != NULL)
  {
    free(LocalSetupData->LocalScreenInfo); LocalSetupData->LocalScreenInfo = NULL;
  }
  free(LocalSetupData);

  return 0;
}


int LS_closeHandle(LOCAL_SETUP_HANDLE LocalSetup)
{
  /**
	*   Closing of the local setup handle
	*   @param    LocalSetup    : local setup handle
	*
	*   @return   ERROR_CODE
	*/
  unsigned int k = 0;
  unsigned int k2, b, nr;
  unsigned int nBrirPairs;

  if (LocalSetup->rendering_type == 0)
  {
    if(LocalSetup->LoudspeakerRenderingQuant != NULL)
    {
      if (LocalSetup->LoudspeakerRenderingQuant->loudspeakerCalibrationGain != NULL)
      {
        free(LocalSetup->LoudspeakerRenderingQuant->loudspeakerCalibrationGain); LocalSetup->LoudspeakerRenderingQuant->loudspeakerCalibrationGain = NULL;
      }
      if (LocalSetup->LoudspeakerRenderingQuant->knownAzimuth != NULL)
      {
        free(LocalSetup->LoudspeakerRenderingQuant->knownAzimuth); LocalSetup->LoudspeakerRenderingQuant->knownAzimuth = NULL;
      }
      if (LocalSetup->LoudspeakerRenderingQuant->knownElevation != NULL)
      {
        free(LocalSetup->LoudspeakerRenderingQuant->knownElevation); LocalSetup->LoudspeakerRenderingQuant->knownElevation = NULL;
      }
      free(LocalSetup->LoudspeakerRenderingQuant); LocalSetup->LoudspeakerRenderingQuant = NULL;
    }

    if (LocalSetup->LoudspeakerRendering->loudspeakerCalibrationGain != NULL)
    {
      free(LocalSetup->LoudspeakerRendering->loudspeakerCalibrationGain); LocalSetup->LoudspeakerRendering->loudspeakerCalibrationGain = NULL;
    }
    if(LocalSetup->LoudspeakerRendering->loudspeakerDistance != NULL)
    {
      free(LocalSetup->LoudspeakerRendering->loudspeakerDistance); LocalSetup->LoudspeakerRendering->loudspeakerDistance = NULL;
    }

    if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D != NULL)
    {
      if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerIdx != NULL)
      {
        free(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerIdx); LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerIdx = NULL;
      }
      if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions != NULL)
      {
        for (k = 0; k < 3; k++)
        {
          free(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[k]); LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[k] = NULL;
        }
        free(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions); LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions = NULL;
      }

      if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig != NULL)
      {
        if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->alsoAddSymmetricPair != NULL)
        {
          free(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->alsoAddSymmetricPair); LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->alsoAddSymmetricPair = NULL;
        }
        if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Azimuth != NULL)
        {
          free(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Azimuth); LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Azimuth = NULL;
        }
        if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx != NULL)
        {
          free(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx); LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx = NULL;
        }
        if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthDirection != NULL)
        {
          free(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthDirection); LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthDirection = NULL;
        }
        if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->CICPspeakerIdx != NULL)
        {
          free(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->CICPspeakerIdx); LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->CICPspeakerIdx = NULL;
        }
        if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Elevation != NULL)
        {
          free(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Elevation); LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Elevation = NULL;
        }
        if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationAngleIdx != NULL)
        {
          free(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationAngleIdx); LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationAngleIdx = NULL;
        }
        if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationClass != NULL)
        {
          free(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationClass); LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationClass = NULL;
        }
        if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationDirection != NULL)
        {
          free(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationDirection); LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationDirection = NULL;
        }
        if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isCICPspeakerIdx != NULL)
        {
          free(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isCICPspeakerIdx); LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isCICPspeakerIdx = NULL;
        }
        if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isLFE != NULL)
        {
          free(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isLFE); LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isLFE = NULL;
        }
        free(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig);  LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig = NULL;
      }
      free(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D); LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D = NULL;
    }
    free(LocalSetup->LoudspeakerRendering); LocalSetup->LoudspeakerRendering = NULL;
  }
  else
  {
    for (k = 0; k < LocalSetup->BinauralRendering->pBR->numBinauralRepresentation; k++)
    {
      if (LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->binauralDataFormatID == 1)
      {
        nBrirPairs = LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->nBrirPairs;

        for(k2=0; k2<LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->kMax; k2++)
        {
          for( b=0; b<LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->nBlk[k2]; b++)
          {	
            for( nr=0; nr < nBrirPairs; nr++)
	          {
		          free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffLeftReal[k2][b][nr]);
		          free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffLeftImag[k2][b][nr]);
		          free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffRightReal[k2][b][nr]);
		          free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffRightImag[k2][b][nr]);
            }	
	          free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffLeftReal[k2][b]);
	          free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffLeftImag[k2][b]);
	          free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffRightReal[k2][b]);
	          free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffRightImag[k2][b]);
          }
          free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffLeftReal[k2]);
          free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffLeftImag[k2]);
          free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffRightReal[k2]);
          free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffRightImag[k2]);
        }
        free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffLeftReal);LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffLeftReal = NULL;
        free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffLeftImag);LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffLeftImag = NULL;
        free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffRightReal);LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffRightReal = NULL;
        free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffRightImag); LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppppVoffCoeffRightImag = NULL;
        

        for(k2=0; k2<LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->kMax-LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->kConv; k2++)
        {
          free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppQtdlGainLeftReal[k2]);
          free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppQtdlGainLeftImag[k2]);
          free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppQtdlGainRightReal[k2]);
          free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppQtdlGainRightImag[k2]);
          free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppQtdlLagLeft[k2]);
          free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->ppQtdlLagRight[k2]);
        }


        free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->fcAna);
        free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->nBlk);
        free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->nFft);
        free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->nFilter);
        free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->nrgLr);
        free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam->rt60);

        free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pFdBinauralRendererParam);

      }
      else if (LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->binauralDataFormatID == 0)
      {
        free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pBinauralFirData); LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pBinauralFirData = NULL;
      }
      else if (LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->binauralDataFormatID == 2)
      {
        free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pTdBinauralRendererParam); LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]->pTdBinauralRendererParam = NULL;
      }

      free(LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k]); LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[k] = NULL;
    }
    free(LocalSetup->BinauralRendering->pBR); LocalSetup->BinauralRendering->pBR = NULL;
    free(LocalSetup->BinauralRendering); LocalSetup->BinauralRendering = NULL;
  }

  free(LocalSetup->LocalScreenInfo); LocalSetup->LocalScreenInfo = NULL;
  if (LocalSetup->LocalScreenInfoQuant != NULL)
  {
    free(LocalSetup->LocalScreenInfoQuant); LocalSetup->LocalScreenInfoQuant = NULL;
  }
  if (LocalSetup->numWireOutputs > 0)
  {
    free(LocalSetup->WireIDs); LocalSetup->WireIDs = NULL;
  }

  free(LocalSetup);
  
  return 0;
}

int LS_writeBits_BinauralRendering(LOCAL_SETUP_HANDLE LocalSetup, H_LS_BITSTREAM_ENCODER h_bitstream, unsigned long *noBitsWritten, unsigned long *noBitsWrittenTotal, char *filename)
{
  short i_representation;
  unsigned long no_BitsWritten = 0;
  unsigned long ii;
	BinauralRepresentation *pBinauralRepresentation = NULL;

	unsigned char *bitbuf;
  int k;
  int error = 0;

	long	nBytes, nBits, nBits_NoByteAlign;
	wobitbuf writebitBuffer;
	wobitbufHandle hBitstream = &writebitBuffer;
	unsigned short numCharName, brirSamplingFrequencyIdx;

  /* write all bitstream file into the bitbuffer */
	nBytes = BITSTREAM_MAX_NUM_MEGA_BYTES * 1024 * 1024;
	nBits = 8 * nBytes;
	bitbuf = (unsigned char*)malloc(nBytes * sizeof(unsigned char));
	wobitbuf_Init(hBitstream, bitbuf, nBits, 0);

  wobitbuf_WriteBits(hBitstream, LocalSetup->rendering_type, 1); /* bsFileSignature */
  /* Write BinauralRendering "header" */
  wobitbuf_WriteBits(hBitstream, LocalSetup->BinauralRendering->pBR->fileSignature, 32); /* bsFileSignature */
  wobitbuf_WriteBits(hBitstream, LocalSetup->BinauralRendering->pBR->fileVersion, 8);    /* bsFileVersion */
	numCharName = (strlen(filename) < 255) ? (unsigned char)strlen(filename) : 255;      /* max 255 char */
	wobitbuf_WriteBits(hBitstream, numCharName, 8);                                      /* bsNumCharName */
	for (ii = 0; ii < numCharName; ++ii)
	{
		wobitbuf_WriteBits(hBitstream, filename[ii], 8); /* bsName[ii] */
	}
  wobitbuf_WriteBits(hBitstream, LocalSetup->BinauralRendering->pBR->useTrackingMode, 1); /* useTrackingMode */
	wobitbuf_WriteBits(hBitstream, LocalSetup->BinauralRendering->pBR->numBinauralRepresentation, 4); /* bsNumBinauralDataRepresentation */

	/* Write BinauralRepresentations */
	for (i_representation=0; i_representation<LocalSetup->BinauralRendering->pBR->numBinauralRepresentation; ++i_representation)
	{
		pBinauralRepresentation = LocalSetup->BinauralRendering->pBR->ppBinauralRepresentation[i_representation];
		brirSamplingFrequencyIdx = getBrirSamplingFrequencyIdx(pBinauralRepresentation->brirSamplingFrequency); 
		wobitbuf_WriteBits(hBitstream, brirSamplingFrequencyIdx, 5); /* brirSamplingFrequencyIndex */
		if (brirSamplingFrequencyIdx == 0x1F)
		{
			wobitbuf_WriteBits(hBitstream, pBinauralRepresentation->brirSamplingFrequency, 24); /* brirSamplingFrequency */
		}
		wobitbuf_WriteBits(hBitstream, pBinauralRepresentation->isHoaData, 1); /* isHoaData */
		if (pBinauralRepresentation->isHoaData)
		{
			WriteEscapedValue(hBitstream, pBinauralRepresentation->hoaOrderBinaural, 3,5,0); /* hoaOrderBinaural  */
		}
		else
		{
			/* write Setup_SpeakerConfig3d */
			writeSpeakerConfig3d(&pBinauralRepresentation->Setup_SpeakerConfig3d, hBitstream);

			if (pBinauralRepresentation->Setup_SpeakerConfig3d.speakerLayoutType == 0)
			{
				WriteEscapedValue(hBitstream, pBinauralRepresentation->nBrirPairs - 1, 5,8,0); /* nBrirPairs (-1) */
			}
		}

		wobitbuf_WriteBits(hBitstream, pBinauralRepresentation->binauralDataFormatID, 2); /* binauralDataFormatID */
		wobitbuf_ByteAlign(hBitstream);

		switch (pBinauralRepresentation->binauralDataFormatID)
		{
		case 0:  /* 0 : raw FIR */     
			writeBinauralFirData(
				pBinauralRepresentation->pBinauralFirData, 
				pBinauralRepresentation->nBrirPairs,
				hBitstream); 
			break;

		case 1: /* 1 : FD parameters */      
			writeFdBinauralRendererParam(
				pBinauralRepresentation->pFdBinauralRendererParam, 
				pBinauralRepresentation->nBrirPairs,
				hBitstream); 
			break;

		case 2: /* 2 : TD parameters*/
			writeTdBinauralRendererParam(
				pBinauralRepresentation->pTdBinauralRendererParam, 
				pBinauralRepresentation->nBrirPairs,
				hBitstream);
			break;

		default:
			fprintf(stderr,"\nError : Unknown binauralDataFormatID");
			return -1;

		}
	}
  
  if ((hBitstream->bitsWritten % 8) != 0)
  {
    wobitbuf_ByteAlign(hBitstream);
  }

  wobitbuf_WriteBits(hBitstream,(unsigned int)LocalSetup->numWireOutputs,16);
  if (LocalSetup->numWireOutputs > 0)
  {
    for (k = 0; k < LocalSetup->numWireOutputs; k++)
    {
      wobitbuf_WriteBits(hBitstream,(unsigned int)LocalSetup->WireIDs[k], 16);
    }
  }
  if (LocalSetup->LocalScreenInfo->hasLocalScreenInfo == 1)
  {
    error = LS_quantize_LocalScreen_Data(LocalSetup);
    wobitbuf_WriteBits(hBitstream,1,1);

    if (fabs(LocalSetup->LocalScreenInfo->az_left) == fabs(LocalSetup->LocalScreenInfo->az_right))
    {
      wobitbuf_WriteBits(hBitstream,1,1);
      wobitbuf_WriteBits(hBitstream,(unsigned int)LocalSetup->LocalScreenInfoQuant->az_left,9);

    }
    else
    {
      wobitbuf_WriteBits(hBitstream,0,1);
      wobitbuf_WriteBits(hBitstream,(unsigned int)LocalSetup->LocalScreenInfoQuant->az_left,10);
      wobitbuf_WriteBits(hBitstream,(unsigned int)LocalSetup->LocalScreenInfoQuant->az_right,10);
    }
    wobitbuf_WriteBits(hBitstream,LocalSetup->LocalScreenInfo->hasElevationInfo,1);
    if (LocalSetup->LocalScreenInfo->hasElevationInfo == 1)
    {
      wobitbuf_WriteBits(hBitstream,(unsigned int)LocalSetup->LocalScreenInfoQuant->el_top,9);
      wobitbuf_WriteBits(hBitstream,(unsigned int)LocalSetup->LocalScreenInfoQuant->el_bottom,9);
    }
  }
  else
  {
    wobitbuf_WriteBits(hBitstream,0,1);
  }

  nBits_NoByteAlign = wobitbuf_GetBitsWritten(hBitstream);
  wobitbuf_ByteAlign(hBitstream);

  /* write bitbuffer to file */
	nBits = wobitbuf_GetBitsWritten(hBitstream);
	nBytes = nBits / 8;
  
  fwrite(bitbuf, nBytes, 1, h_bitstream->bsFile);
  no_BitsWritten += nBits_NoByteAlign; 

  *noBitsWritten = no_BitsWritten;
  *noBitsWrittenTotal += no_BitsWritten;

  free(bitbuf);
  return error;

}

int LS_writeBits_LSRendering(LOCAL_SETUP_HANDLE LocalSetup, H_LS_BITSTREAM_ENCODER h_bitstream, unsigned long *noBitsWritten, unsigned long *noBitsWrittenTotal)
{
  /**
	*   Writing of the bitstream for loudspeaker rendering ()LoudspeakerRendering() syntax)
	*   @param    LocalSetup        : local setup handle
	*   @param    h_bitstream	      : bitstream handle
  *   @param    no_BitsWritten		: output parameter, number of read bits
  *   @param    noBitsWrittenTotal: output parameter, total number of read bits, add the number of read bits to the previous value of the variable
	*
	*   @return   ERROR_CODE
	*/
  unsigned long no_BitsWritten = 0;
  int k = 0;

  no_BitsWritten += LS_DecInt_writeBits(h_bitstream,(unsigned int)LocalSetup->LoudspeakerRendering->numSpeakers,16);
  
  /*Setup_SpeakerConfig3d*/
  no_BitsWritten += LS_DecInt_writeBits(h_bitstream,(unsigned int)LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType, 2);
  if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType == 0)
  {
    no_BitsWritten += LS_DecInt_writeBits(h_bitstream,(unsigned int)LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerLayoutIdx,6);
  }
  else if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType < 3)
  {
    no_BitsWritten += LS_DecInt_writeEscapedValue(h_bitstream,(unsigned int)LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->numSpeakers,5,8,16);
    
    if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType == 1)
    {
      for (k = 0; k < LocalSetup->LoudspeakerRendering->numSpeakers; k++)
      {
        no_BitsWritten += LS_DecInt_writeBits(h_bitstream,(unsigned int)LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerIdx[k],7);
      }
    }
    else if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType == 2)
    {
      if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->angularPrecision == 5)
      {
        no_BitsWritten += LS_DecInt_writeBits(h_bitstream,0,1); /* angularPrecision to flag */
      }
      else
      {
        no_BitsWritten += LS_DecInt_writeBits(h_bitstream,1,1);
      }
      for (k = 0; k < LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->numExplicitelySignaledSpeakers; k++)
      {
        if ((LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isCICPspeakerIdx != NULL) && (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isCICPspeakerIdx[k] == 1))
        {
          no_BitsWritten += LS_DecInt_writeBits(h_bitstream,1,1);
          no_BitsWritten += LS_DecInt_writeBits(h_bitstream,(unsigned int)LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->CICPspeakerIdx[k],7);
        }
        else
        {
          no_BitsWritten += LS_DecInt_writeBits(h_bitstream,0,1);
          no_BitsWritten += LS_DecInt_writeBits(h_bitstream,(unsigned int)LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationClass[k],2);
          if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationClass[k] == 3)
          {
            if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->angularPrecision == 5)
            {
              /*5 degree precision */
              no_BitsWritten += LS_DecInt_writeBits(h_bitstream,(unsigned int)LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationAngleIdx[k],5);
            }
            else
            {
              /* 1 degree precision */
              no_BitsWritten += LS_DecInt_writeBits(h_bitstream,(unsigned int)LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationAngleIdx[k],7);
            }
            if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationAngleIdx[k] != 0)
            {
              no_BitsWritten += LS_DecInt_writeBits(h_bitstream,(unsigned int)LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationDirection[k],1);
            }
          }
          if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->angularPrecision == 5)
          {
            no_BitsWritten += LS_DecInt_writeBits(h_bitstream,(unsigned int)LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx[k],6);
          }
          else
          {
            no_BitsWritten += LS_DecInt_writeBits(h_bitstream,(unsigned int)LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx[k],8);
          }
          if ((LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx[k] * LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->angularPrecision != 0) && (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx[k] * LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->angularPrecision != 180))
          {
            no_BitsWritten += LS_DecInt_writeBits(h_bitstream,(unsigned int)LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthDirection[k],1);
          }
          no_BitsWritten += LS_DecInt_writeBits(h_bitstream,(unsigned int)LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isLFE[k],1);
        }
        if ((LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isCICPspeakerIdx != NULL) && (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isCICPspeakerIdx[k] == 1))
        {
          if ((LS_getAzimuthFromCICPspeakerIdx(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->CICPspeakerIdx[k]) != 0) && (LS_getAzimuthFromCICPspeakerIdx(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->CICPspeakerIdx[k]) != 180))
          {
            no_BitsWritten += LS_DecInt_writeBits(h_bitstream,(unsigned int)LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->alsoAddSymmetricPair[k],1);
          }
        }
        else
        {
          if ((LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx[k] * LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->angularPrecision != 0) && (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx[k] * LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->angularPrecision != 180))
          {
            no_BitsWritten += LS_DecInt_writeBits(h_bitstream,(unsigned int)LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->alsoAddSymmetricPair[k],1);
          }
        }
      }
    }
  }

  if (LocalSetup->LoudspeakerRendering->loudspeakerDistance != NULL)
  {
    no_BitsWritten += LS_DecInt_writeBits(h_bitstream,1,1); /* hasLoudspeakerDistance */
  }
  else
  {
    no_BitsWritten += LS_DecInt_writeBits(h_bitstream,0,1);
  }
  
  if (LocalSetup->LoudspeakerRenderingQuant->loudspeakerCalibrationGain != NULL)
  {
    no_BitsWritten += LS_DecInt_writeBits(h_bitstream,1,1); /* hasLoudspeakerCalibrationGain */
  }
  else
  {
    no_BitsWritten += LS_DecInt_writeBits(h_bitstream,0,1);
  }

  if (LocalSetup->LoudspeakerRendering->useTrackingMode == 1)
  {
    no_BitsWritten += LS_DecInt_writeBits(h_bitstream,1,1); /* useTrackingMode */
  }
  else
  {
    no_BitsWritten += LS_DecInt_writeBits(h_bitstream,0,1);
  }

  for (k = 0; k < LocalSetup->LoudspeakerRendering->numSpeakers; k++)
  {
    if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType <= 1)
    {
      if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions != NULL)
      {
        no_BitsWritten += LS_DecInt_writeBits(h_bitstream,(unsigned int)LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[0][k],1);
        if ((int)LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[0][k] == 1)
        {
          no_BitsWritten += LS_DecInt_writeBits(h_bitstream,(unsigned int)LocalSetup->LoudspeakerRenderingQuant->knownAzimuth[k],9);
          no_BitsWritten += LS_DecInt_writeBits(h_bitstream,(unsigned int)LocalSetup->LoudspeakerRenderingQuant->knownElevation[k],8);
        }
      }
      else
      {
        no_BitsWritten += LS_DecInt_writeBits(h_bitstream,0,1);
      }
    }
    if (LocalSetup->LoudspeakerRendering->loudspeakerDistance != NULL)
    {
      no_BitsWritten += LS_DecInt_writeBits(h_bitstream,(unsigned int)LocalSetup->LoudspeakerRendering->loudspeakerDistance[k],10);
    }
    if (LocalSetup->LoudspeakerRenderingQuant->loudspeakerCalibrationGain != NULL)
    {
      no_BitsWritten += LS_DecInt_writeBits(h_bitstream,(unsigned int)LocalSetup->LoudspeakerRenderingQuant->loudspeakerCalibrationGain[k],7);
    }
  }
  no_BitsWritten += LS_DecInt_writeBits(h_bitstream,(unsigned int)LocalSetup->LoudspeakerRendering->externalDistanceCompensation,1);

  *noBitsWritten = no_BitsWritten;
  *noBitsWrittenTotal += no_BitsWritten;
  

  return 0;
}

int LS_setLocalScreenSize(LOCAL_SETUP_HANDLE h_LocalSetupHandle, H_LOCAL_SCREEN_INFO LocalScreenInfo)
{
  /**
	*   Set the local screen size
	*   @param    h_LocalSetupHandle    : local setup handle
	*   @param    LocalScreenInfo	      : local screen size handle
	*
	*   @return   ERROR_CODE
	*/
  h_LocalSetupHandle->LocalScreenInfo->hasLocalScreenInfo = LocalScreenInfo->hasLocalScreenInfo;

  if (LocalScreenInfo->hasLocalScreenInfo == 1)
  {
    h_LocalSetupHandle->LocalScreenInfo->az_left = LocalScreenInfo->az_left;
    h_LocalSetupHandle->LocalScreenInfo->az_right = LocalScreenInfo->az_right;

    h_LocalSetupHandle->LocalScreenInfo->hasElevationInfo = LocalScreenInfo->hasElevationInfo;
    if (LocalScreenInfo->hasElevationInfo == 1)
    {
      h_LocalSetupHandle->LocalScreenInfo->el_top = LocalScreenInfo->el_top;
      h_LocalSetupHandle->LocalScreenInfo->el_bottom = LocalScreenInfo->el_bottom;
    }
  }

  return 0;
}
int LS_setBRIRsFromWav_HOA(LOCAL_SETUP_HANDLE h_LocalSetupHandle, char *path, int hoaOrder, int useTrackingMode)
{
  int error = 0;
  error = BinauralRendering_addRepresentationFromWav_HOA(h_LocalSetupHandle->BinauralRendering->pBR, path, hoaOrder, useTrackingMode);
  h_LocalSetupHandle->BinauralRendering->pBR->useTrackingMode = useTrackingMode;
  h_LocalSetupHandle->rendering_type = 1;
  return error;
}

int LS_setBRIRsFromWav_CICP_GEO(LOCAL_SETUP_HANDLE h_LocalSetupHandle, char *path, int cicp, char*geo_path, int useTrackingMode)
{
  int error = 0;
  error = BinauralRendering_addRepresentationFromWav_CICP_GEO(h_LocalSetupHandle->BinauralRendering->pBR, path, cicp, geo_path, useTrackingMode);
  h_LocalSetupHandle->BinauralRendering->pBR->useTrackingMode = useTrackingMode;
  h_LocalSetupHandle->rendering_type = 1;
  return error;
}

int LS_setBRIRsFromBitstream(LOCAL_SETUP_HANDLE h_LocalSetupHandle,  char *path)
{
    BinauralRendering *pT; 
    pT = (BinauralRendering*)calloc(1,sizeof(BinauralRendering));
    pT = BinauralRendering_initAlloc();
	  if (bitstreamRead(path, pT) != 0)
	  {
		  return -1;
	  }
    h_LocalSetupHandle->BinauralRendering->pBR = pT;
  return 0;
}

int LS_initHandle(LOCAL_SETUP_HANDLE *h_LocalSetupHandle)
{
  /**
	*   Initialization of the local setup handle
	*   @param    LOCAL_SETUP_HANDLE    : pointer to the local setup handle, needs to point to NULL or non-allocated memory before the function is called
	*
	*   @return   ERROR_CODE
	*/
  LOCAL_SETUP_HANDLE temp = (LOCAL_SETUP_HANDLE)calloc(1,sizeof(struct _LocalSetupHandle));

  H_LOCAL_SCREEN_INFO screenInfo_temp = (H_LOCAL_SCREEN_INFO)calloc(1, sizeof(struct _LocalScreenInfo));
  H_LOCAL_SCREEN_INFO_QUANT screenInfoQuant_temp = (H_LOCAL_SCREEN_INFO_QUANT)calloc(1, sizeof(struct _LocalScreenInfoQuant));
  H_LOUDSPEAKER_RENDERING loudspeakerRendering_temp = (H_LOUDSPEAKER_RENDERING)calloc(1, sizeof(struct _LoudspeakerRendering));
  H_LOUDSPEAKER_RENDERING_QUANT loudspeakerRenderingQuant_temp = (H_LOUDSPEAKER_RENDERING_QUANT)calloc(1, sizeof(struct _LoudspeakerRenderingQuant));
  
  H_LOUDSPEAKER_SETUP loudspeakerSetup_temp = (H_LOUDSPEAKER_SETUP)calloc(1,sizeof(struct _LoudspeakerSetup));
  H_FLEXIBLE_SPEAKER_CONFIG flexibleSpeakerConfig_temp = (H_FLEXIBLE_SPEAKER_CONFIG)calloc(1, sizeof(struct _FlexibleSpeakerConfig));
  
  H_BINAURAL_RENDERING binauralRendering_temp = (H_BINAURAL_RENDERING)calloc(1, sizeof(struct _BinauralRendering));
  BinauralRendering *pBinauralRendering;

  pBinauralRendering = BinauralRendering_initAlloc();
  binauralRendering_temp->pBR = pBinauralRendering;

  temp->LocalScreenInfo = screenInfo_temp;
  temp->LocalScreenInfoQuant = screenInfoQuant_temp;
  temp->LoudspeakerRendering = loudspeakerRendering_temp;
  loudspeakerSetup_temp->FlexibleSpeakerConfig = flexibleSpeakerConfig_temp;
  loudspeakerSetup_temp->numSpeakers = -1;
  loudspeakerRendering_temp->Setup_SpeakerConfig3D = loudspeakerSetup_temp;
  temp->LoudspeakerRenderingQuant = loudspeakerRenderingQuant_temp;
  temp->BinauralRendering = binauralRendering_temp;

  temp->rendering_type = -1;
  temp->numWireOutputs = -1;
  temp->WireIDs = NULL;
  temp->noBits = -1;

  temp->LocalScreenInfo->hasLocalScreenInfo = 0;
  temp->LocalScreenInfo->az_left = 0.0f;
  temp->LocalScreenInfo->az_right = 0.0f;
  temp->LocalScreenInfo->el_top = 0.0f;
  temp->LocalScreenInfo->el_bottom = 0.0f;

  temp->LocalScreenInfoQuant->az_right = 0;
  temp->LocalScreenInfoQuant->az_left = 0;
  temp->LocalScreenInfoQuant->el_top = 0;
  temp->LocalScreenInfoQuant->el_bottom = 0;

  temp->LoudspeakerRendering->loudspeakerDistance = NULL;
  temp->LoudspeakerRendering->loudspeakerCalibrationGain = NULL;
  temp->LoudspeakerRendering->externalDistanceCompensation = -1;
  temp->LoudspeakerRendering->numSpeakers = -1;
  temp->LoudspeakerRendering->useTrackingMode = 0;
  temp->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerLayoutIdx = -1;
  temp->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerIdx = NULL;
  temp->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->alsoAddSymmetricPair = NULL;
  temp->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->angularPrecision = -1;
  temp->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Azimuth = NULL;
  temp->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx = NULL;
  temp->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthDirection = NULL;
  temp->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthDirection = NULL;
  temp->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Elevation = NULL;
  temp->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationAngleIdx = NULL;
  temp->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationClass = NULL;
  temp->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationDirection = NULL;
  temp->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isCICPspeakerIdx = NULL;
  temp->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isLFE = NULL;

  temp->LoudspeakerRenderingQuant->loudspeakerCalibrationGain = NULL;
  temp->LoudspeakerRenderingQuant->knownAzimuth = NULL;
  temp->LoudspeakerRenderingQuant->knownElevation = NULL;

  /* temp->BinauralRendering->name = NULL; */

  *h_LocalSetupHandle = temp;
  return 0;

}

int LS_initRenderingType(LOCAL_SETUP_HANDLE h_LocalSetupHandle, int rendering_type)
{
  h_LocalSetupHandle->rendering_type = rendering_type;
  return 0;
}

int LS_initWireOutputs(LOCAL_SETUP_HANDLE h_LocalSetupHandle, int numWireOutputs, int *WireIDs)
{  
  /**
	*   Initialization of the WIRE outputs, sets number of WIREs and WireIDs
	*   @param    h_LocalSetupHandle  : local setup handle
	*   @param    numWireOutputs	    : number of WIRE outputs
  *   @param    WireIDs		          : array of WireIDs
	*
	*   @return   ERROR_CODE
	*/

  int k = 0;
  if ((numWireOutputs > 0) && (WireIDs != NULL))
  {
    if (h_LocalSetupHandle->WireIDs != NULL)
    {
      free(h_LocalSetupHandle->WireIDs); h_LocalSetupHandle->WireIDs = NULL;
    }
    h_LocalSetupHandle->WireIDs = (int*)calloc(numWireOutputs, sizeof(int));
    for (k = 0; k < numWireOutputs; k ++)
    {
      h_LocalSetupHandle->WireIDs[k] = WireIDs[k];
    }
    h_LocalSetupHandle->numWireOutputs = numWireOutputs;
  }
  else
  {
    if (h_LocalSetupHandle->WireIDs != NULL)
    {
      free(h_LocalSetupHandle->WireIDs); h_LocalSetupHandle->WireIDs = NULL;
    }
    h_LocalSetupHandle->numWireOutputs = 0;
  }
  
return 0; 

}

int LS_setLoudspeakerRenderingConfig(LOCAL_SETUP_HANDLE h_LocalSetupHandle, int extDistComp, int numLS, int *LSdist, float *LScalibGain, int useTrackingMode)
{
  /**
	*   Setting of the external distance compensation flag, the speaker distances and calibration gains
	*   @param    h_LocalSetupHandle  : local setup handle
	*   @param    extDistComp	        : flag that signals if external distance compensation is applied
  *   @param    numLS		            : number of speakers
  *   @param    LSdist		          : array of loudspeaker distances
  *   @param    LScalibGain		      : array of loudspeaker calibration gains
	*
	*   @return   ERROR_CODE
	*/
  int k = 0;
  h_LocalSetupHandle->rendering_type = 0;
  h_LocalSetupHandle->LoudspeakerRendering->externalDistanceCompensation = extDistComp;
  h_LocalSetupHandle->LoudspeakerRendering->numSpeakers = numLS;
  h_LocalSetupHandle->LoudspeakerRendering->useTrackingMode = useTrackingMode;

  if (h_LocalSetupHandle->LoudspeakerRendering->loudspeakerDistance != NULL)
  {
    free(h_LocalSetupHandle->LoudspeakerRendering->loudspeakerDistance); h_LocalSetupHandle->LoudspeakerRendering->loudspeakerDistance = NULL;
  }
  if (h_LocalSetupHandle->LoudspeakerRendering->loudspeakerCalibrationGain != NULL)
  {
    free(h_LocalSetupHandle->LoudspeakerRendering->loudspeakerCalibrationGain); h_LocalSetupHandle->LoudspeakerRendering->loudspeakerCalibrationGain = NULL;
  }

  if (numLS > 0)
  {
    if (LSdist != NULL)
    {
      h_LocalSetupHandle->LoudspeakerRendering->loudspeakerDistance = (int*)calloc(numLS, sizeof(int));
      for (k = 0; k < numLS; k++)
      {
        h_LocalSetupHandle->LoudspeakerRendering->loudspeakerDistance[k] = LSdist[k];
      }
    }
    if (LScalibGain != NULL)
    {
      h_LocalSetupHandle->LoudspeakerRenderingQuant->loudspeakerCalibrationGain = (int*)calloc(numLS, sizeof(int));
      h_LocalSetupHandle->LoudspeakerRendering->loudspeakerCalibrationGain = (float*)calloc(numLS, sizeof(float));
      for (k = 0; k < numLS; k++)
      {
        h_LocalSetupHandle->LoudspeakerRendering->loudspeakerCalibrationGain[k] = LScalibGain[k];
      }
    }
    return 0;
  }
  else
  {
    return -1;
  }
}

int LS_setFixedLoudspeakerLayout(LOCAL_SETUP_HANDLE h_LocalSetupHandle, int CICPspeakerLayoutIdx, int *CICPspeakerIdx, int *hasKnownPosition, float *Azimuth, float *Elevation)
{
  /**
	*   Set a fixed loudspeaker layout (either CICPLayoutIdx or list of CICPspeakerIdx, plus eventially knownPositions)
	*   @param    h_LocalSetupHandle    : local setup handle
	*   @param    CICPspeakerLayoutIdx	: CICPspeakerLayoutIdx, -1 if none
  *   @param    CICPspeakerIdx		    : list of CICPspeakerIdx, NULL if none
  *   @param    hasKnownPosition		  : list of 0|1 that signal if a speaker has an associated known position
  *   @param    Azimuth		            : list of known azimuth values
  *   @param    Elevation		          : list of known elevation values
	*
	*   @return   ERROR_CODE
	*/
  int k = 0;
  int intPosition = -1;

  /* Flush h_LocalSetupHandle */
  if (h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerIdx != NULL)
  {
    free(h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerIdx); h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerIdx = NULL;
  }
  if (h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions != NULL)
  {
    for (k = 0; k < 3; k++)
    {
      if (h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[k] != NULL)
      {
        free(h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[k]); h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[k] = NULL;
      }
    }
    free(h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions); h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions = NULL;
  }
  if (h_LocalSetupHandle->LoudspeakerRenderingQuant->knownAzimuth != NULL)
  {
    free(h_LocalSetupHandle->LoudspeakerRenderingQuant->knownAzimuth); h_LocalSetupHandle->LoudspeakerRenderingQuant->knownAzimuth = NULL;
  }
  if (h_LocalSetupHandle->LoudspeakerRenderingQuant->knownElevation != NULL)
  {
    free(h_LocalSetupHandle->LoudspeakerRenderingQuant->knownElevation); h_LocalSetupHandle->LoudspeakerRenderingQuant->knownElevation = NULL;
  }

  h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->numSpeakers = h_LocalSetupHandle->LoudspeakerRendering->numSpeakers;
  if (CICPspeakerIdx != NULL)
  {
    h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType = 1;
    h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerIdx = (int*)calloc(h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->numSpeakers, sizeof(int));
    h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->numExplicitelySignaledSpeakers = -1;
    for (k = 0; k < h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->numSpeakers; k++)
    {
      h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerIdx[k] = CICPspeakerIdx[k];
    }
  }
  else
  {
    h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType = 0;
    h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerLayoutIdx = CICPspeakerLayoutIdx;
    h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->numExplicitelySignaledSpeakers = -1;
  }
  if (hasKnownPosition != NULL)
  {
    h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions = (float**)calloc(3,sizeof(float*));
    for (k = 0; k < 3; k++)
    {
      h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[k] = (float*)calloc(h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->numSpeakers, sizeof(float));
    }
    for (k = 0; k < h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->numSpeakers; k++)
    {
      if (hasKnownPosition[k] == 1)
      {
        intPosition++;
        h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[0][k] = 1;
        h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[1][k] = Azimuth[intPosition];
        h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[2][k] = Elevation[intPosition];
      }
      else
      {
        h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[0][k] = 0;
        h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[1][k] = 0.0f;
        h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[2][k] = 0.0f;
      }
    }
    h_LocalSetupHandle->LoudspeakerRenderingQuant->knownAzimuth = (int*)calloc(h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->numSpeakers, sizeof(int));
    h_LocalSetupHandle->LoudspeakerRenderingQuant->knownElevation = (int*)calloc(h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->numSpeakers, sizeof(int));
  }
  return 0;
}

int LS_setFlexibleLoudspeakerLayout(LOCAL_SETUP_HANDLE h_LocalSetupHandle, int *CICPspeakerIdx, float *Azimuth, float *Elevation, int *isLFE, int *alsoAddSymmetricPair, int numExplicitelySignaledSpeakers)
{
  /**
	*   Set a flexible loudspeaker layout (mixed between CICPspeakerIdx plus explicit positions plus the possibility to add symmetric speakers)
	*   @param    h_LocalSetupHandle                  : local setup handle
  *   @param    CICPspeakerIdx		                  : list of CICPspeakerIdx, NULL if none
  *   @param    Azimuth		                          : list of azimuth values
  *   @param    Elevation		                        : list of elevation values
  *   @param    isLFE		                            : list of (0|1) that signals if a speaker is an LFE
  *   @param    alsoAddSymmetricPair	              : list of (0|1) that signals if a symmetric speaker has to be added 
  *   @param    numExplicitelySignaledSpeakers		  : number of explicitely signaled speakers (number of total speakers - symmetric speakers)
	*
	*   @return   ERROR_CODE
	*/
  int k = 0;
  int flexibleIndex = -1;
  int numSpeakers = h_LocalSetupHandle->LoudspeakerRendering->numSpeakers;
  int angularPrecision = 0;
  int numCICPspeakers = 0;
  int numFlexibleSpeakers = 0;

  /* Flush h_LocalSetupHandle */
  if (h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerIdx != NULL)
  {
    free(h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerIdx); h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerIdx = NULL;
  }
  if (h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions != NULL)
  {
    for (k = 0; k < 3; k++)
    {
      if (h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[k] != NULL)
      {
        free(h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[k]); h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[k] = NULL;
      }
    }
    free(h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions); h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions = NULL;
  }
  if (h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isCICPspeakerIdx != NULL)
  {
    free(h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isCICPspeakerIdx);
  }
  if (h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->CICPspeakerIdx != NULL)
  {
    free(h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->CICPspeakerIdx);
  }
    if (h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isLFE != NULL)
  {
    free(h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isLFE);
  }
  if (h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationDirection != NULL)
  {
    free(h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationDirection);
  }
  if (h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationClass != NULL)
  {
    free(h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationClass);
  }
  if (h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationAngleIdx != NULL)
  {
    free(h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationAngleIdx);
  }
  if (h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Elevation != NULL)
  {
    free(h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Elevation);
  }
  if (h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthDirection != NULL)
  {
    free(h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthDirection);
  }
  if (h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx != NULL)
  {
    free(h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx);
  }
  if (h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Azimuth != NULL)
  {
    free(h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Azimuth);
  }
  if (h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->alsoAddSymmetricPair != NULL)
  {
    free(h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->alsoAddSymmetricPair);
  }

  h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->numSpeakers = numSpeakers;
  h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType = 2;
  h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerLayoutIdx = -1;
  h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->numExplicitelySignaledSpeakers = numExplicitelySignaledSpeakers;
  
  if (CICPspeakerIdx != NULL)
  {
    for (k = 0; k < numExplicitelySignaledSpeakers; k++)
    {
      if (CICPspeakerIdx[k] >= 0)
      {
        numCICPspeakers++;
      }
    }
    numFlexibleSpeakers = numExplicitelySignaledSpeakers - numCICPspeakers;
  }
  else
  {
    numCICPspeakers = 0;
    numFlexibleSpeakers = numExplicitelySignaledSpeakers;
  }


  if (Azimuth == NULL || Elevation == NULL)
  { 
    return -1;
  }
  else
  {
    for (k = 0; k < numFlexibleSpeakers; k++)
    {
      if (Azimuth[k] - (int)(Azimuth[k]) > 0)
      {
        angularPrecision = 1;
        break;
      }
      else
      {
        angularPrecision = (int)Azimuth[k] % 5;
      }
      if (angularPrecision != 0)
      {
        angularPrecision = 1;
        break;
      }

      if (Elevation[k] - (int)(Elevation[k]) > 0)
      {
        angularPrecision = 1;
        break;
      }
      else
      {
        angularPrecision = (int)Elevation[k] % 5;
      }
      if (angularPrecision != 0)
      {
        angularPrecision = 1;
        break;
      }
    }
    if (angularPrecision == 0)
    {
      angularPrecision = 5;
    }
  }

  h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->angularPrecision = angularPrecision;

  if (CICPspeakerIdx != NULL)
  {
    h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isCICPspeakerIdx = (int*)calloc(numExplicitelySignaledSpeakers, sizeof(int));
    h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->CICPspeakerIdx = (int*)calloc(numExplicitelySignaledSpeakers, sizeof(int));
  }
  
  h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isLFE = (int*)calloc(numExplicitelySignaledSpeakers, sizeof(int));
  h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationDirection = (int*)calloc(numExplicitelySignaledSpeakers, sizeof(int));
  h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationClass = (int*)calloc(numExplicitelySignaledSpeakers, sizeof(int));
  h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationAngleIdx = (int*)calloc(numExplicitelySignaledSpeakers, sizeof(int));
  h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Elevation = (float*)calloc(numExplicitelySignaledSpeakers, sizeof(float));
  h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthDirection = (int*)calloc(numExplicitelySignaledSpeakers, sizeof(int));
  h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx = (int*)calloc(numExplicitelySignaledSpeakers, sizeof(int));
  h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Azimuth = (float*)calloc(numExplicitelySignaledSpeakers, sizeof(float));
  h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->alsoAddSymmetricPair= (int*)calloc(numExplicitelySignaledSpeakers, sizeof(int));
  
  flexibleIndex = -1;

  for (k = 0; k < numExplicitelySignaledSpeakers; k++)
  {
    if (CICPspeakerIdx != NULL)
    {
      if (CICPspeakerIdx[k] >= 0)
      {
        h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isCICPspeakerIdx[k] = 1;
        h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->CICPspeakerIdx[k] = CICPspeakerIdx[k];
        h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationAngleIdx[k] = -1;
        h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationDirection[k] = -1;
        h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx[k] = -1;
        h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthDirection[k] = -1;
        h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationClass[k] = -1;
        h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isLFE[k] = -1;
        h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->alsoAddSymmetricPair[k] = alsoAddSymmetricPair[k]; 
      }
      else
      {
        flexibleIndex++;
        h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isCICPspeakerIdx[k] = 0;
        h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->CICPspeakerIdx[k] = -1;
        h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Azimuth[k] = Azimuth[flexibleIndex];
        h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Elevation[k] = Elevation[flexibleIndex];
        h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationClass[k] = 3;
        h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationAngleIdx[k] = (int)(fabs(floor(Elevation[flexibleIndex]+0.5f)) / (angularPrecision*1.0f));
        if (Elevation[k] >= 0)
        {
          h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationDirection[k] = 0;
        }
        else
        {
          h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationDirection[k] = 1;
        }
        h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx[k] = (int)(fabs(floor(Azimuth[flexibleIndex]+0.5f)) / (angularPrecision*1.0f));
        if (Azimuth[k] >= 0)
        {
          h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthDirection[k] = 0;
        }
        else
        {
          h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthDirection[k] = 1;
        }
        h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isLFE[k] = isLFE[flexibleIndex];
        h_LocalSetupHandle->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->alsoAddSymmetricPair[k] = alsoAddSymmetricPair[k];
      }
    }
  }
  return 0;
}


int LS_readBinauralRenderingBits(LOCAL_SETUP_HANDLE LocalSetup, robitbufHandle h_bitstream, unsigned long *noBitsRead, unsigned long *noBitsReadTotal)
{
  unsigned long ii;
	short i_representation;
	BinauralRepresentation *pNewBinauralRepresentation = NULL;

	unsigned long temp_uL;
  int pre = h_bitstream->bitsRead;
  int tempRend = -1;
  tempRend = robitbuf_ReadBits(h_bitstream, 1); /* read renderingType again */

  /* Read BinauralRendering "header" */
  LocalSetup->BinauralRendering->pBR->fileSignature = robitbuf_ReadBits(h_bitstream, 32); /* bsFileSignature */
	LocalSetup->BinauralRendering->pBR->fileVersion = (unsigned short) robitbuf_ReadBits(h_bitstream, 8); /* bsFileVersion */
	temp_uL = robitbuf_ReadBits(h_bitstream, 8); /* bsNumCharName */
	for (ii = 0; ii < temp_uL; ++ii)
	{
		LocalSetup->BinauralRendering->pBR->name[ii] = (char) robitbuf_ReadBits(h_bitstream, 8); /* bsName[ii] */
	}
	LocalSetup->BinauralRendering->pBR->name[ii] = '\0'; /* add end of string */
	LocalSetup->BinauralRendering->pBR->useTrackingMode = (unsigned short) robitbuf_ReadBits(h_bitstream, 1); /* useTrackingMode */
	LocalSetup->BinauralRendering->pBR->numBinauralRepresentation = (unsigned short) robitbuf_ReadBits(h_bitstream, 4); /* bsNumBinauralDataRepresentation */

	/* Read BinauralRepresentations */
	pNewBinauralRepresentation = BinauralRepresentation_initAlloc();
	for (i_representation = 0; 
		i_representation < LocalSetup->BinauralRendering->pBR->numBinauralRepresentation; 
		i_representation++)
	{
		temp_uL = robitbuf_ReadBits(h_bitstream, 5); /* brirSamplingFrequencyIndex */
		if (temp_uL == 0x1F)
		{
			pNewBinauralRepresentation->brirSamplingFrequency = robitbuf_ReadBits(h_bitstream, 24); /* brirSamplingFrequency */
		}
		else
		{
			pNewBinauralRepresentation->brirSamplingFrequency = brirSamplingFrequencyTable[temp_uL];
		}
		pNewBinauralRepresentation->isHoaData = (unsigned short) robitbuf_ReadBits(h_bitstream, 1); /* isHoaData */
		if (pNewBinauralRepresentation->isHoaData)
		{
			pNewBinauralRepresentation->hoaOrderBinaural = ReadEscapedValue(h_bitstream,3,5,0); /* hoaOrderBinaural  */
			pNewBinauralRepresentation->nBrirPairs = (pNewBinauralRepresentation->hoaOrderBinaural+1)*(pNewBinauralRepresentation->hoaOrderBinaural+1);
		}
		else
		{
			/* load Setup_SpeakerConfig3d */
			readSpeakerConfig3d(&pNewBinauralRepresentation->Setup_SpeakerConfig3d, h_bitstream);

			if (pNewBinauralRepresentation->Setup_SpeakerConfig3d.speakerLayoutType == 0)
			{
				pNewBinauralRepresentation->nBrirPairs = ReadEscapedValue(h_bitstream,5,8,0) + 1; /* nBrirPairs  */
			}
			else
			{
				if (pNewBinauralRepresentation->Setup_SpeakerConfig3d.numSpeakers > 32767)
				{
					fprintf(stderr,"\nError : nBrirPairs is a signed 16-bit short number <= 32767");
					return -1;
				}
				pNewBinauralRepresentation->nBrirPairs = (short) pNewBinauralRepresentation->Setup_SpeakerConfig3d.numSpeakers;
			}
		}

		pNewBinauralRepresentation->binauralDataFormatID = (short) robitbuf_ReadBits(h_bitstream, 2); /* binauralDataFormatID */
		robitbuf_ByteAlign(h_bitstream);

		/* check nBrirPairs */
		if (pNewBinauralRepresentation->nBrirPairs > MAX_NUM_BRIR_PAIRS)
		{
			fprintf(stderr,"\nError : nBrirPairs should be <= %d", MAX_NUM_BRIR_PAIRS);
			return -1;
		}
		if ( (pNewBinauralRepresentation->isHoaData == 0) && 
			 (pNewBinauralRepresentation->nBrirPairs > CICP2GEOMETRY_MAX_LOUDSPEAKERS) )
		{
			fprintf(stderr,"\nError : nBrirPairs should be <= %d for channel-based configurations !", CICP2GEOMETRY_MAX_LOUDSPEAKERS);
			return -1;
		}

		switch (pNewBinauralRepresentation->binauralDataFormatID)
		{
		case 0:  /* 0 : raw FIR */     
			if ( readBinauralFirData(
				pNewBinauralRepresentation->pBinauralFirData, 
				pNewBinauralRepresentation->nBrirPairs,
				h_bitstream) != 0) return -1;
			break;

		case 1: /* 1 : FD parameters */      
			if ( readFdBinauralRendererParam(
				pNewBinauralRepresentation->pFdBinauralRendererParam, 
				pNewBinauralRepresentation->nBrirPairs,
				h_bitstream) != 0) return -1;  
			break;

		case 2: /* 2 : TD parameters*/
			if ( readTdBinauralRendererParam(
				pNewBinauralRepresentation->pTdBinauralRendererParam, 
				pNewBinauralRepresentation->nBrirPairs,
				h_bitstream) != 0) return -1;
			break;

		default:
			fprintf(stderr,"\nError : Unknown binauralDataFormatID");
			return -1;

		}

		/* set representation in BinauralRendering */
		if ( BinauralRendering_setBinauralRepresentation(
			LocalSetup->BinauralRendering->pBR, 
			pNewBinauralRepresentation,
			i_representation) != 0) return -1;
	}



  robitbuf_ByteAlign(h_bitstream);

  *noBitsRead = h_bitstream->bitsRead - pre;
  *noBitsReadTotal += h_bitstream->bitsRead - pre;

  return 0;
}

int LS_readLoudspeakerRenderingBits(LOCAL_SETUP_HANDLE LocalSetup, H_LS_BITSTREAM_DECODER h_bitstream, unsigned long *noBitsRead, unsigned long *noBitsReadTotal)
{
  /**
	*   Reading the loudspeaker rendering data (LoudspeakerRendering() syntax) from the bitstream
	*   @param    LocalSetup        : local setup handle
	*   @param    h_bitstream	      : bitstream handle
  *   @param    no_BitsRead		    : output parameter, number of read bits
  *   @param    noBitsReadTotal		: output parameter, number of total read bits, adds the number of read bits to the previous value of the variable
	*
	*   @return   ERROR_CODE
	*/
  unsigned long no_BitsReadTotal = 0;
  int k = 0;
  int tempNumExplicitelySignaledSpeakers = 0;
  unsigned int temp;

  /* flush buffer */
  if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerIdx != NULL)
  {
    free(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerIdx); LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerIdx = NULL;
  }
  if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->alsoAddSymmetricPair != NULL)
  {
    free(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->alsoAddSymmetricPair); LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->alsoAddSymmetricPair = NULL;
  }
  if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Azimuth != NULL)
  {
    free(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Azimuth); LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Azimuth = NULL;
  }
  if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx != NULL)
  {
    free(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx); LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx = NULL;
  }
  if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthDirection != NULL)
  {
    free(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthDirection); LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthDirection = NULL;
  }
  if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->CICPspeakerIdx != NULL)
  {
    free(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->CICPspeakerIdx); LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->CICPspeakerIdx = NULL;
  }
  if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Elevation != NULL)
  {
    free(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Elevation); LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Elevation = NULL;
  }
  if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationAngleIdx != NULL)
  {
    free(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationAngleIdx); LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationAngleIdx = NULL;
  }
  if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationClass != NULL)
  {
    free(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationClass); LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationClass = NULL;
  }
  if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationDirection != NULL)
  {
    free(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationDirection); LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationDirection = NULL;
  }
  if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isCICPspeakerIdx != NULL)
  {
    free(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isCICPspeakerIdx); LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isCICPspeakerIdx = NULL;
  }
  if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isLFE != NULL)
  {
    free(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isLFE); LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isLFE = NULL;
  }
  if (LocalSetup->LoudspeakerRendering->loudspeakerDistance != NULL)
  {
    free(LocalSetup->LoudspeakerRendering->loudspeakerDistance); LocalSetup->LoudspeakerRendering->loudspeakerDistance = NULL;
  }
  if (LocalSetup->LoudspeakerRenderingQuant->loudspeakerCalibrationGain != NULL)
  {
    free(LocalSetup->LoudspeakerRenderingQuant->loudspeakerCalibrationGain); LocalSetup->LoudspeakerRenderingQuant->loudspeakerCalibrationGain = NULL;
  }
  if (LocalSetup->LoudspeakerRendering->loudspeakerCalibrationGain != NULL)
  {
    free(LocalSetup->LoudspeakerRendering->loudspeakerCalibrationGain); LocalSetup->LoudspeakerRendering->loudspeakerCalibrationGain = NULL;
  }
  if (LocalSetup->LoudspeakerRenderingQuant->knownAzimuth != NULL)
  {
    free(LocalSetup->LoudspeakerRenderingQuant->knownAzimuth); LocalSetup->LoudspeakerRenderingQuant->knownAzimuth = NULL;
  }
  if (LocalSetup->LoudspeakerRenderingQuant->knownElevation != NULL)
  {
    free(LocalSetup->LoudspeakerRenderingQuant->knownElevation); LocalSetup->LoudspeakerRenderingQuant->knownElevation = NULL;
  }
  if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions != NULL)
  {
    for (k = 0; k < 3; k++)
    {
      if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[k] != NULL)
      {
        free(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[k]); LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[k] = NULL;
      }
    }
    free(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions); LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions = NULL;
  }

  no_BitsReadTotal += LS_DecInt_readBits(h_bitstream,&temp,16);
  LocalSetup->LoudspeakerRendering->numSpeakers = (int)temp;

  tempNumExplicitelySignaledSpeakers = LocalSetup->LoudspeakerRendering->numSpeakers;

  /* Setup_SpeakerConfig3d */
  no_BitsReadTotal += LS_DecInt_readBits(h_bitstream,&temp,2);
  LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType = (int)temp;
  if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType == 0)
  {
    no_BitsReadTotal += LS_DecInt_readBits(h_bitstream,&temp,6);
    LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerLayoutIdx = (int)temp;
    LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->numSpeakers = tempNumExplicitelySignaledSpeakers;
    LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->numExplicitelySignaledSpeakers = -1;
  }
  else if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType < 3)
  {
    no_BitsReadTotal += LS_DecInt_readEscapedValue(h_bitstream,&temp,5,8,16);
    LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->numSpeakers = (int)temp;

    if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType == 1)
    {
      LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->numSpeakers = tempNumExplicitelySignaledSpeakers;
      LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerIdx = (int*)calloc(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->numSpeakers,sizeof(int));
      for (k = 0; k < LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->numSpeakers; k++)
      {
        no_BitsReadTotal += LS_DecInt_readBits(h_bitstream,&temp,7);
        LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerIdx[k] = (int)temp;
      }
      LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->numExplicitelySignaledSpeakers = -1;
    }
    else if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType == 2)
    {
      unsigned int temp2;
      LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->alsoAddSymmetricPair = (int*)calloc(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->numSpeakers,sizeof(int));
      LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Azimuth = (float*)calloc(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->numSpeakers,sizeof(float));
      LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx = (int*)calloc(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->numSpeakers,sizeof(int));
      LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthDirection = (int*)calloc(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->numSpeakers,sizeof(int));
      LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->CICPspeakerIdx = (int*)calloc(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->numSpeakers,sizeof(int));
      LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Elevation = (float*)calloc(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->numSpeakers,sizeof(float));
      LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationAngleIdx= (int*)calloc(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->numSpeakers,sizeof(int));
      LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationClass = (int*)calloc(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->numSpeakers,sizeof(int));
      LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationDirection = (int*)calloc(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->numSpeakers,sizeof(int));
      LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isCICPspeakerIdx = (int*)calloc(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->numSpeakers,sizeof(int));
      LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isLFE = (int*)calloc(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->numSpeakers,sizeof(int));

      no_BitsReadTotal += LS_DecInt_readBits(h_bitstream,&temp2,1);
      if (temp2 == 0)
      {
        LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->angularPrecision = 5;
      }
      else
      {
        LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->angularPrecision = 1;
      }
      for (k = 0; k < LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->numSpeakers; k++)
      {
        no_BitsReadTotal += LS_DecInt_readBits(h_bitstream,&temp,1);
        LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isCICPspeakerIdx[k] = (int)temp;
        if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isCICPspeakerIdx[k] == 1)
        {
          no_BitsReadTotal += LS_DecInt_readBits(h_bitstream,&temp,7);
          LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->CICPspeakerIdx[k] = (int)temp;
        }
        else
        {
          no_BitsReadTotal += LS_DecInt_readBits(h_bitstream,&temp,2);
          LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationClass[k] = (int)temp;
          if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationClass[k] == 3)
          {
            if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->angularPrecision == 5)
            {
              no_BitsReadTotal += LS_DecInt_readBits(h_bitstream,&temp,5);
              LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationAngleIdx[k] = (int)temp;
            }
            else if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->angularPrecision == 1)
            {
              no_BitsReadTotal += LS_DecInt_readBits(h_bitstream,&temp,7);
              LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationAngleIdx[k] = (int)temp;
            }
            if(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationAngleIdx[k] != 0)
            {
              no_BitsReadTotal += LS_DecInt_readBits(h_bitstream,&temp,1);
              LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationDirection[k] = (int)temp;
            }
            else
            {
              LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationDirection[k] = -1;
            }
          }
          if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->angularPrecision == 5)
          {
            no_BitsReadTotal += LS_DecInt_readBits(h_bitstream,&temp,6);
            LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx[k] = (int)temp;
          }
          else if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->angularPrecision == 1)
          {
            no_BitsReadTotal += LS_DecInt_readBits(h_bitstream,&temp,8);
            LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx[k] = (int)temp;
          }
          if ((LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx[k] * LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->angularPrecision != 0) && (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx[k] * LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->angularPrecision != 180))
          {
            no_BitsReadTotal += LS_DecInt_readBits(h_bitstream,&temp,1);
            LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthDirection[k] = (int)temp;
          }
          else
          {
            LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthDirection[k] = -1;
          }
          no_BitsReadTotal += LS_DecInt_readBits(h_bitstream,&temp,1);
          LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isLFE[k] = (int)temp;
        }
        /* also add symmetric pair */
        if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isCICPspeakerIdx[k] == 1)
        {
          if ((LS_getAzimuthFromCICPspeakerIdx(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->CICPspeakerIdx[k]) != 0) && (LS_getAzimuthFromCICPspeakerIdx(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->CICPspeakerIdx[k]) != 180))
          {
            no_BitsReadTotal += LS_DecInt_readBits(h_bitstream,&temp,1);
            LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->alsoAddSymmetricPair[k] = (int)temp;
            if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->alsoAddSymmetricPair[k] == 1)
            {
              int symmetricCICP = LS_getSymmetricCICPspeakerIdx(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->CICPspeakerIdx[k]);
              k++;
              LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isCICPspeakerIdx[k] = 1;
              LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->alsoAddSymmetricPair[k] = -1;
              if (symmetricCICP != -1)
              {
                LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->CICPspeakerIdx[k] = symmetricCICP;
              }
              tempNumExplicitelySignaledSpeakers--;
            }
          }
        }
        else
        {
          if ((LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx[k] * LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->angularPrecision != 0) && (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx[k] * LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->angularPrecision != 180))
          {
            no_BitsReadTotal += LS_DecInt_readBits(h_bitstream,&temp,1);
            LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->alsoAddSymmetricPair[k] = (int)temp;
            if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->alsoAddSymmetricPair[k] == 1)
            {
              k++;
              LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isCICPspeakerIdx[k] = 0;
              LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->CICPspeakerIdx[k] = -1;
              LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->alsoAddSymmetricPair[k] = -1;
              
              LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx[k] = LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx[k-1];
              LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthDirection[k] = !(LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthDirection[k-1]);
              LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationAngleIdx[k] = LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationAngleIdx[k-1];
              LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationClass[k] = LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationClass[k-1];
              LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationDirection[k] = LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationDirection[k-1];
              LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isLFE[k] = LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isLFE[k-1];
              tempNumExplicitelySignaledSpeakers--;
            }
          }
        }
      }
    }
  }
  no_BitsReadTotal += LS_DecInt_readBits(h_bitstream,&temp,1);
  if (temp == 1) /* hasLoudspeakerDistance */
  {
    LocalSetup->LoudspeakerRendering->loudspeakerDistance = (int*)calloc(LocalSetup->LoudspeakerRendering->numSpeakers, sizeof(int));
  }
  no_BitsReadTotal += LS_DecInt_readBits(h_bitstream,&temp,1);
  if (temp == 1) /* hasLoudspeakerCalibrationGain */
  {
    LocalSetup->LoudspeakerRenderingQuant->loudspeakerCalibrationGain = (int*)calloc(LocalSetup->LoudspeakerRendering->numSpeakers, sizeof(int));
    LocalSetup->LoudspeakerRendering->loudspeakerCalibrationGain = (float*)calloc(LocalSetup->LoudspeakerRendering->numSpeakers, sizeof(float));
  }

  no_BitsReadTotal += LS_DecInt_readBits(h_bitstream,&temp,1);
  if (temp == 1) /* useTrackingMode */
  {
    LocalSetup->LoudspeakerRendering->useTrackingMode = 1;
  }

  LocalSetup->LoudspeakerRenderingQuant->knownAzimuth = (int*)calloc(LocalSetup->LoudspeakerRendering->numSpeakers, sizeof(int));
  LocalSetup->LoudspeakerRenderingQuant->knownElevation = (int*)calloc(LocalSetup->LoudspeakerRendering->numSpeakers, sizeof(int));
  LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions = (float**)calloc(3, sizeof(float*));
  for (k = 0; k < 3; k++)
  {
    LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[k] = (float*)calloc(LocalSetup->LoudspeakerRendering->numSpeakers, sizeof(float));
  }
  
  for (k = 0; k < LocalSetup->LoudspeakerRendering->numSpeakers; k++)
  {
    if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType <= 1)
    {
      no_BitsReadTotal += LS_DecInt_readBits(h_bitstream, &temp,1);
      LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[0][k] = (float)temp;
      if ((int)LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[0][k] == 1)
      {
        no_BitsReadTotal += LS_DecInt_readBits(h_bitstream, &temp,9);
        LocalSetup->LoudspeakerRenderingQuant->knownAzimuth[k] = (int)temp;
        no_BitsReadTotal += LS_DecInt_readBits(h_bitstream, &temp,8);
        LocalSetup->LoudspeakerRenderingQuant->knownElevation[k] = (int)temp;
      }
    }
    else
    {
      LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[0][k] = -1.0;
    }
    if (LocalSetup->LoudspeakerRendering->loudspeakerDistance != NULL)
    {
      no_BitsReadTotal += LS_DecInt_readBits(h_bitstream, &temp,10);
      LocalSetup->LoudspeakerRendering->loudspeakerDistance[k] = (int)temp;
    }
    if (LocalSetup->LoudspeakerRendering->loudspeakerCalibrationGain != NULL)
    {
      no_BitsReadTotal += LS_DecInt_readBits(h_bitstream, &temp,7);
      LocalSetup->LoudspeakerRenderingQuant->loudspeakerCalibrationGain[k] = (int)temp;
    }
  }
  no_BitsReadTotal += LS_DecInt_readBits(h_bitstream, &temp,1);
  LocalSetup->LoudspeakerRendering->externalDistanceCompensation = (int)temp;

  *noBitsRead = no_BitsReadTotal;
  *noBitsReadTotal += no_BitsReadTotal;
  if (LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType == 2)
  {
    LocalSetup->LoudspeakerRendering->Setup_SpeakerConfig3D->numExplicitelySignaledSpeakers = tempNumExplicitelySignaledSpeakers;
  }
  return 0;
}