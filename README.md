# BFS

## About

Goal of this project is to provide implementation of various file systems.

## Compile

There are various possibilities to compile this project. Below a few ones are outlined to show how the project is structured.

### Generic compilation

```bash
mkdir build
cd build
cmake -G"Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=../toolchain/generic-gcc.cmake ..
```

### Compilation including unit tests

```bash
mkdir build
cd build
cmake -G"Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=../toolchain/generic-test-gcc.cmake ..
```

### Compilation with specific toolchain

```bash
mkdir build
cd build
cmake -G"Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=../toolchain/arm-raspi2b_r1-bolthur-eabi.cmake ..
```
