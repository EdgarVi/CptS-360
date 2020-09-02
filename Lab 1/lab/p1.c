/************ PART 1 PARTITION TABLE ************/
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