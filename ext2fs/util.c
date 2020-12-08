#include "global.c"

// read disk block into buffer
int get_block(int fd, int blk, char *buf){
    if(lseek(fd, (long)(blk * BLKSIZE), 0) == -1){
        printf("Error in get_block\n");
        return -1;    
    } 
    return read(fd, buf, BLKSIZE); 
}

// write buffer data to disk
int put_block(int dev, int blk, char buf[])
{

    lseek(dev, blk * BLKSIZE, SEEK_SET);
    int n = write(dev, buf, BLKSIZE);
    
    if(n != BLKSIZE)
        printf("put_block [%d %d] error\n", dev, blk);
}

int test_bit(char *buf, int bit){
    
    // Mailman's algorithm, check if bit set to 1 or 0
    int i, j;
    i = bit / 8;
    j = bit % 8;
    if(buf[i] & (1 << j))
        return 1;
    return 0;
}

int set_bit(char *buf, int bit){
    int i, j;
    i = bit / 8;
    j = bit % 8;
    buf[i] |= (1 << j);
}

int clr_bit(char *buf, int bit){
    int i, j;
    i = bit / 8;
    j = bit % 8;
    buf[i] &= ~(i << j);
}


int decFreeInodes(int dev){
    char buf[BLKSIZE];

    // dec free inodes count by 1 in SUPER and GD
    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_inodes_count--;
    put_block(dev, 1, buf);

    get_block(dev, 2, buf);
    gp = (GD *)buf;
    gp->bg_free_inodes_count--;
    put_block(dev, 2, buf);
}


// decrement or increment the free blocks count in both SUPER and GD block
int dec_inc_free_blocks(int dev, char action){
    char buf[BLKSIZE];
    SUPER *sup;
    GD *gd;

    get_block(dev, 1, buf); // get super block
    sup = (SUPER *)buf;

    if(action == '+'){
        
        // put super block
        sup->s_free_blocks_count++;
        put_block(dev, 1, buf);

        // get and put gd block
        get_block(dev, 2, buf);
        gd = (GD*) buf;
        gd->bg_free_blocks_count++;
        put_block(dev, 2, buf);

    } else if(action == '-'){
        // put super block
        sup->s_free_blocks_count--;
        put_block(dev, 1, buf);

        // get and put gd block
        get_block(dev, 2, buf);
        gd = (GD*) buf;
        gd->bg_free_blocks_count--;
        put_block(dev, 2, buf);

    }

}

// allocate inode
int ialloc(int dev){
    int i;
    char buf[BLKSIZE];

    // read inode_bitmap block
    for(i = 0; i < ninodes; i++){
        if(test_bit(buf, i) == 0){
            set_bit(buf, i);
            put_block(dev, imap, buf);
            decFreeInodes(dev);
            return i + 1;
        }
    }

    return 0;
}

void clear_block(int dev, int block)
{
	char buf[BLKSIZE];
	get_block(dev, block, buf);
	memset(buf, 0, BLKSIZE);
	put_block(dev, block, buf);
}

// allocate new data block
// loads block bitmap, tests each bit in order and
// returns index of first unallocated block
int balloc(dev){

    int i;
    char buf[BLKSIZE];

    get_block(dev, bmap, buf); // read bmap block

    for(i = 0; i < nblocks; i++){
        
        if(test_bit(buf, i) == 0){
            
            set_bit(buf, i);
            dec_inc_free_blocks(dev, '-'); // decrement free blocks
            put_block(dev, bmap, buf);
            clear_block(dev, i + 1);
            return i + 1; // return index of this block
        }
    }

    printf("ERROR: No free blocks left\n");
    return -1;
}


int dir_or_file(MINODE *mip){  
  int type = -1;
  
  if ((mip->ip.i_mode & 0xF000) == 0x8000) // if (S_ISREG())
  {
    type = 0;
  }
  if ((mip->ip.i_mode & 0xF000) == 0x4000) // if (S_ISDIR())
  {
    type = 1;
  }
  if ((mip->ip.i_mode & 0xF000) == 0xA000) // if (S_ISLNK())
  {
    type = 2;
  }

  return type;
}


// allocate a FREE minode for use
MINODE *mialloc()
{
    int i;
    for(i = 0; i < NMINODES; i++){
        MINODE *mp = &minode[i];
        if(mp->refCount == 0){
            mp->refCount = 1;
            return mp;
        }
    }

    printf("FS panic: out of minodes\n");
    return 0;
}


// release a used minode
int midalloc(MINODE *mp)
{
    mp->refCount = 0;
}

int tokenize(char *pathname){
    char *token;
    strcpy(gpath, pathname);

    n = 0; // set path count to 0
    token = strtok(gpath, "/");
    while(token){
        name[n] = token;
        token = strtok(0, "/");
        n++;
    }
}

int findPath(char *pathname){
    int i;
    for (i = 0; i < strlen(pathname); i++){
        if(pathname[i] == '/'){
            return 1;
        }
    }
    return 0;
}


// implement EXT2 FS traversal
int search(MINODE *mip, char *name){
    
    int i, inode_num, blk, offset;
    char sbuf[BLKSIZE], temp[256], dir_name[256];
    char *char_p;

    strcpy(dir_name, name);

    for (i=0; i < 12; i++){         // assume DIR at most 12 direct blocks

      if(mip->ip.i_block[i] != 0){   //never found the dir name

        // YOU SHOULD print i_block[i] number here
        get_block(mip->dev, mip->ip.i_block[i], sbuf);

        dp = (DIR *)sbuf;
        char_p = sbuf;

        while(char_p < sbuf + BLKSIZE){
            strncpy(temp, dp->name, dp->name_len);  //make name a string
            temp[dp->name_len] = 0;                 //ensure null at end
            
            if(strcmp(temp,name) == 0){  //found dir, return inode number
                
                return dp->inode;
            }

            char_p += dp->rec_len;    //advance char_p by rec_len
            dp = (DIR *)char_p;       //pull dir_p to next entry
        }
      }
  }

  return 0; // nothing found
}

/*
returns a pointer to the in-memory minode containing the 
INODE of (dev, ino). The returned minode is unique
*/
MINODE *iget(int dev, int ino)
{
    int i;
    MINODE *mip;
    char buf[BLKSIZE];
    int blk, offset;
    INODE *ip, *table;

    for (i=0; i < NMINODES; i++){
        mip = &minode[i];
        
        if (mip->dev == dev && mip->ino == ino){
            mip->refCount++;
            return mip;
        }
    }

    for (i = 0; i < NMINODES; i++){
        mip = &minode[i];
        if (mip->refCount == 0){

            mip->refCount = 1;
            mip->dev = dev;
            mip->ino = ino;
            mip->dirty = 0;

            // get INODE of ino to buf    
            blk  = (ino - 1) / 8 + iblock; // iblock is the inode table block
            offset = (ino - 1) % 8;

            
            get_block(dev, blk, buf);
            ip = (INODE *)buf + offset;
            
            // copy INODE to mp->INODE
            mip->ip = *ip;

            return mip;
        }
    }   
    printf("PANIC: no more free minodes\n");
    return 0;
}

int iput(MINODE *mip){
    INODE *ip;
    int i, block, offset;
    char buf[BLKSIZE];

    if(mip == 0)
        return;
    
    mip->refCount--; 
    
    // still has user
    if(mip->refCount > 0)
        return;

    // no need to write back
    if(mip->dirty == 0)
        return;
    
    // write INODE back to disk
    block = (mip->ino -1) / 8 + iblock;
    offset = (mip->ino - 1) % 8;

    // get block containing this inode
    get_block(mip->dev, block, buf);
    ip = (INODE *)buf + offset; // ip points at INODE
    *ip = mip->ip; // copy INODE to inode in block
    put_block(mip->dev, block, buf); // write back to disk
    midalloc(mip); 
}


int switch_dev(int disk)
{
    fd = mtable[disk].dev;
    dev = mtable[disk].dev;
    nblocks = mtable[disk].nblocks;
    ninodes = mtable[disk].ninodes;
    bmap = mtable[disk].bmap;
    imap = mtable[disk].imap;
    inode_start = mtable[disk].iblock;
}

// path to node algorithm
int getino(int dev, char *pathname){
    
   int i, ino, block, disp;
   INODE *ip;
   MINODE *mip;

   if(strcmp(pathname, "/") == 0){
       return 2; // root
   } 

   if(pathname[0] == '/'){
       mip = iget(dev, 2);
   } else {
       mip = iget(running->cwd->dev, running->cwd->ino);
   }


    tokenize(pathname);

   for(i = 0; i < n; i++){

       printf("Getting i_node number of file named: %s\n", name[i]);
       ino = search(mip, name[i]);

       if(ino == 0){
           iput(mip);
           printf("ERROR: name does not exist\n");
           return 0;
       }

       iput(mip);
       mip = iget(dev, ino);
   }

    iput(mip);
    return ino;
    
}


int incFreeInodes(int dev){
    char buf[BLKSIZE];

    // inc free inodes count in SUPER and GD
    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_inodes_count++;
    put_block(dev, 1, buf);
    get_block(dev, 2, buf);
    gp = (GD*)buf;
    gp->bg_free_inodes_count++;
    put_block(dev, 2, buf);
}

unsigned long idalloc(int dev, int ino)
{
  int i;  
  char buf[BLKSIZE];

  if (ino > ninodes){
    printf("inumber %d out of range\n", ino);
    return;
  }

  // get inode bitmap block
  get_block(dev, bmap, buf);
  clr_bit(buf, ino-1);

  // write buf back
  put_block(dev, imap, buf);

  // update free inode count in SUPER and GD
  incFreeInodes(dev);
}


int truncate(MINODE *mip)
{
    deallocInodeBlk(mip);
    mip->ip.i_atime = time(0L);
    mip->ip.i_mtime = time(0L);
    mip->ip.i_size = 0;
    mip->dirty = 1;
}

int deallocInodeBlk(MINODE *mip)
{
    char bitmap[BLKSIZE], buf[BLKSIZE], dbuf[BLKSIZE];
    int i, j, iblk, dblk;
    int indirect, doubleIndirect;

    get_block(dev, BBITMAP, bitmap);

    for (i = 0; i < 12; i++)
    {
        if (mip->ip.i_block[i] != 0)
        {
            clr_bit(bitmap, mip->ip.i_block[i]);
            mip->ip.i_block[i] = 0;
        }

        else
        {
            put_block(dev, BBITMAP, bitmap);
            return;
        }
    }

    if (mip->ip.i_block[i] != 0)
    {
        iblk = mip->ip.i_block[i];
        get_block(dev, iblk, buf);
        indirect = (int *) buf;
        
        for (i = 0; i < 256; i++)
        {
            if (indirect != 0)
            {
                clr_bit(bitmap, indirect - 1);
                indirect = 0;
                indirect++;
            }

            else
            {
                clr_bit(bitmap, iblk - 1);
                put_block(dev, iblk, buf);
                put_block(dev, BBITMAP, bitmap);
                mip->ip.i_block[12] = 0;
                return;
            }
        }
    } else {
        put_block(dev, BBITMAP, bitmap);
        return;
    }

    if (mip->ip.i_block[13] != 0) {
        dblk = mip->ip.i_block[13];
        get_block(dev, dblk, dbuf);

        doubleIndirect = (int *) dbuf;

        for (i = 0; i < 256; i++) {
            iblk = doubleIndirect;
            get_block(dev, iblk, buf);
            indirect = (int *) buf;

            for (j = 0; j < 256; j++) {
                if (indirect != 0) {
                    clr_bit(bitmap, indirect - 1);
                    indirect = 0;
                    indirect++;
                } else {
                    clr_bit(bitmap, iblk - 1);
                    clr_bit(bitmap, dblk - 1);
                    put_block(dev, iblk, buf);
                    put_block(dev, BBITMAP, bitmap);
                    put_block(dev, dblk, dbuf);
                    mip->ip.i_block[13] = 0;
                    return;
                }

                clr_bit(bitmap, iblk - 1);
            }

            doubleIndirect++;

            if (doubleIndirect == 0)
            {
                clr_bit(bitmap, iblk - 1);
                clr_bit(bitmap, dbuf - 1);
                put_block(dev, iblk, buf);
                put_block(dev, BBITMAP, bitmap);
                put_block(dev, dblk, dbuf);
                mip->ip.i_block[13] = 0;
                return;
            }
        }
    }

    else
    {
        put_block(dev, BBITMAP, bitmap);
        return;
    }
}

// deallocate an existing block
int bdalloc(int device, int bno){
    int i;
    char buf[BLKSIZE];

    if(bno <= 0) {
        printf("Error: block number invalid\n");
        return -1;
    }

    get_block(device, bmap, buf);  //get INODE bitmap block
    clr_bit(buf, bno - 1);         //clear bit

    dec_inc_free_blocks(device, '+');  //increment free BLOCKS
    put_block(device, bmap, buf);     //write INODE back
}

int child_count(MINODE *pmip){
    int count = 0,i;
    char sbuf[BLKSIZE], *cp;
    INODE *ip = &pmip->ip;

     for(i = 0; i < 12;i++){
        
        if(ip->i_block[i] == 0){
            break;
        }

        //get pmip's data block into sbuf
        get_block(pmip->dev,ip->i_block[i],sbuf);
       
        dp = (DIR *)sbuf; 
        cp = sbuf;

        while(cp < sbuf + BLKSIZE){                  
            count++;
            cp += dp->rec_len;  //advance char_p by rec_len
            dp = (DIR *)cp;     //pull dir_p to next entry
        }
    }
    
    return count;
}

// prints opened file descriptors
int my_pfd() {
    int i;
    printf("Filename\tFD\tmode\toffset\n");
    for(i = 0; i < 10; i++){
        if (running->fd[i]!= NULL)
		{
			rpwd(running->fd[i]->minodePtr);
			printf("\t\t%d\t",i);
			switch(running->fd[i]->mode)
			{
				case 0:
					printf("READ\t");
					break;
				case 1:
					printf("WRITE\t");
					break;
				case 2:
					printf("R/W\t");
					break;
				case 3:
					printf("APPEND\t");
					break;
				default:
					printf("??????\t");//this should never happen
					break;
			}
			printf("%li\n",running->fd[i]->offset);
		}
    }
    return 1;
}


// clear blocks
int clear_blocks(MINODE *mip)
{	
	int device = mip->dev;
	int *indirect_block, *double_indirect_block;
	char indirect_buf[BLKSIZE], double_indirect_buf[BLKSIZE];
	int indirect_sentinel, double_indirect_sentinel, total = 0;


	//direct blocks
	for(indirect_sentinel = 0; indirect_sentinel < NUM_DIRECT_BLOCKS; indirect_sentinel++)
	{
		if(mip->ip.i_block[indirect_sentinel] == 0)
		{
			continue;
		}
		bdalloc(device, mip->ip.i_block[indirect_sentinel]);
		mip->ip.i_block[indirect_sentinel] = 0;
	}

	//indirect blocks
	if(mip->ip.i_block[NUM_INDIRECT_BLOCKS] != 0)
	{
		get_block(device, mip->ip.i_block[NUM_INDIRECT_BLOCKS], indirect_buf);
		indirect_block = (int *)indirect_buf;
	
		for(indirect_sentinel = 0; indirect_sentinel < BLOCK_NUMBERS_PER_BLOCK; indirect_sentinel++)
		{
			if(indirect_block[indirect_sentinel] == 0)
			{
				continue;
			}
			bdalloc(device, indirect_block[indirect_sentinel]);
		}
		bdalloc(device, mip->ip.i_block[NUM_INDIRECT_BLOCKS]);
 		mip->ip.i_block[NUM_INDIRECT_BLOCKS] = 0;
	}

	memset(indirect_buf, 0, BLKSIZE);
	memset(double_indirect_buf, 0, BLKSIZE);
	
	//doubly indirect blocks 
	if(mip->ip.i_block[NUM_DOUBLE_INDIRECT_BLOCKS] != 0)
	{
		get_block(device, mip->ip.i_block[NUM_DOUBLE_INDIRECT_BLOCKS], double_indirect_buf);
		double_indirect_block = (int *)double_indirect_buf;

		for(indirect_sentinel = 0; indirect_sentinel < BLOCK_NUMBERS_PER_BLOCK; indirect_sentinel++)
		{
			if(double_indirect_block[indirect_sentinel] == 0)
			{
				continue;
			}
			
			get_block(device, double_indirect_block[indirect_sentinel], indirect_buf);
			indirect_block = (int *)indirect_buf;

			for(double_indirect_sentinel = 0; double_indirect_sentinel < BLOCK_NUMBERS_PER_BLOCK; double_indirect_sentinel++)
			{
				if(indirect_block[double_indirect_sentinel] == 0)
				{
					continue;
				}
				bdalloc(device, indirect_block[double_indirect_sentinel]);
			}

			memset(indirect_buf, 0, BLKSIZE);

		}
		bdalloc(device, mip->ip.i_block[NUM_DOUBLE_INDIRECT_BLOCKS]);
		mip->ip.i_block[NUM_DOUBLE_INDIRECT_BLOCKS] = 0;
	}

	mip->dirty = 1;
	mip->ip.i_size = 0;
	mip->ip.i_ctime = mip->ip.i_mtime = time(0L);  
	mip->ip.i_blocks = 0;

	return 0;
}


int assign_first_empty_bno(MINODE *mip, int bno)
{

	int i, ind_index, double_index;
	int *indirect, *double_indirect;
	INODE *parent_ip = &mip->ip;
	int block_number;
	int device = mip->dev;
	char *current_ptr, buf[BLKSIZE], buf_ind[BLKSIZE], buf_double[BLKSIZE];

	//direct blocks
	for(i = 0; i < NUM_DIRECT_BLOCKS; i++)
	{
		if(parent_ip->i_block[i] == 0)
		{
			parent_ip->i_block[i] = bno;
			return 0;
		}
	}

	//indirect blocks
	if(parent_ip->i_block[NUM_INDIRECT_BLOCKS] == 0)
	{
		parent_ip->i_block[NUM_INDIRECT_BLOCKS] = balloc(mip->dev);
		mip->dirty = 1;
	}
	get_block(device, parent_ip->i_block[NUM_INDIRECT_BLOCKS], buf_ind);
	indirect = (int *)buf_ind;

	for(ind_index = 0; ind_index < BLOCK_NUMBERS_PER_BLOCK; ind_index++)
	{
		if(indirect[i] == 0)
		{
			indirect[i] = bno;
			put_block(device, parent_ip->i_block[NUM_INDIRECT_BLOCKS], buf_ind);
			return 0;
		}
	}
	
	//double indirect blocks
	if(parent_ip->i_block[NUM_DOUBLE_INDIRECT_BLOCKS] == 0)
	{
		parent_ip->i_block[NUM_DOUBLE_INDIRECT_BLOCKS] = balloc(mip->dev);
		mip->dirty = 1;
	}

	get_block(device, parent_ip->i_block[NUM_DOUBLE_INDIRECT_BLOCKS], buf_double);
	double_indirect = (int *)buf_double;

	for(double_index = 0; double_index < BLOCK_NUMBERS_PER_BLOCK; double_index++)
	{
		if(double_indirect[double_index] == 0)
		{
			double_indirect[double_index] = balloc(mip->dev);
			put_block(device, parent_ip->i_block[NUM_DOUBLE_INDIRECT_BLOCKS], buf_double);
			return 0;
		}

		get_block(device, double_indirect[double_index], buf_ind);
		indirect = (int *)buf_ind;

		for(ind_index = 0; ind_index < BLOCK_NUMBERS_PER_BLOCK; ind_index++)
		{
			if(indirect[i] == 0)
			{
				indirect[i] = bno;
				put_block(device, double_indirect[double_index], buf_ind);
				return 0;
			}
		}
	}

	printf("ERROR: No blocks left\n");
	return -1;
}

// enters a [ino, name] tuple as a new dir entry into a parent dir
int enter_name(MINODE *parent_mip, int ino, char *name){
    


    int i, ind_index, double_index, block_number, device = parent_mip->dev;
	int need_length, remaining_length, ideal_last_entry;
	int *indirect, *double_indirect;
	char buf[BLKSIZE], buf_ind[BLKSIZE], buf_double[BLKSIZE], *current;
	DIR *dp;
	INODE *parent_ip = &parent_mip->ip;

	//direct blocks
	for(i = 0; i < NUM_DIRECT_BLOCKS; i++)
	{
		if(parent_ip->i_block[i] == 0)
		{
			continue;
		}

		block_number = parent_ip->i_block[i];

		get_block(device, block_number, buf);

		current = buf;
		dp = (DIR*)buf;

		while(current + dp->rec_len < buf + block_size)
		{
			current += dp->rec_len;
			dp = (DIR *)current;
		}

		need_length = (4 * (( 8 + strlen(name) + 3) / 4));
		ideal_last_entry = (4 * (( 8 + dp->name_len + 3) / 4));
		remaining_length = dp->rec_len - ideal_last_entry;

		if(remaining_length >= need_length)
		{
			dp->rec_len = ideal_last_entry;

			current += dp->rec_len;
			dp = (DIR*)current;

			dp->inode = ino;
			dp->rec_len = (block_size - (current - buf));
			dp->name_len = strlen(name);
			dp->file_type = EXT2_FT_DIR;
			strcpy(dp->name, name);

			put_block(device, block_number, buf);
			
			return 0;
		}
	}


    block_number = balloc(device);

	assign_first_empty_bno(parent_mip, block_number);
	parent_ip->i_size += block_size;
	parent_ip->i_blocks += block_size / 512;
	parent_mip->dirty = 1;

	get_block(device, block_number, buf);

	current = buf;
	dp = (DIR *)buf;

	dp->inode = ino;
	dp->rec_len = block_size;
	dp->name_len = strlen(name);
	dp->file_type = EXT2_FT_DIR;
	strcpy(dp->name, name);

	put_block(device, block_number, buf);

	return 0;
}

// Shows if duplicate oft exists
int duplicate_oft(MINODE *mip)
{
	int i;
	OFT *oftp;

	for(int i = 0; i < NOFT; i++)
	{
		oftp = &oft[i];


		if(oftp->refCount > 0)
		{
			if(oftp->minodePtr->dev == mip->dev && 
				oftp->minodePtr->ino == mip->ino)
			{
				if(oftp->mode != READ_TYPE)
				{
					return 1;
				}
			}
		}
	}
	return 0;
}

// get open file table
OFT * get_oft() {
    int i;
    OFT *oftp;

    for(i = 0; i < NOFT; i++){
        oftp = &oft[i];
        if(oftp->refCount == 0){
            oftp->refCount++;
            return oftp;
        }
    }

    printf("Error: No open file tables left\n");
    return NULL;
}


// write all modified minodes to disk
int quit(char *rootdev){
    int i;

    MINODE *mip;
    for(i = 0; i < NMINODES; i++){
        
        mip = &minode[i];
        if(mip->refCount > 0){
            iput(mip);
        }
            
    }

    printf("Device: %s unmounted\n", rootdev);
    exit(0);
}