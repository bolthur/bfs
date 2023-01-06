# Name of the target
set(CMAKE_SYSTEM_NAME Generic)
set(SYSTEM_TYPE bolthur)

add_definitions(-D_GNU_SOURCE=1)
# bunch of processor specific options
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv6zk -mtune=arm1176jzf-s -mfpu=vfpv2 -mfloat-abi=hard")
set(CMAKE_EXE_LINKER_FLAGS, "${CMAKE_EXE_LINKER_FLAGS} -march=armv6zk -mtune=arm1176jzf-s")

# Toolchain settings
set(CMAKE_C_COMPILER arm-raspi0_1-bolthur-eabi-gcc)
#set(CMAKE_CXX_COMPILER arm-raspi0_1-bolthur-eabi-g++)
set(AS arm-raspi0_1-bolthur-eabi-as)
set(AR arm-raspi0_1-bolthur-eabi-ar)
set(OBJCOPY arm-raspi0_1-bolthur-eabi-objcopy)
set(OBJDUMP arm-raspi0_1-bolthur-eabi-objdump)

# test active flag
set(TEST_ACTIVE OFF)
