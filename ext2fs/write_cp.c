#include "read_cat.c"

// implementation of command "cp" to copy contents of one file to another
int my_cp(char * src_file, char * dest_file) {


    int src_fd, dest_fd, num_bytes, device = running->cwd->dev;
    int src_ino, dest_ino, parent_ino;

    MINODE * src_mip, * dest_mip, * parent_mip;
    char *buf[BLKSIZE];

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

}