

--- Dynamic Object Priority Generator ---

This module allows to generate the dynamic object priority values.

It works in the following steps:
1.	The input to the module are object signals (wave files) and OAM file.
2.	The module generates a priority value for each frame/object.
3.	The output from the module is an OAM file that includes generated priority values. (The other data in the OAM file remain
unchanged.)

The dynamic object priority generator solution (DynamicObjectPriorityGeneratorCmdl.sln) consists of the following
projects:
* DynamicObjectPriorityGeneratorCmdl : commandline tool to generate priority values. This project has to be set as startup project.
* DynamicObjectPriorityGeneratorlib  : library that holds the functions to generate priority values
* tool for reading wave files.

To build the dynamic object priority generator command line tool, open and compile DynamicObjectPriorityGeneratorCmdl.sln.

The resulting commandline tool (DynamicObjectPriorityGeneratorCmdl.exe) takes 4 input arguments :
	-if_wav   <name of the input object wave files without wav extension>
	-if_oam   <name of the input OAM file>
	-of_oam   <name of the output OAM file>
	-bs       <frame size for the dynamic object priority generation>

Usage example :
DynamicObjectPriorityGeneratorCmdl.exe -if_wav in -if_oam in.oam -of_oam out.oam -bs 2048
