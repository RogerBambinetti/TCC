cmake_minimum_required(VERSION 3.9.2)

###############################################################################
# project name
###############################################################################
project(dynamicObjectPriorityGenerator)

###############################################################################
# target sources (libraries)
###############################################################################
# DynamicObjectPriorityGeneratorLib
add_library(DynamicObjectPriorityGeneratorLib
  DynamicObjectPriorityGeneratorLib/include/DynamicObjectPriorityGeneratorlib.h
  DynamicObjectPriorityGeneratorLib/src/DynamicObjectPriorityGeneratorlib.c
)
target_compile_definitions(DynamicObjectPriorityGeneratorLib PRIVATE _CRT_SECURE_NO_WARNINGS)
target_include_directories(DynamicObjectPriorityGeneratorLib PUBLIC "${PROJECT_SOURCE_DIR}/DynamicObjectPriorityGeneratorLib/include")
target_link_libraries(DynamicObjectPriorityGeneratorLib oamCommonLib oamEncoderLib wavIO_3da)
set_target_properties(DynamicObjectPriorityGeneratorLib PROPERTIES FOLDER "Libraries/modules")

###############################################################################
# target sources (executables)
###############################################################################
# DynamicObjectPriorityGeneratorCmdl
add_executable(DynamicObjectPriorityGeneratorCmdl
  DynamicObjectPriorityGeneratorCmdl/src/DynamicObjectPriorityGeneratorCmdl.c
)
target_link_libraries(DynamicObjectPriorityGeneratorCmdl DynamicObjectPriorityGeneratorLib)
set_target_properties(DynamicObjectPriorityGeneratorCmdl PROPERTIES FOLDER "Executables/modules")
install(TARGETS DynamicObjectPriorityGeneratorCmdl DESTINATION bin)
