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

#ifndef CONVERTER_H
#define CONVERTER_H

/* definitions
 * ************************************************************************** */
/* #define RM6_INTERNAL_CHANNEL */

/* channel ids */

/*
IMPORTANT/WARNING:
    -the converter implementation assumes that all horizontal channels are:
        CH_M_000 <= i <= CH_M_180
    -the converter implementation assumes that all height channels are:
        CH_U_000 <= i <= CH_U_180
    Should this be changed, then converter.c needs to be modified!
*/
typedef enum
{
    /* horizontal channels */
    CH_M_000 =      0,
    CH_M_L022 =     1,
    CH_M_R022 =     2,
    CH_M_L030 =     3,
    CH_M_R030 =     4,
    CH_M_L045 =     5,
    CH_M_R045 =     6,
    CH_M_L060 =     7,
    CH_M_R060 =     8,
    CH_M_L090 =     9,
    CH_M_R090 =     10,
    CH_M_L110 =     11,
    CH_M_R110 =     12,
    CH_M_L135 =     13,
    CH_M_R135 =     14,
    CH_M_L150 =     15,
    CH_M_R150 =     16,
    CH_M_180 =      17,
    /* height channels */
    CH_U_000 =      18,
    CH_U_L045 =     19,
    CH_U_R045 =     20,
    CH_U_L030 =     21,
    CH_U_R030 =     22,
    CH_U_L090 =     23,
    CH_U_R090 =     24,
    CH_U_L110 =     25,
    CH_U_R110 =     26,
    CH_U_L135 =     27,
    CH_U_R135 =     28,
    CH_U_180 =      29,
    /* top channel */
    CH_T_000 =      30,
    /* low channels */
    CH_L_000 =      31,
    CH_L_L045 =     32,
    CH_L_R045 =     33,
    /* low frequency effects */
    CH_LFE1 =       34,
    CH_LFE2 =       35,
#ifdef RM6_INTERNAL_CHANNEL
    CH_I_CNTR  =    43,
     CH_I_LFE  =    44,
    CH_I_LEFT	 =    45,
    CH_I_RIGHT =    46,
#endif
    /* empty channel */
    CH_EMPTY =      -1
} converter_chid_t;

/* audio format IDs */

typedef enum
{
    FORMAT_2_0          = 0,
    FORMAT_5_1          = 1,
    FORMAT_5_2_1        = 2,
    FORMAT_7_1          = 3,
    FORMAT_7_1_ALT      = 4,
    FORMAT_8_1          = 5,
    FORMAT_10_1         = 6,
    FORMAT_22_2         = 7,
    FORMAT_9_1          = 8,
    FORMAT_9_0          = 9,
    FORMAT_11_1         = 10,
    FORMAT_12_1         = 11,
    FORMAT_4_4_0        = 12,
    FORMAT_4_4_T_0      = 13,
    FORMAT_14_0         = 14,
    FORMAT_15_1         = 15,
    FORMAT_3_0_FC       = 16,
    FORMAT_3_0_RC       = 17,
    FORMAT_4_0          = 18,
    FORMAT_5_0          = 19,
    FORMAT_6_1          = 20,
    FORMAT_IN_LISTOFCHANNELS = 21,
    FORMAT_OUT_LISTOFCHANNELS = 22,
    NFORMATS            = 23
} converter_formatid_t;

/* downmix processing rules */

typedef enum
{
    /* external rules (indices 0..10) */
    RULE_NOPROC   = 0,  /* no processing channel copy with gain */
    RULE_EQ1      = 1,  /* eq for front up-median downmix */
    RULE_EQ2      = 2,  /* eq for surround up-median downmix */
    RULE_EQ3      = 3,  /* eq for top-up downmix */
    RULE_EQ4      = 4,  /* eq for top-median downmix */
    RULE_EQ5      = 5,  /* eq for horizontal channel that is displaced to height */    
    RULE_REQ      = 6,  /* first eq for random setups */
    N_EQ          = 12, /* number of eqs */
    /* rules only used within converter only */
    RULE_PANNING  = 100, /* manual amplitude panning (specifying alpha0, alpha) */
    RULE_TOP2ALLU = 101, /* top channel to all upper channels */
    RULE_TOP2ALLM = 102, /* top channel to all horizontal channels */
    RULE_AUTOPAN  = 103  /* automatic amplitude panning (alpha0 defined by destimation */
                         /* channels, alpha defined by source channels) */
} converter_dmxrulesid_t;


/**
 * Error codes
 * ************************************************************************** */
typedef enum
{
    FORMAT_CONVERTER_STATUS_OK =              0,      /* success */
    FORMAT_CONVERTER_STATUS_FAILED =          -1,     /* generic error */
    FORMAT_CONVERTER_STATUS_MISSING_RULE =    -2,     /* missing downmix rule */
    FORMAT_CONVERTER_STATUS_INFORMAT =        -3,     /* invalid input format index */
    FORMAT_CONVERTER_STATUS_OUTFORMAT =       -4,     /* invalid output format index */
    FORMAT_CONVERTER_STATUS_SFREQ =           -5,     /* invalid sampling frequency */
    FORMAT_CONVERTER_STATUS_BLOCKLENGTH =     -6,     /* invalid block length */
    FORMAT_CONVERTER_STATUS_TRIM =            -7,     /* out-of-range trim values */
    FORMAT_CONVERTER_STATUS_RANDOMIZATION =   -8,     /* randomization out-of-range */
    FORMAT_CONVERTER_STATUS_BANDS =           -9,     /* bands out-of-range */
} converter_status_t;


/* public data types / structures
 * ************************************************************************** */

/**
 * Converter parameters
 */
#define     IN_OUT_N        (60)
#define     MAXBANDS        (100)
#define     NCHANOUT_MAX    (32)
typedef struct converter_pr
{
    int     nchanin;                    /* number of input channels */
    int     nchanout;                   /* number of output channels */
    float   aeq_max_limit;              /* maximum gain of adaptive eq [lin] */
    float   aeq_min_limit;              /* maximum gain of adaptive eq [lin] */
    int     in_out_src[IN_OUT_N];       /* in-out conversion source */
    int     in_out_dst[IN_OUT_N];       /* in-out conversion destination */
    float   in_out_gain[IN_OUT_N];      /* in-out conversion gain [lin] */
    int     in_out_proc[IN_OUT_N];      /* in-out conversion processing */
    float   eq[N_EQ][MAXBANDS];         /* equalizers [lin gains] */
    int     trim_delay[NCHANOUT_MAX];   /* delay trim [samples] */
    float   trim_gain[NCHANOUT_MAX];    /* gain trim [lin] */
#ifndef FORMATCONVERTER_LOWCOMPLEXITY   /* RMO code */
    int     in_out_gainComp[IN_OUT_N];  /* in-out conversion gain compensation */
    float   gainCompValue;              /* gain compensation value */
#endif
} converter_pr_t;




/* API functions
 * ************************************************************************** */

/**
  * Set input format or output format that is not in list of known formats.
  * Each function call either sets input _or_ output format.
  * Allocates memory.
  */
int converter_set_inout_format(
        const int                  inout,           /* in:  0: set input format, 1: set output format */
        const int                  num_channels,    /* in:  number of channels in setup to define */
        const converter_chid_t*    channel_vector   /* in:  vector containing channel enums */
);


/**
 * Initialize audio processing.
 *
 * No memory is allocated, nothing to release.
 */
converter_status_t
converter_init(
        converter_pr_t              *params,        /* out: initialized parameters */
        const converter_formatid_t  input_format,   /* in:  input format id */
        const converter_formatid_t  output_format,  /* in:  output format id */
        const float*                randomization,  /* in:  randomization angles [azi,ele,azi,ele, ... in degrees] */
        const float                 sfreq_Hz,       /* in:  sampling frequency [Hz] */
        const int                   blocklength,    /* in:  processing block length [samples] */
        const int                   nbands,         /* in:  number of filterbank processing bands */
        const float*                bands_nrm,      /* in:  filterbank processing bands center frequencies [normalized frequency] */
        const float*                trim,           /* in:  vector specifying for each channel its radius [m] */
        const int                   trim_max        /* in:  maximum delay that can be used for delay trim [samples] */
);


float peak_filter(
                  const float f,    /* peak frequency [Hz] */
                  const float q,    /* peak Q factor */ 
                  const float g,    /* peak gain */ 
                  const float G,    /* gain */
                  const float b);   /* band center frequency [Hz] */

#endif  /* CONVERTER_H */
