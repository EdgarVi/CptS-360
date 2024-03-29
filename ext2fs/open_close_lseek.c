#include "symlink.c"


int my_close(char * fd_input) {

    OFT * ofp;

    // convert fd from char to int
    if(strcmp(fd_input, "0") == 0)
	{
		fd = 0;

	} else {
        fd = atoi(fd_input);
		if(fd == 0) {
			printf("Error: Invalid file descriptor\n");
			return -1;
		}
	}
	

	if(fd < 0 || fd > NOFT)
	{
		printf("Error: Invalid file descriptor\n");
		return -1;
	}

	if(running->fd[fd] == NULL)
	{
		printf("Error: Not open file descriptor\n");
		return -1;
	}
	else if(running->fd[fd]->refCount == 0)
	{
		printf("Error: Not open file descriptor\n");
		return -1;
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

int open_file(MINODE * mip, int mode){
	printf("made to open_file\n");
    OFT *ofp;
	int i;

	if(duplicate_oft(mip) == 1)
	{
		printf("Error: File already opened for something other than READ\n");
		return -1;
	}

	ofp = get_oft();
	

	ofp->mode = mode;
	ofp->minodePtr = mip;
	
	switch(ofp->mode)
	{
		case READ_TYPE: 
            ofp->offset = 0;
		    break;
		case WRITE_TYPE: 
            clear_blocks(mip);
		    break;
		case READ_WRITE_TYPE: 
            ofp->offset = 0;
		    break;
		case APPEND_TYPE: 
            ofp->offset = mip->ip.i_size;
		    break;
	}

	for(i = 0; i < NFD; i++)
	{
		if(running->fd[i] == NULL)
		{
			running->fd[i] = ofp;
			return i;
		}
	}
	
    printf("Panic: Out of proc oft pointers\n");
	return -1;

}

int my_open(char * path, char * mode_input) {
    int mode, i_number, device = running->cwd->dev, i;
    MINODE * mip;
    OFT *oftp;

    // set mode type
    if(!strcmp(mode_input, "R"))
        mode = READ_TYPE;
    if(!strcmp(mode_input, "W"))
        mode = WRITE_TYPE;
    if(!strcmp(mode_input, "RW"))
        mode = READ_WRITE_TYPE;
    if(!strcmp(mode_input, "APPEND"))
        mode = APPEND_TYPE;

    i_number = getino(device, path);

    // error checking
    if(i_number < 1) {
        printf("Error: file does not exist\n");
        return -1;
    }

    mip = iget(device, i_number);

    if(!S_ISREG(mip->ip.i_mode)) {
        printf("Error: file is not regular\n");
        return -1;
    }

    // passed checks now open file
    fd = open_file(mip, mode);
    printf("fd of opened file: %d\n", fd);

    return fd;
        
}