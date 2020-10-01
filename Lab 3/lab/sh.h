#include <stdio.h>
#include <libgen.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

// Globals
char line[128];
char cmd[16];
char *myargv[64];
char cwd[64];
int myargc;

char *head[64];
char *tail[64];
char *file;

int tokenize(char *input);
int check_file_redirect(char *env[]);
int check_pipe();
int run_prompt(char *env[]);