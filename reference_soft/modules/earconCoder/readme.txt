ISO/IEC 23008-3 MPEG-H 3D Audio earcon Codec
===============================================================================

1. General
-------------------------------------------------------------------------------
The earcon Codec implements  writing and reading of mahs streams containing the
MHAS packets of type:
 * PACTYP_EARCON
 * PACTYP_PCMCONFIG
 * PACTYP_PCMDATA
 
2. Usage
-------------------------------------------------------------------------------

2.1 terminology
-------------------------------------------------------------------------------
<s>: string, usually containing  a file path, e.g. "input.wav"
<f>: floating point value, e.g. "1.0"

2.2 encoder
-------------------------------------------------------------------------------
The earcon encoder is called the following way:
earconEncoder[.exe] -if <s> -of <s> -loudness <f>

2.3 decoder
-------------------------------------------------------------------------------
The earcon decoder is called the following way:
earconDecoder[.exe] -if <s> -of <s> -bs <s>

The option "-bs" in the earconDecoder is optional.

3. Syntax of the bit-stream (BS) file
-------------------------------------------------------------------------------

When calling the earconDecoder with the option "-bs" the decoder outputs an
additional bit-stream (BS) file containing the parsed earcon meta-data payload
from the MHAS stream.

The data is stored as 32 bit data for each and every syntax element.
Most of the data is stored as 32 bit unsigned integer except of the raw PCM
data which is stored as 32 bit floating point data (float).

The following gives a translation of the earcon syntax as defined in
ISO/IEC 23008-3 to the BS syntax.

3.1 Syntax of pcmDataPayload() as defined in ISO/IEC 23008-3
-------------------------------------------------------------------------------
Syntax                                                 No. of bits    Mnemonic

pcmDataPayload()
{
  numPcmSignalsInFrame;                                          7    uimsbf
  for (i = 0; i < numPcmSignalsInFrame + 1; i++) {
    pcmSignal_ID[i];                                             7    uimsbf
  }
  if (pcmHasAttenuationGain == 2) {
    bsPcmAttenuationGain;                                        8    uimsbf
  }
  if (pcmFrameSizeIndex == 6) {
    pcmVarFrameSize;                                            16    uimsbf
  }
  for (i = 0; i < pcmFrameSize; i++) {
    for (j = 0; j < numPcmSignalsInFrame + 1; j++) {
      pcmSample;                                     /* NOTE */ nBits tcimsbf
    }
  }
}

NOTE: nBits = pcmBitsPerSample

3.1 Syntax of pcmDataPayload() as in the BS file
-------------------------------------------------------------------------------
Syntax                                                 No. of bits    Mnemonic

pcmDataPayload()
{
  numPcmSignalsInFrame;                                         32    uint32
  for (i = 0; i < numPcmSignalsInFrame + 1; i++) {
    pcmSignal_ID[i];                                            32    uint32
  }
  if (pcmHasAttenuationGain == 2) {
    bsPcmAttenuationGain;                                       32    uint32
  }
  if (pcmFrameSizeIndex == 6) {
    pcmVarFrameSize;                                            32    uint32
  }
  for (i = 0; i < pcmFrameSize; i++) {
    for (j = 0; j < numPcmSignalsInFrame + 1; j++) {
      pcmSample;                                                32    float
    }
  }
}

uint32: Basic unsigned integer type. Contains at least the [0, 65,535] range;
float:  Real floating-point type, usually referred to as a single-precision
        floating-point type. Actual properties unspecified (except minimum
        limits), however on most systems this is the IEEE 754 single-precision
        binary floating-point format (32 bits). This format is required by the
        optional Annex F "IEC 60559 floating-point arithmetic".
