section .text
extern __start
jmp __start
extern _main
global ___main
___main:
    jmp _main