/************ PART 1 PARTITION TABLE ************/

/*
Contents of virtual disk: 

Device     Boot Start   End Sectors  Size Id Type
vdisk1             18   359     342  171K  7 HPFS/NTFS/exFAT
vdisk2            360   719     360  180K 83 Linux
vdisk3            720  1439     720  360K  c W95 FAT32 (LBA)
vdisk4           1440  2879    1440  720K  5 Extended
vdisk5           1458  1799     342  171K  6 FAT16
vdisk6           1818  2339     522  261K 82 Linux swap / Solaris
vdisk7           2358  2879     522  261K 83 Linux

*/


#include <stdio.h>
#include <fcntl.h> // needed for file descriptor
#define EXTEND_TYPE 5

char buff[512];

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

struct partition {
    u8 drive;             /* drive number FD=0, HD=0x80, etc. */

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

char *dev = "vdisk";
int fd;

int read_sector(int fd, int sector, char *buf)
{
    // same as shown above
}

int main()
{
  struct partition *p;
  char buf[512];

  fd = open(dev, O_RDONLY);   // open dev for READ

   read(fd, buf, 512);        // read MBR into buf[]
   // read_sector(fd, 0, buf);    // OR call read_sector()    
   p = (struct partition *)(&buf[0x1be]); // p->P1
   
   printf("%8d %8d %8x\n", p->start_sector, p->nr_sectors, p->sys_type);

   // Write YOUR code to print all 4 partitions


   // ASSUME P4 is EXTEND type; 
   p += 3;      // p-> P4
   printf("P4 start_sector = %d\n", p->start_sector);
   
   read_sector(fd, p->start_sector, buf);

   p = (struct partition *)&buf[0x1BE];    // p->localMBR
   printf("FIRST entry of localMBR\n");
   printf("start_sector=%d, nsectors=%d\n", p->start_sector, p->nr_sectors);

   // Write YOUR code to get 2nd entry, which points to the next localMBR, etc.
   // NOTE: all sector numbers are relative to P4's start_sector
}