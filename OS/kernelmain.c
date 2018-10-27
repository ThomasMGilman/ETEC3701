#include "console.h"
#include "file.h"
#include "interrupt.h"

void sweet();

void kmain(struct MultibootInfo *mbi){
    consol_init(mbi);   //initialize framebuffer and blank screen
    disk_init();
    interrupt_init();
    //if(listDiskInfo() != 0)
    //   logString("error listing DiskInfo");
    sweet();
    // int a=5;
    // int b=0;
    // int c = a/b;                                 //Division by zero
    // kprintf("%d\n",c);
    // asm volatile ("int 3" :::"memory");           //Interrupt3 debug trap
    // asm volatile( ".byte 15,255" ::: "memory");   //bad opcode
    while(1){           //loop forever
    }  
}