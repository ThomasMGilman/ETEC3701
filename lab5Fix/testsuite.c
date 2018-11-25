#include "console.h"

const char* bigstring = 
    "When in the course of human. No.\f"
    "We the people of. Oh crud.\f"
    "|       |       |       |       |       |       |       |       |       |       |       |       |\n"
    "Fourscore and\t^seven years ago, our\t^forefathers brought\t^forth upon\t^this continent\n"
    "a new \t^NATION conceived in libibertarian\x7f\x7f\x7f\x7f\x7fy and\t\t\t^dedicated to\t\t^the"
    "\t\t^NOTION\t^that\n"
    "\n"
    "all men\x7f\x7f\x7fpeople are created equal.\n"
    "\n"
    "\n"
    "Now\n"
    "\twe\n\t\tare\n\t\t\tengaged.................,..............in..............,.............a............,..........large,"
    "yooge, egregious, massive, overbearing grate\x7f\x7f\x7f""eat civil strife,\n"
    "~~~~~~~\t:-)\n"
    "\t\t\t\t\ttesting whether that "
    "nation or fooby"
    "any\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f""any "
    "nation can so endure."
    ;
    

void sweet(){
    int i;
    const char* p = bigstring;
    while(*p){
        console_putc(*p++);
    }
    console_putc('\n');
    for(i=0;i<201;++i)
        console_putc('$');
    for(i=0;i<200;++i)
        console_putc('\x7f');
    console_putc('\n');
    console_putc('A');
    console_putc('B');
    console_putc('\b');
    console_putc('C');
    console_putc('D');
    console_putc('\b');
    console_putc('D');
    console_putc('D');
    console_putc('\n');
    for(i=0;i<20;i++)
        console_putc('*');
    console_putc('\r');
    console_putc('!');
    console_putc('#');
    console_putc('\x7f');
}



