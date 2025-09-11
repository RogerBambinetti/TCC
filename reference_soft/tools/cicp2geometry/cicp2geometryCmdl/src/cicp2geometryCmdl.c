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
 
 Fraunhofer IIS retains full right to modify and use the code for its own purpose,
 assign or donate the code to a third party and to inhibit third parties from using 
 the code for products that do not conform to MPEG-related ITU Recommendations and/or 
 ISO/IEC International Standards.
 
 This copyright notice must be included in all copies or derivative works. 
 
 Copyright (c) ISO/IEC 2013.
 
 ***********************************************************************************/

/* system includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* locals includes */
#include "cicp2geometry.h"

/* prototypes */
void printHelp( void );

void printDisclaimer( void );

int parseCmdl( int argc, char *** argv, int* inConf, char* filename);


/* main function */
int main(int argc, char *argv[])
{

  int cicpIndex = 0;
  int tmpCicpIndex = 0;
  int error = 0;
  CICP2GEOMETRY_CHANNEL_GEOMETRY myTable[32] = { {0} };

  int numChannels = 0;
  int numLFEs = 0;
  char filename[256] = {0};

  /* print disclaimer info */
  printDisclaimer();


  /* parse command line */
  if ( parseCmdl( argc,  &argv, &cicpIndex, filename ) ) {
    return 1;
  }

  /* read geometry from file- if file is given */
  if ( *filename != '\0' ) {
    error = cicp2geometry_get_geometry_from_file( filename, myTable, &numChannels, &numLFEs );
    if( error ) {
      cicpIndex = -1000;
    }
    else {
      /* any setup*/
      cicpIndex = 0;
    }
  }
  else {

    error = cicp2geometry_get_geometry_from_cicp( cicpIndex, myTable, &numChannels, &numLFEs);
    if (error) {
      fprintf(stderr, "ERROR in cicp2geometry_get_geometry_from_cicp()!\n");
    }
    else {
      fprintf(stderr, "Valid CICP index %i!\n", cicpIndex);
    }

  }

  error = cicp2geometry_get_cicpIndex_from_geometry(myTable, numChannels, numLFEs, &tmpCicpIndex);
  if (error) {
    fprintf(stderr, "Geometry can't be matched with a CICP index.\n");
  }
  else {
    fprintf(stderr, "Geometry describes index %i!\n", tmpCicpIndex);
  }

  error = cicp2geometry_write_geometry_to_file("user_geo.txt", myTable, numChannels, numLFEs);
  if (error) {
    fprintf(stderr, "Error writing the geometry file.\n");
  }

  return 0;

}

int parseCmdl( int argc, char *** pargv, int* cicpIndex, char* filename) {

  /* command line parsing */
  int i;
  char ** argv = *pargv;
  *cicpIndex = -1;

  for ( i = 1; i < argc; ++i )
  {
    if (!strcmp(argv[i],"-cicpIn"))      /* Optional */
    {
      *cicpIndex = atoi(argv[i+1]);
      i++;
    }
    if (!strcmp(argv[i],"-inGeo"))      /* Optional */
    {
      strcpy(filename, argv[i+1]);
      i++;
    }
  }

  if ( *cicpIndex == -1 && *filename == '\0' ) {
    printHelp();
    return 1;
  }

  return 0;
}

void printHelp( void ) {
    fprintf( stderr, "Invalid arguments, usage:\n");
    fprintf( stderr, "-cicpIn <int>        cicpIndex\n");
    fprintf( stderr, "-inGeo <char>        filename\n");
}


void printDisclaimer( void ) {
  fprintf(stderr, "\n\n");
  fprintf( stdout, "\n"); 
  fprintf( stdout, "******************** MPEG-H 3D Audio Coder - Edition 4.0 **********************\n");
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*                           cicp2geometry module                              *\n");
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*                                  %s                                *\n", __DATE__);
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*    This software may only be used in the development of the MPEG 3D Audio   *\n");
  fprintf( stdout, "*    standard, ISO/IEC 23008-3 or in conforming implementations or products.  *\n");
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*******************************************************************************\n"); 
  fprintf( stdout, "\n");
  
}
