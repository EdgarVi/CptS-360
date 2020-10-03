/* Lab 3 SH Simulator */
#include "sh.h"

int main(int argc, char *argv[], char *env[]){
    
    printf("Entered sh simulator\n");

    int status = 0;
    while(status != -1){
        printf("Enter a command: \n");
        status = run_prompt(env);
    }
    
    return (0);
}