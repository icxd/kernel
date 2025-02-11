cmake_minimum_required(VERSION 3.27)

project(kernel C CXX ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(STATUS "${CMAKE_TOOLCHAIN_FILE}")

set(CMAKE_CXX_STANDARD 23)

set(FLAGS_COMMON "-g -fno-PIC -W -Wall -ffreestanding -mno-red-zone -nostdinc -static-libgcc -L${CMAKE_CURRENT_LIST_DIR}/libgcc-i386.a")
set(CMAKE_C_FLAGS "${FLAGS_COMMON} -m32")
set(CMAKE_CXX_FLAGS "${FLAGS_COMMON} -m32 -fno-exceptions -fno-rtti -fuse-cxa-atexit -fno-threadsafe-statics")
set(CMAKE_ASM_FLAGS "${FLAGS_COMMON} -m32")

# Debug flags
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C Flags: ${CMAKE_C_FLAGS}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ Flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "ASM Compiler: ${CMAKE_ASM_COMPILER}")
message(STATUS "ASM Flags: ${CMAKE_ASM_FLAGS}")

function(add_sources)
  foreach(s IN ITEMS ${ARGV})
    get_filename_component(a "${s}" ABSOLUTE)
    set_property(GLOBAL APPEND PROPERTY SourceList "${a}")
  endforeach()
endfunction()

set(KERNEL_BIN "kernel.bin")
set(KERNEL_ISO "kernel.iso")
set(KERNEL_PRELINKED "kernel_prelinked.bin")

add_subdirectory(src/Arch/x86)
add_subdirectory(src/Kernel)
add_subdirectory(src/Libraries)

include_directories(src/Libraries)

get_property(SOURCES GLOBAL PROPERTY SourceList)
add_executable(${KERNEL_PRELINKED} ${SOURCES})
set_target_properties(${KERNEL_PRELINKED}
  PROPERTIES LINK_FLAGS "-m32 -march=i386 -T ${CMAKE_CURRENT_LIST_DIR}/link.ld -ffreestanding -O2 -nostdlib -g")
add_custom_command(
  TARGET ${KERNEL_PRELINKED}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin
  COMMAND ${CMAKE_COMMAND} -E copy ${KERNEL_PRELINKED} ${CMAKE_SOURCE_DIR}/bin/)

add_custom_command(
  COMMENT "Generate kernel symbol table"
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/src/Kernel/symbol.gen.c
  COMMAND sh ${CMAKE_CURRENT_LIST_DIR}/src/Kernel/symbol_gen.sh ${KERNEL_PRELINKED} ${CMAKE_CURRENT_LIST_DIR}/src/Kernel/symbol.gen.c
  DEPENDS ${KERNEL_PRELINKED})
add_custom_command(
  COMMENT "Compile kernel symbol table"
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/src/Kernel/symbol.gen.o
  COMMAND ${CMAKE_C_COMPILER} -fno-PIC -W -Wall -ffreestanding -mno-red-zone -nostdinc -static-libgcc -L${CMAKE_CURRENT_LIST_DIR}/libgcc-i386.a -m32 -c ${CMAKE_CURRENT_LIST_DIR}/src/Kernel/symbol.gen.c -o ${CMAKE_CURRENT_LIST_DIR}/src/Kernel/symbol.gen.o
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/src/Kernel/symbol.gen.c)

# Step 3: Create a custom target that ensures symbol.gen.o is built
add_custom_target(symbol_table DEPENDS ${CMAKE_CURRENT_LIST_DIR}/src/Kernel/symbol.gen.o)

# Step 4: Final linking step that includes symbol_table dependency
add_executable(${KERNEL_BIN} ${SOURCES} ${CMAKE_CURRENT_LIST_DIR}/src/Kernel/symbol.gen.o)
add_dependencies(${KERNEL_BIN} ${KERNEL_PRELINKED} symbol_table)
set_target_properties(${KERNEL_BIN}
  PROPERTIES LINK_FLAGS "-m32 -march=i386 -T ${CMAKE_CURRENT_LIST_DIR}/link.ld -ffreestanding -O2 -nostdlib -g")

# Generating the ISO file
add_custom_command(
  OUTPUT ${CMAKE_SOURCE_DIR}/bin/${KERNEL_BIN}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin
  COMMAND ${CMAKE_COMMAND} -E copy ${KERNEL_BIN} ${CMAKE_SOURCE_DIR}/bin/${KERNEL_BIN}
  DEPENDS ${KERNEL_BIN}
  COMMENT "Copying kernel binary to output directory")
add_custom_target(copy_kernel_bin ALL DEPENDS ${CMAKE_SOURCE_DIR}/bin/${KERNEL_BIN})

add_custom_command(
  OUTPUT ${KERNEL_ISO}
  COMMAND ../generate-iso.sh
  COMMENT "Generating the kernel bootable iso file"
  DEPENDS ${KERNEL_BIN})
add_custom_target(GenerateISO ALL DEPENDS ${KERNEL_ISO} copy_kernel_bin)
add_custom_command(
  TARGET GenerateISO
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin
  COMMAND ${CMAKE_COMMAND} -E copy ${KERNEL_ISO} ${CMAKE_SOURCE_DIR}/bin/)
