
--- Binaural Interface Lib & Bitstream Writer ---

This module contains the library that allows to read and write a binaural bitstream following to the MPEG-H 3D Audio standard. 

Additionally, the Bitstream Writer command line tool allows to write a FIR binaural bitstream from a set of BRIR wavfiles.

The Bitstream Writer solution (bitstreamWriterCmdl.sln) consists of the following 
projects:
* bitstreamWriterCmdl  : commandline tool to create the binaural bitstream. This project has to be set as startup project.
* binauralInterfaceLib : library that holds the functions to read/write binaural bitstreams
* tools for reading wavfiles, bitbuffers, cicp geometry, etc.

To build the Bitstream Writer command line tool, open and compile binauralParameterizationCmdl.sln. 

The resulting commandline tool (bitstreamWriterCmdl.exe) uses these input arguments :
	-wavprefix  <in  : path to input brir wavfiles (with prefix)> (required)
			       Each input BRIR must be stored as stereo wavfile, with approriate suffix for indicating the position
			       (e.g., A+000_E+00.wav, or LFE1.wav and LFE2.wav for LFEs)
	-hoaOrder   <in  : HOA order>       (optional)
	-cicp       <in  : CICP index>      (optional) 
			       Index of the CICP configuration describing the BRIR layout, e.g., 5.1->6  22.2->13
	-geo        <in  : path to geometric information file>       (optional)
	-bs         <out : output file name> (required)
			       Name of the output binaural bitstream. 
	-bsSig      <out : brir bitstream signature> (optional, default value: 0)		
	-bsVer      <out : brir bitstream version>   (optional, default value: 0)

The BRIR configuration can be HOA (-hoaOrder) CICP (-cicp) or defined by a geometry file (-geo). One of them must be specified.	
	
Usage ex1:
In this example, the folder BRIR must contain 24 wavfiles (IIS_BRIR_A+000_E+00.wav ... IIS_BRIR_LFE2.wav)
The output binaural bitstream contains a FIR representation of input BRIRs, for the given CICP configuration.
bitstreamWriterCmdl.exe -wavprefix ../BRIRdata/IIS_BRIR_ -cicp 13 -bs IIS_BRIR_CICP13.bs
	
Usage ex2:
In this example, the folder BRIR must contain 49 wavfiles (IIS_BRIR_6_00+.wav ... IIS_BRIR_6_66+.wav)
The output binaural bitstream contains a FIR representation of input BRIRs, for the given HOA order.
bitstreamWriterCmdl.exe -wavprefix ../BRIRdata/IIS_BRIR_ -hoaOrder 6 -bs IIS_BRIR_hoaOrder6.bs -bsSig 1789 -bsVer 1