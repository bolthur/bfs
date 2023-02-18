
cmake_minimum_required(VERSION 3.25.1)
project(
  bfs
  VERSION 0.1.0
  DESCRIPTION "bolthur filesystem library project"
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
option(BFS_BUILD_WITH_COVERAGE "Build with coverage data" OFF)
option(BFS_MALLOC_FALLBACK_SIZE "Malloc fallback size" 0)
option(BFS_USE_TRANSACTION "Use transaction for file operations" 1)

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

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif(CCACHE_FOUND)

find_package(Doxygen)
if (DOXYGEN_FOUND AND BFS_BUILD_DOC)
  # set input and output files
  set(DOXYGEN_IN ${CMAKE_SOURCE_DIR}/.doc/Doxyfile.in)
  set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  # request to configure the file
  configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
  message("Doxygen build started")

  # note the option ALL which allows to build the docs together with the application
  add_custom_target(doc
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM )
endif()

if(${BFS_BUILD_WITH_COVERAGE})
  # find necessary programs
  find_program(LCOV_PATH lcov)
  find_program(GENHTML_PATH genhtml)
  # hide behind the 'advanced' mode flag for GUI/ccmake
  mark_as_advanced(FORCE LCOV_PATH GENHTML_PATH)
  # code coverage is only possible with gnu compiler
  if(CMAKE_C_COMPILER_ID STREQUAL "GNU" AND LCOV_PATH AND GENHTML_PATH)
    # adjust cflags according to compiler
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
    # set output directory
    set(CMAKE_COVERAGE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/cov)
    # note the option ALL which allows to build the docs together with the application
    add_custom_target(coverage_collect
      COMMAND lcov --capture --directory . --output-file coverage.info
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating lcov information"
      VERBATIM )
    add_custom_target(coverage_strip
      COMMAND lcov --remove coverage.info '/usr/*' --remove coverage.info '*tests/*' --output-file ${CMAKE_CURRENT_BINARY_DIR}/coverage.info
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Strip out test and system include"
      DEPENDS coverage_collect )
    add_custom_target(coverage
      COMMAND genhtml coverage.info --output-directory ${CMAKE_SOURCE_DIR}/cov
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating coverage report"
      DEPENDS coverage_strip
      VERBATIM )
  endif()
endif()

if(${BFS_BUILD_WITH_PROFILING})
  # adjust cflags according to compiler
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")
endif()

# check for include files
check_include_file("errno.h" HAVE_ERRNO_H)
check_include_file("stdlib.h" HAVE_STDLIB_H)
check_include_file("string.h" HAVE_STRING_H)
check_include_file("stddef.h" HAVE_STDDEF_H)
check_include_file("stdint.h" HAVE_STDINT_H)
check_include_file("limits.h" HAVE_LIMITS_H)
check_include_file("stdbool.h" HAVE_STDBOOL_H)
check_include_file("libgen.h" HAVE_LIBGEN_H)
check_include_file("stdio.h" HAVE_STDIO_H)
# set compile flags
compile_set_flags()

# configurations for source code
add_definitions(
  -DCONFIG_PATH_SEPARATOR_CHAR='/'
  -DCONFIG_PATH_SEPARATOR_STRING="/")

if (${HAVE_ERRNO_H})
  add_definitions(-DCONFIG_HAVE_ERRNO_H=1)
else()
  add_definitions(-DCONFIG_HAVE_ERRNO_H=0)
endif()
if (${HAVE_STDIO_H})
  add_definitions(-DCONFIG_HAVE_STDIO_H=1)
else()
  add_definitions(-DCONFIG_HAVE_STDIO_H=0)
endif()
if (${HAVE_STDLIB_H})
  add_definitions(-DCONFIG_HAVE_STDLIB_H=1)
else()
  add_definitions(-DCONFIG_HAVE_STDLIB_H=0)
endif()
if (${HAVE_STRING_H})
  add_definitions(-DCONFIG_HAVE_STRING_H=1)
else()
  add_definitions(-DCONFIG_HAVE_STRING_H=0)
endif()
if (${HAVE_STDDEF_H})
  add_definitions(-DCONFIG_HAVE_STDDEF_H=1)
else()
  add_definitions(-DCONFIG_HAVE_STDDEF_H=0)
endif()
if (${HAVE_STDINT_H})
  add_definitions(-DCONFIG_HAVE_STDINT_H=1)
else()
  add_definitions(-DCONFIG_HAVE_STDINT_H=0)
endif()
if (${HAVE_LIMITS_H})
  add_definitions(-DCONFIG_HAVE_LIMITS_H=1)
else()
  add_definitions(-DCONFIG_HAVE_LIMITS_H=0)
endif()
if (${HAVE_STDBOOL_H})
  add_definitions(-DCONFIG_HAVE_STDBOOL_H=1)
else()
  add_definitions(-DCONFIG_HAVE_STDBOOL_H=0)
endif()
if (${HAVE_LIBGEN_H})
  add_definitions(-DCONFIG_HAVE_LIBGEN_H=1)
else()
  add_definitions(-DCONFIG_HAVE_LIBGEN_H=0)
endif()

if(${BFS_USE_TRANSACTION})
  add_definitions(-DCONFIG_USE_TRANSACTION=1)
else()
  add_definitions(-DCONFIG_USE_TRANSACTION=0)
endif()

if (NOT ${HAVE_STDLIB_H} AND ${BFS_MALLOC_FALLBACK_SIZE})
  add_definitions(-DCONFIG_MALLOC_FALLBACK_AREA=${BFS_MALLOC_FALLBACK_SIZE})
endif()

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

# integrate unit tests only when generic-test toolchain is used
if (${TEST_ACTIVE})
  add_subdirectory(tests)
endif()

# distribution stuff
set(CPACK_PACKAGE_VENDOR "bolthur")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "bolthur filesystem library project")
set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH}")
set(CPACK_SOURCE_IGNORE_FILES "/build" ".git")
include(CPack)

add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

set(pc_libs_private "-L\${libdir}/blockdev -lbfsblockdev -L\${libdir}/common -lbfscommon")
configure_file(bfs.pc.in bfs.pc @ONLY)
if(DEFINED INSTALL_LIB)
  INSTALL(FILES ${PROJECT_BINARY_DIR}/bfs.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
  INSTALL(FILES ${PROJECT_BINARY_DIR}/bfsconfig.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/bfs)
endif()

if(DEFINED INSTALL_LIB)
  INSTALL(DIRECTORY ./thirdparty DESTINATION ${CMAKE_INSTALL_PREFIX}/include/bfs
    FILES_MATCHING PATTERN "*.h*")
endif()
