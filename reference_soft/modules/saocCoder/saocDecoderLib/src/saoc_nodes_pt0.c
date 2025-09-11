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

#include "saoc_huff_nodes.h"



HUFF_PT0_NODES huffPart0Nodes =
{
  { { 2, 1}, { 4, 3}, { 6, 5}, { 8, 7}, {10, 9}, {12,11}, {14,13}, {-8,15}, {-9,16}, {-10,17}, {-18,18}, {-17,-19}, {-16,19}, {-11,-20}, {-15,-21}, {-7,20}, {-22,21}, {-12,-14}, {-13,-23}, {23,22}, {-24,-31}, {-6,24}, {-25,-26}, {26,25}, {-5,-27}, {-28,27}, {-4,28}, {-29,29}, {-1,-30}, {-2,-3} },
  { { 2, 1}, {-5, 3}, {-4,-6}, {-3, 4}, {-2, 5}, {-1, 6}, {-7,-8} },
  { { 2, 1}, { 4, 3}, { 6, 5}, {-15, 7}, {-14,-16}, {-13, 8}, {-12, 9}, {-11,10}, {-10,11}, {-8,-9}, {-17,12}, {14,13}, {-7,15}, {-18,16}, {-6,17}, {-5,18}, {-4,-19}, {-3,19}, {-1,20}, {-2,-20}, {22,21}, {-21,23}, {-22,-26}, {-23,24}, {-24,-25} },
  { {-1, 1}, { 3, 2}, {-8, 4}, { 6, 5}, {-16, 7}, { 9, 8}, {11,10}, {-2,-7}, {-6,12}, {-4,-5}, {-3,13}, {-10,14}, {-11,-12}, {-14,-15}, {-9,-13} },
  { { 2, 1}, { 4, 3}, { 6, 5}, { 8, 7}, {10, 9}, {12,11}, {14,13}, {16,15}, {18,17}, {20,19}, {22,21}, {24,23}, {26,25}, {28,27}, {30,29}, {32,31}, {-47,33}, {-54,34}, {-46,35}, {-48,36}, {-23,-27}, {-45,37}, {-55,38}, {-22,-49}, {-24,-53}, {-44,39}, {-57,40}, {-28,41}, {-52,-56}, {-43,42}, {-50,43}, {-25,-26}, {-29,-64}, {-62,44}, {-21,-51}, {-58,45}, {-32,46}, {-31,-42}, {-60,47}, {-30,48}, {-20,-61}, {-41,-63}, {-19,-59}, {-40,49}, {-18,-38}, {-39,50}, {-36,-37}, {-35,51}, {-17,52}, {-16,-34}, {-33,53}, {-15,54}, {-14,55}, {-13,56}, {-12,57}, {-11,58}, {-10,59}, {-9,60}, {-7,61}, {-1,-4}, {-6,62}, {-5,-8}, {-2,-3} }
};


HUFF_LAV_NODES huffLavIdxNodes =
{
  { {-1, 1}, {-2, 2}, {-3,-4} }
};
