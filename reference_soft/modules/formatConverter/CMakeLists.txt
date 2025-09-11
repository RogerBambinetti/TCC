cmake_minimum_required(VERSION 3.9.2)

###############################################################################
# project name
###############################################################################
project(formatConverter)

###############################################################################
# target sources (libraries)
###############################################################################
# formatConverterLib
add_library(formatConverterLib
  formatConverterLib/src/formatConverter.h
  formatConverterLib/include/formatConverter_api.h
  formatConverterLib/src/formatConverter_activeDmx_stft.h
  formatConverterLib/src/formatConverter_data.h
  formatConverterLib/src/formatConverter_init.h
  formatConverterLib/src/formatConverter_phaseAlign.h
  formatConverterLib/src/formatConverter_phaseAlign_lowcplx.h
  formatConverterLib/src/formatConverter_process.h
  formatConverterLib/src/formatConverter.c
  formatConverterLib/src/formatConverter_activeDmx_stft.c
  formatConverterLib/src/formatConverter_data.c
  formatConverterLib/src/formatConverter_init.c
  formatConverterLib/src/formatConverter_phaseAlign.c
  formatConverterLib/src/formatConverter_phaseAlign_lowcplx.c
  formatConverterLib/src/formatConverter_process.c
)
target_compile_definitions(formatConverterLib PRIVATE FORMATCONVERTER_LOWCOMPLEXITY)
target_include_directories(formatConverterLib PUBLIC "${PROJECT_SOURCE_DIR}/formatConverterLib/include"
                                             PRIVATE "${PROJECT_SOURCE_DIR}/formatConverterLib/src")
target_link_libraries(formatConverterLib cicp2geometryLib)
set_target_properties(formatConverterLib PROPERTIES FOLDER "Libraries/modules")

###############################################################################
# target sources (executables)
###############################################################################
# formatConverterCmdl
add_executable(formatConverterCmdl
  formatConverterCmdl/src/formatConverter_main.h
  formatConverterCmdl/src/formatConverter_main.c
)
# increase stack size to prevent stack overflow
target_link_options(
    formatConverterCmdl
    PRIVATE
        $<$<C_COMPILER_ID:MSVC>:/STACK:5000000>
    )
target_compile_definitions(formatConverterCmdl PRIVATE FORMATCONVERTER_LOWCOMPLEXITY _CRT_SECURE_NO_WARNINGS)
target_include_directories(formatConverterCmdl PRIVATE "${PROJECT_SOURCE_DIR}/formatConverterCmdl/src")
target_link_libraries(formatConverterCmdl formatConverterLib fftlib_3da qmflib_3da wavIO_3da gVBAPRendererLib)
set_target_properties(formatConverterCmdl PROPERTIES FOLDER "Executables/modules")
install(TARGETS formatConverterCmdl DESTINATION bin)
