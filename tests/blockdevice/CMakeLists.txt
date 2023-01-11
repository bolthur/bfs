
find_package(GTest REQUIRED)

enable_compiler_flag_if_supported("-Wno-analyzer-use-of-uninitialized-value")

add_executable(test_blockdev_validate_test_driver validate_test_driver.cc)
if(${BFS_BUILD_WITH_COVERAGE} AND CMAKE_COMPILER_IS_GNUCXX)
  target_link_libraries(test_blockdev_validate_test_driver GTest::gtest_main gcov bfsfat bfsblockdev bfscommon)
else()
  target_link_libraries(test_blockdev_validate_test_driver GTest::gtest_main bfsfat bfsblockdev bfscommon)
endif()
gtest_discover_tests(test_blockdev_validate_test_driver)
