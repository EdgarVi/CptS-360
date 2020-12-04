#include "open_close_lseek.c"


// Reads nbytes from a file descriptor into a buffer and
// returns the number of bytes read
int my_read(int fd, char * buf, int nbytes) {
    
    int dev, count = 0, remaining, available, logical_block, physical_block, start_byte;
	int ind_index, double_index, size;
	char *src, *dest;
	int *indirect, *double_indirect;
	char local_buf[BLKSIZE];
	OFT *ofp;
	char *cp, *buf_cp;
	INODE *ip;

	ofp = running->fd[fd];
	dev = ofp->minodePtr->dev;
	ip = &ofp->minodePtr->ip;
	available = ip->i_size - ofp->offset;
	buf_cp = buf;

	while(nbytes && available)
	{
		logical_block = ofp->offset / BLKSIZE;
		start_byte = ofp->offset % BLKSIZE;


		//direct
		if(logical_block < NUM_DIRECT_BLOCKS)
		{
			physical_block = ip->i_block[logical_block];
		}
		//indirect
		else if(logical_block >= NUM_DIRECT_BLOCKS && 
			logical_block < NUM_DIRECT_BLOCKS + BLOCK_NUMBERS_PER_BLOCK)
		{
			ind_index = logical_block - NUM_DIRECT_BLOCKS;
			get_block(dev, ip->i_block[NUM_INDIRECT_BLOCKS], local_buf);
			indirect = (int *)local_buf;

			physical_block = indirect[ind_index];
		}
		//double indirect
		else
		{
			double_index = (logical_block - (NUM_DIRECT_BLOCKS + BLOCK_NUMBERS_PER_BLOCK)) / BLOCK_NUMBERS_PER_BLOCK;
			ind_index = (logical_block - (NUM_DIRECT_BLOCKS + BLOCK_NUMBERS_PER_BLOCK)) % BLOCK_NUMBERS_PER_BLOCK;

			get_block(dev, ip->i_block[NUM_DOUBLE_INDIRECT_BLOCKS], local_buf);
			double_indirect = (int *)local_buf;

			get_block(dev, double_indirect[double_index], local_buf);
			indirect = (int *)local_buf;

			physical_block = indirect[ind_index];
		}

		get_block(dev, physical_block, local_buf);

		cp = local_buf + start_byte;
		remaining = BLKSIZE - start_byte;

		if(available <= remaining && available <= nbytes)
		{
			size = available;
		}
		else if(remaining <= available && remaining <= nbytes)
		{
			size = remaining;
		}
		else if(nbytes <= remaining && nbytes <= available)
		{
			size = nbytes;
		}

		memcpy(buf_cp, cp, size);
		ofp->offset += size;
		count += size;
		available -= size;
		nbytes -= size;
		remaining -= size;

	}

	return count;
}


// Display contents of a file opened for R or RW
int my_cat(char * filename, char * mode_input) {
	
    char buf[BLKSIZE];
	char print_buf[BLKSIZE + 1];
	int n, ino, device = running->cwd->dev;
	OFT *ofp;
	MINODE *mip;
    
	ino = getino(device, filename);
	
	if(ino < 0)
	{
		printf("Error: File does not exist\n");
		return -1;
	}
	

	mip = iget(device, ino);
	fd = my_open(pathname, mode_input); 
	
	while((n = my_read(fd, buf, BLKSIZE)))
	{
	
		memcpy(print_buf, buf, BLKSIZE);
		print_buf[n] = 0;

		printf("%s", print_buf);
	}

	ofp = running->fd[fd];
	running->fd[fd] = 0;

	ofp->refCount--;

	if(ofp->refCount == 0)
	{
		iput(ofp->minodePtr);
	}

	return 0;
}