#include "console.h"
#include "util.h"
#include "disk.h"

void kmain(struct MultibootInfo *mbi){
    consol_init(mbi);   //initialize framebuffer and blank screen
    listDiskInfo();
    while(1){           //loop forever
    }  
}