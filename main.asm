org 0x7c00
bits 16

jmp main                        ;avoid executing data

gdt_start:
    ; Null descriptor
    dd 0x00000000,0x00000000        ; Null descriptor
    dd 0x0000ffff,0x00cb9a00        ; Kernel Code Segment (base: 0x00000000, limit: 1 GiB)
    dd 0x0000ffff,0x00cb9200        ; Kernel Data Segment (base: 0x00000000, limit: 1 GiB)
    dd 0x0000ffff,0x40cffa00        ; User Code Segment (base: 0x40000000, limit: 4 GiB)
    dd 0x0000ffff,0x40cff200        ; User Data Segment (base: 0x40000000, limit: 4 GiB)
gdt_end:

gdt_desc:
    dw gdt_end-gdt_start-1
    dd gdt_start

disk_packet:
    db 0x10                     ; Size of packet (16 bytes)
    db 0                        ; Reserved
    dw krnl_end-krnl_start      ; Number of sectors to read
    dw 0x7E00                   ; Target buffer segment (ES:BX)
    dw 0x0000                   ; Target buffer offset
    dq krnl_start
krnl_start equ 593
krnl_end equ 600

main:
    mov ah, 0x42                ; read from disk
    mov dl, 0xE0                ; select  disk
    mov si, disk_packet         ; use packet to load kernel
    int 0x13                    ; issue interupt

    cli                         ; disable interupts beucase i dont have an idt

    mov ah, 0x01
    mov al, 0x20
    int 0x10                    ; Set cursor to be invisible
    ;mov ax, 0x6a
    ;int 10h
    ;mov ax, 0x4F02	; set display grapics (not being used for debugging reasons)
    ;mov bx, 0x4115
    ;int 0x10
    lgdt [gdt_desc]     ; Load GDT

    mov eax, cr0
    or eax, 1           ; change to 32bit mode
    mov cr0, eax

    jmp 0x08:setup     ; Far jump to the 32-bit code segment

bits 32

setup:

    mov ax, 0x10        ; Kernel Data Segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x10000    ; Set up stack 0xc0100000 may change
    mov esi, 0x7e00
    mov edi, 0xb8000
    
    in al, 0x92    ;enable a20 line so i can make drivers
    or al, 2
    out 0x92, al

    call 0x7E00
    
end:
    jmp end

times 510 - ($ - $$) db 0
dw 0xaa55               ; Boot signature