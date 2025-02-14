add_sources(
  Common.hpp
  CMOS.cpp CMOS.hpp
  Disk.hpp Disk.cpp
  Drivers/Serial.cpp Drivers/Serial.hpp
  Drivers/VGA.cpp Drivers/VGA.hpp
  icxxabi.cpp icxxabi.hpp
  Interrupts/Interrupts.cpp Interrupts/Interrupts.hpp
  Interrupts/IrqHandler.cpp Interrupts/IrqHandler.hpp
  IO.cpp IO.hpp
  Kernel.cpp
  kmalloc.cpp kmalloc.hpp
  kprintf.cpp kprintf.hpp
  MemoryManager.cpp MemoryManager.hpp
  Multiboot.hpp
  PIC.cpp PIC.hpp
  PIT.cpp PIT.hpp
  Process.cpp Process.cpp
  RTC.cpp RTC.hpp
  symbol.h
)
