
find_package(Check REQUIRED)

enable_compiler_flag_if_supported("-Wno-missing-prototypes")

add_executable(test_blockdev_validate_test_driver validate_test_driver.c)
if(${BFS_BUILD_WITH_COVERAGE} AND CMAKE_COMPILER_IS_GNUC)
  target_link_libraries(test_blockdev_validate_test_driver check gcov bfsfat bfsblockdev bfscommon)
else()
  target_link_libraries(test_blockdev_validate_test_driver check bfsfat bfsblockdev bfscommon)
endif()
add_test(NAME test_blockdev_validate_test_driver COMMAND test_blockdev_validate_test_driver)
