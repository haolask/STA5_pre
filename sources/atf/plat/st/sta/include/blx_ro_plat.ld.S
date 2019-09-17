        /* Ensure 8-byte alignment. */
        . = ALIGN(8);
        __mmap_regions_start = .;
        KEEP(*(.mmap_regions*))
        __mmap_regions_end = .;
