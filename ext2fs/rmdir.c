#include "mkdir_creat.c"

int k_rmdir(MINODE *parent_mip, char * name) {
    int device = parent_mip->dev, i;
	char buf[BLKSIZE], *current_ptr, *last_ptr, *start, *end;
	int *indirect, *double_indirect;
	char buf_ind[BLKSIZE],  buf_double[BLKSIZE];
	int ind_index, double_index;

	DIR *dp, *prev_dp, *last_dp;
	
	// direct blocks;
	for(i = 0; i < NUM_DIRECT_BLOCKS; i++)
	{
		if(parent_mip->ip.i_block[i] == 0)
		{
			continue;
		}

		get_block(device, parent_mip->ip.i_block[i], buf);

		current_ptr = buf;
		dp = (DIR *)buf;

		while(current_ptr < buf + block_size)
		{
			//child found
			if(strncmp(dp->name, name, dp->name_len) == 0)
			{
				goto found;
			}

			prev_dp = dp;
			current_ptr += dp->rec_len;
			dp = (DIR *)current_ptr;
		}
	}

	//indirect blocks
	if(parent_mip->ip.i_block[NUM_INDIRECT_BLOCKS] != 0)
	{
		get_block(device, parent_mip->ip.i_block[NUM_INDIRECT_BLOCKS], buf_ind);
		indirect = (int *)buf_ind;

		for(ind_index = 0; ind_index < BLOCK_NUMBERS_PER_BLOCK; ind_index++)
		{
			if(indirect[ind_index] == 0)
			{
				continue;
			}
			
			get_block(device, indirect[ind_index], buf);
			current_ptr = buf;
			dp = (DIR *)buf;

			while(current_ptr < buf + block_size)
			{
				//child found
				if(strncmp(dp->name, name, dp->name_len) == 0)
				{
					goto found;
				}

				prev_dp = dp;
				current_ptr += dp->rec_len;
				dp = (DIR *)current_ptr;
			}
		}
	}

	//double indirect blocks
	if(parent_mip->ip.i_block[NUM_DOUBLE_INDIRECT_BLOCKS] != 0)
	{
		get_block(device, parent_mip->ip.i_block[NUM_DOUBLE_INDIRECT_BLOCKS], buf_double);
		double_indirect = (int *)buf_double;

		for(double_index = 0; double_index < BLOCK_NUMBERS_PER_BLOCK; double_index++)
		{
			get_block(device, double_indirect[double_index], buf_ind);
			indirect = (int *)buf_ind;
			for(ind_index = 0; ind_index < BLOCK_NUMBERS_PER_BLOCK; ind_index++)
			{
				if(indirect[ind_index] == 0)
				{
					continue;
				}
				
				get_block(device, indirect[ind_index], buf);
				current_ptr = buf;
				dp = (DIR *)buf;

				while(current_ptr < buf + block_size)
				{
					//child found
					if(strncmp(dp->name, name, dp->name_len) == 0)
					{
						goto found;
					}

					prev_dp = dp;
					current_ptr += dp->rec_len;
					dp = (DIR *)current_ptr;
				}
			}
		}
	}


	printf("Error: File does not exist\n");
	return -1;

	found:

	if(current_ptr == buf &&
		current_ptr + dp->rec_len == buf + block_size)
	{
		bdalloc(device, parent_mip->ip.i_block[i]);

		parent_mip->ip.i_size -= block_size;

		while((i + 1) < NUM_DIRECT_BLOCKS)
		{
			if(parent_mip->ip.i_block[i + 1] == 0)
			{
				continue;
			}
			i++;
			get_block(device, parent_mip->ip.i_block[i], buf);
			put_block(device, parent_mip->ip.i_block[i - 1], buf);
		}
	}
	//last entry in block
	else if(current_ptr + dp->rec_len == buf + block_size)
	{
		prev_dp->rec_len += dp->rec_len;
		put_block(device, parent_mip->ip.i_block[i], buf);
	}
	//some middle entry
	else
	{
		last_ptr = current_ptr;
		while(last_ptr + dp->rec_len < buf + block_size)
		{
			last_ptr += dp->rec_len;
			dp = (DIR *)last_ptr;
		}
		dp = (DIR *)current_ptr;
		last_dp = (DIR *)last_ptr;

		last_dp->rec_len += dp->rec_len;

		start = current_ptr + dp->rec_len;
		end = buf + block_size;
		memmove(current_ptr, start, end - start);

		put_block(device, parent_mip->ip.i_block[i], buf);
	}

	parent_mip->dirty = 1;

	return 0;
}

int rmdir(char *pathname) {

    int i = 1, j, ino, device = running->cwd->dev, parent_ino;
    char buf[BLKSIZE], * current, * parent, * child;
    MINODE * mip, * parent_mip;

    parent = basename(pathname);
    child = dirname(pathname);

    ino = getino(device, parent);

    if(ino < 0) {
        printf("Error: File does not exist\n");
        return -1;
    }

    mip = iget(device, ino);

    // error checking
    if(running->uid != SUPER_USER && running->uid != mip->ip.i_uid) {
        printf("Error: Permissions not enabled\n");
        iput(mip);
        return -1;
    }

    if(!S_ISDIR(mip->ip.i_mode)) {
        printf("ERROR: %s is not a dir\n", pathname);
        return -1;
    }

    if(mip->refCount > 1) {
        printf("Error: %s is in use\n", pathname);
        return -1;
    }

    if(mip->ip.i_links_count > 2) {
        printf("Error: %s is not empty\n", pathname);
    }

    // get parent inode (the second entry in i_block[0])
    get_block(mip->dev, mip->ip.i_block[0], buf);

    // guaranteed that the second dir entry is the parent entry (..)
    current = buf;
    dp = (DIR*)buf;

    ino = dp->inode;	//first  entry inode number (.)

    current += dp->rec_len;	//go to next entry (..)
    dp = (DIR*)current;

    parent_ino = dp->inode;		//get inode number of parent (..)
    parent_mip = iget(device, parent_ino);

    clear_blocks(mip);
    bdalloc(mip->dev, mip->ino);
    iput(mip);


    k_rmdir(parent_mip, child);	//remove directory's dir entry from its parent directory

    parent_mip->ip.i_links_count--;
    parent_mip->ip.i_atime = time(0L);
    parent_mip->ip.i_mtime = time(0L);
    parent_mip->dirty = 1;

    iput(parent_mip);


}