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
read_size:    dw 1      ; Number of sectors to read
    dw 0x7E00                   ; Target buffer segment (ES:BX)
    dw 0x0000 
    segment_number:
    dd 0                  ; Target buffer offset
    dd 0

main:
    mov ecx, 0x0000

disk_loop:
    mov di, segment_number
    mov [di], ecx
    call try_disks
    inc ecx
    cmp ecx, 600
    jne disk_loop
    mov ax, 0x0e42
    mov bx, 0
    int 0x10
    jmp end

try_disks:
    mov bx, 0xe0                ; use bx to keep track of wich disk ah dl and si will likely be edited byt the interupt
    mov ah, 0x42                ; read from disk
    mov dl, 0xE0                ; select  disk
    mov si, disk_packet         ; use packet to load kernel
    int 0x13                    ; issue interupt
    call check
    mov bx, 0xe1
    mov ah, 0x42                ; read from disk
    mov dl, 0xE1                ; select  disk
    mov si, disk_packet         ; use packet to load kernel
    int 0x13                    ; issue interupt
    call check
    mov bx, 0x80
    mov ah, 0x42                ; read from disk
    mov dl, 0x80                ; select  disk
    mov si, disk_packet         ; use packet to load kernel
    int 0x13                    ; issue interupt
    call check
    mov bx, 0x81
    mov ah, 0x42                ; read from disk
    mov dl, 0x81                ; select  disk
    mov si, disk_packet         ; use packet to load kernel
    int 0x13                    ; issue interupt
    call check
    ret

check:
    push bx
    mov ebx, [0x7e00]
    mov eax, [0x7e04]
    cmp ebx, 0x7f409dd4
    jne no
    cmp eax, 0xede4e9fa ;test for the signature
    jne no
    pop bx ;get old bx back
    pop ax ;get rid of return addresses
    pop ax
    jmp sucess
no:
    pop bx
    ret

sucess:
    mov ecx, 0

scan_for_kernel:
    mov di, read_size
    mov word [di], 0x0008
    mov di, segment_number
    mov [di], ecx
    mov ah, 0x42
    mov dl, bl
    mov si, disk_packet
    int 0x13
    jc disk_error
    call check_kernel 
    inc ecx
    cmp ecx, 600
    jne scan_for_kernel
    mov ax,0x0e42
    mov bx, 0
    int 0x10
    jmp end

check_kernel:
    push bx
    mov eax, [0x7e00]
    mov ebx, [0x7e04]
    cmp eax, 0x83e58955
    jne no
    and ebx, 0x00ffffff
    cmp ebx, 0x00e808ec
    jne no
    pop bx
    pop ax ;get rid of return address

found_kernel:

    cli                         ; disable interupts beucase i dont have an idt

    mov ah, 0x01
    mov cx, 0x2607
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

disk_error:
    mov ax, 0x0e45
    mov bx, 0
    int 0x10

end:
    jmp $

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

    jmp $

times 510 - ($ - $$) db 0
dw 0xaa55               ; Boot signature