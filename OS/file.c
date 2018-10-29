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
    file_table[fd].flags = flags;
    //filename: check it
    if((pass = checkDirs(1, 0, 0, kstrln((char*)fileName), fileName)) < 0)
        return -ENOENT;
    ksprintf(debugMsg, "fd:%d, fileInode at:%d\t", fd, pass);
    logString(debugMsg);
    if((pass = disk_read_inode(pass, &fileIno)) < 0)
        return -DRIERR;
    file_table[fd].ino = *fileIno;
    file_table[fd].in_use = 1;
    file_table[fd].offset = 0;

    ksprintf(debugMsg,"file opened:%s, size:%d, fileOffset:%d\n",fileName, file_table[fd].ino.size, file_table[fd].offset);
    logString(debugMsg);
    return fd;
}

int file_close(int fd)
{
    if(fd >= MAX_FILES) //no such file
        return -EINVAL;
    if(file_table[fd].in_use == 0) //file not open
        return -EINVAL;
    file_table[fd].in_use = 0;  //set file for not in use so close
    return SUCCESS;
}

int file_read(int fd, void* buf, int count)
{
    struct File* fp = &file_table[fd];
    ksprintf(debugMsg, "fd:%d", fd);
    logString(debugMsg);
    unsigned bi = fp->offset / BLOCK_SIZE;  //Inode Block index
    unsigned bo = fp->offset % BLOCK_SIZE;  //Buffer offset
    signed pass;
    int remaining = fp->ino.size - bo, byteCount = 0;
    static char buffer[4096];
    if(fp->in_use <= 0 || fp->offset >= fp->ino.size) //file is not in use or at end of size
    {
        ksprintf(debugMsg, "here: byteCount:%d,count:%d,fileOffset:%d,inodeSize:%d,remaining:%d,bo:%d,bi:%d\n"
            ,byteCount,count, fp->offset, fp->ino.size, remaining, bo, bi);
        logString(debugMsg);
        return 0;
    }
    if(fp->ino.size < 48000) //small file
    {
        while((byteCount < count) && (bi < 12) && (fp->offset < fp->ino.size))
        {
            if((pass = disk_read_block(fp->ino.direct[bi], buffer)) < 0)
                return pass;

            kmemcpy((char*)buf+byteCount, buffer + bo + byteCount, (remaining < count) ? remaining : count);
            //update variables
            byteCount += (remaining < count) ? remaining : count;
            fp->offset += (remaining < count) ? remaining : count;

            ksprintf(debugMsg, "byteCount:%d,count:%d,fileOffset:%d,inodeSize:%d,remaining:%d,bo:%d,bi:%d\ncontents:%.*s\n"
            ,byteCount,count, fp->offset, fp->ino.size, remaining, bo, bi, (remaining<count)?remaining:count, buffer+bo);
            logString(debugMsg);

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
        ksprintf(debugMsg,"fileSize:%d, newOffset:%d\n",file_table[fd].ino.size, offset);
        logString(debugMsg);
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
        file_table[fd].offset = sizeof(file_table[fd]) + offset;
    }
    else
        return -EINVAL;
    return file_table[fd].offset;
}