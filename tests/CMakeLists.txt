
find_package(Check REQUIRED)
include_directories(${CHECK_INCLUDE_DIRS})
link_directories(${CHECK_LIBRARY_DIRS})

add_subdirectory(blockdevice)
add_subdirectory(fat12)
add_subdirectory(fat16)
add_subdirectory(fat32)

# custom commands for generation of image files
add_custom_target(
  dummy_target ALL
  DEPENDS
  generate_fat12_image generate_fat16_image generate_fat32_image
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
