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


const HUFF_CLD_TABLE huffCLDTab =
{
  {
    { 0x52, 0xae, 0xaf, 0x28, 0x6e, 0x36, 0x1e, 0x0e, 0x0c, 0x0a, 0x02, 0x16, 0x12, 0x17, 0x00, 0x04, 0x06, 0x08, 0x07, 0x03, 0x01, 0x1a, 0x13, 0x3e, 0x16, 0x17, 0x6f, 0x2a, 0x56, 0x53, 0x3f },
    {    8,    9,    9,    7,    7,    6,    5,    4,    4,    4,    4,    5,    5,    5,    4,    4,    4,    4,    4,    4,    4,    5,    5,    6,    6,    6,    7,    7,    8,    8,    6 }
  },
  {
    {
      { 0x0000, 0x0002, 0x0006, 0x000e, 0x001e, 0x003e, 0x007e, 0x00fe, 0x01fe, 0x03fe, 0x07fe, 0x0ffe, 0x1ffe, 0x7ffe, 0x7ffc, 0xfffe, 0xfffa, 0x1fffe, 0x1fff6, 0x3fffe, 0x3ffff, 0x7ffde, 0x3ffee, 0xfffbe, 0x1fff7e, 0xfffbfc, 0xfffbfd, 0xfffbfe, 0xfffbff, 0x7ffdfc, 0x7ffdfd },
      {      1,      2,      3,      4,      5,      6,      7,      8,      9,     10,     11,     12,     13,     15,     15,     16,     16,     17,     17,     18,     18,     19,     18,     20,     21,     24,     24,     24,     24,     23,     23 }
    },
    {
      { 0x0000, 0x0002, 0x0006, 0x000e, 0x001e, 0x003e, 0x007e, 0x01fe, 0x01fc, 0x03fe, 0x03fa, 0x07fe, 0x07f6, 0x0ffe, 0x0fee, 0x1ffe, 0x1fde, 0x3ffe, 0x3fbe, 0x3fbf, 0x7ffe, 0xfffe, 0x1fffe, 0x7fffe, 0x7fffc, 0xffffa, 0x1ffffc, 0x1ffffd, 0x1ffffe, 0x1fffff, 0xffffb },
      {      1,      2,      3,      4,      5,      6,      7,      9,      9,     10,     10,     11,     11,     12,     12,     13,     13,     14,     14,     14,     15,     16,     17,     19,     19,     20,     21,     21,     21,     21,     20 }
    }
  }
};
