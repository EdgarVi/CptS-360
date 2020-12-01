#include "cd_ls_pwd.c"



int k_mkdir(MINODE *parent_mip, char *name)
{

    int inode_number, block_number, device = parent_mip->dev, i;
    char buf[BLKSIZE], *current;
    MINODE *mip;
    DIR *dp;

    inode_number = ialloc(device); // get index of next free inode
    block_number = balloc(device); // get index of next free block

    mip = iget(device, inode_number);

    // set metadata
    mip->ip.i_mode = DIR_MODE; // DIR type
    mip->ip.i_uid = running->uid;
    mip->ip.i_gid = running->gid;
    mip->ip.i_size = BLKSIZE;
    mip->ip.i_links_count = 2; // link to '.' and '..'
    mip->ip.i_blocks = 2;
    mip->ip.i_atime = time(0L);
    mip->ip.i_mtime = time(0L);
    mip->ip.i_ctime = time(0L);
    mip->ip.i_block[0] = block_number;

    // set new block indices to free
    for(i = 1; i < I_BLOCKS; i++){
        mip->ip.i_block[i] = 0;
    }

    mip->dirty = 1;

    iput(mip);
    get_block(device, block_number, buf);

    // set data for new dir
    current = buf;
    dp = (DIR *)buf;


    dp->inode = inode_number;
	dp->rec_len = 4 * (( 8 + strlen(".") + 3) / 4);
	dp->name_len = strlen(".");
	dp->file_type = EXT2_FT_DIR;
	dp->name[0] = '.';

	current += dp->rec_len;
	dp = (DIR*)current;

	dp->inode = parent_mip->ino;
	dp->rec_len = (block_size - (current - buf));

	dp->name_len = strlen("..");
	dp->file_type = EXT2_FT_DIR;
	dp->name[0] = '.';
	dp->name[1] = '.';

	put_block(device, block_number, buf); // write buf data to this block number

	enter_name(parent_mip, inode_number, name); // enter new dir entry of its parent

    return 0;
}

int my_mkdir(char *pathname)
{

    int i, parent_ino, device = running->cwd->dev;
    char * path, * newdir, * parent = NULL, * child = NULL;
    MINODE * parent_mip;

    // separate dirname and base
    path = dirname(pathname); // path to new file 
    newdir = basename(pathname); // name of new file
    printf("pathname: %s, path: %s, newdir: %s\n", pathname, path, newdir);


    // error checking
    parent_ino = getino(device, path);
    if(parent_ino < 0) {

        printf("ERROR: File does not exist\n");
        
        return -1;

    }

    parent_mip = iget(device, parent_ino);

    if(!S_ISDIR(parent_mip->ip.i_mode)) {
        iput(parent_mip);
        printf("ERROR: Not a directory\n");
        
        return -1;
    }

    if(getino(dev, pathname) > 0) {

        iput(parent_mip);
        printf("ERROR: File already exists\n");
        return -1;
    };

    k_mkdir(parent_mip, newdir); // creates new dir

    parent_mip->ip.i_links_count++;
    parent_mip->ip.i_atime = time(0L); // grab current timestamp
    parent_mip->dirty = 1;

    iput(parent_mip);

    return 0;
}

int k_creat(MINODE *parent_mip, char *name){
    
    int inode_number, blk_number, device = parent_mip->dev, i;
    char buf[BLKSIZE], *current;
    MINODE *mip;
    INODE *ip;
    DIR *dp;

    inode_number = ialloc(device);
    mip = iget(device, inode_number);

    if(inode_number < 0){
        printf("Error: Unable to allocate inode\n");
        return -1;
    }

    ip = &mip->ip;

    ip->i_mode = 0100664;
    ip->i_uid = running->uid;
    ip->i_gid = running->gid;
    ip->i_size = 0;
    ip->i_links_count = 1;
    ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
    ip->i_blocks = 0;


    for(i = 0; i < I_BLOCKS; i++){
        ip->i_block[i] = 0;
    }

    mip->dirty = 1;
    iput(mip);
    
    enter_name(parent_mip, inode_number, name);
    return inode_number;
     
}

int my_creat(char *pathname){

    int ino, pino, i, device = running->cwd->dev;
    char * child, * parent;
    MINODE * mip, * parent_mip;

    // check if path exists and is actually dir
    if(pathname[0] == '/'){
        mip = root;
    } else {
        mip = running->cwd;
    }

    child = basename(pathname);
    parent = dirname(pathname);

    pino = getino(device, parent); // get inode number of parent directory
    
    if(pino < 0) {
        printf("Error: File does not exist\n");
        return -1;
    }

    parent_mip = iget(device, pino);

    if(!S_ISDIR(parent_mip->ip.i_mode)) {
        iput(parent_mip);
        printf("Error: not a dir\n");
        return -1;
    }

    k_creat(mip, child);

    parent_mip->ip.i_atime = time(0L);
    parent_mip->dirty = 1;

    iput(parent_mip); // write parent back to disk
}