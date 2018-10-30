#include "file.h"

struct File file_table[MAX_FILES];

char debugMsg[200];

int file_open(const char* fileName, int flags)
{
    struct Inode* fileIno;
    int fd = 0, pass;
    while(fd++ < MAX_FILES)
    {
        if(file_table[fd].in_use == 0)   //found free space
            break;
        if(fd == MAX_FILES - 1 && file_table[fd].in_use == 1)
            return -EMFILE;
    }
    if((pass = checkDirs(1, 0, 0, kstrln((char*)fileName), fileName)) < 0)  //filename: check it
        return -ENOENT;
    if((pass = disk_read_inode(pass, &fileIno)) < 0)
        return -DRIERR;
    file_table[fd].flags = flags;
    file_table[fd].ino = *fileIno;
    file_table[fd].in_use = 1;
    file_table[fd].offset = 0;
    return fd;
}

int file_close(int fd)
{
    if(fd >= MAX_FILES)             //no such file
        return -EINVAL;
    if(file_table[fd].in_use == 0)  //file not open
        return -EINVAL;
    file_table[fd].in_use = 0;      //set file for not in use so close
    return SUCCESS;
}

int file_read(int fd, void* buf, int count)
{
    struct File* fp = &file_table[fd];
    static char buffer[4096];
    unsigned bi = fp->offset / BLOCK_SIZE;  //Inode Block index
    unsigned bo = fp->offset % BLOCK_SIZE;  //Buffer offset
    unsigned remaining = fp->ino.size - bo, byteCount = 0, numToAdd;
    signed pass;
    
    
    if(fp->in_use <= 0 || fp->offset >= fp->ino.size) //file is not in use or at end of size
        return 0;
    if(fp->ino.size < 48000) //small file
    {
        numToAdd = (remaining < count) ? remaining : count;
        while((byteCount < numToAdd) && (bi < 12) && (fp->offset < fp->ino.size))
        {
            if((pass = disk_read_block(fp->ino.direct[bi], buffer)) < 0)
                return pass;
            kmemcpy((char*)buf+byteCount, buffer + bo, 1);
            //update variables
            byteCount += 1;
            fp->offset += 1;
            bo = fp->offset % BLOCK_SIZE;
            bi = fp->offset / BLOCK_SIZE;
            remaining = fp->ino.size - bo;
        }
        return byteCount;
    }
    else 
        return 0;
        
}

int file_write(int fd, const void* buf, int count)
{
    return -ENOSYS; //no such system call
}

int file_seek(int fd, int offset, int whence)
{
    if(file_table[fd].in_use <= 0)
        return 0;
    if(whence == SEEK_SET)
    {
        if(offset < 0) 
            return -EINVAL;
        file_table[fd].offset = offset;
    }
    else if(whence == SEEK_CUR)
    {
        if(file_table[fd].offset + offset < 0)
            return -ERRROR;
        else
            file_table[fd].offset += offset;
    }
    else if(whence == SEEK_END)
    {
        if(offset + sizeof(struct File) < 0)
            return -EINVAL;
        file_table[fd].offset = file_table[fd].ino.size + offset;
    }
    else
        return -EINVAL;
    return file_table[fd].offset;
}