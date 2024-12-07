add_sources(
        Common.hpp
        Drivers/Serial.cpp Drivers/Serial.hpp
        Drivers/VGA.cpp Drivers/VGA.hpp
        icxxabi.cpp icxxabi.hpp
        Interrupts/Interrupts.cpp Interrupts/Interrupts.hpp
        Interrupts/IrqHandler.cpp Interrupts/IrqHandler.hpp
        IO.cpp IO.hpp
        Kernel.cpp
        kmalloc.cpp kmalloc.hpp
        kprintf.cpp kprintf.hpp
        Multiboot.hpp
        PIC.cpp PIC.hpp
        Symbol.cpp Symbol.hpp
)