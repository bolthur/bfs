
if (${TEST_ACTIVE})
  aux_source_directory(tests BLOCKDEV_SRC)
  if(DEFINED INSTALL_LIB)
    INSTALL(FILES tests/blockdev.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/bfs/blockdev)
  endif()
elseif(SYSTEM_TYPE STREQUAL bolthur)
  aux_source_directory(bolthur BLOCKDEV_SRC)
  if(DEFINED INSTALL_LIB)
    INSTALL(FILES bolthur/blockdev.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/bfs/blockdev)
  endif()
else()
  aux_source_directory(. BLOCKDEV_SRC)
  if(DEFINED INSTALL_LIB)
    INSTALL(FILES blockdev.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/bfs/blockdev)
  endif()
endif()

if(BFS_BUILD_SHARED_LIB)
  add_library(bfsblockdev SHARED ${BLOCKDEV_SRC})
else()
  add_library(bfsblockdev STATIC ${BLOCKDEV_SRC})
endif()

set_target_properties(bfsblockdev
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION}
    C_STANDARD 17
    C_STANDARD_REQUIRED 17
)

GENERATE_EXPORT_HEADER(bfsblockdev)

if(DEFINED INSTALL_LIB)
  INSTALL(DIRECTORY ${PROJECT_BINARY_DIR}/blockdev/. DESTINATION ${CMAKE_INSTALL_PREFIX}/include/bfs/blockdev
    FILES_MATCHING PATTERN "*.h*"
    PATTERN "CMakeFiles" EXCLUDE
    PATTERN "bfsblockdev_export.h" EXCLUDE)
  INSTALL(TARGETS bfsblockdev DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/bfs/blockdev)
endif()
