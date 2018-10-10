
#include "kprintf.h"
#include "disk.h"

const char* decl = 
"IN CONGRESS, July 4, 1776.\n\n"
"The unanimous Declaration of the thirteen united States of America,\n\n"
"When in the Course of human events, it becomes necessary for one people to "
"dissolve the political bands which have connected them with another, "
"and to assume among the powers of the earth, the separate and equal station to "
"to which the Laws of Nature and of Nature's God entitle them, a decent "
"respect to the opinions of mankind requires that they should declare the "
"causes which impel them to the separation. \n\n"
"We hold these truths to be self-evident, that all men are created equal, "
"that they are endowed by their Creator with certain unalienable Rights, that "
"among these are Life, Liberty and the pursuit of Happiness. \n"
"\t--That to secure these rights, Governments are instituted among Men,\n"
" \t--That whenever any Form of Government becomes destructive of these "
"ends, it is the Right of the\t\tPeople\t\tto alter or to abolish it, and to "
"institute new Government, laying its foundation on such principles and "
"organizing its powers in such form, as to them shall seem most likely to "
"effect their Safety and Happiness.\n";

char decl2[512];
void sweet(){
    int i;

    for(i=0;i<512;++i)
        decl2[i] = 'A';
        
    disk_write_sector( 4, decl+512, 1);
    disk_write_sector( 2,decl, 1);
    disk_write_sector( 3,decl2, 1);
    
    disk_read_sector( 2, decl2, 1);
    for(i=0;i<512;++i){
        if( decl[i] != decl2[i] ){
            kprintf("Bad\n");
            kprintf("index:%d, decl[i]:%d, decl2[i]:%d\n",i,decl[i], decl2[i]);
            return;
        }
    }
    disk_read_sector( 3, decl2, 1);
    for(i=0;i<512;++i){
        if( decl2[i] != 'A' ){
            kprintf("Badder\n");
            return;
        }
    }
    disk_read_sector( 4, decl2, 1);
    for(i=0;i<512;++i){
        if( decl[512+i] != decl2[i] ){
            kprintf("Baddest\n");
            return;
        }
    }
            
    kprintf("Muy bien\n");
}
