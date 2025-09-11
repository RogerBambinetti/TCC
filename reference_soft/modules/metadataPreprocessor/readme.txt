

--- Metadata Preprocessor Module - Software Structure ---

This module provides functionality for processing of element interaction data (user interaction with object-based content).

The module consists of a commandline tool that illustrates the writing and reading functionality and a library that holds the 
corresponding funtions. The solution also includes the modules for the local setup information interface as well as the element
interaction interface and additional libraries for reading, writing etc.

To build the command line tool, open and compile metadataPreprocessorCmdl.sln. 


The resulting commandline tool has following arguments :

	-mp4 	<path to an mp4 file> /* Required */
	-ei		<path to an element interaction interface bitstream file> /* Required */
	-ls		<path to a local setup information interface bistream file> /* Required */
	-cfg	<path to a text file containing the paths for the needed executables> /* Required */
	-dmxId 	downmixID to identify target layout /* Optional */
    -sd 	path to scene displacement 'bitstream' file /* Optional */
    -objOut enable object output interface /* Optional */

The commandline tool reads and decodes an .mp4 file, it writes a .wav file (or .qmf file, depending on the configuration of the mp4 file)  plus an .oam file.
The local loudspeaker/binaural setup is read from the local setup information interface bistream file.

The audio framelength is set to the oam granularity of the oam data in the .mp4 file.
Once per audio frame element interaction data are read from the element interaction interface bitstream file.

This user interaction and processing is applied in the metadata preprocessor:
- on/off interaction
- position and gain interaction
- screen-related remapping and zooming
- closest speaker playout processing
- divergence processing
- diffuseness processing
- excluded sectors processing
- scene-displacement processing

The modified gains and positions are included in the output .oam file.
The modified gain for elements without oam data (channel-based elements) is directly applied before saving the corresponding data to the output wave/qmf file.

Usage example: 

-mp4 ..\inputData\mp4\test.mp4 -ls ..\inputData\ls\localSetup.bs -cfg ..\inputData\binaries_Win32_Debug.cfg -ei ..\inputData\ei\elementInteraction.bs


	


