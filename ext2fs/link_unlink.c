#include "rmdir.c"

int my_link(char * file1, char * file2) {
    
    int old_inode, parent_inode, i, device = running->cwd->dev;
	MINODE *old_mip, *parent_mip;

	char *path, *child, *parent;


	old_inode = getino(device, file1);	//get inode number of <oldfile>
	
	// error checking
	if(old_inode < 0)
	{
		printf("Error: %s does not exist", file1);
		return -1;
	}	

	old_mip = iget(device, old_inode);

	if(S_ISDIR(old_mip->ip.i_mode))
	{
		iput(old_mip);
		printf("Error: Can't link to dir\n");
		return -1;
	}

	path = strdup(file2);	//get null terminated <newfile> name
	

	child = basename(path);
	parent = dirname(path);

	parent_inode = getino(device, parent);
	if(parent_inode < 0)
	{
		iput(old_mip);
	    printf("Error: File does not exist\n");
		return -1;
	}

	parent_mip = iget(device, parent_inode);	

	if(!S_ISDIR(parent_mip->ip.i_mode)) //make sure parent is actually a directory
	{
		iput(parent_mip);
		iput(old_mip);
		printf("Error: Parent not a directory\n");
		return -1;
	}

	if(getino(device, file2) > 0)
	{
		iput(parent_mip);
		iput(old_mip);
		printf("Error: %s already exists", file2);
		return -1;
	}

	enter_name(parent_mip, old_inode, child);	// enter a new entry (the link metadata) of the parent mip

	old_mip->ip.i_links_count++;
	old_mip->dirty = 1;

	//write back to disk
	iput(old_mip);
	iput(parent_mip);


	return 0;
    
}