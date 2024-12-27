ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)

SECTIONS {
    . = 0x10000;

    .text BLOCK(4K) : ALIGN(4K) {
/*        code = .; _code = .; __code = .;*/
        *(.multiboot)
        *(.text)
    }

    .data BLOCK(4K) : ALIGN(4K) {
/*        __CTOR_LIST__ = .; LONG((__CTOR_END__ - __CTOR_LIST__) / 4 - 2) *(.ctors) LONG(0) __CTOR_END__ = .;*/
/*        __DTOR_LIST__ = .; LONG((__DTOR_END__ - __DTOR_LIST__) / 4 - 2) *(.dtors) LONG(0) __DTOR_END__ = .;*/
/*        data = .; _data = .; __data = .;*/
        *(.data)
    }

    .rodata BLOCK(4K) : ALIGN(4K) {
/*        rodata = .; _rodata = .; __rodata = .;*/
        *(.rodata)
    }

    .bss BLOCK(4K) : ALIGN(4K) {
/*        bss = .; _bss = .; __bss = .;*/
        *(COMMON)
        *(.bss)
    }

/*    end = .; _end = .; __end = .;*/
}