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
 
 Fraunhofer IIS retains full right to modify and use the code for 
 their own purpose, assign or donate the code to a third party and to inhibit third 
 parties from using the code for products that do not conform to MPEG-related ITU 
 Recommendations and/or ISO/IEC International Standards.
 
 This copyright notice must be included in all copies or derivative works. 
 
 Copyright (c) ISO/IEC 2013.
 
 ***********************************************************************************/

#ifndef IAR_CONVERTER_H
#define IAR_CONVERTER_H

#include "iar_rom.h"

/* definitions
 * ************************************************************************** */

/* channel ids */

/*
IMPORTANT/WARNING:
    -the converter implementation assumes that all horizontal channels are:
        IAR_CH_M_000 <= i <= IAR_CH_M_180
    -the converter implementation assumes that all height channels are:
        IAR_CH_U_000 <= i <= IAR_CH_U_180
    Should this be changed, then converter.c needs to be modified!
*/
typedef enum
{
    /* horizontal channels */
    IAR_CH_M_000 =      0,
    IAR_CH_M_L022 =     1,
    IAR_CH_M_R022 =     2,
    IAR_CH_M_L030 =     3,
    IAR_CH_M_R030 =     4,
    IAR_CH_M_L045 =     5,
    IAR_CH_M_R045 =     6,
    IAR_CH_M_L060 =     7,
    IAR_CH_M_R060 =     8,
    IAR_CH_M_L090 =     9,
    IAR_CH_M_R090 =     10,
    IAR_CH_M_L110 =     11,
    IAR_CH_M_R110 =     12,
    IAR_CH_M_L135 =     13,
    IAR_CH_M_R135 =     14,
    IAR_CH_M_L150 =     15,
    IAR_CH_M_R150 =     16,
    IAR_CH_M_180 =      17,
    /* height channels */
    IAR_CH_U_000 =      18,
    IAR_CH_U_L045 =     19,
    IAR_CH_U_R045 =     20,
    IAR_CH_U_L030 =     21,
    IAR_CH_U_R030 =     22,
    IAR_CH_U_L090 =     23,
    IAR_CH_U_R090 =     24,
    IAR_CH_U_L110 =     25,
    IAR_CH_U_R110 =     26,
    IAR_CH_U_L135 =     27,
    IAR_CH_U_R135 =     28,
    IAR_CH_U_180 =      29,
    /* top channel */
    IAR_CH_T_000 =      30,
    /* low channels */
    IAR_CH_L_000 =      31,
    IAR_CH_L_L045 =     32,
    IAR_CH_L_R045 =     33,
    /* low frequency effects */
    IAR_CH_LFE1 =       34,
    IAR_CH_LFE2 =       35,
    /* empty channel */
    IAR_CH_EMPTY =      -1
} iar_converter_chid_t;

/* audio format IDs */

typedef enum
{
    IAR_FORMAT_2_0          = 0,
    IAR_FORMAT_5_1          = 1,
    IAR_FORMAT_5_2_1        = 2,
    IAR_FORMAT_7_1          = 3,
    IAR_FORMAT_7_1_ALT      = 4,
    IAR_FORMAT_8_1          = 5,
    IAR_FORMAT_10_1         = 6,
    IAR_FORMAT_22_2         = 7,
    IAR_FORMAT_9_1          = 8,
    IAR_FORMAT_9_0          = 9,
    IAR_FORMAT_11_1         = 10,
    IAR_FORMAT_12_1         = 11,
    IAR_FORMAT_4_4_0        = 12,
    IAR_FORMAT_4_4_T_0      = 13,
    IAR_FORMAT_14_0         = 14,
    IAR_FORMAT_15_1         = 15,
    IAR_FORMAT_3_0_FC       = 16,
    IAR_FORMAT_3_0_RC       = 17,
    IAR_FORMAT_4_0          = 18,
    IAR_FORMAT_5_0          = 19,
    IAR_FORMAT_6_1          = 20,
    IAR_FORMAT_IN_LISTOFCHANNELS = 21,
    IAR_FORMAT_OUT_LISTOFCHANNELS = 22,
    IAR_NFORMATS            = 23
} iar_converter_formatid_t;

/* downmix processing rules */

typedef enum
{
    /* external rules (indices 0..10) */
    RULE_NOPROC   = 0,  /* no processing channel copy with gain */
    IAR_RULE_EQ1      = 1,  /* eq for front up-median downmix */
    IAR_RULE_EQ2      = 2,  /* eq for surround up-median downmix */
    IAR_RULE_EQ3      = 3,  /* eq for top-up downmix */
    IAR_RULE_EQ4      = 4,  /* eq for top-median downmix */
    IAR_RULE_EQ5      = 5,  /* eq for horizontal channel that is displaced to height */    
#if    IAR                    /* eqs for the immersive audio rendering */
    IAR_RULE_EQVF       = 7,    /* eq for IAR_CH_U_L030, IAR_CH_U_R030, IAR_CH_U_L045, IAR_CH_U_R045 */
    IAR_RULE_EQVB       = 8,    /* eq for IAR_CH_U_L135, IAR_CH_U_R135 */
    IAR_RULE_EQVFC      = 9,    /* eq for IAR_CH_U_000 */
    IAR_RULE_EQVBC      = 10, /* eq for IAR_CH_U_180 */
    IAR_RULE_EQVOG      = 11,    /* eq for IAR_CH_T_000 */
    IAR_RULE_EQVS       = 12, /* eq for IAR_CH_U_L090, IAR_CH_U_R110 */
    IAR_RULE_EQBTM      = 13, /* eq for IAR_CH_L_000, IAR_CH_L_L045, IAR_CH_L_R045 */
    IAR_RULE_EQVBA      = 14, /* eq for IAR_CH_U_L110, IAR_CH_U_R110 */

    IAR_RULE_06_03      = 15, /* eq for IAR_CH_M_L060 and IAR_CH_M_R060 to IAR_CH_M_L030 and IAR_CH_M_R030 */
    IAR_RULE_09_03      = 16, /* eq for IAR_CH_M_L090 and IAR_CH_M_R090 to IAR_CH_M_L030 and IAR_CH_M_R030 */
    IAR_RULE_06_11      = 17, /* eq for IAR_CH_M_L060 and IAR_CH_M_R060 to IAR_CH_M_L110 and IAR_CH_M_R110 */
    IAR_RULE_09_11      = 18, /* eq for IAR_CH_M_L090 and IAR_CH_M_R090 to IAR_CH_M_L110 and IAR_CH_M_R110 */
    IAR_RULE_13_11      = 19, /* eq for IAR_CH_M_L135 and IAR_CH_M_R135 to IAR_CH_M_L110 and IAR_CH_M_R110 */
    IAR_RULE_18_11      = 20, /* eq for IAR_CH_M_180                    to IAR_CH_M_L110 and IAR_CH_M_R110 */
    IAR_RULE_REQ        = 21, /* first eq for random setups */
    IAR_N_EQ            = 40, /* number of eqs */

    /* rules only used within converter only */
    IAR_RULE_PANNING  = 100, /* manual amplitude panning (specifying alpha0, alpha) */
    IAR_RULE_TOP2ALLU = 101, /* top channel to all upper channels */
    IAR_RULE_TOP2ALLM = 102, /* top channel to all horizontal channels */
    IAR_RULE_AUTOPAN  = 103, /* automatic amplitude panning (alpha0 defined by destimation */
    IAR_RULE_VIRTUAL  = 104
#else
    IAR_RULE_REQ      = 6,  /* first eq for random setups */
    IAR_N_EQ          = 12, /* number of eqs */
    /* rules only used within converter only */
    IAR_RULE_PANNING  = 100, /* manual amplitude panning (specifying alpha0, alpha) */
    IAR_RULE_TOP2ALLU = 101, /* top channel to all upper channels */
    IAR_RULE_TOP2ALLM = 102, /* top channel to all horizontal channels */
    IAR_RULE_AUTOPAN  = 103  /* automatic amplitude panning (alpha0 defined by destimation */
#endif

                         /* channels, alpha defined by source channels) */
} iar_converter_dmxrulesid_t;


/**
 * Error codes
 * ************************************************************************** */
typedef enum
{
    IAR_FORMAT_CONVERTER_STATUS_OK =              0,      /* success */
    IAR_FORMAT_CONVERTER_STATUS_FAILED =          -1,     /* generic error */
    IAR_FORMAT_CONVERTER_STATUS_MISSING_RULE =    -2,     /* missing downmix rule */
    IAR_FORMAT_CONVERTER_STATUS_INFORMAT =        -3,     /* invalid input format index */
    IAR_FORMAT_CONVERTER_STATUS_OUTFORMAT =       -4,     /* invalid output format index */
    IAR_FORMAT_CONVERTER_STATUS_SFREQ =           -5,     /* invalid sampling frequency */
    IAR_FORMAT_CONVERTER_STATUS_BLOCKLENGTH =     -6,     /* invalid block length */
    IAR_FORMAT_CONVERTER_STATUS_TRIM =            -7,     /* out-of-range trim values */
    IAR_FORMAT_CONVERTER_STATUS_RANDOMIZATION =   -8,     /* randomization out-of-range */
    IAR_FORMAT_CONVERTER_STATUS_BANDS =           -9,     /* bands out-of-range */
} iar_converter_status_t;


/* public data types / structures
 * ************************************************************************** */

/**
 * Converter parameters
 */
#if IAR
#define        IAR_IN_OUT_N        (100)
#else
#define     IAR_IN_OUT_N        (60)
#endif

#define     IAR_MAXBANDS        (100)
#define     IAR_NCHANOUT_MAX    (32)
typedef struct iar_converter_pr
{
    int     nchanin;                    /* number of input channels */
    int     nchanout;                   /* number of output channels */
    float   aeq_max_limit;              /* maximum gain of adaptive eq [lin] */
    float   aeq_min_limit;              /* maximum gain of adaptive eq [lin] */
    int     in_out_src[IAR_IN_OUT_N];       /* in-out conversion source */
    int     in_out_dst[IAR_IN_OUT_N];       /* in-out conversion destination */
    float   in_out_gain[IAR_IN_OUT_N];      /* in-out conversion gain, G_vH [lin] */
    int     in_out_proc[IAR_IN_OUT_N];      /* in-out conversion processing */
    float   eq[IAR_N_EQ][IAR_MAXBANDS];         /* equalizers [lin gains] */
#if IAR
    float   in_out_gainL[IAR_IN_OUT_N];        /* in-out conversion gain, G_vL [lin] */
    float   ieq[IAR_N_EQ][IAR_MAXBANDS];        /* inverse equalizers [lin gains] */

    /* variables for the timbral elevation rendering */
    int     in_out_src2[IAR_IN_OUT_N];      /* in-out conversion source */
    int     in_out_dst2[IAR_IN_OUT_N];      /* in-out conversion destination */
    float   in_out_gain2[IAR_IN_OUT_N];     /* in-out conversion gain [lin] */
    int     in_out_proc2[IAR_IN_OUT_N];     /* in-out conversion processing */
    int     in_out_proc_h2[IAR_IN_OUT_N];   /* in-out conversion processing */
    float   in_out_gaini[IAR_IN_OUT_N];     /* in-out mixing gain [lin] */
#endif
    int     trim_delay[IAR_NCHANOUT_MAX];   /* delay trim [samples] */
    float   trim_gain[IAR_NCHANOUT_MAX];    /* gain trim [lin] */
#ifndef FORMATCONVERTER_LOWCOMPLEXITY   /* RMO code */
    int     in_out_gainComp[IAR_IN_OUT_N];  /* in-out conversion gain compensation */
    float   gainCompValue;              /* gain compensation value */
#endif
} iar_converter_pr_t;




/* API functions
 * ************************************************************************** */

/**
  * Set input format or output format that is not in list of known formats.
  * Each function call either sets input _or_ output format.
  * Allocates memory.
  */
int iar_converter_set_inout_format(
        const int                  inout,           /* in:  0: set input format, 1: set output format */
        const int                  num_channels,    /* in:  number of channels in setup to define */
        const iar_converter_chid_t*    channel_vector   /* in:  vector containing channel enums */
);


/**
 * Initialize audio processing.
 *
 * No memory is allocated, nothing to release.
 */
iar_converter_status_t
iar_converter_init(
        iar_converter_pr_t              *params,        /* out: initialized parameters */
        const iar_converter_formatid_t  input_format,   /* in:  input format id */
        const iar_converter_formatid_t  output_format,  /* in:  output format id */
        const float*                randomization,  /* in:  randomization angles [azi,ele,azi,ele, ... in degrees] */
        const float                 sfreq_Hz,       /* in:  sampling frequency [Hz] */
        const int                   blocklength,    /* in:  processing block length [samples] */
        const int                   nbands,         /* in:  number of filterbank processing bands */
        const float*                bands_nrm,      /* in:  filterbank processing bands center frequencies [normalized frequency] */
        const float*                trim,           /* in:  vector specifying for each channel its radius [m] */
        const int                   trim_max        /* in:  maximum delay that can be used for delay trim [samples] */
);


float iar_peak_filter(
                  const float f,    /* peak frequency [Hz] */
                  const float q,    /* peak Q factor */ 
                  const float g,    /* peak gain */ 
                  const float G,    /* gain */
                  const float b);   /* band center frequency [Hz] */

#endif  /* CONVERTER_H */
