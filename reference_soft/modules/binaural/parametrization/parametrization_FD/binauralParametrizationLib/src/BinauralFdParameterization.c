/***********************************************************************************

This software module was originally developed by 

Fraunhofer IIS, ETRI, Yonsei University, WILUS Institute

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

Fraunhofer IIS, ETRI, Yonsei University, WILUS Institute retain full right to modify 
and use the code for its own purpose, assign or donate the code to a third party and
to inhibit third parties from using the code for products that do not conform to 
MPEG-related ITU Recommendations and/or ISO/IEC International Standards.

This copyright notice must be included in all copies or derivative works. 

Copyright (c) ISO/IEC 2014.

***********************************************************************************/

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "BinauralFdParameterization.h"
#include "cicp2geometry.h"
#include "fftlib.h"

#define EARLYTIME 80e-3
#define MAX_QMF_FILTERLENGTH 128

#ifndef BUGFIX_KCONV_KMAX_15JUNE
#define NUM_BAND_PROC 48
#define NUM_BAND_CONV 32
#endif
#define THR_DE -20
#define BAND_CRITICAL 0

static __inline int IsLittleEndian (void); 
static __inline short LittleEndian16 (short v); 
float ReverseFloat(const float inFloat ); 
static unsigned int BigEndian32 (char a, char b, char c, char d); 


static double B[24][7] = {
	{0.000000002978966, 0.000000000000000, -0.000000008936898, 0.000000000000000, 0.000000008936898, 0.000000000000000, -0.000000002978966},
	{0.000000005899584, 0.000000000000000, -0.000000017698751, 0.000000000000000, 0.000000017698751, 0.000000000000000, -0.000000005899584},
	{0.000000011789497, 0.000000000000000, -0.000000035368490, 0.000000000000000, 0.000000035368490, 0.000000000000000, -0.000000011789497},
	{0.000000023548606, 0.000000000000000, -0.000000070645819, 0.000000000000000, 0.000000070645819, 0.000000000000000, -0.000000023548606},
	{0.000000047026089, 0.000000000000000, -0.000000141078268, 0.000000000000000, 0.000000141078268, 0.000000000000000, -0.000000047026089},
	{0.000000093874874, 0.000000000000000, -0.000000281624623, 0.000000000000000, 0.000000281624623, 0.000000000000000, -0.000000093874874},
	{0.000000187307829, 0.000000000000000, -0.000000561923486, 0.000000000000000, 0.000000561923486, 0.000000000000000, -0.000000187307829},
	{0.000000373504653, 0.000000000000000, -0.000001120513960, 0.000000000000000, 0.000001120513960, 0.000000000000000, -0.000000373504653},
	{0.000000744228866, 0.000000000000000, -0.000002232686597, 0.000000000000000, 0.000002232686597, 0.000000000000000, -0.000000744228866},
	{0.000001481490914, 0.000000000000000, -0.000004444472741, 0.000000000000000, 0.000004444472741, 0.000000000000000, -0.000001481490914},
	{0.000002945568317, 0.000000000000000, -0.000008836704950, 0.000000000000000, 0.000008836704950, 0.000000000000000, -0.000002945568317},
	{0.000005847694618, 0.000000000000000, -0.000017543083853, 0.000000000000000, 0.000017543083853, 0.000000000000000, -0.000005847694618},
	{0.000011587291750, 0.000000000000000, -0.000034761875251, 0.000000000000000, 0.000034761875251, 0.000000000000000, -0.000011587291750},
	{0.000022906469387, 0.000000000000000, -0.000068719408160, 0.000000000000000, 0.000068719408160, 0.000000000000000, -0.000022906469387},
	{0.000045150610814, 0.000000000000000, -0.000135451832443, 0.000000000000000, 0.000135451832443, 0.000000000000000, -0.000045150610814},
	{0.000088673275133, 0.000000000000000, -0.000266019825400, 0.000000000000000, 0.000266019825400, 0.000000000000000, -0.000088673275133},
	{0.000173370214378, 0.000000000000000, -0.000520110643135, 0.000000000000000, 0.000520110643135, 0.000000000000000, -0.000173370214378},
	{0.000337103447126, 0.000000000000000, -0.001011310341379, 0.000000000000000, 0.001011310341379, 0.000000000000000, -0.000337103447126},
	{0.000651075235971, 0.000000000000000, -0.001953225707913, 0.000000000000000, 0.001953225707913, 0.000000000000000, -0.000651075235971},
	{0.001247284713280, 0.000000000000000, -0.003741854139840, 0.000000000000000, 0.003741854139840, 0.000000000000000, -0.001247284713280},
	{0.002366322106562, 0.000000000000000, -0.007098966319686, 0.000000000000000, 0.007098966319686, 0.000000000000000, -0.002366322106562},
	{0.004438169688363, 0.000000000000000, -0.013314509065088, 0.000000000000000, 0.013314509065088, 0.000000000000000, -0.004438169688363},
	{0.008214651510826, 0.000000000000000, -0.024643954532479, 0.000000000000000, 0.024643954532479, 0.000000000000000, -0.008214651510826},
	{0.014980703571592, 0.000000000000000, -0.044942110714776, 0.000000000000000, 0.044942110714776, 0.000000000000000, -0.014980703571592}
};

static double A[24][7] = {
	{1.000000000000000, -5.993751016763135, 14.969279072784566, -19.939603992021354, 14.940647795274490, -5.970844821630246, 0.994272962360464},
	{1.000000000000000, -5.991961445134535, 14.960639671319811, -19.922939882503549, 14.924596293523440, -5.963124398568642, 0.992789761382594},
	{1.000000000000000, -5.989609767964868, 14.949371631318428, -19.901379920011749, 14.904008213044975, -5.953314349635694, 0.990924193325314},
	{1.000000000000000, -5.986493089373522, 14.934568071720706, -19.873322981098088, 14.877492822310906, -5.940823535229365, 0.988578711974606},
	{1.000000000000000, -5.982322583166484, 14.914956502895425, -19.836561944151942, 14.843176218160425, -5.924879679481255, 0.985631486962872},
	{1.000000000000000, -5.976681836082384, 14.888729126620820, -19.788018121780837, 14.798506980371531, -5.904466854668645, 0.981930710405792},
	{1.000000000000000, -5.968963204438088, 14.853287019657653, -19.723347050471695, 14.739973841057109, -5.878238363575107, 0.977287777184542},
	{1.000000000000000, -5.958270126407491, 14.804853333473961, -19.636351633672902, 14.662693694537232, -5.844394434554799, 0.971469244020969},
	{1.000000000000000, -5.943266716356360, 14.737888463499093, -19.518109356078305, 14.559810012283357, -5.800509649746865, 0.964187554640025},
	{1.000000000000000, -5.921945879281351, 14.644209432330609, -19.355682487394688, 14.421618545659840, -5.743289060834721, 0.955090675488847},
	{1.000000000000000, -5.891272053317806, 14.511677079902135, -19.130239207390304, 14.234312652347755, -5.668224665450618, 0.943751061239065},
	{1.000000000000000, -5.846632613931893, 14.322276215973289, -18.814390396441560, 13.978227786574614, -5.569116536160665, 0.929654821150504},
	{1.000000000000000, -5.781001286253467, 14.049405018511159, -18.368601439334121, 13.625500348622651, -5.437419214533596, 0.912192664215007},
	{1.000000000000000, -5.683678077357811, 13.654293601443985, -17.736819219552288, 13.137230968058500, -5.261383586695196, 0.890655254306507},
	{1.000000000000000, -5.538431150435243, 13.081896267444138, -16.842288758666236, 12.460762205420412, -5.025009003406554, 0.864237092185886},
	{1.000000000000000, -5.320855496910868, 12.257825487897835, -15.586541172177160, 11.528970962883021, -4.706945111668716, 0.832054991343535},
	{1.000000000000000, -4.994862939199035, 11.090899392402983, -13.858558427508518, 10.266297975027744, -4.279774723365612, 0.793189508854480},
	{1.000000000000000, -4.508633849835456, 9.492307997838553, -11.567081708283867, 8.611562042517182, -3.710717285192899, 0.746759879904296},
	{1.000000000000000, -3.791566479016410, 7.433709378886649, -8.711008375712691, 6.575213188380568, -2.965921277956436, 0.692044028301664},
	{1.000000000000000, -2.756730866820556, 5.078384616947228, -5.476030603444230, 4.351407566193580, -2.022277271752888, 0.628653072944205},
	{1.000000000000000, -1.319726797918420, 3.001645881863894, -2.235723204296885, 2.473284706980526, -0.892513256112752, 0.556761178207907},
	{1.000000000000000, 0.543821839225891, 2.367900783391379, 0.842176405781307, 1.858884029029895, 0.331706060878868, 0.477372283671088},
	{1.000000000000000, 2.667654841026451, 4.479530540428327, 4.526976093111068, 3.286179336952533, 1.426496187396251, 0.392570938427190},
	{1.000000000000000, 4.472803785543190, 8.630679353924112, 9.234059565895342, 5.792824924160126, 2.020263319760948, 0.305653299838751}
};

static double B_LP[7] = {4.07451850037432e-14,
	2.44471110022459e-13,
	6.11177775056149e-13,
	8.14903700074865e-13,
	6.11177775056149e-13,
	2.44471110022459e-13,
	4.07451850037432e-14};

static double A_LP[7] = {1.000000000000000,	
	-5.95448186943460,	
	14.7734441411088,
	-19.5489429752463,	
	14.5509829161372,	
	-5.77650412293260,	
	0.955501910370147};

static int Filter_Delay[24] = {702,
	1448,
	1202,
	870,
	731,
	478,
	368,
	284,
	224,
	177,
	142,
	110,
	88,
	69,
	55,
	44,
	35,
	27,
	22,
	17,
	14,
	11,
	8,
	6};

int binauralParam_BRIRtoQMF(H_BRIRANALYSISSTRUCT h_BRIRs, int maxband, int min_length)
{
	float q[192] = {-0.202934338f, -0.198033159f, -0.192941152f, -0.187674422f, -0.182247401f, 
		-0.17667302f, -0.170962864f, -0.165127301f, -0.159175602f, -0.153116046f, 
		-0.146956001f, -0.140702013f, -0.134359874f, -0.127934679f, -0.121430888f, 
		-0.114852369f, -0.108202445f, -0.101483934f, -0.094699178f, -0.08785008f, 
		-0.080938127f, -0.073964417f, -0.066929683f, -0.059834308f, -0.052678347f, 
		-0.045461539f, -0.038183325f, -0.030842857f, -0.023439012f, -0.015970396f, 
		-0.008435358f, -0.000831996f, 0.006841844f, 0.014588553f, 0.022410765f, 
		0.03031135f, 0.038293413f, 0.046360296f, 0.054515579f, 0.062763081f, 
		0.071106866f, 0.079551245f, 0.088100788f, 0.096760326f, 0.105534966f, 
		0.1144301f, 0.123451422f, 0.132604943f, 0.141897012f, 0.151334337f, 
		0.160924013f, 0.170673552f, 0.180590919f, 0.190684575f, 0.200963519f, 
		0.211437346f, 0.222116308f, 0.233011387f, 0.244134374f, 0.255497966f, 
		0.26711587f, 0.279002924f, 0.291175235f, 0.303650335f, 0.902527571f, 
		0.91035852f, 0.917697783f, 0.924576068f, 0.931021458f, 0.937059674f, 
		0.942714314f, 0.948007061f, 0.952957857f, 0.957585067f, 0.961905616f, 
		0.965935107f, 0.96968793f, 0.973177355f, 0.976415612f, 0.979413964f, 
		0.982182769f, 0.984731538f, 0.987068979f, 0.989203046f, 0.991140973f, 
		0.992889307f, 0.99445394f, 0.995840132f, 0.997052535f, 0.998095212f, 
		0.99897165f, 0.999684781f, 1.000236984f, 1.000630103f, 1.000865448f, 
		1.000943806f, 1.000865448f, 1.000630103f, 1.000236984f, 0.999684781f, 
		0.99897165f, 0.998095212f, 0.997052535f, 0.995840132f, 0.99445394f, 
		0.992889307f, 0.991140973f, 0.989203046f, 0.987068979f, 0.984731538f, 
		0.982182769f, 0.979413964f, 0.976415612f, 0.973177355f, 0.96968793f, 
		0.965935107f, 0.961905616f, 0.957585067f, 0.952957857f, 0.948007061f, 
		0.942714314f, 0.937059674f, 0.931021458f, 0.924576068f, 0.917697783f, 
		0.91035852f, 0.902527571f, 0.894171297f, 0.291175235f, 0.279002924f, 
		0.26711587f, 0.255497966f, 0.244134374f, 0.233011387f, 0.222116308f, 
		0.211437346f, 0.200963519f, 0.190684575f, 0.180590919f, 0.170673552f, 
		0.160924013f, 0.151334337f, 0.141897012f, 0.132604943f, 0.123451422f, 
		0.1144301f, 0.105534966f, 0.096760326f, 0.088100788f, 0.079551245f, 
		0.071106866f, 0.062763081f, 0.054515579f, 0.046360296f, 0.038293413f, 
		0.03031135f, 0.022410765f, 0.014588553f, 0.006841844f, -0.000831996f, 
		-0.008435358f, -0.015970396f, -0.023439012f, -0.030842857f, -0.038183325f, 
		-0.045461539f, -0.052678347f, -0.059834308f, -0.066929683f, -0.073964417f, 
		-0.080938127f, -0.08785008f, -0.094699178f, -0.101483934f, -0.108202445f, 
		-0.114852369f, -0.121430888f, -0.127934679f, -0.134359874f, -0.140702013f, 
		-0.146956001f, -0.153116046f, -0.159175602f, -0.165127301f, -0.170962864f, 
		-0.17667302f, -0.182247401f, -0.187674422f, -0.192941152f, -0.198033159f, 
		-0.202934338f, -0.207626714f}; 
	int channels = h_BRIRs->channels; 
	int Nh = h_BRIRs->timedomain_samples;
	int Nq = sizeof(q)/sizeof(float); 
	int Kq = (int)ceil(Nq/(QMFLIB_NUMBANDS*1.0f)); 
	int Nmid = Nq/2; 
	int Kh = (int)ceil(Nh/(QMFLIB_NUMBANDS*1.0f)); 
	float *hext = (float*)calloc(QMFLIB_NUMBANDS*(2*Kq+Kh-2), sizeof(float)); 
	int N = Kh + Kq - 1; 
	float **HQMF_real = (float**)calloc(N, sizeof(float*)); 
	float **HQMF_imag = (float**)calloc(N, sizeof(float*)); 
	float **E_real = (float**)calloc(QMFLIB_NUMBANDS, sizeof(float*)); 
	float **E_imag = (float**)calloc(QMFLIB_NUMBANDS, sizeof(float*)); 
	int ns, ls; 
	int shift; 
	float *factor = (float*)calloc(Nq, sizeof(float)); 
	float temp_real, temp_imag; 
	int band, chIn, chOut, ct, k, i;
	int max_BRIRlength = 0;

	max_BRIRlength = N;

	for (band = 0; band < maxband; band++)
	{
		E_real[band] = (float*)calloc(Nq, sizeof(float)); 
		E_imag[band] = (float*)calloc(Nq, sizeof(float)); 
	}

	for (k = 0; k < N; k++)
	{
		HQMF_real[k] = (float*)calloc(maxband, sizeof(float)); 
		HQMF_imag[k] = (float*)calloc(maxband, sizeof(float)); 
	}

	h_BRIRs->QMFdomain_left_real = (float***)calloc(max_BRIRlength, sizeof(float**)); 
	h_BRIRs->QMFdomain_left_imag = (float***)calloc(max_BRIRlength, sizeof(float**)); 
	h_BRIRs->QMFdomain_right_real = (float***)calloc(max_BRIRlength, sizeof(float**)); 
	h_BRIRs->QMFdomain_right_imag = (float***)calloc(max_BRIRlength, sizeof(float**)); 
	for (k = 0; k < max_BRIRlength; k++)
	{
		h_BRIRs->QMFdomain_left_real[k] = (float**)calloc(channels, sizeof(float*)); 
		h_BRIRs->QMFdomain_left_imag[k] = (float**)calloc(channels, sizeof(float*)); 
		h_BRIRs->QMFdomain_right_real[k] = (float**)calloc(channels, sizeof(float*)); 
		h_BRIRs->QMFdomain_right_imag[k] = (float**)calloc(channels, sizeof(float*)); 
		for (chIn = 0; chIn < channels; chIn++)
		{
			h_BRIRs->QMFdomain_left_real[k][chIn] = (float*)calloc(maxband, sizeof(float)); 
			h_BRIRs->QMFdomain_left_imag[k][chIn] = (float*)calloc(maxband, sizeof(float)); 
			h_BRIRs->QMFdomain_right_real[k][chIn] = (float*)calloc(maxband, sizeof(float)); 
			h_BRIRs->QMFdomain_right_imag[k][chIn] = (float*)calloc(maxband, sizeof(float)); 
		}
	}
	h_BRIRs->QMF_Timeslots = max_BRIRlength; 
	h_BRIRs->QMFbands = maxband; 

	ns = -1; 
	for (band = 0; band < maxband; band++)
	{
		ns++; 
		ls = -Nmid; 
		for (k = 0; k < Nq; k++)
		{
			ls++; 
			E_real[band][k] = (float)cos(PI/QMFLIB_NUMBANDS * (ns+0.5f)*ls); 
			E_imag[band][k] = (float)(-1*sin(PI/QMFLIB_NUMBANDS * (ns+0.5f)*ls)); 
		}
	}

	for (chOut = 0; chOut < BINAURALPARAM_NUM_OUTCHANNELS; chOut++)
	{
		for (chIn = 0; chIn < channels; chIn++)
		{
			ct = -1; 
			for (k = (Kq-1)*QMFLIB_NUMBANDS; k < (Kq-1)*QMFLIB_NUMBANDS+Nh; k++)
			{
				ct++; 
				if (chOut == 0)
				{
					hext[k] = h_BRIRs->timedomain_left[chIn][ct]; 
				}
				else
				{
					hext[k] = h_BRIRs->timedomain_right[chIn][ct]; 
				}
			}

			for (i = 0; i < N; i++)
			{
				shift = i*QMFLIB_NUMBANDS; 
				for (k = 0; k < Nq; k++)
				{
					factor[k] = hext[shift+k] * q[k]; 
				}
				for (band = 0; band < maxband; band++)
				{
					temp_real = 0; 
					temp_imag = 0; 
					for (k = 0; k < Nq; k++)
					{
						temp_real += E_real[band][k] * factor[k]; 
						temp_imag += E_imag[band][k] * factor[k]; 
					}
					HQMF_real[i][band] = temp_real; 
					HQMF_imag[i][band] = temp_imag; 
				}
				if (chOut == 0)
				{
					memcpy(h_BRIRs->QMFdomain_left_real[i][chIn], HQMF_real[i], maxband*sizeof(float)); 
					memcpy(h_BRIRs->QMFdomain_left_imag[i][chIn], HQMF_imag[i], maxband*sizeof(float)); 
				} 
				else
				{ 
					memcpy(h_BRIRs->QMFdomain_right_real[i][chIn], HQMF_real[i], maxband*sizeof(float)); 
					memcpy(h_BRIRs->QMFdomain_right_imag[i][chIn], HQMF_imag[i], maxband*sizeof(float)); 
				}
			}
		}
	}

	for (k = 0; k < N; k++)
	{
		free(HQMF_real[k]); HQMF_real[k] = NULL;
		free(HQMF_imag[k]); HQMF_imag[k] = NULL;
	}
	free(HQMF_real); HQMF_real = NULL;
	free(HQMF_imag); HQMF_imag = NULL;

	for (band = 0; band < maxband; band++)
	{
		free(E_real[band]); E_real[band] = NULL;
		free(E_imag[band]); E_imag[band] = NULL;
	}
	free(E_imag); E_imag = NULL;
	free(E_real); E_real = NULL;

	free(factor); factor = NULL;
	free(hext); hext = NULL; 

	return 0; 
}
void binauralParam_analyzeLR(H_BRIRANALYSISSTRUCT h_BrirSet, FdBinauralRendererParam *pFdBinauralRendererParam, int end_analysis)
{
	int i = 0;
	int temp = sizeof(B)/sizeof(float);

	int bandsperoctave = 3;
	int fmin = 90;

	int numbands_all = 2*(bandsperoctave*6)+1;
	int numbands = 0;
	float *fctemp = (float*)calloc(numbands_all, sizeof(float));
	float *fc;
	int ct = 0,ct2 = 0;
	int numcoeffs = 0;
	double *B_temp = NULL;
	double *A_temp = NULL;
	float *BRIR_temp = (float*)calloc(min(end_analysis,h_BrirSet->timedomain_samples),sizeof(float));
	int ch = 2;
	int pos = h_BrirSet->channels;
	int j, k, t, s;
	float *fc_QMF = (float*)calloc(QMFLIB_NUMBANDS,sizeof(float));
	int *startband = NULL;
	int *endband = NULL;
	int stop = 0;
	int minv_init = -100;
	int *minv = NULL;
	float noiselevel = 0;
	int endcalc = 0;
	float *RT60 = NULL;
	float *NRG = NULL;
	int ct_num = 0;
	float NRG_temp = 0.0f;
	float RT60_temp = 0.0f;

	float ****BRIRs_bandwise = NULL; /* [positions][ears][bands][samples] */

	fmin = (int)floor((fmin / 48000.0f*(h_BrirSet->fs_brirs*1.0f))+0.5);

	for (i= -bandsperoctave*6; i <= bandsperoctave*6; i++)
	{
		fctemp[ct] = (float)(pow(2.0f,((i*1.0f)/(bandsperoctave*1.0f)))*1000.0f) / 48000.0f*(h_BrirSet->fs_brirs*1.0f);
		if ((fctemp[ct] >= fmin) && (fctemp[ct] < h_BrirSet->fs_brirs/2.0f))
		{
			ct2++;
		}
		ct++;
	}
	numbands = ct2;

	fc = (float*)calloc(numbands+1,sizeof(float));
	ct2 = 1;
	for (i = 0; i < ct; i++)
	{
		if ((fctemp[i] >= fmin) && (fctemp[i] < h_BrirSet->fs_brirs/2.0f))
		{
			fc[ct2] = fctemp[i];
			ct2++;
		}
	}
	fc[0] = max(20.0f,(float)(fmin-30));

	numcoeffs = 7;
	B_temp = (double*)calloc(numcoeffs,sizeof(double));
	A_temp = (double*)calloc(numcoeffs,sizeof(double));

	stop = min(end_analysis,h_BrirSet->timedomain_samples);

	BRIRs_bandwise = (float****)calloc(pos, sizeof(float***));
	for (j = 0; j < pos; j++)
	{
		BRIRs_bandwise[j] = (float***)calloc(BINAURALPARAM_NUM_OUTCHANNELS,sizeof(float**));
		for (k = 0; k < BINAURALPARAM_NUM_OUTCHANNELS; k++)
		{
			BRIRs_bandwise[j][k] = (float**)calloc(numbands+1,sizeof(float*)); /* +1 because of additional LP filter */
			for (t = 0; t < numbands+1; t++)
			{
				BRIRs_bandwise[j][k][t] = (float*)calloc(stop,sizeof(float));
			}
		}
	}
	/* [positions][ears][bands][samples] */


	for (j = 0; j < pos; j++) 
	{
		for (k = 0; k < ch; k++)
		{
			/* read BRIR */
			for (t = 0; t < stop; t++)
			{
				if (k == 0)
				{
					BRIR_temp[t] = h_BrirSet->timedomain_left[j][t];
				}
				else
				{
					BRIR_temp[t] = h_BrirSet->timedomain_right[j][t];
				}
			}
			/* filter with bandwise filter coefficients, note that coefficients are defined for double precision */
			for (i = 0; i < numbands; i++)
			{
				int c;
				for (c = 0; c < numcoeffs; c++)
				{
					B_temp[c] = B[i][c];
					A_temp[c] = A[i][c];
				}
				binauralParam_applyfilter(B_temp, A_temp, BRIR_temp, numcoeffs, stop, BRIRs_bandwise, j, k, i+1);
			}
			/* additional LP filter */
			binauralParam_applyfilter(B_LP, A_LP, BRIR_temp,numcoeffs,stop, BRIRs_bandwise,j,k,0);
		}
	}
	/* compensate delay of 1/3rd octave filter bank */
	binauralParam_compensatedelay(BRIRs_bandwise, stop, pos, BINAURALPARAM_NUM_OUTCHANNELS, numbands);

	/* map transition to one-third octave bands */
	for (j = 0; j < QMFLIB_NUMBANDS; j++)
	{
		fc_QMF[j] = ((float)(h_BrirSet->fs_brirs/2.0f))/QMFLIB_NUMBANDS * (j+0.5f);
	}

	numbands = numbands + 1;
	startband = (int*)calloc(numbands, sizeof(int));
	endband = (int*)calloc(numbands, sizeof(int));
	minv = (int*)calloc(numbands, sizeof(int));

	for (j = 0; j < numbands; j++)
	{ 
		int min_diff_fc_idx = 0;
		float min_diff_fc = 0.0f;
		float *diff_fc = (float*)calloc(QMFLIB_NUMBANDS,sizeof(float));

		min_diff_fc = (float)fabs(fc[j] - fc_QMF[0]);

		for (k = 0; k < QMFLIB_NUMBANDS; k++)
		{
			diff_fc[k] = (float)fabs(fc[j] - fc_QMF[k]);
			if (diff_fc[k] <= min_diff_fc)
			{
				min_diff_fc = diff_fc[k];
				min_diff_fc_idx = k;
			}
		}
		/*startband[j] = h_BrirSet->transition[min(pFdBinauralRendererParam->kMax-1,min_diff_fc_idx)]+ h_BRIRmetadata->initDelay;*/
		startband[j] = h_BrirSet->transition[min(pFdBinauralRendererParam->kMax-1,min_diff_fc_idx)] + pFdBinauralRendererParam->dInit;

		startband[j] = min(startband[j],h_BrirSet->timedomain_samples);

		if (j == 0)
		{
			minv[j] = minv_init + 2;
			endband[j] = stop;
		}
		else if (j < numbands -3)
		{
			minv[j] = minv[j-1] + 2;
			endband[j] = stop;
		}
		else
		{
			minv[j] = -1;
			endband[j] =  stop - startband[j];
		}

		endband[j] = max(endband[j],startband[j]);


		free(diff_fc); diff_fc = NULL;
	}

	/* start analysis */
	NRG = (float*)calloc(numbands, sizeof(float));
	RT60 = (float*)calloc(numbands, sizeof(float));

	pFdBinauralRendererParam->fcAna = (float*)calloc(numbands, sizeof(float));
	pFdBinauralRendererParam->rt60 = (float*)calloc(numbands, sizeof(float));
	pFdBinauralRendererParam->nrgLr = (float*)calloc(numbands, sizeof(float));
	pFdBinauralRendererParam->kAna = numbands;

	ct_num = 0;

	fprintf(stderr, "\n>> Late reverberation analysis started...\n");

	for (j = 0; j < pos; j++)
	{
		float max_BRIR = 0.0f;
		int not_LFE = 1;
		for (s = 0; s < h_BrirSet->timedomain_samples; s++)
		{
			max_BRIR = max(max_BRIR,(float)fabs(h_BrirSet->timedomain_left[j][s]));
			max_BRIR = max(max_BRIR,(float)fabs(h_BrirSet->timedomain_right[j][s]));
		}
		fprintf(stderr, ">> ...analysing BRIR %d of %d\n", j+1, pos);

		if ((max_BRIR > 0.0f) && (h_BrirSet->LFE_IND[j] != 1))
		{
			for (k = 0; k < BINAURALPARAM_NUM_OUTCHANNELS; k++)
			{      
				for (i = 0; i < numbands; i++)
				{
					RT60_temp = 0.0f;
					NRG_temp = 0.0f;

					if (i >= 4)
					{
						/* estimate noisefloor */
						binauralParam_estimate_noisefloor(BRIRs_bandwise,startband,endband,j,i,k, &noiselevel, &endcalc, minv[i], h_BrirSet->fs_brirs);
					}
					else
					{
						endcalc = endband[i] - startband[i];
					}           

					if (endcalc > 0)
					{        
						/* RT60 and energy */
						binauralParam_getRT60(BRIRs_bandwise,startband,endcalc,j,i,k,h_BrirSet->fs_brirs,&RT60_temp);
						binauralParam_getNRG(BRIRs_bandwise,startband,endcalc,j,i,k,h_BrirSet->fs_brirs,&NRG_temp);
					}

					NRG[i] += NRG_temp;
					RT60[i] += RT60_temp; 
				}  
			}
			ct_num += BINAURALPARAM_NUM_OUTCHANNELS; 
		}    
	}

	for (i = 0; i < numbands; i++)
	{
		NRG[i]  = NRG[i] / ct_num;
		RT60[i] = RT60[i] / ct_num;

		pFdBinauralRendererParam->fcAna[i] = fc[i];
		pFdBinauralRendererParam->nrgLr[i] = NRG[i];
		pFdBinauralRendererParam->rt60[i] = RT60[i];
	}

	free(NRG); NRG = NULL;
	free(RT60); RT60 = NULL;
	free(fc_QMF); fc_QMF = NULL;
	free(B_temp); B_temp = NULL;
	free(A_temp); A_temp = NULL;
	free(BRIR_temp); BRIR_temp = NULL;
	free(fctemp); fctemp = NULL;
	free(fc); fc = NULL;
	free(startband); startband = NULL;
	free(minv); minv = NULL;

	for (j = 0; j < pos; j++)
	{
		for (k = 0; k < BINAURALPARAM_NUM_OUTCHANNELS; k++)
		{
			for (t = 0; t < numbands; t++)
			{
				free(BRIRs_bandwise[j][k][t]);  BRIRs_bandwise[j][k][t] = NULL;
			}
			free(BRIRs_bandwise[j][k]);  BRIRs_bandwise[j][k] = NULL;
		}
		free(BRIRs_bandwise[j]);  BRIRs_bandwise[j] = NULL;
	}
	free(BRIRs_bandwise); BRIRs_bandwise = NULL;

}

void binauralParam_compensatedelay(float ****BRIRs_bandwise, int length, int numpos, int numch, int numbands)
{
	/* [positions][ears][bands][samples] */

	int i = 0, j, k, m, delay;
	int maxdelay = 0;
	float *BRIR_temp = NULL;
	float *BRIR_comp = NULL;
	float *BRIR_comp2 = NULL;

	for (i = 0; i < numbands; i++)
	{
		maxdelay = max(maxdelay, Filter_Delay[i]);
	}
	BRIR_temp = (float*)calloc(maxdelay + length,sizeof(float));
	BRIR_comp = (float*)calloc(maxdelay + length,sizeof(float));


	for (m = 0; m < numpos; m++)
	{
		for (j = 0; j < numch; j++)
		{
			for (i = 1; i < numbands+1; i++)
			{
				delay = Filter_Delay[i-1];
				memset(BRIR_temp,0,maxdelay+length*sizeof(float));
				for (k = maxdelay; k < maxdelay + length; k++)
				{
					BRIR_temp[k] = BRIRs_bandwise[m][j][i][k-maxdelay];
				}
				memset(BRIRs_bandwise[m][j][i],0,length*sizeof(float));
				memset(BRIR_comp,0,(maxdelay + length)*sizeof(float));
				for (k = 0; k < maxdelay + length; k++)
				{
					if ( (k + delay) < (maxdelay + length))
					{
						BRIR_comp[k] = BRIR_temp[k+delay];
					}
				}
				for (k = 0; k < length; k++)
				{
					BRIRs_bandwise[m][j][i][k] = BRIR_comp[k+maxdelay];
				}
			}
		}
	}

	free(BRIR_temp); BRIR_temp = NULL;
	free(BRIR_comp); BRIR_comp = NULL;

}

void binauralParam_getNRG(float ****BRIRs_bandwise, int *startband, int endcalc, int pos, int numband, int channel, int fs, float *NRG)
{
	int ct = 0, i = 0;
	float energy = 0.0f;

	ct = (startband[numband]+endcalc) - (startband[numband]-1);

	for (i = 0; i < ct; i++)
	{
		energy += (float)(fabs(BRIRs_bandwise[pos][channel][numband][i+startband[numband]-1])*fabs(BRIRs_bandwise[pos][channel][numband][i+startband[numband]-1]));
	}

	*NRG = energy;
}

void binauralParam_getRT60(float ****BRIRs_bandwise, int *startband, int endcalc, int pos, int numband, int channel, int fs, float *RT60)
{
	float *BRIR_temp_norm = (float*)calloc(endcalc+1,sizeof(float));
	float *EDC = NULL;
	int ct = 0, i = 0, j = 0;
	float sum1 = 0.0f, sum2 = 0.0f;
	float max_dB = 0.0f;
	int i0 = -1, i5 = -1, i35 = -1;
	float fact = 2.0f;
	float amp = 0.0f;
	float maxBRIR = 0.0f;

	ct = (startband[numband]+endcalc) - (startband[numband]-1);
	EDC = (float*)calloc(ct,sizeof(float));

	for (i = 0; i < ct; i++)
	{
		maxBRIR = max(maxBRIR,(float)fabs(BRIRs_bandwise[pos][channel][numband][i+startband[numband]-1]));
	}

	for (i = 0; i < ct; i++)
	{
		BRIR_temp_norm[i] = (float)fabs(BRIRs_bandwise[pos][channel][numband][i+startband[numband]-1])/maxBRIR;
		if (BRIR_temp_norm[i] == 0)
		{
			BRIR_temp_norm[i] += (float)1.0E-20;
		}
		sum2 += BRIR_temp_norm[i]*BRIR_temp_norm[i];
	}
	EDC[0] = 0.0f;

	for (i = 1; i < ct; i++)
	{
		sum1 = 0.0f;
		for (j = i; j < ct; j++)
		{
			sum1 += BRIR_temp_norm[j]*BRIR_temp_norm[j];
		}
		EDC[i] = 10*(float)log10(sum1/sum2);
	}

	i0 = max(0,binauralParam_find_last(EDC,0.0f,4,0,ct));

	i5 = i0 + 1;
	while (EDC[i5] > (EDC[i0] -5))
	{
		i5++;
	}

	i35 = i5+1;
	while ((EDC[i35] > (EDC[i0] - 35)) && (i35 < ct))
	{
		i35++;
	}

	if (i35 > floor(ct * 0.95))
	{
		i35 = (int)floor(ct * 0.95 + 0.5)-1;
		amp = EDC[i35];
		fact = -65.0f / amp;
	}
	else
	{
		fact = 2;
	}

	if (i5 > floor(ct * 0.5))
	{
		i5 = 0;
	}

	*RT60 = ((i35*((ct*1.0f)/(float)(fs*ct))) - (i5*((ct*1.0f)/(float)(fs*ct))))*fact;

	free(BRIR_temp_norm); BRIR_temp_norm = NULL;
	free(EDC); EDC = NULL;
}

void binauralParam_estimate_noisefloor(float ****BRIRs_bandwise, int *startband, int *endband, int pos, int numband, int channel, float *noiselevel, int *endcalc, int minv, int fs)
{
	float *BRIR_temp_norm = (float*)calloc(endband[numband],sizeof(float));
	int i;
	int percent = 80;
	int ct = 0;
	float size_mean = 0.025f;
	int l = 0, hopsize = 0;
	int start = 0, ct2 = 0;
	int max_num_mean = 0;
	float *smoothIR = NULL;
	int estimate_end_idx = 0;
	float add = (float)1.0E-20;
	float maxBRIR = 0.0f;
	int estimate_start_idx = 0;
	int length_estimate  = 0;

	float maxvalue = 0;
	int maxindex = 0;
	float threshold = 0.0f;
	int startidx = 0; 
	int stopidx = 0;
	int endpt = -1;
	float noise = 0.0f;
	float startdecay = 0;

	int length = 0;
	float m, b;
	int start_t, stop_t;
	float *decay = NULL;
	float *noisefloor = NULL;


	for (i = startband[numband]-1; i < endband[numband]; i++)
	{
		maxBRIR = max(maxBRIR,(float)fabs(BRIRs_bandwise[pos][channel][numband][i]));
		ct++;
	}
	for (i = 0; i < ct; i++)
	{
		BRIR_temp_norm[i] = 10*(float)log10(pow(((BRIRs_bandwise[pos][channel][numband][i+startband[numband]-1] + add)/maxBRIR),2.0f));
	}

	l = (int)floor(size_mean * fs + 0.5);
	hopsize = (int)floor(l/2 + 0.5);
	max_num_mean = (ct/l)*2;
	smoothIR = (float*)calloc(max_num_mean,sizeof(float));


	while (1)
	{
		if (start + l -1 >= ct)
		{
			break;
		}

		smoothIR[ct2] = binauralParam_mean(BRIR_temp_norm,start,l);
		start = start + hopsize;
		ct2++;
	}
	if (minv == -1)
	{
		minv = (int)floor(binauralParam_mean(smoothIR,0,ct2)+0.5);
	}
	estimate_end_idx = binauralParam_find_last(smoothIR,(float)minv,2,0,ct2);
	if (estimate_end_idx == -1)
	{
		*noiselevel = binauralParam_mean(smoothIR,0,ct2);
	}
	else
	{
		estimate_start_idx = (int)floor((((float)percent)/100.0f)*estimate_end_idx + 0.5);
		length_estimate = estimate_end_idx - estimate_start_idx +1;
		*noiselevel = binauralParam_mean(smoothIR,estimate_start_idx,length_estimate);
	}

	/* calculate decay */

	binauralParam_findmax(smoothIR,ct2,&maxvalue,&maxindex);
	startidx = maxindex;
	threshold = *noiselevel + 2;
	stopidx = binauralParam_find_first(smoothIR,threshold,1,startidx,ct2);
	if (stopidx == -1)
	{
		endpt = 0;
		noise = binauralParam_mean(smoothIR,0,ct2);
	}
	else
	{
		decay = (float*)calloc(ct,sizeof(float));
		noisefloor = (float*)calloc(ct, sizeof(float));

		if (stopidx == startidx)
		{
			stopidx++;
		}
		binauralParam_findmax(BRIR_temp_norm,ct,&maxvalue,&maxindex);
		length = maxindex + 10 - max(0,maxindex-10) + 1;
		startdecay = binauralParam_mean(BRIR_temp_norm,max(0,maxindex-10),length);
		startdecay = (startdecay + maxvalue) / 2.0f;

		start_t = (startidx+1) * hopsize;
		stop_t = (stopidx+1) * hopsize;

		m = (threshold - startdecay)/(float)((int)floor(stop_t + 0.5) - (int)floor(start_t + 0.5));
		b = startdecay - m*(float)floor(start_t + 0.5);
		for (i = 0; i < ct; i++)
		{
			decay[i] = (i+1)*m + b;
			noisefloor[i] = *noiselevel;
			if (decay[i] < noisefloor[i])
			{
				endpt = i;
				break;
			}
		}
		if (endpt == -1)
		{
			endpt = ct;
		}

		*endcalc = endpt;

		free(decay); decay = NULL;
		free(noisefloor); noisefloor = NULL;
	}

	free(BRIR_temp_norm); BRIR_temp_norm = NULL;
}

void binauralParam_findmax(float *data, int l, float *max, int *idx)
{
	int i,idx_temp;
	float max_temp = data[0];
	int max_found = 0;

	idx_temp = 0;

	for (i=0;i<l;i++)
	{
		if (data[i] > max_temp)
		{
			max_temp = data[i];
			idx_temp = i;
			max_found = 1;
		}
	}
	if (max_found == 0)
	{
		*max = max_temp;
		*idx = 0;
	}
	else
	{
		*max = max_temp;
		*idx = idx_temp;
	}
}

void binauralParam_applyfilter(double *B, double *A, float *signal, int filterlength, int signallength, float ****signal_bandwise, int position, int channel, int numband)
{
	int j, k;
	double y;
	double *buffer = (double*)calloc(filterlength, sizeof(double));
	double *paddedsig = (double*)calloc(signallength+2*filterlength,sizeof(double));
	float *filteredsig = (float*)calloc(signallength,sizeof(float));
	int ct = 0;

	for (j = filterlength-1; j < signallength+filterlength-1; j++)
	{
		paddedsig[j] = (double)signal[j-filterlength+1];
	}

	for(j = filterlength-1; j < signallength+filterlength-1; j++)
	{
		double temp = 0;

		y = 0;
		for (k = 0; k < filterlength; k++)
		{
			temp += B[k] * paddedsig[j-k];
		}
		y += temp;

		temp = 0;
		for (k = 1; k < filterlength; k++)
		{
			temp -= A[k] * buffer[k-1];
		}
		y += temp;

		for(k=filterlength-1; k>0; k--)
		{
			buffer[k] = buffer[k-1];
		}
		buffer[0] = y;

		filteredsig[ct] = (float)y;
		ct++;
	}


	/* copy to output array */
	for (j = 0; j < signallength; j++)
	{
		signal_bandwise[position][channel][numband][j] = filteredsig[j];  /*[positions][ears][bands][samples] */
	}

	free(buffer); buffer = NULL;
	free(filteredsig); filteredsig = NULL;
	free(paddedsig); paddedsig = NULL;

}

float binauralParam_mean(float *data, int start, int l)
{
	int i;
	float sum = 0.f;
	for (i=start;i<start+l;i++)
	{
		sum = sum + data[i];
	}
	sum = sum / l;
	return sum;
}

int binauralParam_find_last(float *data, float test, int modus, int start, int l_data)
{
	int foundlast = -1;
	int i;
	switch (modus)
	{
	case 0: /*bigger*/
		for (i=start;i<l_data;i++)
		{
			if (data[i] > test)
			{
				foundlast = i;
			}
		}
		break;
	case 1: /*smaller */
		for (i=start;i<l_data;i++)
		{
			if (data[i] < test)
			{
				foundlast = i;
			}
		}
		break;
	case 2: /* bigger or equal */
		for (i=start;i<l_data;i++)
		{
			if (data[i] >= test)
			{
				foundlast = i;
			}
		}
		break;
	case 3: /*smaller or equal */
		for (i=start;i<l_data;i++)
		{
			if (data[i] <= test)
			{
				foundlast = i;
			}
		}
		break;
	case 4: /* equal */
		for (i=start;i<l_data;i++)
		{
			if (data[i] == test)
			{
				foundlast = i;
			}
		}
		break;
	}
	return foundlast;
}

int binauralParam_find_first(float *data, float test, int modus, int start, int l_data)
{
	int foundfirst = -1;
	int i;
	switch (modus)
	{
	case 0: /*bigger*/
		for (i=start;i<l_data;i++)
		{
			if (data[i] > test)
			{
				foundfirst = i;
				break;
			}
		}
		break;
	case 1: /*smaller */
		for (i=start;i<l_data;i++)
		{
			if (data[i] < test)
			{
				foundfirst = i;
				break;
			}
		}
		break;
	case 2: /* bigger or equal */
		for (i=start;i<l_data;i++)
		{
			if (data[i] >= test)
			{
				foundfirst = i;
				break;
			}
		}
		break;
	case 3: /*smaller or equal */
		for (i=start;i<l_data;i++)
		{
			if (data[i] <= test)
			{
				foundfirst = i;
				break;
			}
		}
		break;
	case 4: /* equal */
		for (i=start;i<l_data;i++)
		{
			if (data[i] == test)
			{
				foundfirst = i;
				break;
			}
		}
		break;
	}
	return foundfirst;
}

void binauralParam_linearfit(FdBinauralRendererParam *pFdBinauralRendererParam, int x_begin, int x_end, float* MixingTime)
{
	float n, x_bar, y_bar, ss_xx, ss_yy, ss_xy;
	float b, a;
	float logMixingTime;
	int k;
	n = (float)(x_end - x_begin + 1);
	x_bar = (float)(x_end + x_begin)/2;

	y_bar = 0; ss_yy = 0; ss_xy = 0;
	for (k = x_begin; k<=x_end; k++)
	{
		logMixingTime = (float)(log(MixingTime[k-1])/log(2.0f));
		ss_xy += k * logMixingTime;
		y_bar += logMixingTime;
	}
	y_bar = y_bar / n;

	ss_xx = x_end*(x_end+1)*(2*x_end+1)/6 - (x_begin-1)*(x_begin)*(2*x_begin-1)/6 - n*x_bar*x_bar;
	ss_xy -= n*x_bar*y_bar;

	b = ss_xy / ss_xx;
	a = y_bar - b * x_bar;

	for (k = x_begin; k<=x_end; k++)
	{
		pFdBinauralRendererParam->nFilter[k-1] = (int) pow(2.0f, (float)((int)(b*(float)k + a +.5f)));
	}
}

static __inline int IsLittleEndian (void)
{
	short s = 0x01 ; 

	return *((char *)&s)? 1 : 0; 
}


static __inline short LittleEndian16 (short v)
{ /* get signed little endian (2-compl.)and returns in native format, signed */
	if (IsLittleEndian ())return v ; 

	else return ((v << 8)& 0xFF00)| ((v >> 8)& 0x00FF); 
}

float ReverseFloat(const float inFloat )
{
	float retVal; 
	char *floatToConvert = (char* )& inFloat; 
	char *returnFloat = (char* )& retVal; 

	/* swap the bytes into a temporary buffer */
	returnFloat[0] = floatToConvert[3]; 
	returnFloat[1] = floatToConvert[2]; 
	returnFloat[2] = floatToConvert[1]; 
	returnFloat[3] = floatToConvert[0]; 

	return retVal; 
}


static unsigned int BigEndian32 (char a, char b, char c, char d)
{
	if (IsLittleEndian ())
	{
		return (unsigned int)d << 24 |
			(unsigned int)c << 16 |
			(unsigned int)b <<  8 |
			(unsigned int)a ; 
	}
	else
	{
		return (unsigned int)a << 24 |
			(unsigned int)b << 16 |
			(unsigned int)c <<  8 |
			(unsigned int)d ; 
	}
}

int binauralParam_createFdBinauralParam(BinauralRepresentation *pBinauralRepresentation, int framesize)
{
	BRIRANALYSISSTRUCT BrirSet;
	H_BRIRANALYSISSTRUCT h_BrirSet = &BrirSet;

	int k = 0, l = 0;

	int error_brir = 0;
	unsigned int band = 0;

	int end_analysis = 0;

	int minlength_LRanalysis = 0;
	int min_BRIR_QMF_length = 0;
	int max_filterlength = 0;

	int fs = 0;
	int f_center_max = 0;
	int f_max = 0;

	int *DE_filter_length = NULL;
	int max_transition = 0;

	int *LFE_IND = NULL;
	float ***ptr_timedomain = NULL;

	CICP2GEOMETRY_CHANNEL_GEOMETRY geoInfo[CICP2GEOMETRY_MAX_LOUDSPEAKERS] = { 0 };
	FdBinauralRendererParam *mpFdBinauralRendererParam;

#ifdef BUGFIX_KCONV_KMAX_15JUNE
	int NUM_BAND_CONV = pBinauralRepresentation->pFdBinauralRendererParam->kConv;
    int NUM_BAND_PROC = pBinauralRepresentation->pFdBinauralRendererParam->kMax;
#endif


	/* ******************** DirectSound + Early Reflections (D+E) Parameters *********************** */
	HANDLE_FFTLIB h_FFT = NULL; 

	/* **************************** INITIALIZATION ******************** */

	BinauralFirData *mpBinauralFirData = pBinauralRepresentation->pBinauralFirData;

#ifndef BUGFIX_KCONV_KMAX_15JUNE
	pBinauralRepresentation->pFdBinauralRendererParam = (FdBinauralRendererParam*)calloc(1, sizeof(FdBinauralRendererParam));
#endif
	mpFdBinauralRendererParam = pBinauralRepresentation->pFdBinauralRendererParam;
	
	/* read BRIRs, write geometric information */
	h_BrirSet->timedomain_left = NULL;
	h_BrirSet->timedomain_right = NULL;


	h_BrirSet->channels = (int) pBinauralRepresentation->nBrirPairs;
	h_BrirSet->fs_brirs = pBinauralRepresentation->brirSamplingFrequency;
	h_BrirSet->timedomain_samples = mpBinauralFirData->Ntaps;
	h_BrirSet->timedomain_left = (float**) calloc(h_BrirSet->channels, sizeof(float*));
	h_BrirSet->timedomain_right = (float**) calloc(h_BrirSet->channels, sizeof(float*));
	h_BrirSet->LFE_IND = (int*)calloc(h_BrirSet->channels, sizeof(int));
	h_BrirSet->numLFEs = pBinauralRepresentation->Setup_SpeakerConfig3d.numLFEs;
	mpFdBinauralRendererParam->kMax = NUM_BAND_PROC;
	mpFdBinauralRendererParam->kConv = NUM_BAND_CONV;

	for( k=0; k<h_BrirSet->channels; k++)
	{
		h_BrirSet->timedomain_left[k] = (float*) calloc(h_BrirSet->timedomain_samples, sizeof(float));
		h_BrirSet->timedomain_right[k] = (float*) calloc(h_BrirSet->timedomain_samples, sizeof(float));
		h_BrirSet->LFE_IND[k] = pBinauralRepresentation->Setup_SpeakerConfig3d.pGeometry[k].LFE;
		memcpy(h_BrirSet->timedomain_left[k], mpBinauralFirData->pppTaps[0][k], h_BrirSet->timedomain_samples*sizeof(float));
		memcpy(h_BrirSet->timedomain_right[k], mpBinauralFirData->pppTaps[1][k], h_BrirSet->timedomain_samples*sizeof(float));
	}

	ptr_timedomain = (float***)calloc(2, sizeof(float**)); /* left , right */
	ptr_timedomain[0] = (float**)calloc(h_BrirSet->channels , sizeof(float*));
	ptr_timedomain[1] = (float**)calloc(h_BrirSet->channels , sizeof(float*));

	binauralParam_PropagationTimeCalc(h_BrirSet, mpFdBinauralRendererParam, ptr_timedomain);
	binauralParam_FilterConversion(h_BrirSet, mpFdBinauralRendererParam);
	binauralParam_VoffParamGeneration(h_BrirSet, mpFdBinauralRendererParam, framesize);
	binauralParam_QtdlParamGeneration(h_BrirSet, mpFdBinauralRendererParam);

	for( k=0; k<mpFdBinauralRendererParam->kMax; k++) /* log */
	{
		mpFdBinauralRendererParam->nFft[k] = log10((float)mpFdBinauralRendererParam->nFft[k]) / log10(2.0f);
	}
	
	/* ****************************** LATE REVERB ANALYSIS ******************************** */
	if (mpFdBinauralRendererParam->flagHRIR == 0)
	{
		for (band = 0; band < mpFdBinauralRendererParam->kMax; band++)
		{
			max_transition = max(max_transition,h_BrirSet->transition[band]);
		}
		mpFdBinauralRendererParam->fcAna = NULL;
		mpFdBinauralRendererParam->rt60 = NULL;
		mpFdBinauralRendererParam->nrgLr = NULL;

		end_analysis = (int)pow(2.0f,floor(log(h_BrirSet->timedomain_samples/4.0f)/log(2.0f))+1);

		if (end_analysis < max_transition)
		{
			end_analysis = max_transition;
		}

		binauralParam_analyzeLR(h_BrirSet, mpFdBinauralRendererParam, end_analysis);
	}
	else
	{
		mpFdBinauralRendererParam->kAna = 0;
	}
	/* ****************************** FREE MEMORY ******************************** */

	for( k=0; k< h_BrirSet->QMF_Timeslots; k++)
	{
		for ( l=0; l<h_BrirSet->channels ; l++ )
		{
			free(h_BrirSet->QMFdomain_left_real[k][l]);
			free(h_BrirSet->QMFdomain_left_imag[k][l]);
			free(h_BrirSet->QMFdomain_right_real[k][l]);
			free(h_BrirSet->QMFdomain_right_imag[k][l]);
		}
		free(h_BrirSet->QMFdomain_left_real[k]);
		free(h_BrirSet->QMFdomain_left_imag[k]);
		free(h_BrirSet->QMFdomain_right_real[k]);
		free(h_BrirSet->QMFdomain_right_imag[k]);
	}
	free(h_BrirSet->QMFdomain_left_real);
	free(h_BrirSet->QMFdomain_left_imag);
	free(h_BrirSet->QMFdomain_right_real);
	free(h_BrirSet->QMFdomain_right_imag);

	h_BrirSet->QMFdomain_left_real = NULL;
	h_BrirSet->QMFdomain_left_imag = NULL;
	h_BrirSet->QMFdomain_right_imag = NULL;
	h_BrirSet->QMFdomain_right_imag = NULL;

	if (h_BrirSet->timedomain_left != NULL)
	{
		for (k = 0; k < h_BrirSet->channels; k++)
		{
			h_BrirSet->timedomain_left[k] = NULL;	
			h_BrirSet->timedomain_right[k] = NULL;				
			free(ptr_timedomain[0][k]);	
			free(ptr_timedomain[1][k]);	
			ptr_timedomain[0][k] = NULL;
			ptr_timedomain[1][k] = NULL;
		}
		ptr_timedomain[0] = NULL; ptr_timedomain[1] = NULL; 
		free(h_BrirSet->timedomain_left); h_BrirSet->timedomain_left = NULL;
	}

	free(h_BrirSet->LFE_IND); h_BrirSet->LFE_IND = NULL;
	free(h_BrirSet->transition); h_BrirSet->transition = NULL;

	return 0;
}
void binauralParam_PropagationTimeCalc(BRIRANALYSISSTRUCT *h_BrirSet, FdBinauralRendererParam *pFdBinauralRendererParam, float ***ptr_timedomain)
{
	int LFrm = 0;
	int Nhop = 0;

	int earlyTime_samples = 0;
	int Niter = 0;

	float *Energy_Brir = NULL;
	float max_Energy_Brir = 0;
	int idx_Energy_Brir = 0;

	int l, m, n;

	/* ****************************** Propagation (initDelay) Estimation ******************************** */
	earlyTime_samples = (int)(EARLYTIME*h_BrirSet->fs_brirs);
	if (h_BrirSet->timedomain_samples >= earlyTime_samples)
	{
		LFrm = 32;
		Nhop = 8;
		Niter = (int)(earlyTime_samples/Nhop);
		pFdBinauralRendererParam->flagHRIR = 0;
	}
	else
	{
		LFrm = 8;
		Nhop = 1;
		Niter = (int)((h_BrirSet->timedomain_samples-LFrm)/Nhop);
		pFdBinauralRendererParam->flagHRIR = 1;
	}

	Energy_Brir = (float*)calloc(Niter, sizeof(float));

	for(l=0; l< Niter; l++)
	{
    int tmp;
		Energy_Brir[l] = 0;
		for( m=0; m < h_BrirSet->channels ; m++)
		{
			if( h_BrirSet->LFE_IND[m] != 1 )
			{
				Energy_Brir[l] +=   h_BrirSet->timedomain_left[m][l*Nhop] * h_BrirSet->timedomain_left[m][l*Nhop] 
				+ h_BrirSet->timedomain_right[m][l*Nhop] * h_BrirSet->timedomain_right[m][l*Nhop];
				for ( n=1; n<LFrm; n++ )
				{
					Energy_Brir[l] +=	h_BrirSet->timedomain_left[m][l*Nhop+n] * h_BrirSet->timedomain_left[m][l*Nhop+n] 
					+ h_BrirSet->timedomain_right[m][l*Nhop+n] * h_BrirSet->timedomain_right[m][l*Nhop+n];
				}
        tmp = 1;
			}
		}
	}
	for(l=0; l< Niter; l++)
	{
		if( Energy_Brir[l] > max_Energy_Brir)
		{
			max_Energy_Brir = Energy_Brir[l];
			idx_Energy_Brir = l+1;
		}
	}
	l = 0;
	while( Energy_Brir[l] / max_Energy_Brir < 1e-6 )
	{
		l++;
	}
	pFdBinauralRendererParam->dInit = LFrm/2+Nhop*(l+1);
	h_BrirSet->timedomain_samples -= pFdBinauralRendererParam->dInit;

	for(l=0; l<h_BrirSet->channels; l++)
	{
		if( h_BrirSet->LFE_IND[l] != -1)
		{
			ptr_timedomain[0][l] = (float*) &(h_BrirSet->timedomain_left[l][0]);
			ptr_timedomain[1][l] = (float*) &(h_BrirSet->timedomain_right[l][0]);
			(h_BrirSet->timedomain_left[l]) = &(h_BrirSet->timedomain_left[l][pFdBinauralRendererParam->dInit]);
			(h_BrirSet->timedomain_right[l]) = &(h_BrirSet->timedomain_right[l][pFdBinauralRendererParam->dInit]);
		}
	}

	free(Energy_Brir); Energy_Brir = NULL;
}
int binauralParam_FilterConversion(BRIRANALYSISSTRUCT *h_BrirSet, FdBinauralRendererParam *pFdBinauralRendererParam)
{
	int min_BRIR_QMF_length;
	int error_brir = 0;
#ifdef BUGFIX_KCONV_KMAX_15JUNE
	int NUM_BAND_PROC = pFdBinauralRendererParam->kMax;
#endif
	/* ****************************** QMF TRANSFORM OF BRIRS ******************************** */
	min_BRIR_QMF_length = MAX_QMF_FILTERLENGTH + (int)(floor((pFdBinauralRendererParam->dInit/QMFLIB_NUMBANDS) + 0.5f)+1); /* What is this? */

	h_BrirSet->QMFdomain_left_imag = NULL;
	h_BrirSet->QMFdomain_left_real = NULL;
	h_BrirSet->QMFdomain_right_imag = NULL;
	h_BrirSet->QMFdomain_right_real = NULL;
	error_brir = binauralParam_BRIRtoQMF(h_BrirSet, NUM_BAND_PROC, min_BRIR_QMF_length); /* BRIR -> QMF */

	return error_brir;
}
void binauralParam_VoffParamGeneration(BRIRANALYSISSTRUCT *h_BrirSet, FdBinauralRendererParam *pFdBinauralRendererParam, int framesize)
{

	int l, m, n, k;
	unsigned int band;
	float *MixingTime = NULL;
	float THR = 0;

	int total_max_fft = framesize / QMFLIB_NUMBANDS * 2;
	int max_numblocks = 1; 
	int max_numframes = 1;
	int chOut = 0, chIn = 0; 

	float ***EDR_L_real, ***EDR_L_imag, ***EDR_R_real, ***EDR_R_imag;
	HANDLE_FFTLIB h_FFT = NULL; 

#ifdef BUGFIX_KCONV_KMAX_15JUNE
	int NUM_BAND_PROC = pFdBinauralRendererParam->kMax;
#endif
	/* ****************************** Filter Order Decision ******************************** */

	/* Energy Decay Relief Calculation */
	EDR_L_real = (float***)calloc(h_BrirSet->QMFbands, sizeof(float**));
	EDR_L_imag = (float***)calloc(h_BrirSet->QMFbands, sizeof(float**));
	EDR_R_real = (float***)calloc(h_BrirSet->QMFbands, sizeof(float**));
	EDR_R_imag = (float***)calloc(h_BrirSet->QMFbands, sizeof(float**));
	for ( l=0; l<h_BrirSet->QMFbands; l++)
	{
		EDR_L_real[l] = (float**)calloc(h_BrirSet->channels, sizeof(float*));
		EDR_L_imag[l] = (float**)calloc(h_BrirSet->channels, sizeof(float*));
		EDR_R_real[l] = (float**)calloc(h_BrirSet->channels, sizeof(float*));
		EDR_R_imag[l] = (float**)calloc(h_BrirSet->channels, sizeof(float*));
		for( m=0; m<h_BrirSet->channels ; m++ )
		{
			if( h_BrirSet->LFE_IND[m] != -1)
			{
				EDR_L_real[l][m] = (float*)calloc(h_BrirSet->QMF_Timeslots+1, sizeof(float));
				EDR_L_imag[l][m] = (float*)calloc(h_BrirSet->QMF_Timeslots+1, sizeof(float));
				EDR_R_real[l][m] = (float*)calloc(h_BrirSet->QMF_Timeslots+1, sizeof(float));
				EDR_R_imag[l][m] = (float*)calloc(h_BrirSet->QMF_Timeslots+1, sizeof(float));
			}
		}	
	}
	for ( l=0; l<h_BrirSet->QMFbands; l++)
	{
		for ( m=0; m<h_BrirSet->channels ; m++ )
		{
			if( h_BrirSet->LFE_IND[m] != -1)
			{
				n = h_BrirSet->QMF_Timeslots-1;
				EDR_L_real[l][m][n] = h_BrirSet->QMFdomain_left_real[n][m][l] * h_BrirSet->QMFdomain_left_real[n][m][l];
				EDR_L_imag[l][m][n] = h_BrirSet->QMFdomain_left_imag[n][m][l] * h_BrirSet->QMFdomain_left_imag[n][m][l];
				EDR_R_real[l][m][n] = h_BrirSet->QMFdomain_right_real[n][m][l] * h_BrirSet->QMFdomain_right_real[n][m][l];
				EDR_R_imag[l][m][n]	= h_BrirSet->QMFdomain_right_imag[n][m][l] * h_BrirSet->QMFdomain_right_imag[n][m][l];
				for ( n=h_BrirSet->QMF_Timeslots-2; n>=0; n--)
				{
					EDR_L_real[l][m][n] = EDR_L_real[l][m][n+1] + h_BrirSet->QMFdomain_left_real[n][m][l] * h_BrirSet->QMFdomain_left_real[n][m][l];
					EDR_L_imag[l][m][n] = EDR_L_imag[l][m][n+1] + h_BrirSet->QMFdomain_left_imag[n][m][l] * h_BrirSet->QMFdomain_left_imag[n][m][l];
					EDR_R_real[l][m][n] = EDR_R_real[l][m][n+1] + h_BrirSet->QMFdomain_right_real[n][m][l] * h_BrirSet->QMFdomain_right_real[n][m][l];
					EDR_R_imag[l][m][n] = EDR_R_imag[l][m][n+1] + h_BrirSet->QMFdomain_right_imag[n][m][l] * h_BrirSet->QMFdomain_right_imag[n][m][l];

				}
			}
		}
	}

	MixingTime = (float*)calloc(h_BrirSet->QMFbands, sizeof(float));
	for ( l=0; l<h_BrirSet->QMFbands; l++)
	{
		MixingTime[l] = 0;
		for ( m=0; m<h_BrirSet->channels ; m++ )
		{
			if( h_BrirSet->LFE_IND[m] != -1)
			{
				THR = (EDR_L_real[l][m][0] + EDR_L_imag[l][m][0]) * (float) pow((float)10, (float)(THR_DE/10));
				n = 1;
				while( EDR_L_real[l][m][n] + EDR_L_imag[l][m][n] > THR )
				{
					n++;
				}
				MixingTime[l] += (n+1);
			}
		}
		for ( m=0; m<h_BrirSet->channels ; m++ )
		{
			if( h_BrirSet->LFE_IND[m] != -1)
			{
				THR = (EDR_R_real[l][m][0] + EDR_R_imag[l][m][0]) * (float) pow((float)10, (float)(THR_DE/10));
				n = 1;	
				while( EDR_R_real[l][m][n] + EDR_R_imag[l][m][n] > THR )
				{
					n++;
				}
				MixingTime[l] += (n+1);
			}
		}

#ifdef BUGFIX_BINAURALPARAM_DIVISION
		MixingTime[l] = MixingTime[l]/(2*(h_BrirSet->channels - h_BrirSet->numLFEs));
#else
		MixingTime[l] = MixingTime[l]/(h_BrirSet->channels- h_BrirSet->numLFEs + 28 );
#endif	
	}

	pFdBinauralRendererParam->nFilter = (int*)calloc(NUM_BAND_PROC,sizeof(int));
	pFdBinauralRendererParam->nFft = (int*)calloc(NUM_BAND_PROC,sizeof(int));

	h_BrirSet->transition = NULL;
	h_BrirSet->transition = (int*)calloc(NUM_BAND_PROC,sizeof(int)); 

	if(!pFdBinauralRendererParam->flagHRIR)
	{
		pFdBinauralRendererParam->nFilter[BAND_CRITICAL] = (int) min(pow(2, (double)(ceil(log(MixingTime[BAND_CRITICAL])/log(2.0f)))), (double)h_BrirSet->QMF_Timeslots);
		binauralParam_linearfit(pFdBinauralRendererParam, BAND_CRITICAL+2, NUM_BAND_PROC, MixingTime);		
	}
	else
	{
		pFdBinauralRendererParam->nFilter[BAND_CRITICAL] =  (int) min(pow(2, (double)(ceil(log(MixingTime[BAND_CRITICAL])/log(2.0f)))), (double)h_BrirSet->QMF_Timeslots);
		for( l=BAND_CRITICAL+1; l<h_BrirSet->QMFbands; l++)
		{
			pFdBinauralRendererParam->nFilter[l] = (int) (min( pow(2, (double)((int)(log(MixingTime[l])/log(2.f) + .5f))),  (double)(h_BrirSet->QMF_Timeslots)));
		}
	}

	pFdBinauralRendererParam->nFft = (int*)calloc(NUM_BAND_PROC, sizeof(int));
	pFdBinauralRendererParam->nBlk = (int*)calloc(NUM_BAND_PROC, sizeof(int));

	for (band = 0; band < pFdBinauralRendererParam->kMax; band++)
	{
		pFdBinauralRendererParam->nFft[band] = (int) min(pow(2.f, ceil(log((float)(2*pFdBinauralRendererParam->nFilter[band])/log(2.f))+1)), total_max_fft);
		pFdBinauralRendererParam->nBlk[band] = (int) ceil((float)(pFdBinauralRendererParam->nFilter[band])/(float)(pFdBinauralRendererParam->nFft[band]/2));
		max_numblocks = max(max_numblocks,pFdBinauralRendererParam->nBlk[band]);

    h_BrirSet->transition[band] = pFdBinauralRendererParam->nFft[band]* (framesize/64);
	}

	/* ****************************** FFT TRANSFORM OF D+E ******************************** */

	/* FFT transform of early reflection BRIRs */
	/* h_BrirSet->FFTdomain_left/right_real/imag (4D Buffer [bands x channels x max_num_blocks x 128]) for FFT domain BRIRs */
	pFdBinauralRendererParam->ppppVoffCoeffLeftReal  = (float****)calloc(NUM_BAND_PROC,sizeof(float***));
	pFdBinauralRendererParam->ppppVoffCoeffLeftImag  = (float****)calloc(NUM_BAND_PROC,sizeof(float***));
	pFdBinauralRendererParam->ppppVoffCoeffRightReal = (float****)calloc(NUM_BAND_PROC,sizeof(float***));
	pFdBinauralRendererParam->ppppVoffCoeffRightImag = (float****)calloc(NUM_BAND_PROC,sizeof(float***));

	for (band = 0; band < pFdBinauralRendererParam->kMax; band++)
	{
		pFdBinauralRendererParam->ppppVoffCoeffLeftReal[band] = (float***)calloc(max_numblocks,sizeof(float**));
		pFdBinauralRendererParam->ppppVoffCoeffLeftImag[band] = (float***)calloc(max_numblocks,sizeof(float**));
		pFdBinauralRendererParam->ppppVoffCoeffRightReal[band] = (float***)calloc(max_numblocks,sizeof(float**));
		pFdBinauralRendererParam->ppppVoffCoeffRightImag[band] = (float***)calloc(max_numblocks,sizeof(float**));
		for (k=0; k< pFdBinauralRendererParam->nBlk[band]; k++) 
		{
			pFdBinauralRendererParam->ppppVoffCoeffLeftReal[band][k] = (float**)calloc(h_BrirSet->channels,sizeof(float*));
			pFdBinauralRendererParam->ppppVoffCoeffLeftImag[band][k] = (float**)calloc(h_BrirSet->channels,sizeof(float*));
			pFdBinauralRendererParam->ppppVoffCoeffRightReal[band][k] = (float**)calloc(h_BrirSet->channels,sizeof(float*));
			pFdBinauralRendererParam->ppppVoffCoeffRightImag[band][k] = (float**)calloc(h_BrirSet->channels,sizeof(float*));
			for (chIn = 0; chIn < h_BrirSet->channels; chIn++)
			{
				pFdBinauralRendererParam->ppppVoffCoeffLeftReal[band][k][chIn] = (float*)calloc(total_max_fft,sizeof(float));
				pFdBinauralRendererParam->ppppVoffCoeffLeftImag[band][k][chIn] = (float*)calloc(total_max_fft,sizeof(float));
				pFdBinauralRendererParam->ppppVoffCoeffRightReal[band][k][chIn] = (float*)calloc(total_max_fft,sizeof(float));
				pFdBinauralRendererParam->ppppVoffCoeffRightImag[band][k][chIn] = (float*)calloc(total_max_fft,sizeof(float));
			}
		}
	}
	/* Normalization */
	if (pFdBinauralRendererParam->flagHRIR == 1)
	{
		for (band = 0; band < pFdBinauralRendererParam->kMax; band++)
		{
			int chIn;
			for ( chIn = 0; chIn < h_BrirSet->channels; chIn++)
			{
				float Scale_L_real =  (float)sqrt(EDR_L_real[band][chIn][0]/(EDR_L_real[band][chIn][0]-EDR_L_real[band][chIn][pFdBinauralRendererParam->nFilter[band]]));
				float Scale_L_imag =  (float)sqrt(EDR_L_imag[band][chIn][0]/(EDR_L_imag[band][chIn][0]-EDR_L_imag[band][chIn][pFdBinauralRendererParam->nFilter[band]]));
				float Scale_R_real =  (float)sqrt(EDR_R_real[band][chIn][0]/(EDR_R_real[band][chIn][0]-EDR_R_real[band][chIn][pFdBinauralRendererParam->nFilter[band]]));
				float Scale_R_imag =  (float)sqrt(EDR_R_imag[band][chIn][0]/(EDR_R_imag[band][chIn][0]-EDR_R_imag[band][chIn][pFdBinauralRendererParam->nFilter[band]]));

				for ( k=0; k < pFdBinauralRendererParam->nFilter[band]; k++)
				{
					h_BrirSet->QMFdomain_left_real[k][chIn][band]  = Scale_L_real*h_BrirSet->QMFdomain_left_real[k][chIn][band];
					h_BrirSet->QMFdomain_left_imag[k][chIn][band]  = Scale_L_imag*h_BrirSet->QMFdomain_left_imag[k][chIn][band];
					h_BrirSet->QMFdomain_right_real[k][chIn][band] = Scale_R_real*h_BrirSet->QMFdomain_right_real[k][chIn][band];
					h_BrirSet->QMFdomain_right_imag[k][chIn][band] = Scale_R_imag*h_BrirSet->QMFdomain_right_imag[k][chIn][band];
				}	
			}
		}
	}
	/* FFT transform loop */
	for (band = 0; band < pFdBinauralRendererParam->kMax; band++)
	{      
		/* buffers for fft including padded zeros for early reflections */
		float *temp_real_in = NULL; 
		float *temp_imag_in = NULL; 
		float *temp_real_out = NULL; 
		float *temp_imag_out = NULL; 
		float *temp_unused = NULL; 
		float *temp_zeros = NULL; 
		int block;
		int fftlength = pFdBinauralRendererParam->nFft[band];

		for (block=0; block < pFdBinauralRendererParam->nBlk[band]; block++)
		{
			int blocklength = pFdBinauralRendererParam->nFft[band]/2;

			FFTlib_Create(&h_FFT, fftlength, -1); 

			temp_real_in = (float*)calloc(fftlength, sizeof(float)); 
			temp_imag_in = (float*)calloc(fftlength, sizeof(float)); 
			temp_real_out = (float*)calloc(fftlength, sizeof(float)); 
			temp_imag_out = (float*)calloc(fftlength, sizeof(float)); 
			temp_unused = (float*)calloc(fftlength, sizeof(float)); 
			temp_zeros = (float*)calloc(fftlength, sizeof(float)); 

			for (chOut = 0; chOut < h_BrirSet->channels; chOut++)
			{
				/* left channel */
				for(k = 0; k < min(blocklength, pFdBinauralRendererParam->nFilter[band]); k++)
				{
					temp_real_in[k] = h_BrirSet->QMFdomain_left_real[block*blocklength+k][chOut][band]; 
					temp_imag_in[k] = h_BrirSet->QMFdomain_left_imag[block*blocklength+k][chOut][band]; 
				}
				FFTlib_Apply(h_FFT, temp_real_in, temp_imag_in, temp_real_out, temp_imag_out); 
				for(k = 0; k < fftlength; k++)
				{
					pFdBinauralRendererParam->ppppVoffCoeffLeftReal[band][block][chOut][k] = temp_real_out[k]; 
					pFdBinauralRendererParam->ppppVoffCoeffLeftImag[band][block][chOut][k] = temp_imag_out[k]; 
				}  

				/* right channel */
				for(k = 0; k < min(blocklength, pFdBinauralRendererParam->nFilter[band]); k++)
				{
					if (block*blocklength+k < pFdBinauralRendererParam->nFilter[band])
					{
						temp_real_in[k] = h_BrirSet->QMFdomain_right_real[block*blocklength+k][chOut][band]; 
						temp_imag_in[k] = h_BrirSet->QMFdomain_right_imag[block*blocklength+k][chOut][band]; 
					}
				}
				FFTlib_Apply(h_FFT, temp_real_in, temp_imag_in, temp_real_out, temp_imag_out); 
				for(k = 0; k < fftlength; k++)
				{
					pFdBinauralRendererParam->ppppVoffCoeffRightReal[band][block][chOut][k] = temp_real_out[k]; 
					pFdBinauralRendererParam->ppppVoffCoeffRightImag[band][block][chOut][k] = temp_imag_out[k]; 
				}
			}  

			free(temp_real_in); temp_real_in = NULL;
			free(temp_imag_in); temp_imag_in = NULL;
			free(temp_real_out); temp_real_out = NULL;
			free(temp_imag_out); temp_imag_out = NULL;
			free(temp_unused); temp_unused = NULL;
			free(temp_zeros); temp_zeros = NULL;
			FFTlib_Destroy(&h_FFT);
		}
	}
	/* Memory Free
	1) EDR
	2) MixingTime */

	/* 1) EDR */
	for ( k=0; k<h_BrirSet->QMFbands; k++)
	{
		int chIn;
		for (chIn = 0; chIn < h_BrirSet->channels; chIn++)
		{
			free(EDR_L_real[k][chIn]); EDR_L_real[k][chIn] = NULL;
			free(EDR_L_imag[k][chIn]); EDR_L_imag[k][chIn] = NULL;
			free(EDR_R_real[k][chIn]); EDR_R_real[k][chIn] = NULL;
			free(EDR_R_imag[k][chIn]); EDR_R_imag[k][chIn] = NULL;
		}
		free(EDR_L_real[k]); EDR_L_real[k] = NULL;
		free(EDR_L_imag[k]); EDR_L_imag[k] = NULL;
		free(EDR_R_real[k]); EDR_R_real[k] = NULL;
		free(EDR_R_imag[k]); EDR_R_imag[k] = NULL;
	}
	free(EDR_L_real); EDR_L_real = NULL;
	free(EDR_L_imag); EDR_L_imag = NULL;
	free(EDR_R_real); EDR_R_real = NULL;
	free(EDR_R_imag); EDR_R_imag = NULL;
	/* 2) MixingTime */
	free(MixingTime); MixingTime = NULL;
}

void binauralParam_QtdlParamGeneration(BRIRANALYSISSTRUCT *h_BrirSet, FdBinauralRendererParam *pFdBinauralRendererParam)
{


	int k;
	int band, chOut;

#ifdef BUGFIX_KCONV_KMAX_15JUNE
    int NUM_BAND_CONV = pFdBinauralRendererParam->kConv;
	int NUM_BAND_PROC = pFdBinauralRendererParam->kMax;
#endif
	/* **************************** INIT TDL ****************************** */

	/* Parameterization */

	pFdBinauralRendererParam->ppQtdlGainLeftReal = (float**) calloc(NUM_BAND_PROC-NUM_BAND_CONV, sizeof(float*));
	pFdBinauralRendererParam->ppQtdlGainLeftImag = (float**) calloc(NUM_BAND_PROC-NUM_BAND_CONV, sizeof(float*));
	pFdBinauralRendererParam->ppQtdlGainRightReal = (float**) calloc(NUM_BAND_PROC-NUM_BAND_CONV, sizeof(float*));
	pFdBinauralRendererParam->ppQtdlGainRightImag = (float**) calloc(NUM_BAND_PROC-NUM_BAND_CONV, sizeof(float*));
	pFdBinauralRendererParam->ppQtdlLagLeft = (unsigned int**) calloc(NUM_BAND_PROC-NUM_BAND_CONV, sizeof(unsigned int*));
	pFdBinauralRendererParam->ppQtdlLagRight = (unsigned int**) calloc(NUM_BAND_PROC-NUM_BAND_CONV, sizeof(unsigned int*));
	for( band = 0; band < NUM_BAND_PROC-NUM_BAND_CONV; band++)
	{
		pFdBinauralRendererParam->ppQtdlGainLeftReal[band] = (float*) calloc((int)h_BrirSet->channels, sizeof(float));
		pFdBinauralRendererParam->ppQtdlGainLeftImag[band] = (float*) calloc((int)h_BrirSet->channels, sizeof(float));
		pFdBinauralRendererParam->ppQtdlGainRightReal[band] = (float*) calloc((int)h_BrirSet->channels, sizeof(float));
		pFdBinauralRendererParam->ppQtdlGainRightImag[band] = (float*) calloc((int)h_BrirSet->channels, sizeof(float));
		pFdBinauralRendererParam->ppQtdlLagLeft[band] = (unsigned int*) calloc((int)h_BrirSet->channels, sizeof(unsigned int));
		pFdBinauralRendererParam->ppQtdlLagRight[band] = (unsigned int*) calloc((int)h_BrirSet->channels, sizeof(unsigned int));
	}

	for( band = 0; band < NUM_BAND_PROC-NUM_BAND_CONV; band++)
	{
		for( chOut = 0; chOut < h_BrirSet->channels; chOut++)
		{	
			float E_real = 0, E_imag = 0;
			float tmp_real = 0, tmp_imag = 0;
			float max_val = 0.f;
			int max_idx = 0;
			for( k = 0; k < h_BrirSet->QMF_Timeslots; k++)
			{
				tmp_real = h_BrirSet->QMFdomain_left_real[k][chOut][band+NUM_BAND_CONV] * h_BrirSet->QMFdomain_left_real[k][chOut][band+NUM_BAND_CONV];
				tmp_imag = h_BrirSet->QMFdomain_left_imag[k][chOut][band+NUM_BAND_CONV] * h_BrirSet->QMFdomain_left_imag[k][chOut][band+NUM_BAND_CONV];
				E_real += tmp_real;
				E_imag += tmp_imag;
				if( max_val < tmp_real + tmp_imag)
				{
					max_val = tmp_real + tmp_imag;
					max_idx = k;
				}
			}
			pFdBinauralRendererParam->ppQtdlGainLeftReal[band][chOut] = (h_BrirSet->QMFdomain_left_real[max_idx][chOut][band+NUM_BAND_CONV] >= 0)? (float)sqrt((double)E_real) : -1.f*(float)sqrt((double)E_real);
			pFdBinauralRendererParam->ppQtdlGainLeftImag[band][chOut] = (h_BrirSet->QMFdomain_left_imag[max_idx][chOut][band+NUM_BAND_CONV] >= 0)? (float)sqrt((double)E_imag) : -1.f*(float)sqrt((double)E_imag);
			pFdBinauralRendererParam->ppQtdlLagLeft[band][chOut] = max_idx;

			E_real = 0, E_imag = 0;
			max_val = 0.f; max_idx = 0;
			for( k = 0; k < h_BrirSet->QMF_Timeslots; k++)
			{
				tmp_real = h_BrirSet->QMFdomain_right_real[k][chOut][band+NUM_BAND_CONV] * h_BrirSet->QMFdomain_right_real[k][chOut][band+NUM_BAND_CONV];
				tmp_imag = h_BrirSet->QMFdomain_right_imag[k][chOut][band+NUM_BAND_CONV] * h_BrirSet->QMFdomain_right_imag[k][chOut][band+NUM_BAND_CONV];
				E_real += tmp_real;
				E_imag += tmp_imag;
				if( max_val < tmp_real + tmp_imag)
				{
					max_val = tmp_real + tmp_imag;
					max_idx = k;
				}
			}
			pFdBinauralRendererParam->ppQtdlGainRightReal[band][chOut] = (h_BrirSet->QMFdomain_right_real[max_idx][chOut][band+NUM_BAND_CONV] >= 0)? (float)sqrt((double)E_real) : -1.f*(float)sqrt((double)E_real);
			pFdBinauralRendererParam->ppQtdlGainRightImag[band][chOut] = (h_BrirSet->QMFdomain_right_imag[max_idx][chOut][band+NUM_BAND_CONV] >= 0)? (float)sqrt((double)E_imag) : -1.f*(float)sqrt((double)E_imag);
			pFdBinauralRendererParam->ppQtdlLagRight[band][chOut] = max_idx;

		}
	}

}