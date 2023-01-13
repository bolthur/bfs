
# hack around for pthread not found
if("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
  set(CMAKE_THREAD_LIBS_INIT "-lpthread")
  set(CMAKE_HAVE_THREADS_LIBRARY 1)
  set(CMAKE_USE_WIN32_THREADS_INIT 0)
  set(CMAKE_USE_PTHREADS_INIT 1)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()

# Toolchain settings
set(CMAKE_C_COMPILER /usr/bin/clang)
set(AS llvm-as)
set(AR llvm-ar)
set(OBJCOPY llvm-objcopy)
set(OBJDUMP llvm-objdump)

# include what you use
set(CMAKE_C_INCLUDE_WHAT_YOU_USE "include-what-you-use")

# test active flag
set(TEST_ACTIVE ON)
