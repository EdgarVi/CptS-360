#include "util.c"

int chdir(char *pathname){
    
    int ino;
    MINODE *mip, * old_cwd = running->cwd;

    ino = getino(dev, pathname);


    if(ino < 0) {
        printf("ERROR: File does not exist\n");
        return - 1;
    }

    mip = iget(dev, ino);

    if(!S_ISDIR(mip->ip.i_mode)) { // if file
        iput(mip);
    }
    
    iput(old_cwd); // write old cwd back to disk

    running->cwd = mip;

    return 0;
}

// lists metadata of a single file
void ls_file(MINODE * mip, char *name) {

    int mode, links, uid, gid, size;
    char * time;
    static const char * permissions = "rwxrwxrwx";
    INODE * ip = &mip->ip;

    mode = ip->i_mode;
    links = ip->i_links_count;
    uid = ip->i_uid;
    gid = ip->i_gid;
    size = ip->i_size;

    // get timestamps
    time = ctime((time_t*)&ip->i_mtime);
    time[strlen(time) - 1] = 0;

    switch(mode & 0xF000) //filetypes (directory, link, etc)
    {
        case 0x8000:  
            putchar('-');     
            break; // 0x8 = 1000
        case 0x4000:  
            putchar('d');     
            break; // 0x4 = 0100
        case 0xA000:  
            putchar('l');     
            break; // oxA = 1010
        default:      
            putchar('?');     
            break;
    }

	//print permissions
    for(int i = 0; i < strlen(permissions); i++)
    {
        putchar(mode & (1 << (strlen(permissions) - 1 - i)) ? permissions[i] : '-');
    }
	//print rest of metadata
    printf("%d %d %d %d %26s %s", links, gid, uid, size, time, name);

	//if a link, print what link points to
    if(S_ISLNK(ip->i_mode))
    {
        printf(" -> %s", (char*)ip->i_block);
    }

    printf("\n");
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
    char buf[BLKSIZE], buf_index[BLKSIZE], buf_double[BLKSIZE], * current, name[256];

    int i;
    int pos;
    DIR *dp = (DIR*)buf;
    char *cp = buf;
    INODE * parent_ip = &mip->ip;
    int indirect_index, double_indirect_index;
    int * indirect, * double_indirect;

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

    //indirect blocks
	if(parent_ip->i_block[NUM_INDIRECT_BLOCKS])
	{
		get_block(dev, parent_ip->i_block[NUM_INDIRECT_BLOCKS], buf_index);
		indirect = (int *)buf_index;

		for(indirect_index = 0; indirect_index < BLOCK_NUMBERS_PER_BLOCK; indirect_index++)
		{
			if(indirect[i] == 0)
			{
				continue;
			}

			get_block(dev, indirect[i], buf);

	        current = buf;
	        dp = (DIR*)buf;

	        while(current < buf + block_size)
	        {
	        	//dont print '.' and '..' entries
	        	if(((strncmp(dp->name, ".", dp->name_len)) != 0) && 
	        		((strncmp(dp->name, "..", dp->name_len)) != 0))
	        	{
	        		strncpy(name, dp->name, dp->name_len);
	        		name[dp->name_len] = 0;

	        		dir_entry_mip = get_minode(dev, dp->inode);
	        		ls_file(dir_entry_mip, name);
	        		
	        		put_minode(dir_entry_mip);
	        	}        	   
	        	current += dp->rec_len;
	        	dp = (DIR*)current;
			}
		}
	}
	//double indirect blocks
	if(parent_ip->i_block[NUM_DOUBLE_INDIRECT_BLOCKS])
	{
		get_block(dev, parent_ip->i_block[NUM_DOUBLE_INDIRECT_BLOCKS], buf_double);
		double_indirect = (int *)buf_double;

		for(double_indirect_index = 0; double_indirect_index < BLOCK_NUMBERS_PER_BLOCK; double_indirect_index++)
		{
			if(double_indirect[double_indirect_index] == 0)
			{
				continue;
			}

			get_block(dev, double_indirect[double_indirect_index], buf_index);
			indirect = (int *)buf_index;

			for(indirect_index = 0; indirect_index < BLOCK_NUMBERS_PER_BLOCK; indirect_index++)
			{
				if(indirect[i] == 0)
				{
					continue;
				}
				get_block(dev, indirect[i], buf);

				current = buf;
		        dp = (DIR*)buf;

		        while(current < buf + block_size)
		        {
		        	//dont print '.' and '..' entries
		        	if(((strncmp(dp->name, ".", dp->name_len)) != 0) && 
		        		((strncmp(dp->name, "..", dp->name_len)) != 0))
		        	{
		        		strncpy(name, dp->name, dp->name_len);
		        		name[dp->name_len] = 0;

		        		dir_entry_mip = iget(dev, dp->inode);
		        		ls_file(dir_entry_mip, name); // ls file
		        		
		        		iput(dir_entry_mip);
		        	}        	   
		        	current += dp->rec_len;
		        	dp = (DIR*)current;
				}
			}
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