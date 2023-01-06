
cmake_minimum_required(VERSION 3.25.1)
project(
  bfs
  VERSION 0.1.0
  DESCRIPTION "lightweight fat library implementation"
  HOMEPAGE_URL "https://github.com/bolthur/bfs"
  LANGUAGES C)

if(${TEST_ACTIVE})
  enable_language(CXX)
endif()

# set standards
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)
# cppcheck
set(CMAKE_CXX_CPPCHECK "cppcheck")
set(CMAKE_C_CPPCHECK "cppcheck")

# necessary when building with windows
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
# necessary stuff for building export header
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
# makefile verbose output
#set(CMAKE_VERBOSE_MAKEFILE TRUE)

# set module path
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/.module")
# modules
include(GenerateExportHeader)
include(CheckIncludeFile)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CheckLinkerFlag)
include(Dart)
# local modules
include(EnableCompilerFlagIfSupported)
include(EnableCompilerFlagDebugIfSupported)
include(EnableCompilerFlagReleaseIfSupported)
include(EnableLinkerFlagIfSupported)
include(CompileSetFlags)

# compiling define
add_compile_definitions(_BFS_COMPILING)

# specify include directories
include_directories(${PROJECT_BINARY_DIR})
include_directories(.)

# build options
option(BFS_BUILD_SHARED_LIB "Build shared library" OFF)
option(BFS_BUILD_DOC "Build library documentation" OFF)

# Set system type if not set
if(NOT DEFINED SYSTEM_TYPE)
  set(SYSTEM_TYPE unknown)
endif()
# install lib flag
if(NOT DEFINED INSTALL_LIB)
  set(INSTALL_LIB 1)
endif()
# set install lib to 0 for test
if(${TEST_ACTIVE})
  set(INSTALL_LIB 0)
  set(BFS_BUILD_SHARED_LIB OFF)
endif()

find_package(Doxygen)
if (DOXYGEN_FOUND)
  # set input and output files
  set(DOXYGEN_IN ${CMAKE_SOURCE_DIR}/doc/Doxyfile.in)
  set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  # request to configure the file
  configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
  message("Doxygen build started")

  # note the option ALL which allows to build the docs together with the application
  add_custom_target(doc_doxygen ALL
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM )
endif()

# check for include files
check_include_file("errno.h" HAVE_ERRNO_H)
check_include_file("stdio.h" HAVE_STDIO_H)
check_include_file("sys/queue.h" HAVE_SYS_QUEUE_H)
# set compile flags
compile_set_flags()

# configurations for source code
add_definitions(
  -DCONFIG_PATH_SEPARATOR_CHAR='/'
  -DCONFIG_PATH_SEPARATOR_STRING="/"
  -DCONFIG_HAVE_ERRNO_H=${HAVE_ERRNO_H}
  -DCONFIG_HAVE_STDIO_H=${HAVE_STDIO_H}
  -DCONFIG_HAVE_SYS_QUEUE_H=${HAVE_SYS_QUEUE_H})
# macro for configuration header with defines
macro(output_configure)
  get_property(definitions DIRECTORY PROPERTY COMPILE_DEFINITIONS)
  file(WRITE ${PROJECT_BINARY_DIR}/bfsconfig.h "")
  foreach(item ${definitions})
    string(REGEX MATCH "^CONFIG_" match_res ${item})
    if(match_res)
      string(REGEX REPLACE "=(.+)$" "" replace_res ${item})
      string(CONFIGURE "#define ${replace_res} ${CMAKE_MATCH_1}" output_str)
      file(APPEND ${PROJECT_BINARY_DIR}/bfsconfig.h "${output_str}\n")
    endif()
  endforeach()
endmacro()
output_configure()

# add subdirectory blockdev and fat
add_subdirectory(blockdev)
add_subdirectory(common)
add_subdirectory(fat)
add_subdirectory(ext)
add_subdirectory(cdfs)

# integrate unit tests only when generic-test toolchain is used
if (${TEST_ACTIVE})
  enable_testing()
  add_subdirectory(tests)
endif()

# distribution stuff
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_SOURCE_IGNORE_FILES "/build" ".git")
include(CPack)

add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

configure_file(bfs.pc.in bfs.pc @ONLY)
if(DEFINED INSTALL_LIB)
  INSTALL(FILES ${PROJECT_BINARY_DIR}/bfs.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
  INSTALL(FILES ${PROJECT_BINARY_DIR}/bfsconfig.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/bfs)
endif()
