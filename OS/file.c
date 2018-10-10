#include "file.h"

int file_open(const char* fileName, int flags)
{
    //if no free space in file table
        //return -EMFILE;
    
    int fd, index;
    for(index = 0; index < MAX_FILES; index++)
    {
        if(file_table[index].in_use == 0)
        {
            fd = index;
            break;
        }
        //else no unused table
    }
    //flag check
    //filename: check it
    unsigned fn_len = kstrlen(fileName);
    //for each entry e of root dir:
        //if(e.name_len == fnl && kmemcmp(e.name, filename, fnl) == 0)
            //file_table[fd].in_use = 1;
            //disk_read_inode(e.inode, &file_table[fd].ino);
            //return fd;
        //else error if filename isnt there (in root directory)
            //return -ENOENT;
}