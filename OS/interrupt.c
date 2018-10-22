#include "interrupt.h"
#include "kprintf.h"

struct GDTEntry gdt[] = {
    { 0,0,0,0,0,0 },            //zeros
    { 0xffff, 0,0,0, 0xcf92, 0},//data
    { 0xffff, 0,0,0, 0xcf9a, 0} //code
};

struct IDTEntry idt[32];

void haltForever(void)
{
    while(1){
        asm volatile("hlt" ::: "memory");
    }
}

__attribute__((interrupt))
void unknownInterrupt(struct InterruptFrame* fr)
{
    kprintf("\nFatal exception at eip=%x\n",fr->eip);
    haltForever();
}

__attribute__((interrupt))
void unknownInterruptWithCode(struct InterruptFrame* fr, unsigned code)
{
    kprintf("Fatal exception: Code=%x eip=%x\n", code, fr->eip);
    haltForever();
}

void table(int i, void* func){
    unsigned x = (unsigned)func;
    idt[i].addrLow = x&0xffff;
    idt[i].selector = 2 << 3;
    idt[i].zero = 0;
    idt[i].flags = 0x8e;
    idt[i].addrHigh = x>>16;
}

void interrupt_init()
{
    struct LGDT lgdt;
    lgdt.size = sizeof(gdt);
    lgdt.addr = &gdt[0];
    asm volatile( "lgdt [eax]" : : "a"(&lgdt) : "memory" );
    unsigned index;
    for(index = 0; index < 32; index++)
    {
        if(index == 0 || index == 1 || index == 3 ||
            index == 6 || index == 13 || 
            index == 14)
            table(index, unknownInterruptWithCode);
        else
            table(index, unknownInterrupt);
    }
    struct LIDT tmp;
    tmp.size = sizeof(idt);
    tmp.addr = &idt[0];
    asm volatile("lidt [eax]" : : "a"(&tmp) : "memory" );
}