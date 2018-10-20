#include "disk.h"
#include "util.h"

#pragma once
#define MAX_FILES 30

struct File file_table[MAX_FILES];

struct File{
    int in_use;         //if using file set to 1
    int flags;          //reading, writing, rdwd, ect...
    int offset;         //need to set to 0 when opening a file
    struct Inode ino;
};

int file_open(const char* fileName, int flags);

int file_close(int fd);
//if fd > MAX_FILES Error
//else set that file_table[fd].in_use = 0;
//then return 0;

//get inode location ex):
//GLOBALS:  inodesPerGroup = 100     blockPerGroup = 1000
//inode I want 3141
//group = (3141-1)/100 = 31
//inodeTableStartingBlock = 1000 * 31 + 4 = 31004
//inodesFromTableStart = (3141-1) % 100 = 40
//bytesFromTableStart = 40*sizeof(Inode) = 40*128 = 5120bytes
//blocksFromTableStart = 5120/4096 = 1

//load_block(alpha + beta, buffer)
//inodesPerBlock = 4096/128 = 32
//inodesToSkip = 40 % 8
//kmemcpy(ino, buffer+inodesToSkipOver*sizeof(inode),sizeof(inode))


//file write{
//DirEntry for foo.txt
//allocate inode -> inode bitmap
//  free inode count-> BGDT <- Several
//  update free space and stuff on superblock
//Allocate data blocks -> blockbitmap
//  free block count-> BGDTs <- several
//  update space on superblock
//write data