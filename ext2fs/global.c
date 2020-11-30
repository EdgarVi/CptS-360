#include "type.h"


GD * gp;
SUPER * sp;
INODE * ip;
DIR * dp;
MINODE * root;

// Globals
int fd;
int ninodes, nblocks, block_size, inode_size, inodes_per_block; // ninodes, nblocks from SUPER
int bmap, imap, iblock; // BMAP, IMAP, inodes from start block numbers
int dev;
int inode_start;

char gpath[128]; // token strings
int n; // number of token strings
char * name[64]; // pointers to token strings
char buff[BLKSIZE];

// newpath is second path for link
char line[128], command[32], pathname[64], newpath[64];


MINODE minode[NMINODES]; // in memory INODEs
MTABLE mtable[NMTABLE]; // mount tables
OFT oft[NOFT]; // opened file instances
PROC proc[NPROC]; // PROC structures
PROC proc[NPROC], * running; // current executing PROC