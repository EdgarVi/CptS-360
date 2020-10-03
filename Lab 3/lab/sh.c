#include "sh.h"

int tokenize(char *input)
{
      
    char *s;
    myargc = 0;
    s = strtok(input, " ");  // first call to strtok()
    strcpy(cmd, s);
    while(s != NULL){
        myargv[myargc] = s;
        s = strtok(NULL, " ");  // call strtok() until it returns NULL
        myargc++;
    }


    // cmd should be first item in myargv

    //print tokenized input
    int i = 0;
    while(i < myargc){
        printf("%d %s\n", i, myargv[i]);
        i++;
    }
    
}

int check_file_redirect(char *env[]){
    printf("In check_file_redirect()\n");
    int i = 0;
    while(i < myargc)
    {
        if(strcmp(myargv[i], "<") == 0){
            i++;
            char *filename = myargv[i];
            printf("new file: %s\n", filename);

            strcpy(cmd, "/bin/");
            strcat(cmd, filename);
            
            close(0);
            open(filename, O_RDONLY|O_CREAT, 0777);    
        
            i--;
        
            while(i <= 64){
                myargv[i] = NULL;
                i++;
            }
                
            int r = execve(cmd, myargv, env); // t.c terminates
        }
        if(strcmp(myargv[i], ">") == 0){
            
            
            i++;
            char *filename = myargv[i];
            printf("new file: %s\n", filename);
            
            close(1);
            open(filename, O_RDONLY|O_CREAT, 0644);    
        
            i--;
        
            while(i <= 64){
                myargv[i] = NULL;
                i++;
            }
                
            int r = execve(cmd, myargv, env); // t.c terminates
            
        }   
        if (strcmp(myargv[i], ">>") == 0){
            i++;
            char *filename = myargv[i];
            printf("new file: %s\n", filename);
            
            close(1);
            open(filename, O_RDONLY|O_CREAT, 0644);    
        
            i--;
        
            while(i <= 64){
                myargv[i] = NULL;
                i++;
            }
                
            int r = execve(cmd, myargv, env); // t.c terminates
        }
        i++;
    }
}

int check_pipe(){
    int i = 0;
    while(i < myargc)
    {
        char *head = myargv[0];
        char *tail = myargv[i+1];
        if(strcmp(myargv[i], "|") == 0){
            printf("pipe found\n");
            int pd[2];
            int pid = pipe(pd);

            if(pid){
                close(pd[0]);
                close(pd[1]);
                dup(pd[1]);
                //exec(head);
            } else {
                close(pd[1]);
                close(0);
                dup(pd[0]);
                close(pd[0]);
                //exec(tail);
            }
        }
        i++;
    }
}

int run_prompt(char *env[])
{
    int result = -1;
    
    char home[100];
    getcwd(home, 100);

    // 1. Prompt for input
    scanf ("%[^\n]%*c", line); // scan all characters until \n encountered   
    tokenize(line);

    // 2. Handle simple commands
    if(strcmp(cmd, "exit") == 0){
        exit(0);
    }

    if(strcmp(cmd, "cd") == 0){
        if(myargv[1] == NULL && myargv[2] == NULL){
            chdir(home);
            char s[100];
            printf("cwd: %s\n", getcwd(s, 100));
            return 1;
        } else if (myargv[2] == NULL){
            chdir(home);
            chdir(myargv[1]);
            char s[100];
            printf("cwd: %s\n", getcwd(s, 100));
            return 1;
        }
    }


    strcpy(cmd, "/bin/");
    strcat(cmd, myargv[0]);

    printf("cmd = %s\n", cmd); // show file to be executed

    //check_file_redirect(env);


    int pid, status; 
    pid = fork(); // for a child
    
    if(pid){
        pid = wait(&status);
        printf("PARENT WAITS FOR CHILD TO DIE\n");
        pid = wait(&status);
        printf("DEAD CHILD = %d, STATUS = 0x%04x\n", pid, status);
    } else {
        printf("CHILD %d DIES BY exit(VALUE)\n", getpid());
        exit(100);
    }

    //check_pipe();

    int r = execve(cmd, myargv, env); // t.c terminates
    printf("execve() failed, r = %d", r); // execve failed


    return r;
}