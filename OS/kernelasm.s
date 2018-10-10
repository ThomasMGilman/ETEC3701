section .text
fileStart:
    extern _bssStart
    extern _bssEnd
    extern _clearBss
    push ebx
    push _bssEnd
    push _bssStart
    call _clearBss
    add esp,8
    pop ebx
    extern _kmain
    mov esp, stack
    push ebx
    call _kmain
    forever:
        jmp forever
    %define KERNEL_ADDRESS 0x100000
    ;2 = want memory info; 4 = want video mode set;
    ;65536 = we give load address
    %define MBFLAGS (2|4|65536)
    align 8
multibootHeaderStart:
    dd 0x1badb002           ;magic number
    dd MBFLAGS
    ;checksum: checksum + magic + flags = 0
    dd -(0x1badb002 + MBFLAGS)
    ;header addr
    dd KERNEL_ADDRESS + (multibootHeaderStart-fileStart)
    dd KERNEL_ADDRESS       ;load addr
    dd 0                    ;load end; 0=entire file
    dd 0                    ;bss end. 0=none
    dd KERNEL_ADDRESS       ;entry point
    dd 0                    ;video mode: 0=linear, 1=ega
    dd 800                  ;width
    dd 600                  ;height
    dd 16                   ;depth
section .bss
resd 1024
stack: