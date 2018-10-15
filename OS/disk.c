#include "disk.h"
#include "util.h"
#include "kprintf.h"

void selectSector(unsigned int sector)
{
    outb(0x1f6, 0xe0 | sector >> 24);   //select device and sector num
    outb(0x3f6, 0x02);                  //turn off interrupts
    outb(0x1f2, 0x01);		            //indicate read range 0 is for full 256 sectors
	outb(0x1f3, sector);                //low 8bits
	outb(0x1f4, sector >> 8);           //next 8bits
	outb(0x1f5, sector >> 16);	        //next 8bits
}

int isBusy()
{
    return inb(0x1f7) & 0x80; //return 0: not busy, or 1: busy
}

int isDiskReady()
{
    while(isBusy())                                 //wait while disk is working
        ;//logString("Waiting for disk_ready!!\n\0");
    for(;;)
    {
        if(inb(0x1f7) & 8)                          //ready
            return 1;
        else if(inb(0x1f7) & 1 || inb(0x1f7) & 32)	//error bit toggled
            return -1;
    }
}

int disk_read_sector(unsigned sector, void* datablock, unsigned numSec)
{
    unsigned dataOffset = 0;
    while(numSec--)
    {
        while(isBusy())
            ;//logString("Waiting before disk_read!!\n\0");

        selectSector(sector);
        outb(0x1f7, 0x20);		 //start a read

        if(isDiskReady() == 1)
            ;//logString("Disk Ready\n\0");
        else
        {
            logString("Disk Error!!\n\0");
            return -1;
        }
        unsigned index;
        for(index = 0; index < 256; ++index)
        {
            unsigned short data = inw(0x1f0);
            ((unsigned short*)datablock)[index+dataOffset] = data;
        }
        sector++;
        dataOffset += 256;
    }
    return 0;
}

int disk_write_sector(unsigned sector, const void* datablock, unsigned numSec)
{
    while(numSec--)
    {
        while(isBusy())
            ;//logString("Waiting before disk_write!!\n\0");

        selectSector(sector);
        outb(0x1f7, 0x30);		 //start writting

        if(isDiskReady() == 1)
            ;//logString("Disk Ready\n\0");
        else
        {
            logString("Disk Error!!\n\0");
            return -1;
        }
        unsigned short* dataToWrite = (void*)datablock;
        unsigned index;
        for(index = 0; index < 256; ++index)
        {
            outw(0x1f0, *dataToWrite);
            dataToWrite++;
        }
        outb(0x1f7, 0xe7);      //flush
        sector++;
    }
    return 0;
}

int disk_read_block(unsigned blockNum, void *bg)
{
    unsigned spb = 8;                                           //number of sectors per block, 512byte sectors
    unsigned blockSecStart = blockNum * spb;                    //Convert blocks to units of sectors
    signed pass;
    if((pass = disk_read_sector(blockSecStart, bg, spb)) < 0)   //read in full block and check for errors
        return pass;

    return 0;
}

int disk_read_partial(unsigned blockNum, void *bg, unsigned start, unsigned count)
{
    static char buffer[4096];
    int pass;
    if((pass = disk_read_block(blockNum, bg)) < 0 
     || start+count > 4095)                  //check if reading out of bounds of array
        return pass;

    kmemcpy(bg, buffer+start, count);
    return 0;
}

int disk_read_inode(unsigned num, struct Inode* ino)
{
    static char buffer[4096];                                            //1 block is 4KB
    unsigned InodeBlock = 4 + ((num / 32) * SB.blocks_per_group);       //InodeBlockGroup
    signed pass;
    if((pass = disk_read_block(InodeBlock, buffer)) < 0)
        return pass;

    ino = (struct Inode*)buffer;
    char msg[50];
    ksprintf(msg, "InodeBlockReading:%d\nInodeSize:%d\n", InodeBlock, ino[2].size);
    logString(msg);
    return 0;
}

int list_dir(int inodeWanted)
{
    static char buffer[4096];
    static struct Inode *inode;
    struct DirEntry *dir;
    signed pass;
    unsigned dirInode;
    if(inodeWanted <= 0)    //give them first inode to read
        dirInode = 0;   
    else
        dirInode = inodeWanted - 1;

    if((pass = disk_read_inode(dirInode, inode)) < 0)
        return pass;

    unsigned offset = 0, dirNum = 0, atEnd = 1;
    while((offset < inode[dirInode].size  && dirNum < 12 && atEnd) && offset != inode[dirInode].size)
    {
        if((pass = disk_read_block(inode[dirInode].direct[dirNum++], buffer)) < 0)  //get dir
            return pass;
        dir = (struct DirEntry*) buffer;
        if(dir->rec_len == 0)
            atEnd = 0;
        else if(dir->inode > 0)
        {
            kprintf("< %d> %*.s\n", dir->inode, dir->name_len, dir->name);          //print dir name
            offset += dir->rec_len;                                                 //adjust offset count for block size
        }
        logString("listingDir\n");
    }
    return 0;
}

int listDiskInfo()
{
    unsigned bg_num = 0, Groups;                     //blockGroup, Num Groups, BGD num
    static union blockGroup BG;

    //SUPERBLOCK INFO
    signed pass;
    if((pass = disk_read_sector(2, &SB, 2)) < 0)    //main SUPERBLOCK 1KB big starts at offset of 1KB
    {
        logString("Failed to read SuperBlock");
        return pass;
    }

    kprintf("Volume Label: %.*s  Free:%d\n",16, SB.volname, SB.free_block_count);
    kprintf("BlocksPerGroup: %d  TotalBlocks: %d\n", SB.blocks_per_group, SB.block_count);
    Groups = SB.block_count / SB.blocks_per_group;  //Get num block groups in memory

    //BGDT INFO
    unsigned block;
    while(bg_num <= Groups)
    {
        //Populate BGDT for BlockGroup
        kprintf("Reading BGDT from Group %d\n", bg_num);
        block = 1 + (SB.blocks_per_group*(bg_num++));
        kprintf("Reading starting at block %d\n", block);

        if((pass = disk_read_block(block, &BG)) < 0)
        {
            logString("Failed to read blockGroup\n");
            return pass;
        }
        //three BGD's per BGDT
        unsigned bgd_num;
        for(bgd_num = 0; bgd_num <= Groups; bgd_num++)
            kprintf("Group %d: Free Blocks = %d\n", bgd_num, BG.bgd[bgd_num].free_blocks);
    }

    if(list_dir(2))
    {
        logString("error reading directory info");
        return pass;
    }
    logString("Done\n");

    return 0; //all is good
}