#include "util.c"

int chdir(char *pathname){
    
    int ino = getino(dev, pathname);
    MINODE *mip = iget(dev, ino);     

    if(dir_or_file(mip) == 1) {
        iput(running->cwd);
        running->cwd = mip;
    } else {
        printf("ERROR: Not a dir\n");   
    }
}

// list stats of DIR and FILE in dir
void ls(char *pathname){
    
    int ino, i = 0; 
    MINODE *mip; 
    mip = running->cwd;
    mip->dev = running->cwd->dev;
    char **path, *token;
    

    // show root
    if (strcmp(pathname, "/") == 0)
    {
        showdir(root); // as normal
        iput(mip);
        return;
    }

    // ls by pathname   
    if(pathname[0] == '/'){
        ino = getino(dev, pathname);
        mip = iget(dev, ino);
    }
    
    // ls cwd
    showdir(mip);
    iput(mip);
}            


void showdir(MINODE *mip)
{ 
    char buf[BLKSIZE];

    int i;
    int pos;
    DIR *dp = (DIR*)buf;
    char *cp = buf;
    INODE * parent_ip = &mip->ip;

    // direct blocks
    for(i = 0; i < NUM_DIRECT_BLOCKS; ++i)   
    {


        if(parent_ip->i_block[i] == 0) //note, this is always evaluating to true
        {
            continue;
        }
        
        get_block(dev, parent_ip->i_block[i], buf);
        DIR *dir = (DIR *)buf;
        pos = buf;
        

        // traverse directory entries
        while(pos < buf + BLKSIZE)          
        {
            char dirname[dir->name_len + 1];
            strncpy(dirname, dir->name, dir->name_len);
            dirname[dir->name_len] = '\0';
            MINODE *curmip = iget(mip->dev, dir->inode);

            // print type
            printf((curmip->ip.i_mode & 0x4000) ? "d" : "");
            printf((curmip->ip.i_mode & 0x8000) == 0x8000 ? "-" : "");
            printf((curmip->ip.i_mode & 0xA000) == 0xA000 ? "l" : "");
        
            char *badctime = ctime(&curmip->ip.i_mtime);            
            badctime[24] = '\0';

            // print permissions
            printf( (curmip->ip.i_mode & 0x0100) ? "r" : " -");
            printf( (curmip->ip.i_mode & 0x0080) ? "w" : "-");
            printf( (curmip->ip.i_mode & 0x0040) ? "x" : "-");
            printf( (curmip->ip.i_mode & 0x0020) ? "r" : "-");
            printf( (curmip->ip.i_mode & 0x0010) ? "w" : "-");
            printf( (curmip->ip.i_mode & 0x0008) ? "x" : "-");
            printf( (curmip->ip.i_mode & 0x0004) ? "r" : "-");
            printf( (curmip->ip.i_mode & 0x0002) ? "w" : "-");
            printf( (curmip->ip.i_mode & 0x0001) ? "x" : "-");
        
            printf("\t%d\t%d\t%d\t%s\t%d\t%s", curmip->ip.i_links_count, curmip->ip.i_gid, curmip->ip.i_uid, badctime, curmip->ip.i_size, dirname);
            if (S_ISLNK(curmip->ip.i_mode)) {
                printf(" -> %s\n", (char *) curmip->ip.i_block);
            } else {
                printf("\n");
            }

            iput(curmip);
            char *loc = (char *)dir;
            loc += dir->rec_len;
            pos += dir->rec_len;
            dir = (DIR *)loc;
        }
    }

}

// recursive print working directory
int rpwd(MINODE *wd){
    printf("CWD = ");
    pwd(running->cwd, 0);
    printf("\n");
    return 0;
}

int pwd(MINODE *wd, int childIno){
    if(wd->ino == root->ino){
        printf("/");
    }

    DIR *dir;
    MINODE *parent;
    char buf[BLKSIZE], *cp, name[64];
    get_block(fd, wd->ip.i_block[0], (char *)&buf);

    dir = (DIR *)buf;
    cp = buf + dir->rec_len;
    dir = (DIR *)cp;

    if(wd->ino != root->ino){
        int ino = dir->inode;
        parent = iget(fd, ino);
        pwd(parent, wd->ino);
    } 

    if(childIno != 0){
        while(dir->inode != childIno){
            cp += dir->rec_len;
            dir = (DIR *)cp;
        }
        strncpy(name, dir->name, dir->name_len);
        name[dir->name_len] = '\0';
        printf("%s/", name);
        
    }

    return;
}