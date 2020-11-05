#include "util.c"

char * rootdev = "mydisk"; // default root device if none given


// initialize fs data structures
void fs_init() {
    
    int i, j;
    MINODE * mip;
    MTABLE * mtp;
    PROC * p;

    // initialize all minodes as FREE
    for(i = 0; i < NMINODES; i++) {
        mip = &minode[i]; // use mip to manipulate inodes
        mip->dev = mip->ino = 0;
        mip->refCount = 0;
        mip->mounted = 0;
        mip->mntPtr = 0;
    }

    // initialize PROCs
    for(i = 0; i < NPROC; i++) {
        p = &proc[i];
        p->pid = i;
        p->uid = i;
        p->cwd = 0;
        p->status = FREE;
        for(j = 0; j < NFD; j++) {
            p->fd[j] = 0; // all FD are NULL
        }
    }

    for(i = 1; i < NMTABLE; i++) {
        mtp = &mtable[i];
        mtp->dev = 0;
    }

}

// mount the root file system
void mount_root() {
    root = iget(dev, 2);
}

int main(int argc, char * argv[]) {

    int i, ino;
    char buf[BLKSIZE];

    if (argc > 1)
        rootdev = argv[1];


    fd = open(rootdev, O_RDWR);
    if(fd < 0) {
        printf("open %s failed\n", rootdev);
        exit(1);
    }

    dev = fd;

    // read SUPER block and verify it's an EXT2 fs
    if(sp->s_magic != SUPER_MAGIC) {
        printf("Super magic = %x: %s is not an EXT2 file system\n", sp->s_magic, rootdev);
        exit(0);
    }


    ninodes = sp->s_inodes_count;
    nblocks = sp->s_blocks_count;

    get_block(dev, 2, buf);
    gp = (GD *) buf;

    // initialize bitmap
    bmap = gp->bg_block_bitmap;
    imap = gp->bg_inode_bitmap;
    inode_start = gp->bg_inode_table;
    printf("bmap = %d imap = %d inode_start = %d\n", bmap, imap, inode_start);


    // initialize FS data structure
    fs_init();


    // mount the root file system
    mount_root();


    printf("Setting P0 as running proc\n");
    running = &proc[0];
    running->status = READY;
    running->cwd = iget(dev, 2);

    printf("root refCount = %d\n", root->refCount);

    // ask for command line args
    while(1) {
        printf("input command: ");
        fgets(line, 128, stdin);
        line[strlen(line) - 1] = 0;

        if(line[0] == 0)
            continue;

        sscanf(line, "%s %s %s", command, pathname, newpath);

        if(!strcmp(command, "quit"))
            quit(rootdev);
    }

    return 0;
}