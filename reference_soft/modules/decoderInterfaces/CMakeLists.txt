cmake_minimum_required(VERSION 3.9.2)

###############################################################################
# project name
###############################################################################
project(Interfaces)

###############################################################################
# target sources (libraries)
###############################################################################

if(UNIX AND NOT APPLE)
  set(ADD_MATH_LIB m)
endif()

# elementInteractionInterfaceLib
add_library(elementInteractionInterfaceLib
  elementInteractionInterface/elementInteractionInterfaceLib/include/elementInteractionInterfaceLib.h
  elementInteractionInterface/elementInteractionInterfaceLib/include/elementInteractionInterfaceLib_API.h
  elementInteractionInterface/elementInteractionInterfaceLib/include/elementInteractionInterfaceLib_BitstreamIO.h
  elementInteractionInterface/elementInteractionInterfaceLib/src/elementInteractionInterfaceLib.c
  elementInteractionInterface/elementInteractionInterfaceLib/src/elementInteractionInterfaceLib_BitstreamIO.c
)
target_compile_definitions(elementInteractionInterfaceLib PRIVATE _CRT_SECURE_NO_WARNINGS _MBCS)
target_include_directories(elementInteractionInterfaceLib PUBLIC "${PROJECT_SOURCE_DIR}/elementInteractionInterface/elementInteractionInterfaceLib/include")
target_link_libraries(elementInteractionInterfaceLib ${ADD_MATH_LIB})
set_target_properties(elementInteractionInterfaceLib PROPERTIES FOLDER "Libraries/Interfaces")

# localSetupInformationInterfaceLib
add_library(localSetupInformationInterfaceLib
  localSetupInformationInterface/localSetupInformationInterfaceLib/include/localSetupInformationInterfaceLib.h
  localSetupInformationInterface/localSetupInformationInterfaceLib/include/localSetupInformationInterfaceLib_API.h
  localSetupInformationInterface/localSetupInformationInterfaceLib/include/localSetupInformationInterfaceLib_BitstreamIO.h
  localSetupInformationInterface/localSetupInformationInterfaceLib/src/localSetupInformationInterfaceLib.c
  localSetupInformationInterface/localSetupInformationInterfaceLib/src/localSetupInformationInterfaceLib_BitstreamIO.c
)
target_compile_definitions(localSetupInformationInterfaceLib PRIVATE _CRT_SECURE_NO_WARNINGS)
target_include_directories(localSetupInformationInterfaceLib PUBLIC "${PROJECT_SOURCE_DIR}/localSetupInformationInterface/localSetupInformationInterfaceLib/include")
target_link_libraries(localSetupInformationInterfaceLib binauralInterfaceLib)
set_target_properties(localSetupInformationInterfaceLib PROPERTIES FOLDER "Libraries/Interfaces")

# sceneDisplacementInterfaceLib
add_library(sceneDisplacementInterfaceLib
  sceneDisplacementInterface/sceneDisplacementInterfaceLib/include/sceneDisplacementInterfaceLib.h
  sceneDisplacementInterface/sceneDisplacementInterfaceLib/include/sceneDisplacementInterfaceLib_API.h
  sceneDisplacementInterface/sceneDisplacementInterfaceLib/include/sceneDisplacementInterfaceLib_BitstreamIO.h
  sceneDisplacementInterface/sceneDisplacementInterfaceLib/src/sceneDisplacementInterfaceLib.c
  sceneDisplacementInterface/sceneDisplacementInterfaceLib/src/sceneDisplacementInterfaceLib_BitstreamIO.c
)
target_compile_definitions(sceneDisplacementInterfaceLib PRIVATE _CRT_SECURE_NO_WARNINGS)
target_include_directories(sceneDisplacementInterfaceLib PUBLIC "${PROJECT_SOURCE_DIR}/sceneDisplacementInterface/sceneDisplacementInterfaceLib/include")
target_link_libraries(sceneDisplacementInterfaceLib ${ADD_MATH_LIB})
set_target_properties(sceneDisplacementInterfaceLib PROPERTIES FOLDER "Libraries/Interfaces")

###############################################################################
# target sources (executables)
###############################################################################
# elementInteractionInterfaceExample
add_executable(elementInteractionInterfaceExample
  elementInteractionInterface/elementInteractionInterfaceExample/src/elementInteractionInterfaceCmdl.c
)
target_link_libraries(elementInteractionInterfaceExample elementInteractionInterfaceLib)
set_target_properties(elementInteractionInterfaceExample PROPERTIES FOLDER "Executables/Interfaces")
install(TARGETS elementInteractionInterfaceExample DESTINATION bin)

# localSetupInformationInterfaceExample
add_executable(localSetupInformationInterfaceExample
  localSetupInformationInterface/localSetupInformationInterfaceExample/src/localSetupInformationInterfaceCmdl.c
)
target_link_libraries(localSetupInformationInterfaceExample localSetupInformationInterfaceLib)
set_target_properties(localSetupInformationInterfaceExample PROPERTIES FOLDER "Executables/Interfaces")
if(MSVC)
  set_target_properties(localSetupInformationInterfaceExample PROPERTIES OUTPUT_NAME "localSetupInterfaceExample")
endif()
install(TARGETS localSetupInformationInterfaceExample DESTINATION bin)

# sceneDisplacementInterfaceExample
add_executable(sceneDisplacementInterfaceExample
  sceneDisplacementInterface/sceneDisplacementInterfaceExample/src/sceneDisplacementInterfaceCmdl.c
)
target_link_libraries(sceneDisplacementInterfaceExample sceneDisplacementInterfaceLib)
set_target_properties(sceneDisplacementInterfaceExample PROPERTIES FOLDER "Executables/Interfaces")
install(TARGETS sceneDisplacementInterfaceExample DESTINATION bin)
