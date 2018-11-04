#include "disk.h"
#include "util.h"
#include "kprintf.h"
#include "errno.h"

void selectSector(unsigned int sector)
{
    outb(0x1f6, 0xe0 | sector >> 24);   //select device and sector num
    outb(0x3f6, 0x02);                  //turn off interrupts
    outb(0x1f2, 0x01);		            //indicate read range 0 is for full 256 sectors
	outb(0x1f3, sector);                //low 8bits
	outb(0x1f4, sector >> 8);           //next 8bits
	outb(0x1f5, sector >> 16);	        //next 8bits
}

int isBusy(void)
{
    return inb(0x1f7) & 0x80; //return 0: not busy, or 1: busy
}

int isDiskReady(void)
{
    while(isBusy()){;}                              //wait while disk is working
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
        while(isBusy()){;}

        selectSector(sector);
        outb(0x1f7, 0x20);		 //start a read

        if(isDiskReady() != 1)
        {
            logString("READ Disk Error!!\n");
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
    return SUCCESS;
}

int disk_write_sector(unsigned sector, const void* datablock, unsigned numSec)
{
    while(numSec--)
    {
        while(isBusy()){;}

        selectSector(sector);
        outb(0x1f7, 0x30);		 //start writting

        if(isDiskReady() != 1)
        {
            logString("WRITE Disk Error!!\n");
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
    return SUCCESS;
}

int disk_read_block(unsigned blockNum, void *bg)
{
    unsigned spb = 8;                                           //number of sectors per block, 512byte sectors
    unsigned blockSecStart = blockNum * spb;                    //Convert blocks to units of sectors
    signed pass;
    if((pass = disk_read_sector(blockSecStart, bg, spb)) < 0)   //read in full block and check for errors
        return pass;

    return SUCCESS;
}

int disk_read_partial(unsigned blockNum, void *bg, unsigned start, unsigned count)
{
    static char buffer[BLOCK_SIZE];
    int pass;
    if((pass = disk_read_block(blockNum, bg)) < 0 
     || start+count >= BLOCK_SIZE)                  //check if reading out of bounds of array
        return pass;

    kmemcpy(bg, buffer+start, count);
    return SUCCESS;
}

int disk_read_inode(unsigned num, struct Inode** ino)
{
    static char buffer[4096];
    unsigned inodeFromTableStart    = num % SB.inodes_per_group;
    unsigned InodeBlock             = 4 + (((num-1) / SB.inodes_per_group) * SB.blocks_per_group);
    unsigned blocksFromTableStart   = (sizeof(struct Inode) * inodeFromTableStart) / BLOCK_SIZE;
    unsigned inodesToSkipOver       = inodeFromTableStart % (BLOCK_SIZE / sizeof(struct Inode));
    signed pass;
    if((pass = disk_read_block(InodeBlock + blocksFromTableStart, buffer)) < 0)
        return pass;
    *ino = (struct Inode*)(buffer + inodesToSkipOver*sizeof(struct Inode));
    return SUCCESS;
}

void list_SB_info()
{
    kprintf("Volume Label: %.*s  Free:%d\n",16, SB.volname, SB.free_block_count);
    kprintf("BlocksPerGroup: %d  TotalBlocks: %d\n", SB.blocks_per_group, SB.block_count);
}

int list_BGDTS_info()
{
    signed pass;
    unsigned bg_num = 0, Groups, block, bgd_num;    //blockGroup, Num Groups, BGD num
    static union blockGroup BG;
    Groups = SB.block_count / SB.blocks_per_group;  //Get num block groups in memory
    //BGDT INFO
    while(bg_num <= Groups)
    {
        //Populate BGDT for BlockGroup
        block = 1 + (SB.blocks_per_group*(bg_num++));
        kprintf("Reading BGDT from Group %d\nReading starting at block %d\n", bg_num, block);
        if((pass = disk_read_block(block, &BG)) < 0)
        {
            logString("Failed to read blockGroup\n");
            return pass;
        }
        //three BGD's per BGDT
        for(bgd_num = 0; bgd_num <= Groups; bgd_num++)
            kprintf("Group %d: Free Blocks = %d\n", bgd_num, BG.bgd[bgd_num].free_blocks);
    }
    return SUCCESS;
}

int checkDirs(unsigned inodeWanted, unsigned subIndent, unsigned listDirs, unsigned fileNameLen, const char* fileName)
{
    struct Inode* inode, *dirInode;
    struct DirEntry *dir;
    static char buffer[4096];
    char* p;
    signed pass;
    unsigned dirNum = 0, offset = 0, atEnd = 1, tmpCounter = subIndent;

    if((pass = disk_read_inode(inodeWanted, &inode)) < 0)
        return pass;
    if(inode->size <= 0)
    {
        ksprintf(buffer,"Inode:%d is empty size:%d\n", inodeWanted, inode->size);
        logString(buffer);
        if(inode->size == 0)
            return SUCCESS;
        return -ENOENT;
    }
    else
    {
        while(dirNum < 12 || atEnd)
        {
            if((pass = disk_read_block(inode->direct[dirNum], buffer)) < 0)   //get directory
                return pass;
            p = &buffer[0];                                                   //point to start of dir entries
            while(offset < 4096 && atEnd)
            {
                dir = (struct DirEntry*)(p+offset);
                if(dir->rec_len == 0 || (offset + dir->rec_len) > 4096)
                    atEnd = 0;
                else if(dir->inode > 0)
                {
                    if((pass = disk_read_inode(dir->inode-1, &dirInode)) < 0)
                        return pass;
                    if(dir->name_len == fileNameLen && fileNameLen > 0)         //check FileName with fileName passed
                    {
                        tmpCounter = 0;
                        while(tmpCounter < fileNameLen)
                        {
                            if(fileName[tmpCounter] != dir->name[tmpCounter])
                                break;
                            else if(fileName[tmpCounter] == dir->name[tmpCounter]     //found file
                                    && tmpCounter == fileNameLen - 1)
                            {
                                //kprintf("found dir:%.*s, dirInode:%d dirInodeSize:%d\n",dir->name_len, dir->name, dir->inode, dirInode->size);
                                return dir->inode-1;
                            }
                            tmpCounter++;
                        }
                    }
                    if(listDirs)
                    {
                        tmpCounter = subIndent;
                        while(tmpCounter--) kprintf("\t");
                            kprintf("< %u> %.*s size:%d\n",dir->inode, dir->name_len, dir->name, dirInode->size);//print dir name
                    }
                    // if((dirInode->mode>>12) & 4)    //directory dir
                    // {
                    //     if((pass = checkDirs(dir->inode, subIndent+1, listDirs, fileNameLen, fileName)) < 0)
                    //         return pass;
                    //     if((pass = disk_read_block(inode->direct[dirNum], buffer)) < 0)   //get directory
                    //         return pass;
                    //     p = &buffer[0];
                    // }
                    offset += dir->rec_len;                                     //adjust offset count for block size
                    tmpCounter = subIndent;
                }
            }
            offset = 0;
            dirNum++;
        }
    }
    if(fileNameLen > 0)
        return -ENOENT;
    else
        return SUCCESS;
}

int listDiskInfo(void)
{
    char* emptyBuff[1] = {'\0'};
    signed pass;
    list_SB_info();
    if((pass = list_BGDTS_info()) < 0)
    {
       logString("ERROR: Bad BGDT Read\n");
       return pass;
    }
    if((pass = checkDirs(1, 0, 1, 0, *emptyBuff)) < 0)         //list root dirs
    {
        logString("ERROR: Bad directory Read\n");
        return pass;
    }
    logString("Done\n");
    return SUCCESS;
}
int disk_init(void)
{
    //SUPERBLOCK INFO
    signed pass;
    if((pass = disk_read_sector(2, &SB, 2)) < 0)    //main SUPERBLOCK 1KB big starts at offset of 1KB
    {
        logString("Failed to read SuperBlock");
        return pass;
    }
    return SUCCESS;
}