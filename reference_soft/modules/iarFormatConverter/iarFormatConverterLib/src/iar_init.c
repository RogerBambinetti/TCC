/***********************************************************************************
 
 This software module was originally developed by 
 
 Samsung Electronics
 
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
 
 Samsung Electronics retains full right to modify and use the code for 
 their own purpose, assign or donate the code to a third party and to inhibit third 
 parties from using the code for products that do not conform to MPEG-related ITU 
 Recommendations and/or ISO/IEC International Standards.
 
 This copyright notice must be included in all copies or derivative works. 
 
 Copyright (c) ISO/IEC 2013.
 
 ***********************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "iar_rom.h"
#include "iarFormatConverter_init.h"
#include <math.h>

#define SAMSUNG 1
#ifndef min
#define min(a,b) (( a < b ) ? a : b)
#endif
#ifndef max
#define max(a,b) (( a > b ) ? a : b)
#endif
#ifndef M_PI
#define M_PI                3.14159265358979323846264338327950288f
#endif

#define SAMSUNG_RAND5 1

static float iar_fc_QMF[71] = {
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

static float iar_fc_StftErb[58] = {
	0.000000000000000f,
	0.003891050583658f,
	0.007782101167315f,
	0.011673151750973f,
	0.015564202334630f,
	0.019455252918288f,
	0.023346303501946f,
	0.027237354085603f,
	0.031128404669261f,
	0.035019455252918f,
	0.038910505836576f,
	0.042801556420233f,
	0.046692607003891f,
	0.050583657587549f,
	0.054474708171206f,
	0.058365758754864f,
	0.062256809338521f,
	0.066147859922179f,
	0.070038910505837f,
	0.073929961089494f,
	0.077821011673152f,
	0.081712062256809f,
	0.085603112840467f,
	0.089494163424125f,
	0.093385214007782f,
	0.097276264591440f,
	0.101167315175097f,
	0.105058365758755f,
	0.108949416342412f,
	0.112840466926070f,
	0.116731517509728f,
	0.120622568093385f,
	0.124513618677043f,
	0.132295719844358f,
	0.143968871595331f,
	0.157587548638132f,
	0.173151750972763f,
	0.188715953307393f,
	0.204280155642023f,
	0.221789883268482f,
	0.241245136186770f,
	0.260700389105058f,
	0.284046692607004f,
	0.311284046692607f,
	0.338521400778210f,
	0.365758754863813f,
	0.394941634241245f,
	0.428015564202335f,
	0.464980544747082f,
	0.505836575875486f,
	0.550583657587549f,
	0.597276264591440f,
	0.647859922178988f,
	0.704280155642023f,
	0.764591439688716f,
	0.828793774319066f,
	0.898832684824903f,
	0.966926070038911f
};

void iar_setActiveDownmixRange (int fs);
void iar_setActiveDownmixRange (int fs)
{
	int		i;

	for ( i=0; i<71; i++ )
	{
		if		( fs * iar_fc_QMF[i] <  2800 *2 )	is4GVH[i]	= 0;
		else if ( fs * iar_fc_QMF[i] > 10000 *2 )	is4GVH[i]	= 0;
		else								is4GVH[i]	= 1;
	}
}

void iar_setActiveDownmixRange_StftErb (int fs);
void iar_setActiveDownmixRange_StftErb (int fs)
{
	int		i;

	for ( i=0; i<58; i++ )
	{
		if		( fs * iar_fc_StftErb[i] <  2800 *2 )	is4GVH_StftErb[i]	= 0;
		else if ( fs * iar_fc_StftErb[i] > 10000 *2 )	is4GVH_StftErb[i]	= 0;
		else								is4GVH_StftErb[i]	= 1;
	}
}

void iar_normalizePG ()
{
	int chin, chout;

	/* power normalization of GVH */
	for ( chin = TFC; chin<=VOG; chin++ )
	{
		float	P	= 0.0f;

		for ( chout = FL; chout <= SR; chout++ )
		{
			P	+=	GVH[chin][chout] * GVH[chin][chout];
		}

		P		= sqrtf (P);

		for ( chout = FL; chout <= SR; chout++ )
		{
			GVH[chin][chout] /= P;
		}
	}

	/* power normalization of GVL */
	for ( chin = TFC; chin<=VOG; chin++ )
	{
		float	P	= 0.0f;

		for ( chout = FL; chout <= SR; chout++ )
		{
			P	+=	GVL[chin][chout] * GVL[chin][chout];
		}

		P		= sqrtf (P);

		for ( chout = FL; chout <= SR; chout++ )
		{
			GVL[chin][chout] /= P;
		}
	}
}

void iar_makeIdentical ( int chin, int chout, int * flag )
{
	int		m;

	if	( flag[chin] )	return;

	for ( m=0; m<6; m++ )
	{
		GVH[chin][m]	= 0.0f;
		GVL[chin][m]	= 0.0f;  
	}

	GVH[chin][chout]	= 1.0f;
	GVL[chin][chout]	= 1.0f;

	flag[chin]			= 1;
}

void iar_makeDualmono ( int chin, int chout1, int chout2, int * flag )
{
	int		m;

	if	( flag[chin] )	return;

	for ( m=0; m<6; m++ )
	{
		GVH[chin][m]	= 0.0f;
		GVL[chin][m]	= 0.0f;  
	}

	GVH[chin][chout1]	= 1.0f;
	GVL[chin][chout1]	= 1.0f;
	GVH[chin][chout2]	= 1.0f;
	GVL[chin][chout2]	= 1.0f;

	flag[chin]			= 1;
}

void iar_keepCentralImag ( int dir, float ele, int * flag )
{
	int IL, CL, C;	/* ipsilateral, contralateral, central channel indexes */

	/* for the frontal elevated channel */
	if	( dir < FC )
	{
		if	( dir == FL )	
		{
			IL		= FL;	/* ipsilateral to the elevated channel */
			CL		= FR;	/* contralateral to the elevated channel */
			C		= FC;	/* central channel */
		}
		if	( dir == FR )
		{
			IL		= FR;	/* ipsilateral to the elevated channel */
			CL		= FL;	/* contralateral to the elevated channel */
			C		= FC;	/* central channel */
		}


		if	( flag[TFC] < 1 )
		{
			GVH[TFC][IL] *= (float)pow( 10.0f, 3.0f*ele/20.0f/35.0f );
			GVL[TFC][IL] *= (float)pow( 10.0f, 2.0f*ele/20.0f/35.0f );
			GVH[TFC][CL] *= (float)pow( 10.0f, 2.0f*ele/20.0f/35.0f );
			GVL[TFC][CL] *= (float)pow( 10.0f, 2.0f*ele/20.0f/35.0f );
			GVH[TFC][C ] *= (float)pow( 10.0f, 2.0f*ele/20.0f/35.0f );
			GVL[TFC][C ] *= (float)pow( 10.0f, 2.0f*ele/20.0f/35.0f );

			flag[TFC]	 += 1;
		}
		if	( flag[VOG] < 1 )
		{
			GVH[VOG][IL] *= (float)pow( 10.0f, 3.0f*ele/20.0f/35.0f );
			GVL[VOG][IL] *= (float)pow( 10.0f, 3.0f*ele/20.0f/35.0f );
			GVH[VOG][CL] *= (float)pow( 10.0f, 2.0f*ele/20.0f/35.0f );
			GVL[VOG][CL] *= (float)pow( 10.0f, 2.0f*ele/20.0f/35.0f );
			GVH[VOG][C ] *= (float)pow( 10.0f, 2.0f*ele/20.0f/35.0f );
			GVL[VOG][C ] *= (float)pow( 10.0f, 2.0f*ele/20.0f/35.0f );

			flag[VOG]	 += 1;
		}
		if	( flag[TBC] < 1 )
		{
			GVH[TBC][CL] *= (float)pow( 10.0f, -2.0f*ele/20.0f/35.0f );
			flag[TBC]	 += 1;
		}
	}
	/* for the rear elevated channel */
	else
	{
		if	( dir == SL )	
		{
			IL		= SL;	/* ipsilateral to the elevated channel */
			CL		= SR;	/* contralateral to the elevated channel */
		}
		if	( dir == SR )
		{
			IL		= SR;	/* ipsilateral to the elevated channel */
			CL		= SL;	/* contralateral to the elevated channel */
		}
		
		if	( flag[TBC] < 2 )
		{
			GVH[TBC][IL] *= (float)pow( 10.0f, 2.0f*ele/20.0f/35.0f );
			GVL[TBC][IL] *= (float)pow( 10.0f, 2.0f*ele/20.0f/35.0f );

			flag[TBC]	 += 1;
		}
		
		if	( flag[VOG] < 2 )
		{
			GVH[VOG][IL] *= (float)pow( 10.0f, 2.0f*ele/20.0f/35.0f );
			GVL[VOG][IL] *= (float)pow( 10.0f, 2.0f*ele/20.0f/35.0f );

			flag[VOG]	 += 1;
		}

		if	( flag[TFC] < 2 )
		{
			GVH[TFC][CL] *= (float)pow( 10.0f, -2.0f*ele/20.0f/35.0f );
			GVL[TFC][CL] *= (float)pow( 10.0f, -2.0f*ele/20.0f/35.0f );

			flag[TFC]	 += 1;
		}
	}
}

void iar_keepLRBalance ( int dir, float ele, int * flag )
{
	int IF, CF, IS, CS, O[5];

	if	( dir == FL )
	{
		IF		= FL;			/* ipsilateral frontal */
		CF		= FR;			/* contralateral frontal */
		IS		= SL;			/* ipsilateral rear */
		CS		= SR;			/* contralateral rear */
		O[0]	= TFR;			/* CH_U_L045/CH_U_R045 */
		O[1]	= TFRA;			/* CH_U_L030/CH_U_R030 */
		O[2]	= TSL;			/* CH_U_L090/CH_U_R090 */
		O[3]	= TBLA;			/* CH_U_L110/CH_U_R110 */
		O[4]	= TBL;			/* CH_U_L135/CH_U_R135 */
	}
	else if ( dir == FR )		
	{
		IF		= FR;
		CF		= FL;
		IS		= SR;
		CS		= SL;
		O[0]	= TFL;
		O[1]	= TFLA;
		O[2]	= TSR;
		O[3]	= TBRA;
		O[4]	= TBR;
	}

	if ( !flag[O[0]] )
	{
		GVH[O[0]][IF] *= (float)pow( 10.0f, -1.0f*ele/20.0f/35.0f );
		GVL[O[0]][IF] *= (float)pow( 10.0f, -1.0f*ele/20.0f/35.0f );
		GVH[O[0]][IS] *= (float)pow( 10.0f, -4.0f*ele/20.0f/35.0f );
		GVL[O[0]][IS] *= (float)pow( 10.0f, -4.0f*ele/20.0f/35.0f );

		flag[O[0]] = 1;
	}
	if ( !flag[O[1]] )
	{
		GVH[O[1]][IF] *= (float)pow( 10.0f, -1.0f*ele/20.0f/35.0f );
		GVL[O[1]][IF] *= (float)pow( 10.0f, -1.0f*ele/20.0f/35.0f );
		GVH[O[1]][IS] *= (float)pow( 10.0f, -4.0f*ele/20.0f/35.0f );
		GVL[O[1]][IS] *= (float)pow( 10.0f, -4.0f*ele/20.0f/35.0f );

		flag[O[1]] = 1;
	}
	if	( !flag[O[2]] )
	{
		GVH[O[2]][IF] *= (float)pow( 10.0f,  9.7f*ele/20.0f/35.0f );
		GVH[O[2]][IS] *= (float)pow ( 10.0f, -4.0f*ele/20.0f/35.0f );
		GVH[O[2]][CS]  = 0.0f;

		flag[O[2]] = 1;
	}
	if	( !flag[O[3]] )
	{
		GVH[O[3]][IF] *= (float)pow( 10.0f,  5.6f*ele/20.0f/35.0f );
		GVL[O[3]][IF] *= (float)pow( 10.0f,  5.6f*ele/20.0f/35.0f );
		GVH[O[3]][IS] *= (float)pow( 10.0f, -4.6f*ele/20.0f/35.0f );
		GVL[O[3]][IS] *= (float)pow( 10.0f, -4.6f*ele/20.0f/35.0f );
		GVH[O[3]][CS] *= (float)pow( 10.0f, -1.0f*ele/20.0f/35.0f );
		GVL[O[3]][CS] *= (float)pow( 10.0f, -1.0f*ele/20.0f/35.0f );

		flag[O[3]] = 1;
	}
	if	( !flag[O[4]] )
	{
		GVH[O[4]][IF] *= (float)pow( 10.0f,  5.6f*ele/20.0f/35.0f );
		GVL[O[4]][IF] *= (float)pow( 10.0f,  5.6f*ele/20.0f/35.0f );
		GVH[O[4]][IS] *= (float)pow( 10.0f, -4.6f*ele/20.0f/35.0f );
		GVL[O[4]][IS] *= (float)pow( 10.0f, -4.6f*ele/20.0f/35.0f );
		GVH[O[4]][CS] *= (float)pow( 10.0f, -1.0f*ele/20.0f/35.0f );
		GVL[O[4]][CS] *= (float)pow( 10.0f, -1.0f*ele/20.0f/35.0f );

		flag[O[4]] = 1;
	}

}

void iar_azimuthPanningPost ( float fbias, float bbias )
{
	if	( fbias > 10 )
	{
		GVH[TFC][FL] *= (float)pow( 10.0f, -1.0f*fbias/20.0f/15.0f );
		GVL[TFC][FL] *= (float)pow( 10.0f, -2.0f*fbias/20.0f/15.0f );
		GVH[VOG][FL] *= (float)pow( 10.0f, -1.0f*fbias/20.0f/15.0f );
		GVL[VOG][FL] *= (float)pow( 10.0f, -1.0f*fbias/20.0f/15.0f );
	}
	if	( fbias < -10 )
	{
		GVH[TFC][FR] *= (float)pow( 10.0f,  1.0f*fbias/20.0f/15.0f );
		GVL[TFC][FR] *= (float)pow( 10.0f,  2.0f*fbias/20.0f/15.0f );
		GVH[VOG][FR] *= (float)pow( 10.0f,  1.0f*fbias/20.0f/15.0f );
		GVL[VOG][FR] *= (float)pow( 10.0f,  1.0f*fbias/20.0f/15.0f );
	}
	if	( bbias > 10 )
	{
		GVH[TBC][SL] *= (float)pow( 10.0f,  1.0f*bbias/20.0f/15.0f );
		GVL[TBC][SL] *= (float)pow( 10.0f,  1.0f*bbias/20.0f/15.0f );
		GVH[VOG][SL] *= (float)pow( 10.0f,  1.0f*bbias/20.0f/15.0f );
		GVL[VOG][SL] *= (float)pow( 10.0f,  1.0f*bbias/20.0f/15.0f );
	}
	if	( bbias < -10 )
	{
		GVH[TBC][SR] *= (float)pow( 10.0f, -1.0f*bbias/20.0f/15.0f );
		GVL[TBC][SR] *= (float)pow( 10.0f, -1.0f*bbias/20.0f/15.0f );
		GVH[VOG][SR] *= (float)pow( 10.0f, -1.0f*bbias/20.0f/15.0f );
		GVL[VOG][SR] *= (float)pow( 10.0f, -1.0f*bbias/20.0f/15.0f );
	}
}

void iar_makePanner ( int chin, float * azi,  int * flag )
{
	float azi_1, azi_2, azi_target, mn, alpha, alpha0, center;
    float a1, a2, nrm;
	int		ch;

	if	( flag[chin] )	return;

	if	( chin == TSL )
	{
		azi_1		= azi[FL]+30;
		azi_2		= azi[SL]+110;
		azi_target	= 90;
	}
	else if ( chin == TSR )
	{
		azi_1		= azi[FR]-30;
		azi_2		= azi[SR]-110;
		azi_target	= -90;
	}

    mn         = (float)min((float)min(azi_1,azi_2), azi_target);
    azi_1      = azi_1 - mn;
    azi_2      = azi_2 - mn;
    azi_target = azi_target - mn;

	/* amplitude panning angle alpha0 */
    alpha0 = 0.5f * (float)fabs(azi_1-azi_2);
    /* center angle */
    center = 0.5f * (azi_1 + azi_2);
    /* amplitude panning angle alpha */
    alpha = center - azi_target;
    if (azi_1 > azi_2) {
        alpha = -alpha;
    }

	alpha0   = alpha0 * (float)M_PI / 180.0f;
    alpha    = alpha  * (float)M_PI / 180.0f;
    a2       = 1.0f;
    a1       = a2*((float)tan(alpha0)+(float)tan(alpha)+1e-10f)/((float)tan(alpha0)-(float)tan(alpha)+1e-10f);
    nrm      = 1.0f / (float)sqrt(a1*a1+a2*a2);
    a1       = a1 * nrm;
    a2       = a2 * nrm;

	for	(ch=FL; ch<=SR; ch++)
	{
		GVH[chin][ch]	= 0.0f;
		GVL[chin][ch]	= 0.0f;
	}

	if	( chin == TSL )
	{
		GVH[TSL][FL]	= a1;
		GVH[TSL][SL]	= a2;
		GVL[TSL][FL]	= a1;
		GVL[TSL][SL]	= a2;
	}
	if	( chin == TSR )
	{
		GVH[TSR][FR]	= a1;
		GVH[TSR][SR]	= a2;
		GVL[TSR][FR]	= a1;
		GVL[TSR][SR]	= a2;
	}

	flag[chin]	= 1;
}

#if SAMSUNG_RAND5
void iar_randElevSptlParms ( const float * randomization, int nchanout )
#else
void iar_randElevSptlParms ( const float * randomization )
#endif
{
	int		i, flag[13];
	float	rand_ele[6], rand_azi[6];
#if SAMSUNG_RAND5
	if	( nchanout == 6 )
	{
		for ( i=0; i<6; i++ )
		{
			rand_azi[i]		= randomization[2*i];
			rand_ele[i]		= randomization[2*i+1];
		}
	}
	else
	{
		for ( i=0; i<3; i++ )
		{
			rand_azi[i]		= randomization[2*i];
			rand_ele[i]		= randomization[2*i+1];
		}
		rand_azi[3]		= 0.0f;
		rand_ele[3]		= 0.0f;

		for ( i=3; i<5; i++ )
		{
			rand_azi[i+1]		= randomization[2*i];
			rand_ele[i+1]		= randomization[2*i+1];
		}
	}
#else
	for ( i=0; i<6; i++ )
	{
		rand_azi[i]		= randomization[2*i];
		rand_ele[i]		= randomization[2*i+1];
	}
#endif
	for ( i=0; i<13; i++ )
	{
		flag[i]		= 0;
	}

	/*	(1) find the "practically identical" */
	if	( rand_ele[FL] > 20 )
	{
		if	( abs((int)rand_azi[FL] )	<= 15 )	{	iar_makeIdentical	( TFLA,	FL,	flag );
													iar_makeIdentical	( TFL,	FL,	flag );		}
		if	( abs((int)rand_ele[SL] )	>  20 ) {	iar_makePanner		( TSL,	rand_azi, flag );	}
	}
	if	( rand_ele[FR] > 20 )
	{
		if	( abs((int)rand_azi[FR] )	<= 15 )	{	iar_makeIdentical	( TFRA,	FL,	flag );
													iar_makeIdentical	( TFR,	FL,	flag );		}
		if	( abs((int)rand_ele[SR] )	>  20 ) {	iar_makePanner		( TSR,	rand_azi, flag );	}
	}
	if	( rand_ele[FC] > 20 )
	{
		if	( abs((int)rand_azi[FC] )	<= 15 )		iar_makeIdentical	( TFC,	FC,	flag );
	}
	if	( rand_ele[SL] > 20 )
	{
		if	( abs((int)rand_azi[SL] )	<= 25 )	{	iar_makeIdentical	( TBLA,	SL, flag );
													iar_makeIdentical	( TBL,	SL, flag );
													iar_makeIdentical	( TSL,	SL, flag );		}
	}
	if	( rand_ele[SR] > 20 )
	{
		if	( abs((int)rand_azi[SR] )	<= 25 )	{	iar_makeIdentical	( TBRA,	SR, flag );
													iar_makeIdentical	( TBR,	SR, flag );
													iar_makeIdentical	( TSR,	SR, flag );		}
	}

	/*	(2) find the "practically dual mono" */
	if	( rand_ele[FL] > 20 && rand_ele[FR] > 20 )	iar_makeDualmono	( TFC, FL, FR, flag );
	if	( rand_ele[SL] > 20 && rand_ele[SR] > 20 )	iar_makeDualmono	( TBC, SL, SR, flag );

	/*	(3) keep the central image */
	if	( rand_ele[FL] > 20 && rand_ele[FR] <= 20 )	iar_keepCentralImag	( FL, rand_ele[FL], flag );
	if	( rand_ele[FR] > 20 && rand_ele[FL] <= 20 ) iar_keepCentralImag	( FR, rand_ele[FR], flag );
	if	( rand_ele[SL] > 20 && rand_ele[SR] <= 20 ) iar_keepCentralImag	( SL, rand_ele[SL], flag );
	if	( rand_ele[SR] > 20 && rand_ele[SL] <= 20 ) iar_keepCentralImag	( SR, rand_ele[SR], flag );

	/*	(4) keep the L/R balance */
	if	( rand_ele[FL] > 20 && rand_ele[FR] <= 20 ) iar_keepLRBalance	( FL, rand_ele[FL], flag );
	if	( rand_ele[FR] > 20 && rand_ele[FL] <= 20 ) iar_keepLRBalance	( FR, rand_ele[FR], flag );

	/*	(5) azimuth post-processing */
	iar_azimuthPanningPost	( rand_azi[FL]+rand_azi[FR], rand_azi[SL]+rand_azi[SR] );

	/*	(6) power normalization */
	iar_normalizePG ();
}
#if SAMSUNG_RAND5
void	iar_initElevSptlParms ( int elv, int num_band, int fs, const float * randomization, int nchanout )
#else
void	iar_initElevSptlParms ( int elv, int num_band, int fs, const float * randomization )
#endif
{
	float * eqin;
	float	w;
	float	tmp;
	int		band, VEQ, CH;
    float * iar_fc;

    switch (num_band) { /* select correct vector of center frequencies */
        case 71:         
            iar_fc = iar_fc_QMF;
            fprintf(stderr, "INFO: iar_initElevSptlParms() using 71 hybrid QMF bands.\n");
            break;
        case 58:
            iar_fc = iar_fc_StftErb;
            fprintf(stderr, "INFO: iar_initElevSptlParms() using 58 processing bands for STFT.\n");
            break;
        default:
            fprintf(stderr, "================= ERROR ================= \n");
            fprintf(stderr, "Unsupported number of bands in iar_initElevSptlParms(). Behavior undefined.\n");
            fprintf(stderr, "================= ERROR ================= \n");
    }

	iar_setActiveDownmixRange (fs);
	iar_setActiveDownmixRange_StftErb (fs);
	D2		= (int) ( fs * 0.003 / 64 + 0.5 );

	for ( CH=0; CH<13; CH++ )
	{
		topIn[CH]	= -1;
	}

#if SAMSUNG_RAND5
	for ( CH=0; CH<nchanout; CH++ )
	{
		midOut[CH]	= CH;	/* 5.1 layout */
	}
	if ( nchanout == 5 )
	{
		midOut[3]++;
		midOut[4]++;
	}
#else
	for ( CH=0; CH<6; CH++ )
	{
		midOut[CH]	= CH;	/* 5.1 layout */
	}
#endif
	elv		= min ( max ( 0, elv-35 ), 25 );
	
	if	( elv > 0 )	
	{
		/* update the spatial elevation filter */
		{
			/* selection of EQ_0,lin(eq(i_in)) */
			for ( VEQ = IAR_RULE_EQVF; VEQ <= IAR_RULE_EQVBA; VEQ ++ )
			{
                switch (num_band) { /* select equalizer frequency response vectors depending on filter bank */
                    case 71:
                    switch ( VEQ )
                    {
                        case IAR_RULE_EQVF  :       eqin	= EQVF ;	break;
                        case IAR_RULE_EQVB  :       eqin	= EQVB ;	break;
                        case IAR_RULE_EQVFC :       eqin	= EQVFC;	break;
                        case IAR_RULE_EQVBC :       eqin	= EQVBC;	break;
                        case IAR_RULE_EQVOG :       eqin	= EQVOG;	break;
                        case IAR_RULE_EQVS  :       eqin	= EQVS ;	break;
                        case IAR_RULE_EQBTM :       eqin	= EQBTM;	break;
                        case IAR_RULE_EQVBA :       eqin	= EQVBA;	break;
                    }
                    break;
                    case 58:
                    switch ( VEQ )
                    {
                        case IAR_RULE_EQVF  :       eqin	= EQVF_StftErb ;	break;
                        case IAR_RULE_EQVB  :       eqin	= EQVB_StftErb ;	break;
                        case IAR_RULE_EQVFC :       eqin	= EQVFC_StftErb;	break;
                        case IAR_RULE_EQVBC :       eqin	= EQVBC_StftErb;	break;
                        case IAR_RULE_EQVOG :       eqin	= EQVOG_StftErb;	break;
                        case IAR_RULE_EQVS  :       eqin	= EQVS_StftErb ;	break;
                        case IAR_RULE_EQBTM :       eqin	= EQBTM_StftErb;	break;
                        case IAR_RULE_EQVBA :       eqin	= EQVBA_StftErb;	break;
                    }
                    break;
                }

				/* calculation of spatial elevation filter */
				for ( band = 0; band < num_band; band++ )
				{			
					/* selection of high frequency decay curve : frontal or not */
					switch ( VEQ )
					{
						case IAR_RULE_EQVF:
						case IAR_RULE_EQVFC:
						case IAR_RULE_EQVOG:
							w	= 0.05f * logf(iar_fc[band]*fs/6000.0f) / logf(2.0f);
							break;
						default :
							w	= 0.07f * logf(iar_fc[band]*fs/6000.0f) / logf(2.0f);
					}

					/* calculation of EQ_1,db */
					if	( iar_fc[band]*fs > 8000 )
						tmp		= 20 * logf(eqin[band]) / logf(10.0f) + w + 1;
					else
						tmp		= 20 * logf(eqin[band]) / logf(10.0f) + 1;

					/* calculation of EQ_2,db and G_EQ */
					if	( VEQ != IAR_RULE_EQVOG && VEQ != IAR_RULE_EQBTM )
					{
						/* EQ_2,db */
						tmp		*= (1+elv*0.05f);
						tmp		-= 1;
						/* G_EQ */
						tmp		= (float)pow( 10.0f, tmp/20.0f );
						if	( iar_fc[band]*fs > 8000 )
							tmp *= (float)pow( 10.0f, -w );

						eqin[band]	= tmp;
					}
				}
			}
		}

		/* update the spatial panning coefficients filter */
		{
			float gI, gC;

			GVH[TFC][SL]	*= (float)pow( 10.0f, 0.25f*elv/20.0f );
			GVH[TFC][SR]	*= (float)pow( 10.0f, 0.25f*elv/20.0f );
	
			/* for the channels @ SIDE */
			gI				 = (float)pow( 10.0f, -0.05522f*elv/20.0f );
			gC				 = (float)pow( 10.0f, 0.41879f*elv/20.0f );

			GVH[TSL][FL]	*= gI;
			GVH[TSL][SL]	*= gI;
			GVH[TSL][FR]	*= gC;
			GVH[TSL][SR]	*= gC;
			GVH[TSR][FR]	*= gI;
			GVH[TSR][SR]	*= gI;
			GVH[TSR][FL]	*= gC;
			GVH[TSR][SL]	*= gC;

			/* for the channels @ FRONT or REAR */
			gI				 = (float)pow( 10.0f, -0.047401f*elv/20.0f );
			gC				 = (float)pow( 10.0f, 0.14985f*elv/20.0f );

			GVH[TFL ][FL]	*= gI;
			GVH[TFL ][SL]	*= gI;
			GVH[TFL ][FR]	*= gC;
			GVH[TFL ][SR]	*= gC;
			GVH[TFR ][FR]	*= gI;
			GVH[TFR ][SR]	*= gI;
			GVH[TFR ][FL]	*= gC;
			GVH[TFR ][SL]	*= gC;
			GVH[TBL ][FL]	*= gI;
			GVH[TBL ][SL]	*= gI;
			GVH[TBL ][FR]	*= gC;
			GVH[TBL ][SR]	*= gC;
			GVH[TBR ][FR]	*= gI;
			GVH[TBR ][SR]	*= gI;
			GVH[TBR ][FL]	*= gC;
			GVH[TBR ][SL]	*= gC;
			GVH[TFLA][FL]	*= gI;
			GVH[TFLA][SL]	*= gI;
			GVH[TFLA][FR]	*= gC;
			GVH[TFLA][SR]	*= gC;
			GVH[TFRA][FR]	*= gI;
			GVH[TFRA][SR]	*= gI;
			GVH[TFRA][FL]	*= gC;
			GVH[TFRA][SL]	*= gC;
			GVH[TBLA][FL]	*= gI;
			GVH[TBLA][SL]	*= gI;
			GVH[TBLA][FR]	*= gC;
			GVH[TBLA][SR]	*= gC;
			GVH[TBRA][FR]	*= gI;
			GVH[TBRA][SR]	*= gI;
			GVH[TBRA][FL]	*= gC;
			GVH[TBRA][SL]	*= gC;

			/* for the channels @ REAR */
			GVL[TBL ][FL]	*= gI;
			GVL[TBL ][SL]	*= gI;
			GVL[TBL ][FR]	*= gC;
			GVL[TBL ][SR]	*= gC;
			GVL[TBR ][FR]	*= gI;
			GVL[TBR ][SR]	*= gI;
			GVL[TBR ][FL]	*= gC;
			GVL[TBR ][SL]	*= gC;
			GVL[TBLA][FL]	*= gI;
			GVL[TBLA][SL]	*= gI;
			GVL[TBLA][FR]	*= gC;
			GVL[TBLA][SR]	*= gC;
			GVL[TBRA][FR]	*= gI;
			GVL[TBRA][SR]	*= gI;
			GVL[TBRA][FL]	*= gC;
			GVL[TBRA][SL]	*= gC;
		}
		iar_normalizePG ();
	}

	/* update panning coefficients if random */
#if SAMSUNG_RAND5
	if	( randomization != NULL )	iar_randElevSptlParms ( randomization, nchanout );
#else
	if	( randomization != NULL )	iar_randElevSptlParms ( randomization );
#endif
	return;
}

void iar_compute_ieq ( iar_converter_pr_t * params, int inidx, int outidx, int nb )
{
	float	ul, ll;
	int		n;

	ul	= 1.5849f;
	ll	= 0.6310f;

	for ( n=0; n<nb; n++ )
	{
		params->eq[outidx][n]	= min ( ul, max ( ll, (params->ieq[inidx][n]) ) );
	}
}
