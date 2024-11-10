@echo off
setlocal

rem Compile the kernel source code to an object file
gcc -O0 -m32 -ffreestanding -c kernel.c -o kernel.o

rem Link the object file to create a PE executable for the kernel
ld -m i386pe -O0 -nostdlib -T link.ld kernel.o -o kernel.exe

rem Convert the PE file to a binary file
objcopy --strip-all -O binary kernel.exe kernel.bin

endlocal
