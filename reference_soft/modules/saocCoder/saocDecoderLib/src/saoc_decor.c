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
software module or modifications thereof for the sole purpose of develoSAOC_PIng
ISO/IEC 23003.

#endif

Coding Technologies, Fraunhofer IIS, Philips retain full right to
modify and use the code for its (their) own purpose, assign or donate the code
to a third party and to inhibit third parties from using the code for products
that do not conform to MPEG-related ITU Recommendations and/or ISO/IEC
International Standards. This copyright notice must be included in all coSAOC_PIes or
derivative works.

Copyright (c) ISO/IEC 2007.
*******************************************************************************/

/* assorted definitions and constants */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "saoc_decor.h"
#include "saoc_decorr_tables.h"
#include "saoc_const.h"
#include "error.h"

#include "saoc_svd_tool.h"
#include "hierChnlGrouping.h"

#include "saoc_decorr_tables.h"

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

/* #define RINGING_PATCH */

#define MAX_NO_LATTICE_SEEDS (10)

#define PS_REVERB_SLOPE                 ( 0.05f )
#define PS_REVERB_DECAY_CUTOFF          ( 10 )
#define PS_REVERB_SERIAL_TIME_CONST     ( 7.0f )
#define PS_DUCK_PEAK_DECAY_FACTOR       ( 0.765928338364649f )
#define PS_DUCK_FILTER_COEFF            ( 0.25f )


#define NO_DECORR_BANDS                 (  4 )
#define DECORR_FILTER_ORDER_BAND_0      ( 20 )
#define DECORR_FILTER_ORDER_BAND_1      ( 15 )
#define DECORR_FILTER_ORDER_BAND_2      (  6 )
#define DECORR_FILTER_ORDER_BAND_3      (  3 )

#define MAX_DECORR_FILTER_ORDER         ( DECORR_FILTER_ORDER_BAND_0 )
#define MAX_NO_TIME_SLOTS_DELAY         ( 14 )

#define DUCK_ALPHA                      ( 0.8f )
#define DUCK_GAMMA                      ( 1.5f )




/* pre-delays: */
static int REV_delay[NO_DECORR_BANDS] = {
  8,   7,   2,   1
};


static int REV_delay_PS[NO_DECORR_BANDS] = {
  8,  14,   1,   0
};


static int REV_splitfreq0_STD[NO_DECORR_BANDS] = { 3, 15, 24, 65 };
static int REV_splitfreq0_LD[NO_DECORR_BANDS] = { 0, 15, 24, 65 };
static int REV_splitfreq1[NO_DECORR_BANDS] = { 3, 50, 65, 65 };
static int REV_splitfreq2[NO_DECORR_BANDS] = { 0, 15, 65, 65 };
static int REV_splitfreq3[NO_DECORR_BANDS] = { 0,  5, 65, 65 };
static int REV_splitfreqLP[NO_DECORR_BANDS] = { 0, 8, 24, 65 };

static int REV_splitfreq0_PS[NO_DECORR_BANDS] = { 24, 35, 65, 65 };
static int REV_splitfreq1_PS[NO_DECORR_BANDS] = {  8, 36, 65, 65 };

static float hybridCenterFreq_STD[] = {
  -0.5f/4, 0.5f/4, -1.5f/4,1.5f/4, 2.5f/4, 3.5f/4, 2.5f/2, 3.5f/2, 4.5f/2, 5.5f/2
};

static float hybridCenterFreq_LD[] = {
  0.5f, 1.5f, 2.5f
};

/* lattice coefficients: */
static float lattice_coeff_0[MAX_NO_LATTICE_SEEDS][DECORR_FILTER_ORDER_BAND_0] = {
{  0.2475f,  0.4533f, -0.0220f, -0.3082f, -0.0056f,  0.3794f, -0.3458f,  0.3804f, -0.3260f,  0.4896f, -0.2288f,  0.3549f,  0.1939f, -0.1401f,  0.0386f, -0.4055f, -0.3048f, -0.2495f,  0.0393f,  0.2459f },
{  0.3952f, -0.4245f, -0.0287f,  0.4315f,  0.0721f, -0.1158f,  0.0684f, -0.0127f, -0.2566f,  0.1936f, -0.4014f, -0.2554f, -0.0591f,  0.3383f, -0.1892f, -0.0731f,  0.2103f,  0.4499f, -0.3635f, -0.1903f },
{ -0.0944f, -0.2082f, -0.1001f, -0.4370f, -0.4155f,  0.3806f, -0.1479f, -0.4522f,  0.4083f,  0.0612f,  0.4879f,  0.1645f, -0.1656f,  0.1730f,  0.0200f, -0.2621f,  0.3497f, -0.2280f,  0.3139f, -0.3197f },
{  0.0722f,  0.1846f, -0.4198f,  0.0889f, -0.2632f, -0.2662f, -0.2742f, -0.4682f,  0.2816f,  0.3099f, -0.2279f,  0.0864f,  0.0272f, -0.2042f,  0.2306f,  0.0122f, -0.2310f, -0.2642f, -0.3957f, -0.3469f },
{ -0.3443f, -0.4403f,  0.2218f, -0.1461f, -0.0943f, -0.1049f, -0.1388f,  0.4451f, -0.0789f,  0.4745f, -0.1422f, -0.0887f,  0.0618f,  0.1735f,  0.1547f,  0.0238f, -0.1051f, -0.1086f,  0.4759f, -0.1258f },
{  0.0662f, -0.1323f, -0.3477f, -0.3828f, -0.0762f,  0.3154f, -0.4346f, -0.2740f,  0.3700f, -0.1195f,  0.0170f,  0.3909f, -0.4284f, -0.0454f, -0.2715f, -0.4931f,  0.1152f,  0.3087f, -0.3798f, -0.3429f },
{ -0.4724f,  0.3396f,  0.1733f, -0.3915f,  0.2796f,  0.2915f, -0.0024f, -0.1738f,  0.2879f,  0.0898f,  0.2862f, -0.2312f,  0.4374f, -0.4526f,  0.3793f,  0.2658f,  0.4782f, -0.1826f,  0.1648f,  0.2059f },
{  0.1577f,  0.3950f,  0.4540f,  0.2996f, -0.4999f,  0.3938f,  0.1888f,  0.4772f,  0.3688f, -0.0501f, -0.4821f,  0.0074f,  0.0502f, -0.2279f, -0.4157f,  0.1614f,  0.1336f,  0.2854f,  0.3497f,  0.3133f },
{ -0.1580f, -0.1936f, -0.1458f, -0.2817f, -0.3064f,  0.0990f,  0.4764f, -0.2513f,  0.4283f, -0.3693f,  0.2617f,  0.3667f,  0.1676f,  0.1514f,  0.0075f, -0.0974f,  0.0162f, -0.1117f, -0.4114f,  0.3487f },
{  0.0264f,  0.2251f,  0.4612f, -0.1914f, -0.3648f,  0.2407f,  0.0500f,  0.3089f,  0.3084f, -0.4694f,  0.3599f, -0.4348f, -0.3981f,  0.4145f, -0.1872f,  0.1813f, -0.1671f, -0.3112f,  0.0975f, -0.4025f }
};


static float lattice_coeff_1_STD[MAX_NO_LATTICE_SEEDS][DECORR_FILTER_ORDER_BAND_1] = {
{  0.2959f, -0.3776f, -0.3481f,  0.3072f,  0.3985f,  0.3516f,  0.1688f, -0.1407f, -0.0015f,  0.0513f,  0.2666f, -0.2972f, -0.1947f,  0.4759f, -0.2021f },
{  0.0420f, -0.1524f,  0.3408f,  0.4892f,  0.1777f, -0.0398f, -0.0390f, -0.0707f, -0.3841f,  0.4600f,  0.1767f,  0.2617f,  0.0709f, -0.1727f,  0.0810f },
{  0.4053f,  0.2174f, -0.4658f,  0.0482f,  0.0518f,  0.0721f, -0.3893f, -0.0579f,  0.0906f, -0.4522f, -0.3281f,  0.1896f, -0.0635f,  0.0012f,  0.0643f },
{  0.3905f,  0.0992f, -0.1648f,  0.1944f, -0.0626f,  0.3910f, -0.4001f,  0.3090f,  0.0900f, -0.1152f, -0.3058f, -0.4664f, -0.3929f, -0.2836f, -0.2735f },
{ -0.2622f,  0.2350f,  0.1764f, -0.3021f,  0.1762f,  0.2920f,  0.4809f, -0.1202f, -0.2795f, -0.1888f,  0.1865f,  0.0060f,  0.3520f, -0.1109f, -0.2948f },
{ -0.4827f,  0.2138f, -0.2622f, -0.4881f,  0.4742f,  0.3745f,  0.1271f,  0.4817f, -0.2170f, -0.0060f,  0.1829f, -0.1673f, -0.0047f,  0.3961f, -0.4351f },
{ -0.3883f,  0.0318f, -0.2874f, -0.0542f, -0.2755f, -0.2018f,  0.4665f,  0.4241f,  0.4543f,  0.3546f,  0.0625f,  0.0208f, -0.0145f,  0.2692f, -0.2424f },
{ -0.3210f, -0.4454f,  0.1732f,  0.1553f, -0.2767f,  0.0540f, -0.1828f,  0.2446f,  0.1456f,  0.1168f, -0.3568f,  0.2494f, -0.2735f,  0.3923f,  0.0280f },
{ -0.3958f, -0.2023f,  0.2725f, -0.1185f,  0.3422f, -0.2500f,  0.1185f,  0.3660f, -0.1043f, -0.4890f, -0.0352f, -0.2951f, -0.1169f, -0.1835f,  0.4774f },
{  0.0489f,  0.4583f,  0.4420f,  0.1117f, -0.2788f, -0.2301f, -0.1932f, -0.4608f,  0.0235f, -0.3540f,  0.4354f, -0.0101f,  0.1331f,  0.3887f, -0.4932f }
};

static float lattice_coeff_1_LD[MAX_NO_LATTICE_SEEDS][DECORR_FILTER_ORDER_BAND_1] = {
{  0.2959f, -0.3776f, -0.3481f,  0.3072f,  0.3985f,  0.3516f,  0.1688f, -0.1407f, -0.0015f,  0.0513f,  0.2666f, -0.2972f, -0.1947f,  0.4759f, -0.2021f },
{  0.0420f, -0.1524f,  0.3408f,  0.4892f,  0.1777f, -0.0398f, -0.0390f, -0.0707f, -0.3841f,  0.4600f,  0.1767f,  0.2617f,  0.0709f, -0.1727f,  0.0810f },
{  0.4053f,  0.2174f, -0.4658f,  0.0482f,  0.0518f,  0.0721f, -0.3893f, -0.0579f,  0.0906f, -0.4522f, -0.3281f,  0.1896f, -0.0635f,  0.0012f,  0.0643f },
{  0.3905f,  0.0992f, -0.1648f,  0.1944f, -0.0626f,  0.3910f, -0.4001f,  0.3090f,  0.0900f, -0.1152f, -0.3058f, -0.4664f, -0.3929f, -0.2836f, -0.2735f },
{ -0.2622f,  0.2350f,  0.1764f, -0.3021f,  0.1762f,  0.2920f,  0.4809f, -0.1202f, -0.2795f, -0.1888f,  0.1865f,  0.0060f,  0.3520f, -0.1109f, -0.2948f },
{ -0.4827f,  0.2138f, -0.2622f, -0.4881f,  0.4742f,  0.3745f,  0.1271f,  0.4817f, -0.2170f, -0.0060f,  0.1829f, -0.1673f, -0.0047f,  0.3961f, -0.4351f },
{ -0.3883f,  0.0318f, -0.2874f, -0.0542f, -0.2755f, -0.2018f,  0.4665f,  0.4241f,  0.4543f,  0.3546f,  0.0625f,  0.0208f, -0.0145f,  0.2692f, -0.2424f },
{ -0.4807f,  0.4291f,  0.2229f,  0.0479f, -0.1401f, -0.0704f,  0.2764f, -0.3689f, -0.0679f,  0.2940f, -0.2212f,  0.0692f, -0.0547f,  0.2685f,  0.3356f },
{ -0.3958f, -0.2023f,  0.2725f, -0.1185f,  0.3422f, -0.2500f,  0.1185f,  0.3660f, -0.1043f, -0.4890f, -0.0352f, -0.2951f, -0.1169f, -0.1835f,  0.4774f },
{  0.0489f,  0.4583f,  0.4420f,  0.1117f, -0.2788f, -0.2301f, -0.1932f, -0.4608f,  0.0235f, -0.3540f,  0.4354f, -0.0101f,  0.1331f,  0.3887f, -0.4932f }
};

static float lattice_coeff_2[MAX_NO_LATTICE_SEEDS][DECORR_FILTER_ORDER_BAND_2] = {
{ -0.1752f,  0.1654f, -0.0749f, -0.2015f,  0.0252f, -0.0046f },
{  0.4971f, -0.2105f, -0.4137f, -0.1850f,  0.2221f,  0.3854f },
{  0.4634f,  0.3522f,  0.4063f,  0.4550f, -0.4505f, -0.4529f },
{  0.3080f,  0.3154f,  0.0978f,  0.0724f,  0.1062f,  0.3840f },
{ -0.1464f, -0.1539f,  0.1191f, -0.3026f,  0.2561f,  0.3713f },
{  0.1747f,  0.2682f, -0.2343f,  0.4958f, -0.4733f,  0.4257f },
{ -0.4940f,  0.1058f, -0.2767f, -0.0963f, -0.4389f,  0.1026f },
{  0.0301f, -0.0336f, -0.0461f,  0.4854f,  0.3068f, -0.4624f },
{  0.1232f, -0.0505f,  0.0823f,  0.1865f,  0.1451f,  0.4782f },
{ -0.2890f,  0.2020f,  0.4111f, -0.4178f, -0.2513f,  0.4822f }
};

static float lattice_coeff_3[MAX_NO_LATTICE_SEEDS][DECORR_FILTER_ORDER_BAND_3] = {
{  0.1358f, -0.0373f,  0.0357f },
{ -0.4428f, -0.2286f, -0.4690f },
{  0.4041f, -0.2044f,  0.4640f },
{ -0.4388f,  0.3804f, -0.4791f },
{  0.3004f,  0.2375f,  0.4400f },
{  0.4334f,  0.3321f, -0.1605f },
{ -0.1511f,  0.0254f, -0.1600f },
{ -0.3740f,  0.1246f,  0.2468f },
{ -0.4942f, -0.3039f,  0.4780f },
{  0.3816f, -0.3901f, -0.4434f }
};

static float lattice_deltaPhi[MAX_NO_LATTICE_SEEDS][DECORR_FILTER_ORDER_BAND_0] = {
{  1.7910f,  0.4357f,  1.1439f,  0.9161f,  1.6801f,  1.4365f,  0.8604f,  0.0349f,  1.5483f,  0.8382f,  1.1601f,  1.4928f,  1.7376f,  1.3915f,  0.3323f,  0.7647f,  1.7633f,  1.7283f,  0.7733f,  1.6845f },
{  0.1091f,  0.6651f,  1.5328f,  0.0186f,  0.2618f,  0.3822f,  0.3746f,  1.1381f,  0.5131f,  0.3748f,  0.0288f,  1.4077f,  0.8390f,  1.7564f,  0.8784f,  0.7891f,  1.5951f,  0.9899f,  0.3820f,  1.2669f },
{  1.5798f,  0.0370f,  1.2842f,  0.7153f,  1.5679f,  0.9478f,  1.3373f,  0.8084f,  0.5742f,  0.3575f,  0.3646f,  1.2860f,  0.5707f,  1.0210f,  0.2844f,  1.3155f,  0.7132f,  1.6211f,  1.6091f,  1.1188f },
{  0.9360f,  1.6960f,  1.5487f,  1.2156f,  1.5418f,  1.2445f,  0.6446f,  0.5461f,  0.6431f,  1.0067f,  1.3706f,  0.5830f,  1.5805f,  1.0708f,  0.6982f,  1.3246f,  1.0303f,  0.8386f,  1.3092f,  1.1711f },
{  1.4982f,  1.8036f,  0.9851f,  1.6590f,  0.3260f,  1.8468f,  0.5117f,  0.4756f,  1.6507f,  1.3898f,  0.2573f,  0.0222f,  1.6850f,  0.3754f,  0.5631f,  1.2468f,  0.5361f,  0.8845f,  0.1221f,  1.8630f },
{  1.0985f,  0.7983f,  0.9717f,  0.6295f,  0.8160f,  0.4259f,  1.0929f,  1.4333f,  0.9987f,  1.2074f,  0.3941f,  0.7159f,  1.4765f,  1.2834f,  0.8691f,  1.0703f,  1.4971f,  0.1116f,  1.1364f,  0.0948f },
{  0.7830f,  0.5749f,  1.6481f,  0.0283f,  1.4476f,  1.8300f,  1.8663f,  1.4870f,  0.8269f,  0.9393f,  0.4033f,  1.2130f,  0.6033f,  1.8097f,  1.3697f,  0.7765f,  1.4035f,  0.5051f,  0.8292f,  1.7594f },
{  1.2881f,  0.4007f,  1.5819f,  1.1852f,  0.2522f,  0.3904f,  1.1445f,  1.1873f,  0.6983f,  1.0841f,  0.8509f,  0.0827f,  0.0512f,  0.5894f,  0.0242f,  0.7238f,  1.2876f,  0.1750f,  0.0666f,  1.1543f },
{  1.1471f,  0.0297f,  0.0308f,  0.3583f,  1.1063f,  0.1085f,  0.6928f,  1.1903f,  1.3527f,  1.3057f,  0.1585f,  0.8564f,  0.8328f,  0.6659f,  0.2895f,  1.2736f,  1.3180f,  1.3713f,  0.9017f,  1.0459f },
{  0.2282f,  0.8497f,  1.3494f,  1.6830f,  0.5148f,  0.4802f,  1.6316f,  0.4380f,  1.5171f,  1.7123f,  0.4371f,  0.4511f,  0.0938f,  0.1478f,  1.2079f,  0.3598f,  1.5907f,  0.3278f,  0.3219f,  1.8742f }
};


/* Decorrelation Level                   22   14  11  10   9   8   7   5   2*/
const unsigned int NumDecorr[4][9] =  { {11,  11, 11, 10,  9,  8,  7,  5,  2},  /* Level 0 */
                                        {   9,   7,  6,  5,  5,  4,  4,  3,  1},  /* Level 1 */
                                        {   7,   5,  5,  4,  3,  3,  3,  2,  0},  /* Level 2 */
                                        {   5,   4,  4,  3,  3,  2,  1,  1,  0}   /* Level 3 */
                                     
};


typedef struct DECORR_FILTER_INSTANCE
{
    int    length;
    int    complex;

    float *stateReal;
    float *stateImag;


    float *numeratorReal;
    float *numeratorImag;        /*!< used only for dec type=1 */

    float *denominatorReal;
    float *denominatorImag;      /*!< used only for dec type=1 */


    int   noSampleDelay;
    int   delayBufferIndex;
    float *DelayBufferReal;       /*!< Real part delay buffer */
    float *DelayBufferImag;       /*!< Imaginary part delay buffer */
} DECORR_FILTER_INSTANCE;

typedef struct DUCKER_INSTANCE
{
    int            hybridBands;
    int            parameterBands;
    int            partiallyComplex;
    int            psFlag;
    
    float          alpha;
    float          gamma;
    float          abs_thr;
    float          SmoothDirectNrg[SAOC_MAX_PARAMETER_BANDS];
    float          SmoothReverbNrg[SAOC_MAX_PARAMETER_BANDS];
    /*
      parametric stereo
    */
    float          peakDecay[SAOC_MAX_PARAMETER_BANDS];
    float          peakDiff[SAOC_MAX_PARAMETER_BANDS];

} DUCKER_INSTANCE;




static void ConvertLatticeCoefsReal
(
    float       const * const rfc,
    float             * const apar,
    int                 const order
);

static void ConvertLatticeCoefsComplex
(
    float       const * const rfcReal,
    float       const * const rfcImag,
    float             * const aparReal,
    float             * const aparImag,
    int                 const order
);

static int DecorrFilterCreate
(
    DECORR_FILTER_INSTANCE    ** const selfPtr,
    int                          const hybridBand,
    int                          const partiallyComplex
);

static void DecorrFilterInit
(
    DECORR_FILTER_INSTANCE     * const selfPtr,
    int                          const decorr_seed,
    int                          const qmf_band,
    int                          const reverb_band,
    int                          const useFractDelay,
    int                          noSampleDelay,
    int                          lowDelay  
);

static void Convolute
(
    float                const * const xReal,
    float                const * const xImag,
    int                          const xLength,
    float                const * const yReal,
    float                const * const yImag,
    int                          const yLength,
    float                      * const zReal,
    float                      * const zImag,
    int                        * const zLength
);

static void DecorrFilterInitPS
(
    DECORR_FILTER_INSTANCE     * const self,
    int                          const hybridBand,
    int                          const reverbBand,
    int                          noSampleDelay,
    QMFLIB_HYBRID_FILTER_MODE    hybridMode,
    int                          lowDelay);


static void DecorrFilterDestroy
(
    DECORR_FILTER_INSTANCE    ** const selfPtr 
);

static void DecorrFilterApply
(
    DECORR_FILTER_INSTANCE     * const self,
    float                        const inputReal,
    float                        const inputImag,
    float                      * const outputReal,
    float                      * const outputImag
);

static void DecorrFilterApplyReal
(
    DECORR_FILTER_INSTANCE     * const self,
    float                        const inputReal,
    float                      * const outputReal
);

static int DuckerCreate
(
    DUCKER_INSTANCE **  const selfPtr
);


static void DuckerInit
(
    DUCKER_INSTANCE *   const selfPtr,
    int                 const hybridBands,
    int                 partiallyComplex,
    int                 psFlag
);



static void DuckerDestroy
(
    DUCKER_INSTANCE ** const selfPtr
);

static void DuckerApply
(
    DUCKER_INSTANCE * const self,
    float              const inputReal[SAOC_MAX_HYBRID_BANDS],
    float              const inputImag[SAOC_MAX_HYBRID_BANDS],
    float                    outputReal[SAOC_MAX_HYBRID_BANDS],
    float                    outputImag[SAOC_MAX_HYBRID_BANDS],
    int                      startHybBand,
    int                      lowDelay

);

static void DuckerApplyPS
(
    DUCKER_INSTANCE * const self,
    float              const inputReal[SAOC_MAX_HYBRID_BANDS],
    float              const inputImag[SAOC_MAX_HYBRID_BANDS],
    float                    outputReal[SAOC_MAX_HYBRID_BANDS],
    float                    outputImag[SAOC_MAX_HYBRID_BANDS],
    int                      startHybBand,
    int                      lowDelay

);



static void ConvertLatticeCoefsReal
(
    float       const * const rfc,
    float             * const apar,
    int                 const order
)
{
    int     i, j;
    float   tmp[MAX_DECORR_FILTER_ORDER+1];
    
    apar[0] = 1.0f;
    for (i = 0; i < order; i++)
    {
        apar[i+1] = rfc[i];
        for (j = 0; j < i; j++)
        {
            apar[j+1] = tmp[j] + rfc[i] * tmp[i-j-1];
        }
        for (j = 0; j <= i; j++)
        {
            tmp[j] = apar[j+1];        
        }
    }
}

static void ConvertLatticeCoefsComplex
(
    float       const * const rfcReal,
    float       const * const rfcImag,
    float             * const aparReal,
    float             * const aparImag,
    int                 const order
)
{
    int     i, j;
    float   tmpReal[MAX_DECORR_FILTER_ORDER+1];
    float   tmpImag[MAX_DECORR_FILTER_ORDER+1];
    
    aparReal[0] = 1.0f;
    aparImag[0] = 0.0f;
    for (i = 0; i < order; i++)
    {
        aparReal[i+1] = rfcReal[i];
        aparImag[i+1] = rfcImag[i];
        for (j = 0; j < i; j++)
        {
            aparReal[j+1] = tmpReal[j] + rfcReal[i] * tmpReal[i-j-1] + rfcImag[i] * tmpImag[i-j-1];
            aparImag[j+1] = tmpImag[j] - rfcReal[i] * tmpImag[i-j-1] + rfcImag[i] * tmpReal[i-j-1];
        }
        for (j = 0; j <= i; j++)
        {
            tmpReal[j] = aparReal[j+1];        
            tmpImag[j] = aparImag[j+1];        
        }
    }
}


/*
    Memory requirements
  
    Partially complex mode, either lattice or ps decorrelator, decorrConfig don't care
    ----------------------------------------------------------------------------------

    lattice delay        ps delay  
     0..13: 7             0..13: 8
    14..29: 2            14..41: 14
    30..70: 1            42..70: 1
     
    lattice length       ps length
     0..13: 16            0..13: 20
    14..29:  7           14..70:  0 
    30..70:  4

    HQ mode, only lattice decorrelator
    -----------------------------------

    decorrConfig 0       decorrConfig 1          decorrConfig 2
  
    lattice delay        lattice delay           lattice delay  
     0.. 7: 8             0.. 7: 8                0.. 20: 7
     8..20: 7             8..55: 7               21..70: 2
    21..29: 2            56..70: 2                   
    30..70: 1                        
       
    lattice length       lattice length          lattice length 
     0.. 7: 21            0.. 7: 21                0..20: 16
     8..20: 16            8..55: 16               21..70: 7
    21..29: 7            56..70: 7                    
    30..70: 4                    

*/



    
static int DecorrFilterCreate(DECORR_FILTER_INSTANCE    ** const selfPtr,
                              int                          const hybridBand,
                              int                          const partiallyComplex)
{
  int errorCode = ERROR_NONE;

  DECORR_FILTER_INSTANCE *self      = NULL;
  int maxLength=MAX_DECORR_FILTER_ORDER+1;
  int maxNoSampleDelay=MAX_NO_TIME_SLOTS_DELAY;

  if(partiallyComplex){
    if(hybridBand >29)
      maxLength=DECORR_FILTER_ORDER_BAND_3+1;  /* lattice is max */
    else if (hybridBand >13)
      maxLength=DECORR_FILTER_ORDER_BAND_2+1;  /* lattice is max */
    else
      maxLength=MAX_DECORR_FILTER_ORDER;       /* ps is max */
    
    if(hybridBand >41)
      maxNoSampleDelay=1;                      /* ps is max */
    else if(hybridBand >13)
      maxNoSampleDelay=14;                     /* ps is max */
    else
      maxNoSampleDelay=8;                      /* ps is max */
  }
  else
  {
    /* worst case is decorrConfig 1 */
    if(hybridBand >55){
      maxLength=DECORR_FILTER_ORDER_BAND_2+1;
      maxNoSampleDelay= 2;
    }
    else if(hybridBand >7){
      maxLength=DECORR_FILTER_ORDER_BAND_1+1;
      maxNoSampleDelay= 7;
    }
    else{

     maxLength=DECORR_FILTER_ORDER_BAND_0+1;
      maxNoSampleDelay= 8;
    }
  }

  self = ( DECORR_FILTER_INSTANCE* )calloc( 1, sizeof( *self ) );

  if ( self == NULL ){
        errorCode = ERROR_NO_FREE_MEMORY;
  }
  if ( errorCode == ERROR_NONE ){
   
    self->numeratorReal = ( float* )calloc(maxLength, sizeof( *self->numeratorReal ) );
    self->denominatorReal = ( float* )calloc(maxLength, sizeof( *self->denominatorReal ) );
  
    
    if(partiallyComplex){ /* complex decorrelation, fractional delay used only for ps decorrelator*/
      self->numeratorImag = ( float* )calloc(maxLength, sizeof( *self->numeratorImag ) );
      self->denominatorImag = ( float* )calloc(maxLength, sizeof( *self->denominatorImag ) );
    }

    self->stateReal = ( float* )calloc(maxLength, sizeof( *self->stateReal ) );
    self->stateImag = ( float* )calloc(maxLength, sizeof( *self->stateImag ) );

    self->DelayBufferReal = ( float* )calloc(maxNoSampleDelay,sizeof(*self->DelayBufferReal));
    self->DelayBufferImag = ( float* )calloc(maxNoSampleDelay,sizeof(*self->DelayBufferImag));
  }
  if ( errorCode != ERROR_NONE ){
    DecorrFilterDestroy( &self);
  }

  *selfPtr = self;

  return(errorCode);
}





/*******************************************************************************
*******************************************************************************/
static void DecorrFilterInit
(
    DECORR_FILTER_INSTANCE    *  const self,
    int                          const decorr_seed,
    int                          const qmf_band,
    int                          const reverb_band,
    int                          const useFractDelay,
    int                          noSampleDelay,
    int                          lowDelay)
 
{
    int           i;
    float         *lattice_coeff = NULL;
    float         lattice_coeffReal[MAX_DECORR_FILTER_ORDER];
    float         lattice_coeffImag[MAX_DECORR_FILTER_ORDER];

   
    switch (reverb_band)
    {
        case 0: self->length  = DECORR_FILTER_ORDER_BAND_0+1;
   				lattice_coeff = &(lattice_coeff_0[decorr_seed][0]);
		break;
        case 1: self->length  = DECORR_FILTER_ORDER_BAND_1+1;
          if(!lowDelay)
					  lattice_coeff = &(lattice_coeff_1_STD[decorr_seed][0]);
          else
            lattice_coeff = &(lattice_coeff_1_LD[decorr_seed][0]);
 		break;
        case 2: self->length =  DECORR_FILTER_ORDER_BAND_2+1;
                lattice_coeff = &(lattice_coeff_2[decorr_seed][0]);
        break;
        case 3: self->length =  DECORR_FILTER_ORDER_BAND_3+1;
                lattice_coeff = &(lattice_coeff_3[decorr_seed][0]);
        break;
    }
        
    if(useFractDelay) /* do complex decorrelation, not used for lattice decorrelators*/
    { 

        for ( i = 0; i < self->length-1; i++ )
        {
            lattice_coeffReal[i] =  (float)cos ( qmf_band * 0.5f * lattice_deltaPhi[decorr_seed][i] ) * lattice_coeff[i];
            lattice_coeffImag[i] =  (float)sin ( qmf_band * 0.5f * lattice_deltaPhi[decorr_seed][i] ) * lattice_coeff[i];
        }

        /* calculate complex IIR Filter coefficients */
        ConvertLatticeCoefsComplex (lattice_coeffReal, lattice_coeffImag, self->denominatorReal, self->denominatorImag, self->length-1);
        
        for ( i = 0; i < self->length; i++ )
        {
            self->numeratorReal[i] = self->denominatorReal[self->length-1 - i];
            self->numeratorImag[i] = -self->denominatorImag[self->length-1 - i];
        }
        self->complex = 1;
    }
    else
    {
        /* calculate real IIR Filter coefficients */
      ConvertLatticeCoefsReal (lattice_coeff, self->denominatorReal, self->length-1);

      for ( i = 0; i < self->length; i++ )
      {
        self->numeratorReal[i] = self->denominatorReal[self->length-1 - i];
      }
      self->complex = 0;
    }
    memset(self->stateReal,0, self->length*sizeof( *self->stateReal ));
    memset(self->stateImag,0, self->length*sizeof( *self->stateImag ));


    
    self->noSampleDelay    = noSampleDelay;
    self->delayBufferIndex = 0;
  
    memset(self->DelayBufferReal,0,noSampleDelay*sizeof(*self->DelayBufferReal));
    memset(self->DelayBufferImag,0,noSampleDelay*sizeof(*self->DelayBufferImag));
}

/*******************************************************************************
*******************************************************************************/
static void Convolute
(
    float const * const xReal,
    float const * const xImag,
    int           const xLength,
    float const * const yReal,
    float const * const yImag,
    int           const yLength,
    float       * const zReal,
    float       * const zImag,
    int         * const zLength
)
{
    int   i;
    int   j;
    int   k;
    int   count;

    *zLength = xLength + yLength - 1;

    for ( i = 0; i < *zLength; i++ )
    {
        k     = ( i < yLength ) ? i : yLength - 1;
        j     = i - k;
        count = min( k + 1, xLength );

        zReal[i] = 0.0f;
        zImag[i] = 0.0f;

        for ( ; j < count; j++, k-- )
        {
            zReal[i] += xReal[j] * yReal[k] - xImag[j] * yImag[k];
            zImag[i] += xReal[j] * yImag[k] + xImag[j] * yReal[k];
        }
    }
}

/*******************************************************************************
*******************************************************************************/
static void DecorrFilterInitPS
(
    DECORR_FILTER_INSTANCE     * const self,
    int                          const hybridBand,
    int                          const reverbBand,
    int                          noSampleDelay,
    QMFLIB_HYBRID_FILTER_MODE    hybridMode,
    int                          lowDelay)
{
    int                     i;
    int                     qmfBand;
    float                   centerFreq;
    float                   decay;
    float                   temp0Real[MAX_DECORR_FILTER_ORDER];
    float                   temp0Imag[MAX_DECORR_FILTER_ORDER];
    float                   temp1Real[MAX_DECORR_FILTER_ORDER];
    float                   temp1Imag[MAX_DECORR_FILTER_ORDER];
    float                   numReal[MAX_DECORR_FILTER_ORDER];
    float                   numImag[MAX_DECORR_FILTER_ORDER];
    float                   denReal[MAX_DECORR_FILTER_ORDER];
    float                   denImag[MAX_DECORR_FILTER_ORDER];
    int                     length;
    float                   fract0Real;
    float                   fract0Imag;
    float                   temp;

   
    if ( reverbBand == 0 ){
      self->length       = MAX_DECORR_FILTER_ORDER;

      qmfBand = QMFlib_GetQmfSubband( hybridBand, hybridMode, lowDelay );

      if ( qmfBand < SAOC_QMF_BANDS_TO_HYBRID )
      {
        if(!lowDelay)         
          centerFreq = hybridCenterFreq_STD[hybridBand];
        else
          centerFreq = hybridCenterFreq_LD[hybridBand];
       
      }
      else
      {
          centerFreq = qmfBand + 0.5f;
      }

      decay = 1.0f + PS_REVERB_SLOPE * ( PS_REVERB_DECAY_CUTOFF - hybridBand );

      if ( decay > 1.0f )
      {
          decay = 1.0f;
      }

      memset( temp0Real, 0, sizeof( temp0Real ) );
      memset( temp0Imag, 0, sizeof( temp0Imag ) );

      temp0Real[0] = ( float ) -exp( -3.0f / PS_REVERB_SERIAL_TIME_CONST ) * decay;
      temp0Real[3] = ( float ) cos( -SAOC_PI * centerFreq * 0.43f );
      temp0Imag[3] = ( float ) sin( -SAOC_PI * centerFreq * 0.43f );

      memset( temp1Real, 0, sizeof( temp1Real ) );
      memset( temp1Imag, 0, sizeof( temp1Imag ) );

      temp1Real[0] = ( float ) -exp( -4.0f / PS_REVERB_SERIAL_TIME_CONST ) * decay;
      temp1Real[4] = ( float ) cos( -SAOC_PI * centerFreq * 0.75f );
      temp1Imag[4] = ( float ) sin( -SAOC_PI * centerFreq * 0.75f );

      Convolute( temp0Real, temp0Imag, 4, temp1Real, temp1Imag, 5, numReal, numImag, &length );

      temp0Real[3] *= temp0Real[0];
      temp0Imag[3] *= temp0Real[0];
      temp0Real[0]  = 1.0f;

      temp1Real[4] *= temp1Real[0];
      temp1Imag[4] *= temp1Real[0];
      temp1Real[0]  = 1.0f;

      Convolute( temp0Real, temp0Imag, 4, temp1Real, temp1Imag, 5, denReal, denImag, &length );

      memset( temp0Real, 0, sizeof( temp0Real ) );
      memset( temp0Imag, 0, sizeof( temp0Imag ) );

      temp0Real[0] = ( float ) -exp( -5.0f / PS_REVERB_SERIAL_TIME_CONST ) * decay;
      temp0Real[5] = ( float ) cos( -SAOC_PI * centerFreq * 0.347f );
      temp0Imag[5] = ( float ) sin( -SAOC_PI * centerFreq * 0.347f );

      Convolute( temp0Real, temp0Imag, 6, numReal, numImag, 8, self->numeratorReal, self->numeratorImag, &self->length );

      temp0Real[5] *= temp0Real[0];
      temp0Imag[5] *= temp0Real[0];
      temp0Real[0]  = 1.0f;

      Convolute( temp0Real, temp0Imag, 6, denReal, denImag, 8, self->denominatorReal, self->denominatorImag, &self->length );

      fract0Real = ( float ) cos( -SAOC_PI * centerFreq * 0.39f );
      fract0Imag = ( float ) sin( -SAOC_PI * centerFreq * 0.39f );

      for ( i = 0; i < self->length; i++ )
      {
          temp                   = self->numeratorReal[i] * fract0Real - self->numeratorImag[i] * fract0Imag;
          self->numeratorImag[i] = self->numeratorImag[i] * fract0Real + self->numeratorReal[i] * fract0Imag;
          self->numeratorReal[i] = temp;
      }

      self->complex = 1;
    }
    else
      self->length=0;
    
    if (self->length > 0 ){
        memset(self->stateReal,0, self->length*sizeof( *self->stateReal));
        memset(self->stateImag,0, self->length*sizeof( *self->stateImag));
    }

    self->noSampleDelay    = noSampleDelay;
    self->delayBufferIndex = 0;

    memset(self->DelayBufferReal,0, noSampleDelay*sizeof(*self->DelayBufferReal));
    memset(self->DelayBufferImag,0, noSampleDelay*sizeof(*self->DelayBufferImag));
}

/*******************************************************************************
*******************************************************************************/
static void DecorrFilterDestroy
(
    DECORR_FILTER_INSTANCE ** const selfPtr
)
{
    DECORR_FILTER_INSTANCE *self = *selfPtr;

    if ( self != NULL )
    {
        if (self->complex)
        {
            if ( self->numeratorReal != NULL )
            {
                free( self->numeratorReal );
            }

            if ( self->numeratorImag != NULL )
            {
                free( self->numeratorImag );
            }

            if ( self->denominatorReal != NULL )
            {
                free( self->denominatorReal );
            }

            if ( self->denominatorImag != NULL )
            {
                free( self->denominatorImag );
            }
        }
        else
        {
            if ( self->numeratorReal != NULL )
            {
                free( self->numeratorReal );
            }

            if ( self->denominatorReal != NULL )
            {
                free( self->denominatorReal );
            }
        }

        if ( self->stateReal != NULL )
        {
            free( self->stateReal );
        }

        if ( self->stateImag != NULL )
        {
            free( self->stateImag );
        }

        if(self->DelayBufferReal != NULL)
          free(self->DelayBufferReal);

        if(self->DelayBufferImag != NULL)
          free(self->DelayBufferImag);

        free( self );
    }

    *selfPtr = NULL;
}

/*******************************************************************************
*******************************************************************************/
static void DecorrFilterApply
(
    DECORR_FILTER_INSTANCE       * const self,
    float                         const inputReal,
    float                         const inputImag,
    float                        * const outputReal,
    float                        * const outputImag
)
{
    float  xReal;
    float  xImag;
    float  yReal;
    float  yImag;
    int    j;
   
    if ( self->length > 0 ){
      if (self->complex)
      {
  
        xReal         = self->DelayBufferReal[self->delayBufferIndex];
        xImag         = self->DelayBufferImag[self->delayBufferIndex];
        yReal         = self->stateReal[0] + xReal * self->numeratorReal[0] - xImag * self->numeratorImag[0];
        yImag         = self->stateImag[0] + xReal * self->numeratorImag[0] + xImag * self->numeratorReal[0];
        *outputReal = yReal;
        *outputImag = yImag;

        for ( j = 1; j < self->length; j++ )
        {
            self->stateReal[j - 1]  = self->stateReal[j];
            self->stateImag[j - 1]  = self->stateImag[j];

            self->stateReal[j - 1] += xReal * self->numeratorReal[j] - xImag * self->numeratorImag[j];
            self->stateImag[j - 1] += xReal * self->numeratorImag[j] + xImag * self->numeratorReal[j];

            self->stateReal[j - 1] -= yReal * self->denominatorReal[j] - yImag * self->denominatorImag[j];
            self->stateImag[j - 1] -= yReal * self->denominatorImag[j] + yImag * self->denominatorReal[j];
        }
      }
      else
      {
        xReal         = self->DelayBufferReal[self->delayBufferIndex];
        xImag         = self->DelayBufferImag[self->delayBufferIndex];;
        yReal         = self->stateReal[0] + xReal * self->numeratorReal[0];
        yImag         = self->stateImag[0] + xImag * self->numeratorReal[0];
        *outputReal = yReal;
        *outputImag = yImag;

        for ( j = 1; j < self->length; j++ ){
            self->stateReal[j - 1] = self->stateReal[j] + self->numeratorReal[j] * xReal - self->denominatorReal[j] * yReal;
            self->stateImag[j - 1] = self->stateImag[j] + self->numeratorReal[j] * xImag - self->denominatorReal[j] * yImag;
        }

        
      }
    }
    else{
      *outputReal = self->DelayBufferReal[self->delayBufferIndex];
      *outputImag = self->DelayBufferImag[self->delayBufferIndex];
    }
    
    /*  
        update delay buffers
    */
    self->DelayBufferReal[self->delayBufferIndex] = inputReal;
    self->DelayBufferImag[self->delayBufferIndex] = inputImag;
    self->delayBufferIndex = (self->delayBufferIndex + 1) % self->noSampleDelay;
}


static void DecorrFilterApplyReal
(
 DECORR_FILTER_INSTANCE       * const self,
 float                         const inputReal,
 float                        * const outputReal

 )
{
    float  xReal;
    float  yReal;
    int    j;
  
    if ( self->length > 0 ){
      if (self->complex)
      {

        xReal         = self->DelayBufferReal[self->delayBufferIndex];
        yReal         = self->stateReal[0] + xReal * self->numeratorReal[0];
        *outputReal = yReal;

        for ( j = 1; j < self->length; j++ )
        {
            self->stateReal[j - 1]  = self->stateReal[j];
            self->stateReal[j - 1] += xReal * self->numeratorReal[j];
            self->stateReal[j - 1] -= yReal * self->denominatorReal[j];
        }
      }
      else
      {
        xReal         = self->DelayBufferReal[self->delayBufferIndex];
        yReal         = self->stateReal[0] + xReal * self->numeratorReal[0];
        *outputReal = yReal;

        for ( j = 1; j < self->length; j++ )
        {
            self->stateReal[j - 1] = self->stateReal[j] + self->numeratorReal[j] * xReal - self->denominatorReal[j] * yReal;
        }
      }
    }
    else{
      *outputReal = self->DelayBufferReal[self->delayBufferIndex];
    }
    
    /*  
        update delay buffers
    */
    self->DelayBufferReal[self->delayBufferIndex] = inputReal;
    self->delayBufferIndex = (self->delayBufferIndex + 1) % self->noSampleDelay;
}

/*******************************************************************************
*******************************************************************************/
static int DuckerCreate
(
    DUCKER_INSTANCE  ** const self
)
{
    int               errorCode = ERROR_NONE;

    *self = calloc( 1, sizeof(DUCKER_INSTANCE));

    if ( *self == NULL )
    {
        errorCode = ERROR_NO_FREE_MEMORY;
        DuckerDestroy(self);

    }
    return errorCode;
}




/*******************************************************************************
*******************************************************************************/
static void DuckerInit
(
    DUCKER_INSTANCE  *  const self,
    int                 const hybridBands,
    int                 partiallyComplex,
    int                 psFlag
)
{
    self->alpha = DUCK_ALPHA;
    self->gamma = DUCK_GAMMA;
    self->abs_thr =  SAOC_ABS_THR;
    self->hybridBands = hybridBands;
    self->parameterBands = SAOC_MAX_PARAMETER_BANDS;
    self->partiallyComplex = partiallyComplex;
    self->psFlag= psFlag;
  
}

/*******************************************************************************
*******************************************************************************/
static void DuckerDestroy
(
    DUCKER_INSTANCE ** const self
)
{
  if(self){
    if (*self != NULL )
    {
        free( *self);
        *self = NULL;
    }
  }
   
}

static int hybrid2param_28_STD[SAOC_MAX_HYBRID_BANDS] = {
   1,  0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13,
  14, 15, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 21, 22, 22,
  22, 23, 23, 23, 23, 24, 24, 24, 24, 24, 25, 25, 25, 25, 25, 25,
  26, 26, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 27, 27, 27, 27,
  27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
  27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
  27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
  27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
  27, 27, 27, 27, 27, 27, 27
};
static int hybrid2param_23_LD[SAOC_MAX_HYBRID_BANDS] = {

   0,  1,  2,  3,  4,  5,  6,  7,  8,  
   9, 10, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 16, 17, 17,
  17, 18, 18, 18, 18, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 20,
  21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 22, 22,
  22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
  22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
  22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
  22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
  22, 22, 22, 22, 22, 22, 22
};

static int SpatialDecGetParameterBand(int parameterBands, int hybridBand, int lowDelay) {

  assert(parameterBands == SAOC_MAX_PARAMETER_BANDS);

  if(!lowDelay)
    return hybrid2param_28_STD[hybridBand];
  else
    return hybrid2param_23_LD[hybridBand];
}

/*******************************************************************************
*******************************************************************************/
static void DuckerApply(
                       DUCKER_INSTANCE  * const self,
                       float              const inputReal[SAOC_MAX_HYBRID_BANDS],
                       float              const inputImag[SAOC_MAX_HYBRID_BANDS],
                       float                    outputReal[SAOC_MAX_HYBRID_BANDS],
                       float                    outputImag[SAOC_MAX_HYBRID_BANDS],
                       int                      startHybBand,
                       int                      lowDelay
                       )
{
    float          directNrg[SAOC_MAX_PARAMETER_BANDS] = { 0.0f };
    float          reverbNrg[SAOC_MAX_PARAMETER_BANDS] = { 0.0f };
    float          duckGain[SAOC_MAX_PARAMETER_BANDS] = { 0.0f };
    int            qs = 0;
    int            pb = 0;
    int            startParamBand = 0;

    /* memset( directNrg, 0, sizeof( directNrg ) ); */
    /* memset( reverbNrg, 0, sizeof( reverbNrg ) ); */

    startParamBand =  SpatialDecGetParameterBand(self->parameterBands, startHybBand, lowDelay);
    
    for ( qs = startHybBand; qs < self->hybridBands; qs++ )
    {
        pb =  SpatialDecGetParameterBand(self->parameterBands, qs, lowDelay);
        
        if(!self->partiallyComplex){
          directNrg[pb] += inputReal [qs] * inputReal [qs] + inputImag [qs] * inputImag [qs];
          reverbNrg[pb] += outputReal[qs] * outputReal[qs] + outputImag[qs] * outputImag[qs];
        }
        else{
          if (qs < SAOC_PC_NUM_HYB_BANDS) {
              directNrg[pb] += inputReal [qs] * inputReal [qs] + inputImag [qs] * inputImag [qs];
              reverbNrg[pb] += outputReal[qs] * outputReal[qs] + outputImag[qs] * outputImag[qs];
          }
          else {
              directNrg[pb] += inputReal [qs] * inputReal [qs];
              reverbNrg[pb] += outputReal[qs] * outputReal[qs];
          }
        }
    }

    for ( pb = 0; pb < self->parameterBands; pb++)
    {
        self->SmoothDirectNrg[pb] = self->SmoothDirectNrg[pb] * self->alpha + directNrg[pb] * ( 1.0f - self->alpha );
        self->SmoothReverbNrg[pb] = self->SmoothReverbNrg[pb] * self->alpha + reverbNrg[pb] * ( 1.0f - self->alpha );

        duckGain[pb] = 1.0f;

        if ( self->SmoothReverbNrg[pb] > self->SmoothDirectNrg[pb] * self->gamma )
        {
            duckGain[pb] = ( float ) sqrt( self->SmoothDirectNrg[pb] * self->gamma / ( self->SmoothReverbNrg[pb] + self->abs_thr ) );
        }

        if ( self->SmoothDirectNrg[pb] > self->SmoothReverbNrg[pb] * self->gamma )
        {
            duckGain[pb] = min( 2.0f, ( float ) sqrt( self->SmoothDirectNrg[pb] / ( self->gamma * self->SmoothReverbNrg[pb] + self->abs_thr ) ) );
        }
    }

    for ( qs = startHybBand; qs < self->hybridBands; qs++)
    {
       pb =  SpatialDecGetParameterBand(self->parameterBands, qs, lowDelay);
 
       outputReal[qs] *= duckGain[pb];

        if(!self->partiallyComplex)
          outputImag[qs] *= duckGain[pb];
        else{
          if (qs < SAOC_PC_NUM_HYB_BANDS) 
            outputImag[qs] *= duckGain[pb];
        }
    }
}



/*******************************************************************************
*******************************************************************************/
static void DuckerApplyPS(
                         DUCKER_INSTANCE * const self,
                         float             const inputReal[SAOC_MAX_HYBRID_BANDS],
                         float             const inputImag[SAOC_MAX_HYBRID_BANDS],
                         float                   outputReal[SAOC_MAX_HYBRID_BANDS],
                         float                   outputImag[SAOC_MAX_HYBRID_BANDS],
                         int                     startHybBand,
                         int                     lowDelay
                         )
{
    float             directNrg[SAOC_MAX_PARAMETER_BANDS] = { 0.0f };
    float             duckGain[SAOC_MAX_PARAMETER_BANDS] = { 0.0f };
    int               qs = 0;
    int               pb = 0;
    int               startParamBand = SAOC_MAX_PARAMETER_BANDS;


    /* memset( directNrg, 0, sizeof( directNrg ) ); */

    startParamBand = SpatialDecGetParameterBand(self->parameterBands,startHybBand,lowDelay);


    for ( qs = startHybBand; qs < self->hybridBands; qs++ )
    {
        pb = SpatialDecGetParameterBand(self->parameterBands,qs,lowDelay);
  
        if(!self->partiallyComplex)
          directNrg[pb] += inputReal[qs] * inputReal[qs] + inputImag[qs] * inputImag[qs];
        else{
          if (qs < SAOC_PC_NUM_HYB_BANDS) 
              directNrg[pb] += inputReal[qs] * inputReal[qs] + inputImag[qs] * inputImag[qs];
          else
              directNrg[pb] += inputReal[qs] * inputReal[qs];
        }
    }

    for ( pb = startParamBand; pb < self->parameterBands; pb++)
    {
        self->peakDecay[pb] = max( directNrg[pb], self->peakDecay[pb] * PS_DUCK_PEAK_DECAY_FACTOR );

        self->peakDiff[pb] += PS_DUCK_FILTER_COEFF * ( self->peakDecay[pb] - directNrg[pb] - self->peakDiff[pb] );

        self->SmoothDirectNrg[pb] += PS_DUCK_FILTER_COEFF * ( directNrg[pb] - self->SmoothDirectNrg[pb] );

        if ( ( 1.5f * self->peakDiff[pb] ) > self->SmoothDirectNrg[pb] )
        {
            duckGain[pb] = (float)(self->SmoothDirectNrg[pb] / ( 1.5f * self->peakDiff[pb] + self->abs_thr ));
        }
        else
        {
            duckGain[pb] = 1.0f;
        }
    }

    for ( qs = startHybBand; qs < self->hybridBands; qs++)
    {
         pb =  SpatialDecGetParameterBand(self->parameterBands,qs,lowDelay);

        outputReal[qs] *= duckGain[pb];

        if(!self->partiallyComplex)
          outputImag[qs] *= duckGain[pb];
        else{
          if (qs < SAOC_PC_NUM_HYB_BANDS) 
            outputImag[qs] *= duckGain[pb];
        }
    }
}

int   SAOC_SpatialDecDecorrelateCreate(SAOC_HANDLE_DECORR_DEC *hDecorrDec,
                                  int  maxNumHybridBands,
                                  int  partiallyComplex)
{
  int             errorCode = ERROR_NONE;
  int i;
  
  SAOC_HANDLE_DECORR_DEC self      = NULL;
  self = ( SAOC_HANDLE_DECORR_DEC )calloc( 1, sizeof( *self ) );

  if(self){  

    self->partiallyComplex = partiallyComplex;

    for (i = 0; i < maxNumHybridBands; i++){
    /* create decorrelator */
      errorCode = DecorrFilterCreate( &(self->Filter[i]),i,partiallyComplex);  /* we use only fractional delay in ps case */
    }
    errorCode = DuckerCreate(&self->ducker);
  }
  else
    errorCode=ERROR_NO_FREE_MEMORY;

  if (errorCode){
   SAOC_SpatialDecDecorrelateDestroy( &self, maxNumHybridBands);
  }

  *hDecorrDec = self;

  return (errorCode);

}




int SAOC_SpatialDecDecorrelateInit(SAOC_HANDLE_DECORR_DEC  hDecorrDec,
                             int  subbands,
                             int  seed,
                             int  decorrType,
                             int  decorrConfig,
                             QMFLIB_HYBRID_FILTER_MODE hybridMode,
                             int  lowDelay)
{
  int             errorCode = ERROR_NONE;
  int           i, reverb_band;
  int             *REV_splitfreq;

  switch(decorrConfig)
  {
    case 0:
      if(!lowDelay)
        REV_splitfreq = REV_splitfreq0_STD;
      else
        REV_splitfreq = REV_splitfreq0_LD;
      break;
    case 1:
      REV_splitfreq = REV_splitfreq1;
      break;
    case 2:
      REV_splitfreq = REV_splitfreq2;
      break;
    case 3:
      REV_splitfreq = REV_splitfreq3;
      break;
    default:
      return(ERROR_INVALID_ARGUMENT);
      break;
  }

  if( hDecorrDec->partiallyComplex)
    REV_splitfreq = REV_splitfreqLP;

  if ( decorrType == 1 )
  {
    if( hDecorrDec->partiallyComplex)
      REV_splitfreq = REV_splitfreq1_PS;
    else
    {   
      REV_splitfreq = REV_splitfreq0_PS;
    }
  }

  hDecorrDec->decorr_seed = seed;
  hDecorrDec->numbins    = subbands;
  hDecorrDec->lowDelay   = lowDelay;
    
  for (i = 0; i < hDecorrDec->numbins; i++)
  {
    reverb_band = 0;
    while ( (reverb_band < 3) && (QMFlib_GetQmfSubband(i,hybridMode,lowDelay) >= (REV_splitfreq[reverb_band]-1)) )
     reverb_band++;

      if(decorrType == 1)
        DecorrFilterInitPS(hDecorrDec->Filter[i], 
                           i,
                           reverb_band , 
                           REV_delay_PS[reverb_band],
                           hybridMode,
                           lowDelay);

      else{
			  DecorrFilterInit(hDecorrDec->Filter[i], 
                         hDecorrDec->decorr_seed, 
                         QMFlib_GetQmfSubband(i,hybridMode,lowDelay),  
                         reverb_band, 
                         0,                               
                         REV_delay[reverb_band],
                         lowDelay);
	  }
  }    
  if ( decorrType == 1 )
  {
    DuckerInit( hDecorrDec->ducker, hDecorrDec->numbins, hDecorrDec->partiallyComplex,1);
  }
  else
  {
    if(hDecorrDec->partiallyComplex){
      DuckerInit( hDecorrDec->ducker, hDecorrDec->numbins, hDecorrDec->partiallyComplex, 1);
    } else
    {
      DuckerInit( hDecorrDec->ducker, hDecorrDec->numbins, hDecorrDec->partiallyComplex, 0);
    }
  }
  return (errorCode);
}

void	SAOC_SpatialDecDecorrelateDestroy(   SAOC_HANDLE_DECORR_DEC *hDecorrDec, int nHybridBands)
{
    SAOC_HANDLE_DECORR_DEC self = *hDecorrDec;
    int           i;

    if ( self != NULL )
    {
        for (i = 0; i < nHybridBands; i++)
        {
            if ( self->Filter[i] != NULL )
            {
                DecorrFilterDestroy( &(self->Filter[i]));
            }
        }
      
        DuckerDestroy( &self->ducker );

        free (self);

        *hDecorrDec = NULL;
    }
}

int SAOC_SpatialDecDecorrelateApply(SAOC_HANDLE_DECORR_DEC hDecorrDec,
                                   float InputReal[SAOC_MAX_HYBRID_BANDS],
                                   float InputImag[SAOC_MAX_HYBRID_BANDS],
                                   float OutputReal[SAOC_MAX_HYBRID_BANDS],
                                   float OutputImag[SAOC_MAX_HYBRID_BANDS],
                                   int   startHybBand
                                   )
{
  SAOC_HANDLE_DECORR_DEC self = hDecorrDec;
  int idx;
   
  if ( self != NULL ) {
    int nHybBands = 0;  
    /* copy new samples */

    if(hDecorrDec->partiallyComplex){
      nHybBands= SAOC_PC_NUM_BANDS-SAOC_QMF_BANDS_TO_HYBRID+10;
    } else 
    {
      nHybBands=self->numbins;
    }

    for (idx = startHybBand; idx < nHybBands; idx++) {
          
       DecorrFilterApply( self->Filter[idx],
                           InputReal[idx],
                           InputImag[idx],
                           &OutputReal[idx],
                           &OutputImag[idx]
                           );

    }
    for (; idx < self->numbins; idx++){
        DecorrFilterApplyReal( self->Filter[idx],
                             InputReal[idx],
                             &OutputReal[idx]
                             );
    }
    if(self->ducker->psFlag) {
      DuckerApplyPS( self->ducker, InputReal, InputImag, OutputReal, OutputImag, startHybBand,hDecorrDec->lowDelay);
    } else {
      DuckerApply( self->ducker, InputReal, InputImag, OutputReal, OutputImag, startHybBand,hDecorrDec->lowDelay);
    }

  }
  else
    return ERROR_NO_INSTANCE;

  return ERROR_NONE;
}

int SAOC_GetHybridSubbands(int qmfSubbands, QMFLIB_HYBRID_FILTER_MODE hybridMode, int LdMode)
{

  if (LdMode) {
    return qmfSubbands;
  } else {
    return QMFlib_GetHybridSubbands(qmfSubbands, hybridMode);
  }    
}

static void saoc_pseudoInverse(float inputMatrix[][SAOC_MAX_RENDER_CHANNELS],
                               float outputMatrix[][SAOC_MAX_RENDER_CHANNELS],
                               int nRows,
                               int nCols)
{
  float tmpMatrix[SAOC_MAX_CHANNELS][SAOC_MAX_CHANNELS] = {{0}};
  float in2svd[SAOC_MAX_CHANNELS][SAOC_MAX_CHANNELS] = {{0}};
  float svd_vectors_L[SAOC_MAX_CHANNELS][SAOC_MAX_CHANNELS] = {{0}};
  float svd_vectors_R[SAOC_MAX_CHANNELS][SAOC_MAX_CHANNELS] = {{0}};
  float svd_values[SAOC_MAX_CHANNELS] = {0}; 

	int i,j,l,k,rank;
  float relative_Thresh = 1e-2f;
  
  /* Compute the input matrix for inversion A = M*M' */
  for (i = 0; i< nRows; i++) {
	  for (j=0;  j<nCols; j++) { 
      for (k=0;  k<nCols; k++) { 
			  in2svd[i][j] += inputMatrix[i][k]*inputMatrix[j][k];
      }
		}
	}

  /* Compute the singular value decomposition of the in[][] matrix */
  saoc_SVD(in2svd, svd_vectors_L, svd_values, svd_vectors_R, nRows);
  
  relative_Thresh = svd_values[0]* relative_Thresh;

  rank =  nRows;
	for (i=0; i < nRows; i++ ){
		if (svd_values[i] == 0.0f || svd_values[i] < relative_Thresh){
			rank = i;
			break;
		}
	}  

	/* Compute the pseudoinverse */
	for (i = 0; i < nRows; i++) {
		for (j = 0; j < nRows; j++) {
			for (l = 0; l < rank; l++) {
				/* A = U S V' -> pinv_A = V S^-1 U' */
				tmpMatrix[i][j] += (svd_vectors_R[i][l] * (1.0f / (svd_values[l] + SAOC_EPSILON)) * svd_vectors_L[j][l]); 
			}
		}
	}  

  /* Compute the inverse matrix X = M'(M*M')^-1 */
  for (i = 0; i< nCols; i++) {
	  for (j=0;  j<nRows; j++) { 
      for (k=0;  k<nRows; k++) { 
			  outputMatrix[i][j] += inputMatrix[k][i] * tmpMatrix[k][j];
      }
		}
	}
}

void SAOC_DecorrMpreMpostInit(unsigned int DecorrelationLevel,
                                unsigned int numOutCh,
                                unsigned int numLFEs,
                                int outCICPIndex,
                                CICP2GEOMETRY_CHANNEL_GEOMETRY* saocOutputGeometryInfo,
                                unsigned int *numDecorrelators,
                                float Mpre[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS],
                                float Mpost[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS],
                                float Hproto[SAOC_MAX_RENDER_CHANNELS][2*SAOC_MAX_RENDER_CHANNELS])
{

  float azimuth[SAOC_MAX_RENDER_CHANNELS]   = {0};
  float elevation[SAOC_MAX_RENDER_CHANNELS] = {0};
  unsigned int idx,i,j;  
  int noInterMed = 1;         /* default value */
  float heightScalar = 0.5;   /* default value */


  for (i=0; i<SAOC_MAX_RENDER_CHANNELS; i++) {
    memset(Mpre[i], 0, SAOC_MAX_RENDER_CHANNELS * sizeof(float));
    memset(Mpost[i], 0, SAOC_MAX_RENDER_CHANNELS * sizeof(float));
  }

  if (outCICPIndex == 13) {  /* 22.2 setup*/
    *numDecorrelators = 11;
    for (i=0; i<*numDecorrelators; i++) {
      for (j=0; j<numOutCh ; j++) {
        Mpre[i][j] = PremixingMatrix_11_Decorr_22_Output[i][j];
        Mpost[j][i] = PostmixingMatrix_11_Decorr_22_Output[j][i];
      }
    }
  } else {
    if (numOutCh <= SAOC_MAX_NUM_DECORRELATORS) {
      *numDecorrelators = numOutCh;
      for (i=0; i<*numDecorrelators; i++) {
        Mpre[i][i] = 1.0f;
        Mpost[i][i] = 1.0f;;
      }
    } else { 
      *numDecorrelators = SAOC_MAX_NUM_DECORRELATORS;
      idx = 0;
      for (i=0; i<numOutCh + numLFEs ; i++) {
        if (saocOutputGeometryInfo[i].LFE == 0) {
          azimuth[idx]   = (float) saocOutputGeometryInfo[i].Az;
          elevation[idx] = (float) saocOutputGeometryInfo[i].El;
          idx++;
        }
      }

      saoc_hierChnlGrouping(azimuth, elevation, numOutCh, *numDecorrelators, noInterMed, heightScalar, Mpre);

      saoc_pseudoInverse(Mpre, Mpost, *numDecorrelators, numOutCh);

    }
  }

  for (i=0; i<SAOC_MAX_RENDER_CHANNELS; i++) {
    memset(Hproto[i], 0, 2*SAOC_MAX_RENDER_CHANNELS * sizeof(float));
  }

  for (i=0; i<numOutCh ; i++) {
    Hproto[i][i] = 0.9f;
    Hproto[i][numOutCh + i] = (float)(sqrt(1 - Hproto[i][i] * Hproto[i][i]));
  }
}
