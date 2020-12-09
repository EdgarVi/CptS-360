#include "write_cp.c"

int my_mount(char * pathname) {
    
    int i, fd, ino, device = running->cwd->dev;
    MINODE * mip;
    MTABLE * mountPtr;
    char buf[BLKSIZE];
    SUPER * ext;

    // if no params, display current mounted file systems
    if(strcmp(pathname, "") == 0) {

        printf("mounted file systems:\n");
        for(i = 0; i < NMTABLE; i++){
            if(mtable[i].dev) {
                printf("%s\t%s\n", mtable[i].mntName, mtable[i].devName);
            }
        }
    }

}

int my_unmount(char * pathname) {
    printf("unmount filesystem\n");
}