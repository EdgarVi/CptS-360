#include "read_cat.c"


int my_write(int dest_fd, char * buf, int num_bytes) {
    
	int dev, count = 0, remaining, logical_block, physical_block, double_block, start_byte;
	int ind_index, double_index, blk;
	int size;
	char *src, *dest;
	int *indirect, *double_indirect;
	char local_buf[BLKSIZE];
	OFT *ofp;
	char *cp, *buf_cp;
	MINODE *mip;
	INODE *ip;

	ofp = running->fd[fd];
	dev = ofp->minodePtr->dev;
	mip = ofp->minodePtr;
	ip = &ofp->minodePtr->ip;
	buf_cp = buf;

	while(num_bytes)
	{
		logical_block = ofp->offset / BLKSIZE;
		start_byte = ofp->offset % BLKSIZE;

		//direct
		if(logical_block < NUM_DIRECT_BLOCKS)
		{
			if(ip->i_block[logical_block] == 0)
			{
				mip->ip.i_block[logical_block] = ialloc(mip->dev);
			}
			physical_block = ip->i_block[logical_block];
		}
		//indirect
		else if(logical_block >= NUM_DIRECT_BLOCKS && 
			logical_block < NUM_DIRECT_BLOCKS + BLOCK_NUMBERS_PER_BLOCK)
		{

			if(ip->i_block[NUM_INDIRECT_BLOCKS] == 0)
			{
				mip->ip.i_block[logical_block] = ialloc(mip->dev);
			}
			get_block(dev, ip->i_block[NUM_INDIRECT_BLOCKS], local_buf);
			indirect = (int *)local_buf;

			if(indirect[logical_block - NUM_DIRECT_BLOCKS] == 0)
			{
				indirect[logical_block - NUM_DIRECT_BLOCKS] = ialloc(mip->dev);
				put_block(mip->dev, ip->i_block[NUM_INDIRECT_BLOCKS], local_buf);
			}

			physical_block = indirect[logical_block - NUM_DIRECT_BLOCKS];
		}
		//double indirect
		else
		{
			if(ip->i_block[NUM_DOUBLE_INDIRECT_BLOCKS] == 0)
			{
				ip->i_block[NUM_DOUBLE_INDIRECT_BLOCKS] = ialloc(mip->dev);
			}
			double_index = (logical_block - (NUM_DIRECT_BLOCKS + BLOCK_NUMBERS_PER_BLOCK)) / BLOCK_NUMBERS_PER_BLOCK;
			ind_index = (logical_block - (NUM_DIRECT_BLOCKS + BLOCK_NUMBERS_PER_BLOCK)) % BLOCK_NUMBERS_PER_BLOCK;

			get_block(dev, ip->i_block[NUM_DOUBLE_INDIRECT_BLOCKS], local_buf);
			double_indirect = (int *)local_buf;

			physical_block = double_indirect[double_index];

			if(physical_block == 0)
			{
				double_indirect[double_index] = ialloc(mip->dev);
				physical_block = double_indirect[double_index];
				put_block(mip->dev, ip->i_block[NUM_DOUBLE_INDIRECT_BLOCKS], local_buf);
			}

			get_block(dev, physical_block, local_buf);
			double_block = physical_block;

			indirect = (int *)local_buf;

			physical_block = indirect[ind_index];

			if(physical_block == 0)
			{
				indirect[ind_index] = ialloc(mip->dev);
				physical_block = indirect[ind_index];
				put_block(mip->dev, double_block, local_buf);
			}
		}

		get_block(dev, physical_block, local_buf);

		cp = local_buf + start_byte;
		remaining = BLKSIZE - start_byte;

		if(remaining <= num_bytes)
		{
			size = remaining;
		}
		else if(num_bytes <= remaining)
		{
			size = num_bytes;
		}

			
		memcpy(cp, buf, size);
		ofp->offset += size;
		if(ofp->offset > ofp->minodePtr->ip.i_size)
		{
			mip->ip.i_size += size;
		}
		count += size;
		num_bytes -= size;
		remaining -= size;
		put_block(dev, physical_block, local_buf);
	}
	mip->dirty = 1;
	return count;
}

// implementation of command "cp" to copy contents of one file to another
int my_cp(char * src_file, char * dest_file) {


    int src_fd, dest_fd, num_bytes, device = running->cwd->dev;
    int src_ino, dest_ino, parent_ino;

    MINODE * src_mip, * dest_mip, * parent_mip;
    char *buf[BLKSIZE];

    OFT * ofp;

    src_ino = getino(device, src_file);
    if(src_ino < 0) {
        printf("my_cp() error: %s does not exist\n", src_file);
        return -1; 
    }

    dest_ino = getino(device, dest_file);
    if(dest_ino < 0) { // if dest_file DNE, try creating it

        char * basepath = dest_file, * dirpath = dest_file;
        char * parent = dirname(dirpath), * child = basename(basepath);

        parent_ino = getino(device, parent);
        if(parent_ino < 0) { // path to file doesn't even exist
            printf("my_cp() error: %s does not exist\n", parent);
            return -1;
        }

        parent_mip = iget(device, parent_ino);
        dest_ino = my_creat(child); // finally create file

        iput(parent_mip);

    }

    src_mip = iget(device, src_ino);
	dest_mip = iget(dev, dest_ino);

    src_fd = my_open(src_file, "R");
	dest_fd = my_open(dest_file, "W");
    
    // files could not be opened
	if((src_fd == -1) || (dest_fd == -1)) {
		iput(src_mip);
		iput(dest_mip);
		return -1;
	}

	while((num_bytes = my_read(src_fd, buf, BLKSIZE)))
	{
		my_write(dest_fd, buf, num_bytes);
	}

	ofp = running->fd[src_fd];
	running->fd[src_fd] = 0;

	ofp->refCount--;
	if(ofp->refCount == 0)
	{
		ofp->offset = 0;
		iput(ofp->minodePtr);
	}

	ofp = running->fd[dest_fd];
	running->fd[dest_fd] = 0;

	ofp->refCount--;
	if(ofp->refCount == 0)
	{
		ofp->offset = 0;
		iput(ofp->minodePtr);
	}
	
	return 0;

}