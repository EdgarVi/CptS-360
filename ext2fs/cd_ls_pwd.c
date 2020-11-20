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
int ls(char *pathname){
    
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

    get_block(dev, mip->INODE.i_block[0], buf);
    int i;
    DIR *dp = (DIR*)buf;
    char *cp = buf;
 
    for(i = 0; i < 12; ++i) // is 12 correct here?  
    {
        if(mip->INODE.i_block[i] == 0)
        {
            return 0;
        }

        char buffer[BLKSIZE];
        get_block(dev, mip->INODE.i_block[i], buffer);
        DIR *dir = (DIR *)buffer;
        int pos = 0;

        while(pos < BLKSIZE)          
        {
            char dirname[dir->name_len + 1];
            strncpy(dirname, dir->name, dir->name_len);
            dirname[dir->name_len] = '\0';
            MINODE *curmip = iget(mip->dev, dir->inode);

            // print type
            printf((curmip->INODE.i_mode & 0x4000) ? "d" : "");
            printf((curmip->INODE.i_mode & 0x8000) == 0x8000 ? "-" : "");
            printf((curmip->INODE.i_mode & 0xA000) == 0xA000 ? "l" : "");
        
            char *badctime = ctime(&curmip->INODE.i_mtime);            
            badctime[24] = '\0';

            // print permissions
            printf( (curmip->INODE.i_mode & 0x0100) ? "r" : " -");
            printf( (curmip->INODE.i_mode & 0x0080) ? "w" : "-");
            printf( (curmip->INODE.i_mode & 0x0040) ? "x" : "-");
            printf( (curmip->INODE.i_mode & 0x0020) ? "r" : "-");
            printf( (curmip->INODE.i_mode & 0x0010) ? "w" : "-");
            printf( (curmip->INODE.i_mode & 0x0008) ? "x" : "-");
            printf( (curmip->INODE.i_mode & 0x0004) ? "r" : "-");
            printf( (curmip->INODE.i_mode & 0x0002) ? "w" : "-");
            printf( (curmip->INODE.i_mode & 0x0001) ? "x" : "-");
        
            printf("\t%d\t%d\t%d\t%s\t%d\t%s", curmip->INODE.i_links_count, curmip->INODE.i_gid, curmip->INODE.i_uid, badctime, curmip->INODE.i_size, dirname);
            if (S_ISLNK(curmip->INODE.i_mode))
                printf(" -> %s\n", (char *) curmip->INODE.i_block);
            
            else
                printf("\n");

            iput(curmip);
            char *loc = (char *)dir;
            loc += dir->rec_len;
            pos += dir->rec_len;
            dir = (DIR *)loc;
        }
    }

    return 0;
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
    get_block(fd, wd->INODE.i_block[0], (char *)&buf);

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