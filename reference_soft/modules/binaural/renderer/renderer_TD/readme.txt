

--- TD Binaural Renderer Module - Software Structure ---

The TD binaural renderer module solution (binauralRendererCmdl_VS2012.sln) consists of the following projects:
* binauralRendererCmdl : commandline tool to perform binaural rendering
* binauralRendererLib  : library that holds the binauralization functions 
* tools for reading/writing wave files and binaural bitstreams ... 

To build the renderer command line tool, open and compile binauralRendererCmdl_VS2012.sln. 

The resulting commandline tool (binauralRendererTdCmdl.exe) has following arguments :
	-if         <in  : input wavfile>                               (required);
	-h2b        <in  : use H2B filters>                             (optional) OR
	-cicp	    <in  : input CICP index> e.g., 5.1->6  22.2->13     (optional) OR
	-geo        <in  : path to geometric information file>          (optional);
	-bs         <in  : binaural bitstream (BRIR parameters)>        (required);
	-delay      <in  : apply BRIR begin delay, 1:yes, 0:no>         (optional, 0 by default);
	-of         <out : output binaural soundfile>                   (required);

The binaural bitstream must contain filters as low-level parameters. 
They can be virtual loudspeakers (VL) or HOA to binaural (H2B) filters.

The spatial configuration of the input wavfile must be specidfied :
	-h2b  1      : input wavfile contains HOA components                     => use H2B filters
	-cicp index  : input wavfile is described by CICP index                  => use VL filters 
	-geo         : input wavfile configuration is described in the geo file  => use VL filters 

Usage ex1: 
binauralRendererTdCmdl.exe -if in24chan.wav -cicp 13 -of outBinau.wav -bs BRIR_vl.bs -delay 1
	
Usage ex2: 
binauralRendererTdCmdl.exe -if inHoa.wav -h2b 1 -of outBinau.wav -bs BRIR_h2b.bs

Usage ex3: 
binauralRendererTdCmdl.exe -if in.wav -geo geofile.txt -of outBinau.wav -bs BRIR_vl.bs
	


