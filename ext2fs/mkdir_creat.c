#include "cd_ls_pwd.c"

// enters a [ino, name] tuple as a new dir entry into a parent dir
int enter_name(MINODE *mip, int ino, char *name){
    
    printf("In enter_name()\n\tino: %d, name: %s\n", ino, name);


    int i, ideal_length, need_length, remain;
    char *cp, sbuf[BLKSIZE], temp[256];
    INODE *ip = &mip->INODE;

    need_length = 4*((8 + strlen(name) + 3)/4);  //rec_len needed for the new DIR entry

    // traverse blocks in INODE
    for(i = 0; i < 12;i++){
        if(ip->i_block[i] == 0){
            break;
        }

        // get pmip's data block into sbuf
        get_block(mip->dev,ip->i_block[i],sbuf);

        dp = (DIR *)sbuf; 
        cp = sbuf;

        // step to last entry in the data block
        while(cp + dp->rec_len < sbuf + BLKSIZE)
        {
            // rec_len (except for last direntry)
            ideal_length = 4*((8 + dp->name_len + 3)/4);  

            cp += dp->rec_len;  // advance char_p by rec_len
            dp = (DIR *)cp;     // pull dir_p to next entry
        }

        // remaining space in Data Block after last entry
        remain = dp->rec_len - ideal_length; 

        if(remain >= need_length) {
            // set Last entry rec_len to its ideal length
            dp->rec_len = ideal_length; 

            cp += dp->rec_len;   // advance cp & pull new dir entry
            dp = (DIR *)cp;

            // dp points to a new empty dir entry 
            // where we will enter the new DIR
            dp->rec_len = remain;
            dp->name_len = strlen(name);
            dp->inode = ino;
            strcpy(dp->name,name);
            
            // write the block back to the disk
            put_block(mip->dev,ip->i_block[i],sbuf);
            iput(mip);

            return;
        } else {
            i++;

            // get pmip's new data block into sbuf
            get_block(mip->dev,ip->i_block[i],sbuf);

            dp = (DIR *)sbuf;

            mip->INODE.i_size += BLKSIZE;  // increment parent size by 1024  
            
            // dp points to a new empty data block  
            dp->rec_len = BLKSIZE;
            dp->name_len = strlen(name);
            dp->inode = ino;
            strcpy(dp->name,name);

            //write the block back to the disk
            put_block(mip->dev,ip->i_block[i],sbuf);
            iput(mip);

            return 1;
        }
    }
}

int k_mkdir(MINODE *pmip, char *name)
{
    // allocate inode and disk block
    int i, ino = ialloc(pmip->dev), blk = balloc(pmip->dev);
    if(ino == 0){
        printf("ERROR: Unable to allocate inode\n");
        return -1;
    }
    printf("In k_mkdir\n\tino: %d, blk: %d\n", ino, blk);

    // load MINODE into a minode
    dev = pmip->dev;
    MINODE *mip = iget(dev, ino);

    // initialize the minode
    mip->INODE.i_mode = 0x41ED; // DIR file type
    mip->INODE.i_uid = running->uid;
    mip->INODE.i_gid = running->gid;
    mip->INODE.i_size = BLKSIZE;
    mip->INODE.i_links_count = 2; // for "." and ".."
    mip->INODE.i_blocks = BLKSIZE / 512;
    mip->INODE.i_atime = time(0L);
    mip->INODE.i_mtime = time(0L);
    mip->INODE.i_ctime = time(0L);
    
    // set mip's blocks
    mip->INODE.i_block[0] = blk;
    for(i = 1; i < 12; i++){
        mip->INODE.i_block[i] = 0;
    }

    
    mip->ino = ino;
    mip->dirty = 1;
    iput(mip); // write inode back to disk


    // create data block containing . and ..
    char buf[BLKSIZE];
    dp = (DIR *)buf;
    bzero(buf, BLKSIZE);

    // make . entry in DIR block
    dp->inode = ino;
    dp->rec_len = 12;
    dp->name_len = 1;
    dp->name[0] = '.';

    // make .. entry in DIR block
    dp = (char *)dp + 12;
    dp->inode = pmip->ino; // parent's ino
    dp->rec_len = BLKSIZE - 12;
    dp->name_len = 2;
    dp->name[0] = dp->name[1] = '.';

    put_block(dev, blk, buf); // finally, write to blk on disk
    
    enter_name(pmip, ino, name); // create new INODE under pmip
    pmip->INODE.i_links_count++; // inc parent DIR link count
}

int my_mkdir(char *pathname)
{

    int ino, pino, i;
    char *path, *newdir, *temp;

    // first step is to separate dirname and base
    path = dirname(pathname); // path to new file
    newdir = basename(pathname); // name of new file
    printf("path: %s, newdir: %s\n", path, newdir);


    // check if path exists and is actually dir
    if(pathname[0] == '/'){
        dev = root->dev; //
    } else {
        dev = running->cwd->dev;
    }

    MINODE *pmip, *mip;
    pino = getino(dev, path);
    pmip = iget(dev, pino);

    if(pino == 0){
        printf("ERROR: Path does not exist\n");
        return -1;
    }

    if((pmip->INODE.i_mode & 0x4000) != 0x4000){
        printf("ERROR: Path not a dir\n");
        return -1;
    }

    // check to see if child exists in parent dir

    if(findPath(pathname) == 1){
        printf("ERROR: Path already exists\n");
        return -1;
    }

    // search() returns 0 if nothing is found
    if(search(pmip, newdir) != 0){
        printf("ERROR: this dir already exists\n");
        return -1;
    } 


    // finally call k_mkdir if pass all cases
    printf("Passed all checks, entering k_mkdir\n");
    k_mkdir(pmip, newdir);
}

int k_creat(MINODE *pip, char *name){
    
    int i, ino = ialloc(dev);

    if(ino == 0){
        printf("ERROR: Unable to allocate inode\n");
        return -1;
    }

    dev = pip->dev;

    MINODE *mip = iget(dev, ino);

    mip->INODE.i_mode = 0x81A4; // file type
    mip->INODE.i_uid = running->uid;
    mip->INODE.i_gid = running->gid;
    mip->INODE.i_size = 0;
    mip->INODE.i_links_count = 1; // link to '.'
    mip->INODE.i_blocks = BLKSIZE / 512;
    mip->INODE.i_atime = time(0L);
    mip->INODE.i_mtime = time(0L);
    mip->INODE.i_ctime = time(0L);

    for (i = 0; i < 12; i++){
        mip->INODE.i_block[i] = 0;
    }
    
    mip->dirty = 1;
    iput(mip); // write back to disk

    enter_name(pip, ino, name);

    return 1; 
}

int my_creat(char *pathname){

    int ino, pino, i;
    char *path, *newfile, *temp;

    // check if path exists and is actually dir
    if(pathname[0] == '/'){
        dev = root->dev; //
    } else {
        dev = running->cwd->dev;
    }
    MINODE *pmip, *mip;
    pino = getino(dev, pathname);
    pmip = iget(dev, pino);

    if(findPath(path)){
        path = dirname(pathname); // path to new file
        newfile = basename(pathname); // name of new file

        pino = getino(&dev, path);

        if(pino == 0){
            printf("ERROR: Path does not exist");
            return -1;
        }

        mip = iget(dev, pino);
    } else {
        mip = iget(running->cwd->dev, running->cwd->ino);
        newfile = (char *) malloc((strlen(pathname) + 1) * sizeof(char));
        strcpy(newfile, pathname);
    }

    if ((mip->INODE.i_mode & 0x4000) != 0x4000)
    {
        printf("ERROR: Not a directory!\n");
        return -1;
    }

    if (search(mip, newfile) != 0)
    {
        printf("ERROR: File already exists!\n");
        return -1;
    }
    printf("passed all checks\n");

    k_creat(mip, newfile);
}