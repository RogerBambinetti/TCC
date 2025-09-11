

--- Element Interaction Interface Module - Software Structure ---

This module provides reading functionality of files containing bitstream in the element interaction interface syntax.
Besides, this module provides the possibility to generate such bitstream files from a dedicated .txt file.

The module consists of a commandline tool that illustrates the writing and reading functionality and a library that holds the 
corresponding funtions.


To build the command line tool, open and compile elementInteractionInterfaceExample.sln. 


The resulting commandline tool has following arguments :
	-of         <path of the bitstream that should be written> 
	-ei        	<path of the txt file that hold the interaction data that should be converted to bistream>   
	-sg	    	<string that holds the signature of the interaction>

The commandline tool reads the txt file, and writes framewise bistream in the element interaction interface syntax to a bitstream file.
This file is then read again by the commandline tool to show the reading functionality.

The txt file has to have the following form:
  *     line 1:   number of frames
  *     line 2:   number of groups : GroupIDs (comma separated)
  *     line 3 and ongoing:
  *       for each frame:
  *       line N:     interactionMode, groupPresetID (groupPresetID not needed if interactionMode == 0)
  *       line N+1:   hasZoomArea, Az, AzCenter, El, ElCenter (Az, AzCenter, El, ElCenter not needed if hasZoomArea == 0)
  *       line N+2:   onOffStatus for all groups (comma separated, order as groupIDs above, to be skipped in case interactionMode == 1)
  *       line N+3:   azOffset for all groups (comma separated, order as groupIDs above)
  *       line N+4:   elOffset for all groups (comma separated, order as groupIDs above)
  *       line N+5:   distFactor for all groups (comma separated, order as groupIDs above)
  *       line N+6:   gain_dB for all groups (comma separated, order as groupIDs above)
  *       line N+7:   routeToWireID for all groups (comma separated, order as groupIDs above, -1 if not routed to WIRE)
  
An example file is included.
  

Usage example: 
-of elementInteraction.bs -ei elementInteractionText.txt -sg interactionSignatureString
	


