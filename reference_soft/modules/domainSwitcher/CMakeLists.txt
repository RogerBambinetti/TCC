cmake_minimum_required(VERSION 3.9.2)

###############################################################################
# project name
###############################################################################
project(domainSwitcher)

###############################################################################
# target sources (libraries)
###############################################################################

###############################################################################
# target sources (executables)
###############################################################################
# domainSwitcherCmdl
add_executable(domainSwitcherCmdl
  domainSwitcherCmdl/src/domainSwitcherCmdl.c
)
target_compile_definitions(domainSwitcherCmdl PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(domainSwitcherCmdl wavIO_3da fftlib_3da qmflib_3da)
set_target_properties(domainSwitcherCmdl PROPERTIES FOLDER "Executables/modules")
install(TARGETS domainSwitcherCmdl DESTINATION bin)
