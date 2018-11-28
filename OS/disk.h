/*
Code written by Thomas Gilman for Operating Systems One taught James Hudson.
Some functions and implementations of code use code from the OS slides,
the rest was written by Thomas Gilman.
*/
#pragma once

#define BLOCK_SIZE 4096

struct Superblock SB;

/*SB uses full block 
this is first block in blockgroup
copied into every blockgroup as first block in case of corruption*/
#pragma pack(push,1)
struct Superblock{
    unsigned inode_count;
    unsigned block_count;
    unsigned r_block_count;
    unsigned free_block_count;
    unsigned free_inode_count;
    unsigned first_data_block;
    unsigned logical_block_size;
    unsigned logical_fragment_size;
    unsigned blocks_per_group;
    unsigned fragments_per_group;
    unsigned inodes_per_group;
    unsigned mounttime;
    unsigned writetime;
    unsigned short mountcount;
    unsigned short maxmountcount;
    unsigned short magic;
    unsigned short state;
    unsigned short errors;
    unsigned short minorrev;
    unsigned lastcheck;
    unsigned checktime;
    unsigned creator;
    unsigned revision;
    unsigned short resuid;
    unsigned short resgid;
    unsigned first_inode;
    unsigned short inode_size;
    unsigned short block_group_number;
    unsigned feature_compat;
    unsigned feature_incompat;
    unsigned feature_ro_compat;
    unsigned char uuid[16];
    char volname[16];
    char lastmount[64];
    char reserved[824];
};
#pragma pack(pop)

/*BGD takes up full block but may not use all 4kb.
This is second block in blockgroup*/
#pragma pack(push,1)
struct BlockGroupDescriptor{
    unsigned block_bitmap;
    unsigned inode_bitmap;
    unsigned inode_table;
    unsigned short free_blocks;
    unsigned short free_inodes;
    unsigned short used_dirs;
    unsigned short pad;
    char reserved[12];
};
#pragma pack(pop)

union blockGroup
{
    char block[4096];
    struct BlockGroupDescriptor bgd[1];
};

#pragma pack(push,1)
struct Inode {
    unsigned short mode;
    unsigned short uid;
    unsigned size;
    unsigned atime;
    unsigned ctime;
    unsigned mtime;
    unsigned dtime;
    unsigned short gid;
    unsigned short links;
    unsigned blocks;
    unsigned flags;
    unsigned osd1;
    unsigned direct[12];
    unsigned indirect;
    unsigned doubleindirect;
    unsigned tripleindirect;
    unsigned generation;
    unsigned fileacl;
    unsigned diracl;
    unsigned osd2;
    char reserved[12];
};
#pragma pack(pop)

#pragma pack(push,1)
struct DirEntry{
    unsigned inode;
    unsigned short rec_len;
    unsigned short name_len;
    char name[1]; //might be longer! Variable size
};
#pragma pack(pop)

void selectSector(unsigned int sector);

int isBusy();

int isDiskReady();

int disk_read_sector(unsigned sector, void* datablock, unsigned numSec);

int disk_write_sector(unsigned sector, const void* datablock, unsigned numSec);

int disk_read_block(unsigned blockNum, void *bg);

int disk_read_partial(unsigned blockNum, void*bg, unsigned start, unsigned cout);

int disk_read_inode(unsigned num, struct Inode** ino);

void list_SB_info(void);

int list_BGDTS_info(void);

int checkDirs(unsigned inodeWanted, unsigned subIndent, unsigned listDirs, unsigned fileNameLen, const char* fileName);

int listDiskInfo(void);

int disk_init(void);