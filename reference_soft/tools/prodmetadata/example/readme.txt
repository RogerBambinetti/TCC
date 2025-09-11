Production Metadata Coding Tool
_______________________________________________________________________________

1. General
_______________________________________________________________________________
The Production Metadata (PMD) Coding Tool is a tool capable of writing and
parsing of data blobs reflecting PMD payload.

The tool requires several input files configuring the tool:

-ipmc: input PMD configuration file
-iomf: input object metadata file
-opmc: output PMD configuration file 
-ccfg: channel group configuration
-ocfg: object group configuration

2. Input format
_______________________________________________________________________________
2.1 ipmc
_______________________________________________________________________________
The input PMD configuration file contains in an ASCII coded manner the
sequential payload elements as in prodMetadataConfig().
Each element has to be in one line, e.g:
"
1
7.7
1
1
0
0
1
"

The file name of frame payload as defined by prodMetadataFrame() is derived by
the argument of ipmc by removing the file extension and adding _1.txt, e.g.:
-ipmc pmd.txt -> frame data is searched in pmd_1.txt, pmd_2.txt, ...

The integer defines the frame number.
The content of the frame data is ASCII coded an contains the sequential payload
elements as in prodMetadataFrame(), e.g.:
"
1
1
42.1
1
23.5
"

2.2 iomf
_______________________________________________________________________________
The input object metadata file contains in an ASCII coded manner the
sequential occurrence of has_object_metadata as given by the
dynamic_object_metadata() structure in ISO/IEC 23008-3:2019 Table 142.
Each element has to be in one line according to the occurrence of each object of
each object group, e.g:
"
1
0
1
0
"

2.3 ccfg
_______________________________________________________________________________
The channel group configuration is a single unsigned integer defining the number
of channel groups present in the signal.

2.4 ocfg
_______________________________________________________________________________
The object group configuration is a list of unsigned integers defining the
number of object groups followed by the number of objects per group, e.g.:
3 2 1 0
means:
numObjectGroups = 3
1st group num_objects = 2
2nd group num_objects = 1
3rd group num_objects = 0

2.5 cocfg
_______________________________________________________________________________
When calling the PMD encoder within the 3D Audio encoder framework the
3DAudioEncoder requires the option -cocfg (channel/object configuration).
This argument takes an ASCII coded .txt file combining ccfg and ocfg in the
following manner.
1st line shall contain ccfg, 2nd line chall contain ocfg, e.g.
"
2
3 2 1 0
"
meaning:
numChannelGroups = 2
numObjectGroups = 3
1st group num_objects = 2
2nd group num_objects = 1
3rd group num_objects = 0
