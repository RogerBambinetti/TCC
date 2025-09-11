#include "initRenderPaths.h"

#include <stdio.h>
#include <math.h>

#define MAX_RESAMPLING_DELAY (256)
#define MAX_LOW_PROFILE_DELAY_WITHOUT_FORMAT_CONVERTER


extern PROFILE profile;

extern PROCESSING_DOMAIN mixerDomain;

extern int GetCoreCoderConstantDelay (void);

extern float resamplingRatio;

extern int bResamplerActive;

static int _setInitialValues ( 
                   DOMAIN_SWITCHING_INFO      *domainSwitchingDecisions, 
                   MODULE_PROCESSING_DOMAINS  *moduleProcessingDomains
                   );

static int _decideProcessingDomains (
                   DECODER_PROPERTIES         *decoderProperties,
                   MODULE_PROCESSING_DOMAINS  *moduleProcessingDomains,
                   DOMAIN_SWITCHING_INFO      *domainSwitchingDecision
                   );

  static int _decideProcessingDomains_Rendering_TimeCore (
                   DECODER_PROPERTIES         *decoderProperties,
                   MODULE_PROCESSING_DOMAINS  *moduleProcessingDomains,
                   DOMAIN_SWITCHING_INFO      *domainSwitchingDecision
                   );

  static int _decideProcessingDomains_Rendering_QMFCore (
                   DECODER_PROPERTIES         *decoderProperties,
                   MODULE_PROCESSING_DOMAINS  *moduleProcessingDomains,
                   DOMAIN_SWITCHING_INFO      *domainSwitchingDecision
                   );
  
  static int _decideProcessingDomains_PostProcessing (
                   DECODER_PROPERTIES         *decoderProperties,
                   MODULE_PROCESSING_DOMAINS  *moduleProcessingDomains,
                   DOMAIN_SWITCHING_INFO      *domainSwitchingDecision
                   );



static int _calculateRendererPathDelays( 
                                  DECODER_PROPERTIES         *decoderProperties,
                                  DOMAIN_SWITCHING_INFO      *domainSwitchingDecisions, 
                                  MODULE_PROCESSING_DOMAINS  *moduleProcessingDomains,
                                  RENDERER_PATH_DELAYS       *rendererPathDelays
                                  );

static int _getDomainSwitchDelay( DOMAIN_SWITCH domSwitch );

static int _getProfileLevelRendererDelay( PROFILE profile, int receiverDelayCompensationBit, RENDERER_PATH_DELAYS *rendererPathDelays );


int _setInitialValues ( 
                   DOMAIN_SWITCHING_INFO      *domainSwitchingDecisions, 
                   MODULE_PROCESSING_DOMAINS  *moduleProcessingDomains
                   )
{
  /************************************************
                 INIT DOMAINSWITCHING
  ************************************************/ 
  
  domainSwitchingDecisions->channelDomains.PreDrc1              = KEEP_DOMAIN;
  domainSwitchingDecisions->channelDomains.PostDrc1             = KEEP_DOMAIN;
  domainSwitchingDecisions->channelDomains.PreFormatConverter   = KEEP_DOMAIN;
  domainSwitchingDecisions->channelDomains.PostFormatConverter  = KEEP_DOMAIN;

  domainSwitchingDecisions->objectDomains.PreDrc1               = KEEP_DOMAIN;
  domainSwitchingDecisions->objectDomains.PostDrc1              = KEEP_DOMAIN;
  domainSwitchingDecisions->objectDomains.PreObjectRendering    = KEEP_DOMAIN;
  domainSwitchingDecisions->objectDomains.PostObjectRendering   = KEEP_DOMAIN;

  domainSwitchingDecisions->saocDomains.PreSaocRendering        = KEEP_DOMAIN;
  domainSwitchingDecisions->saocDomains.PostSaocRendering       = KEEP_DOMAIN;

  domainSwitchingDecisions->hoaDomains.PreHoaRendering          = KEEP_DOMAIN;
  domainSwitchingDecisions->hoaDomains.PostHoaRendering         = KEEP_DOMAIN;

  domainSwitchingDecisions->binauralDomains.PreFdBinaural       = KEEP_DOMAIN;
  domainSwitchingDecisions->binauralDomains.PostFdBinaural      = KEEP_DOMAIN;

  domainSwitchingDecisions->binauralDomains.PreTdBinaural       = KEEP_DOMAIN;
  domainSwitchingDecisions->binauralDomains.PostTdBinaural      = KEEP_DOMAIN;

  domainSwitchingDecisions->binauralDomains.PreHoaBinaural      = KEEP_DOMAIN;
  domainSwitchingDecisions->binauralDomains.PostHoaBinaural     = KEEP_DOMAIN;

  domainSwitchingDecisions->binauralDomains.PostBinauralDrc2    = KEEP_DOMAIN;

  /************************************************
                INIT PROCESSING DOMAINS
  ************************************************/ 

  moduleProcessingDomains->coreCoder                            = NOT_DEFINED;

  moduleProcessingDomains->drc1channels                         = NOT_DEFINED;
  moduleProcessingDomains->formatConverter                      = NOT_DEFINED;

  moduleProcessingDomains->drc1objects                          = NOT_DEFINED;
  moduleProcessingDomains->objectRenderer                       = NOT_DEFINED;

  moduleProcessingDomains->saocRenderer                         = NOT_DEFINED;

  moduleProcessingDomains->hoaRendererInput                     = NOT_DEFINED;
  moduleProcessingDomains->hoaRendererOutput                    = NOT_DEFINED;

  moduleProcessingDomains->mixer                                = NOT_DEFINED;

  moduleProcessingDomains->drc2channels                         = NOT_DEFINED;

  moduleProcessingDomains->fdBinaural                           = NOT_DEFINED;
  moduleProcessingDomains->tdBinaural                           = NOT_DEFINED;
  moduleProcessingDomains->hoaBinaural                          = NOT_DEFINED;
  moduleProcessingDomains->drc2binaural                         = NOT_DEFINED;
  moduleProcessingDomains->drc3channel                          = NOT_DEFINED;
  moduleProcessingDomains->loudnessNorm                         = NOT_DEFINED;
  moduleProcessingDomains->peakLimiter                          = NOT_DEFINED;

  return 0;
}


int _decideProcessingDomains (
                   DECODER_PROPERTIES         *decoderProperties,
                   MODULE_PROCESSING_DOMAINS  *moduleProcessingDomains,
                   DOMAIN_SWITCHING_INFO      *domainSwitchingDecision
                   )
{
  int error = 0; 
    /************************************************
                    CORE DOMAIN
  ************************************************/

  if ( decoderProperties->bCoreHasSbr == 0 ) {
    moduleProcessingDomains->coreCoder = TIME_DOMAIN;
  }
  else {
    moduleProcessingDomains->coreCoder = QMF_DOMAIN;
  }

  /************************************************
                    MIXER DOMAIN
  ************************************************/

  moduleProcessingDomains->mixer = mixerDomain;

  if( moduleProcessingDomains->coreCoder == TIME_DOMAIN )
  {
    error = _decideProcessingDomains_Rendering_TimeCore ( decoderProperties, moduleProcessingDomains, domainSwitchingDecision );
    if ( error ) return error;
  } else {
    error = _decideProcessingDomains_Rendering_QMFCore ( decoderProperties, moduleProcessingDomains, domainSwitchingDecision );
    if ( error ) return error;
  }

  error = _decideProcessingDomains_PostProcessing( decoderProperties, moduleProcessingDomains, domainSwitchingDecision );
  if ( error ) return error;

  return error;
}


int _decideProcessingDomains_Rendering_TimeCore (
                   DECODER_PROPERTIES         *decoderProperties,
                   MODULE_PROCESSING_DOMAINS  *moduleProcessingDomains,
                   DOMAIN_SWITCHING_INFO      *domainSwitchingDecisions
                   )
{
  int error = 0;

  /* channel domains */
  if ( decoderProperties->bCoreChannels ) {

    PROCESSING_DOMAIN currentProcessingDomain_Channels = TIME_DOMAIN;

    if ( decoderProperties->renderingComponents.modeDrc1Channels == MULTIBAND ) {
      domainSwitchingDecisions->channelDomains.PreDrc1 = STFT_ANALYSIS;
      currentProcessingDomain_Channels                = STFT_DOMAIN;
      moduleProcessingDomains->drc1channels            = currentProcessingDomain_Channels;

      if ( decoderProperties->renderingComponents.bFormatConversionChannels ) {
        moduleProcessingDomains->formatConverter       = currentProcessingDomain_Channels;
      }

    }
    else if ( decoderProperties->renderingComponents.modeDrc1Channels == SINGLEBAND ) {
      moduleProcessingDomains->drc1channels            = currentProcessingDomain_Channels;
    }

    if ( decoderProperties->renderingComponents.bFormatConversionChannels && 
         currentProcessingDomain_Channels == TIME_DOMAIN ) {
      domainSwitchingDecisions->channelDomains.PreFormatConverter = STFT_ANALYSIS;
      currentProcessingDomain_Channels                           = STFT_DOMAIN;
      moduleProcessingDomains->formatConverter                    = currentProcessingDomain_Channels;
    }

    if ( moduleProcessingDomains->mixer == TIME_DOMAIN && currentProcessingDomain_Channels == STFT_DOMAIN ) {

      if ( decoderProperties->renderingComponents.modeDrc1Channels == MULTIBAND &&
           decoderProperties->renderingComponents.bFormatConversionChannels ) {

        domainSwitchingDecisions->channelDomains.PostDrc1 = STFT_SYNTHESIS;
        currentProcessingDomain_Channels                            = TIME_DOMAIN;

      }
      else if ( decoderProperties->renderingComponents.modeDrc1Channels == MULTIBAND ) {

        domainSwitchingDecisions->channelDomains.PostDrc1 = STFT_SYNTHESIS;
        currentProcessingDomain_Channels                 = TIME_DOMAIN;

      }
      else if ( decoderProperties->renderingComponents.bFormatConversionChannels ) {

        domainSwitchingDecisions->channelDomains.PostFormatConverter = STFT_SYNTHESIS;
        currentProcessingDomain_Channels                            = TIME_DOMAIN;

      }

    }

    if ( moduleProcessingDomains->mixer == QMF_DOMAIN ) {

      if ( !decoderProperties->renderingComponents.modeDrc1Channels == MULTIBAND &&
           !decoderProperties->renderingComponents.bFormatConversionChannels ) {

        if ( decoderProperties->renderingComponents.modeDrc1Channels == SINGLEBAND ) {
          domainSwitchingDecisions->channelDomains.PostDrc1 = QMF_ANALYSIS;
          currentProcessingDomain_Channels                 = QMF_DOMAIN;
        }
        else {
          domainSwitchingDecisions->channelDomains.PostCore = QMF_ANALYSIS;
          currentProcessingDomain_Channels                 = QMF_DOMAIN;
        }

      }

    }

    if ( moduleProcessingDomains->mixer != currentProcessingDomain_Channels ) {
      fprintf(stderr, "Filterbank rule set error: processing domain after channel rendering is not matching mixer processing domain!\n");
      error = -1;
      return error;
    }

  }

  /* object domains */
  if ( decoderProperties->renderingComponents.bObjectRendering ) {

    PROCESSING_DOMAIN currentProcessingDomain_Objects  = TIME_DOMAIN;

    if ( decoderProperties->renderingComponents.modeDrc1Objects == MULTIBAND ) {

      domainSwitchingDecisions->objectDomains.PreDrc1    = STFT_ANALYSIS;
      currentProcessingDomain_Objects                    = STFT_DOMAIN;
      moduleProcessingDomains->drc1objects               = currentProcessingDomain_Objects;
      domainSwitchingDecisions->objectDomains.PostDrc1   = STFT_SYNTHESIS;
      currentProcessingDomain_Objects                    = TIME_DOMAIN;
      moduleProcessingDomains->objectRenderer            = currentProcessingDomain_Objects;
    }
    else if ( decoderProperties->renderingComponents.modeDrc1Objects == SINGLEBAND ) {
      moduleProcessingDomains->drc1objects            = currentProcessingDomain_Objects;
      moduleProcessingDomains->objectRenderer         = currentProcessingDomain_Objects;
    }
    else {
      /* Unreachable!?! */
      moduleProcessingDomains->objectRenderer         = currentProcessingDomain_Objects;

    }

    if ( moduleProcessingDomains->mixer == TIME_DOMAIN && currentProcessingDomain_Objects == STFT_DOMAIN ) {

      domainSwitchingDecisions->objectDomains.PostObjectRendering = QMF_SYNTHESIS;
      currentProcessingDomain_Objects                            = TIME_DOMAIN;

    }

    if ( moduleProcessingDomains->mixer != currentProcessingDomain_Objects ) {
      fprintf(stderr, "Filterbank rule set error: processing domain after object rendering is not matching mixer processing domain!\n");
      error = -1;
      return error;
    }

  }


  if ( decoderProperties->renderingComponents.bSaocRendering ) {

    PROCESSING_DOMAIN currentProcessingDomain_Saoc         = TIME_DOMAIN;

    domainSwitchingDecisions->saocDomains.PreSaocRendering = QMF_ANALYSIS;
    currentProcessingDomain_Saoc                           = QMF_DOMAIN;
    moduleProcessingDomains->saocRenderer                  = currentProcessingDomain_Saoc;

    if ( moduleProcessingDomains->mixer == TIME_DOMAIN ) {
      domainSwitchingDecisions->saocDomains.PostSaocRendering = QMF_SYNTHESIS;
      currentProcessingDomain_Saoc                           = TIME_DOMAIN;
    }

    if ( moduleProcessingDomains->mixer != currentProcessingDomain_Saoc ) {
      fprintf(stderr, "Filterbank rule set error: processing domain after SAOC rendering is not matching mixer processing domain!\n");
      error = -1;
      return error;
    }

  }



  if ( decoderProperties->renderingComponents.bHoaRendering ) {

    PROCESSING_DOMAIN currentProcessingDomain_Hoa       = TIME_DOMAIN;

    if ( decoderProperties->renderingComponents.modeDrc1Hoa == MULTIBAND ) {

      domainSwitchingDecisions->hoaDomains.PreDrc1  = STFT_ANALYSIS;
      currentProcessingDomain_Hoa                   = STFT_DOMAIN;
      moduleProcessingDomains->drc1hoa              = currentProcessingDomain_Hoa;
      domainSwitchingDecisions->hoaDomains.PostDrc1 = STFT_SYNTHESIS;
      currentProcessingDomain_Hoa                   = TIME_DOMAIN;
      moduleProcessingDomains->hoaRendererInput     = currentProcessingDomain_Hoa;
      moduleProcessingDomains->hoaRendererOutput    = currentProcessingDomain_Hoa;
    }
    else if ( decoderProperties->renderingComponents.modeDrc1Hoa == SINGLEBAND ) {
      moduleProcessingDomains->drc1hoa           = currentProcessingDomain_Hoa;
      moduleProcessingDomains->hoaRendererInput  = currentProcessingDomain_Hoa;
      moduleProcessingDomains->hoaRendererOutput = currentProcessingDomain_Hoa;
    }
    
    if ( moduleProcessingDomains->mixer != currentProcessingDomain_Hoa ) {
      fprintf(stderr, "Filterbank rule set error: processing domain after HOA rendering is not matching mixer processing domain!\n");
      error = -1;
      return error;
    }

  }

  return error;

}



int _decideProcessingDomains_Rendering_QMFCore (
                   DECODER_PROPERTIES         *decoderProperties,
                   MODULE_PROCESSING_DOMAINS  *moduleProcessingDomains,
                   DOMAIN_SWITCHING_INFO      *domainSwitchingDecisions
                   )
{
  int error = 0;

  if ( decoderProperties->bCoreChannels ) {

    PROCESSING_DOMAIN currentProcessingDomain_Channels = QMF_DOMAIN;

    if ( decoderProperties->renderingComponents.modeDrc1Channels != DISABLED ) {
      moduleProcessingDomains->drc1channels    = currentProcessingDomain_Channels;
    }

    if ( decoderProperties->renderingComponents.bFormatConversionChannels ) {
      moduleProcessingDomains->formatConverter = currentProcessingDomain_Channels;
    }

    if ( moduleProcessingDomains->mixer == TIME_DOMAIN ) {

      if ( decoderProperties->renderingComponents.modeDrc1Channels == MULTIBAND && 
           decoderProperties->renderingComponents.bFormatConversionChannels ) {
        domainSwitchingDecisions->channelDomains.PostFormatConverter = QMF_SYNTHESIS;
        currentProcessingDomain_Channels                            = TIME_DOMAIN;
      }
      else if ( decoderProperties->renderingComponents.modeDrc1Channels == MULTIBAND ) {
        domainSwitchingDecisions->channelDomains.PostDrc1 = QMF_SYNTHESIS;
        currentProcessingDomain_Channels                 = TIME_DOMAIN;
      }
      else if ( decoderProperties->renderingComponents.bFormatConversionChannels ) {
        domainSwitchingDecisions->channelDomains.PostFormatConverter = QMF_SYNTHESIS;
        currentProcessingDomain_Channels                            = TIME_DOMAIN;
      }
      else {
        domainSwitchingDecisions->channelDomains.PostCore = QMF_SYNTHESIS;
        currentProcessingDomain_Channels                 = TIME_DOMAIN;
      }
    }

    if ( moduleProcessingDomains->mixer != currentProcessingDomain_Channels ) {
      fprintf(stderr, "Filterbank rule set error: processing domain after channel rendering is not matching mixer processing domain!\n");
      error = -1;
      return error;
    }

  }

  if ( decoderProperties->renderingComponents.bObjectRendering ) {

    PROCESSING_DOMAIN currentProcessingDomain_Objects = QMF_DOMAIN;

    if ( decoderProperties->renderingComponents.modeDrc1Objects != DISABLED ) {
      moduleProcessingDomains->drc1objects    = currentProcessingDomain_Objects;
    }

    moduleProcessingDomains->objectRenderer   = currentProcessingDomain_Objects;

    if ( moduleProcessingDomains->mixer == TIME_DOMAIN ) {
      domainSwitchingDecisions->objectDomains.PostObjectRendering = QMF_SYNTHESIS;
      currentProcessingDomain_Objects                            = TIME_DOMAIN;
    }

    if ( moduleProcessingDomains->mixer != currentProcessingDomain_Objects ) {
      fprintf(stderr, "Filterbank rule set error: processing domain after object rendering is not matching mixer processing domain!\n");
      error = -1;
      return error;
    }

  }

  if ( decoderProperties->renderingComponents.bSaocRendering ) {

    PROCESSING_DOMAIN currentProcessingDomain_Saoc     = QMF_DOMAIN;

    moduleProcessingDomains->saocRenderer = currentProcessingDomain_Saoc;

    if ( moduleProcessingDomains->mixer == TIME_DOMAIN ) {
      domainSwitchingDecisions->saocDomains.PostSaocRendering = QMF_SYNTHESIS;
      currentProcessingDomain_Saoc = TIME_DOMAIN;
    }

    if ( moduleProcessingDomains->mixer != currentProcessingDomain_Saoc ) {
      fprintf(stderr, "Filterbank rule set error: processing domain after SAOC rendering is not matching mixer processing domain!\n");
      error = -1;
      return error;
    }


  }

  if ( decoderProperties->renderingComponents.bHoaRendering ) {

    PROCESSING_DOMAIN currentProcessingDomain_Hoa       = QMF_DOMAIN;

    domainSwitchingDecisions->hoaDomains.PreHoaRendering = QMF_SYNTHESIS;

    currentProcessingDomain_Hoa = TIME_DOMAIN;
    moduleProcessingDomains->hoaRendererInput = currentProcessingDomain_Hoa;


    if ( moduleProcessingDomains->mixer == QMF_DOMAIN && decoderProperties->renderingComponents.modeDrc1Hoa == MULTIBAND ) {
      /* HOA: is it possible to output the qmf data from HOA renderer in case of DRC1 multiband? */
      currentProcessingDomain_Hoa               = QMF_DOMAIN;
      moduleProcessingDomains->hoaRendererOutput = currentProcessingDomain_Hoa;
    }
    else {
      currentProcessingDomain_Hoa               = TIME_DOMAIN;
      moduleProcessingDomains->hoaRendererOutput = currentProcessingDomain_Hoa;
    }

    if ( moduleProcessingDomains->mixer == QMF_DOMAIN && currentProcessingDomain_Hoa == TIME_DOMAIN ) {
      domainSwitchingDecisions->hoaDomains.PostHoaRendering = QMF_ANALYSIS;
      currentProcessingDomain_Hoa                          = QMF_DOMAIN;
    }

    if ( moduleProcessingDomains->mixer != currentProcessingDomain_Hoa ) {
      fprintf(stderr, "Filterbank rule set error: processing domain after HOA rendering is not matching mixer processing domain!\n");
      error = -1;
      return error;
    }

  }


  return error;
}


int _decideProcessingDomains_PostProcessing (
                   DECODER_PROPERTIES         *decoderProperties,
                   MODULE_PROCESSING_DOMAINS  *moduleProcessingDomains,
                   DOMAIN_SWITCHING_INFO      *domainSwitchingDecisions
                   )
{
  int error = 0;
    /************************************************
               POST PROCESSING DOMAINS
  ************************************************/

  if ( moduleProcessingDomains->mixer == TIME_DOMAIN ) {


    PROCESSING_DOMAIN currentProcessingDomain_PostProcessing = TIME_DOMAIN;

    int bNoBinaural = !( decoderProperties->postProcessingComponents.bFdBinauralizer ||
                         decoderProperties->postProcessingComponents.bTdBinauralizer ||
                         decoderProperties->postProcessingComponents.bHoaBinauralizer );

    if ( bNoBinaural ) {

      if ( decoderProperties->postProcessingComponents.modeDrc2Channels == MULTIBAND ) {
        domainSwitchingDecisions->PreDrc2       = QMF_ANALYSIS;
        currentProcessingDomain_PostProcessing = QMF_DOMAIN;

        moduleProcessingDomains->drc2channels   = currentProcessingDomain_PostProcessing;

        domainSwitchingDecisions->PostDrc2      = QMF_SYNTHESIS;
        currentProcessingDomain_PostProcessing = TIME_DOMAIN;
      }
      else if ( decoderProperties->postProcessingComponents.modeDrc2Channels == SINGLEBAND ) {
        moduleProcessingDomains->drc2channels   = currentProcessingDomain_PostProcessing;
      }

    }

    /* FD Binaural */
    else if ( decoderProperties->postProcessingComponents.bFdBinauralizer ) {

      domainSwitchingDecisions->binauralDomains.PreFdBinaural = QMF_ANALYSIS;
      currentProcessingDomain_PostProcessing                 = QMF_DOMAIN;
      moduleProcessingDomains->fdBinaural                     = currentProcessingDomain_PostProcessing;

      if ( decoderProperties->postProcessingComponents.modeDrc2Binaural != DISABLED ) {
        moduleProcessingDomains->drc2binaural                      = currentProcessingDomain_PostProcessing;
        domainSwitchingDecisions->binauralDomains.PostBinauralDrc2 = QMF_SYNTHESIS;
        currentProcessingDomain_PostProcessing                    = TIME_DOMAIN;
      }
      else {
        domainSwitchingDecisions->binauralDomains.PostFdBinaural = QMF_SYNTHESIS;
        currentProcessingDomain_PostProcessing                  = TIME_DOMAIN;
      }

    }

    /* TD Binaural */
    else if ( decoderProperties->postProcessingComponents.bTdBinauralizer ) {

      moduleProcessingDomains->tdBinaural = currentProcessingDomain_PostProcessing;

      if ( decoderProperties->postProcessingComponents.modeDrc2Binaural == MULTIBAND ) {

        domainSwitchingDecisions->binauralDomains.PostTdBinaural = QMF_ANALYSIS;
        currentProcessingDomain_PostProcessing                  = QMF_DOMAIN;

        moduleProcessingDomains->drc2binaural = currentProcessingDomain_PostProcessing;

        domainSwitchingDecisions->binauralDomains.PostBinauralDrc2 = QMF_SYNTHESIS;
        currentProcessingDomain_PostProcessing                    = TIME_DOMAIN;
      }
      else if ( decoderProperties->postProcessingComponents.modeDrc2Binaural == SINGLEBAND ) {
        moduleProcessingDomains->drc2binaural = currentProcessingDomain_PostProcessing;
      }

    }

    /* HOA Binaural */
    else if ( decoderProperties->postProcessingComponents.bHoaBinauralizer ) {

      moduleProcessingDomains->hoaBinaural = currentProcessingDomain_PostProcessing;

      if ( decoderProperties->postProcessingComponents.modeDrc2Binaural == MULTIBAND ) {

        domainSwitchingDecisions->binauralDomains.PostHoaBinaural = QMF_ANALYSIS;
        currentProcessingDomain_PostProcessing                   = QMF_DOMAIN;

        moduleProcessingDomains->drc2binaural = currentProcessingDomain_PostProcessing;

        domainSwitchingDecisions->binauralDomains.PostBinauralDrc2 = QMF_SYNTHESIS;
        currentProcessingDomain_PostProcessing                    = TIME_DOMAIN;
      }
      else if ( decoderProperties->postProcessingComponents.modeDrc2Binaural == SINGLEBAND ) {
        moduleProcessingDomains->drc2binaural = currentProcessingDomain_PostProcessing;
      }

    }

    if ( currentProcessingDomain_PostProcessing != TIME_DOMAIN ) {
        fprintf(stderr, "Filterbank rule set error: processing domain after post processing is not TIME_DOMAIN!\n");
        error = -1;
        return error;
    }

  }

  /* moduleProcessingDomains->mixer == QMF_DOMAIN */
  else {

    PROCESSING_DOMAIN currentProcessingDomain_PostProcessing = QMF_DOMAIN;

    /* Mixer Output & No Binaural */
    if (  !( decoderProperties->postProcessingComponents.bFdBinauralizer ||
           decoderProperties->postProcessingComponents.bTdBinauralizer   ||
           decoderProperties->postProcessingComponents.bHoaBinauralizer ) ) {    

      if ( decoderProperties->postProcessingComponents.modeDrc2Channels != DISABLED ) {

        moduleProcessingDomains->drc2channels   = currentProcessingDomain_PostProcessing;

        domainSwitchingDecisions->PostDrc2      = QMF_SYNTHESIS;
        currentProcessingDomain_PostProcessing = TIME_DOMAIN;

      }
      else {
        domainSwitchingDecisions->PostMixer       = QMF_SYNTHESIS;
        currentProcessingDomain_PostProcessing   = TIME_DOMAIN;
      }

    }

    /* FD Binaural */
    else if ( decoderProperties->postProcessingComponents.bFdBinauralizer ) {

      currentProcessingDomain_PostProcessing = QMF_DOMAIN;
      moduleProcessingDomains->fdBinaural     = currentProcessingDomain_PostProcessing;

      if ( decoderProperties->postProcessingComponents.modeDrc2Binaural != DISABLED ) {

        moduleProcessingDomains->drc2binaural = currentProcessingDomain_PostProcessing;

        domainSwitchingDecisions->binauralDomains.PostBinauralDrc2 = QMF_SYNTHESIS;
        currentProcessingDomain_PostProcessing                    = TIME_DOMAIN;

      }
      else {
        domainSwitchingDecisions->binauralDomains.PostFdBinaural = QMF_SYNTHESIS;
        currentProcessingDomain_PostProcessing                  = TIME_DOMAIN;
      }

    }

    /* TD Binaural */
    else if ( decoderProperties->postProcessingComponents.bTdBinauralizer ) {

      domainSwitchingDecisions->binauralDomains.PreTdBinaural = QMF_SYNTHESIS;

      currentProcessingDomain_PostProcessing = TIME_DOMAIN;
      moduleProcessingDomains->tdBinaural     = currentProcessingDomain_PostProcessing;

      if ( decoderProperties->postProcessingComponents.modeDrc2Binaural == MULTIBAND ) {
        domainSwitchingDecisions->binauralDomains.PostTdBinaural = QMF_ANALYSIS;
        currentProcessingDomain_PostProcessing                  = QMF_DOMAIN;
        moduleProcessingDomains->drc2binaural                    = currentProcessingDomain_PostProcessing;

        domainSwitchingDecisions->binauralDomains.PostBinauralDrc2 = QMF_SYNTHESIS;
        currentProcessingDomain_PostProcessing                    = TIME_DOMAIN;
      }
      else if ( decoderProperties->postProcessingComponents.modeDrc2Binaural == SINGLEBAND ) {
        moduleProcessingDomains->drc2binaural                    = currentProcessingDomain_PostProcessing;
      }
    }

    /* HOA Binaural */
    if ( decoderProperties->postProcessingComponents.bHoaBinauralizer ) {

      domainSwitchingDecisions->binauralDomains.PreHoaBinaural = QMF_SYNTHESIS;

      currentProcessingDomain_PostProcessing = TIME_DOMAIN;
      moduleProcessingDomains->hoaBinaural    = currentProcessingDomain_PostProcessing;

      if ( decoderProperties->postProcessingComponents.modeDrc2Binaural  == MULTIBAND ) {
        domainSwitchingDecisions->binauralDomains.PostHoaBinaural = QMF_ANALYSIS;
        currentProcessingDomain_PostProcessing                   = QMF_DOMAIN;
        moduleProcessingDomains->drc2binaural                     = currentProcessingDomain_PostProcessing;

        domainSwitchingDecisions->binauralDomains.PostBinauralDrc2 = QMF_SYNTHESIS;
        currentProcessingDomain_PostProcessing                    = TIME_DOMAIN;
      }
    }

    if ( currentProcessingDomain_PostProcessing != TIME_DOMAIN ) {
        fprintf(stderr, "Filterbank rule set error: processing domain after post processing is not TIME!\n");
        error = -1;
        return error;
    }

  }

  if ( decoderProperties->endOfChainComponents.bsHasDrc3 ) {
    moduleProcessingDomains->drc3channel = TIME_DOMAIN;
  }

  if ( decoderProperties->endOfChainComponents.bLoudnessNormalization ) {
    moduleProcessingDomains->loudnessNorm = TIME_DOMAIN;
  }
  
  if ( decoderProperties->endOfChainComponents.bPeakLimiter ) {
    moduleProcessingDomains->peakLimiter = TIME_DOMAIN;
  }
  
  return error;
}














































int initRenderPaths( 
                     DECODER_PROPERTIES         *decoderProperties,
                     DOMAIN_SWITCHING_INFO      *domainSwitchingDecisions, 
                     MODULE_PROCESSING_DOMAINS  *moduleProcessingDomains,
                     RENDERER_PATH_DELAYS       *rendererPathDelays,
                     int                        receiverDelayCompensationBit
                     )
{
  int error = 0;
  
  error = _setInitialValues ( domainSwitchingDecisions, moduleProcessingDomains );
  if ( error ) return error;
  error = _decideProcessingDomains( decoderProperties, moduleProcessingDomains, domainSwitchingDecisions );
  if ( error ) return error;
  error = _calculateRendererPathDelays( decoderProperties, domainSwitchingDecisions, moduleProcessingDomains, rendererPathDelays );
  if ( error ) return error;
  error = _getProfileLevelRendererDelay( profile, receiverDelayCompensationBit, rendererPathDelays );

  return error;
}


int _calculateRendererPathDelays( 
                                  DECODER_PROPERTIES         *decoderProperties,
                                  DOMAIN_SWITCHING_INFO      *domainSwitchingDecisions, 
                                  MODULE_PROCESSING_DOMAINS  *moduleProcessingDomains,
                                  RENDERER_PATH_DELAYS       *rendererPathDelays
                                  )
{
  int pathDelay;
  
  /* CH-pathdelay */
  if ( decoderProperties->bCoreChannels )
  {
    pathDelay = 0;

    if ( moduleProcessingDomains->coreCoder == QMF_DOMAIN ) {
      pathDelay += GetCoreCoderConstantDelay ();
    }
    pathDelay += _getDomainSwitchDelay( domainSwitchingDecisions->channelDomains.PostCore );
    pathDelay += _getDomainSwitchDelay( domainSwitchingDecisions->channelDomains.PreDrc1 );
    pathDelay += _getDomainSwitchDelay( domainSwitchingDecisions->channelDomains.PostDrc1 );
    pathDelay += _getDomainSwitchDelay( domainSwitchingDecisions->channelDomains.PreFormatConverter );
    pathDelay += getFormatConverterDelay( profile, moduleProcessingDomains );
    pathDelay += _getDomainSwitchDelay( domainSwitchingDecisions->channelDomains.PostFormatConverter );

    rendererPathDelays->channelPath.pathDelay = (int)floor((float)pathDelay * resamplingRatio);
  }

  if( decoderProperties->renderingComponents.bObjectRendering )
  {
    pathDelay = 0;

    if ( moduleProcessingDomains->coreCoder == QMF_DOMAIN ) {
      pathDelay += GetCoreCoderConstantDelay ();
    }
    pathDelay += _getDomainSwitchDelay( domainSwitchingDecisions->objectDomains.PreDrc1 );
    pathDelay += _getDomainSwitchDelay( domainSwitchingDecisions->objectDomains.PostDrc1 );
    pathDelay += _getDomainSwitchDelay( domainSwitchingDecisions->objectDomains.PreObjectRendering );
    rendererPathDelays->objectPath.delayToPayloadApplication = pathDelay;
    pathDelay += _getDomainSwitchDelay( domainSwitchingDecisions->objectDomains.PostObjectRendering );

    rendererPathDelays->objectPath.pathDelay = (int)floor((float)pathDelay * resamplingRatio);
  }

  if( decoderProperties->renderingComponents.bHoaRendering )
  {
    pathDelay = 0;

    if ( moduleProcessingDomains->coreCoder == QMF_DOMAIN ) {
      pathDelay += GetCoreCoderConstantDelay ();
    }
    /* pathDelay += _getDomainSwitchDelay( domainSwitchingDecisions->hoaDomains.PreDrc1 ); */ /* disabled due to missing HOA metadata alignment */
    /* pathDelay += _getDomainSwitchDelay( domainSwitchingDecisions->hoaDomains.PostDrc1 ); */ /* disabled due to missing HOA metadata alignment */
    pathDelay += _getDomainSwitchDelay( domainSwitchingDecisions->hoaDomains.PreHoaRendering );
    rendererPathDelays->hoaPath.delayToPayloadApplication = pathDelay;
    pathDelay += _getDomainSwitchDelay( domainSwitchingDecisions->hoaDomains.PostHoaRendering );

    rendererPathDelays->hoaPath.pathDelay = (int)floor((float)pathDelay * resamplingRatio);
  }

  if( decoderProperties->renderingComponents.bSaocRendering )
  {
    pathDelay = 0;

    if ( moduleProcessingDomains->coreCoder == QMF_DOMAIN ) {
      pathDelay += GetCoreCoderConstantDelay ();
    }
    pathDelay += _getDomainSwitchDelay( domainSwitchingDecisions->saocDomains.PreSaocRendering );
    rendererPathDelays->saocPath.delayToPayloadApplication = pathDelay;
    pathDelay += _getDomainSwitchDelay( domainSwitchingDecisions->saocDomains.PostSaocRendering );

    rendererPathDelays->saocPath.pathDelay = (int)floor((float)pathDelay * resamplingRatio);
  }

  return 0;
}


int _getDomainSwitchDelay( DOMAIN_SWITCH domSwitch )
{
  int delay;
  switch ( domSwitch )
  {
    case KEEP_DOMAIN:
      delay = 0;
      break;
    case QMF_SYNTHESIS:
      delay = 257;
      break;
    case QMF_ANALYSIS:
      delay = 320;
      break;
    case STFT_SYNTHESIS:
      delay = 256;
      break;
    case STFT_ANALYSIS:
      delay = 0;
      break;
    default:
      delay = -10000;
  }

  return delay;
}

int _getProfileLevelRendererDelay( PROFILE profile, int receiverDelayCompensationBit, RENDERER_PATH_DELAYS *rendererPathDelays )
{
  int retDelay = 0;

  if( receiverDelayCompensationBit )
  {
    switch ( profile )
    {
      case PROFILE_LOW_COMPLEXITY:
      case PROFILE_BASELINE:
      {
        retDelay = MAX_RESAMPLING_DELAY + (256 * RSR_MAX); /* Resampler + STFT */
        break;
      }

      case PROFILE_RESERVED_ISO:
        fprintf(stderr, "\ngetFormatConverterDelay: Warning. Profile reserved for ISO, assuming High Profile.\n\n");

      case PROFILE_MAIN:
      case PROFILE_HIGH:
      {
        /* ResamplerDelay + 321 (SBR-QMF-Ana) + 257 (SBR-QMF-Syn) + 384 (SBR-Lookahead) + 384 (MPS212-3) + 256 (STFT-Analysis) + FormatConverter */
        retDelay = MAX_RESAMPLING_DELAY + (5057 * RSR_MAX);
        break;
      }
    }
    
  } else {
    retDelay = rendererPathDelays->channelPath.pathDelay;
    if( rendererPathDelays->objectPath.pathDelay > retDelay )
    {
      retDelay = rendererPathDelays->objectPath.pathDelay;
    }
    if( rendererPathDelays->hoaPath.pathDelay > retDelay )
    {
      retDelay = rendererPathDelays->hoaPath.pathDelay;
    }
    if( rendererPathDelays->saocPath.pathDelay > retDelay )
    {
      retDelay = rendererPathDelays->saocPath.pathDelay; 
    }
    retDelay = (int)floor((float)retDelay * resamplingRatio);

    if ( bResamplerActive ) {
      retDelay += MAX_RESAMPLING_DELAY;
    }
  }

  rendererPathDelays->maxDelay = retDelay;
  return 0;
}


int getFormatConverterDelay( PROFILE profile, MODULE_PROCESSING_DOMAINS *moduleProcessingDomains )
{
  int foCoDelay = 0;
  switch( profile )
  {
    case PROFILE_RESERVED_ISO:
      fprintf(stderr, "\ngetFormatConverterDelay: Warning. Profile reserved for ISO, assuming High Profile.\n\n");
    case PROFILE_MAIN:
    case PROFILE_HIGH:
    {
      if( moduleProcessingDomains->formatConverter == STFT_DOMAIN )
      {
        foCoDelay = 0;
      } else if ( moduleProcessingDomains->formatConverter == QMF_DOMAIN )
      {
        foCoDelay = 3456;
      } else if ( moduleProcessingDomains->formatConverter == TIME_DOMAIN )
      {
        foCoDelay = 0;
      }
      else if ( moduleProcessingDomains->formatConverter == NOT_DEFINED )
      {
        foCoDelay = 0;
      }
      break;
    }
    case PROFILE_LOW_COMPLEXITY:
    case PROFILE_BASELINE:
    {
      if ( moduleProcessingDomains->formatConverter == TIME_DOMAIN 
        || moduleProcessingDomains->formatConverter == STFT_DOMAIN )
      {
        foCoDelay = 0;
      }

      break;
    }
    default:
      return 0;
  }

  return foCoDelay;
}


