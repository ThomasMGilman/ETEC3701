#include "console.h"
#include "util.h"
#include "disk.h"
#include "file.h"

void kmain(struct MultibootInfo *mbi){
    consol_init(mbi);   //initialize framebuffer and blank screen
    if(listDiskInfo() != 0)
        logString("error listing DiskInfo");
    while(1){           //loop forever
    }  
}