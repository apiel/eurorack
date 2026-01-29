/* Minimal linker script for STM32H723 */
ENTRY(main)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 2048K
  RAM   (rwx) : ORIGIN = 0x24000000, LENGTH = 1024K
}

SECTIONS
{
  .text :
  {
    KEEP(*(.isr_vector))   /* Interrupt vectors */
    *(.text*)              /* Code */
    *(.rodata*)            /* Read-only data */
  } > FLASH

  .data : { *(.data*) } > RAM
  .bss  : { *(.bss*) } > RAM
}
