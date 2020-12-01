#include "link_unlink.c"

int my_symlink(char * file1, char * file2) {
    int old_ino, new_ino, new_parent_ino, device = running->cwd->dev;
	MINODE *old_mip, *new_mip, *new_parent_mip;
	char *path, *child, *parent, buf[BLKSIZE];


	old_ino = getino(device, file1);
	if(old_ino < 0)
	{
		printf("Error: File does not exist\n");
		return -1;
	}

	old_mip = iget(device, old_ino);
	
	if(S_ISDIR(old_mip->ip.i_mode))
	{
		iput(old_mip);
		printf("Error: File cannot be a directory\n");
		return -1;
	}

	iput(old_mip);

	path = strdup(file2);	//argv[2] + null character
	

	child = basename(path);
	parent = dirname(path);

	new_parent_ino = getino(device, parent);
	if(new_parent_ino < 0)
	{
		printf("Error: Directory does not exist\n");
		return -1;
	}

	new_parent_mip = iget(device, new_parent_ino);

	if(!S_ISDIR(new_parent_mip->ip.i_mode))
	{
		iput(new_parent_mip);
		printf("Error: File not a directory\n");
		return -1;
	}

	if(getino(device, file2) > 0)
	{
		iput(new_parent_mip);
		printf("Error: File already exists\n");
		return -1;
	}

	//- - error checking done - -
	new_ino = k_creat(new_parent_mip, child);	//create symbolic link file. return inode number of file

	new_parent_mip->ip.i_atime = time(0L);
	new_parent_mip->dirty = 1;


	

	new_mip = iget(device, new_ino);	//get minode of new_ino

	strcpy((char*)new_mip->ip.i_block, file1);	//copy filename to i_block

	new_mip->ip.i_mode = 0120777;
	new_mip->ip.i_size = strlen(file1);
	new_mip->dirty = 1;
	
	iput(new_mip);
	iput(new_parent_mip);


	return 0;
}