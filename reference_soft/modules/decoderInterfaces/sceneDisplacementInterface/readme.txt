# Decoder interface for scene displacement data

This module implements the decoder scene displacement interface (SDI).
This folder contains:

- Scene displacement interface library `sceneDisplacementInterfaceLib`
- Scene displacement interface example application `sceneDisplacementInterfaceExample`

## Generating interface streams

SDI bitstreams can be generated with the `sceneDisplacementInterfaceExample` command line application. The application reads a text file and writes a SDI stream.

    sceneDisplacementInterfaceExample -sd SDI.txt -of SDI.bs
      -sd    -    path to SDI input text file
      -of    -    path to SDI bitstream

The bitstream format is defined in ISO/IEC 23008-3: "Syntax of mpegh3daSceneDisplacementData()". The bitstream format of positional scene displacement interface (PSDI) data is defined in ISO/IEC 23008-3: "Syntax of mpegh3daPositionalSceneDisplacementData()".

### SDI input file format

The input text shall contain SDI paramaters as comma-separated values. The first line in the input file indicates the CSV format.

To write both rotational (`mpegh3daSceneDisplacementData`) and positional (`mpeg3daPositionalSceneDisplacementData`) SDI data, each SDI parameter set in the input file shall be formatted:

    yaw,pitch,roll,azimuth,elevation,radius

To write rotational SDI data only, each SDI parameter set shall be formatted:

    yaw,pitch,roll

All angles are in degrees. Example files are located at:

    sceneDisplacementInterfaceExample/make/testSD.txt
    sceneDisplacementInterfaceExample/make/testPSDI.txt

### Invocation

Generate a SDI stream file with both rotational and positional scene displacement data:

    sceneDisplacementInterfaceExample -sd /path/to/testPSDI.txt -of /path/to/sdi.bs

The resulting file `sdi.bs` can now be used with the `-sd2` switch of the MPEG-H 3D audio decoder (`3DAudioDecoder`).
