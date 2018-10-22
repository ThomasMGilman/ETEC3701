#include "console.h"
#include "util.h"
#include "disk.h"
#include "file.h"
#include "interrupt.h"
#include "kprintf.h"

void kmain(struct MultibootInfo *mbi){
    consol_init(mbi);   //initialize framebuffer and blank screen
    disk_init();
    interrupt_init();
    if(listDiskInfo() != 0)
       logString("error listing DiskInfo");
    // int a=5;
    // int b=0;
    // int c = a/b;
    // kprintf("%d\n",c);
    //asm volatile ("int 3" :::"memory");
    while(1){           //loop forever
    }  
}