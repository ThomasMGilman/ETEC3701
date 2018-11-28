/*
Code written by Thomas Gilman for Operating Systems One taught James Hudson.
Some functions and implementations of code use code from the OS slides,
the rest was written by Thomas Gilman.
*/
#include "file.h"

struct BufferEntry blockbuffer[BUFFERSIZE];

void logString(char* myString);                     //UTIL functions
int kstrln(char *string);
void kmemcpy(void *dst, const void *src, unsigned num);
int kprintf(const char* fmt, ... ) __attribute__((format (printf , 1, 2 ) ));

void read_block(unsigned blocknum, void* buffer)
{
    static int counter=0;
    int index;
    counter++;
    if( counter > 200 )
    {
        for(index = 0; index < BUFFERSIZE; ++index)
            blockbuffer[index].used = 0;
    }
    for(int i=0;i<BUFFERSIZE;++i)
    {
        if(blockbuffer[i].blocknum == blocknum ){
            kmemcpy(buffer, blockbuffer[i].data, BLOCK_SIZE);
            blockbuffer[i].used=1;
            return;
        }
    }
    int v = 0;
    for(int i=0;i<BUFFERSIZE;i++)
    {
        if( blockbuffer[i].used == 0)
            v = i;
    }
    disk_read_block(blocknum, &blockbuffer[v].data);
    blockbuffer[v].blocknum = blocknum;
    blockbuffer[v].used = counter;
    kmemcpy( buffer, blockbuffer[v].data, BLOCK_SIZE );
    return;
}

//mark screen and keyboard in use, not to be written to
int file_init(void)
{
    unsigned index;
    for(index = 0; index < 3; index++)
        file_table[index].in_use = 1;
    return SUCCESS;
}

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
    if(fd == stdout || fd == stderr)
        return -EINVAL;
    struct File* fp = &file_table[fd];
    static char buffer[4096];
    static unsigned U[1024];
    unsigned bi = fp->offset / BLOCK_SIZE;  //Inode Block index
    unsigned bo = fp->offset % BLOCK_SIZE;  //Buffer offset
    unsigned oi, ii, ro, once = 1;
    unsigned remaining = fp->ino.size - fp->offset;
    unsigned byteCount = 0, numToAdd, countToAdd;
    signed pass;
    
    if(fp->in_use <= 0 || fp->offset >= fp->ino.size) //file is not in use or at end of size
        return 0;
    numToAdd = (remaining < count) ? remaining : count;
    while(fp->offset < fp->ino.size && byteCount < numToAdd)
    {
        if(bi < 1024)   //direct & indirect
        {
            if(bo == 0 || once)
            {
                if(bi < 12) //inode direct
                {
                    if((pass = disk_read_block(fp->ino.direct[bi], buffer)) < 0)
                    {
                        logString("ERROR reading inode direct!!\n");
                        return pass;
                    } 
                }
                else        //inode indirect
                {
                    bi -= 12;
                    if((pass = disk_read_block(fp->ino.indirect, U)) < 0)
                    {
                        logString("ERROR reading inode indirect!!\n");
                        return pass;
                    }
                    if((pass = disk_read_block(U[bi],buffer)) < 0)
                    {
                        logString("ERROR reading inode indirect block to buff!!\n");
                        return pass;
                    }
                }
                if(once)
                    once--;
            }
        }
        else            //double & triple indirect
        {
            if(bo == 0 || once)
            {
                if(bi < 1024*1024) //double indirect
                {
                    bi -= 12 + 1024;
                    oi = bi>>10;    //same as bi / 10
                    ii = bi & 1023; //same as bi % 1024
                    if((pass = disk_read_block(fp->ino.doubleindirect, U)) < 0)
                    {
                        logString("ERROR reading inode DoubleIndirect!!!\n");
                        return pass;
                    }
                    if((pass = disk_read_block(U[oi], U)) < 0)
                    {
                        logString("ERROR reading inode DoubleIndirect indirect block!!!\n");
                        return pass;
                    }
                    if((pass = disk_read_block(U[ii], buffer)) < 0)
                    {
                        logString("ERROR reading inode DoubleIndirect indirect block to buff!!!\n");
                        return pass;
                    }
                }
                else               //triple indirect
                {
                    bi -= 12 + 1024 + 1024*1024;
                    ro = bi>>20;            //same as bi/1024/1024
                    oi = (bi>>10)&0x3ff;    //same as (bi/1024)%1024
                    ii = bi&0x3ff;
                    if((pass = disk_read_block(fp->ino.tripleindirect, U)) < 0)
                    {
                        logString("ERROR reading inode Tripledirect!!!\n");
                        return pass;
                    }
                    if((pass = disk_read_block(U[ro], U)) < 0)
                    {
                        logString("ERROR reading inode triple DoubleIndirect!!!\n");
                        return pass;
                    }
                    if((pass = disk_read_block(U[oi], U)) < 0)
                    {
                        logString("ERROR reading inode triple DoubleIndirect!!!\n");
                        return pass;
                    }
                    if((pass = disk_read_block(U[ii], buffer)) < 0)
                    {
                        logString("ERROR reading inode triple indirect block to buff!!!\n");
                        return pass;
                    }
                }
                if(once)
                    once --;
            }
        }
        countToAdd = ((BLOCK_SIZE - bo) < numToAdd) ? (BLOCK_SIZE - bo) : numToAdd;
        kmemcpy(((char*)buf)+byteCount, (buffer + bo), countToAdd);
        //update variables
        byteCount += countToAdd;
        count -= countToAdd;
        fp->offset += countToAdd;
        bo = fp->offset % BLOCK_SIZE;
        bi = fp->offset / BLOCK_SIZE;
        remaining = fp->ino.size - fp->offset;
        numToAdd = (remaining < count) ? remaining : count;
    }
    return byteCount;
}

int file_write(int fd, const void* buf, int count)
{
    if(fd == stdin)
    {
        kprintf("cant write to keyboard!!! fd:%d\n",fd);
        return -ENOSYS; //no such system call
    }
    else
    {
        kprintf("%.*s",count, (char*)buf);
    }
        
    return count;
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
            return -EINVAL;
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