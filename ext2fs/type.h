#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
#include <sys/stat.h>
#include <ext2fs/ext2_fs.h>
#include <time.h>
#include <stdbool.h>


#define BLKSIZE 1024
#define BBITMAP 3

// Block number of EXT2 FS on FD
#define SUPERBLOCK 1
#define GDBLOCK 2
#define ROOT_INODE 2


// Defualt directories and regular file nodes
#define DIR_MODE 0x41ED
#define FILE_MODE 0x81AE
#define SUPER_MAGIC 0xEF53
#define SUPER_USER 0

// PROC status
#define FREE 0
#define BUSY 1
#define RUNNING 2
#define READY 3


// File system table sizes
#define NMINODES 100
#define NMTABLE 10
#define NPROC 10
#define NFD 10
#define NOFT 40

// Defining shorter TYPES
typedef struct ext2_group_desc GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode INODE;
typedef struct ext2_dir_entry_2 DIR; // Needed for new version of e2fs


// In-memory inodes structure
typedef struct minode {
    INODE INODE; // disk inode
    int dev, ino;
    int refCount; // use count
    int dirty; // modified flag
    int mounted; // mounted flag
    struct mount * mntPtr; // mount table pointer
} MINODE;


// Open file table
typedef struct oft {
    int mode;
    int refCount; // number of PROCs sharing this instance
    MINODE * minodePtr;
    int offset; // byte offset for R/W
} OFT;


// PROC structure
typedef struct proc {
    struct Proc * next;
    int pid;
    int uid;
    int gid;
    int ppid;
    int status;
    struct minode * cwd;
    OFT *fd[NFD];
} PROC;


// Mount Table structure
typedef struct mtable {
    int dev; // device number
    int ninodes; // from superblock
    int nblocks;
    int free_blocks;
    int free_inodes;
    int bmap;
    int imap;
    int iblock;
    MINODE * mntDirPtr; // mount point DIR pointer
    char devName[64]; // device name
    char mntName[64]; // mount point DIR name
} MTABLE;

MTABLE * mp;