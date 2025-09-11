cmake_minimum_required(VERSION 3.9.2)

###############################################################################
# project name
###############################################################################
project(gVBAPRenderer)

###############################################################################
# target sources (libraries)
###############################################################################
# gVBAPRendererLib
add_library(gVBAPRendererLib
  gVBAPRendererLib/include/gVBAPRenderer.h
  gVBAPRendererLib/src/gVBAPRenderer.cpp
  gVBAPRendererLib/src/core/quickHull.h
  gVBAPRendererLib/src/core/quickHull_functions.h
  gVBAPRendererLib/src/core/quickHull_objects.h
  gVBAPRendererLib/src/core/exception.hpp
  gVBAPRendererLib/src/core/functions.hpp
  gVBAPRendererLib/src/core/speakersetup.hpp
  gVBAPRendererLib/src/core/types.hpp
  gVBAPRendererLib/src/core/vbap_core.hpp
  gVBAPRendererLib/src/core/vbap_ghost_wrapper.hpp
  gVBAPRendererLib/src/core/quickHull.cpp
  gVBAPRendererLib/src/core/functions.cpp
  gVBAPRendererLib/src/core/speakersetup.cpp
  gVBAPRendererLib/src/core/types.cpp
  gVBAPRendererLib/src/core/vbap_core.cpp
  gVBAPRendererLib/src/core/vbap_ghost_wrapper.cpp
)
target_compile_definitions(gVBAPRendererLib PRIVATE _USE_MATH_DEFINES CLASSIC_VBAP RM0_3D_BUGFIX_DIFFERENT_TRIANGLES)
target_include_directories(gVBAPRendererLib PUBLIC "${PROJECT_SOURCE_DIR}/gVBAPRendererLib/include"
                                           PRIVATE "${PROJECT_SOURCE_DIR}/gVBAPRendererLib/src/core")
target_link_libraries(gVBAPRendererLib cicp2geometryLib)
set_target_properties(gVBAPRendererLib PROPERTIES FOLDER "Libraries/modules")

###############################################################################
# target sources (executables)
###############################################################################
# gVBAPRendererCmdl
add_executable(gVBAPRendererCmdl
  gVBAPRendererCmdl/src/gVBAPRendererCmdl.c
)
target_compile_definitions(gVBAPRendererCmdl PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(gVBAPRendererCmdl gVBAPRendererLib cicp2geometryLib wavIO_3da writeonlybitbuf_3da oamEncoderLib oamCommonLib)
set_target_properties(gVBAPRendererCmdl PROPERTIES FOLDER "Executables/modules")
install(TARGETS gVBAPRendererCmdl DESTINATION bin)
