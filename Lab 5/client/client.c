#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <sys/stat.h>

#define MAX   256
#define PORT 1234

char line[MAX], ans[MAX], temp[MAX], res[MAX], cwd[MAX];
int n, i, m;
char *token;
char *piece;
char *cmd;
char *myargv[32];
char myargc;

struct hostent *hp;
struct sockaddr_in server_addr;
int cfd, r;


char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

int checkValid(){
    // -1 for local, 1 for not 
    if(strcmp(cmd, "ls") == 0){
        return 1;
    } else if(cmd[0] == 'l'){
        printf("running local command\n");
        return -1;
    } else {
        return 1;
    }
}

int validCommand(){
    if(strcmp(cmd, "ls") == 0)
        return 1;
    if(strcmp(cmd, "pwd") == 0)
        return 1;
    if(strcmp(cmd, "cd") == 0)
        return 1;
    if(strcmp(cmd, "mkdir") == 0)
        return 1;
    if(strcmp(cmd, "rmdir") == 0)
        return 1;
    if(strcmp(cmd, "rm") == 0)
        return 1;
    if(strcmp(cmd, "get") == 0)
        return 1;
    if(strcmp(cmd, "put") == 0)
        return 1;
    if(strcmp(cmd, "quit") == 0)
        return 1;
    
    // local commands
    if(strcmp(cmd, "lcat") == 0)
        return 1;
    if(strcmp(cmd, "lpwd") == 0)
        return 1;
    if(strcmp(cmd, "lls") == 0)
        return 1;
    if(strcmp(cmd, "lcd") == 0)
        return 1;
    if(strcmp(cmd, "lmkdir") == 0)
        return 1;
    if(strcmp(cmd, "lrmdir") == 0)
        return 1;
    if(strcmp(cmd, "lrm") == 0)
        return 1;
    

    return 0;
}

// function which makes sense of server response
int response(char *cmd, int myargc, char *myargv[]){
    
    int fd;
    int fp;
    char buf[1024];

    struct stat fstat, *sp;

    if(myargc > 1 && strcmp(cmd, "get") == 0){
        fp = open(myargv[1], O_WRONLY | O_CREAT);
        read(cfd, res, MAX);
        int size = atoi(res);
        printf("size = %d\n", size);
        int count = 0;
        while(count < size){
            n = read(cfd, buf, MAX);
            count += n;
            write(fp, buf, n);
        }
        close(fp);
        printf("Server done sending file\n");
    }

    if(myargc > 1 && strcmp(cmd, "put") == 0){

        lstat(myargv[1], &fstat);

        fd = open(myargv[1], O_RDONLY);
        sprintf(ans, "%8d", sp->st_size);

        write(cfd, ans, MAX);
        printf("putting file %s to server\n", myargv[1]);
        while(m = read(fd, buf, MAX)){
            write(cfd, buf, m);
        }
        close(fd);
    }


    if(strcmp(cmd, "pwd") == 0){
        n = read(cfd, cwd, MAX);
        printf("pwd server response: %s\n", cwd);
    }

    if(strcmp(cmd, "ls") == 0){
        n = read(cfd, res, MAX);
        printf("server response: %s\n", res);
        
        // res is dir
        if(strcmp(res, "dir") == 0){
            while(strcmp(res, "ls complete") != 0){
                bzero(res, MAX);
                read(cfd, res, MAX);
                printf("%s\n", res);
            }
        } else {
            // res is file
            bzero(res, MAX);
            read(cfd, res, MAX);
            printf("%s\n", res);
        }
    }
}



// print file and permissions
int LSFile(char *filename){
    // following algorithm from textbook, Ch. 8, the LS program
    struct stat mystat, *sp;
    struct stat fstat;
    int r, i;
    char ftime[64];


    sp = &fstat;

    if((r = lstat(filename, &fstat)) < 0){
        printf("Can't stat %s\n", filename);
        exit(1);
    }

    // if S_ISREG
    if((sp->st_mode & 0XF000) == 0x8000){
        printf("%c", '-');
    }

    // if S_ISDIR
    if((sp->st_mode & 0XF000) == 0x4000){
        printf("%c", 'd');
    }

    // if S_ISLINK
    if((sp->st_mode & 0XF000) == 0xA000){
        printf("%c", '-1');
    }

    // print permissions
    for(i = 8; i >= 0; i--){
        if(sp->st_mode & (1 << i)){
            printf("%c", t1[i]);
        } else {
            printf("%c", t2[i]);
        }
    }

    printf("%4d ", sp->st_nlink);
    printf("%4d ", sp->st_gid);
    printf("%4d ", sp->st_uid);
    printf("%4d ", sp->st_size);
    
    // print time
    printf("%s", ctime(&sp->st_ctime));
    ftime[strlen(ftime) - 1] = 0; // kill newline char
    printf("%s", ftime);

    // print name
    printf("%s", basename(filename));

    // print linkname
    if((sp->st_mode & 0xF000) == 0xA000){
        char *linkname;
        const char *pathname;
        pathname = dirname(linkname);
        r = readlink(filename, linkname, sp->st_size + 1);
        if(r < 0){
            printf("readlink failed\n");
        }
        printf(" -> %s", *linkname);
    }
    printf("\n");
}

int LSDir(char *dirname){
    DIR *dp;
    struct dirent *ep;
    dp = opendir(dirname);
    while(ep = readdir(dp)){
        LSFile(ep->d_name);
    }
}

// execute commands locally
int executeLocal(char *cmd, int myargc, char *myargv[]){

    struct stat fstat, *sp;
    sp = &fstat;

    char *op = 0;

    if(myargc > 1){
        op = myargv[1];
    }

    if(strcmp(cmd, "lpwd") == 0){
        getcwd(cwd, sizeof(cwd));
        printf("LOCAL cwd: %s\n", cwd);
        return 1;
    }

   if(strcmp(cmd, "lls") == 0){
       if(myargc > 1){
           if(S_ISDIR(sp->st_mode)){
               LSDir(myargv[1]);
               return 1;
           } else {
               LSFile(myargv[1]);
               return 1;
           }
       } else if(myargc == 1){
           LSDir("./"); // ls cwd
           return 1;
       }
   }

   if(strcmp(cmd, "lcd") == 0){
       if(myargc > 1){
            getcwd(cwd, MAX);
            strcat(cwd, "/");
            strcat(cwd, op);
            printf("myargv[1] = %s\n", myargv[1]);
            chdir(cwd);
            printf("cwd: %s\n", cwd);
            return 1;
            

       } else if(myargc == 1){
           // print home dir
           chdir("home/edgar");
           printf("cwd set to home\n");
           printf("cwd: %s\n", cwd);
           return 1;
       }
       
   }

    if(strcmp(cmd, "lmkdir") == 0){
        int res = mkdir(op, 0777);

        if(res < 0){
            printf("mkdir failed\n");
            return -1;
        } else {
            printf("mkdir worked\n");
        }
    }

    return 0;
}

void printMenu(){
    printf("\n-------------REMOTE COMMANDS------------\n");
    printf("pwd | ls | cd | mkdir | rmdir | rm | get | put\n");
    printf("-------------LOCAL COMMANDS------------\n");
    printf("lcat | lpwd | lls | lcd | lmkdir | lrmdir | lrm\n");
    printf("Enter \"quit\" to kill client\n");
    printf("---------------------------------\n\n");
}

int main(int argc, char *argv[ ]) 
{ 
     
    struct sockaddr_in saddr; 
    hp = getenv("HOME");

    printf("1. create a TCP socket\n");
    cfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (cfd < 0) { 
        printf("socket creation failed\n"); 
        exit(1); 
    }

    printf("2. fill in [localhost IP port=1234] as server address\n");
    bzero(&saddr, sizeof(saddr)); 
    saddr.sin_family = AF_INET; 
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    saddr.sin_port = htons(PORT); 

    printf("3. connect client socket to server\n");
    if (connect(cfd, (struct sockaddr *)&saddr, sizeof(saddr)) != 0) { 
        printf("connection to server failed\n"); 
        exit(2); 
    }
    
    printMenu();

    printf("********  processing loop  *********\n");
    while(1){
        int o = 0;
        printf("input a line : ");
        bzero(line, MAX);                // zero out line[ ]
        fgets(line, MAX, stdin);         // get a line (end with \n) from stdin

        line[strlen(line)-1] = 0;        // kill \n at end
        if (line[0]==0){
            printf("NULL line encountered\n");
            printf("Client dies\n");
            exit(0);
        }

        strcpy(temp, line);
        token = strtok(temp, " ");

        while(token != NULL){
            myargc++;
            myargv[o] = token;
            token = strtok(NULL, " ");
            o++;
        }
        cmd = myargv[0];

        if(strcmp(cmd, "quit") == 0){
            n = write(cfd, line, MAX);
            printf("client: wrote n=%d bytes; line=(%s)\n", n, cmd);
            exit(0);
        }

        // now I need to check if command is local
        int localOrNot = checkValid(); 

        // need to check if command is valid, implement when I get home from 464!!!!
        int valid = validCommand();

        if(valid == 0){
            printf("ERROR: Command invalid\n");
            printf("Client dies\n");
            exit(0);
        }

        // now send command to server
        if(localOrNot == 1 && valid == 1){
            
            n = write(cfd, line, MAX);
            m = read(cfd, res, MAX);

            if(strcmp(res, "1") == 0){
                // success
                printf("server responding\n");
                response(cmd, myargc, myargv);
            } else {
                printf("FAIL: No response from server\n");
            }
            
        } else if (localOrNot == -1 && valid == 1){
            // run commands locally
            executeLocal(cmd, myargc, myargv);
        }

        
    }
} 