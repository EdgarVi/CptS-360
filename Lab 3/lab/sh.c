#include "sh.h"

int tokenize(char *input) {


    char *s;
    myargc = 0;

    s = strtok(input, " ");
    strcpy(cmd, s);
    while(s != NULL) {
        myargv[myargc] = s;
        s = strtok(NULL, " "); // call strtok() until it returns NULL
        myargc++;
    }

    // cmd should be first item in myargv
    int i = 0;
    while(i < myargc) {
        printf("%d %s\n", i, myargv[i]);
        i++;
    }
}