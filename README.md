# kernel

## Prerequisites

- CMake (3.27+)
- x86-64 ELF Compiler Toolchain
  - For Linux: see [here](./toolchain/build-toolchain.sh)
  - For MacOS: `brew install x86_64-elf-gcc`

## Build & Run

### Linux (Not tested since before MacOS support)

```bash
cmake -DCMAKE_TOOLCHAIN_FILE=./toolchain/toolchain.cmake -B build
cmake --build build
./run-iso.sh
```

### MacOS

```bash
cmake -DCMAKE_TOOLCHAIN_FILE=./toolchain/apple-toolchain.cmake -B build
cmake --build build
./run-iso.sh
```
