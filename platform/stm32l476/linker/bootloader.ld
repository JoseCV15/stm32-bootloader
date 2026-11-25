
ENTRY(Reset_Handler)

/* STM32L476RG
 *
 * Bootloader:
 *   Flash: 32 KB
 *   SRAM1: 96 KB
 *
 * Application starts at 0x08008000.
 */

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 32K
    SRAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 96K
}

/* Initial stack pointer: top of SRAM1 */
_estack = ORIGIN(SRAM) + LENGTH(SRAM);

SECTIONS
{
    /* Start Flash sections at the beginning of bootloader Flash */
    . = ORIGIN(FLASH);

    /* ---------------------------------------------------------
     * Interrupt vector table
     * --------------------------------------------------------- */
    .isr_vector :
    {
        . = ALIGN(4);

        __isr_vector_start = .;

        KEEP(*(.isr_vector))
        KEEP(*(.isr_vector.*))

        . = ALIGN(4);

        __isr_vector_end = .;
    } > FLASH

    /* ---------------------------------------------------------
     * Program code
     * --------------------------------------------------------- */
    .text :
    {
        . = ALIGN(4);

        *(.text)
        *(.text.*)

        *(.glue_7)
        *(.glue_7t)

        . = ALIGN(4);
    } > FLASH

    /* ---------------------------------------------------------
     * Read-only data
     * --------------------------------------------------------- */
    .rodata :
    {
        . = ALIGN(4);

        *(.rodata)
        *(.rodata.*)

        *(.constdata)
        *(.constdata.*)

        . = ALIGN(4);
    } > FLASH

    /* ---------------------------------------------------------
     * Pre-initialization array
     * --------------------------------------------------------- */
    .preinit_array :
    {
        . = ALIGN(4);

        PROVIDE_HIDDEN(__preinit_array_start = .);

        KEEP(*(.preinit_array))
        KEEP(*(.preinit_array.*))

        PROVIDE_HIDDEN(__preinit_array_end = .);

        . = ALIGN(4);
    } > FLASH

    /* ---------------------------------------------------------
     * Initialization array
     * --------------------------------------------------------- */
    .init_array :
    {
        . = ALIGN(4);

        PROVIDE_HIDDEN(__init_array_start = .);

        KEEP(*(.init_array))
        KEEP(*(.init_array.*))

        PROVIDE_HIDDEN(__init_array_end = .);

        . = ALIGN(4);
    } > FLASH

    /* ---------------------------------------------------------
     * Finalization array
     * --------------------------------------------------------- */
    .fini_array :
    {
        . = ALIGN(4);

        PROVIDE_HIDDEN(__fini_array_start = .);

        KEEP(*(.fini_array))
        KEEP(*(.fini_array.*))

        PROVIDE_HIDDEN(__fini_array_end = .);

        . = ALIGN(4);
    } > FLASH

    /* ---------------------------------------------------------
     * Initialized data
     *
     * VMA: SRAM (where variables execute)
     * LMA: FLASH (where initial values are stored)
     * --------------------------------------------------------- */

    .data :
    {
        . = ALIGN(4);

        _sdata = .;

        *(.data)
        *(.data.*)

        . = ALIGN(4);

        _edata = .;
    } > SRAM AT > FLASH

    /* Flash address containing the initial .data values */
    _sidata = LOADADDR(.data);

    /* ---------------------------------------------------------
     * Zero-initialized data
     * --------------------------------------------------------- */
    .bss (NOLOAD) :
    {
        . = ALIGN(4);

        _sbss = .;

        *(.bss)
        *(.bss.*)

        *(COMMON)

        . = ALIGN(4);

        _ebss = .;
    } > SRAM

    /* ---------------------------------------------------------
     * No-init data
     *
     * This section is not cleared by startup code.
     * --------------------------------------------------------- */
    .noinit (NOLOAD) :
    {
        . = ALIGN(4);

        _snoinit = .;

        *(.noinit)
        *(.noinit.*)

        . = ALIGN(4);

        _enoinit = .;
    } > SRAM

    /* ---------------------------------------------------------
     * Heap / end of static SRAM allocation
     * --------------------------------------------------------- */
    . = ALIGN(8);

    end = .;
    _end = .;

    /* ---------------------------------------------------------
     * Linker assertions
     * --------------------------------------------------------- */

    ASSERT(
        _end <= ORIGIN(SRAM) + LENGTH(SRAM),
        "ERROR: SRAM overflow"
    )

    ASSERT(
        __isr_vector_start == ORIGIN(FLASH),
        "ERROR: Vector table is not at Flash origin"
    )

    ASSERT(
        __isr_vector_end <= ORIGIN(FLASH) + LENGTH(FLASH),
        "ERROR: Vector table exceeds bootloader Flash"
    )

    ASSERT(
        LOADADDR(.data) + SIZEOF(.data) <=
        ORIGIN(FLASH) + LENGTH(FLASH),
        "ERROR: Bootloader Flash overflow (including .data)"
    )

    ASSERT(
        SIZEOF(.text) + SIZEOF(.rodata) <= LENGTH(FLASH),
        "ERROR: Bootloader code/data exceeds Flash"
    )

    /* ---------------------------------------------------------
     * Discard unused metadata sections
     * --------------------------------------------------------- */
    /DISCARD/ :
    {
        *(.ARM.exidx*)
        *(.gnu.linkonce.armexidx.*)
        *(.ARM.attributes)
        *(.comment)
        *(.debug_frame)
    }
}
