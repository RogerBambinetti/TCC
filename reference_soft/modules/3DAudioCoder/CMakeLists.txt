cmake_minimum_required(VERSION 3.9.2)

###############################################################################
# project name
###############################################################################
project(3DAudioCoder)

###############################################################################
# target sources (executables)
###############################################################################

if(UNIX AND NOT APPLE)
  set(ADD_MATH_LIB m)
endif()

# 3DAudioDecoder
add_executable(3DAudioDecoder
  3DAudioDecoder/src/elementMetadataPreprocessorPath.h
  3DAudioDecoder/src/fdBinauralPath.h
  3DAudioDecoder/src/initRenderPaths.h
  3DAudioDecoder/src/profileLevelHandling.h
  3DAudioDecoder/src/tdBinauralPath.h
  3DAudioDecoder/src/decode_types.h
  3DAudioDecoder/src/channelPath.c
  3DAudioDecoder/src/decode.c
  3DAudioDecoder/src/elementMetadataPreprocessorPath.c
  3DAudioDecoder/src/fdBinauralPath.c
  3DAudioDecoder/src/hoaPath.c
  3DAudioDecoder/src/initRenderPaths.c
  3DAudioDecoder/src/objectPath.c
  3DAudioDecoder/src/profileLevelHandling.c
  3DAudioDecoder/src/saocPath.c
  3DAudioDecoder/src/tdBinauralPath.c
)
target_compile_definitions(3DAudioDecoder PRIVATE SAOC_WD1_REFERENCE_LAYOUT_3D _CRT_SECURE_NO_WARNINGS)
target_include_directories(3DAudioDecoder PRIVATE "${PROJECT_SOURCE_DIR}/3DAudioDecoder/src")
target_link_libraries(3DAudioDecoder
  ascParserLib
  binauralInterfaceLib
  cicp2geometryLib
  elementInteractionInterfaceLib
  formatConverterLib
  libisomediafile
  libtsplite
  localSetupInformationInterfaceLib
  metadataPreprocessorLib
  mpegh_in_isom
  oamEncoderLib
  qmflib_3da
  readonlybitbuf_3da
  saocDecoderLib
  sceneDisplacementInterfaceLib
  wavIO_3da
  writeonlybitbuf_3da
  ${ADD_MATH_LIB}
)
set_target_properties(3DAudioDecoder PROPERTIES FOLDER "Executables/3DAudioCoder")
install(TARGETS 3DAudioDecoder DESTINATION bin)

# 3DAudioEncoder
add_executable(3DAudioEncoder
  3DAudioEncoder/src/encode.c
)
target_compile_definitions(3DAudioEncoder PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(3DAudioEncoder)
set_target_properties(3DAudioEncoder PROPERTIES FOLDER "Executables/3DAudioCoder")
install(TARGETS 3DAudioEncoder DESTINATION bin)

###############################################################################
# tests
###############################################################################

include(${CMAKE_SOURCE_DIR}/example/3DAudioDecoderTest.cmake OPTIONAL)
