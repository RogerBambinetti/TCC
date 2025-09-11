#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "ic_defs.h"
#include "ic_rom.h"

enum{
  ID_SCE                = 0,
  ID_CPE,
  ID_LFE,
  ID_EXT,
};

static int	getOutputChannelwithIC ( void );

void setQCEs(int ch1A, int ch1B, int ch2A, int ch2B )
{
	ICDmx.chQCEs[0]		= ch1A;
	ICDmx.chQCEs[1]		= ch1B;
	ICDmx.chQCEs[2]		= ch2A;
	ICDmx.chQCEs[3]		= ch2B;
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
        
        if (eq_1_idx == 1) {
            eq[i] *= peak_filter(12000.0f, 0.3f, -2.0f*eq_1_strength, 1.0f*eq_1_strength, f);
        }
        if (eq_2_idx == 1) {
            eq[i] *= peak_filter(12000.0f, 0.3f, -2.0f*eq_2_strength, 1.0f*eq_2_strength, f);
        }
        
        /* EQ2: for mixing surround height to surround horizontal */
        
        if (eq_1_idx == 2) {
            eq[i] *= peak_filter(12000.0f, 0.3f, -3.5f*eq_1_strength, 1.0f*eq_1_strength, f);
        }
        if (eq_2_idx == 2) {
            eq[i] *= peak_filter(12000.0f, 0.3f, -3.5f*eq_2_strength, 1.0f*eq_2_strength, f);
        }
        
        /* EQ3: for mixing top to height loudspeakers */
        
        if (eq_1_idx == 3) {
            eq[i] *= peak_filter(200.0f, 0.3f, -6.5f*eq_1_strength, 0.7f*eq_1_strength, f);
            eq[i] *= peak_filter(1300.0f, 0.5f, 1.8f*eq_1_strength, 0.0f*eq_1_strength, f);
            eq[i] *= peak_filter(600.0f, 1.0f, 2.0f*eq_1_strength, 0.0f*eq_1_strength, f);
        }
        if (eq_2_idx == 3) {
            eq[i] *= peak_filter(200.0f, 0.3f, -6.5f*eq_2_strength, 0.7f*eq_2_strength, f);
            eq[i] *= peak_filter(1300.0f, 0.5f, 1.8f*eq_2_strength, 0.0f*eq_2_strength, f);
            eq[i] *= peak_filter(600.0f, 1.0f, 2.0f*eq_2_strength, 0.0f*eq_2_strength, f);
        }
        
        /* EQ4: for mixing top to horizontal loudspeakers */
        
        if (eq_1_idx == 4) {
            eq[i] *= peak_filter(5000.0f, 1.0f, 4.5f*eq_1_strength, -3.1f*eq_1_strength, f);
            eq[i] *= peak_filter(1100.0f, 0.8f, 1.8f*eq_1_strength, 0.0f*eq_1_strength, f);
        }
        if (eq_2_idx == 4) {
            eq[i] *= peak_filter(5000.0f, 1.0f, 4.5f*eq_2_strength, -3.1f*eq_2_strength, f);
            eq[i] *= peak_filter(1100.0f, 0.8f, 1.8f*eq_2_strength, 0.0f*eq_2_strength, f);
        }
        
        /* EQ5: for mixing M to U for rand5 */
        
        if (eq_1_idx == 5) {
            eq[i] *= peak_filter(35.0f, 0.25f, -1.3f*eq_1_strength, 1.0f*eq_1_strength, f);
        }
        if (eq_2_idx == 5) {
            eq[i] *= peak_filter(35.0f, 0.25f, -1.3f*eq_2_strength, 1.0f*eq_2_strength, f);
        }
    }
}


void loadEQs ( void )
{
  int		i;

  for ( i=0; i<100; i++ ) {
    ic_EQ[A][i]	= 1.0f;
  }

  compute_eq(f_bands_nrm, 71, 48000, 1, 1.0, 0, 0.0, ic_EQ[B] );
  compute_eq(f_bands_nrm, 71, 48000, 2, 1.0, 0, 0.0, ic_EQ[C] );
  compute_eq(f_bands_nrm, 71, 48000, 4, 1.0, 0, 0.0, ic_EQ[D] );
}

void ICGConfig ( void )
{
  unsigned int elemIdx;
	int elemCPE, i;
	unsigned char bitindex[8];
	int bitcnt	= 1, bytecnt = 0;

	for ( i=6, bitindex[7]=1; i>=0; i-- )
	{
		bitindex[i]		= bitindex[i+1]*2;
	}
	
	ICConfig.ICPresent		= (ICConfig.bitstream[0] & bitindex[0]) > 0;
	

	if ( ICConfig.ICPresent )
	{
		for ( elemIdx=0, elemCPE=0; elemIdx < ICConfig.numElements; elemIdx++ )
		{
			if ( ICConfig.ElementType[elemIdx] == ID_CPE )
			{
				/* although elemCPE is used in the standard text, eleIdx is used for the convenience */
				ICConfig.ICinCPE[elemIdx]	= (ICConfig.bitstream[bytecnt] & bitindex[bitcnt]) > 0;
				elemCPE++;

				bitcnt++;				
				if ( bitcnt == 8 )
				{
					bytecnt++;
					bitcnt = 0;
				}
			}
		}
		
		ICConfig.ICGPrePresent	= (ICConfig.bitstream[bytecnt] & bitindex[bitcnt]) > 0;

		bitcnt++;				
		if ( bitcnt == 8 )
		{
			bytecnt++;
			bitcnt = 0;
		}

		if ( ICConfig.ICGPrePresent )
		{
			for ( elemIdx=0, elemCPE=0; elemIdx<ICConfig.numElements; elemIdx++ )
			{
				if ( ICConfig.ElementType[elemIdx] == ID_CPE )
				{
					/* although elemCPE is used in the standard text, eleIdx is used for the convenience */
					ICConfig.ICGPreAppliedCPE[elemIdx]	= (ICConfig.bitstream[bytecnt] & bitindex[bitcnt]) > 0;
					elemCPE++;

					bitcnt++;				
					if ( bitcnt == 8 )
					{
						bytecnt++;
						bitcnt = 0;
					}

				}
			}
		}
	}

}

void initICConfig ( void )
{
	unsigned int		i;
	/*ICConfig.numElements	= 14;*/
	loadEQs ();

	for ( i=0; i<ICConfig.numElements; i++ )
	{
		ICConfig.lblOut[i]				= CHN_EMPTY;
	}

	ICGConfig();
#ifdef NO_FHG_BUGFIX_20160216
  if ( ICConfig.ICPresent)
#endif
	  getOutputChannelwithIC();
}

void loadEQnGain4CPE ( int chA, int chB )
{
	ICDmx.G[0]		= ic_gains[ICRules[ICConfig.lbl[chA]].gtype];
	ICDmx.G[1]		= ic_gains[ICRules[ICConfig.lbl[chB]].gtype];
	ICDmx.EQ[0]		= ic_EQ[ICRules[ICConfig.lbl[chA]].eqidx];
	ICDmx.EQ[1]		= ic_EQ[ICRules[ICConfig.lbl[chB]].eqidx];
}

float calICG (float cldL, float cldR, int band )
{
	float ret, l, r;

	l		= cldL * ICDmx.G[0] * ICDmx.EQ[0][band]; 
	r		= cldR * ICDmx.G[1] * ICDmx.EQ[1][band];
	ret		= sqrt ( l*l + r*r );

	return		ret;
}

float calInvICG (float cldL, float cldR, int band )
{
	float ret, l, r;
	l		= cldL * ICDmx.G[0] * ICDmx.EQ[0][band]; 
	r		= cldR * ICDmx.G[1] * ICDmx.EQ[1][band];

	if ( l+r)
		ret		= sqrt ( (cldL*cldL+cldR*cldR)/(l*l+r*r) );
	else
		ret		= 1;

	return	ret;
}



void resetICConfig ( void )
{
	ICConfig.isICON		= IC_OFF;
	ICConfig.QCE		= 0;
}

void setICConfig (int instance, int isQCE)
{
	if	( isQCE )
	{
		if		( ICConfig.ICinCPE[instance] && ICConfig.ICinCPE[instance+1] )
		{
			if ( ICConfig.isStereoOut )
			{
				if		( ICConfig.ICGPreAppliedCPE[instance] )	ICConfig.isICON		= IC_PRE_STR;
				else											ICConfig.isICON		= IC_POST_STR;
			}
			else
			{
				if		( ICConfig.ICGPreAppliedCPE[instance] ) ICConfig.isICON		= IC_PRE_MUL;
				else											ICConfig.isICON		= IC_POST_MUL;
			}
		}
		else
		{
			ICConfig.isICON		= IC_OFF;
		}
		ICConfig.QCE	= isQCE;
	}
	else
	{
		if		( ICConfig.ICinCPE[instance] )
		{
			if ( ICConfig.isStereoOut )
			{
				if		( ICConfig.ICGPreAppliedCPE[instance] )	ICConfig.isICON		= IC_PRE_STR;
				else											ICConfig.isICON		= IC_POST_STR;
			}
			else
			{
				if		( ICConfig.ICGPreAppliedCPE[instance] ) ICConfig.isICON		= IC_PRE_MUL;
				else											ICConfig.isICON		= IC_POST_MUL;
			}
		}
		else
		{
			ICConfig.isICON		= IC_OFF;
		}

		ICConfig.QCE	= isQCE;

	}
}

static int	getOutputChannelwithIC ( void )
{
  unsigned int i;
	int          si, in, out;
	FILE	*      fp;

  if	( ICConfig.isStereoOut == 0 )
	{
		memcpy ( ICConfig.lblOut, ICConfig.lbl, sizeof (int)*64 );
		ICConfig.nspkrOut	= ICConfig.nspkr;

		if ( ICConfig.spkrFileName[0] )
		{
			fp	= fopen ( ICConfig.spkrFileName, "wt" );
			fprintf ( fp, "%d\n", ICConfig.nspkrOut );
			for ( si=0; si<ICConfig.nspkr; si++ )
			{
				fprintf ( fp, "c,%d\n", ICConfig.lbl[si] );
			}
			fclose (fp);
		}
		return	0;
	}

	for (i=0, out=0, ICConfig.nspkrOut=0; i<ICConfig.numElements; i++ )
	{
		if	( ICConfig.ElementType[i] == ID_SCE )
		{
			for ( in=0; in<ICConfig.nspkr; in++ )
			{
				if (ICConfig.lbl[in] == ICConfig.CPEOut[i][0])
				{
					ICConfig.isOutIC[in]	= 1;
					ICConfig.lblOut[in]	= ICConfig.CPEOut[i][0];
					ICConfig.nspkrOut++;
					break;
				}
			}
		}
		else if	( ICConfig.ElementType[i] == ID_LFE )
		{
			for ( in=0; in<ICConfig.nspkr; in++ )
			{
				if (ICConfig.lbl[in] == ICConfig.CPEOut[i][0])
				{
					ICConfig.isOutIC[in]	= 1;
					ICConfig.lblOut[in]	= CHN_I_LFE;
					ICConfig.nspkrOut++;
					break;
				}
			}
		}
		else if ( ICConfig.ElementType[i] == ID_CPE )
		{
			if	( ICConfig.ICinCPE[i] == 1 )
			{
				if	( ICRules[ICConfig.CPEOut[i][0]].ctype == ICRules[ICConfig.CPEOut[i][1]].ctype )
				{
					for (in=0; in<ICConfig.nspkr; in++ )
					{
						if ( ICConfig.lbl[in] == ICConfig.CPEOut[i][0] )
						{
							ICConfig.isOutIC[in]	= 1;
							if		( ICRules[ICConfig.CPEOut[i][0]].ctype == L ) ICConfig.lblOut[in]	= CHN_I_LEFT;
							else if ( ICRules[ICConfig.CPEOut[i][0]].ctype == R ) ICConfig.lblOut[in]	= CHN_I_RIGHT;
							else if ( ICRules[ICConfig.CPEOut[i][0]].ctype == C ) 
							{
								if (ICConfig.lbl[in] == CHN_LFE1 || ICConfig.lbl[in] == CHN_LFE2 || ICConfig.lbl[in] == CHN_LFE3 )
									ICConfig.lblOut[in]	= CHN_I_LFE;
								else
									ICConfig.lblOut[in]	= CHN_I_CNTR;
							}
							ICConfig.nspkrOut ++;
							break;
						}
					}
				}
				else
				{
					 /*bitstream error*/
					exit(-100);
				}
			}
			else
			{
				for ( in=0; in<ICConfig.nspkr; in++ )
				{
					if (ICConfig.lbl[in] == ICConfig.CPEOut[i][0])
					{
						ICConfig.isOutIC[in]	= 1;
						ICConfig.lblOut[in]		= ICConfig.CPEOut[i][0];
						ICConfig.nspkrOut++;
					}
					else if (ICConfig.lbl[in] == ICConfig.CPEOut[i][1])
					{
						ICConfig.isOutIC[in]	= 1;
						ICConfig.lblOut[in]		= ICConfig.CPEOut[i][1];
						ICConfig.nspkrOut++;
					}
				}
			}
		}
	}

	/* Wrting internal channel index file for the format converter*/
	if ( ICConfig.spkrFileName[0] )
	{
		fp	= fopen ( ICConfig.spkrFileName, "wt" );
		fprintf ( fp, "%d\n", ICConfig.nspkrOut );

		for ( si=0, out=0; si<ICConfig.nspkr; si++ )
		{
			if (ICConfig.isOutIC[si] == 1)
			{
				fprintf ( fp, "c,%d\n", ICConfig.lblOut[si] );
			}
		}
		fclose(fp);
	}

  return 0;
}