#include "read_cat.c"

char * rootdev = "mydisk"; // default root device if none given


// initialize fs data structures
void mount_root(int dev) {
    
    char buf[BLKSIZE];

    int i, j;
    MINODE * mip;
    MTABLE * mtp;
    PROC * p;
    OFT *ofp;

    // initialize all minodes as FREE
    for(i = 0; i < NMINODES; i++) {
        mip = &minode[i]; // use mip to manipulate inodes
        mip->dev = mip->ino = 0;
        mip->refCount = 0;
        mip->mounted = 0;
        mip->mntPtr = 0;
        mip->dirty = 0;
    }

    // initialize PROCs
    for(i = 0; i < NPROC; i++) {
        p = &proc[i];
        p->pid = i;
        p->gid = 0;
        p->uid = i;
        p->cwd = NULL;
        p->next = NULL;
        p->status = FREE;
        for(j = 0; j < NFD; j++) {
            p->fd[j] = 0; // all FD are NULL
        }
    }

    for(i = 1; i < NOFT; i++) {
        ofp = &oft[i];
        ofp->mode = 0;
        ofp->refCount = 0;
        ofp->minodePtr = NULL;
        ofp->offset = 0;
        
    }


    // get and record super block
    sp = (SUPER *) malloc (sizeof(SUPER));
    get_block(dev, SUPER_BLOCK_OFFSET, (char *) sp);

    // verify it's an EXT2 FS by checking magic number
    if(sp->s_magic != SUPER_MAGIC) {
        printf("super magic = %x: %s is not an EXT2 file system\n", sp->s_magic, rootdev);
        exit(0);
    }

    ninodes = sp->s_inodes_count;
    nblocks = sp->s_blocks_count;
    block_size = 1024 << sp->s_log_block_size;
    inode_size = sp->s_inode_size;

    // get and record group descriptor block
    gp = (GD *) malloc(BLKSIZE);
    get_block(dev, GD_BLOCK_OFFSET, (char *) gp);

    bmap = gp->bg_block_bitmap; // represents free and used blocks in this group
    imap = gp->bg_inode_bitmap; // represents free and used inodes in this block group 
    iblock = gp->bg_inode_table; // block id of first block of the inode table in this block group
    inodes_per_block = block_size / inode_size;

    get_block(dev, bmap, buf);

    root = iget(dev, ROOT_INODE);
    root->refCount++;

    printf("creating P0 as running proc\n");
    running = &proc[0];
    running->status = READY;
    running->cwd = root;

    printf("root refCount = %d\n", root->refCount);

}


int main(int argc, char * argv[]) {

    
    int i, ino;
    char buf[BLKSIZE];

    if(argc > 1){
        rootdev = argv[1];
        printf("Attempting to open: %s\n", rootdev);    
    }

    fd = open(rootdev, O_RDWR);
    if(fd < 0){
        printf("open %s failed\n", rootdev);
        exit(1);
    }

    dev = fd;

    mount_root(dev);
    printf("mounted %s\n", rootdev);

    // ask for command line args
    while(1) {
        printf("input command: ");
        fgets(line, 128, stdin);
        line[strlen(line) - 1] = 0;

        if(line[0] == 0)
            continue;

        sscanf(line, "%s %s %s", command, pathname, newpath);

        if(!strcmp(command, "ls"))    
            ls(pathname);
        
        if(!strcmp(command, "cd"))
            chdir(pathname);

        if(!strcmp(command, "pwd"))
            rpwd(running->cwd);
            
        if(!strcmp(command, "quit"))
            quit(rootdev);

        if(!strcmp(command, "mkdir"))
            my_mkdir(pathname);

        if(!strcmp(command, "creat"))
            my_creat(pathname);

        if(!strcmp(command, "rmdir"))
            my_rmdir(pathname);

        if(!strcmp(command, "link")) 
            my_link(pathname, newpath);

        if(!strcmp(command, "unlink")) 
            my_unlink(pathname, newpath);

        if(!strcmp(command, "symlink"))
            my_symlink(pathname, newpath);

        if(!strcmp(command, "open"))
            fd = my_open(pathname, newpath);

        if(!strcmp(command, "close"))
            fd = my_close(pathname); // user must supply fd as a string
        
    }

    return 0;
}