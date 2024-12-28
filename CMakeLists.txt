cmake_minimum_required(VERSION 3.27)

project(kernel C CXX ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(STATUS "${CMAKE_TOOLCHAIN_FILE}")

set(CMAKE_CXX_STANDARD 23)

set(FLAGS_COMMON "-fno-PIC -W -Wall -ffreestanding -mno-red-zone -nostdinc -static-libgcc -l${CMAKE_CURRENT_LIST_DIR}/src/Libraries/libgcc-i386.a")
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

add_subdirectory(src/Arch/x86)
add_subdirectory(src/Kernel)
add_subdirectory(src/Libraries)

include_directories(src/Libraries)

get_property(SOURCES GLOBAL PROPERTY SourceList)
add_executable(${KERNEL_BIN} ${SOURCES})
set_target_properties(${KERNEL_BIN}
  PROPERTIES LINK_FLAGS "-m32 -march=i386 -T ${CMAKE_CURRENT_LIST_DIR}/link.ld -ffreestanding -O2 -nostdlib")
add_custom_command(
  TARGET ${KERNEL_BIN}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin
  COMMAND ${CMAKE_COMMAND} -E copy ${KERNEL_BIN} ${CMAKE_SOURCE_DIR}/bin/)

# # Generating the ISO file
add_custom_command(
  OUTPUT ${KERNEL_ISO}
  COMMAND ../generate-iso.sh
  COMMENT "Generating the kernel bootable iso file"
  DEPENDS ${KERNEL_BIN})
add_custom_target(GenerateISO ALL DEPENDS ${KERNEL_ISO})
add_custom_command(
  TARGET GenerateISO
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin
  COMMAND ${CMAKE_COMMAND} -E copy ${KERNEL_ISO} ${CMAKE_SOURCE_DIR}/bin/)
