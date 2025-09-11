#ifndef __INTERNAL_CHANNEL_DEFITIONS_LIBRARY_HEADER__
#define __INTERNAL_CHANNEL_DEFITIONS_LIBRARY_HEADER__

#include <stdio.h>

enum {
	 /*panning gain type for internal channel*/
	L = 0,		 /*internal left*/
	R = 1,		 /*internal right*/
	C = 2,		/* internal center or EQ index is 2*/

	 /*EQ index or Gain Type for internal channel*/
	A = 0,		 /*EQ index is 0 or Gain type is G1=.8f*/
	B = 1,		 /*EQ index is 1 or Gain type is G2=.6f*/ 	
  /*_C = 2,		 EQ index is 2 or Gain type is G3=.85f		(But it is already declared in panning gain type)*/ 
	D = 3		 /*EQ index is 4 or Gain type is G4=1.0f		(Note that EQ index 3 is not in use for internal channel)*/
};

enum {
	CHN_EMPTY = -1,
	CHN_M_L030,
	CHN_M_R030,
	CHN_M_000,
	CHN_LFE1,
	CHN_M_L110,
	CHN_M_R110,
	CHN_M_L022,
	CHN_M_R022,
	CHN_M_L135,
	CHN_M_R135,
	CHN_M_180,
	CHN_RES_1,
	CHN_RES_2,
	CHN_M_L090,
	CHN_M_R090,
	CHN_M_L060,
	CHN_M_R060,
	CHN_U_L030,
	CHN_U_R030,
	CHN_U_000,
	CHN_U_L135,
	CHN_U_R135,
	CHN_U_180,
	CHN_U_L090,
	CHN_U_R090,
	CHN_T_000,
	CHN_LFE2,
	CHN_L_L045,
	CHN_L_R045,
	CHN_L_000,
	CHN_U_L110,
	CHN_U_R110,
	CHN_U_L045,
	CHN_U_R045,
	CHN_M_L045,
	CHN_M_R045,
	CHN_LFE3,
	CHN_M_LSCR,
	CHN_M_RSCR,
	CHN_M_LSCH,
	CHN_M_RSCH,
	CHN_M_L150,
	CHN_M_R150,
	CHN_I_CNTR,
	CHN_I_LFE,
	CHN_I_LEFT,
	CHN_I_RIGHT
};

enum
{
	IC_OFF,
	IC_PRE_STR,
	IC_PRE_MUL,
	IC_POST_STR,
	IC_POST_MUL,
};

typedef struct
{
	int idx;
	int a;
	int e;
	int lfe;
	int ctype;
	int gtype;
	int eqidx;
} IC_RULES;

typedef struct
{
	float *EQ[2];
	float G[2];
	float chQCEs[4];
} IC_DMX;

typedef struct
{
	unsigned char bitstream[32];
	unsigned int ICPresent;
	unsigned int numElements;
	unsigned int ElementType[32];
	unsigned int ICinCPE[32];
	unsigned int CPEOut[32][2];
	unsigned int ICGPrePresent;
	unsigned int ICGPreAppliedCPE[32];
	int isStereoOut;
	int cicpidx;
	int lbl[64];
	int nspkr;
	int lblOut[64];
	int nspkrOut;
	int isICON;
	int isOutIC[64];
	int	QCE;
	int isQMFOut;
	char spkrFileName[FILENAME_MAX];
} IC_CONFIG;

void initICConfig ( void );		 /*temporary function for the ICConfig*/
int getSpkrLbl ( int * lbl, int * nspkr, int cicpidx );
void loadEQnGain4CPE ( int chA, int chB );
float calICG (float cldL, float cldR, int band );
float calInvICG (float cldL, float cldR, int band );
void resetICConfig ( void );
void setICConfig ( int instance, int isQCE );
void setQCEs(int ch1A, int ch1B, int ch2A, int ch2B );

static float f_bands_nrm[71] = {
    0.004583300000000f,
    0.000833330000000f,
    0.002083300000000f,
    0.005875000000000f,
    0.009791700000000f,
    0.014292000000000f,
    0.019792000000000f,
    0.027000000000000f,
    0.035417000000000f,
    0.042625000000000f,
    0.056750000000000f,
    0.072375000000000f,
    0.088000000000000f,
    0.103620000000000f,
    0.119250000000000f,
    0.134870000000000f,
    0.150500000000000f,
    0.166120000000000f,
    0.181750000000000f,
    0.197370000000000f,
    0.213000000000000f,
    0.228620000000000f,
    0.244250000000000f,
    0.259880000000000f,
    0.275500000000000f,
    0.291130000000000f,
    0.306750000000000f,
    0.322380000000000f,
    0.338000000000000f,
    0.353630000000000f,
    0.369250000000000f,
    0.384880000000000f,
    0.400500000000000f,
    0.416130000000000f,
    0.431750000000000f,
    0.447380000000000f,
    0.463000000000000f,
    0.478630000000000f,
    0.494250000000000f,
    0.509870000000000f,
    0.525500000000000f,
    0.541120000000000f,
    0.556750000000000f,
    0.572370000000000f,
    0.588000000000000f,
    0.603620000000000f,
    0.619250000000000f,
    0.634870000000000f,
    0.650500000000000f,
    0.666120000000000f,
    0.681750000000000f,
    0.697370000000000f,
    0.713000000000000f,
    0.728620000000000f,
    0.744250000000000f,
    0.759870000000000f,
    0.775500000000000f,
    0.791120000000000f,
    0.806750000000000f,
    0.822370000000000f,
    0.838000000000000f,
    0.853620000000000f,
    0.869250000000000f,
    0.884870000000000f,
    0.900500000000000f,
    0.916120000000000f,
    0.931750000000000f,
    0.947370000000000f,
    0.963000000000000f,
    0.974540000000000f,
    0.999040000000000f
};
#endif