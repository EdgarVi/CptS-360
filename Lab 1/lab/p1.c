/*
Contents of vdisk:
Device     Boot Start   End Sectors  Size Id Type
vdisk1             18   359     342  171K  7 HPFS/NTFS/exFAT
vdisk2            360   719     360  180K 83 Linux
vdisk3            720  1439     720  360K  c W95 FAT32 (LBA)
vdisk4           1440  2879    1440  720K  5 Extended
vdisk5           1458  1799     342  171K  6 FAT16
vdisk6           1818  2339     522  261K 82 Linux swap / Solaris
vdisk7           2358  2879     522  261K 83 Linux
*/

/**************PART 1 PARTITION TABLE*********************/
#include <stdio.h>
#include <fcntl.h> // needed for file descriptor
#define EXTEND_TYPE 5

char buff[512]; // buffer of size 512

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

struct partition {
	u8 drive;             /* drive number FD=0, HD=0x80, etc. */

    // for old floppy disks, not relevant
	u8  head;             /* starting head */
	u8  sector;           /* starting sector */
	u8  cylinder;         /* starting cylinder */

	u8  sys_type;         /* partition type: NTFS, LINUX, etc. */

	u8  end_head;         /* end head */
	u8  end_sector;       /* end sector */
	u8  end_cylinder;     /* end cylinder */

	u32 start_sector;     /* starting sector counting from 0 */
	u32 nr_sectors;       /* number of of sectors in partition */
};


int main(int argc, char *argv[ ], char *env[ ])
{ 
    int fd = open("vdisk", O_RDONLY); // open vdisk for read
    read(fd, buff, 512);
    struct partition * p = (struct partition *)&buff[0X1BE];

    // output header
    printf("Device Boot Start End Sectors ID\n");
    
    int i = 1;
    int EBRFound = 0;
    while(EBRFound != 1)
    {
        
        // regular case
        int end_sectors = (p->start_sector + p->nr_sectors) - 1;
        printf("vdisk%d  %d   (%d, %d)  %x\n", i, p->start_sector, end_sectors, p->nr_sectors, p->sys_type);
        
        // case we've reached the EXTEND PARTITION
        if(p->sys_type == EXTEND_TYPE)
        {
            EBRFound = 1;

            // both functions work, toggle between output format
            printExtendPartition(i + 1, p, fd);
            //printFormattedExtendPartition(p, fd); 
        }
        
        p++; 
        i++;
    }
    

}

getSector(int fd, int sector, char buff[])
{
    lseek(fd, (long)(sector*512), SEEK_SET);
    read(fd, buff, 512);
}

void printExtendPartition(int vDiskIndex, struct partition * p, int fd)
{

    printf("--------EXTEND PARTITION---------\n");
    int extStart = ((p->start_sector + p->nr_sectors) - 1);
    printf("extStart = %d\n", extStart);
    int n = 0;

    // get first sector
    n = p->start_sector;
    getSector(fd, n, buff);
    p = (struct partition *)&buff[0X1BE];
    printf("vdisk%d  %d  (%d, %d)  %x\n", vDiskIndex, p->start_sector + n, ((n + p->nr_sectors) - 1 + 18), p->nr_sectors, p->sys_type);
    vDiskIndex++;
    
    // get all following sectors
    do{
        n += p->start_sector + p->nr_sectors;
        getSector(fd, n, buff);
        p = (struct partition *)&buff[0X1BE];
        printf("vdisk%d  %d  (%d, %d)  %x\n", vDiskIndex, p->start_sector + n, ((n + p->nr_sectors) - 1 + 18), p->nr_sectors, p->sys_type);
        vDiskIndex++; 
        if(vDiskIndex == 10){
            break;
        }
    } while(((n + p->nr_sectors) - 1 + 18) != extStart);

}

// print extended partition as shown in KC's example
void printFormattedExtendPartition(struct partition * p, int fd)
{
    printf("--------EXTEND partition---------\n");    
    int extStart = ((p->start_sector + p->nr_sectors) - 1);
    printf("start_sector: %d, end_sector: %d\n", p->start_sector, extStart);
    int n = 0;


    // get first sector
    n = p->start_sector;
    getSector(fd, n, buff);
    p = (struct partition *)&buff[0X1BE];
    printf("-------EXTEND partition raw data---------\n");
    printf("start_sector: %d nr_sectors: %d\nstart_sector: %d nr_sectors: %d\n", p->start_sector, p->nr_sectors, (p->start_sector + p->nr_sectors), (p->nr_sectors + p->start_sector));
    printf("------EXTEND partition data---------\n");
    printf("begin_sector: %d  end_sector: %d nr_sectors: %d  type: %x\n", p->start_sector + n, ((n + p->nr_sectors) - 1 + 18), p->nr_sectors, p->sys_type);
    
    
    // get all following sectors
    do{
        n += p->start_sector + p->nr_sectors;
        int nr_sectorRaw = (p->start_sector + p->nr_sectors); // get from previous
        getSector(fd, n, buff);
        p = (struct partition *)&buff[0X1BE];
        printf("-------EXTEND partition raw data---------\n");
        printf("start_sector: %d nr_sectors: %d\nstart_sector: %d nr_sectors: %d\n", p->start_sector, p->nr_sectors, nr_sectorRaw, (p->nr_sectors + p->start_sector));
        printf("------EXTEND partition data---------\n");
        printf("begin_sector: %d  end_sector: %d nr_sectors: %d  type: %x\n", p->start_sector + n, ((n + p->nr_sectors) - 1 + 18), p->nr_sectors, p->sys_type);
        
    } while(((n + p->nr_sectors) - 1 + 18) != extStart);
}