OUTPUT_FORMAT("elf32-i386")
ENTRY(kernel_entry)

SECTIONS {
    . = 0x100000;           /* or 1 MiB — very common safe place */

    .text : { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    .bss : { *(.bss) *(COMMON) }
}