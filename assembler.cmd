@echo off
nasm -f bin boot.asm -o boot.bin
copy boot.bin "c:\gitlnk\jamblox os\build\boot"
