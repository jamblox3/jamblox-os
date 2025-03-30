@echo off
setlocal

rem Compile the kernel source code to an object file
"C:\gitlnk\zip programs\mingw32\bin\gcc" -O0 -m32 -ffreestanding -fno-stack-protector -c kernel.c -o kernel.o

rem Link the object file to create a PE executable for the kernel
"C:\gitlnk\zip programs\mingw32\bin\ld" -m i386pe -O0 -nostdlib -T link.ld kernel.o -o kernel.exe

rem Convert the PE file to a binary file
"C:\gitlnk\zip programs\mingw32\bin\objcopy" --strip-all -O binary kernel.exe kernel.bin

rem Copy the binary to the desired location
copy kernel.bin "c:\gitlnk\jamblox os\build\boot"

endlocal
