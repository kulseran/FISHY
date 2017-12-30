cmake_minimum_required (VERSION 2.6)
include (CMake/util.cmake)
include (CMake/proto.cmake)
project (Fishy)

# Output locations
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
 
# Get the current working branch
execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the latest abbreviated commit hash of the working branch
execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/version.h.in"
	"${PROJECT_SOURCE_DIR}/.generated/version.h"
)

# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
include_directories("${PROJECT_SOURCE_DIR}")
# add the generated folder, so that files mirroring the main
# repo structure within generated can be included without a
# prefix
set(FISHY_GENERATED_ROOT "${PROJECT_SOURCE_DIR}/.generated")
include_directories(${FISHY_GENERATED_ROOT})

# Squelch visual studio warnings
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

set(FISHY_SOURCE_DIR "${PROJECT_SOURCE_DIR}")
################
# All Buildables
################
add_subdirectory(CORE)
add_subdirectory(APP_SHARED)
add_subdirectory(TESTS)
add_subdirectory(TOOLS/bin2h)
add_subdirectory(TOOLS/protoc)
add_subdirectory(GAME)

