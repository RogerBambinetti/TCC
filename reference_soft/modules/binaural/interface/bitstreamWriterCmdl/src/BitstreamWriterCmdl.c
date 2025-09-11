/***********************************************************************************
 
 This software module was originally developed by
 
 Orange Labs
 
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
 
 Orange Labs retain full right to modify
 and use the code for its own purpose, assign or donate the code to a third party and
 to inhibit third parties from using the code for products that do not conform to
 MPEG-related ITU Recommendations and/or ISO/IEC International Standards.
 
 This copyright notice must be included in all copies or derivative works.
 
 Copyright (c) ISO/IEC 2014.
 
 ***********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#include "bitstreamBinaural.h"



static void helpCmd()
{
	fprintf(stderr,"bitstreamWriterCmdl.exe\n");
	fprintf(stderr,"Purpose:\nCreate binaural bitsream (FIR) from BRIR wavfiles\n");
	fprintf(stderr,"Args:\n");
	fprintf(stderr,"  -wavprefix  <in  : path to input brir wavfiles (with prefix)>   (required)\n");
	fprintf(stderr,"  -hoaOrder   <in  : HOA order>                                   (optional)\n");
	fprintf(stderr,"  -cicp	      <in  : CICP index> e.g., 5.1->6  22.2->13           (optional)\n");
	fprintf(stderr,"  -geo        <in  : path to geometric information file>          (optional)\n");
	fprintf(stderr,"  -bs         <out : brir bitstream filename>                     (required)\n");
	fprintf(stderr,"  -bsSig      <out : brir bitstream signature>                    (optional, default value: 0)\n");
	fprintf(stderr,"  -bsVer      <out : brir bitstream version>                      (optional, default value: 0)\n");
  fprintf(stderr,"  -track                                                          (optional, switches on/off useTrackingMode)\n");
	fprintf(stderr,"\n");
	fprintf(stderr,"Usage ex1: \n");
	fprintf(stderr,"    bitstreamWriterCmdl.exe -wavprefix ../BRIRdata/IIS/IIS_BRIR_ -cicp 13 -bs ../BRIRdata/IIS_BRIR_CICP13.bs");
	fprintf(stderr,"\n");
	fprintf(stderr,"Usage ex2: \n");
	fprintf(stderr,"    bitstreamWriterCmdl.exe -wavprefix ../BRIRdata/IIS/IIS_BRIR_ -hoaOrder 6 -bs ../BRIRdata/IIS_BRIR_hoaOrder6.bs -bsSig 1789 -bsVer 1");
}


/*---------------------------------------------------------------------------*/
static int parseCmdParameter(	int argc,
							 char *argv[],			
							 char *sBRIRpath,
							 int	 *configType,
							 int	 *hoaOrder,
							 int	 *cicp,
							 char	 *sGeoPath,
							 unsigned long	*fileSignature,
							 unsigned short	*fileVersion,
               int *useTrackingMode,
							 char *sBSpath)
{
	unsigned int nArgs = 0;
	int	i;

	*configType = -1; /* 0:HOA  1:CICP 2:GEO */

	/* loop over all parameters */
	for (i=1; i<argc; ++i )
	{
		if (!strcmp(argv[i],"-wavprefix"))      /* Required */
		{
			
			if (++i < argc )
			{
				strcpy(sBRIRpath, argv[i]);
				nArgs++;
				continue;
			}
		}
		else if (!strcmp(argv[i],"-hoaOrder"))      /* Optional */
		{
			if (*configType != -1)
			{
				fprintf(stderr,"Error : Wrong argument! -hoaOrder, -cicp and -geo are not compatible \n\n");
				return -1;
			}
			*configType = 0;
			if (++i < argc )
			{
				*hoaOrder = atoi(argv[i]);
				nArgs++;
				continue;
			}
		}
		else if (!strcmp(argv[i],"-cicp"))      /* Optional */
		{
			if (*configType != -1)
			{
				fprintf(stderr,"Error : Wrong argument! -hoaOrder, -cicp and -geo are not compatible \n\n");
				return -1;
			}
			*configType = 1;
			if (++i < argc )
			{
				*cicp = atoi(argv[i]);
				nArgs++;
				continue;
			}
		}	
		else if (!strcmp(argv[i],"-geo"))      /* Optional */
		{
			if (*configType != -1)
			{
				fprintf(stderr,"Error : Wrong argument! -hoaOrder, -cicp and -geo are not compatible \n\n");
				return -1;
			}
			*configType = 2;
			if (++i < argc )
			{
				strcpy(sGeoPath, argv[i]);
				nArgs++;
				continue;
			}
		}	

		else if (!strcmp(argv[i],"-bs"))      /* Required */
		{
			if (++i < argc )
			{
				strcpy(sBSpath, argv[i]);
				nArgs++;
				continue;
			}
		}	
		else if (!strcmp(argv[i],"-bsSig"))      /* Optional */
		{
			if (++i < argc )
			{
				*fileSignature = atoi(argv[i]);
				continue;
			}
		}
		else if (!strcmp(argv[i],"-bsVer"))      /* Optional */
		{
			if (++i < argc )
			{
				*fileVersion = atoi(argv[i]);
				continue;
			}
		}
    else if (!strcmp(argv[i],"-track"))      /* Optional */
		{
			*useTrackingMode = 1;
			continue;
		}
		else 
		{
			fprintf(stderr,"Error : Wrong argument! Require at least 3 arguments : -wavprefix, -bs, and (-hoaOrder OR -cicp OR -geo)\n\n");
			helpCmd();
			return -1;
		}  
	}

	/* check number of parameters found */
	if(nArgs < 3)
	{
		fprintf(stderr,"Error : Wrong argument! Require at least 3 arguments : -wavprefix, -bs, and (-hoaOrder OR -cicp OR -geo)\n\n");
		helpCmd();
		return -1;
	}
	else
		return 0;
};

/*---------------------------------------------------------------------------*/
static int replaceSlash( char *pPathFile)
{
	char	*pTmp = NULL;

	/* replace '\' by '/' */
	pTmp = pPathFile;

	while( *pTmp != '\0' )
	{
		if ( *pTmp == '\\' )
		{
			*pTmp = '/';
		}
		pTmp++;
	};
	return 0;
}


/*---------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	int configType = -1; /* 0:HOA  1:CICP 2:GEO */
	int cicp = -1;
	int hoaOrder = -1;
	char sGeoPath[256] = ("");
	char sBRIRpath[256];
	char sOuputBitstream[256] = ("");
	int initError = 0;
  int useTrackingMode = 0;

	BinauralRendering *pBinauralRendering;

	unsigned long	fileSignature = 0;
	unsigned short  fileVersion = 0;

	fprintf(stderr, "\n\n"); 
	fprintf(stdout, "\n");
	fprintf(stdout, "******************** MPEG-H 3D Audio Coder - Edition 4.0 **********************\n");
	fprintf(stdout, "*                                                                             *\n");
	fprintf(stdout, "*                      TD Binaural Bitstream Writer                           *\n");
	fprintf(stdout, "*                                                                             *\n");
	fprintf(stdout, "*                                  %s                                *\n", __DATE__);
	fprintf(stdout, "*                                                                             *\n");
	fprintf(stdout, "*    This software may only be used in the development of the MPEG 3D Audio   *\n");
	fprintf(stdout, "*    standard, ISO/IEC 23008-3 or in conforming implementations or products.  *\n");
	fprintf(stdout, "*                                                                             *\n");
	fprintf(stdout, "*******************************************************************************\n");
	fprintf(stdout, "\n");

	/* Parse input parameters */
	if (parseCmdParameter(argc, argv, 
		sBRIRpath, 
		&configType, 
		&hoaOrder, 
		&cicp, 
		sGeoPath, 
		&fileSignature, 
		&fileVersion, 
    &useTrackingMode,
		sOuputBitstream) != 0)
	{
		fprintf(stderr,"\n");
		return -1;
	}
	replaceSlash(sBRIRpath);
	replaceSlash(sGeoPath);
	replaceSlash(sOuputBitstream);

	/* Init BinauralRendering ptr */
	pBinauralRendering = BinauralRendering_initAlloc();
	if (pBinauralRendering == NULL)
	{
		fprintf(stderr,"\n Error in BinauralRendering_initAlloc()");
		return -1;
	}

	/* load filters from wavfiles */
	if (configType == 0) /* HOA */
	{
		if ( BinauralRendering_addRepresentationFromWav_HOA(pBinauralRendering, sBRIRpath, hoaOrder, useTrackingMode) != 0 )
		{
			fprintf(stderr,"\nError -> could not load filters ! \n");
			return -1;
		}
    else
    {
      fprintf(stderr,"\nBRIRs read from %s\n", sBRIRpath);
    }
	}
	else /* CICP or GEO */
	{
		if ( BinauralRendering_addRepresentationFromWav_CICP_GEO(pBinauralRendering, sBRIRpath, cicp, sGeoPath, useTrackingMode) != 0 )
		{
			fprintf(stderr,"\nError -> could not load filters ! \n");
			return -1;
		}
    else
    {
      fprintf(stderr,"\nBRIRs read from %s\n", sBRIRpath);
    }
	}
	
	/* export BinauralRendering as binaural bitstream */
	if (bitstreamWrite(sOuputBitstream, fileSignature, fileVersion, pBinauralRendering) == 0)
	{
		fprintf(stderr,"\nBRIR bitstream was successfully written in %s\n", sOuputBitstream);
	}
	else
	{
		fprintf(stderr,"\n");
		fprintf(stderr,"\nError, write BRIR bitstream failed %s\n", sOuputBitstream);
		return -1;
	}

	return 0;

}





