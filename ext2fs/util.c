#include "global.c"

// write all modified minodes to disk
int quit(char * rootdev) {
    int i;
    MINODE * mip;
    
    for(i = i; i < NMINODES; i++) {
        mip = &minode[i];
        if(mip->refCount > 0){
            iput(mip);
        }
    }

    printf("Device %s unmounted\n", rootdev);
    exit(0);
}