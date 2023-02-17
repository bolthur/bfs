
find_package(Check REQUIRED)
include_directories(${CHECK_INCLUDE_DIRS})
link_directories(${CHECK_LIBRARY_DIRS})

find_package(GTest REQUIRED)
enable_compiler_flag_if_supported("-Wno-analyzer-use-of-uninitialized-value")

enable_compiler_flag_if_supported("-Wno-missing-prototypes")
enable_compiler_flag_if_supported("-Wno-unused-variable")

file (GLOB blockdev_file "blockdevice/*")
file (GLOB fat12_file "fat12/*")
file (GLOB fat16_file "fat16/*")
file (GLOB fat32_file "fat32/*")
file (GLOB ext_file "ext/*")

# custom commands for generation of image files
add_custom_target(
  dummy_target ALL
  DEPENDS
  generate_fat12_image generate_fat16_image generate_fat32_image generate_ext2_image
)
add_custom_command(
  OUTPUT always_rebuild
  COMMAND cmake -E echo
)
add_custom_command(
  OUTPUT generate_fat12_image
  COMMAND nim -d:release -d:ssl compile --hints:off --run image.nim fat12 fat12 ${CMAKE_CURRENT_BINARY_DIR}
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/.tools"
  COMMENT "Generating FAT12 test image"
  DEPENDS always_rebuild
)
add_custom_command(
  OUTPUT generate_fat16_image
  COMMAND nim -d:release -d:ssl compile --hints:off --run image.nim fat16 fat16 ${CMAKE_CURRENT_BINARY_DIR}
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/.tools"
  COMMENT "Generating FAT16 test image"
  DEPENDS always_rebuild
)
add_custom_command(
  OUTPUT generate_fat32_image
  COMMAND nim -d:release -d:ssl compile --hints:off --run image.nim fat32 fat32 ${CMAKE_CURRENT_BINARY_DIR}
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/.tools"
  COMMENT "Generating FAT32 test image"
  DEPENDS always_rebuild
)
add_custom_command(
  OUTPUT generate_ext2_image
  COMMAND nim -d:release -d:ssl compile --hints:off --run image.nim ext2 ext2 ${CMAKE_CURRENT_BINARY_DIR}
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/.tools"
  COMMENT "Generating ext2 test image"
  DEPENDS always_rebuild
)

add_executable(bfs_check
  _helper.cc
  ${blockdev_file}
  ${fat12_file}
  ${fat16_file}
  ${fat32_file}
  ${ext_file} )
if(${BFS_BUILD_WITH_COVERAGE} AND CMAKE_C_COMPILER_ID STREQUAL "GNU")
  target_link_libraries(bfs_check GTest::gtest_main gcov bfsfat bfsext bfsblockdev bfscommon)
else()
  target_link_libraries(bfs_check GTest::gtest_main bfsfat bfsext bfsblockdev bfscommon)
endif()
gtest_discover_tests(bfs_check)
add_dependencies(bfs_check dummy_target)
