#include "file.h"
#include "errno.h"

int kprintf( const char* fmt0, ... );
int disk_read_inode(unsigned num, struct Inode** ino);
int checkDirs(unsigned inodeWanted, unsigned subIndent, unsigned listDirs, unsigned fileNameLen, const char* fileName);

struct File file_table[MAX_FILES];

int file_open(const char* fileName, int flags)
{
    struct File file;
    struct Inode* fileIno;
    int fd = 0, pass;
    while(fd++ < MAX_FILES)
    {
        if(file_table[fd].in_use == 0)   //found free space
            break;
        if(fd == MAX_FILES - 1 && file_table[fd].in_use == 1)
            return -EMFILE;
    }
    file = file_table[fd];
    file.flags = flags;
    //filename: check it
    if((pass = checkDirs(1, 0, 0, kstrln((char*)fileName), fileName)) < 0)
        return -ENOENT;
    if((pass = disk_read_inode(pass, &fileIno)) < 0)
        return -DRIERR;
    kmemcmp(&file.ino, fileIno,sizeof(struct Inode));
    file_table[fd].in_use = 1;
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
    ;
}

int file_write(int fd, const void* buf, int count)
{
    return -ENOSYS; //no such system call
}

int file_seek(int fd, int offset, int whence)
{
    ;
}