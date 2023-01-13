# Name of the target
set(CMAKE_SYSTEM_NAME Generic)
set(SYSTEM_TYPE bolthur)

add_definitions(-D_GNU_SOURCE=1)
# bunch of processor specific options
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv7-a -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard")
set(CMAKE_EXE_LINKER_FLAGS, "${CMAKE_EXE_LINKER_FLAGS} -march=armv7-a -mtune=cortex-a7")

# Toolchain settings
set(CMAKE_C_COMPILER arm-raspi2b_r1-bolthur-eabi-gcc)
set(AS arm-raspi2b_r1-bolthur-eabi-as)
set(AR arm-raspi2b_r1-bolthur-eabi-ar)
set(OBJCOPY arm-raspi2b_r1-bolthur-eabi-objcopy)
set(OBJDUMP arm-raspi2b_r1-bolthur-eabi-objdump)

# test active flag
set(TEST_ACTIVE OFF)
