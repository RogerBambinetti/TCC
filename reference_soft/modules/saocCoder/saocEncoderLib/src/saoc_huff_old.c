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

#include "saoc_huff_tab.h"


const HUFF_OLD_TABLE huffOLDTab =
{
  {
    { 0x00, 0x1e, 0x12, 0x14, 0x15, 0x16, 0x1f, 0x06, 0x22, 0x10, 0x2e, 0x2f, 0x23, 0x26, 0x27, 0x0e },
    {    1,    5,    5,    5,    5,    5,    5,    3,    6,    5,    6,    6,    6,    6,    6,    4 }
  },
  {
    {
      { 0x0000, 0x0006, 0x0004, 0x000e, 0x000a, 0x001e, 0x0016, 0x001f, 0x005c, 0x00be, 0x005d, 0x00bc, 0x017e, 0x017a, 0x017b, 0x017f },
      {      1,      3,      3,      4,      4,      5,      5,      5,      7,      8,      7,      8,      9,      9,      9,      9 }
    },
    {
      { 0x0000, 0x0002, 0x000e, 0x000c, 0x001e, 0x001a, 0x003e, 0x001b, 0x00fc, 0x01fc, 0x01fa, 0x01fb, 0x03fe, 0x03ff, 0x01fd, 0x01fe },
      {      1,      2,      4,      4,      5,      5,      6,      5,      8,      9,      9,      9,     10,     10,      9,      9 }
    }
  }
};
