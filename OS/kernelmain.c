#include "util.h"

const char* userFile = "usertest1.bin";

void consol_init(struct MultibootInfo *m);
int disk_init(void);
void interrupt_init(void);
int kprintf(const char* fmt, ... ) __attribute__((format (printf , 1, 2 ) ));
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
    exec(userFile);
    logString("ERROR\n"); //should never get here
    while(1){;}           //loop forever
}