#include "read_cat.c"


int my_write(int dest_fd, char * buf, int num_bytes) {
    printf("called my write\n");
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

	src_fd = my_open(src_file, READ_TYPE);
	dest_fd = my_open(dest_file, WRITE_TYPE);

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