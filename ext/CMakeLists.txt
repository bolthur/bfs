
aux_source_directory(. BFSEXT_SRC)
if(BFS_BUILD_SHARED_LIB)
  add_library(bfsext SHARED ${BFSEXT_SRC})
else()
  add_library(bfsext STATIC ${BFSEXT_SRC})
endif()

set_target_properties(bfsext
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION}
    C_STANDARD 17
    C_STANDARD_REQUIRED 17
)

GENERATE_EXPORT_HEADER(bfsext)

if(DEFINED INSTALL_LIB)
  INSTALL(TARGETS bfsext DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/bfs)
  INSTALL(
    DIRECTORY ${PROJECT_BINARY_DIR}/ext/. DESTINATION ${CMAKE_INSTALL_PREFIX}/include/bfs/ext
    FILES_MATCHING PATTERN "*.h*"
    PATTERN "CMakeFiles" EXCLUDE
    PATTERN "bfsext_export.h" EXCLUDE)
  INSTALL(DIRECTORY . DESTINATION ${CMAKE_INSTALL_PREFIX}/include/bfs/ext
    FILES_MATCHING PATTERN "*.h*")
endif()
