#include "disk.h"
#include "util.h"

#pragma once
#define MAX_FILES 30

struct File{
    int in_use;         //if using file set to 1
    int flags;          //reading, writing, rdwd, ect...
    int offset;         //need to set to 0 when opening a file
    struct Inode ino;
};

int file_open(const char* fileName, int flags);

int file_close(int fd);

int file_read(int fd, void* buf, int count);

int file_write(int fd, const void* buf, int count);

int file_seek(int fd, int offset, int whence);

//file write{
//DirEntry for foo.txt
//allocate inode -> inode bitmap
//  free inode count-> BGDT <- Several
//  update free space and stuff on superblock
//Allocate data blocks -> blockbitmap
//  free block count-> BGDTs <- several
//  update space on superblock
//write data