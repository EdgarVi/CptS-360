#include "symlink.c"

int my_open(char * path, char * mode) {
    printf("%s %s", path, mode);
}