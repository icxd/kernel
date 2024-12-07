.set MB_MAGIC,       0x1BADB002
.set MB_PAGE_ALIGN,  1 << 0
.set MB_MEMORY_INFO, 1 << 1
.set MB_FLAGS,       MB_PAGE_ALIGN | MB_MEMORY_INFO
.set MB_CHECKSUM,    -(MB_MAGIC + MB_FLAGS)

/* multiboot header */
.section .multiboot
.align 4
    .long MB_MAGIC
    .long MB_FLAGS
    .long MB_CHECKSUM

    .long 0x00000000 /* header_addr */
    .long 0x00000000 /* load_addr */
    .long 0x00000000 /* load_end_addr */
    .long 0x00000000 /* bss_end_addr */
    .long 0x00000000 /* entry_addr */

.section .stack, "aw", @nobits
stack_bottom:
    .skip 16384 # 16 KiB
stack_top:

.section .page_tables
.align 4096
page_tables_start:
.skip 4096 * 3

.extern kmain
.section .text
.global _start
_start:
    /* set up stack */
    mov $stack_top, %esp

    /* reload CS */
#     ljmp $0x08, $fix_cs
# fix_cs:
#     mov $0x10, %ax
#     mov %ax, %ds
#     mov %ax, %es
#     mov %ax, %fs
#     mov %ax, %gs
#     mov %ax, %ss

    /* call kmain with multiboot info */
    push %ebx
    push %eax
    call kmain

.loop:
    hlt
    jmp .loop

/* set_gdt(limit, base, offset) */
# .code32
# set_gdt:
#     mov 4(%esp), %ax
#     mov %ax, $gdt_descriptor
#     mov 8(%esp), %eax
#     add 12(%esp), %eax
#     mov %eax, 2($gdt_descriptor)
#     lgdt $gdt_descriptor
#     ret

# .section .data
# .code32
# gdt:
# gdt_null:
#     .word 0
#     .word 0
#     .byte 0
#     .byte 0
#     .byte 0
#     .byte 0
#
# gdt_code:
#     .word 0xffff     /* limit (bits 0-15) */
#     .word 0          /* base (bits 0-15) */
#     .byte 0          /* base (bits 16-23) */
#     .byte 0b10011010 /* access byte */
#     .byte 0b11001111 /* flags, limit (bits 16-19) */
#     .byte 0          /* base (bits 24-31) */
#
# gdt_data:
#     .word 0xffff
#     .word 0
#     .byte 0
#     .byte 0b10010010
#     .byte 0b11001111
#     .byte 0
# gdt_end:
#
# .section .data
# gdt_descriptor:
#     .word 0
#     .long 0

# .size _start, . - _start
