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


const HUFF_NRG_TABLE huffNRGTab =
{
  {
    { 0x67e, 0x8fe, 0x8ff, 0x67f, 0x2fe, 0x47e, 0x17e, 0x2ff, 0x23e, 0x33e, 0xbe, 0x11e, 0x19e, 0x5e, 0x8e, 0x06, 0x2e, 0x76, 0x06, 0x12, 0x36, 0x0e, 0x14, 0x0c, 0x3e, 0x3f, 0x15, 0x06, 0x3c, 0x16, 0x26, 0x2e, 0xce, 0x07, 0x46, 0x5e, 0x5f, 0x77, 0x66, 0x02, 0x0e, 0x27, 0x02, 0x0a, 0x12, 0x18, 0x1c, 0x16, 0x0f, 0x00, 0x37, 0x04, 0x0d, 0x1a, 0x10, 0x05, 0x08, 0x32, 0x07, 0x22, 0x13, 0x3a, 0x0f, 0x3d },
    {   11,   12,   12,   11,   11,   11,   10,   11,   10,   10,    9,    9,    9,    8,    8,    7,    7,    7,    6,    6,    6,    5,    5,    5,    6,    6,    5,    5,    6,    6,    6,    6,    8,    7,    7,    7,    7,    7,    7,    6,    6,    6,    5,    5,    5,    5,    5,    5,    5,    5,    6,    5,    5,    5,    5,    5,    5,    6,    6,    6,    6,    6,    6,    6 }
  },
  {
    {
      { 0x0006, 0x0000, 0x0007, 0x0004, 0x0002, 0x000a, 0x0006, 0x0016, 0x000e, 0x002e, 0x001e, 0x005e, 0x003e, 0x00be, 0x017e, 0x00fe, 0x00fc, 0x02fe, 0x01fe, 0x05fe, 0x01fa, 0x03f6, 0x03fe, 0x03f7, 0x0bfe, 0x07fe, 0x17fe, 0x1ffe, 0x1fff, 0x1ffc, 0x5ffe, 0x3ffa, 0xffee, 0x5fff, 0x1ffde8, 0x7ff6, 0x2ffe, 0x1ffde9, 0x1ffdea, 0x1ffdeb, 0x1ffdec, 0x1ffded, 0x1ffdee, 0x1ffdef, 0x1ffdf0, 0x1ffdf1, 0x1ffdf2, 0x1ffdf3, 0x1ffdf4, 0x1ffdf5, 0x1ffdf6, 0x1ffdf7, 0x1ffdf8, 0x1ffdf9, 0x1ffdfa, 0x1ffdfb, 0x1ffdfc, 0x1ffdfd, 0x1ffdfe, 0x1ffdff, 0xffef0, 0xffef1, 0xffef2, 0xffef3 },
      {      3,      2,      3,      3,      3,      4,      4,      5,      5,      6,      6,      7,      7,      8,      9,      9,      9,     10,     10,     11,     10,     11,     11,     11,     12,     12,     13,     14,     14,     14,     15,     15,     17,     15,     22,     16,     14,     22,     22,     22,     22,     22,     22,     22,     22,     22,     22,     22,     22,     22,     22,     22,     22,     22,     22,     22,     22,     22,     22,     22,     21,     21,     21,     21 }
    },
    {
      { 0x0000, 0x0002, 0x0006, 0x0002, 0x000e, 0x001e, 0x000e, 0x000c, 0x003e, 0x001e, 0x001a, 0x007e, 0x003e, 0x0036, 0x00fe, 0x007e, 0x006e, 0x00fe, 0x00de, 0x03fe, 0x03fc, 0x01fe, 0x01be, 0x07fa, 0x03fe, 0x037e, 0x037f, 0x0ffc, 0x0ffe, 0x0ff6, 0x07fe, 0x0ffe, 0x0fff, 0x3ffc, 0x7ffc, 0x7ffd, 0x3ff6, 0x7ffe, 0x7fea, 0x3fdc, 0xfff6, 0x3ff7, 0xfffe, 0x7feb, 0x1fffe, 0x3fde, 0x3fdd, 0x3ff4, 0x3fffe, 0xfffffe, 0x3fdf, 0xfff4, 0xfff5, 0xffffff, 0xfff7, 0xffffe, 0x7fffe, 0x7ffff8, 0x7ffff9, 0x7ffffa, 0x7ffffb, 0x7ffffc, 0x7ffffd, 0x7ffffe },
      {      2,      2,      3,      3,      4,      5,      5,      5,      6,      6,      6,      7,      7,      7,      8,      8,      8,      9,      9,     10,     10,     10,     10,     11,     11,     11,     11,     12,     12,     12,     12,     13,     13,     14,     15,     15,     14,     15,     15,     14,     16,     14,     16,     15,     17,     14,     14,     14,     18,     24,     14,     16,     16,     24,     16,     20,     19,     23,     23,     23,     23,     23,     23,     23 }
    }
  }
};
