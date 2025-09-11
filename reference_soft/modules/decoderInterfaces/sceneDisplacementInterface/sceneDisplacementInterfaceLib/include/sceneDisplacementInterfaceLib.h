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

Copyright (c) ISO/IEC 2016.

***********************************************************************************/

#ifndef MAX_LENGTH_SIGNATURE
  #define MAX_LENGTH_SIGNATURE 255
#endif

#include "sceneDisplacementInterfaceLib_API.h"

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

enum
{
  PSDI_AZIMUTH_BITS = 8,
  PSDI_ELEVATION_BITS = 6,
  PSDI_RADIUS_BITS = 4
};

struct _SceneDisplacementHandle
{
  int noBits;
  int bPosSceneDsplEnabled;               /*!< Positional scene displacement interface enabled */
  SceneDisplacementData sceneDsplData;    /*!< Rotational and positional scene displacement data */
};

int SD_quantizeData(SCENE_DISPLACEMENT_HANDLE SceneDisplacement);
int SD_writeSceneDisplacementBits(SCENE_DISPLACEMENT_HANDLE SceneDisplacement, H_SD_BITSTREAM_ENCODER h_bitstream, unsigned long *noBitsWritten, unsigned long *noBitsWrittenTotal);
int SD_readSceneDisplacementBits(SCENE_DISPLACEMENT_HANDLE SceneDisplacement, H_SD_BITSTREAM_DECODER h_bitstream, unsigned long *noBitsWritten, unsigned long *noBitsWrittenTotal);
int SD_invquantizeData(SCENE_DISPLACEMENT_HANDLE SceneDisplacement);

/*------------------------------------------------------------------------------------------------*/
/* Positional scene displacement interface (PSDI)                                                 */
/*------------------------------------------------------------------------------------------------*/

/**
   @brief Parse PSDI data, i.e. `mpegh3daPositionalDisplacementData'
   @param hSceneDspl Scene displacement handle
   @param hBitstream Bitstream handle
   @return > 0 on success, else < 0
 */
int PSDI_parse(SCENE_DISPLACEMENT_HANDLE hSceneDspl, H_SD_BITSTREAM_DECODER hBitstream);


/**
   @brief Write PSDI data
   @param hSceneDspl Scene displacement handle
   @param hBitstream Bitstream handle
   @return Number of bits written, -1 on error
 */
int PSDI_write(SCENE_DISPLACEMENT_HANDLE hSceneDspl, H_SD_BITSTREAM_ENCODER hBitstream);


/**
   @brief Quantization of PSDI data
   @param pPosSceneDsplData Pointer to PSDI data struct
   @return 0 on success else < 0
 */
int PSDI_quantization(PosSceneDisplacementData *pPosSceneDsplData);


/**
   @brief Inverse quantization of PSDI data
   @param pPosSceneDsplData Pointer to PSDI data struct
   @return 0 on success else < 0
 */
int PSDI_invQuantization(PosSceneDisplacementData *pPosSceneDsplData);

/*------------------------------------------------------------------------------------------------*/
