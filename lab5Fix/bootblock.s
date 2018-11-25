
org 0x7c00


; Memory map:
;   0000... ???? -> bios
;   ????... 7bff -> stack
;   7c00... 8bff -> this code (bootblock) 
;   8c00... 9fff -> absolute labels (like a bss, defined at end of this file)
;   a000... afff -> READ_INODE_TEMP_BUFFER
;   b000... bfff -> INDIRECT_BLOCK
;   c000... cfff -> DINDIRECT_BLOCK
;   d000... dfff -> TMPBUFF
;   e000... ffff -> Unused
;  10000...7ffff -> First part of kernel is loaded here for multiboot header search
;  80000...8ffff -> Used for getting VESA info chunk
;  90000...9ffff -> Used for getting video mode info
;  a0000...fffff -> BIOS/Adapter RAM
; 100000...????? -> Free

;initial value of esp
%define STACK_ADDRESS 0x7c00

;buffer for loading root inode + root directory
;needs to be 4KB long (0x1000 bytes)
%define TMPBUFF 0xd000

;temporary buffer for read_inode.
;needs to be 512 bytes long
%define READ_INODE_TEMP_BUFFER 0xa000

;temporary buffer (4KB) for indirect block
%define INDIRECT_BLOCK 0xb000

;temporary buffer (4KB) for double indirect block
%define DINDIRECT_BLOCK 0xc000

;segment for getting vesa info
; Addresses used will be 0x80000...0x8ffff
%define VESA_INFO_SEGMENT 0x8000

;segment for getting video mode info
; Addresses used will be 0x90000...0x9ffff
%define VIDEO_MODE_SEGMENT 0x9000

;temporary place to load first part of kernel while
;searching for multiboot header
%define TEMP_KERNEL_LOAD 0x10000

bootstart:

    bits 16

    ;disable interrupts and clear direction flag
    cli
    cld

    ;set up stack
    mov esp,STACK_ADDRESS

    ;get amount of RAM in system
    mov ax,0xe801
    int 0x15
    ;bx has amount of 64KB segments in excess of 16MB
    and ebx,0xffff              ;mask upper bits
    shl ebx, 16                 ;convert to bytes
    add ebx, 16777216           ;get total amount of RAM
    mov [totalMemory], ebx
    

    ;transfer to protected mode so we can
    ;load to high memory without
    ;segmented addressing
    call go32

    bits 32
    
    ;load the rest of the boot block
    ;load sector 1 -> 0x7e00
    ;skip sector 2 -> 0x8000
    ;skip sector 3 -> 0x8200
    ;load sector 4 -> 0x8400
    ;load sector 5 -> 0x8600
    ;load sector 6 -> 0x8800
    ;load sector 7 -> 0x8a00
    
    mov ecx,1
    mov ebx,0x7e00
    loadSectors:
        push ebx
        push ecx
        call read_sector
        add esp,8
        inc ecx
        add ebx,512
        cmp ecx,8
        jne loadSectors
    
    ;jump to next piece of boot block
    jmp sector_4

    
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
go32:
    bits 16
    ;stack has 16 bit return address on it,
    ;but when we do the ret, we will be
    ;in 32 bit mode
    ;So we must add 2 bytes to the stack.
    ;Since x86 is little endian, we need to
    ;push a zero UNDER the stack top
    pop word [go32temp]
    push word 0
    push word [go32temp] 
    
    lgdt [GDT32pointer] ;0x7c00

    ;turn on PE bit
    mov eax,cr0
    or eax,1
    mov cr0,eax

    ;clear prefetch queue: intel says this
    ;is required
    jmp flush32a
flush32a:
    nop         
    
    ;load segment registers with 32 bit data segments
    mov ax,8
    mov ss,ax
    mov ds,ax
    mov es,ax

    ;reload cs register with code32 segment
    jmp dword 0x10:change_to_pmode_32 ;0x7c00
    
change_to_pmode_32:
    bits 32
    ret
    
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; void print(char* p)
print:
    pusha
    mov ebx,[esp+36]
    mov ecx,[cursor]
    mov eax,0           ;number of characters written
    printloop:
        mov dl,[ebx]
        cmp dl,0
        je endprintloop
        mov [ecx],dl
        inc ecx
        inc ebx
        inc eax
        mov byte [ecx],0xf0
        inc ecx
        jmp printloop
    endprintloop:

    mov edx,80
    sub edx,eax
    add ecx,edx
    add ecx,edx
    mov [cursor],ecx
    popa
    ret

cursor:
    dd 0xb8000

diskErrorMessage:
    db 'Disk error ',0

;needs to be in first 512 bytes
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; void read_sector( sectorNumber, destinationAddress )
%define PORT_DATA 0x1f0
%define PORT_ERROR_FEATURES 0x1f1
%define PORT_SECTORCOUNT 0x1f2
%define PORT_LBALOW 0x1f3
%define PORT_LBAMID 0x1f4
%define PORT_LBAHIGH 0x1f5
%define PORT_FLAGS 0x1f6
%define PORT_COMMANDSTATUS 0x1f7
%define PORT_ALTSTATUS 0x3f6

%define STATUS_ERROR_MASK 1
%define STATUS_READY_MASK (1<<3)
%define STATUS_OVERLAPPED (1<<4)
%define STATUS_DRIVEFAULT (1<<5)
%define STATUS_DRIVEREADY (1<<6)
%define STATUS_DRIVEBUSY (1<<7)

%define CONTROL_INTERRUPT_INHIBIT (1<<1)
%define CONTROL_SOFTWARE_RESET (1<<2)

read_sector:
    ;read sector
    pusha

    ;ecx = sector number
    ;ebx = destination address
    mov ecx, [esp+36]
    mov ebx, [esp+40]
    
    ;wait for disk controller to be ready
    mov dx,0x1f7
    notready:
        in al,dx
        test al,STATUS_DRIVEBUSY
        jnz notready
	
    
    ;4 uppermost bits of sector + flag bits
    ;This also selects the drive to use.
    dec dx         ;dx <- 0x1f6
    mov eax,ecx
    shr eax, 24
	or al,0xe0
	out dx,al

    ;400ns delay
    inc dx      ;dx <- 0x1f7
    in al,dx
    in al,dx
    in al,dx
    in al,dx

	;turn off interrupts; we will use programmed i/o
	mov dx,0x3f6
	mov al,CONTROL_INTERRUPT_INHIBIT
	out dx,al

    
	;sector count
	mov dx,0x1f2
	mov al,1
	out dx,al
	
	;set sector number: 28 bits, stored in ecx
    mov eax,ecx
	mov dx,0x1f3
    
	out dx,al       ;low 8 bits of sector number to 0x1f3
    shr eax,8
    inc dx
	
    out dx,al       ;middle 8 bits of sector number to 0x1f4
	shr eax,8
    inc dx
	
    out dx,al       ;high 8 bits of sector number to 0x1f5
    shr eax,8
    inc dx
	
    ;already did 0x1f6
    inc dx
    
	;initiate the read
	mov al,0x20
	out dx,al
	
	;wait for controller ready
    ;bit 7 = busy bit
    ;bit 3 = data ready bit
    ;bit 0 = error bit
	mov dx,0x1f7
    .waitWhileBusy:
        in al,dx
        test al,0x80
        jnz .waitWhileBusy
        
    ;at this point, the busy bit is not set
    ;and the error bits are meaningful
    .part2:
        test al, (1<<3)
        jnz .dataReady
        test al, (1 | (1<<5) )
        jz .part2
        push diskErrorMessage
        call print
        add esp,4
        jmp forever

    .dataReady:
	;read the data as 256 shorts
	mov cx,256
	mov dx,0x1f0
    readloop:
        in ax,dx
        mov [ebx],ax
        add ebx,2
        dec cx
        test cx,cx
        jnz readloop
        
    ;delay 400ns to give drive time to be ready
    mov dx,0x1f7
    in al,dx
    in al,dx
    in al,dx
    in al,dx
    
    popa
    ret

;~ diskError:
    ;~ mov ebx, diskErrorMessage
    ;~ call print
    ;~ jmp forever
    
forever:
    hlt
    jmp forever


;here we have data that needs to be in the
;first sector of the boot block.

;GDT pointer
GDT32pointer:
    dw GDT32end-GDT32start          ;gdt size
    dd bootstart+(GDT32start-bootstart)         ;gdt location       0x7c00

;32 bit GDT 
GDT32start:
    dq 0x0000000000000000           ;reserved; must be zero
    
    ;L=limit, b=base, A=gdr0, C=epp1
    ;  g: 1=limit unit = 4K pages, 0=unit=bytes
    ;  d: 1=32 bits
    ;  r: 1=64 bit code
    ;  e: 1=present
    ;  pp: ring privilege (0 or 3)
    ;  t: 2=data(rw), 0xa=exe(ro)
    ;    bbALCtbbbbbbLLLL
    dq 0x00cf92000000ffff
    ;      ||||\____/\__/----low 16 bits of limit
    ;      |||| base=0 
    ;      |||0010 = data
    ;      ||1001 = present, priv level 0
    ;      |ffff = limit (upper 4 bits)
    ;      1100 = unit=4K, 32 bits
    dq 0x00cf9a000000ffff         
    ;         |
    ;         1010 = code, read only
GDT32end:

;16 bit GDT
GDT16start:
    dq 0x0000000000000000           ;reserved; must be zero
    dq 0x000092000000ffff           ;data, ring 0, 32 bit
    ;      ||||\____/\__/----low 16 bits of limit
    ;      |||| base=0 
    ;      |||0010 = data
    ;      ||1001 = present, priv level 0
    ;      |0 = limit (upper 4 bits)
    ;      0 = unit=bytes, 16 bit
    dq 0x00009a000000ffff           ;code, ring 0, 32 bit
GDT16end:
   
  
;GDT pointer (16 bits)
GDT16pointer:
    dw GDT16end-GDT16start          ;gdt size
    dd (GDT16start)         ;gdt location   ;0x7c00
 

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bzero:
    ; bzero( void* ptr, int count)
    ; stack: count, ptr, eax, ebx <- low mem
    pusha
    mov eax,[esp+36] ;ptr
    mov ebx,[esp+40] ;count
    
    beginBzeroLoop:
        cmp ebx,0
        je endBzeroLoop
        mov byte [eax],0
        inc eax
        dec ebx
        jmp beginBzeroLoop
    endBzeroLoop:
    popa
    ret
    

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; read_block( blockNumber, destinationAddress)
;Note: This will NOT read block zero!
;If blockNumber is zero, this zeros 4K at
;destinationAddress (as for reading a sparse block)
read_block:

    ;read block
    pusha
    
    ;ecx = block number
    ;ebx = destination address
    mov ecx, [esp+36]
    mov ebx, [esp+40]
    
    cmp ecx,0
    jne .notSparse
        ;sparse: Fill memory with zeros
        ;bzero( ebx, 4096)
        push dword 4096
        push ebx
        call bzero
        add esp,8
        popa 
        ret
    .notSparse:
        ;multiply block number by 8 to get sector number
        shl ecx,3
        ;counter
        mov eax,8
        beginloop:
            push ebx
            push ecx
            call read_sector
            add esp,8
            
            ;increment sector number
            inc ecx
            ;move to next location in RAM
            add ebx,512
            ;decrement counter
            dec eax
            jnz beginloop
        popa
        ret
     
 ;get_file_size(inode).
;Returns size in eax.
getFileSize:
    pusha
    mov esi, [esp+36]       ;inode number
    ;load inode contents to TMPBUFF
    push dword TMPBUFF
    push dword esi     ;inode number
    call read_inode 
    add esp,8
    popa
    mov eax,[TMPBUFF+4]
    ret
    
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; end of sector 0
times 510-($-bootstart) db 0x90
db 0x55,0xaa
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; start of sector 1
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

sector_1:


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; void read_inode( inodeNumber, destAddr )
read_inode:
    ;assumes inode is in group 0
    pusha

    ;ecx = inode number (1...n)
    ;ebx = destination address
    mov ecx, [esp+36]
    mov ebx, [esp+40]
    
    ;inodes start at 1, so remap to start at zero
    dec ecx     
    
    ;convert ecx to a byte offset from the start of the inode table
    ;multiply by 128 (sizeof inode)
    shl ecx,7
    
    ;inode table for group 0
    ;starts at block 4 = 4*4096 = 16384
    ;bytes from start of disk
    add ecx,16384
    
    ;now ecx has byte offset from start of disk
    ;where the inode we want is located
    ;the sector holding the inode we want
    ;is at ecx / 512 = ecx >> 9
    ;and the offset within that sector is ecx % 512 = ecx & 511
    
    ;get sector number. Still want to keep
    ;byte offset, so we do our work in eax.
    mov eax,ecx
    shr eax,9
    
    ; void read_sector( sectorNumber, destinationAddress )
    push dword READ_INODE_TEMP_BUFFER
    push eax
    call read_sector
    add esp,8
    
    ;convert ecx to offset within the sector
    ;This is the same as ecx mod 512
    and ecx,511
    
    ;perform equivalent of
    ;memcpy( ebx, READ_INODE_TEMP_BUFFER+ecx, 128 )
    
    lea esi,[READ_INODE_TEMP_BUFFER+ecx]
    mov edi,ebx
    mov ecx,128
    
    ;copy all of inode to destination spot
    ;movsb:  mem[edi++] <- mem[esi++]
    ;rep: do instruction, decrement ecx, repeat if not zero
    rep movsb

    popa
    ret
 
    
kernelName:
db 'kernel.bin'

notFoundKernelMessage:
db "Can't find kernel.bin ",0

foundKernelMessage:
db 'Found kernel ',0

loadedKernelMessage:
db 'Loaded kernel ',0

kernelTooBigMessage:
db 'Kernel too big ',0
    
noAcceptableVideoMessage:
db 'No acceptable video modes ',0

loadingIndirectMessage:
db 'Loading indirect ',0

loadingDoubleIndirectMessage:
db 'Loading dindirect ',0

A20NotOkMessage:
db 'A20 line disabled ',0

loadStartBadMessage:
db 'Multiboot load start invalid',0

loadEndBadMessage:
db 'Multiboot load end invalid',0


noMultibootHeaderMessage:
db 'No multiboot header',0

badMultibootChecksumMessage:
db 'Bad multiboot checksum',0

badMultibootFlagsMessage:
db 'Bad multiboot flags',0

noKernelAddressMessage:
db 'No multiboot load address',0

kernelEndIsBeforeKernelStartMessage:
db 'Kernel end < start',0

kernelOverlapsReservedMemoryMessage:
kernelEndOverlapsReservedAddressesMessage:
db 'Kernel overlaps reserved memory ',0

bssEndNotOKMessage:
db 'Invalid BSS end',0

headerAddressNotOKMessage:
db 'Bad header address ',0

    
   
    
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; end of sector 1
times 1024-($-bootstart) db 0x90
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;sector 2 is used for the superblock
times 1536-($-bootstart) db 0xf4
;sector 3 is used for the superblock
times 2048-($-bootstart) db 0xf4
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;sectors 4,5,6,7 are available for our use

sector_4:

    ;test A20 line: Write to an odd megabyte address
    ;and an even megabyte address and see 
    ;if values are different
    mov eax, 0x107a00
    mov ebx, 0x007a00
    mov dword [eax], 0x123
    mov dword [ebx], 0x456
    mov eax,[eax]
    mov ebx,[ebx]
    cmp eax,ebx
    jne a20ok
    
    push dword A20NotOkMessage
    call print
    add esp,4
    jmp forever
    
    a20ok:

    ;load inode 2 to memory
    push TMPBUFF
    push 2          ;inode number
    call read_inode
    add esp,8

    ;first block of root directory is in inode.direct[0]
    ;which is 40 bytes past TMPBUFF
    ;load first block of root directory to memory
    push dword TMPBUFF
    push dword [TMPBUFF+40]   ;block number
    call read_block
    add esp,8

    ;search for the text 'kernel.bin'
    mov esi,TMPBUFF

findLoop:
    ;esi points to the start of our direntry
    ;esi points to inode number (32 bits)
    ;esi+4 points to reclen (16 bits)
    ;esi+6 points to namelen (16 bits)
    ;esi+8 points to name
    
    ;if name is not 10 bytes long, skip it.
    ;note: namelen field is only 16 bits
    cmp word [esi+6],10
    jne nextLoop

    ;want to compare 10 bytes
    mov ecx,10
    
    ;first pointer for comparison: point to direntry name
    lea ebx,[esi+8]
    
    ;second pointer for comparison: points to constant 'kernel.bin'
    mov edi,kernelName      ;0x7c00
        
    ;number of matching characters
    mov ebp,0

    tallyLoop:
        ;get the two bytes to compare
        mov ah,[ebx]
        mov al,[edi]
        
        ;move to next spot in RAM
        inc ebx
        inc edi
        
        ;see if the two are alike
        cmp ah,al
        jne L2
        ;if they are the same, increment ebp (our count)
        inc ebp
        
        L2:
        ;do another character, if there are any left
        dec ecx
        jnz tallyLoop
    
    ;if we had 10 matches, we've found the one we want
    cmp ebp,10
    je foundKernel

    nextLoop:
        ;move pointer to next record.
        ;[esi+4] = reclen, but it's a short,
        ;so we must mask it to 16 bits
        mov edx,[esi+4]
        and edx,0xffff
        add esi,edx
        ;if we have not found the file after examining
        ;4KB of direntry data, we assume the kernel
        ;isn't present.
        cmp esi,TMPBUFF+4096
        jge notFound
        jmp findLoop
    
notFound:
    ;if we get here, we didn't find the file.
    push dword notFoundKernelMessage
    call print
    add esp,4
    jmp forever
    
    
foundKernel:
    ;if we get here, we've located the direntry for kernel.bin
    ;the number of the inode to load is in memory at esi
    push dword foundKernelMessage      ;0x7c00
    call print
    add esp,4


    mov esi,[esi]
    mov [kernelInode], esi

    ;load_file( inode, addr, maxblocks)
    push dword 2            ;max of 8K
    push dword TEMP_KERNEL_LOAD     
    push dword [kernelInode]        ;inode number
    call load_file
    add esp,12
    
    ;search for multiboot header
    mov eax,TEMP_KERNEL_LOAD
    .mbloopstart:
        cmp dword [eax],0x1badb002
        je .foundmagic
        add eax,4
        cmp eax,TEMP_KERNEL_LOAD+8192
        jl .mbloopstart

    
    push dword noMultibootHeaderMessage
    call print
    add esp,4
    jmp forever

    .foundmagic:
        ;if checksum isn't what we expect,
        ;resume search in first 8K for a second
        ;instance of header
        ;[eax] = magic
        ;[eax+4] = flags
        ;[eax+8] = checksum
        mov ebx,[eax]
        add ebx,[eax+4]
        add ebx,[eax+8]
        cmp ebx,0
        je .checksumOK
        
        push dword badMultibootChecksumMessage
        call print
        add esp,4
        add eax,4
        jmp .mbloopstart

    .checksumOK:
    test dword [eax+4], 0xfff8
    jz .flagsOK
    
    push dword badMultibootFlagsMessage
    call print
    add esp,4
    jmp forever
    
    .flagsOK:
    
    push dword [eax+4]    ;flags
    pop dword [mbFlags]
    
    ;1 = load on 4K boundary. We don't load modules, so ignore this
    ;2 = include memory info. We always include this, so ignore this bit
    ;4 = set up video
    ;65536 = kernel provides memory location to load it
    test dword [mbFlags], 0x10000
    jnz .memOK
    
    push dword noKernelAddressMessage
    call print
    add esp,4
    jmp forever

    .memOK:
    
    mov ecx,[eax+12]
    mov [headerAddress], ecx
    mov ecx,[eax+16]
    mov [loadAddress], ecx
    mov ecx,[eax+24]
    mov [bssEnd], ecx
    mov ecx,[eax+28]
    mov [entryPoint],ecx
    mov [multibootOffset], eax      ;offset from start of file to magic number
    sub dword [multibootOffset],TEMP_KERNEL_LOAD
    
    ;if flags didn't have bit 2 set, these will be garbage values
    mov ecx, [eax+32]
    mov [mbVideoMode], ecx
    mov ecx, [eax+36]
    mov [mbWidth], ecx
    mov ecx, [eax+40]
    mov [mbHeight], ecx
    mov ecx, [eax+44]
    mov [mbDepth], ecx
    
    ;we only support loading the entire file, so we require header address to
    ;equal load_address + multibootOffset
    mov edi, [headerAddress]
    mov eax, [loadAddress]
    add eax, [multibootOffset]
    cmp edi,eax
    je .headerAddressOK
    
    push dword headerAddressNotOKMessage
    call print
    add esp,4
    jmp forever
    
    .headerAddressOK:
    ;check load address and make sure it's OK
    cmp dword [loadAddress], 0x10000
    jae .loadAddressAtLeast64K
    
    push dword loadAddressMustBeAtLeast65536Message
    call print
    add esp,4
    jmp forever
    
    .loadAddressAtLeast64K:
    
    ;getFileSize(inode)
    push dword [kernelInode]
    call getFileSize
    mov [kernelFileSize],eax


    mov eax,[loadAddress]
    add eax,[kernelFileSize]
    mov [kernelEnd], eax
    
    mov eax,[loadAddress]
    cmp [kernelEnd], eax
    jae .kernelEndIsAboveKernelStart
    
    push dword kernelEndIsBeforeKernelStartMessage
    call print
    add esp,4
    jmp forever
        
    .kernelEndIsAboveKernelStart:

    ;if kernel start < 0x80000: ok
    ;if kernel start > 0xfffff: ok
    cmp dword [loadAddress], 0x80000
    jb .kernelStartOK
    cmp dword [loadAddress], 0xfffff
    ja .kernelStartOK
    
    ;kernel start between 0x80000...0xfffff
    push dword kernelOverlapsReservedMemoryMessage
    call print
    add esp,4
    jmp forever

    .kernelStartOK:

    ;if kernel start not below max ram: error
    mov eax,[totalMemory]
    cmp [loadAddress],eax
    jb .kernelStartOK2
    
    ;kernel starts past end of RAM
    push dword kernelStartTooHighMessage
    call print
    add esp,4
    jmp forever
    
    .kernelStartOK2:

    ;if kernel end < 0x80000: ok
    ;if kernel end > 0xfffff: ok
    cmp dword [kernelEnd], 0x80000
    jbe .kernelEndOK
    cmp dword [kernelEnd], 0xfffff
    ja .kernelEndOK
    
    ;kernel end between 0x80000...0xfffff
    push dword kernelOverlapsReservedMemoryMessage
    call print
    add esp,4
    jmp forever
    
    .kernelEndOK:
    ;if kernel start not below max ram: error
    mov eax,[totalMemory]
    cmp [kernelEnd],eax
    jb .kernelStartOK3
    
    ;kernel ends after end of RAM
    push dword kernelEndTooHighMessage
    call print
    add esp,4
    jmp forever
    
    .kernelStartOK3:
    ;we know start not in 0x80000...0xfffff
    ;and end not in that range
    ;we know end >= start
    ;we must check for start < 0x80000 and end > 0xfffff
    cmp dword [loadAddress],0x80000
    ja .overlap2OK
    ;if we get here, start is below 0x80000
    cmp dword [kernelEnd],0xfffff
    jb .overlap2OK
    
    ;if we get here, start < 0x80000 and end > 0xfffff
    push dword kernelOverlapsReservedMemoryMessage
    mov esi,[loadAddress]
    mov edi,[kernelEnd]
    mov ebp,0xbeef
    call print
    add esp,4
    jmp forever
    
    
    .overlap2OK:
    cmp dword [kernelEnd], 0x80000
    jb .endNotInAdapterRAM
    cmp dword [kernelEnd], 0xfffff
    ja .endNotInAdapterRAM
    
    push kernelEndOverlapsReservedAddressesMessage
    call print
    add esp,4
    jmp forever
    
    .endNotInAdapterRAM:
    
    mov edi,[loadAddress]        ;load start
    cmp [totalMemory], edi
    ja .kernelStartBeforeEndOfRAM
    
    push kernelStartPastEndOfRAMMessage
    call print
    add esp,4
    jmp forever
    
    .kernelStartBeforeEndOfRAM:
    
    mov edi, [kernelEnd]
    cmp dword [totalMemory], edi
    ja .kernelEndBeforeEndOfRAM
    
    push kernelEndPastEndOfRAMMessage
    call print
    add esp,4
    jmp forever
    
    .kernelEndBeforeEndOfRAM:
    
    cmp dword [bssEnd],0
    je .bssEndOK
    
    mov eax,[kernelEnd]
    cmp [bssEnd], eax
    jae .bssEndOK
    
    push bssEndNotOKMessage
    call print
    add esp,4
    jmp forever
    
    
    .bssEndOK:


    mov eax,[entryPoint]
    cmp eax,[loadAddress]
    jae .loadStartOK
    
    push loadStartBadMessage
    call print
    add esp,4
    jmp forever
    
    .loadStartOK:
    cmp eax,[kernelEnd]
    jb .loadEndOK
    
    push loadEndBadMessage
    call print
    add esp,4
    jmp forever
    
    .loadEndOK:
    
    ;OK to load the kernel
    
    ;load_file( inode, addr, maxblocks)
    ;FIXME: Should we support >= 2GB?
    push dword 0x7fffffff
    push dword [loadAddress]   ;load start
    push dword [kernelInode]

    call load_file
    add esp,12
    
    push dword loadedKernelMessage     ;0x7c00
    call print
    add esp,4
    
    cmp dword [bssEnd],0
    je .doneClearBss
    
    ;clear BSS
    mov eax,[loadAddress]
    add eax,[kernelFileSize]
    
    .clearBss:
        cmp eax,[bssEnd]
        jae .doneClearBss
        mov byte [eax],0
        inc eax
        jmp .clearBss
    .doneClearBss:
    
    ;if flag bit is not set, user did not request video to be set.
    test dword [mbFlags], 0x4
    jz .doneSettingVideo

    ;if flag bit is set but user gave 1 in video mode, they are
    ;asking for text
    cmp dword [mbVideoMode], 1
    je .doneSettingVideo

    ;if video mode is not zero, we don't know what user wants
    cmp dword [mbVideoMode], 0
    je .setVideoNow
    
    push dword badMultibootVideoModeMessage
    call print
    add esp,4
    jmp forever
    
    .setVideoNow:
    
    ;set the video mode
    lgdt [GDT16pointer]     ;0x7c00
    jmp 0x10:.pmode16    ;0x7c00
    .pmode16:
    bits 16
    ;switch back to real mode
    ;disable protected mode
    mov eax,cr0
    and eax,~1
    mov cr0,eax
    jmp .flush16
    .flush16:
    nop
    jmp 0x0:.real16      ;0x7c00
    .real16:
    xor ax,ax
    mov ss,ax
    mov fs,ax
    mov ax,VESA_INFO_SEGMENT
    mov es,ax
    mov ds,ax
    ;we haven't changed the interrupt table,
    ;so we can still use the bios int routines

    ;get available video modes
    ;This writes 512 bytes at es:di
    ;The list of video mode numbers (=16 bit shorts)
    ;is at offset 14. List terminates with 0xffff
    mov ax,0x4f00
    xor di,di
    int 0x10

    ;get video mode data
    ;This writes 512 bytes to es:di. Useful items:
    ;offset 0: short: flags: 1=supported, 8=color, 
    ;                   16=graphics, 64=banked (bad), 
    ;                   128=linear (preferred)
    ;offset 16: short: pitch
    ;offset 18: short: width
    ;offset 20: short: height
    ;offset 24: byte: planes: 1=best
    ;offset 25: byte: bpp
    ;offset 27: byte: memory mode (4=packed pixel; 6=direct)
    ;offset 40: int: pointer to framebuffer
    
    ;get data about each video mode.
    ;we want one that has requested width, height, bpp,
    ;flags & (1+8+16+64+128) == (1+8+16+128),
    ;mode = 6
    
    ;select segment 0x9000
    ;so we can keep the video list in segment 0x8000
    mov ax,VIDEO_MODE_SEGMENT
    mov es,ax
    
    ;counter/pointer for mode list
    ;starts at ds:14
    mov si,[ds:14]
    
    .vidloop:
        ;di <- 0
        xor di,di
        ;mode we are querying
        mov cx,[ds:si]
        cmp cx,0xffff
        je .noAcceptableVideo
        ;function number: get mode info
        mov ax,0x4f01
        pusha
        int 0x10
        popa
        
        ;flags
        mov ax, [es:0]
        and ax, 1|8|16|64|128
        cmp ax, 1|8|16|128
        jne .nextmode
        ;width
        mov dx,[fs:mbWidth]
        cmp word [es:18],dx
        jne .nextmode
        ;height
        mov dx,[fs:mbHeight]
        cmp word [es:20],dx
        jne .nextmode
        ;planes
        cmp byte [es:24],1
        jne .nextmode
        ;direct
        cmp byte [es:27],6 
        jne .nextmode
        ;bpp
        mov dl,[fs:mbDepth]
        cmp byte [es:25],dl
        jne .nextmode
        ;if we get here, the mode is acceptable
        ;the mode number is in cx
        jmp .acceptableVideoMode
        
        .nextmode:
        add si,2
        jmp .vidloop
        
        
    .noAcceptableVideo:
        xor ax,ax
        mov ds,ax
        call go32
        bits 32
        push dword noAcceptableVideoMessage    ;0x7c00
        call print
        add esp,4
        jmp forever
        bits 16
    
    .acceptableVideoMode:
     ;copy info to mbi structure
    push ds
    xor eax,eax
    mov ds,ax
    mov ax,[es:16]
    mov [mbiFramebufferPitch],eax
    mov ax,[es:18]
    mov [mbiFramebufferWidth],eax
    mov ax,[es:20]
    mov [mbiFramebufferHeight],eax
    mov al,[es:25]
    mov [mbiFramebufferBpp],al
    mov eax, [es:40]
    mov [mbiFramebufferAddress],eax
    mov al,[es:31]
    mov [mbiFramebufferRedMask],al
    mov al,[es:32]
    mov [mbiFramebufferRedPos],al
    mov al,[es:33]
    mov [mbiFramebufferGreenMask],al
    mov al,[es:34]
    mov [mbiFramebufferGreenPos],al
    mov al,[es:35]
    mov [mbiFramebufferBlueMask],al
    mov al,[es:36]
    mov [mbiFramebufferBluePos],al
    mov byte [mbiFramebufferType], 1   ;1=rgb direct
    pop ds
    
    ;set video mode. According to
    ;http://wiki.osdev.org/VBE and
    ;http://www.brokenthorn.com/Resources/OSDevVid2.html
    ;linear framebuffer modes should be or'd with 0x4000
    mov bx,cx
    or bx,0x4000
    ;function number: set mode
    mov ax,0x4f02
    int 0x10
    
    ;the video mode should now be set

    xor eax,eax
    mov ds,ax

    ;transfer back to protected mode
    call go32
    
    bits 32
    
    .doneSettingVideo:
    
    mov dword [mbiFlags], (1 | (1<<12))
    mov dword [mbiMemLower], 640*1024
    mov ebx,[totalMemory]
    sub ebx, 1024*1024                  ;account for low 1MB not included
    mov dword [mbiMemUpper], ebx              

    ;if we are in text mode, don't store video info in multiboot info struct
    ;we deal with this by simply omitting the flag
    test dword [mbFlags],0x4          ;4=user requesting video mode change
    jnz .userDidHaveVideoModeRequest
    and dword [mbFlags], ~(1<<12)
    jmp .goKernel
    
    cmp dword [mbVideoMode],1     ;1 = user requested text mode
    jne .userDidHaveGraphicalMode
    and dword [mbFlags], ~(1<<12)
    
    .userDidHaveVideoModeRequest:
    .userDidHaveGraphicalMode:
    .goKernel:
    
    mov eax,0x2badb002              ;special flag
    mov ebx,multibootInfoStruct     ;multiboot info structure
    mov ecx,[entryPoint]
    xor edx,edx
    xor esi,esi
    xor edi,edi
    xor ebp,ebp

    jmp ecx
  
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  

;load_file( inode, addr, maxblocks)
load_file:

    pusha
    
    mov esi, [esp+36]       ;inode number

    ;load inode contents to TMPBUFF
    push dword TMPBUFF
    push dword esi     ;inode number
    call read_inode 
    add esp,8

    ;direct is located 40 bytes past the start of the inode: [TMPBUFF+40]
    ;direct holds 12 entries, 4 bytes each = 48 bytes total
    ;indirect is located 40+48 = 88 bytes past start of inode: [TMPBUFF+88]
    ;size of the file is at [TMPBUFF + 4]

    ;copy direct[0:12] to a temporary location,
    ;plus indirect and doubleindirect.
    ;that will be 14 consecutive dwords
    
    mov ecx,14
    mov ebx,TMPBUFF+40
    mov edi,direct
    copyloop:
        mov eax,[ebx]
        mov [edi],eax
        add edi,4
        add ebx,4
        dec ecx
        jnz copyloop

    ;esi holds the size in bytes
    mov esi,[TMPBUFF+4]

    ;let esi hold the size in blocks: divide by 4096
    shr esi,12

    ;if size is not evenly divisible by 4096,
    ;load one more sector
    test dword [TMPBUFF+4],0xfff
    jz noExtra
        inc esi
    noExtra:
    
    ;potentially clamp esi if passed in maxblocks was smaller
    cmp esi,[esp+44]
    jb .noclamp             ;unsigned jump if below
        mov esi,[esp+44]
    .noclamp:
    
    mov ebx, [esp+40]       ;address to load
    mov eax,direct
    mov edx,12              ;max count

    .loadloop:
        push ebx
        push dword [eax]
        call read_block
        add esp,8
        add ebx,4096
        add eax,4
        ;decrement block count remaining
        dec esi
        jz doneload
        dec edx
        jz .endloop
        jmp .loadloop
    .endloop:


    ;if we get here,
    ;there's more data left in file, and we've read
    ;all 12 direct blocks
    
    ;ebx still holds the destination address in memory
    ;esi still holds the remaining block count
    
    ;push dword loadingIndirectMessage
    ;call print
    ;add esp,4
    
    ; load_indirect( blockNumber, memPtr, maxBlocksToLoad )
    push esi
    push ebx
    push dword [indirect]
    call load_indirect
    add esp,12

    ;account for the new destination address
    add ebx, 1024*4096
    
    ;drop 1024 off size since we've now loaded 1024 blocks
    sub esi,1024
    
    cmp esi,0
    jle doneload
    
    ;if we get here, we still have more data to load,
    ;so we must use the double indirect blocks.

    ;push dword loadingDoubleIndirectMessage
    ;call print
    ;add esp,4
    
    ; load_dindirect( blockNumber, memPtr, maxBlocksToLoad)
    push esi
    push ebx
    push dword [doubleindirect]
    call load_dindirect
    add esp,12

    sub esi, 1024*1024
    cmp esi,0
    jle doneload
    
    ;not handling triple indirect
    push dword kernelTooBigMessage
    call print
    add esp,4
    jmp forever
    
doneload: 
    popa
    ret
 
    
    
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; load_indirect( blockNumber, memPtr, maxBlocksToLoad )
; if blockNumber == 0: Sparse
; memPtr = destination for data
;Stop when:
;   -We've loaded 1024 blocks or
;   -We've loaded max blocks
load_indirect:
    pusha     

    mov ecx,[esp+36]    ;block number for the indirect block itself
    mov ebx,[esp+40]    ;dest addr
    mov esi,[esp+44]    ;max blocks to load
    
    ; read_block( blockNumber, destinationAddress )
    ; read_block handles the case where block is zero (sparse)
    push dword INDIRECT_BLOCK
    push ecx
    call read_block
    add esp,8
    
    mov eax, INDIRECT_BLOCK
    mov edx, 1024               ;max we can load
    .loadLoop:
        ;read_block will handle the case
        ;of a block number == 0 (sparse)
        push ebx
        push dword [eax]
        call read_block
        add esp,8
        
        add eax,4
        add ebx,4096
        
        dec edx
        jz .endLoop
        
        dec esi
        jz .endLoop

        jmp .loadLoop
        
    .endLoop:
    popa
    ret
    
    
; load_dindirect( blockNumber, memPtr, maxBlocksToLoad )
load_dindirect:
    pusha

    mov edx,[esp+36]    ;block number
    mov ebx,[esp+40]    ;memPtr
    mov esi,[esp+44]    ;max blocks
    
    push dword DINDIRECT_BLOCK
    push edx        ;block number
    call read_block
    add esp,8

    mov edx,1024        ;1024 indirects maximum
    mov eax, DINDIRECT_BLOCK
    
    .loopStart:
        ;load_indirect( blockNum, memPtr, maxBlocks )
        push dword esi
        push dword ebx
        push dword [eax]
        call load_indirect
        add esp,12

        add ebx,1024*4096
        add eax,4 
        
        dec edx
        jz .loopEnd

        sub esi,1024
        cmp esi,0
        jg .loopStart
        
    .loopEnd:
    popa 
    ret
   
   

loadAddressMustBeAtLeast65536Message:
db 'Kernel start must be >= 65536',0

kernelStartPastEndOfRAMMessage:
kernelStartTooHighMessage:
db 'Load start past end of RAM',0

kernelEndPastEndOfRAMMessage:
kernelEndTooHighMessage:
db 'Kernel end past end of RAM',0

badMultibootVideoModeMessage:
db 'Bad multiboot video flags',0

;%rep 102
; db 0
;%endrep    
    
    
times 4096-($-bootstart) db 0x90

ABSOLUTE 0x8c00    
loadAddress: resd 1  
multibootOffset: resd 1
mbVideoMode: resd 1   
mbWidth: resd 1
mbHeight: resd 1
mbDepth: resd 1
headerAddress: resd 1
mbFlags: resd 1

totalMemory:
    resd 1

direct:
    resd 12
indirect:
    resd 1
doubleindirect:
    resd 1
    
entryPoint:
    resd 1
    
bssEnd:
    resd 1
    
kernelEnd:
    resd 1

kernelInode:
    resd 1

kernelFileSize:
    resd 1
    
go32temp:
    resd 1

multibootInfoStruct:
    mbiFlags: resd 1          ;dd 1 | (1<<12)
    mbiMemLower:    resd 1  ;dd 640*1024
    mbiMemUpper:    resd 1  ;dd 0
    mbiFramebufferAddress:  resq 1 ;dq 0
    mbiFramebufferPitch:  resd 1  ;dd 0
    mbiFramebufferWidth:  resd 1  ;dd 0
    mbiFramebufferHeight:  resd 1  ;dd 0
    mbiFramebufferBpp:  resb 1 ;db 0
    mbiFramebufferType: resb 1 ;db 0
    mbiFramebufferRedPos: resb 1 ;db 0
    mbiFramebufferRedMask: resb 1 ;db 0
    mbiFramebufferGreenPos: resb 1 ;db 0
    mbiFramebufferGreenMask: resb 1 ;db 0
    mbiFramebufferBluePos: resb 1  ;db 0
    mbiFramebufferBlueMask: resb 1 ;db 0
