#include "file.h"
#include "disk.h"
#include "errno.h"

int file_open(const char* fileName, int flags)
{
    struct File* file;
    int fd, index, noSpace;
    while(index++ < MAX_FILES)
    {
        if(file_table[index].in_use == 0)   //found free space
        {
            fd = index;
            break;
        }
        if(index == MAX_FILES - 1 && file_table[index].in_use == 1)
            return -EMFILE;
    }
    file_table[index] = *file;
    file->flags = flags;
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

int file_close(int fd)
{
    if(fd > MAX_FILES) //no such file
        return -EINVAL;
    if(file_table[fd].in_use == 0) //file not open
        return -EINVAL;
    file_table[fd].in_use = 0;  //set file for not in use so close
    return SUCCESS;
}

unsigned get_file_inode(unsigned dir_inode, const char* filename)
{
    signed pass;
    struct Inode *ino;
    if((pass = disk_read_inode(dir_inode, &ino)) < 0)
        return pass;


    //get inode location ex):
    // GLOBALS:  inodesPerGroup = 100     blockPerGroup = 1000
    // inode I want 3141
    // group = (3141-1)/100 = 31
    // inodeTableStartingBlock = 1000 * 31 + 4 = 31004
    // inodesFromTableStart = (3141-1) % 100 = 40
    // bytesFromTableStart = 40*sizeof(Inode) = 40*128 = 5120bytes
    // blocksFromTableStart = 5120/4096 = 1

    // load_block(alpha + beta, buffer)
    // inodesPerBlock = 4096/128 = 32
    // inodesToSkip = 40 % 8
    // kmemcpy(ino, buffer+inodesToSkipOver*sizeof(inode),sizeof(inode))
}