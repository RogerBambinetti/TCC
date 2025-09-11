cmake_minimum_required(VERSION 3.9.2)

###############################################################################
# project name
###############################################################################
project(mixer)

###############################################################################
# target sources (libraries)
###############################################################################
# mixerLib
add_library(mixerLib
  mixerLib/include/mixerlib.h
  mixerLib/src/mixerlib.c
)
target_include_directories(mixerLib PUBLIC "${PROJECT_SOURCE_DIR}/mixerLib/include")
target_link_libraries(mixerLib wavIO_3da)
set_target_properties(mixerLib PROPERTIES FOLDER "Libraries/modules")

###############################################################################
# target sources (executables)
###############################################################################
# mixerCmdl
add_executable(mixerCmdl
  mixerCmdl/src/mixerCmdl.c
)
# increase stack size to prevent stack overflow
target_link_options(
    mixerCmdl
    PRIVATE
        $<$<C_COMPILER_ID:MSVC>:/STACK:5000000>
    )
target_compile_definitions(mixerCmdl PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(mixerCmdl mixerLib)
set_target_properties(mixerCmdl PROPERTIES FOLDER "Executables/modules")
install(TARGETS mixerCmdl DESTINATION bin)
