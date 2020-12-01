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


int my_unlink(char * file1, char * file2) {

    int device = running->cwd->dev, inode_number, parent_ino, i, j;
	char *childc, *parentc, *parent, *child;
	MINODE *mip, *parent_mip;

	for (int i = 0; i < 2; i++) {

        char * control_str;
        if(i == 0) {
            control_str = file1;
        } else if (i == 1){
            control_str = file2;
        }


        //- - error checking -- 
        inode_number = getino(device, control_str);	//get ino of basename
        if(inode_number < 0)
        {
            printf("Error: File does not exist\n");
            return -1;
        }

        mip = iget(device, inode_number);

        if(S_ISDIR(mip->ip.i_mode))
        {
            printf("Error: unlink: cannot unlink dir\n");
            iput(mip);
            return -1;
        }

        if(mip->refCount > 1)
        {
            printf("Error: File busy\n");
            iput(mip);
            return -1;
        }
        //- - error checking done -- 
        
        mip->ip.i_links_count--;
        mip->dirty = 1;

        if(mip->ip.i_links_count == 0 && !S_ISLNK(mip->ip.i_mode))
        {
            clear_blocks(mip);
        } else if(S_ISLNK(mip->ip.i_mode))
        {
            idalloc(device, inode_number); //if symbolic link, deallocate inode
        }

        //argv[i] + null character
        childc = strdup(control_str);
        parentc = strdup(control_str);

        child = basename(childc);
        parent = dirname(parentc);



        parent_ino = getino(device, parent);
        if(parent_ino < 0)
        {
            
            printf("Error: File does not exist\n");
            return -1;
        }

        parent_mip = iget(device, parent_ino);

        clear_blocks(mip);
        k_rmdir(parent_mip, child);
        
        iput(parent_mip);
        iput(mip);
        
    }
    
    return 0;
}