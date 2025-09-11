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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "formatConverter_init.h"

#ifndef min
#define min(a,b) (( a < b ) ? a : b)
#endif

#ifndef round
#define round(x) ((x>=0)?(int)(x+0.5):(int)(x-0.5))
#endif

/* prototypes */

char is_m_channel(const int ch);
char is_u_channel(const int ch);
char exist_channel(const int ch, const int* channels, const int nchan);
void compute_eq(
                const float *bands_nrm,
                const int   nbands,
                const float sfreq_Hz,
                const int   eq_1_idx,
                const float eq_1_strength,
                const int   eq_2_idx,
                const float eq_2_strength,
                float       *eq
                );

int find_channel(const int ch, const int* channels, const int nchan);
void free_inout_format_lists( void );

/* hard coded parameters
 * ************************************************************************** */

#define TC_AEQ_MS           (60.0f)     /* time constant for adaptive eq smoothing [ms] */
#define AEQ_MAX_LIMIT_DB    (8.0f)      /* adaptive equatization gain upper limit [dB] */
#define AEQ_MIN_LIMIT_DB    (-10.0f)    /* adaptive equatization gain upper limit [dB] */
#define SOS_M_S             (340.0f)    /* speed of sound [m/s] */
#define TRIM_EXP            (0.5f)      /* {1 0.5} = {6 3}dB per distance doubling */
#ifndef M_PI
#define M_PI                3.14159265358979323846264338327950288f
#endif

/* parameter range limitations */
#define SFREQ_HZ_MIN                (8000.0f)   /* minimum sampling frequency */
#define SFREQ_HZ_MAX                (384000.0f) /* maximum sampling frequency */
#define NBANDS_MIN                  (10)        /* minimum number of bands */
#define NBANDS_MAX                  (2048)      /* maximum number of bands */
#define BLOCKLENGTH_MIN_MS          (0.167f)    /* minimum blocklength [ms] */
#define BLOCKLENGTH_MAX_MS          (10.0f)     /* maximum blocklength [ms] */
#define TRIM_RATIO_MAX              (4.0f)      /* maximum ratio between largest and smallest radius */
#define TRIM_RADIUS_MIN             (0.4)       /* minimum radius [m] */
#define TRIM_RADIUS_MAX             (200.0)     /* maximum radius [m] */
#define RANDOMIZATION_AZI_MAX       (35.0f)     /* maximum azi deviation [deg] */
#define RANDOMIZATION_ELE_MAX       (55.0f)     /* maximum ele deviation [deg] */
#define RANDOMIZATION_SPK_ANGLE_MIN (15.0f)     /* minimum angle between any loudspeaker pair [deg] */

/* definitions
 * ************************************************************************** */

/* azi/ele angles of the different channels */

const float ch_azi_ele[CH_LFE2+1][2] = {
    {0.0f,      0.0f},
    {+15.0f,    0.0f},
    {-15.0f,    0.0f},
    {+30.0f,    0.0f},
    {-30.0f,    0.0f},
    {+45.0f,    0.0f},
    {-45.0f,    0.0f},
    {+60.0f,    0.0f},
    {-60.0f,    0.0f},
    {+90.0f,    0.0f},
    {-90.0f,    0.0f},
    {+110.0f,   0.0f},
    {-110.0f,   0.0f},
    {+135.0f,   0.0f},
    {-135.0f,   0.0f},
    {+150.0f,   0.0f},
    {-150.0f,   0.0f},
    {180.0f,    0.0f},
    {0.0f,      35.0f},
    {+45.0f,    35.0f},
    {-45.0f,    35.0f},
    {+30.0f,    35.0f},
    {-30.0f,    35.0f},
    {+90.0f,    35.0f},
    {-90.0f,    35.0f},
    {+110.0f,   35.0f},
    {-110.0f,   35.0f},
    {+135.0f,   35.0f},
    {-135.0f,   35.0f},
    {180.0f,    35.0f},
    {0.0f,      90.0f},
    {0.0f,      -15.0f},
    {+45.0f,    -15.0f},
    {-45.0f,    -15.0f},
    {0.0f,      0.0f},
    {0.0f,      0.0f}
};

/* used for checking validity of random setups
   ordering of these triples (elevation angle)
   has to remain the same */
const int ele_ordering[][3] = {
    /* center */
    {CH_L_000,  CH_M_000,   CH_U_000},
    {CH_L_000,  CH_M_L022,  CH_U_000},
    {CH_L_000,  CH_M_R022,  CH_U_000},
    /* front left */
    {CH_L_L045, CH_M_L030,  CH_U_L030},
    {CH_L_L045, CH_M_L030,  CH_U_L045},
    {CH_L_L045, CH_M_L045,  CH_U_L030},
    {CH_L_L045, CH_M_L045,  CH_U_L045},
    {CH_L_L045, CH_M_L060,  CH_U_L030},
    {CH_L_L045, CH_M_L060,  CH_U_L045},
    /* front right */
    {CH_L_R045, CH_M_R030,  CH_U_R030},
    {CH_L_R045, CH_M_R030,  CH_U_R045},
    {CH_L_R045, CH_M_R045,  CH_U_R030},
    {CH_L_R045, CH_M_R045,  CH_U_R045},
    {CH_L_R045, CH_M_R060,  CH_U_R030},
    {CH_L_R045, CH_M_R060,  CH_U_R045},
    /* surround center */
    {CH_M_180,  CH_U_180,   -1},
    /* surround left */
    {CH_M_L090, CH_U_L090,  -1},
    {CH_M_L110, CH_U_L110,  -1},
    {CH_M_L135, CH_U_L135,  -1},
    {CH_M_L090, CH_U_L110,  -1},
    {CH_M_L090, CH_U_L135,  -1},
    {CH_M_L110, CH_U_L090,  -1},
    {CH_M_L110, CH_U_L135,  -1},
    {CH_M_L135, CH_U_L090,  -1},
    {CH_M_L135, CH_U_L135,  -1},
    /* surround right */
    {CH_M_R090, CH_U_R090,  -1},
    {CH_M_R110, CH_U_R110,  -1},
    {CH_M_R135, CH_U_R135,  -1},
    {CH_M_R090, CH_U_R110,  -1},
    {CH_M_R090, CH_U_R135,  -1},
    {CH_M_R110, CH_U_R090,  -1},
    {CH_M_R110, CH_U_R135,  -1},
    {CH_M_R135, CH_U_R090,  -1},
    {CH_M_R135, CH_U_R135,  -1},
    {-1,        -1,         -1},
};

/* number of channels of the formats */

const int format_nchan[NFORMATS-2] = {
    2,
    6,
    8,
    8,
    8,
    9,
    11,
    24,
    10,
    9,
    12,
    13,
    8,
    9,
    14,
    16,
    3,
    3,
    4,
    5,
    7,
};


/* format definitions */
int *format_in_listOfChannels = NULL;
int format_in_listOfChannels_nchan = 0;
int *format_out_listOfChannels = NULL;
int format_out_listOfChannels_nchan = 0;

const int format_2_0[] = {
    CH_M_L030,
    CH_M_R030
};

const int format_5_1[] = {
    CH_M_L030,
    CH_M_R030,
    CH_M_000,
    CH_LFE1,
    CH_M_L110,
    CH_M_R110
};

const int format_5_2_1[] = {
    CH_M_L030,
    CH_M_R030,
    CH_M_000,
    CH_LFE1,
    CH_M_L110,
    CH_M_R110,
    CH_U_L030,
    CH_U_R030
};

const int format_7_1[] = {
    CH_M_L030,
    CH_M_R030,
    CH_M_000,
    CH_LFE1,
    CH_M_L110,
    CH_M_R110,
    CH_M_L135,
    CH_M_R135
};

const int format_7_1_alt[] = {
    CH_M_L030,
    CH_M_R030,
    CH_M_000,
    CH_LFE1,
    CH_M_L110,
    CH_M_R110,
    CH_M_L060,
    CH_M_R060
};

const int format_8_1[] = {
    CH_M_L030,
    CH_M_R030,
    CH_U_000,
    CH_LFE1,
    CH_M_L110,
    CH_M_R110,
    CH_U_L030,
    CH_U_R030,
    CH_L_000
};

const int format_10_1[] = {
    CH_M_L030,
    CH_M_R030,
    CH_M_000,
    CH_LFE1,
    CH_M_L110,
    CH_M_R110,
    CH_U_L030,
    CH_U_R030,
    CH_U_L110,
    CH_U_R110,
    CH_T_000
};

const int format_22_2[] = {
    CH_M_L060,
    CH_M_R060,
    CH_M_000,
    CH_LFE1,
    CH_M_L135,
    CH_M_R135,
    CH_M_L030,
    CH_M_R030,
    CH_M_180,
    CH_LFE2,
    CH_M_L090,
    CH_M_R090,
    CH_U_L045,
    CH_U_R045,    
    CH_U_000,
    CH_T_000,
    CH_U_L135,
    CH_U_R135,
    CH_U_L090,
    CH_U_R090,
    CH_U_180,    
    CH_L_000,
    CH_L_L045,
    CH_L_R045
};

const int format_9_1[] = {
    CH_M_L030,
    CH_M_R030,
    CH_M_000,
    CH_LFE1,
    CH_M_L110,
    CH_M_R110,
    CH_U_L030,
    CH_U_R030,
    CH_U_L110,
    CH_U_R110
};

const int format_9_0[] = {
    CH_M_L030,
    CH_M_R030,
    CH_M_000,
    CH_M_L110,
    CH_M_R110,
    CH_U_L030,
    CH_U_R030,
    CH_U_L110,
    CH_U_R110
};

const int format_11_1[] = {
    CH_M_L030,
    CH_M_R030,
    CH_M_000,
    CH_LFE1,
    CH_M_L110,
    CH_M_R110,
    CH_U_L030,
    CH_U_R030,
    CH_U_L110,
    CH_U_R110,
    CH_T_000,
    CH_U_000
};

const int format_12_1[] = {
    CH_M_L030,
    CH_M_R030,
    CH_M_000,
    CH_LFE2,
    CH_M_L135,
    CH_M_R135,
    CH_U_L030,
    CH_U_R030,
    CH_U_L135,
    CH_U_R135,
    CH_T_000,
    CH_M_L090,
    CH_M_R090
};

const int format_4_4_0[] = {
    CH_M_L030,
    CH_M_R030,
    CH_M_L110,
    CH_M_R110,
    CH_U_L030,
    CH_U_R030,
    CH_U_L110,
    CH_U_R110
};

const int format_4_4_T_0[] = {
    CH_M_L030,
    CH_M_R030,
    CH_M_L110,
    CH_M_R110,
    CH_U_L030,
    CH_U_R030,
    CH_U_L110,
    CH_U_R110,
    CH_T_000
};

const int format_14_0[] = {
    CH_M_L030,
    CH_M_R030,
    CH_M_000,
    CH_M_L135,
    CH_M_R135,
    CH_U_000,
    CH_U_L045,
    CH_U_R045, 
    CH_U_L090,
    CH_U_R090,
    CH_U_L135,
    CH_U_R135,
    CH_U_180,
    CH_T_000,
};

const int format_15_1[] = {
    CH_M_L030,
    CH_M_R030,
    CH_M_000,
    CH_M_L060,
    CH_M_R060,
    CH_M_L110,
    CH_M_R110,
    CH_M_L135,
    CH_M_R135,
    CH_U_L030,
    CH_U_R030,
    CH_U_L045,
    CH_U_R045,
    CH_U_L110,
    CH_U_R110,
    CH_LFE1
};

const int format_3_0_FC[] = { 
    CH_M_L030,
    CH_M_R030,
    CH_M_000
};

const int format_3_0_RC[] = { 
    CH_M_L030,
    CH_M_R030,
    CH_M_180
};

const int format_4_0[] = { 
    CH_M_L030,
    CH_M_R030,
    CH_M_L110,
    CH_M_R110
};

const int format_5_0[] = { 
    CH_M_L030,
    CH_M_R030,
    CH_M_000,
    CH_M_L110,
    CH_M_R110
};

const int format_6_1[] = { 
    CH_M_L030,
    CH_M_R030,
    CH_M_000,
    CH_LFE1,
    CH_M_L110,
    CH_M_R110,
    CH_M_180
};

/* array of pointers to all formats */

const int* formats_all[NFORMATS-2] = {
    format_2_0,
    format_5_1,
    format_5_2_1,
    format_7_1,
    format_7_1_alt,
    format_8_1,
    format_10_1,
    format_22_2,
    format_9_1,
    format_9_0,
    format_11_1,
    format_12_1,
    format_4_4_0,
    format_4_4_T_0,
    format_14_0,
    format_15_1,
    format_3_0_FC,
    format_3_0_RC,
    format_4_0,
    format_5_0,
    format_6_1
};

/* downmix rules */

#define G1  80      /* gain (0..100 corresponds to 0.0..1.0) when mapping rear => front */
#define G2  60      /* gain (0..100 corresponds to 0.0..1.0) when mapping vary far (rear center => front center) */
#define G3  85      /* gain (0..100 corresponds to 0.0..1.0) when mapping height to horizontal */
#define G4  100     /* gain (0..100 corresponds to 0.0..1.0) for 60deg and 90deg sources on ITU target setups */

const int dmx_rules[][8] = {
    /* input    dest 1       dest 2      gain   processing idx/params */
    {CH_M_000,   CH_M_L022,  CH_M_R022,  100,   RULE_AUTOPAN,   0,  0,  RULE_NOPROC},         
    {CH_M_000,   CH_M_L030,  CH_M_R030,  100,   RULE_AUTOPAN,   0,  0,  RULE_NOPROC},         
    /**/
    {CH_M_L022,  CH_M_000,   CH_M_L030,  G4,    RULE_AUTOPAN,   0,  0,  RULE_NOPROC},
    {CH_M_L022,  CH_M_L030,  -1,         G4,    RULE_NOPROC,    0,  0,  RULE_NOPROC},
    /**/
    {CH_M_R022,  CH_M_000,   CH_M_R030,  G4,    RULE_AUTOPAN,   0,  0,  RULE_NOPROC},
    {CH_M_R022,  CH_M_R030,  -1,         G4,    RULE_NOPROC,    0,  0,  RULE_NOPROC},
    /**/
    {CH_M_L045,  CH_M_L030,  CH_M_L060,  G4,    RULE_AUTOPAN,   0,  0,  RULE_NOPROC},
    {CH_M_L045,  CH_M_L030,  -1,         G4,    RULE_NOPROC,    0,  0,  RULE_NOPROC},
    /**/
    {CH_M_R045,  CH_M_R030,  CH_M_R060,  G4,    RULE_AUTOPAN,   0,  0,  RULE_NOPROC},
    {CH_M_R045,  CH_M_R030,  -1,         G4,    RULE_NOPROC,    0,  0,  RULE_NOPROC},
    /**/
    {CH_M_L060,  CH_M_L045,  CH_M_L090,  G4,    RULE_AUTOPAN,   0,  0,  RULE_NOPROC},
    {CH_M_L060,  CH_M_L030,  CH_M_L090,  G4,    RULE_AUTOPAN,   0,  0,  RULE_NOPROC},
    {CH_M_L060,  CH_M_L045,  CH_M_L110,  G4,    RULE_AUTOPAN,   0,  0,  RULE_NOPROC},
    {CH_M_L060,  CH_M_L030,  CH_M_L110,  G4,    RULE_AUTOPAN,   0,  0,  RULE_NOPROC},
    {CH_M_L060,  CH_M_L030,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},
    /**/
    {CH_M_R060,  CH_M_R045,  CH_M_R090,  G4,    RULE_AUTOPAN,   0,  0,  RULE_NOPROC},
    {CH_M_R060,  CH_M_R030,  CH_M_R090,  G4,    RULE_AUTOPAN,   0,  0,  RULE_NOPROC},
    {CH_M_R060,  CH_M_R045,  CH_M_R110,  G4,    RULE_AUTOPAN,   0,  0,  RULE_NOPROC},
    {CH_M_R060,  CH_M_R030,  CH_M_R110,  G4,    RULE_AUTOPAN,   0,  0,  RULE_NOPROC},
    {CH_M_R060,  CH_M_R030,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},
    /**/
    {CH_M_L090,  CH_M_L060,  CH_M_L110,  G4,    RULE_AUTOPAN,   0,  0,  RULE_NOPROC}, 
    {CH_M_L090,  CH_M_L045,  CH_M_L110,  G4,    RULE_AUTOPAN,   0,  0,  RULE_NOPROC},
    {CH_M_L090,  CH_M_L030,  CH_M_L110,  G4,    RULE_AUTOPAN,   0,  0,  RULE_NOPROC},
    {CH_M_L090,  CH_M_L030,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},
    /**/
    {CH_M_R090,  CH_M_R060,  CH_M_R110,  G4,    RULE_AUTOPAN,   0,  0,  RULE_NOPROC},     
    {CH_M_R090,  CH_M_R045,  CH_M_R110,  G4,    RULE_AUTOPAN,   0,  0,  RULE_NOPROC},
    {CH_M_R090,  CH_M_R030,  CH_M_R110,  G4,    RULE_AUTOPAN,   0,  0,  RULE_NOPROC},
    {CH_M_R090,  CH_M_R030,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},
    /**/
    {CH_M_L110,  CH_M_L135,  -1,         100,   RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_M_L110,  CH_M_L090,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_M_L110,  CH_M_L045,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_M_L110,  CH_M_L030,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},
    /**/
    {CH_M_R110,  CH_M_R135,  -1,         100,   RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_M_R110,  CH_M_R090,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_M_R110,  CH_M_R045,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_M_R110,  CH_M_R030,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},
    /**/
    {CH_M_L135,  CH_M_L110,  -1,         100,   RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_M_L135,  CH_M_L150,  -1,         100,   RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_M_L135,  CH_M_L090,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_M_L135,  CH_M_L045,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_M_L135,  CH_M_L030,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},
    /**/
    {CH_M_R135,  CH_M_R110,  -1,         100,   RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_M_R135,  CH_M_R150,  -1,         100,   RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_M_R135,  CH_M_R090,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_M_R135,  CH_M_R045,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_M_R135,  CH_M_R030,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},
    /**/
    {CH_M_L150,  CH_M_L135,  -1,         100,   RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_M_L150,  CH_M_L110,  -1,         100,   RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_M_L150,  CH_M_L045,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_M_L150,  CH_M_L030,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},
    /**/
    {CH_M_R150,  CH_M_R135,  -1,         100,   RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_M_R150,  CH_M_R110,  -1,         100,   RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_M_R150,  CH_M_R045,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_M_R150,  CH_M_R030,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},
    /**/
    {CH_M_180,   CH_M_L150,  CH_M_R150,  100,   RULE_AUTOPAN,   0, 0,  RULE_NOPROC},    
    {CH_M_180,   CH_M_L135,  CH_M_R135,  100,   RULE_AUTOPAN,   0, 0,  RULE_NOPROC},    
    {CH_M_180,   CH_M_L110,  CH_M_R110,  100,   RULE_AUTOPAN,   0, 0,  RULE_NOPROC},   
    {CH_M_180,   CH_M_L090,  CH_M_R090,  G1,    RULE_AUTOPAN,   0, 0,  RULE_NOPROC},   
    {CH_M_180,   CH_M_L045,  CH_M_R045,  G2,    RULE_AUTOPAN,   0, 0,  RULE_NOPROC},   
    {CH_M_180,   CH_M_L030,  CH_M_R030,  G2,    RULE_AUTOPAN,   0, 0,  RULE_NOPROC},
    /**/
    {CH_U_000,   CH_U_L030,  CH_U_R030,  100,   RULE_AUTOPAN,   0,  0,  RULE_NOPROC},
    {CH_U_000,   CH_M_L030,  CH_M_R030,  G3,    RULE_PANNING,   30, 0,  RULE_EQ1},
    /**/
    {CH_U_L045,  CH_U_L030,  -1,         100,   RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_U_L045,  CH_M_L045,  -1,         G3,    RULE_NOPROC,    0,  0,  RULE_EQ1},
    {CH_U_L045,  CH_M_L030,  -1,         G3,    RULE_NOPROC,    0,  0,  RULE_EQ1},
    /**/
    {CH_U_R045,  CH_U_R030,  -1,         100,   RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_U_R045,  CH_M_R045,  -1,         G3,    RULE_NOPROC,    0,  0,  RULE_EQ1},
    {CH_U_R045,  CH_M_R030,  -1,         G3,    RULE_NOPROC,    0,  0,  RULE_EQ1},
    /**/
    {CH_U_L030,  CH_U_L045,  -1,         100,   RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_U_L030,  CH_M_L030,  -1,         G3,    RULE_NOPROC,    0,  0,  RULE_EQ1},
    /**/
    {CH_U_R030,  CH_U_R045,  -1,         100,   RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_U_R030,  CH_M_R030,  -1,         G3,    RULE_NOPROC,    0,  0,  RULE_EQ1},
    /**/
    {CH_U_L090,  CH_U_L030,  CH_U_L110,  G4,    RULE_AUTOPAN,   0,  0,  RULE_NOPROC},
    {CH_U_L090,  CH_U_L030,  CH_U_L135,  G4,    RULE_AUTOPAN,   0,  0,  RULE_NOPROC},
    {CH_U_L090,  CH_U_L045,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_U_L090,  CH_U_L030,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},        
    {CH_U_L090,  CH_M_L045,  CH_M_L110,  G3,    RULE_AUTOPAN,   0,  0,  RULE_EQ2},
    {CH_U_L090,  CH_M_L030,  CH_M_L110,  G3,    RULE_AUTOPAN,   0,  0,  RULE_EQ2},
    {CH_U_L090,  CH_M_L030,  -1,         G3,    RULE_NOPROC,    0,  0,  RULE_EQ2},
    /**/
    {CH_U_R090,  CH_U_R030,  CH_U_R110,  G4,    RULE_AUTOPAN,   0,  0,  RULE_NOPROC},
    {CH_U_R090,  CH_U_R030,  CH_U_R135,  G4,    RULE_AUTOPAN,   0,  0,  RULE_NOPROC},
    {CH_U_R090,  CH_U_R045,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_U_R090,  CH_U_R030,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_U_R090,  CH_M_R045,  CH_M_R110,  G3,    RULE_AUTOPAN,   0,  0,  RULE_EQ2},        
    {CH_U_R090,  CH_M_R030,  CH_M_R110,  G3,    RULE_AUTOPAN,   0,  0,  RULE_EQ2},        
    {CH_U_R090,  CH_M_R030,  -1,         G3,    RULE_NOPROC,    0,  0,  RULE_EQ2},
    /**/
    {CH_U_L110,  CH_U_L135,  -1,         100,   RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_U_L110,  CH_U_L090,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_U_L110,  CH_U_L045,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},        
    {CH_U_L110,  CH_U_L030,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},        
    {CH_U_L110,  CH_M_L110,  -1,         G3,    RULE_NOPROC,    0,  0,  RULE_EQ2},
    {CH_U_L110,  CH_M_L045,  -1,         G3,    RULE_NOPROC,    0,  0,  RULE_EQ2},
    {CH_U_L110,  CH_M_L030,  -1,         G3,    RULE_NOPROC,    0,  0,  RULE_EQ2},
    /**/
    {CH_U_R110,  CH_U_R135,  -1,         100,   RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_U_R110,  CH_U_R090,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_U_R110,  CH_U_R045,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},        
    {CH_U_R110,  CH_U_R030,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},        
    {CH_U_R110,  CH_M_R110,  -1,         G3,    RULE_NOPROC,    0,  0,  RULE_EQ2},
    {CH_U_R110,  CH_M_R045,  -1,         G3,    RULE_NOPROC,    0,  0,  RULE_EQ2},
    {CH_U_R110,  CH_M_R030,  -1,         G3,    RULE_NOPROC,    0,  0,  RULE_EQ2},
    /**/
    {CH_U_L135,  CH_U_L110,  -1,         100,   RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_U_L135,  CH_U_L090,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_U_L135,  CH_U_L045,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},        
    {CH_U_L135,  CH_U_L030,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},        
    {CH_U_L135,  CH_M_L110,  -1,         G3,    RULE_NOPROC,    0,  0,  RULE_EQ2},
    {CH_U_L135,  CH_M_L045,  -1,         G3,    RULE_NOPROC,    0,  0,  RULE_EQ2},
    {CH_U_L135,  CH_M_L030,  -1,         G3,    RULE_NOPROC,    0,  0,  RULE_EQ2},
    /**/
    {CH_U_R135,  CH_U_R110,  -1,         100,   RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_U_R135,  CH_U_R090,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_U_R135,  CH_U_R045,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},        
    {CH_U_R135,  CH_U_R030,  -1,         G1,    RULE_NOPROC,    0,  0,  RULE_NOPROC},        
    {CH_U_R135,  CH_M_R110,  -1,         G3,    RULE_NOPROC,    0,  0,  RULE_EQ2},
    {CH_U_R135,  CH_M_R045,  -1,         G3,    RULE_NOPROC,    0,  0,  RULE_EQ2},
    {CH_U_R135,  CH_M_R030,  -1,         G3,    RULE_NOPROC,    0,  0,  RULE_EQ2},
    /**/
    {CH_U_180,   CH_U_L135,  CH_U_R135,  100,   RULE_AUTOPAN,   0, 0,  RULE_NOPROC},    
    {CH_U_180,   CH_U_L110,  CH_U_R110,  100,   RULE_AUTOPAN,   0, 0,  RULE_NOPROC},    
    {CH_U_180,   CH_M_180,   -1,         G3,    RULE_NOPROC,    0,  0,  RULE_EQ2},
    {CH_U_180,   CH_M_L110,  CH_M_R110,  G3,    RULE_AUTOPAN,   0, 0,  RULE_EQ2},
    {CH_U_180,   CH_U_L030,  CH_U_R030,  G1,    RULE_AUTOPAN,   0, 0,  RULE_NOPROC},
    {CH_U_180,   CH_M_L030,  CH_M_R030,  G3,    RULE_AUTOPAN,   0, 0,  RULE_EQ2},
    /**/
    {CH_T_000,   -1,         -1,         80,    RULE_TOP2ALLU,  0,  0,  RULE_EQ3},
    {CH_T_000,   -1,         -1,         80,    RULE_TOP2ALLM,  0,  0,  RULE_EQ4},
    /**/
    {CH_L_000,   CH_M_000,   -1,         100,   RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_L_000,   CH_M_L030,  CH_M_R030,  100,   RULE_PANNING,   30, 0,  RULE_NOPROC},
    /**/
    {CH_L_L045,   CH_M_L045,  -1,        100,   RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_L_L045,   CH_M_L030,  -1,        100,   RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_L_R045,   CH_M_R045,  -1,        100,   RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_L_R045,   CH_M_R030,  -1,        100,   RULE_NOPROC,    0,  0,  RULE_NOPROC},
    /**/
    {CH_LFE1,     CH_LFE2,    -1,        100,   RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_LFE1,     CH_M_L030,  CH_M_R030, 100,   RULE_PANNING,   30, 0,  RULE_NOPROC},
    /**/
    {CH_LFE2,     CH_LFE1,    -1,        100,   RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_LFE2,     CH_M_L030,  CH_M_R030, 100,   RULE_PANNING,   30, 0,  RULE_NOPROC},
#ifdef RM6_INTERNAL_CHANNEL
    {CH_I_CNTR,   CH_M_L030,  CH_M_R030, 100,   RULE_AUTOPAN,   0,  0,  RULE_NOPROC},
    {CH_I_LFE,    CH_M_L030,  CH_M_R030, 100,   RULE_AUTOPAN,   0,  0,  RULE_NOPROC},
    {CH_I_LEFT,   CH_M_L030,  -1,        100,   RULE_NOPROC,    0,  0,  RULE_NOPROC},
    {CH_I_RIGHT,  CH_M_R030,  -1,        100,   RULE_NOPROC,    0,  0,  RULE_NOPROC},
#endif
    /* indicate end of matrix */
    {-1,          -1,         -1,        -1,    -1,             -1, -1, -1}
};

/* Internal functions
 * ************************************************************************** */

/* functions */

char is_m_channel(const int ch)
{
    if ((ch >= CH_M_000) && (ch <= CH_M_180)) {
        return 1;
    } else {
        return 0;
    }
}

char is_u_channel(const int ch)
{
    if ((ch >= CH_U_000) && (ch <= CH_U_180)) {
        return 1;
    } else {
        return 0;
    }
}

int find_channel(const int ch, const int* channels, const int nchan)
{
    int i;
    for (i = 0; i < nchan; i++) {
        if (channels[i] == ch) {
            return i;
        }
    }
    return -1;
}

float peak_filter(
                  const float f,    /* peak frequency [Hz] */
                  const float q,    /* peak Q factor */ 
                  const float g,    /* peak gain */ 
                  const float G,    /* gain */
                  const float b)    /* band center frequency [Hz] */
{
    float   V0, gain;

    /* peak gain in linear domain */
    V0 = (float)pow(10.0f, (float)fabs(g)/20.0f);

    /* 2nd order peak filter magnitude response */
    if (g < 0.0f) {
        gain = (b*b*b*b + (1.0f/(q*q) - 2.0f)*f*f*b*b + f*f*f*f) / (b*b*b*b + (V0*V0/(q*q) - 2.0f)*f*f*b*b + f*f*f*f);
    } else {
        gain = (b*b*b*b + (V0*V0/(q*q) - 2.0f)*f*f*b*b + f*f*f*f) / (b*b*b*b + (1.0f/(q*q) - 2.0f)*f*f*b*b + f*f*f*f);
    }

    return (float)sqrt(gain) * (float)pow(10.0f, G/20.0f);
}

void compute_eq(
                const float *bands_nrm,
                const int   nbands,
                const float sfreq_Hz,
                const int   eq_1_idx,
                const float eq_1_strength,
                const int   eq_2_idx,
                const float eq_2_strength,
                float       *eq
                )
{
    int i;
    
    /* initialize static eqs */
    
    for (i = 0; i < nbands; i++) {
        float f;
        
        f =     (float)fabs(bands_nrm[i])*sfreq_Hz/2.0f;
        eq[i] = 1.0f;
        
        /* EQ1: for mixing front height to front horizontal */
        
        if (eq_1_idx == RULE_EQ1) {
            eq[i] *= peak_filter(12000.0f, 0.3f, -2.0f*eq_1_strength, 1.0f*eq_1_strength, f);
        }
        if (eq_2_idx == RULE_EQ1) {
            eq[i] *= peak_filter(12000.0f, 0.3f, -2.0f*eq_2_strength, 1.0f*eq_2_strength, f);
        }
        
        /* EQ2: for mixing surround height to surround horizontal */
        
        if (eq_1_idx == RULE_EQ2) {
            eq[i] *= peak_filter(12000.0f, 0.3f, -3.5f*eq_1_strength, 1.0f*eq_1_strength, f);
        }
        if (eq_2_idx == RULE_EQ2) {
            eq[i] *= peak_filter(12000.0f, 0.3f, -3.5f*eq_2_strength, 1.0f*eq_2_strength, f);
        }
        
        /* EQ3: for mixing top to height loudspeakers */
        
        if (eq_1_idx == RULE_EQ3) {
            eq[i] *= peak_filter(200.0f, 0.3f, -6.5f*eq_1_strength, 0.7f*eq_1_strength, f);
            eq[i] *= peak_filter(1300.0f, 0.5f, 1.8f*eq_1_strength, 0.0f*eq_1_strength, f);
            eq[i] *= peak_filter(600.0f, 1.0f, 2.0f*eq_1_strength, 0.0f*eq_1_strength, f);
        }
        if (eq_2_idx == RULE_EQ3) {
            eq[i] *= peak_filter(200.0f, 0.3f, -6.5f*eq_2_strength, 0.7f*eq_2_strength, f);
            eq[i] *= peak_filter(1300.0f, 0.5f, 1.8f*eq_2_strength, 0.0f*eq_2_strength, f);
            eq[i] *= peak_filter(600.0f, 1.0f, 2.0f*eq_2_strength, 0.0f*eq_2_strength, f);
        }
        
        /* EQ4: for mixing top to horizontal loudspeakers */
        
        if (eq_1_idx == RULE_EQ4) {
            eq[i] *= peak_filter(5000.0f, 1.0f, 4.5f*eq_1_strength, -3.1f*eq_1_strength, f);
            eq[i] *= peak_filter(1100.0f, 0.8f, 1.8f*eq_1_strength, 0.0f*eq_1_strength, f);
        }
        if (eq_2_idx == RULE_EQ4) {
            eq[i] *= peak_filter(5000.0f, 1.0f, 4.5f*eq_2_strength, -3.1f*eq_2_strength, f);
            eq[i] *= peak_filter(1100.0f, 0.8f, 1.8f*eq_2_strength, 0.0f*eq_2_strength, f);
        }
        
        /* EQ5: for mixing M to U for rand5 */
        
        if (eq_1_idx == RULE_EQ5) {
            eq[i] *= peak_filter(35.0f, 0.25f, -1.3f*eq_1_strength, 1.0f*eq_1_strength, f);
        }
        if (eq_2_idx == RULE_EQ5) {
            eq[i] *= peak_filter(35.0f, 0.25f, -1.3f*eq_2_strength, 1.0f*eq_2_strength, f);
        }
    }
}

void free_inout_format_lists( void )
{
    /* deallocate channel lists if they have been populated */ 
    if (format_in_listOfChannels != NULL){
        free(format_in_listOfChannels);
        format_in_listOfChannels = NULL;
    }
    if (format_out_listOfChannels != NULL){
        free(format_out_listOfChannels);
        format_out_listOfChannels = NULL;
    }
}


/* API functions
 * ************************************************************************** */

/**
 * Define generic input/output formats.
 */

int converter_set_inout_format(
        const int                  inout,           /* in:  0: set input format, 1: set output format */
        const int                  num_channels,    /* in:  number of channels in setup to define */
        const converter_chid_t*    channel_vector   /* in:  vector containing channel enums */
)
{
    int l;
    if(inout == 0){
        fprintf(stderr,"INFO: setting input format = list of channels\n");
        if(format_in_listOfChannels != NULL){
            fprintf(stderr,"Error: Attempt to set list of input channels, but already set!\n");
            exit(-1);              
        }
        format_in_listOfChannels = (int*)calloc(num_channels,sizeof(int));
        for(l=0;l<num_channels;l++)
            format_in_listOfChannels[l] = channel_vector[l];
        format_in_listOfChannels_nchan = num_channels;
    }
    else if(inout == 1){
        fprintf(stderr,"INFO: setting output format = list of channels\n");
        if(format_out_listOfChannels != NULL){
            fprintf(stderr,"Error: Error: Attempt to set list of output channels, but already set!\n");
            exit(-1);              
        }       
        format_out_listOfChannels = (int*)calloc(num_channels,sizeof(int));
        for(l=0;l<num_channels;l++)
            format_out_listOfChannels[l] = channel_vector[l];
        format_out_listOfChannels_nchan = num_channels;
    }
    else{
        fprintf(stderr,"Error: Setting of input or output format = list of channels failed! Check API parameters!\n");
        exit(-1);              
    }
    return 0;
}


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
)
{
    int     i, nchanin, nchanout, ch, in_out_n, ptr;
    int     in_out_src[IN_OUT_N];
    int     in_out_dst[IN_OUT_N];
    float   in_out_gain[IN_OUT_N];
    int     in_out_proc[IN_OUT_N];
#ifndef FORMATCONVERTER_LOWCOMPLEXITY /* RMO code */
    int     in_out_gainComp[IN_OUT_N];
#endif
    
    const int   *format_in, *format_out;

    float azi_ele[CH_LFE2+1][2];
    
    if (nbands > MAXBANDS) {
        fprintf(stderr,"Constant MAXBANDS is too small! (< nbands)!\n");
        exit(-1);
    }
    
    /* init in_out vectors */
    for(i = 0; i < IN_OUT_N; i++) {
        in_out_src[i] = 0;
        in_out_dst[i] = 0;
        in_out_gain[i] = 0.f;
        in_out_proc[i] = 0;
#ifndef FORMATCONVERTER_LOWCOMPLEXITY /* RMO code */
        in_out_gainComp[i] = 0;
#endif
    }

    /* parameter range checking */
    
    if (input_format >= NFORMATS-1) {
        free_inout_format_lists();
        return FORMAT_CONVERTER_STATUS_INFORMAT;
    }
    
    if ((output_format >= NFORMATS) || (output_format == NFORMATS-2)) {
        free_inout_format_lists();
        return FORMAT_CONVERTER_STATUS_OUTFORMAT;
    }
    
    if ((sfreq_Hz < SFREQ_HZ_MIN) || (sfreq_Hz > SFREQ_HZ_MAX)) {
        free_inout_format_lists();
        return FORMAT_CONVERTER_STATUS_SFREQ;
    }
        
    /* deactivated check since it does not fit FD as well as TD processing
    if ((blocklength < BLOCKLENGTH_MIN_MS*sfreq_Hz/1000.0f) || (blocklength > BLOCKLENGTH_MAX_MS*sfreq_Hz/1000.0f)) {
        fprintf(stderr,"blocklength out of range: %d, min:%f max:%f\n",\
            blocklength,BLOCKLENGTH_MIN_MS*sfreq_Hz/1000.0f,\
            BLOCKLENGTH_MAX_MS*sfreq_Hz/1000.0f);
        free_inout_format_lists();
        return FORMAT_CONVERTER_STATUS_BLOCKLENGTH;
    }
    */    

    if ((nbands < NBANDS_MIN) || (nbands > NBANDS_MAX)) {
        if(nbands == 0)
            fprintf(stderr,"\nWARNING: numFreqBands=0. Only allowed for time domain mode!\n");
        else {
            free_inout_format_lists();
            return FORMAT_CONVERTER_STATUS_BANDS;
        }
    }
    
    for (i = 0; i < nbands; i++) {
        if ((bands_nrm[i] > 1.0f) || (bands_nrm[i] < 0.0f))
            return FORMAT_CONVERTER_STATUS_BANDS;
    }
    
    
    /* get input and output format info */
    if (input_format == FORMAT_IN_LISTOFCHANNELS){
        nchanin   = format_in_listOfChannels_nchan;
        format_in = format_in_listOfChannels;
        if(format_in == NULL){
            fprintf(stderr,"Error: Input format signaled as list of channels but list not filled!\n");
            exit(-1);
        }  
    } else {
        nchanin   = format_nchan[input_format];
        format_in = formats_all[input_format];
    }
    
    if (output_format == FORMAT_OUT_LISTOFCHANNELS){
        nchanout   = format_out_listOfChannels_nchan;
        format_out = format_out_listOfChannels;
        if(format_out == NULL){
            fprintf(stderr,"Error: Output format signaled as list of channels but list not filled!\n");
            exit(-1);
        }  
    } else {
        nchanout   = format_nchan[output_format];  
        format_out = formats_all[output_format];     
    }

    params->nchanin =   nchanin;
    params->nchanout =  nchanout;
    
    /* copy channel angles (they may be modified for random setups) */
    
    for (i = 0; i < CH_LFE2+1; i++) {
        azi_ele[i][0] = ch_azi_ele[i][0];
        azi_ele[i][1] = ch_azi_ele[i][1];
    }
    
    /* random formats: adjust angles */
    
    if (randomization != NULL) {
        for (i = 0; i < nchanout; i++) {
            azi_ele[format_out[i]][0] += randomization[2*i];
            azi_ele[format_out[i]][1] += randomization[2*i+1];
        }
    }
    
    /* more parameter error checking */
    
    if (trim != NULL) {
        float trim_min_m = 1e10f;
        float trim_max_m = 0.0f;
        for (i = 0; i < nchanout; i++) {
            if ((trim[i] < TRIM_RADIUS_MIN) || (trim[i] > TRIM_RADIUS_MAX)) {
                printf("Trim: distance too small or too large (min: %fm, max: %fm)!\n",TRIM_RADIUS_MIN,TRIM_RADIUS_MAX);
                free_inout_format_lists();
                return FORMAT_CONVERTER_STATUS_TRIM;
            }
            if (trim_max_m < trim[i])
                trim_max_m = trim[i];
            if (trim_min_m > trim[i])
                trim_min_m = trim[i];
        }
        if (trim_max_m/trim_min_m > TRIM_RATIO_MAX) {
            printf("Trim: ratio too large!\n");
            free_inout_format_lists();
            return FORMAT_CONVERTER_STATUS_TRIM;
        }
        if (round((trim_max_m-trim_min_m)/SOS_M_S*sfreq_Hz) > (float)trim_max) {
            printf("Trim: delay buffer too small!\n");
            free_inout_format_lists();
            return FORMAT_CONVERTER_STATUS_TRIM;
        }
    }
    
    if (randomization != NULL) {
        for (i = 0; i < nchanout; i++) {
            /* check whether absolute deviation per channel is within allowed range */
            if (((float)fabs(randomization[2*i]) > RANDOMIZATION_AZI_MAX) || ((float)fabs(randomization[2*i+1]) > RANDOMIZATION_ELE_MAX)) {
                fprintf(stderr,"Randomization: absolute angular deviation too large!\n");
                free_inout_format_lists();
                return FORMAT_CONVERTER_STATUS_RANDOMIZATION;
            }
        }
        /* check whether ordering of horizontal channels stays the same */
        for (i = 0; i < nchanout; i++) {
            /* horizontal channel? */
            if (ch_azi_ele[i][1] == 0.0f) {
                /* compare angles between channel pairs */
                for (ch = 0; ch < nchanout; ch++) {
                    if (ch == i) {
                        continue;
                    }
                    if (ch_azi_ele[format_out[i]][0] < ch_azi_ele[format_out[ch]][0]) {
                        /* ordering of randomized angles different? */
                        if (azi_ele[format_out[i]][0] >= azi_ele[format_out[ch]][0]) {
                            fprintf(stderr,"Randomization: ordering of horizontal channels is incorrect!\n");
                            fprintf(stderr,"(ref: %f, %f, rand: %f, %f)\n", ch_azi_ele[format_out[i]][0], ch_azi_ele[format_out[ch]][0], azi_ele[format_out[i]][0], azi_ele[format_out[ch]][0]);
                            free_inout_format_lists();
                            return FORMAT_CONVERTER_STATUS_RANDOMIZATION;
                        }
                    }
                }
            }
        }
        /* check whether ordering of height channels stays the same */
        for (i = 0; i < nchanout; i++) {
            /* height channel? */
            if (ch_azi_ele[i][1] > 0.0f) {
                /* compare angles between channel pairs */
                for (ch = 0; ch < nchanout; ch++) {
                    if (ch == i) {
                        continue;
                    }
                    if (ch_azi_ele[format_out[i]][0] < ch_azi_ele[format_out[ch]][0]) {
                        /* ordering of randomized angles different? */
                        if (azi_ele[format_out[i]][0] >= azi_ele[format_out[ch]][0]) {
                            fprintf(stderr,"Randomization: ordering of height channels is incorrect!\n");
                            free_inout_format_lists();
                            return FORMAT_CONVERTER_STATUS_RANDOMIZATION;
                        }
                    }
                }
            }
        }
        /* check whether ordering of low channels stays the same */
        for (i = 0; i < nchanout; i++) {
            /* height channel? */
            if (ch_azi_ele[i][1] < 0.0f) {
                /* compare angles between channel pairs */
                for (ch = 0; ch < nchanout; ch++) {
                    if (ch == i) {
                        continue;
                    }
                    if (ch_azi_ele[format_out[i]][0] < ch_azi_ele[format_out[ch]][0]) {
                        /* ordering of randomized angles different? */
                        if (azi_ele[format_out[i]][0] >= azi_ele[format_out[ch]][0]) {
                            fprintf(stderr,"Randomization: ordering of low channels is incorrect!\n");
                            free_inout_format_lists();
                            return FORMAT_CONVERTER_STATUS_RANDOMIZATION;
                        }
                    }
                }
            }
        }
        /* check that angle between any loudspeaker pair is not too small */
        for (i = 0; i < nchanout; i++) {
            float azi, ele, angle;
            if ( (format_out[i] == CH_LFE1) || (format_out[i] == CH_LFE2) ) { /* ignore LFEs */
                continue;
            }
            /* compare angles between channel pairs */
            for (ch = 0; ch < nchanout; ch++) {
                if ( (format_out[ch] == CH_LFE1) || (format_out[ch] == CH_LFE2) ) { /* ignore LFEs */
                    continue;
                }
                if (ch == i) {
                    continue;
                }
                /* compute angle between loudspeaker pair */
                azi = azi_ele[format_out[i]][0] - azi_ele[format_out[ch]][0];
                ele = azi_ele[format_out[i]][1] - azi_ele[format_out[ch]][1];
                azi *= M_PI / 180.0f;
                ele *= M_PI / 180.0f;
                angle = (float) acos(cos(azi)*cos(ele)) * 180.0f / M_PI;
                if (fabs(angle) < RANDOMIZATION_SPK_ANGLE_MIN) {
                    fprintf(stderr,"Randomization: loudspeaker pair has too near loudspeakers!\n");
                    free_inout_format_lists();
                    return FORMAT_CONVERTER_STATUS_RANDOMIZATION;
                }
            }
        }
        /* check ordering in elevation dimension */
        ch = 0;
        while (ele_ordering[ch][0] >= 0) {
            float   ele_1, ele_2, ele_3;
            int     count;
            ele_1 = -1e20f;
            ele_2 = 1e20f;
            ele_3 = 1e20f;
            count = 0;
            for (i = 0; i < nchanout; i++) {
                if ( (ele_ordering[ch][0] == format_out[i]) && (format_out[i] != CH_EMPTY) ) {
                    ele_1 = azi_ele[format_out[i]][1];
                    count++;
                }
                if ( (ele_ordering[ch][1] == format_out[i]) && (format_out[i] != CH_EMPTY) ) {
                    ele_2 = azi_ele[format_out[i]][1];
                    count++;
                }
                if ( (ele_ordering[ch][2] == format_out[i]) && (format_out[i] != CH_EMPTY) ) {
                    ele_3 = azi_ele[format_out[i]][1];
                    count++;
                }
            }
            if (ele_2 == 1e20f) /* ensure correct ordering if ele_2 has not been set */
                ele_2 = 0.5f*(ele_1 + ele_3);
            if ((count > 1) && !((ele_1 < ele_2) && (ele_2 < ele_3))) {
                fprintf(stderr,"Randomization: ordering in elevation is incorrect!\n");
                fprintf(stderr,"(ch = %d, %f, %f, %f)\n", ch, ele_1, ele_2, ele_3);
                free_inout_format_lists();
                return FORMAT_CONVERTER_STATUS_RANDOMIZATION;
            }
            ch++;
        }
        
    }
    
    /* define format conversion */
    
    ch = 0;
    for (i = 0; i < nchanin; i++) {
        int input_channel_id, output_channel_idx;
        
        /* map EMPTY channel as silent channel, i.e. generate a dmx mtx line containing zeros */
        input_channel_id = format_in[i];
        if (input_channel_id == CH_EMPTY) {
            in_out_src[ch] =    i;                  /* input channel index */
            in_out_dst[ch] =    0;                  /* use first output channel. unimportant since gain is set to 0.0 */
            in_out_gain[ch] =   0.0f;               /* gain: 0 gain, silence */
            in_out_proc[ch] =   0;                  /* processing: 0 = no EQ */
            ch++;
            continue; 
        } 
            
        /* test if input channel exists in output format */      
        output_channel_idx = find_channel(input_channel_id, format_out, nchanout);
        
        /* input channel exists in output format
           (just copy channel from input to output) */
    
        if (output_channel_idx >= 0) {
            in_out_src[ch] =    i;                  /* input channel index */
            in_out_dst[ch] =    output_channel_idx; /* output channel index */
            in_out_gain[ch] =   1.0f;               /* gain */
            in_out_proc[ch] =   0;                  /* processing */
            ch++;
        
        /* input channel does not exist in output format
           (apply downmix rules) */

        } else {
            int k, ch_start;

            /* search best downmix rule */
            ch_start = ch;
            k = 0;
            while (dmx_rules[k][0] != -1) {
                int ch_count, uch;
            
                /* find rule with input == input_channel_id */
                if (dmx_rules[k][0] == input_channel_id) {
                    int idx, idx2;
                
                    /* top loudspeaker to all height loudspeakers rule */
                    if (dmx_rules[k][4] == RULE_TOP2ALLU) {
                        /* count number of height channels */
                        ch_count = 0;
                        for (uch = CH_U_000; uch <= CH_U_180; uch++) {
                            if (find_channel(uch, format_out, nchanout) != -1)
                                ch_count++;
                        }
                        /* if there are height channels, top is mixed to these */
                        if (ch_count > 0) {  
                            float gain;
                            
                            gain = 1.0f / (float)sqrt(ch_count);
                            for (uch = CH_U_000; uch <= CH_U_180; uch++) {
                                in_out_src[ch] =    i;                                          /* input channel index */
                                in_out_dst[ch] =    find_channel(uch, format_out, nchanout);    /* output channel index */
                                in_out_gain[ch] =   0.01f * dmx_rules[k][3] * gain;             /* gain */
                                in_out_proc[ch] =   dmx_rules[k][7];                            /* processing */
                                if (in_out_dst[ch] != -1)
                                    ch++;
                            }
                            break;
                        }
                        k++;
                        continue;
                    }
                    /* top loudspeaker to all horizontal loudspeakers rule */
                    if (dmx_rules[k][4] == RULE_TOP2ALLM) {
                        /* count number of horizontal channels */
                        ch_count = 0;
                        for (uch = CH_M_000; uch <= CH_M_180; uch++) {
                            if (find_channel(uch, format_out, nchanout) != -1)
                                ch_count++;
                        }
                        /* if there are horizontal channels, top is mixed to these */
                        if (ch_count > 0) {  
                            float gain;
                            
                            gain = 1.0f / (float)sqrt(ch_count);
                            for (uch = CH_M_000; uch <= CH_M_180; uch++) {
                                in_out_src[ch] =    i;                                          /* input channel index */
                                in_out_dst[ch] =    find_channel(uch, format_out, nchanout);    /* output channel index */
                                in_out_gain[ch] =   0.01f * dmx_rules[k][3] * gain;             /* gain */
                                in_out_proc[ch] =   dmx_rules[k][7];                            /* processing */
                                if (in_out_dst[ch] != -1)
                                    ch++;
                            }
                            break;
                        }
                        k++;
                        continue;
                    }
                    /* do target channels of rule exist? */
                    idx = find_channel(dmx_rules[k][1], format_out, nchanout);
                    idx2 = idx;
                    if (dmx_rules[k][2] != -1) {
                        /* rule with two output channels */
                        idx2 = find_channel(dmx_rules[k][2], format_out, nchanout);
                    }
                    /* do all rule output channels exist? */
                    if ((idx >= 0) && (idx2 >= 0)) {
                        if ((dmx_rules[k][4] == RULE_PANNING) || (dmx_rules[k][4] == RULE_AUTOPAN)) {
                            float azi_1, azi_2, azi_target, mn, alpha, alpha0, center;
                            float a1, a2, nrm;
                        
                            /* add offest such that all angles are positive */
                            azi_1      = azi_ele[format_out[idx]][0];
                            azi_2      = azi_ele[format_out[idx2]][0];                        
                            azi_target = azi_ele[input_channel_id][0];                        
                            mn         = (float)min((float)min(azi_1,azi_2), azi_target);
                            azi_1      = azi_1 - mn;
                            azi_2      = azi_2 - mn;
                            azi_target = azi_target - mn;
                            /* manual and auto panning */
                            if (dmx_rules[k][4] == RULE_PANNING) {
                                /* manual definition of parameters */
                                alpha0 = (float) dmx_rules[k][5]; /* angle of system +-alpha0 */
                                alpha =  (float) dmx_rules[k][6]; /* definition: negative is left */
                            } else if (dmx_rules[k][4] == RULE_AUTOPAN) {
                                /* amplitude panning angle alpha0 */
                                alpha0 = 0.5f * (float)fabs(azi_1-azi_2);
                                /* center angle */
                                center = 0.5f * (azi_1 + azi_2);
                                /* amplitude panning angle alpha */
                                alpha = center - azi_target;
                                if (azi_1 > azi_2) {
                                    alpha = -alpha;
                                }
                            }
                            alpha0   = alpha0 * (float)M_PI / 180.0f;
                            alpha    = alpha * (float)M_PI / 180.0f;
                            a2       = 1.0f;
                            a1       = a2*((float)tan(alpha0)+(float)tan(alpha)+1e-10f)/((float)tan(alpha0)-(float)tan(alpha)+1e-10f);
                            nrm      = 1.0f / (float)sqrt(a1*a1+a2*a2);
                            a1       = a1 * nrm;
                            a2       = a2 * nrm;
                            /* panning */
                            in_out_src[ch] =    i;                              /* input channel index */
                            in_out_dst[ch] =    idx;                            /* output channel index */
                            in_out_gain[ch] =   0.01f * dmx_rules[k][3] * a1;   /* gain */
                            in_out_proc[ch] =   dmx_rules[k][7];                /* processing */
                            ch = ch + 1;
                            in_out_src[ch] =    i;                              /* input channel index */
                            in_out_dst[ch] =    idx2;                           /* output channel index */
                            in_out_gain[ch] =   0.01f * dmx_rules[k][3] * a2;   /* gain */
                            in_out_proc[ch] =   dmx_rules[k][7];                /* processing */
                            ch = ch + 1;
                        } else {
                            /* only external processing option */
                            in_out_src[ch] =    i;                              /* input channel index */
                            in_out_dst[ch] =    idx;                            /* output channel index */
                            in_out_gain[ch] =   0.01f * dmx_rules[k][3];        /* gain */
                            in_out_proc[ch] =   dmx_rules[k][7];                /* processing */
                            ch = ch + 1;
                            /* phantom source (and thus 2nd destination channel)? */
                            if (dmx_rules[k][2] != -1) {
                                idx = find_channel(dmx_rules[k][2], format_out, nchanout);
                                if (idx != -1) {
                                    in_out_src[ch] =    i;                          /* input channel index */
                                    in_out_dst[ch] =    idx;                        /* output channel index */
                                    in_out_gain[ch] =   0.01f * dmx_rules[k][3];    /* gain */
                                    in_out_proc[ch] =   dmx_rules[k][7];            /* processing */
                                    ch = ch + 1;
                                }
                            }
                        }
                        /* done (break loop over k) */
                        break;
                    }
                }
                k++;
            } /* = loop over k end */
            if (ch_start == ch) {
                /* downmix rule missing */
                free_inout_format_lists();
                return FORMAT_CONVERTER_STATUS_MISSING_RULE;
            }
        }

    }
    in_out_src[ch] = -1;
    in_out_dst[ch] = -1;
    in_out_gain[ch] = -1;
    in_out_proc[ch] = -1;
    in_out_n = ch;
    
#ifndef FORMATCONVERTER_LOWCOMPLEXITY /* RMO code */
    for (i = 0; i < ch; i++) {
        in_out_gainComp[ch] = 0;   
    }
    in_out_gainComp[ch] = -1;
    params->gainCompValue = 1/(0.01f * G3);
#endif
    
    /* re-order in_out such that ordering is by destimation channel */  
    ptr = 0;
    for (i = 0; i < nchanout; i++) {
        ch = 0;
        while (in_out_src[ch] >= 0) {
/*fprintf(stderr,"i = %d, ch = %d, format_out[i] = %d, in_out_dst[ch] = %d\n", i, ch, format_out[i], (int)in_out_dst[ch]);*/
            if (i == in_out_dst[ch]) {
                params->in_out_src[ptr] = in_out_src[ch];
                params->in_out_dst[ptr] = in_out_dst[ch];
                params->in_out_gain[ptr] = in_out_gain[ch];
                params->in_out_proc[ptr] = in_out_proc[ch];
#ifndef FORMATCONVERTER_LOWCOMPLEXITY /* RMO code */
                params->in_out_gainComp[ptr] = in_out_gainComp[ch];
#endif
                ptr++;
/*fprintf(stderr,"  ptr = %d, ch = %d\n", ptr, ch);*/
            }
            ch++;
        }
    }
    if (ch != ptr) {
        fprintf(stderr,"Bug in resorting in_out!\n");
        exit(-1);
    }
    params->in_out_src[ch] = -1;
    params->in_out_dst[ch] = -1;
    params->in_out_gain[ch] = -1;
    params->in_out_proc[ch] = -1;
#ifndef FORMATCONVERTER_LOWCOMPLEXITY /* RMO code */
    params->in_out_gainComp[ch] = -1;
#endif

    /* random formats: adjust gains and eqs */

    if (randomization != NULL) {
        int rand_eq_index = RULE_REQ-1;
        i = 0;
        while (params->in_out_src[i] >= 0) {
            /* test if horizontal outformat channel */
            if (is_m_channel(format_out[params->in_out_dst[i]])) {
                /* test if this channel is now height */
                if ((azi_ele[format_out[params->in_out_dst[i]]][1] > 0.0f) && (azi_ele[format_out[params->in_out_dst[i]]][1] <= 60.0f)) {
                    /* gain/EQ fading constant ("degree of height") */
                    float nrm_height;
                    nrm_height = min(azi_ele[format_out[params->in_out_dst[i]]][1], 35.0f) / 35.0f;     
                    /* test if corresponding informat channel is height ch */                    
                    if (is_u_channel(format_in[params->in_out_src[i]])) {
                        /* as height increases interpolate to gain 0dB and no eq */
#ifndef FORMATCONVERTER_LOWCOMPLEXITY /* RMO code */
                        if (format_in[params->in_out_src[i]] != CH_U_000) {   
                            /* gain compensation will be directly applied on input signal */
                            params->in_out_gainComp[i] = 1; 
                        }
#else
                        /* gain compensation by modification of dmx matrix */
                        params->in_out_gain[i] = nrm_height*params->in_out_gain[i]/(0.01f * G3) + (1.0f-nrm_height)*params->in_out_gain[i]; 
#endif
                        if (nrm_height == 1.0) {
                            params->in_out_proc[i] = RULE_NOPROC;
                        } else {
                            compute_eq(bands_nrm, nbands, sfreq_Hz, RULE_NOPROC, nrm_height, params->in_out_proc[i], 1.0f-nrm_height, params->eq[rand_eq_index]);
                            params->in_out_proc[i] = rand_eq_index;
                            rand_eq_index++;
                        }
                    } else if (is_m_channel(format_in[params->in_out_src[i]])) {
                        /* set EQ to EQ5 */
                        /* as height increases interpolated current eq to eq5 */
                        if (nrm_height == 1.0) {
                            params->in_out_proc[i] = RULE_EQ5;
                        } else {
                            compute_eq(bands_nrm, nbands, sfreq_Hz, RULE_EQ5, nrm_height, params->in_out_proc[i], 1.0f-nrm_height, params->eq[rand_eq_index]);
                            params->in_out_proc[i] = rand_eq_index;
                            rand_eq_index++;
                        }
                    }
                    if (rand_eq_index == N_EQ) {
                        fprintf(stderr,"Not enough memory for random eqs (increase N_EQ, rand_eq_index = %d)",rand_eq_index);
                    }
                }
            }
#ifndef FORMATCONVERTER_LOWCOMPLEXITY /* RMO code */
            /* U180 --> M-110 (up-down gain compensation was accidently also applied on right side of Rand5 setup for U180 input channel, additionally up-down eq was disabled) */
            if (output_format == FORMAT_5_1 && format_in[params->in_out_src[i]] == CH_U_180 && format_out[params->in_out_dst[i]] == CH_M_R110) {                
                /* gain compensation will be directly applied on input signal */
                params->in_out_gainComp[i] = 1;
                params->in_out_proc[i] = RULE_NOPROC;
            }
            /* M180 --> M-110 (eq5 was accidently also applied on right side of Rand5 setup for M180 input channel) */
            if (output_format == FORMAT_5_1 && format_in[params->in_out_src[i]] == CH_M_180 && format_out[params->in_out_dst[i]] == CH_M_R110) {            
                /* set EQ to EQ5 */
                params->in_out_proc[i] = RULE_EQ5;
            }          
#endif
            i++;
        }
    }
    
    /* initialize static eqs */
    
    compute_eq(bands_nrm, nbands, sfreq_Hz, RULE_EQ1, 1.0, RULE_NOPROC, 0.0, params->eq[0]);
    compute_eq(bands_nrm, nbands, sfreq_Hz, RULE_EQ2, 1.0, RULE_NOPROC, 0.0, params->eq[1]);
    compute_eq(bands_nrm, nbands, sfreq_Hz, RULE_EQ3, 1.0, RULE_NOPROC, 0.0, params->eq[2]);
    compute_eq(bands_nrm, nbands, sfreq_Hz, RULE_EQ4, 1.0, RULE_NOPROC, 0.0, params->eq[3]);
    compute_eq(bands_nrm, nbands, sfreq_Hz, RULE_EQ5, 1.0, RULE_NOPROC, 0.0, params->eq[4]);
        
    /* adaptive eq paramter init */
    
    params->aeq_max_limit = (float)pow(10.0f, AEQ_MAX_LIMIT_DB/20.0f);
    params->aeq_min_limit = (float)pow(10.0f, AEQ_MIN_LIMIT_DB/20.0f);
    
    /* compute delay and gain trim */
    
    for (i = 0; i < NCHANOUT_MAX; i++) {
        params->trim_delay[i] = 0;
        params->trim_gain[i] =  1.0f;
    }
    
    if (trim != NULL) {
        int     min_delay;
        float   max_gain;
        
        for (ch = 0; ch < nchanout; ch++) {
            if (format_out[ch] >= 0) {
                /* delay in samples */
                params->trim_delay[ch] = (int)round( -trim[ch] / SOS_M_S * sfreq_Hz );
                /* gain linear */
                params->trim_gain[ch] = (float)pow(trim[ch], TRIM_EXP);
            } else {
                params->trim_delay[ch] = 0;
                params->trim_gain[ch] = 1.0f;
            }
        }
        /* make smallest delay equal to zero */
        min_delay = 1000000;
        for (ch = 0; ch < nchanout; ch++) {
            if (params->trim_delay[ch] < min_delay)
                min_delay = params->trim_delay[ch];
        }
        for (ch = 0; ch < nchanout; ch++) {
            params->trim_delay[ch] -= min_delay;
        }
        /* make largest gain equal to 1.0 (0dB) */
        max_gain = 0.0f;
        for (ch = 0; ch < nchanout; ch++) {
            if (params->trim_gain[ch] > max_gain)
                max_gain = params->trim_gain[ch];
        }
        for (ch = 0; ch < nchanout; ch++) {
            params->trim_gain[ch] /= max_gain;
        }
    }
    
    free_inout_format_lists();
    return FORMAT_CONVERTER_STATUS_OK;
}
