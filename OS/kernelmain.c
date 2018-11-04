#include "util.h"

const char* userFile = "usertest1.bin";

void consol_init(struct MultibootInfo *m);
int disk_init(void);
void interrupt_init(void);
int exec(const char* filename);

//int listDiskInfo(void);
//void sweet(void);

void kmain(struct MultibootInfo *mbi){
    consol_init(mbi);   //initialize framebuffer and blank screen
    disk_init();
    interrupt_init();
    // if(listDiskInfo() != 0)
    //     logString("error listing DiskInfo");
    //sweet(1);
    //logString("fin\n");
    // int a=5;
    // int b=0;
    // int c = a/b;                                  //Division by zero
    // kprintf("%d\n",c);
    // asm volatile ("int 3" :::"memory");           //Interrupt3 debug trap
    // asm volatile( ".byte 15,255" ::: "memory");   //bad opcode
    exec(userFile);
    logString("ERROR\n");
    while(1){;}           //loop forever
}