 Binaural Renderer 

--- Frequency-Domain Binaural Parameterization Module - Software Structure ---

The binaural parameterization module solution (binauralParametrizationCmdl.sln) consists of the following 
projects:
* binauralParameterizationCmdl: commandline tool to execute the analysis of the binaural room impulse 
responses. This project has to be set as startup project
* binauralParameterizationLib: library that holds the analysis and parameterization functions
* binauralInterfaceLib to read the bitstream syntax of the defined binaural interface containing the BRIR raw data
* tools for reading and writing wave files, fft analysis etc.

To use the binaural parameterization module the file binauralParameterizationCmdl should be built and 
executed. It is a commandline tool and needs the following commandline switch as input:
	-firbs <input path of the BRIR raw data bitstream (FIR data)>
	-fdbs  <output path of the BRIR parameter data bitstream (frequency domain low-level data)>
	-kconv <Number of bands used for convolution> (optional, default value: 32) 
	-kmax  <Number of processed bands via binaural renderer> (optional, default value: 48) 

example usage:	
binauralParametrizationCmdl.exe -firbs in.bs -fdbs out.bs
