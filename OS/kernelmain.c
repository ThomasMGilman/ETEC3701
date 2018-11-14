#include "util.h"

const char* userFile = "keytest.bin"; //"user.bin"
const char* userFile2 = "squawker.bin";

int consol_init(struct MultibootInfo *m);
int file_init(void);
int disk_init(void);
int interrupt_init(void);
int kprintf(const char* fmt, ... ) __attribute__((format (printf , 1, 2 ) ));
int exec(const char* filename);

//int listDiskInfo(void);
//void sweet();

void kmain(struct MultibootInfo *mbi){
    consol_init(mbi);   //initialize framebuffer and blank screen
    disk_init();
    interrupt_init();
    file_init();
    // if(listDiskInfo() != 0)
    //     logString("error listing DiskInfo");
    //sweet(0);
    logString("fin setup\n");
    exec(userFile2);
    logString("ERROR\n"); //should never get here
    while(1){;}           //loop forever
}