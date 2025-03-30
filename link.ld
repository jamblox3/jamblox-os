ENTRY(__main)  /* Ensure this points to your kernel entry function */

SECTIONS
{
    . = 0x7E00;  /* Set the starting address for the kernel */

    .text ALIGN(4): {
        *(.text)  /* Include all .text sections */
    }

    .data ALIGN(4): {
        *(.data)  /* Include all .data sections */
    }

    .bss ALIGN(4): {
        *(.bss)  /* Include all .bss sections */
    }

    /DISCARD/ : {
        *(.comment)
        *(.debug)
        *(.note)
        *(.reloc)
    }
}
