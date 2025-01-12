ENTRY(__main)  /* Ensure this points to your kernel entry function */

SECTIONS
{
    . = 0x7E00;  /* Set the starting address for the kernel */

    .text : {
        *(.text)  /* Include all .text sections */
    }

    .data : {
        *(.data)  /* Include all .data sections */
    }

    .bss : {
        *(.bss)  /* Include all .bss sections */
    }

    /DISCARD/ : {
        *(.comment)
        *(.debug)
        *(.note)
        *(.reloc)
    }
}
