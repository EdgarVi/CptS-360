/************* t.c file ********************/
#include <stdio.h>
#include <stdlib.h>

int *FP;

int main(int argc, char *argv[ ], char *env[ ])
{
    int a,b,c;
    printf("enter main\n");

    printf("&argc=%x argv=%x env=%x\n", &argc, argv, env);
    printf("&a=%8x &b=%8x &c=%8x\n", &a, &b, &c);

    //(1). Write C code to print values of argc and argv[] entries
    printf("Number of arguments: %d\n", argc);
    
    printf("Contents of argv[]: ");
    int i = 0;
    while(i < argc) {
        printf("%s ", argv[i]);
        i++;
    }
    printf("\n");

    a=1; b=2; c=3;
    A(a,b);
    printf("exit main\n");
}

int A(int x, int y)
{
    int d,e,f;
    printf("enter A\n");
    // write C code to PRINT ADDRESS OF d, e, f
    d=4; e=5; f=6;

    printf("Address of d: %8X, ", &d);
    printf("Address of e: %8X, ", &e);
    printf("Address of f: %8X\n", &f);

    B(d,e);
    printf("exit A\n");
}

int B(int x, int y)
{
    int g,h,i;
    printf("enter B\n");
    // write C code to PRINT ADDRESS OF g,h,i
    g=7; h=8; i=9;

    printf("Address of g: %8X, ", &g);
    printf("Address of h: %8X, ", &h);
    printf("Address of i: %8X\n", &i);

    C(g,h);
    printf("exit B\n");
}

int C(int x, int y)
{
    int u, v, w, i, *p;

    printf("enter C\n");
    // write C cdoe to PRINT ADDRESS OF u,v,w,i,p;
    u=10; v=11; w=12; i=13;

    printf("Address of u: %8X, ", &u);
    printf("Address of v: %8X, ", &v);
    printf("Address of w: %8X, ", &w);
    printf("Address of i: %8X, ", &i);
    printf("Address of p: %8X, ", &p);

    FP = (int *)getebp();  // FP = stack frame pointer of the C() function


    //(2). Write C code to print the stack frame link list.
    printf("Contents of frame pointer link list:\n");
    while(FP != 0){
        printf("\tFP -> %8X -> \n", FP);
        FP = *FP;
    }
    printf("\tFP -> %8X\n", FP);

    p = (int *)&p;

    //(3). Print the stack contents from p to the frame of main()
    //    YOU MAY JUST PRINT 128 entries of the stack contents.
    printf("Contents of p:\n");
    int count = 0;
    while (count < 128){
        printf("%d(p) -> %8X\n%d(p) = %d\n\n", count, p, count, *p);
        p++;
        count++;
    }


    //(4). On a hard copy of the print out, identify the stack contents
    //    as LOCAL VARIABLES, PARAMETERS, stack frame pointer of each function.
}