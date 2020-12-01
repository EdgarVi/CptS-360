#include "symlink.c"

int my_open(char * path, char * mode_input) {
    int mode, device, i_number, device = running->cwd->dev, i, fd;
    MINODE * mip;
    OFT *oftp;

    // set mode type
    if(strcmp(mode_input, "R") == 0)
        mode = READ_TYPE;
    if(strcmp(mode_input, "W") == 0)
        mode = WRITE_TYPE;
    if(strcmp(mode_input, "RW") == 0)
        mode = READ_WRITE_TYPE;
    if(strcmp(mode_input, "APPEND") == 0)
        mode = APPEND_TYPE;

    
        
}