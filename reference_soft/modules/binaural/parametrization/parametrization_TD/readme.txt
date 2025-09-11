

--- TD Binaural Parameterization ---

This module allows to calculate the low-level parameters suitable for the TD renderer from a set of FIR filters.
It takes as input a binaural bitstream containing FIR binaural representations. 
It outputs a binaural bitstream containing the corresponding low-level binaural representations.

The TD binaural parameterization solution (binauralParametrizationCmdl_VS2012.sln) consists of the following 
projects:
* binauralParameterizationCmdl : commandline tool to execute the filter parametrization. This project has to be set as startup project.
* binauralParameterizationLib  : library that holds the analysis and parameterization functions
* tools for reading and writing binaural bitstreams, fft analysis, etc.

To build the parameterization command line tool, open and compile binauralParameterizationCmdl_VS2012.sln. 

The resulting commandline tool (binauralParametrizationTdCmdl.exe) takes 2 input arguments :
	-in   <name of the input bitstream, containing FIR representations>
	-out  <name of the output bitstream, containing low-level TD parameters>
			
Usage example : 
binauralParametrizationTdCmdl.exe -in in.bs -out out.bs