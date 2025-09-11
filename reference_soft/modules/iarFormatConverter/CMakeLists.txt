cmake_minimum_required(VERSION 3.9.2)

###############################################################################
# project name
###############################################################################
project(iarFormatConverter)

# include_directories("${PROJECT_SOURCE_DIR}/iarFormatConverterLib/include")
# include_directories("${PROJECT_SOURCE_DIR}/iarFormatConverterLib/src")
# include_directories("${PROJECT_SOURCE_DIR}/iarFormatConverterCmdl/src")
# include_directories("${PROJECT_SOURCE_DIR}/../gVBAPRenderer/gVBAPRendererLib/include")
# include_directories("${PROJECT_SOURCE_DIR}/../../tools/cicp2geometry/cicp2geometryLib/include")
# include_directories("${PROJECT_SOURCE_DIR}/../../tools/fftlib/include")
# include_directories("${PROJECT_SOURCE_DIR}/../../tools/qmflib/include")
# include_directories("${PROJECT_SOURCE_DIR}/../../tools/wavIO/include")

#add_compile_definitions(FORMATCONVERTER_LOWCOMPLEXITY)

# add_definitions(FORMATCONVERTER_LOWCOMPLEXITY)

###############################################################################
# target sources (libraries)
###############################################################################
# iarFormatConverterLib
add_library(iarFormatConverterLib
  iarFormatConverterLib/include/iarFormatConverter_api.h
  iarFormatConverterLib/src/iarFormatConverter.h
  iarFormatConverterLib/src/iarFormatConverter_activeDmx_stft.h
  iarFormatConverterLib/src/iarFormatConverter_data.h
  iarFormatConverterLib/src/iarFormatConverter_init.h
  iarFormatConverterLib/src/iarFormatConverter_phaseAlign.h
  iarFormatConverterLib/src/iarFormatConverter_phaseAlign_lowcplx.h
  iarFormatConverterLib/src/iarFormatConverter_process.h
  iarFormatConverterLib/src/iar_init.h
  iarFormatConverterLib/src/iar_rom.h
  iarFormatConverterLib/src/iarFormatConverter.c
  iarFormatConverterLib/src/iarFormatConverter_activeDmx_stft.c
  iarFormatConverterLib/src/iarFormatConverter_data.c
  iarFormatConverterLib/src/iarFormatConverter_init.c
  iarFormatConverterLib/src/iarFormatConverter_phaseAlign.c
  iarFormatConverterLib/src/iarFormatConverter_phaseAlign_lowcplx.c
  iarFormatConverterLib/src/iarFormatConverter_process.c
  iarFormatConverterLib/src/iar_init.c
  iarFormatConverterLib/src/iar_rom.c
)
target_compile_definitions(iarFormatConverterLib PRIVATE FORMATCONVERTER_LOWCOMPLEXITY)
target_include_directories(iarFormatConverterLib PUBLIC "${PROJECT_SOURCE_DIR}/iarFormatConverterLib/include"
                                                PRIVATE "${PROJECT_SOURCE_DIR}/iarFormatConverterLib/src")
target_link_libraries(iarFormatConverterLib cicp2geometryLib)
set_target_properties(iarFormatConverterLib PROPERTIES FOLDER "Libraries/modules")

###############################################################################
# target sources (executables)
###############################################################################
# iarFormatConverterCmdl
add_executable(iarFormatConverterCmdl
  iarFormatConverterCmdl/src/iarFormatConverter_main.h
  iarFormatConverterCmdl/src/iarFormatConverter_main.c
)
target_compile_definitions(iarFormatConverterCmdl PRIVATE FORMATCONVERTER_LOWCOMPLEXITY _CRT_SECURE_NO_WARNINGS)
target_include_directories(iarFormatConverterCmdl PRIVATE "${PROJECT_SOURCE_DIR}/iarFormatConverterCmdl/src")
target_link_libraries(iarFormatConverterCmdl iarFormatConverterLib cicp2geometryLib fftlib_3da gVBAPRendererLib qmflib_3da wavIO_3da)
set_target_properties(iarFormatConverterCmdl PROPERTIES FOLDER "Executables/modules")
install(TARGETS iarFormatConverterCmdl DESTINATION bin)
