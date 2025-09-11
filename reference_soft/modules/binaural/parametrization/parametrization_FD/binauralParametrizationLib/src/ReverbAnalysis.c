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
#include "ReverbAnalysis.h"
#include "wavIO.h"
#include "cicp2geometry.h"

static __inline int IsLittleEndian (void); 
static __inline short LittleEndian16 (short v); 
float ReverseFloat(const float inFloat ); 
static int binauralParam_getWavHeader(FILE *pInFileName, RIFFWAVHEADER *header); 
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


int binauralParam_readBRIRs_multipleWAV_writeGeoInfo(FILE *fbrirs_filelist, H_BRIRANALYSISSTRUCT h_BRIRs, char *brir_baseinpath, H_BRIRMETADATASTRUCT h_Metadata, char *cicp_file_path)
{
  char *inbuffer; 
  char brir_inpath[4096];
  int pos = 0, pos_init = 0;
  int error = 0, i = 0, k = 0, numBRIRs = 0;
  BRIRANALYSISSTRUCT BRIRs = { 0 }; 
  BRIRMETADATASTRUCT Metadata = { 0 };
  char *ptr;
  char number[10];
  char filename[4096];
  int channels = 0;
  int samples = 0;
  FILE *brir = NULL;
  int nSamplesRead = 0, nSamplesWritten = 0;
  int nSamplesFilled = 0;
  int fs = 0;
  int bps = 0;
  int isLastFrame = 0;
  int nZerosBegin = 0, nZerosEnd = 0;
  float **readBuffer = NULL;
  float **writeBuffer = NULL;
  float **position_data_temp = NULL;
  int ctLFE = 0;
  int *LFE;
  int nBRIRs_noLFE = 0;

  WAVIO_HANDLE h_wavIO = NULL;
  WAVIO_HANDLE h_wavIO2 = NULL;
  RIFFWAVHEADER riffheader = {{ 0 }}; 

  CICP2GEOMETRY_CHANNEL_GEOMETRY geoInfo[CICP2GEOMETRY_MAX_LOUDSPEAKERS] = { 0 };

  inbuffer = (char*)calloc(4096, sizeof(char)); 
  fread(inbuffer, sizeof(char), 4096, fbrirs_filelist); 
  ptr = strstr(inbuffer,",");
  pos_init = ptr - inbuffer;
  for (i = 0; i < pos_init; i++)
  {
    number[i] = inbuffer[i];
  }
  numBRIRs = atoi(number);
  BRIRs.channels = numBRIRs;

  LFE = (int*)calloc(numBRIRs,sizeof(int));
  Metadata.elevation = (float*)calloc(numBRIRs,sizeof(float));
  Metadata.azimuth = (float*)calloc(numBRIRs,sizeof(float));
  Metadata.channels = numBRIRs;

  for (i = 0; i < numBRIRs; i++)
  {
    LFE[i] = -1;
  }

  ptr++;
  pos_init++;
  for (i = 0; i < numBRIRs; i++)
  {
    char *ptr_comma;
    memset(&filename,'\0',4096);
    ptr_comma = strstr(ptr,",");
    if (ptr_comma != NULL)
    {
      pos = ptr_comma-ptr;
      for (k = 0; k < pos; k++)
      {
        filename[k] = inbuffer[pos_init+k];
      }
      filename[k] = '\0';
    }
    else
    {
      char temp = 'a';
      int count = 0;
      while (temp != '\0')
      {
        temp = ptr[count];
        filename[count] = temp;
        count++;
      }
    }


    /* read wave file */
    strcpy(brir_inpath,brir_baseinpath);
    strcat(brir_inpath,filename);
    brir = fopen(brir_inpath,"rb");

    /* extract position data */
    {
      char *ptrA, *ptrE, *ptrD, *ptrLFE;
      int posA, posE, posD, posLFE = -1;
      char Az[4096];
      char El[4096];
      float az = 0.0f;
      float el = 0.0f;
      int length = 0;
      int ct = 0;
      ptrA = strstr(filename,"_A");
      ptrE = strstr(filename,"_E");
      ptrD = strstr(filename,".");
      posA = ptrA-filename;
      posE = ptrE-filename;
      posD = ptrD-filename;

      ptrLFE = strstr(filename,"LFE_");
      length = 4;
      if (ptrLFE == NULL)
      {
        ptrLFE = strstr(filename,"LFE");
        length = 3;
      }

      if ((ptrA != NULL) && (ptrE != NULL) && (ptrD != NULL))
      {
        for (k = posA+2; k < posE; k++)
        {
          Az[ct] = filename[k];
          ct++;
        }
        Az[ct] = '\0';
        az = (float)atof(Az);
        ct = 0;
        for (k = posE+2; k < posD; k++)
        {
          El[ct] = filename[k];
          ct++;
        }
        El[ct] = '\0';
        el = (float)atof(El);

        Metadata.azimuth[i] = az;
        Metadata.elevation[i] = el;

        geoInfo[i].Az = (int)az;
        geoInfo[i].El = (int)el;
        geoInfo[i].LFE = 0;
        geoInfo[i].cicpLoudspeakerIndex = -1;
      }
      else if (ptrLFE != 0) /*LFE cases */
      {
        char numLFE[4096];
        int LFE_number = 0;
        ct = 0;

        ptrD = strstr(filename,".");
        posLFE = ptrLFE-filename;
        posD = ptrD-filename;

        for (k = posLFE+length; k < posD; k++)
        {
          numLFE[ct] = filename[k];
          ct++;
        }
        numLFE[ct] = '\0';
        LFE_number = (int)atoi(numLFE);

        LFE[ctLFE] = i;
        ctLFE++;
        Metadata.azimuth[i] = 400;
        Metadata.elevation[i] = 400;

        if (LFE_number <= 2)
        {
          if (LFE_number == 1)
          {
            geoInfo[i].cicpLoudspeakerIndex = 3;
          }
          else if (LFE_number == 2)
          {
            geoInfo[i].cicpLoudspeakerIndex = 26;
          }
        }
        else
        {
          /* no azimuth and elevation available from file name of LFE files, LFE_number has no cicpLoudspeakerIndex */
          geoInfo[i].LFE = 1;
          geoInfo[i].Az = 400;
          geoInfo[i].El = 400;
          geoInfo[i].cicpLoudspeakerIndex = -1;
        }
      }
      else
      {
          geoInfo[i].LFE = 0;
          geoInfo[i].Az = 400;
          geoInfo[i].El = 400;
          geoInfo[i].cicpLoudspeakerIndex = -1;
      }

      Metadata.numLFE = ctLFE;
      if (ctLFE > 0)
      {
        Metadata.LFEchannels = (int*)calloc(ctLFE,sizeof(int));
        for (k = 0; k < ctLFE; k++)
        {
          Metadata.LFEchannels[k] = LFE[k];
        }
      }
    }


    /* read BRIR data */
    if (i == 0)
    {
      error = binauralParam_getWavHeader(brir, &riffheader); 
      channels = riffheader.channels; 
      samples = riffheader.data_length / (riffheader.bpsample/8)/ riffheader.channels; 

      BRIRs.fs_brirs = riffheader.fs;
      BRIRs.timedomain_samples = samples;
      BRIRs.timedomain_left = (float**)calloc(BRIRs.channels, sizeof(float*)); 
      BRIRs.timedomain_right = (float**)calloc(BRIRs.channels, sizeof(float*)); 
      for (k = 0; k < BRIRs.channels; k++)
      {
        BRIRs.timedomain_left[k] = (float*)calloc(BRIRs.timedomain_samples, sizeof(float)); 
        BRIRs.timedomain_right[k] = (float*)calloc(BRIRs.timedomain_samples, sizeof(float)); 
      }

      if (riffheader.bpsample != 24)
      {
        /* write 24 bit */
        riffheader.bpsample = 24;
      }
      rewind(brir);

      readBuffer = (float**)calloc(channels,sizeof(float*));
      for (k = 0; k < channels; k++)
      {
        readBuffer[k] = (float*)calloc(samples,sizeof(float));
      }
    }

    error = wavIO_init(&h_wavIO,samples,0,0);
    error = wavIO_openRead(h_wavIO,brir,&channels,&fs,&bps,&nSamplesRead,&nSamplesFilled);
    error = wavIO_readFrame(h_wavIO,readBuffer,&nSamplesRead,&isLastFrame,&nZerosBegin,&nZerosEnd);
    error = wavIO_close(h_wavIO);

    for (k = 0; k < samples; k++)
    {
      BRIRs.timedomain_left[i][k] = readBuffer[0][k];
      BRIRs.timedomain_right[i][k] = readBuffer[1][k];
    }
    ptr = ptr_comma+1;
    pos_init += (pos+1);
  }


  fclose(fbrirs_filelist);

  nBRIRs_noLFE = numBRIRs - ctLFE;
  cicp2geometry_write_geometry_to_file(cicp_file_path,geoInfo,nBRIRs_noLFE,ctLFE);


  *h_BRIRs = BRIRs;
  *h_Metadata = Metadata;

  free(inbuffer); inbuffer = NULL;

  for (k = 0; k < channels; k++)
  {
    free(readBuffer[k]); readBuffer[k] = NULL;
  }
  free(readBuffer); readBuffer = NULL;

  free(LFE); LFE = NULL;

  return 0;

}

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


static int binauralParam_getWavHeader(FILE *pInFileName, RIFFWAVHEADER *header)
{
  RIFFWAVHEADER riffheader = {{ 0 }}; 

  unsigned int riff = BigEndian32 ('R','I','F','F');
  unsigned int junk = BigEndian32 ('J','U','N','K');
  unsigned int fmt = BigEndian32  ('f','m','t',' ');

  unsigned int i=0;
  unsigned int chunkID;

  int fmt_read = 0;  

  rewind(pInFileName); 

  if (pInFileName == NULL) 
  {
    return -1;
  }

  while (!fmt_read)
  {
    fread(&chunkID, sizeof(unsigned int),1,pInFileName);
    if (chunkID == riff)
    {
      memcpy(riffheader.riff_name, (char*) &chunkID,4);
      fread(&riffheader.riff_length, sizeof(unsigned int),1,pInFileName);
      fread(&riffheader.riff_typ, sizeof(unsigned int), 1, pInFileName);
    }
    else if (chunkID == junk)
    {
      /* do nothing */
    }
    else if (chunkID == fmt)
    {
      memcpy(riffheader.fmt_name, (char*) &chunkID,4);
      fread(&riffheader.fmt_length, sizeof(unsigned int),1,pInFileName);
      fread(&riffheader.format_tag, 2, 1, pInFileName);
      fread(&riffheader.channels,2,1,pInFileName);
      fread(&riffheader.fs,sizeof(unsigned int),1,pInFileName);
      fread(&riffheader.bytes_per_sec,sizeof(unsigned int),1,pInFileName);
      fread(&riffheader.block_align,2,1,pInFileName);
      fread(&riffheader.bpsample,2,1,pInFileName);
      fmt_read = 1;
    }
    else
    {
      unsigned long int pos = 0;
      i++;
      if (i > 5000)
      {
        break;
      }
      pos = ftell(pInFileName);
      fseek(pInFileName, pos-3, SEEK_SET);
    }
  }

  /* Search for data chunk */
  {
    /* Search data chunk */
    unsigned int i = 0;
    unsigned int dataTypeRead = 0;
    unsigned int dataType = BigEndian32 ('d','a','t','a') ;
    while(1) 
    {
      i++;
      if( i > 5000 ) {
        /* Error */
        break;
      }

      fread( &dataTypeRead, sizeof(unsigned int), 1, pInFileName );
      if ( dataTypeRead == dataType) {
        /* 'data' chunk found - now read dataSize */
        memcpy(riffheader.data_name, (char*) &dataTypeRead, 4);
        fread(&(riffheader.data_length), sizeof(unsigned int), 1 , pInFileName);
        break;
      }
      else {
        /* 3 bytes back */
        unsigned long int pos=0;
        pos = ftell(pInFileName);
        fseek(pInFileName, pos-3, SEEK_SET);
      }
    }
  }

  *header = riffheader; 

  if ((riffheader.format_tag != 1))
  {
    return -1;
  }

  return 0; 
}


void binauralParam_analyzeLR(H_BRIRANALYSISSTRUCT h_BRIRs, H_BRIRMETADATASTRUCT h_BRIRmetadata, int end_analysis)
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
  float *BRIR_temp = (float*)calloc(min(end_analysis,h_BRIRs->timedomain_samples),sizeof(float));
  int ch = 2;
  int pos = h_BRIRs->channels;
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

  fmin = (int)floor((fmin / 48000.0f*(h_BRIRs->fs_brirs*1.0f))+0.5);

  for (i= -bandsperoctave*6; i <= bandsperoctave*6; i++)
  {
    fctemp[ct] = (float)(pow(2.0f,((i*1.0f)/(bandsperoctave*1.0f)))*1000.0f) / 48000.0f*(h_BRIRs->fs_brirs*1.0f);
    if ((fctemp[ct] >= fmin) && (fctemp[ct] < h_BRIRs->fs_brirs/2.0f))
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
    if ((fctemp[i] >= fmin) && (fctemp[i] < h_BRIRs->fs_brirs/2.0f))
    {
      fc[ct2] = fctemp[i];
      ct2++;
    }
  }
  fc[0] = max(20.0f,(float)(fmin-30));

  numcoeffs = 7;
  B_temp = (double*)calloc(numcoeffs,sizeof(double));
  A_temp = (double*)calloc(numcoeffs,sizeof(double));
  
  stop = min(end_analysis,h_BRIRs->timedomain_samples);

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
          BRIR_temp[t] = h_BRIRs->timedomain_left[j][t];
        }
        else
        {
          BRIR_temp[t] = h_BRIRs->timedomain_right[j][t];
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
    fc_QMF[j] = ((float)(h_BRIRs->fs_brirs/2.0f))/QMFLIB_NUMBANDS * (j+0.5f);
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
    startband[j] = h_BRIRmetadata->transition[min(h_BRIRmetadata->noBands_transition-1,min_diff_fc_idx)]+ h_BRIRmetadata->initDelay;

    startband[j] = min(startband[j],h_BRIRs->timedomain_samples);

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

  h_BRIRmetadata->LateReverb_fc = (float*)calloc(numbands, sizeof(float));
  h_BRIRmetadata->LateReverb_RT60 = (float*)calloc(numbands, sizeof(float));
  h_BRIRmetadata->LateReverb_energy = (float*)calloc(numbands, sizeof(float));
  h_BRIRmetadata->LateReverb_AnalysisBands = numbands;

  ct_num = 0;

  fprintf(stderr, "\n>> Late reverberation analysis started...\n");

  for (j = 0; j < pos; j++)
  {
    float max_BRIR = 0.0f;
    int not_LFE = 1;
    for (s = 0; s < h_BRIRs->timedomain_samples; s++)
    {
      max_BRIR = max(max_BRIR,(float)fabs(h_BRIRs->timedomain_left[j][s]));
      max_BRIR = max(max_BRIR,(float)fabs(h_BRIRs->timedomain_right[j][s]));
    }
    fprintf(stderr, ">> ...analysing BRIR %d of %d\n", j+1, pos);

    /* check if BRIR is zero or LFE*/
    for (s = 0; s < h_BRIRmetadata->numLFE; s++)
    {
      if (j == h_BRIRmetadata->LFEchannels[s])
      {
        not_LFE = 0;
        break;
      }
    }

    if ((max_BRIR > 0.0f) && (not_LFE))
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
            binauralParam_estimate_noisefloor(BRIRs_bandwise,startband,endband,j,i,k, &noiselevel, &endcalc, minv[i], h_BRIRs->fs_brirs);
          }
          else
          {
            endcalc = endband[i] - startband[i];
          }           
            
          if (endcalc > 0)
          {        
            /* RT60 and energy */
            binauralParam_getRT60(BRIRs_bandwise,startband,endcalc,j,i,k,h_BRIRs->fs_brirs,&RT60_temp);
            binauralParam_getNRG(BRIRs_bandwise,startband,endcalc,j,i,k,h_BRIRs->fs_brirs,&NRG_temp);
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

    h_BRIRmetadata->LateReverb_fc[i] = fc[i];
    h_BRIRmetadata->LateReverb_energy[i] = NRG[i];
    h_BRIRmetadata->LateReverb_RT60[i] = RT60[i];
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

void binauralParam_writeMetadata(H_BRIRMETADATASTRUCT h_BRIRmetadata, H_BRIRANALYSISSTRUCT h_BRIRs, FILE *fBRIRmetadata)
{

  /*
  Index	  Type            #						What
  -------------------------------------------------
  0		    Int             1						Number of LR analysis bands Kana
  1		    Int             1						Number of D+E analysis bands Ktrans
  2		    float           Kana				fc
  3		    float           Kana				RT60
  4		    float           Kana				nrg
  5		    Int             1						Initial Delay
  6		    int	            Ktrans			transition
  7		    Int             1						nBRIRs
  8		    float           2*nBrirs		azimuth + elevation
  9		    Int             1						numLFE 
  10(Opt)	Int 					  numLFE	    LFEchannels (LFE channel numbers)
  11		  Int             Ktrans			filterlength_fft
  12		  float           Ktrans			N_FFT (fft length per band)
  13		  float           Ktrans			N_BLK (number of block per band)
  14		  float           ??					FFTdomain_left_real
  15		  float           ??					FFTdomain_left_imag
  16		  float           ??					FFTdomain_right_real
  17		  float           ??					FFTdomain_right_imag
  18		  int             1						numband_conv			
  19		  float           ??					TDL_gain_left_real
  20		  float           ??					TDL_gain_left_imag
  21		  float           ??					TDL_gain_right_real
  22		  float           ??					TDL_gain_right_imag
  23		  int	            ??					TDL_delay_left
  24		  int	            ??					TDL_delay_right
  25      unsigned int    1           sampling rate


  binary file, .dat, 32 bit per sample, float, little endian

  */

  int i = 0, k = 0;
  float data = 0.0f;
  int nWritten = 0;

  for (i = 0; i < 26; i++)
  {
    switch(i)
    {
    case 0:
      data = (float)h_BRIRmetadata->LateReverb_AnalysisBands;
      if (!IsLittleEndian())
      {
        ReverseFloat(data);
      }
      nWritten += fwrite(&data,sizeof(float),1,fBRIRmetadata);
      break;
    case 1:
      data = (float)h_BRIRmetadata->noBands_transition;
      if (!IsLittleEndian())
      {
        ReverseFloat(data);
      }
      nWritten += fwrite(&data,sizeof(float),1,fBRIRmetadata);
      break;
    case 2:
      {
        if (h_BRIRmetadata->LateReverb_AnalysisBands > 0)
        {
          float *data2 = (float*)calloc(h_BRIRmetadata->LateReverb_AnalysisBands,sizeof(float));
          for (k = 0; k < h_BRIRmetadata->LateReverb_AnalysisBands; k++)
          {
            data2[k] = h_BRIRmetadata->LateReverb_fc[k];
            if (!IsLittleEndian())
            {
              ReverseFloat(data2[k]);
            }
          }
          nWritten += fwrite(data2,sizeof(float),h_BRIRmetadata->LateReverb_AnalysisBands,fBRIRmetadata);
          free(data2); data2 = NULL;
        }
      }
      break;
    case 3:
      {
        if (h_BRIRmetadata->LateReverb_AnalysisBands > 0)
        {
          float *data2 = (float*)calloc(h_BRIRmetadata->LateReverb_AnalysisBands,sizeof(float));
          for (k = 0; k < h_BRIRmetadata->LateReverb_AnalysisBands; k++)
          {
            data2[k] = h_BRIRmetadata->LateReverb_RT60[k];
            if (!IsLittleEndian())
            {
              ReverseFloat(data2[k]);
            }
          }
          nWritten += fwrite(data2,sizeof(float),h_BRIRmetadata->LateReverb_AnalysisBands,fBRIRmetadata);
          free(data2); data2 = NULL;
        }
      }
      break;
    case 4:
      {
        if (h_BRIRmetadata->LateReverb_AnalysisBands > 0)
        {
          float *data2 = (float*)calloc(h_BRIRmetadata->LateReverb_AnalysisBands,sizeof(float));
          for (k = 0; k < h_BRIRmetadata->LateReverb_AnalysisBands; k++)
          {
            data2[k] = h_BRIRmetadata->LateReverb_energy[k];
            if (!IsLittleEndian())
            {
              ReverseFloat(data2[k]);
            }
          }
          nWritten += fwrite(data2,sizeof(float),h_BRIRmetadata->LateReverb_AnalysisBands,fBRIRmetadata);
          free(data2); data2 = NULL;
        }
      }
      break;
    case 5:
      data = (float)h_BRIRmetadata->initDelay;
      if (!IsLittleEndian())
      {
        ReverseFloat(data);
      }
      nWritten += fwrite(&data,sizeof(float),1,fBRIRmetadata);
      break;
    case 6:
      {
        if (h_BRIRmetadata->noBands_transition > 0)
        {
          float *data2 = (float*)calloc(h_BRIRmetadata->noBands_transition,sizeof(float));
          for (k = 0; k < h_BRIRmetadata->noBands_transition; k++)
          {
            data2[k] = (float)h_BRIRmetadata->transition[k];
            if (!IsLittleEndian())
            {
              ReverseFloat(data2[k]);
            }
          }
          nWritten += fwrite(data2,sizeof(float),h_BRIRmetadata->noBands_transition,fBRIRmetadata);
          free(data2); data2 = NULL;
        }
      }
      break;
    case 7:
      data = (float)h_BRIRs->channels;
      if (!IsLittleEndian())
      {
        ReverseFloat(data);
      }
      nWritten += fwrite(&data,sizeof(float),1,fBRIRmetadata);
      break;
    case 8:
      {
        if (2*h_BRIRs->channels > 0)
        {
          float *data2 = (float*)calloc(2*h_BRIRs->channels,sizeof(float));
          int ct = 0;
          for (k = 0; k < 2*h_BRIRs->channels; k=k+2)
          {
            data2[k] = h_BRIRmetadata->azimuth[ct]; 
            data2[k+1] = h_BRIRmetadata->elevation[ct];
            if (!IsLittleEndian())
            {
              ReverseFloat(data2[k]);
              ReverseFloat(data2[k+1]);
            }
            ct++;
          }
          nWritten += fwrite(data2,sizeof(float),2*h_BRIRs->channels,fBRIRmetadata);
          free(data2); data2 = NULL;
        }
      }
      break;
    case 9:
      data = (float)h_BRIRmetadata->numLFE;
      if (!IsLittleEndian())
      {
        ReverseFloat(data);
      }
      nWritten += fwrite(&data,sizeof(float),1,fBRIRmetadata);
      break;
    case 10:
      {
        if (h_BRIRmetadata->numLFE > 0)
        {
          float *data2 = (float*)calloc(h_BRIRmetadata->numLFE,sizeof(float));
          for (k = 0; k < h_BRIRmetadata->numLFE; k++)
          {
            data2[k] = (float)h_BRIRmetadata->LFEchannels[k]; 
            if (!IsLittleEndian())
            {
              ReverseFloat(data2[k]);
            }
          }
          nWritten += fwrite(data2,sizeof(float),h_BRIRmetadata->numLFE,fBRIRmetadata);
          free(data2); data2 = NULL;
        }
      }
      break;
    case 11:
      {
        if (h_BRIRmetadata->noBands_transition > 0)
        {
          float *data2 = (float*)calloc(h_BRIRmetadata->noBands_transition,sizeof(float));
          for (k = 0; k < h_BRIRmetadata->noBands_transition; k++)
          {
            data2[k] = (float)h_BRIRmetadata->filterlength_fft[k];
            if (!IsLittleEndian())
            {
              ReverseFloat(data2[k]);
            }
          }
          nWritten += fwrite(data2,sizeof(float),h_BRIRmetadata->noBands_transition,fBRIRmetadata);
          free(data2); data2 = NULL;				
        }
      }
      break;
    case 12:
      {
        if (h_BRIRmetadata->noBands_transition > 0)
        {
          float *data2 = (float*)calloc(h_BRIRmetadata->noBands_transition,sizeof(float));
          for (k = 0; k < h_BRIRmetadata->noBands_transition; k++)
          {
            data2[k] = (float)h_BRIRmetadata->N_FFT[k];
            if (!IsLittleEndian())
            {
              ReverseFloat(data2[k]);
            }
          }
          nWritten += fwrite(data2,sizeof(float),h_BRIRmetadata->noBands_transition,fBRIRmetadata);
          free(data2); data2 = NULL;			
        }
      }
      break;
    case 13:
      {
        if (h_BRIRmetadata->noBands_transition > 0)
        {
          float *data2 = (float*)calloc(h_BRIRmetadata->noBands_transition,sizeof(float));
          for (k = 0; k < h_BRIRmetadata->noBands_transition; k++)
          {
            data2[k] = (float)h_BRIRmetadata->N_BLK[k];
            if (!IsLittleEndian())
            {
              ReverseFloat(data2[k]);
            }
          }
          nWritten += fwrite(data2,sizeof(float),h_BRIRmetadata->noBands_transition,fBRIRmetadata);
          free(data2); data2 = NULL;		
        }
      }
      break;
    case 14:
      {
        if (h_BRIRmetadata->noBands_transition > 0)
        {
          int band;
          for (band = 0; band<h_BRIRmetadata->noBands_transition; band++)
          {
            int blk;
            for (blk = 0; blk < h_BRIRmetadata->N_BLK[band]; blk++)
            {
              int chIn;
              for (chIn = 0; chIn < h_BRIRmetadata->channels; chIn++)
              {
                float *data2 = (float*)calloc(h_BRIRmetadata->N_FFT[band],sizeof(float));
                for (k = 0; k < h_BRIRmetadata->N_FFT[band]; k++)
                {
                  data2[k] = h_BRIRmetadata->FFTdomain_left_real[band][blk][chIn][k];
                  if (!IsLittleEndian())
                  {
                    ReverseFloat(data2[k]);
                  }
                }
                nWritten += fwrite(data2, sizeof(float), h_BRIRmetadata->N_FFT[band], fBRIRmetadata);
                free(data2); data2 = NULL;	
              }
            }									
          }
        }
      }
      break;
    case 15:
      {
        if (h_BRIRmetadata->noBands_transition > 0)
        {
          int band;
          for (band = 0; band<h_BRIRmetadata->noBands_transition; band++)
          {
            int blk;
            for (blk = 0; blk < h_BRIRmetadata->N_BLK[band]; blk++)
            {
              int chIn;
              for (chIn = 0; chIn < h_BRIRmetadata->channels; chIn++)
              {
                float *data2 = (float*)calloc(h_BRIRmetadata->N_FFT[band],sizeof(float));
                for (k = 0; k < h_BRIRmetadata->N_FFT[band]; k++)
                {
                  data2[k] = h_BRIRmetadata->FFTdomain_left_imag[band][blk][chIn][k];
                  if (!IsLittleEndian())
                  {
                    ReverseFloat(data2[k]);
                  }
                }
                nWritten += fwrite(data2, sizeof(float), h_BRIRmetadata->N_FFT[band], fBRIRmetadata);
                free(data2); data2 = NULL;	
              }
            }									
          }
        }
      }
      break;
    case 16:
      {
        if (h_BRIRmetadata->noBands_transition > 0)
        {
          int band;
          for (band = 0; band<h_BRIRmetadata->noBands_transition; band++)
          {
            int blk;
            for (blk = 0; blk < h_BRIRmetadata->N_BLK[band]; blk++)
            {
              int chIn;
              for (chIn = 0; chIn < h_BRIRmetadata->channels; chIn++)
              {
                float *data2 = (float*)calloc(h_BRIRmetadata->N_FFT[band],sizeof(float));
                for (k = 0; k < h_BRIRmetadata->N_FFT[band]; k++)
                {
                  data2[k] = h_BRIRmetadata->FFTdomain_right_real[band][blk][chIn][k];
                  if (!IsLittleEndian())
                  {
                    ReverseFloat(data2[k]);
                  }
                }
                nWritten += fwrite(data2, sizeof(float), h_BRIRmetadata->N_FFT[band], fBRIRmetadata);
                free(data2); data2 = NULL;	
              }
            }									
          }
        }
      }
      break;
    case 17:
      {
        if (h_BRIRmetadata->noBands_transition > 0)
        {
          int band;
          for (band = 0; band<h_BRIRmetadata->noBands_transition; band++)
          {
            int blk;
            for (blk = 0; blk < h_BRIRmetadata->N_BLK[band]; blk++)
            {
              int chIn;
              for (chIn = 0; chIn < h_BRIRmetadata->channels; chIn++)
              {
                float *data2 = (float*)calloc(h_BRIRmetadata->N_FFT[band],sizeof(float));
                for (k = 0; k < h_BRIRmetadata->N_FFT[band]; k++)
                {
                  data2[k] = h_BRIRmetadata->FFTdomain_right_imag[band][blk][chIn][k];
                  if (!IsLittleEndian())
                  {
                    ReverseFloat(data2[k]);
                  }
                }
                nWritten += fwrite(data2, sizeof(float), h_BRIRmetadata->N_FFT[band], fBRIRmetadata);
                free(data2); data2 = NULL;	
              }
            }									
          }
        }
      }
      break;
    case 18:
      {
        data = (float)h_BRIRmetadata->numband_conv;
        if (!IsLittleEndian())
        {
          ReverseFloat(data);
        }
        nWritten += fwrite(&data,sizeof(float),1,fBRIRmetadata);
      }
      break;
    case 19:
      {
        if (h_BRIRmetadata->numband_conv < h_BRIRmetadata->noBands_transition)
        {
          int band, chIn;
          int numband_TDL = h_BRIRmetadata->noBands_transition - h_BRIRmetadata->numband_conv;
          int num_Byte = numband_TDL * h_BRIRmetadata->channels;
          float *data2 = (float*)calloc(num_Byte,sizeof(float));
          for (band = 0; band < numband_TDL; band++)
          {
            for (chIn = 0; chIn < h_BRIRmetadata->channels; chIn++)
            {
              data2[band*h_BRIRmetadata->channels+chIn] = h_BRIRmetadata->TDL_gain_left_real[band][chIn];
              if (!IsLittleEndian())
              {
                ReverseFloat(data2[k]);
              }
            }
          }
          nWritten += fwrite(data2,sizeof(float), num_Byte, fBRIRmetadata);
          free(data2); data2 = NULL;		
        }
      }
      break;
    case 20:
      {
        if (h_BRIRmetadata->numband_conv < h_BRIRmetadata->noBands_transition)
        {
          int band, chIn;
          int numband_TDL = h_BRIRmetadata->noBands_transition - h_BRIRmetadata->numband_conv;
          int num_Byte = numband_TDL * h_BRIRmetadata->channels;
          float *data2 = (float*)calloc(num_Byte,sizeof(float));
          for (band = 0; band < numband_TDL; band++)
          {
            for (chIn = 0; chIn < h_BRIRmetadata->channels; chIn++)
            {
              data2[band*h_BRIRmetadata->channels+chIn] = h_BRIRmetadata->TDL_gain_left_imag[band][chIn];
              if (!IsLittleEndian())
              {
                ReverseFloat(data2[k]);
              }
            }
          }
          nWritten += fwrite(data2,sizeof(float), num_Byte, fBRIRmetadata);
          free(data2); data2 = NULL;		
        }
      }
      break;
    case 21:
      {
        if (h_BRIRmetadata->numband_conv < h_BRIRmetadata->noBands_transition)
        {
          int band, chIn;
          int numband_TDL = h_BRIRmetadata->noBands_transition - h_BRIRmetadata->numband_conv;
          int num_Byte = numband_TDL * h_BRIRmetadata->channels;
          float *data2 = (float*)calloc(num_Byte,sizeof(float));
          for (band = 0; band < numband_TDL; band++)
          {
            for (chIn = 0; chIn < h_BRIRmetadata->channels; chIn++)
            {
              data2[band*h_BRIRmetadata->channels+chIn] = h_BRIRmetadata->TDL_gain_right_real[band][chIn];
              if (!IsLittleEndian())
              {
                ReverseFloat(data2[k]);
              }
            }
          }
          nWritten += fwrite(data2,sizeof(float), num_Byte, fBRIRmetadata);
          free(data2); data2 = NULL;		
        }
      }
      break;
    case 22:
      {
        if (h_BRIRmetadata->numband_conv < h_BRIRmetadata->noBands_transition)
        {
          int band, chIn;
          int numband_TDL = h_BRIRmetadata->noBands_transition - h_BRIRmetadata->numband_conv;
          int num_Byte = numband_TDL * h_BRIRmetadata->channels;
          float *data2 = (float*)calloc(num_Byte,sizeof(float));
          for (band = 0; band < numband_TDL; band++)
          {
            for (chIn = 0; chIn < h_BRIRmetadata->channels; chIn++)
            {
              data2[band*h_BRIRmetadata->channels+chIn] = h_BRIRmetadata->TDL_gain_right_imag[band][chIn];
              if (!IsLittleEndian())
              {
                ReverseFloat(data2[k]);
              }
            }
          }
          nWritten += fwrite(data2,sizeof(float), num_Byte, fBRIRmetadata);
          free(data2); data2 = NULL;		
        }
      }
      break;
    case 23:
      {
        if (h_BRIRmetadata->numband_conv < h_BRIRmetadata->noBands_transition)
        {
          int band, chIn;
          int numband_TDL = h_BRIRmetadata->noBands_transition - h_BRIRmetadata->numband_conv;
          int num_Byte = numband_TDL * h_BRIRmetadata->channels;
          float *data2 = (float*)calloc(num_Byte,sizeof(float));
          for (band = 0; band < numband_TDL; band++)
          {
            for (chIn = 0; chIn < h_BRIRmetadata->channels; chIn++)
            {
              data2[band*h_BRIRmetadata->channels+chIn] = (float)(h_BRIRmetadata->TDL_delay_left[band][chIn]);
              if (!IsLittleEndian())
              {
                ReverseFloat(data2[k]);
              }
            }
          }
          nWritten += fwrite(data2,sizeof(float), num_Byte, fBRIRmetadata);
          free(data2); data2 = NULL;		
        }
      }
      break;
    case 24:
      {
        if (h_BRIRmetadata->numband_conv < h_BRIRmetadata->noBands_transition)
        {
          int band, chIn;
          int numband_TDL = h_BRIRmetadata->noBands_transition - h_BRIRmetadata->numband_conv;
          int num_Byte = numband_TDL * h_BRIRmetadata->channels;
          float *data2 = (float*)calloc(num_Byte,sizeof(float));
          for (band = 0; band < numband_TDL; band++)
          {
            for (chIn = 0; chIn < h_BRIRmetadata->channels; chIn++)
            {
              data2[band*h_BRIRmetadata->channels+chIn] = (float)(h_BRIRmetadata->TDL_delay_right[band][chIn]);
              if (!IsLittleEndian())
              {
                ReverseFloat(data2[k]);
              }
            }
          }
          nWritten += fwrite(data2,sizeof(float), num_Byte, fBRIRmetadata);
          free(data2); data2 = NULL;		
        }
      }
      break;
    case 25:
      {
        float fs = (h_BRIRmetadata->fs_BRIRs*1.0f);
        if (!IsLittleEndian())
        {
          ReverseFloat(fs);
        }
        nWritten += fwrite(&fs, sizeof(float),1,fBRIRmetadata);
      }
      break;
    }
  }
  fclose(fBRIRmetadata);

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

void binauralParam_linearfit(H_BRIRMETADATASTRUCT h_BRIRmetadata, int x_begin, int x_end, float* MixingTime)
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
    h_BRIRmetadata->filterlength_fft[k-1] = (int) pow(2.0f, (double)((int)(b*(float)k + a +.5f)));
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
