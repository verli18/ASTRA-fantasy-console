/* linker.lds */
ENTRY(_start)

SECTIONS {
  . = 0x80000000;            /* Base address of your console’s flash */
  .text : { *(.text) }       /* All code */
  .rodata : { *(.rodata) }   /* Read-only data */
  .data : { *(.data) }       /* Initialized data */
  .bss : { *(.bss COMMON) }  /* Zeroed memory */
  . = ALIGN(4);
  __stack_top = . + 0x1000;  /* Reserve 4 KB for stack */
}
