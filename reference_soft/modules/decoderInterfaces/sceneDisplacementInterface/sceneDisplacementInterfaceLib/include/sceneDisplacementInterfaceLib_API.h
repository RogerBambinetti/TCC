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


#ifndef SD_API
#define SD_API

#ifndef NULL
  #define NULL 0
#endif

#include <stdio.h>

typedef struct
{
    float sd_azimuth;
    float sd_elevation;
    float sd_radius;

    /* Quantized values */
    unsigned int bs_sd_azimuth;
    unsigned int bs_sd_elevation;
    unsigned int bs_sd_radius;

} PosSceneDisplacementData;

typedef struct
{
    float roll;
    float pitch;
    float yaw;

    int roll_q;
    int pitch_q;
    int yaw_q;

} RotSceneDisplacementData;

typedef struct
{
    RotSceneDisplacementData rotSceneDsplData;
    PosSceneDisplacementData posSceneDsplData;

} SceneDisplacementData;


typedef struct _SceneDisplacementHandle *SCENE_DISPLACEMENT_HANDLE;

typedef struct _sdBitstreamEncoder *H_SD_BITSTREAM_ENCODER;
typedef struct _sdBitstreamDecoder *H_SD_BITSTREAM_DECODER;

int SD_initHandle(SCENE_DISPLACEMENT_HANDLE *h_SceneDisplacementHandle);
void SD_closeHandle(SCENE_DISPLACEMENT_HANDLE h_SceneDisplacementHandle);

void SD_setSceneDisplacementData(SCENE_DISPLACEMENT_HANDLE hSceneDspl,
                                 SceneDisplacementData const *sceneDsplData);

int SD_getSceneDisplacementData(SCENE_DISPLACEMENT_HANDLE const SceneDisplacement,
                                SceneDisplacementData *sceneDisplacementData);

int SD_writeSceneDisplacementBitstream(SCENE_DISPLACEMENT_HANDLE SceneDisplacement, H_SD_BITSTREAM_ENCODER h_bitstream, unsigned long *noBitsWritten);
int SD_readSceneDisplacementBitstream(SCENE_DISPLACEMENT_HANDLE SceneDisplacement, H_SD_BITSTREAM_DECODER h_bitstream, unsigned long *noBits);


int SD_DecInt_initBitstreamWriter(H_SD_BITSTREAM_ENCODER *h_bitstream, char *file);
void SD_DecInt_closeBitstreamWriter(H_SD_BITSTREAM_ENCODER h_bitstream);
int SD_DecInt_initBitstreamReader(H_SD_BITSTREAM_DECODER *h_bitstream, char *file);
void SD_DecInt_closeBitstreamReader(H_SD_BITSTREAM_DECODER h_bitstream);

/*------------------------------------------------------------------------------------------------*/
/* Positional scene displacement interface (PSDI)                                                 */
/*------------------------------------------------------------------------------------------------*/

/**
   @brief Enable or disable positional scene displacement interface
   @param hSceneDspl Scene displacement handle
   @param enabled 0 to disable, else enable
 */
void PSDI_enabled(SCENE_DISPLACEMENT_HANDLE hSceneDspl, unsigned int enabled);


/**
   @brief Check whether PSDI is enabled
   @param hSceneDspl Scene displacement handle
   @return 1/0 (enabled/disabled)
 */
int PSDI_isEnabled(SCENE_DISPLACEMENT_HANDLE hSceneDspl);

/*------------------------------------------------------------------------------------------------*/
#endif
