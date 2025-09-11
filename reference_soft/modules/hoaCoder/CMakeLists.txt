cmake_minimum_required(VERSION 3.9.2)

###############################################################################
# project name
###############################################################################
project(hoaCoder)

###############################################################################
# target sources (libraries)
###############################################################################
# bitStreamCommonLib
add_library(bitStreamCommonLib
  bitstream/bitStreamIO/bitStreamCommon/include/basicbitstreamfields.h
  bitstream/bitStreamIO/bitStreamCommon/include/cbitbuffer.h
  bitstream/bitStreamIO/bitStreamCommon/include/ibitstreamfield.h
  bitstream/bitStreamIO/bitStreamCommon/src/cbitbuffer.cpp
)
target_include_directories(bitStreamCommonLib PUBLIC "${PROJECT_SOURCE_DIR}/bitstream/bitStreamIO/bitStreamCommon/include")
set_target_properties(bitStreamCommonLib PROPERTIES FOLDER "Libraries/HOA")

# bitStreamReaderLib
add_library(bitStreamReaderLib
  bitstream/bitStreamIO/bitStreamReader/include/basicbitstreamfieldsread.h
  bitstream/bitStreamIO/bitStreamReader/include/cbitstreamreader.h
  bitstream/bitStreamIO/bitStreamReader/include/ibitstreamfieldread.h
  bitstream/bitStreamIO/bitStreamReader/src/cbitstreamreader.cpp
)
target_include_directories(bitStreamReaderLib PUBLIC "${PROJECT_SOURCE_DIR}/bitstream/bitStreamIO/bitStreamReader/include")
target_link_libraries(bitStreamReaderLib bitStreamCommonLib)
set_target_properties(bitStreamReaderLib PROPERTIES FOLDER "Libraries/HOA")

# bitStreamWriterLib
add_library(bitStreamWriterLib
  bitstream/bitStreamIO/bitStreamWriter/include/basicbitstreamfieldswrite.h
  bitstream/bitStreamIO/bitStreamWriter/include/cbitstreamwriter.h
  bitstream/bitStreamIO/bitStreamWriter/include/ibitstreamfieldwrite.h
  bitstream/bitStreamIO/bitStreamWriter/src/cbitstreamwriter.cpp
)
target_include_directories(bitStreamWriterLib PUBLIC "${PROJECT_SOURCE_DIR}/bitstream/bitStreamIO/bitStreamWriter/include")
target_link_libraries(bitStreamWriterLib bitStreamCommonLib)
set_target_properties(bitStreamWriterLib PROPERTIES FOLDER "Libraries/HOA")

# CHoaBitStreamC
add_library(CHoaBitStreamC
  bitstream/hoaBitStream/hoaBitStreamCommon/include/choabitstream.h
  bitstream/hoaBitStream/hoaBitStreamCommon/include/hoaHuffmanTables.h
  bitstream/hoaBitStream/hoaBitStreamCommon/include/stdHoaFrame.h
  bitstream/hoaBitStream/hoaBitStreamCommon/include/TabulatedValuesHoaBitstream.h
  bitstream/hoaBitStream/hoaBitStreamCommon/include/vectorBasedPreDomSoundHuffman_decode.h
  bitstream/hoaBitStream/hoaBitStreamCommon/src/TabulatedValuesHoaBitstream.cpp
  bitstream/hoaBitStream/hoaBitStreamCommon/src/vectorBasedPreDomSoundHuffman_decode.cpp
)
target_include_directories(CHoaBitStreamC PUBLIC "${PROJECT_SOURCE_DIR}/bitstream/hoaBitStream/hoaBitStreamCommon/include")
set_target_properties(CHoaBitStreamC PROPERTIES FOLDER "Libraries/HOA")

# CHoaBitStreamR
add_library(CHoaBitStreamR
  bitstream/hoaBitStream/hoaBitStreamReader/include/choabitstreamr.h
  bitstream/hoaBitStream/hoaBitStreamReader/include/HoaConfigR.h
  bitstream/hoaBitStream/hoaBitStreamReader/include/HoaFrameR.h
  bitstream/hoaBitStream/hoaBitStreamReader/src/choabitstreamr.cpp
  bitstream/hoaBitStream/hoaBitStreamReader/src/HoaConfigR.cpp
  bitstream/hoaBitStream/hoaBitStreamReader/src/HoaFrameR.cpp
)
target_include_directories(CHoaBitStreamR PUBLIC "${PROJECT_SOURCE_DIR}/bitstream/hoaBitStream/hoaBitStreamReader/include")
target_link_libraries(CHoaBitStreamR CHoaBitStreamC bitStreamReaderLib HoaCommonLib)
set_target_properties(CHoaBitStreamR PROPERTIES FOLDER "Libraries/HOA")

# CHoaBitStreamW
add_library(CHoaBitStreamW
  bitstream/hoaBitStream/hoaBitStreamWriter/include/choabitstreamw.h
  bitstream/hoaBitStream/hoaBitStreamWriter/include/HoaConfigW.h
  bitstream/hoaBitStream/hoaBitStreamWriter/include/HoaFrameW.h
  bitstream/hoaBitStream/hoaBitStreamWriter/src/choabitstreamw.cpp
  bitstream/hoaBitStream/hoaBitStreamWriter/src/HoaConfigW.cpp
  bitstream/hoaBitStream/hoaBitStreamWriter/src/HoaFrameW.cpp
)
target_include_directories(CHoaBitStreamW PUBLIC "${PROJECT_SOURCE_DIR}/bitstream/hoaBitStream/hoaBitStreamWriter/include")
target_link_libraries(CHoaBitStreamW CHoaBitStreamC bitStreamWriterLib HoaCommonLib)
set_target_properties(CHoaBitStreamW PROPERTIES FOLDER "Libraries/HOA")

# HoaCommonLib
add_library(HoaCommonLib
  common/include/DataTypes.h
  common/include/FrameParams.h
  common/include/integerFunctions.h
  common/include/sHmodeMtrx.h
  common/include/SimpleMtrx.h
  common/include/sphericalHarmonic.h
  common/include/TabulatedValues.h
  common/src/FrameParams.cpp
  common/src/sHmodeMtrx.cpp
  common/src/SimpleMtrx.cpp
  common/src/sphericalHarmonic.cpp
  common/src/TabulatedValues.cpp
)
target_include_directories(HoaCommonLib PUBLIC "${PROJECT_SOURCE_DIR}/common/include")
set_target_properties(HoaCommonLib PROPERTIES FOLDER "Libraries/HOA")

# HoaDecoderLib
add_library(HoaDecoderLib
  hoaDecLib/include/HoaDecoder.h
  hoaDecLib/src/HoaDecoder.cpp
)
target_include_directories(HoaDecoderLib PUBLIC "${PROJECT_SOURCE_DIR}/hoaDecLib/include")
target_link_libraries(HoaDecoderLib HoaCommonLib CHoaBitStreamR renderHoaLib SpatialDecodingLib wavReader)
set_target_properties(HoaDecoderLib PROPERTIES FOLDER "Libraries/HOA")

# HoaEncoderLib
add_library(HoaEncoderLib
  hoaEncLib/include/HoaEncoder.h
  hoaEncLib/src/HoaEncoder.cpp
)
target_include_directories(HoaEncoderLib PUBLIC "${PROJECT_SOURCE_DIR}/hoaEncLib/include")
target_link_libraries(HoaEncoderLib HoaCommonLib CHoaBitStreamW CHoaBitStreamC SpatialEncodingLib wavWriter)
set_target_properties(HoaEncoderLib PROPERTIES FOLDER "Libraries/HOA")

# renderHoaLib
add_library(renderHoaLib
  hoaRenderer/DRC1/DRC1hoa.h
  hoaRenderer/DRC1/dshtDrcPositions.h
  hoaRenderer/DRC1/dshtTc2sMtrx.h
  hoaRenderer/DRC1/dshtTs2cMtrx.h
  hoaRenderer/hoaNfc/hoaNfcFiltering.h
  hoaRenderer/hoaRenderer/hoaRenderer.h
  hoaRenderer/HOArenderMtrx/HOArenderMtrx.h
  hoaRenderer/HOArenderMtrx/modeMtrxRom.h
  hoaRenderer/robustPan/robustPan.h
  hoaRenderer/rotation/dshtRotationPositions.h
  hoaRenderer/rotation/hoaRotation.h
  hoaRenderer/scrnAdapt/dshtScrnAdaptPositions.h
  hoaRenderer/scrnAdapt/scrnAdapt.h
  hoaRenderer/spacePositions/spacePositions.h
  hoaRenderer/DRC1/DRC1hoa.cpp
  hoaRenderer/DRC1/dshtTc2sMtrx.cpp
  hoaRenderer/DRC1/dshtTs2cMtrx.cpp
  hoaRenderer/hoaNfc/hoaNfcFiltering.cpp
  hoaRenderer/hoaRenderer/hoaRenderer.cpp
  hoaRenderer/HOArenderMtrx/HOArenderMtrx.cpp
  hoaRenderer/rotation/hoaRotation.cpp
  hoaRenderer/robustPan/robustPan.cpp
  hoaRenderer/scrnAdapt/scrnAdapt.cpp
  hoaRenderer/spacePositions/spacePositions.cpp
)
target_include_directories(renderHoaLib PUBLIC "${PROJECT_SOURCE_DIR}/hoaRenderer/DRC1"
                                        PUBLIC "${PROJECT_SOURCE_DIR}/hoaRenderer/hoaRenderer"
                                        PUBLIC "${PROJECT_SOURCE_DIR}/hoaRenderer/rotation"
                                       PRIVATE "${PROJECT_SOURCE_DIR}/hoaRenderer/hoaNfc"
                                       PRIVATE "${PROJECT_SOURCE_DIR}/hoaRenderer/HOArenderMtrx"
                                       PRIVATE "${PROJECT_SOURCE_DIR}/hoaRenderer/robustPan"
                                       PRIVATE "${PROJECT_SOURCE_DIR}/hoaRenderer/scrnAdapt"
                                       PRIVATE "${PROJECT_SOURCE_DIR}/hoaRenderer/spacePositions"
)
target_link_libraries(renderHoaLib HoaCommonLib qmflib_3da qmflibCppWrap)
set_target_properties(renderHoaLib PROPERTIES FOLDER "Libraries/HOA")

# SpatialCommonLib
add_library(SpatialCommonLib
  spatialCoder/common/include/CommonFunctions.h
  spatialCoder/common/include/DirBasedPreDomSoundSynthesis.h
  spatialCoder/common/include/PreDomSoundSynthesis.h
  spatialCoder/common/include/VectorBasedPredomSoundSynthesis.h
  spatialCoder/common/src/DirBasedPreDomSoundSynthesis.cpp
  spatialCoder/common/src/PreDomSoundSynthesis.cpp
  spatialCoder/common/src/VectorBasedPredomSoundSynthesis.cpp
)
target_include_directories(SpatialCommonLib PUBLIC "${PROJECT_SOURCE_DIR}/spatialCoder/common/include")
target_link_libraries(SpatialCommonLib HoaCommonLib)
set_target_properties(SpatialCommonLib PROPERTIES FOLDER "Libraries/HOA")

# SpatialDecodingLib
add_library(SpatialDecodingLib
  spatialCoder/spatialDecoding/include/AmbienceSynthesis.h
  spatialCoder/spatialDecoding/include/ChannelReAssignment.h
  spatialCoder/spatialDecoding/include/DirSubBandHOASynthesis.h
  spatialCoder/spatialDecoding/include/FinalHOAComposition.h
  spatialCoder/spatialDecoding/include/InverseDynCorrection.h
  spatialCoder/spatialDecoding/include/PARDecoder.h
  spatialCoder/spatialDecoding/include/PreliminaryHOAComposition.h
  spatialCoder/spatialDecoding/include/SpatialDecoder.h
  spatialCoder/spatialDecoding/include/SubBandDirSigsSynthesis.h
  spatialCoder/spatialDecoding/src/AmbienceSynthesis.cpp
  spatialCoder/spatialDecoding/src/ChannelReAssignment.cpp
  spatialCoder/spatialDecoding/src/DirSubBandHOASynthesis.cpp
  spatialCoder/spatialDecoding/src/FinalHOAComposition.cpp
  spatialCoder/spatialDecoding/src/InverseDynCorrection.cpp
  spatialCoder/spatialDecoding/src/PARDecoder.cpp
  spatialCoder/spatialDecoding/src/PreliminaryHOAComposition.cpp
  spatialCoder/spatialDecoding/src/SpatialDecoder.cpp
  spatialCoder/spatialDecoding/src/SubBandDirSigsSynthesis.cpp
)
target_include_directories(SpatialDecodingLib PUBLIC "${PROJECT_SOURCE_DIR}/spatialCoder/spatialDecoding/include")
target_link_libraries(SpatialDecodingLib SpatialCommonLib HoaCommonLib CHoaBitStreamC bitStreamCommonLib bitStreamReaderLib parDecorrelator fftlib_3da qmflib_3da qmflibCppWrap)
set_target_properties(SpatialDecodingLib PROPERTIES FOLDER "Libraries/HOA")

# SpatialEncodingLib
add_library(SpatialEncodingLib
  spatialCoder/spatialEncoding/include/AmbientComponentModification.h
  spatialCoder/spatialEncoding/include/AssignmentInfo.h
  spatialCoder/spatialEncoding/include/ChannelAssignment.h
  spatialCoder/spatialEncoding/include/DirectionAndVectorEstimation.h
  spatialCoder/spatialEncoding/include/DynCorrection.h
  spatialCoder/spatialEncoding/include/HOADecomposition.h
  spatialCoder/spatialEncoding/include/PAREncoder.h
  spatialCoder/spatialEncoding/include/SpatialEncoder.h
  spatialCoder/spatialEncoding/include/SubbandDirEstimation.h
  spatialCoder/spatialEncoding/include/SubbandDirSigsPrediction.h
  spatialCoder/spatialEncoding/src/AmbientComponentModification.cpp
  spatialCoder/spatialEncoding/src/ChannelAssignment.cpp
  spatialCoder/spatialEncoding/src/DirectionAndVectorEstimation.cpp
  spatialCoder/spatialEncoding/src/DynCorrection.cpp
  spatialCoder/spatialEncoding/src/HOADecomposition.cpp
  spatialCoder/spatialEncoding/src/PAREncoder.cpp
  spatialCoder/spatialEncoding/src/SpatialEncoder.cpp
  spatialCoder/spatialEncoding/src/SubbandDirEstimation.cpp
  spatialCoder/spatialEncoding/src/SubbandDirSigsPrediction.cpp
)
target_include_directories(SpatialEncodingLib PUBLIC "${PROJECT_SOURCE_DIR}/spatialCoder/spatialEncoding/include")
target_link_libraries(SpatialEncodingLib SpatialCommonLib SpatialDecodingLib HoaCommonLib CHoaBitStreamC bitStreamCommonLib bitStreamWriterLib parDecorrelator fftlib_3da qmflib_3da qmflibCppWrap)
set_target_properties(SpatialEncodingLib PROPERTIES FOLDER "Libraries/HOA")

###############################################################################
# target sources (executables)
###############################################################################
# hoaReferenceDecoder
add_executable(hoaReferenceDecoder
  hoaDec/src/main.cpp
)
# increase stack size to prevent stack overflow
target_link_options(
    hoaReferenceDecoder
    PRIVATE
        $<$<C_COMPILER_ID:MSVC>:/STACK:15000000>
    )
target_link_libraries(hoaReferenceDecoder HoaCommonLib HoaDecoderLib wavWriter)
set_target_properties(hoaReferenceDecoder PROPERTIES FOLDER "Executables/HOA")
install(TARGETS hoaReferenceDecoder DESTINATION bin)

# hoaReferenceEncoder
add_executable(hoaReferenceEncoder
  hoaEnc/src/main.cpp
)
target_link_libraries(hoaReferenceEncoder HoaCommonLib HoaEncoderLib wavReader)
set_target_properties(hoaReferenceEncoder PROPERTIES FOLDER "Executables/HOA")
install(TARGETS hoaReferenceEncoder DESTINATION bin)
