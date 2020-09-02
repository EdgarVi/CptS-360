/*************PART 2 MYPRINTF FUNCTION***************/

typedef unsigned int u32;

char *ctable = "0123456789ABCDEF";


int main(int argc, char *argv[ ], char *env[ ])
{ 

    // command line arguments, print argc contents of argv 
    myprintf("COMMAND LINE ARGUMENTS\n");
    myprintf("argc = %d\n", argc);
    myprintf("Contents of argv: ");
    for(int i = 0; i < argc; i++){
        myprintf("argv[%d]: %s\n", i, argv[i]);
    }
    myprintf("\n");


    // Tests
    // hard coded tests
    myprintf("HARD CODED TESTS\n");
    myprintf("cha=%c string=%s      dec=%d hex=%x oct=%o neg=%d\n", 
    'A', "this is a test", 100,    100,   100,  -100);
    myprintf("here's ANOTHER %d c%d%dl myprintf %x %o test\n", -55, 0, 0, 49, 19);

    // test hex and oct
    for(int l = 0; l < 16; l++){
        myprintf("dec: %d hex: %x oct: %o\n", l, l, l);
    }

    // test negatives
    for(int q = -16; q < 0; q++){
        myprintf("neg dec: %d\n", q);
    }
}


recursivePrint(u32 x, int BASE)
{
    char c;
    if (x){
        c = ctable[x % BASE];
        recursivePrint(x / BASE, BASE);
        putchar(c);
    }  
}

int printu(u32 x)
{
   (x==0)? putchar('0') : recursivePrint(x, 10);
   putchar(' ');
}

/***************ASSIGNMENT CODE*********************/

//pass in format as a string, recall we have to read all of fmt until we see %, 
//then we grab the next char as format and use ip from the stack as value
//had to change the return type to void from int, not sure why the latter would not compile
void myprintf(char *fmt, ...)
{
  
    char * cp = fmt; //points at format string
    int * ip; //points at the first item to be printed on stack
    // by incrementing ip, we'll go towards the higher addresses (towards the other parameters)
    // recall stack looks like: HIGH --- parameters --- retPC --- ebp -- locals --- LOW
    
    
    ip = (int *)getebp(); // ip = stack frame pointer of the C() function, included the assembly code from the pre lab
    ip += 3; // parameters | fmt |retPC| ebp <- ip points here after getebp is called! So advance ip by 3 times
    


    while(*cp != '\0'){
        if(*cp == '%'){
            cp++; // get format character
            printFactory(*cp, ip);
            ip++; // advance pointer 
            cp++; //advance the string 
        } else {
            putchar(*cp); // print a single character (anything that's not a value)
            cp++;
        }
    }
}
    
// design pattern
void printFactory(char context, int * ip)
{
    switch(context)
    {
        case 'c':
            putchar(*ip);
            break;
        case 's':
            myprints(*ip);
            break;
        case 'u':
            printu(*ip);
            break;
        case 'd':
            printd(*ip);
            break;
        case 'o': 
            printo(*ip);
            break;
        case 'x':
            printx(*ip); 
            break;
        default:
            printf("ERROR: Enter a valid format\n");
            break;
    }
}

// pass in a pointer to a string
int myprints(char *s)
{
    while(*s != '\0'){
        putchar(*s);
        s++; //advance pointer
    }
}

// prints an integer (x may be negative!!!)
int  printd(int x)
{
    (x >= 0) ? printu(x) : handleNegative(x);
}

void handleNegative(int x)
{
    x *= -1;
    putchar('-');
    printu(x);
}

// prints x in HEX (start with 0x )
int  printx(u32 x)
{
    putchar('0');
    putchar('X');
    recursivePrint(x, 16);
}

// prints x in Octal (start with 0)
int  printo(u32 x) 
{
    putchar('0');
    recursivePrint(x, 8);
}