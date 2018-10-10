#include "console.h"
#include "util.h"

void kmain(struct MultibootInfo *mbi){
    consol_init(mbi);   //initialize framebuffer and blank screen
    sweet();            //write out chars
    while(1){           //loop forever
    }  
}