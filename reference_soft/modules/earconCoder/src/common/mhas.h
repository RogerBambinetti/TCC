/************************************************************************

This software module was originally developed by Fraunhofer IIS in the
course of development of the ISO/IEC 23008-3 for reference purposes and
its performance may not have been optimized. This software module is an
implementation of one or more tools as specified by the ISO/IEC 23008-3
standard. ISO/IEC gives you a royalty-free, worldwide, non-exclusive,
copyright license to copy, distribute, and make derivative works of this 
software module or modifications thereof for use in implementations or
products claiming conformance to the ISO/IEC 23008-3 standard and which
satisfy any specified conformance criteria. 
Those intending to use this software module in products are advised that 
its use may infringe existing patents.

ISO/IEC have no liability for use of this software module or 
modifications thereof. Copyright is not released for products that do
not conform to the ISO/IEC 23008-3 standard.

Fraunhofer IIS retains full right to modify and use the code for its own
purpose, assign or donate the code to a third party and to inhibit third
parties from using the code for products that do not conform to
MPEG-related ITU Recommendations and/or ISO/IEC International Standards.

This copyright notice must be included in all copies or derivative works.

Copyright (c) ISO/IEC 2018.

*************************************************************************/
#ifndef MHAS_H
#define MHAS_H
/* ######################################################################*/
/* ################################ includes ############################*/
/* ######################################################################*/

/* SYSTEM INCLUDES */

/* INCLUDES OF THIS PROJECT */

/* OTHER INCLUDES */

/* ######################################################################*/
/* ################################ defines #############################*/
/* ######################################################################*/
#define MAX_NUM_BYTES_MHAS_PACKET_HEADER (15)
#define MAX_NUM_BYTES_SUPPORTED_MHAS_PACKET_PAYLOAD (1024*10)
#define MAX_NUM_BYTES_SUPPORTED_MHAS_PACKET (MAX_NUM_BYTES_MHAS_PACKET_HEADER + MAX_NUM_BYTES_SUPPORTED_MHAS_PACKET_PAYLOAD)
/* ######################################################################*/
/* ################################# enums ##############################*/
/* ######################################################################*/

/*! supported MHAS packet types */
enum PACTYP {
  /* other pactyps are irrelevant here */
  PACTYP_EARCON = 19,
  PACTYP_PCMCONFIG = 20,
  PACTYP_PCMDATA = 21
  /* other pactyps are irrelevant here */
};

/* ######################################################################*/
/* ################################ structs #############################*/
/* ######################################################################*/

/* ######################################################################*/
/* ############################# functions ##############################*/
/* ######################################################################*/

#endif /* ifndef MHAS_H */
