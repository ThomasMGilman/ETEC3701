
CC_EXE?=gcc
NASM_EXE?=nasm
#elf, win32, or macho32
NASMPLATFORM?=win32
LD_EXE?=ld
#elf_i386 or i386pe
LDPLATFORM?=i386pe
QEMU?=/cygdrive/c/program files/qemu/qemu-system-i386
MKE2FS?=/usr/sbin/mke2fs
DEBUGFS?=/usr/sbin/debugfs
OBJCOPY?=objcopy
OPTIMIZE?=-O0											#Turning on Optimization will flush out your poor coding skillz
DD?=dd
TRUNCATE?=truncate

############################################
#Don't change these definitions
#fno-builtin to prevent builtin functions
#mgeneral-regs-only to prevent use of xmm (so we can use attribute(__interrupt__))
#nostdinc and nostdlib to prevent use of standard headers/libs
#fleading_underscore so we can use same code from Linux, OSX, and Windows (using OSX/Windows convention)
CFLAGS:=$(OPTIMIZE) \
	-masm=intel \
	-m32 \
	-Wall -Werror \
	-fno-builtin \
	-mgeneral-regs-only \
	-nostdinc -nostdlib -fleading-underscore \
	-ffreestanding
CC=$(CC_EXE) $(CFLAGS)
NASM:=$(NASM_EXE) -f $(NASMPLATFORM)
LD:=$(LD_EXE) -m$(LDPLATFORM)
############################################
all:

	$(NASM) -f bin bootblock.s -o bootblock.bin
	$(NASM) kernelasm.s -o kernelasm.o
	$(CC)	-c	kernelmain.c
	$(CC)	-c	stdarg.c
	$(CC)	-c	console.c
	$(CC)	-c	util.c
	$(CC)	-c	kprintf.c
	$(CC)	-c	disk.c
	$(CC)	-c	file.c
	$(CC)	-c	interrupt.c
	$(CC)	-c	testsuite.c
	$(LD) -Map kernelmap.txt -T linkerscript.txt -o kernel.tmp \
		kernelasm.o	\
		kernelmain.o \
		stdarg.o	\
		console.o \
		util.o	\
		kprintf.o	\
		disk.o	\
		file.o	\
		interrupt.o	\
		testsuite.o
	$(OBJCOPY) -Obinary kernel.tmp kernel.bin
	$(NASM)	crtasm.s
	$(CC)	-c	crtc.c
	$(CC)	-c	usertest1.c
	$(LD)	-T	linkerscript2.txt	-o	usertest1.tmp	usertest1.o
	$(OBJCOPY)	-Obinary	usertest1.tmp	usertest1.bin
	$(TRUNCATE) -s 400000000 hd.img
	$(MKE2FS) -b 4096 -F -I 128 -q -t ext2 -r 0 -L moocow -g 32768 hd.img
	$(DD) bs=512 conv=notrunc if=bootblock.bin of=hd.img count=2
	$(DD) bs=512 conv=notrunc if=bootblock.bin of=hd.img seek=4 skip=4
	$(DEBUGFS) -w -f fscmd.txt hd.img
	"$(QEMU)" -d unimp,guest_errors -serial stdio  -monitor vc:80Cx27C \
		-drive file=hd.img,index=0,media=disk,format=raw -soundhw pcspk 2>&1 

objdump:
	objdump --adjust-vma=0x100000 -D -m i386 -M intel -b binary kernel.bin | less

clean: 
	-/bin/rm $(wildcard *.o) $(wildcard *.exe) $(wildcard *.img) kernelmap.txt $(wildcard *.tmp) $(wildcard *.bin)

zip:
	zip lab.zip $(wildcard *.s) $(wildcard *.c) $(wildcard *.h) $(wildcard *.txt) Makefile
