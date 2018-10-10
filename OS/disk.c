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
        logString("Waiting for disk_ready!!\n\0");
    for(;;)
    {
        if(inb(0x1f7) & 8)                          //ready
            return 1;
        else if(inb(0x1f7) & 1 || inb(0x1f7) & 32)	//error bit toggled
            return -1;
    }
}

void disk_read_sector(unsigned sector, void* datablock, unsigned numSec)
{
    unsigned dataOffset = 0;
    while(numSec--)
    {
        while(isBusy())
            logString("Waiting before disk_read!!\n\0");

        selectSector(sector);
        outb(0x1f7, 0x20);		 //start a read

        if(isDiskReady() == 1)
            logString("Disk Ready\n\0");
        else
        {
            logString("Disk Error!!\n\0");
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
}

void disk_write_sector(unsigned sector, const void* datablock, unsigned numSec)
{
    while(numSec--)
    {
        while(isBusy())
            logString("Waiting before disk_write!!\n\0");

        selectSector(sector);
        outb(0x1f7, 0x30);		 //start writting

        if(isDiskReady() == 1)
            logString("Disk Ready\n\0");
        else
        {
            logString("Disk Error!!\n\0");
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
}

void disk_read_block(unsigned blockNum, void *bg)
{
    unsigned spb = 8;                               //number of sectors per block, 512byte sectors
    unsigned blockSecStart = blockNum * spb;        //Convert blocks to units of sectors
    disk_read_sector(blockSecStart, bg, spb);       //read in full block
}

// void disk_read_inode(unsigned inodeNum, void *In)
// {

// }

void listDiskInfo()
{
    unsigned bg_num = 0, Groups;           //blockGroup, Num Groups, BGD num
    static union blockGroup BG;
    //struct Inode root;

    //SUPERBLOCK INFO
    disk_read_sector(2, &SB, 2);                    //main SUPERBLOCK 1KB big starts at offset of 1KB
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
        disk_read_block(block, &BG);
        
        //three BGD's per BGDT
        unsigned bgd_num;
        for(bgd_num = 0; bgd_num <= Groups; bgd_num++)
            kprintf("Group %d: Free Blocks = %d\n", bgd_num, BG.bgd[bgd_num].free_blocks);
    }

    //Get root directory Info
    //disk_read_block(4, BG.block);
    
}