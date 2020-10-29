#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <arpa/inet.h>
#include <dirent.h>

#define MAX   256
#define PORT 1234
  
char cwd[MAX];
int n, myargc;

int sfd, cfd, len;

char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

int LSFile(char *filename){
    // following algorithm from textbook, Ch. 8, the LS program
    struct stat mystat, *sp;
    struct stat fstat;
    int r, i;
    char ftime[64];

    char ans[MAX]; // what gets sent to client
    char temp[MAX];   

    sp = &fstat;

    if((r = lstat(filename, &fstat)) < 0){
        printf("Can't stat %s\n", filename);
        exit(1);
    }

    // if S_ISREG
    if((sp->st_mode & 0XF000) == 0x8000){
        sprintf(temp, "%c", '-');
        strcpy(ans, temp);
    }

    // if S_ISDIR
    if((sp->st_mode & 0XF000) == 0x4000){
        sprintf(temp, "%c", 'd');
        strcpy(ans, temp);
    }

    // if S_ISLINK
    if((sp->st_mode & 0XF000) == 0xA000){
        sprintf(temp, "%c", '1');
        strcpy(ans, temp);
    }

    // print permissions
    for(i = 8; i >= 0; i--){
        if(sp->st_mode & (1 << i)){
            sprintf(temp, "%c", t1[i]);
            strcat(ans, temp);
        } else {
            sprintf(temp, "%c", t2[i]);
            strcat(ans, temp);
        }
    }

    sprintf(temp, "%4d ", sp->st_nlink);
    strcat(ans, temp);
    sprintf(temp, "%4d ", sp->st_gid);
    strcat(ans, temp);
    sprintf(temp, "%4d ", sp->st_uid);
    strcat(ans, temp);
    sprintf(temp, "%4d ", sp->st_size);
    strcat(ans, temp);
      

    // print time
    //bzero(ftime, MAX);
    strcat(ftime, ctime(&sp->st_ctime));
    ftime[strlen(ftime) - 1] = 0; // kill newline char
    sprintf(temp, "%s", ftime);
    
    //printf("permissions: %s\n", ans); 
    strcat(ans, temp);

    //printf("permissions and time: %s\n", ans);
    


    // print name
    sprintf(temp, "%s", basename(filename));
    strcat(ans, temp);
    printf("%s", ans);
    write(cfd, ans, MAX);

}

int LSDir(char *dirname){
    DIR *dp;
    struct dirent *ep;
    dp = opendir(dirname);
    
    while(ep = readdir(dp)){
        LSFile(ep->d_name);
    }

    write(cfd, "ls complete", MAX);

}

int execute(int myargc, char *myargv[]){

    char *cmd = myargv[0];
    char pwd[MAX];

    char answer[MAX];
    struct stat fstat, *sp;

    sp = &fstat;
    bzero(answer, MAX);

    char *op = 0;
    char buf[1024];
    
    int m = 0;
    int r, i;

    int fd, fp;
    
    // get command line arg
    if(myargc > 1){
        op = myargv[1];
    }

    if(strcmp(cmd, "get") == 0){
        
        if(myargc > 1){
            bzero(buf, 1024);
                        
            fd = open(myargv[1], O_RDONLY);

            write(cfd, "1", MAX);
            sprintf(answer, "%8d", sp->st_size);
            write(cfd, answer, MAX);
            printf("Sending %s to client\n", myargv[1]);
            while(m = read(fd, buf, MAX)){
                write(cfd, buf, m);
            }
            close(fd);
        }
        
    }

    if(strcmp(cmd, "put") == 0){
        if (myargc > 1){
            bzero(buf, 1024);
            write(cfd, "1", MAX);
            fp = open(myargv[1], O_WRONLY | O_CREAT);

            read(cfd, answer, MAX);

            int size = atoi(answer);
            
            printf("size %d\n", size);
            
            int count = 0;
            while(count < size){
                n = read(cfd, buf, MAX);
                count += n;
                write(fp, buf, n);
            }
            printf("put done\n");
            close(fp);
        }

    }

    if(strcmp(cmd, "pwd") == 0){
        printf("in execute\n");
    
        write(cfd, "1", MAX); // write success
        getcwd(cwd, MAX);
        write(cfd, cwd, MAX);
        printf("Client request pwd: %s\n", cwd);
        return 1;
    }

    if(strcmp(cmd, "ls") == 0){
        write(cfd, "1", MAX); // write success
        
        if(myargc > 1){
           if(S_ISDIR(sp->st_mode)){
               strcat(answer, "dir");
               write(cfd, answer, MAX);
               LSDir(myargv[1]);
               return 1;
           } else {
               strcat(answer, "file");
               write(cfd, answer, MAX);
               LSFile(myargv[1]);
               return 1;
           }
       } else if(myargc == 1){
           strcat(answer, "dir");
            write(cfd, answer, MAX);
            LSDir("./"); // ls cwd
            return 1;
       }
    }

    if(strcmp(cmd, "cd") == 0){

        write(cfd, "1", MAX);
        if(myargc > 1){
            getcwd(cwd, MAX);
            strcat(cwd, "/");
            strcat(cwd, op);
            printf("Changing cwd to: %s\n", cwd);
            if(!(chdir(cwd) < 0)){
                printf("lcd worked, cwd: %s\n", cwd);
                write(cfd, cwd, MAX);
                return 1;
            }

       } else if(myargc == 1){
           // print home dir
           chdir("home/edgar");
           printf("cwd set to home\n");
           write(cfd, cwd, MAX);
           printf("cwd: %s\n", cwd);
           return 1;
       }
    }

    if(strcmp(cmd, "mkdir") == 0){
        
        write(cfd, "1", MAX);
        int res = mkdir(op, 0777);

        if(res < 0){
            printf("mkdir failed\n");
            strcat(answer, "mkdir failed");
            write(cfd, answer, MAX);
            return -1;
        } else {
            printf("mkdir worked\n");
            strcat(answer, "mkdir worked");
            write(cfd, answer, MAX);
            return 1;
        }
    }
}

int main(int argc, char *argv[]) 
{ 
    char line[MAX], temp[MAX], *token, *cmd;
    int myargc;
    
    struct sockaddr_in saddr, caddr; 
  
    printf("1. create a TCP socket\n");
    sfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sfd < 0) { 
        printf("socket creation failed\n"); 
        exit(1); 
    }

    printf("2. fill in [localhost IP port=1234] as server address\n");
    bzero(&saddr, sizeof(saddr)); 
    saddr.sin_family = AF_INET; 
    saddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    saddr.sin_port = htons(PORT); 
  
    printf("3. bind socket with server address\n");
    if ((bind(sfd, (struct sockaddr *)&saddr, sizeof(saddr))) != 0) { 
        printf("socket bind failed\n"); 
        exit(2); 
    }
      
    printf("4. server listens\n");
    if ((listen(sfd, 5)) != 0) { 
        printf("Listen failed\n"); 
        exit(3); 
    }

    len = sizeof(caddr);
    while(1){
        char *myargv[32];
        int o = 0;

        printf("server accepting connection\n");
        cfd = accept(sfd, (struct sockaddr *)&caddr, &len); 
        if (cfd < 0) { 
            printf("server acccept failed\n"); 
            exit(4); 
        }
        printf("server acccepted a client connection\n"); 

        // server Processing loop
        while(1){
            myargc = 0;

            printf("server: ready for next request\n");
            n = read(cfd, line, MAX);
            if (n == 0){
                printf("server: client died, server loops\n");
                close(cfd);
                break;
            }

            printf("Command \"%s\" received from client\n", line);

            strcpy(temp, line);
            token = strtok(temp, " ");

            while(token != NULL){
                myargc++;
                myargv[o] = token;
                token = strtok(NULL, " ");
                o++;
            }

            execute(myargc, myargv);
            myargc = 0;
            o = 0;
        }
    }
} 
