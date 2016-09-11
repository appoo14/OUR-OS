
myos.bin: boot.o keyboard.o kernel.o
	i686-elf-gcc -T linker.ld -o myos.bin -ffreestanding -O2 -nostdlib boot.o keyboard.o kernel.o -lgcc

boot.o: boot.asm
	nasm -felf32 boot.asm -o boot.o

keyboard.o: keyboard.c
	i686-elf-gcc -c keyboard.c -o keyboard.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

kernel.o: kernel.c
	i686-elf-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
