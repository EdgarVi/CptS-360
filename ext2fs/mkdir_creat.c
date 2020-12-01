#include "cd_ls_pwd.c"


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

int k_creat(MINODE *pip, char *name){
    
    int i, ino = ialloc(dev);

    if(ino == 0){
        printf("ERROR: Unable to allocate inode\n");
        return -1;
    }

    dev = pip->dev;

    MINODE *mip = iget(dev, ino);

    mip->ip.i_mode = 0x81A4; // file type
    mip->ip.i_uid = running->uid;
    mip->ip.i_gid = running->gid;
    mip->ip.i_size = 0;
    mip->ip.i_links_count = 1; // link to '.'
    mip->ip.i_blocks = BLKSIZE / 512;
    mip->ip.i_atime = time(0L);
    mip->ip.i_mtime = time(0L);
    mip->ip.i_ctime = time(0L);

    for (i = 0; i < 12; i++){
        mip->ip.i_block[i] = 0;
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

    if ((mip->ip.i_mode & 0x4000) != 0x4000)
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