cmake_minimum_required(VERSION 3.9.2)

###############################################################################
# project name
###############################################################################
project(Binaural)

###############################################################################
# target sources (libraries)
###############################################################################

if(UNIX AND NOT APPLE)
  set(ADD_MATH_LIB m)
endif()

# binauralInterfaceLib
add_library(binauralInterfaceLib
  interface/binauralInterfaceLib/include/bitstreamBinaural.h
  interface/binauralInterfaceLib/include/speakerConfig3d.h
  interface/binauralInterfaceLib/src/bitstreamBinaural.c
  interface/binauralInterfaceLib/src/speakerConfig3d.c
)
target_compile_definitions(binauralInterfaceLib PRIVATE _CRT_SECURE_NO_WARNINGS)
target_include_directories(binauralInterfaceLib PUBLIC "${PROJECT_SOURCE_DIR}/interface/binauralInterfaceLib/include")
target_link_libraries(binauralInterfaceLib cicp2geometryLib libtsplite readonlybitbuf_3da writeonlybitbuf_3da ${ADD_MATH_LIB})
set_target_properties(binauralInterfaceLib PROPERTIES FOLDER "Libraries/Binaural")

# binauralParametrizationFdLib
add_library(binauralParametrizationFdLib
  parametrization/parametrization_FD/binauralParametrizationLib/include/BinauralFdParameterization.h
  parametrization/parametrization_FD/binauralParametrizationLib/src/BinauralFdParameterization.c
  #parametrization/parametrization_FD/binauralParametrizationLib/src/ReverbAnalysis.c
)
target_include_directories(binauralParametrizationFdLib PUBLIC "${PROJECT_SOURCE_DIR}/parametrization/parametrization_FD/binauralParametrizationLib/include")
target_link_libraries(binauralParametrizationFdLib binauralInterfaceLib fftlib_3da)
set_target_properties(binauralParametrizationFdLib PROPERTIES FOLDER "Libraries/Binaural")

# binauralParametrizationTdLib
add_library(binauralParametrizationTdLib
  parametrization/parametrization_TD/binauralParametrizationLib/include/binauralization_FilterDesign.h
  parametrization/parametrization_TD/binauralParametrizationLib/src/binauralization_FilterDesign.c
)
target_include_directories(binauralParametrizationTdLib PUBLIC "${PROJECT_SOURCE_DIR}/parametrization/parametrization_TD/binauralParametrizationLib/include")
target_link_libraries(binauralParametrizationTdLib binauralInterfaceLib fftlib_3da)
set_target_properties(binauralParametrizationTdLib PROPERTIES FOLDER "Libraries/Binaural")

# binauralRendererFdLib
add_library(binauralRendererFdLib
  renderer/renderer_FD/binauralRendererLib/include/Binauralizer.h
  renderer/renderer_FD/binauralRendererLib/include/BinauralProfileLevelConstraints.h
  renderer/renderer_FD/binauralRendererLib/include/ReverbGenerator.h
  renderer/renderer_FD/binauralRendererLib/src/Binauralizer.c
  renderer/renderer_FD/binauralRendererLib/src/BinauralProfileLevelConstraints.c
  renderer/renderer_FD/binauralRendererLib/src/ReverbGenerator.c
)
target_compile_definitions(binauralRendererFdLib PRIVATE _FTRD _X86_VC8_TARGET_ CODE_ANALYSIS;CODE_ANALYSIS)
target_include_directories(binauralRendererFdLib PUBLIC "${PROJECT_SOURCE_DIR}/renderer/renderer_FD/binauralRendererLib/include")
target_link_libraries(binauralRendererFdLib fftlib_3da qmflib_3da wavIO_3da cicp2geometryLib)
set_target_properties(binauralRendererFdLib PROPERTIES FOLDER "Libraries/Binaural")

# binauralRendererTdLib
add_library(binauralRendererTdLib
  renderer/renderer_TD/binauralRendererLib/include/binauralization.h
  renderer/renderer_TD/binauralRendererLib/src/binauralization.c
)
target_compile_definitions(binauralRendererTdLib PRIVATE _FTRD _X86_VC8_TARGET_ CODE_ANALYSIS;CODE_ANALYSIS)
target_include_directories(binauralRendererTdLib PUBLIC "${PROJECT_SOURCE_DIR}/renderer/renderer_TD/binauralRendererLib/include")
target_link_libraries(binauralRendererTdLib fftlib_3da)
set_target_properties(binauralRendererTdLib PROPERTIES FOLDER "Libraries/Binaural")

###############################################################################
# target sources (executables)
###############################################################################
# bitstreamWriterCmdl
add_executable(bitstreamWriterCmdl
  interface/bitstreamWriterCmdl/src/BitstreamWriterCmdl.c
)
target_compile_definitions(bitstreamWriterCmdl PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(bitstreamWriterCmdl binauralInterfaceLib)
set_target_properties(bitstreamWriterCmdl PROPERTIES FOLDER "Executables/Binaural")
install(TARGETS bitstreamWriterCmdl DESTINATION bin)

# binauralParametrizationFdCmdl
add_executable(binauralParametrizationFdCmdl
  parametrization/parametrization_FD/binauralParametrizationCmdl/src/BinauralParametrizationCmdl.c
)
target_compile_definitions(binauralParametrizationFdCmdl PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(binauralParametrizationFdCmdl binauralParametrizationFdLib)
set_target_properties(binauralParametrizationFdCmdl PROPERTIES FOLDER "Executables/Binaural")
install(TARGETS binauralParametrizationFdCmdl DESTINATION bin)

# binauralParametrizationTdCmdl
add_executable(binauralParametrizationTdCmdl
  parametrization/parametrization_TD/binauralParametrizationCmdl/src/BinauralParametrizationCmdl.c
)
target_compile_definitions(binauralParametrizationTdCmdl PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(binauralParametrizationTdCmdl binauralParametrizationTdLib)
set_target_properties(binauralParametrizationTdCmdl PROPERTIES FOLDER "Executables/Binaural")
install(TARGETS binauralParametrizationTdCmdl DESTINATION bin)

# binauralRendererFdCmdl
add_executable(binauralRendererFdCmdl
  renderer/renderer_FD/binauralRendererCmdl/src/BinauralRendererCmdl.c
)
target_compile_definitions(binauralRendererFdCmdl PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(binauralRendererFdCmdl binauralRendererFdLib binauralInterfaceLib)
set_target_properties(binauralRendererFdCmdl PROPERTIES FOLDER "Executables/Binaural")
install(TARGETS binauralRendererFdCmdl DESTINATION bin)

# binauralRendererTdCmdl
add_executable(binauralRendererTdCmdl
  renderer/renderer_TD/binauralRendererCmdl/src/BinauralRendererCmdl.c
)
target_compile_definitions(binauralRendererTdCmdl PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(binauralRendererTdCmdl binauralRendererTdLib binauralInterfaceLib wavIO_3da)
set_target_properties(binauralRendererTdCmdl PROPERTIES FOLDER "Executables/Binaural")
if(MSVC)
  set_target_properties(binauralRendererTdCmdl PROPERTIES OUTPUT_NAME "BinauralRendererTdCmdl")
endif()
install(TARGETS binauralRendererTdCmdl DESTINATION bin)
