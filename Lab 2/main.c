/**************LAB 2 UNIX FILE TREE SIMULATOR*********************/
#include <stdio.h>
#include <libgen.h>
#include <string.h>
#include <stdlib.h>

#define EXIT_CODE 10 // index of quit in the function pointer table
#define SUCCESS_CODE 1
#define ERROR_CODE -1 // value to return if code fails
// NODE definition
typedef struct node {
    struct node *child; // points to oldest child (points to a linked list of children pointers (basically this linked list shows the contents of the directory) 
    struct node *sibling; // points to oldest sibling (which will be a linked list of adjacent dir and files)
    struct node *parent; 
    char name[64];
    char type; // Either 'D' for directory or 'F' for file
} NODE;

// GLOBALS
NODE *root, *cwd,*start; // cwd is acting like a linked list to root
char line[128]; // user input line
char command[16], pathname[64]; 
char dname[64], bname[64]; // directory name and base name
char *cmd[] = {"mkdir", "rmdir", "ls", "cd", "pwd", "creat", "rm",
        "save", "reload", "menu", "quit", NULL};
char *name[128]; // pointers to token strings in pathname[]
int n; // number of tokens in pathname
char CWD[64]; // for use in pwd
FILE *fp;

/**************GIVEN CODE*********************/
int tokenize(char *pathname)
{
      
    char *s;
    int i;

    n = 0;
    s = strtok(pathname, "/");  // first call to strtok()
    while(s){
        name[n] = s;
        
        s = strtok(0, "/");  // call strtok() until it returns NULL
        n++;
    }

}

// decompose pathname into dname and bname
int decomposePathname(char *pathname)
{
    char temp[128];  // dirname(), basename() destroy original pathname
    strcpy(temp, pathname);
    strcpy(dname, dirname(temp));
    strcpy(temp, pathname);
    strcpy(bname, basename(temp));
}

NODE * searchChild(NODE *parent, char *name)
{
    NODE *cp;   

    cp = parent->child;
    if(cp == 0){
        return 0;
    }
    while(cp){
        if(strcmp(cp->name, name) == 0){
            return cp;
        }
        cp = cp->sibling;
    }
    return 0;
}

NODE * pathToNode(char *pathname)
{
    NODE *p; 

    start = root;
    if(pathname[0] != '/'){
        start = cwd;
    }

    tokenize(pathname);

    // if no token strings in pathname[]
    if(n == 0){
        return start;
    }

    p = start;
    for(int i = 0; i < n; i++){
        p = searchChild(p, name[i]);
        if(p == 0){
            printf("Can't find %s\n", name[i]);
            return NULL;
        }
    }

    return p;
}

/**************ASSIGNMENT CODE*********************/
NODE * makeNode()
{
    
    NODE * pMem = NULL;
    pMem = (NODE *)malloc(sizeof(NODE));

    if(pMem != NULL)
    {

        // initialize pointers
        pMem->parent = NULL;
        pMem->sibling = NULL;
        pMem->child = NULL;

        strcpy(pMem->name, "");
        pMem->type = '\0';
        
        return pMem;
    } else {
        printf("malloc failed");
        return NULL;
    }

}


int mkdir(char *pathname)
{
    char relPath[64];
    strcpy(relPath, pathname);
    NODE *dirNode = makeNode();
    dirNode = root;
    
    // 1. Break up pathname
    tokenize(pathname);
    strcpy(dname, "");
    int i = 0;
    
    // decompose the pathname
    while(i < n - 1){
        strcat(dname, "/");
        strcat(dname, name[i]);
        i++;
    }

    strcpy(bname, name[i]); // basename is the last string in name[] 
    
    // If root is childless, insert
    if(root->child == NULL)
    {

        printf("root directory is empty\n");
    
        NODE * newDir = makeNode();
        
        strcpy(newDir->name, bname);
        newDir->type = 'D';
        newDir->parent = root;
        root->child = newDir;

        // set cwd
        cwd = root;

        printf("name of node added: %s\n", newDir->name);
    
        return SUCCESS_CODE;
    }


    // 2. Search for dname node    
    
    // relative
    if(relPath[0] != '/'){
        dirNode = cwd;
        printf("%s\n", dirNode->name);
        printf("looking for relative path\n");
    } else {
        printf("looking for absolute path\n");
        dirNode = pathToNode(dname); // look for directory
    }
    

    // directory does not exist
    if(dirNode == NULL && dname[0] != '\0'){
        printf("---------ERROR---------\nDirectory does not exist\n");
        return ERROR_CODE;
    }

    NODE * child = searchChild(dirNode, bname); // look for file
    // directory exists but is actually a file
    if(child != NULL){
        if(child->type == 'F'){  
            printf("---------ERROR---------\nNode with dname exists but is actually a file");
            return ERROR_CODE;
        }
    }
    
    
    // passed all safeguards, add node
    NODE * newDir = makeNode();
    newDir->type = 'D';
    strcpy(newDir->name, bname);
    
    if(dname[0] == '\0' && relPath[0] == '\0'){
        
        dirNode = root;
    }
    
    // insert new child node, newest 
    if(dirNode->child == NULL){
        printf("Inserting first child to child linked list\n");
        dirNode->child = newDir;
        newDir->parent = dirNode;

        // ensure our list stays doubly linked    
        cwd = dirNode;
        cwd->child = newDir;
        
    
    } else {
        
        // insert at front of children linked list
        printf("Inserting at front of child linked list\n");
        NODE * pMem = makeNode(); // current head of list
        pMem = dirNode->child; 

        // reset pointers
        newDir->sibling = pMem;
        newDir->parent = dirNode;
        dirNode->child = newDir;
        pMem->parent = NULL;
        
        // link cwd
        cwd = dirNode;
        cwd->child = newDir;
        
    }
    
    printf("name of node added: %s\n", newDir->name);
    return SUCCESS_CODE;
}
    
// BST Equivalent: remove node
int rmdir(char *pathname)
{
    
    // 1. if pathname is absolute, start = "/"
    //          else start = cwd
    tokenize(pathname);
    strcpy(dname, "");
    int i = 0;
    // decompose the pathname
    while(i < n - 1){
        strcat(dname, "/");
        strcat(dname, name[i]);
        i++;
    }

    strcpy(bname, name[i]); // basename is the last string in name[] 
    
    // check if root is childless
    if(root->child == NULL){
        printf("Root directory childless, nothing deleted\n");
        return SUCCESS_CODE;
    }

    // 2. Search for pathname node
    NODE *dirNode = makeNode();
    dirNode = pathToNode(dname); // pathToNode will take care of if-else statement from previous part
    if(dname[0] == '\0'){
        
        dirNode = root;
    }
    NODE *child = makeNode(); // DIR is the node to be deleted, dirNode is parent
    child = searchChild(dirNode, bname);
    // directory does not exist
    if(child == NULL){
        printf("---------ERROR---------\nDirectory does not exist\n");
        return ERROR_CODE;
    }

    // 3. If pathname exists:
    // Check if it's a DIR
    if(child->type == 'F'){  
        printf("---------ERROR---------\nNode with dname exists but is actually a file");
        return ERROR_CODE;
    }
    
    // Check if DIR is empty (can't rmdir if empty)
    
    if(child->child != NULL)
    {
        printf("---------ERROR---------\n%s must be empty before it can be removed\n", bname);
        return ERROR_CODE;
    }

    // 4. Delete node from parent's child list
    // passed all safeguards, delete bname dir
    NODE *pHead = makeNode();
    NODE *pTemp = makeNode();
    NODE *pPrev = makeNode();

    pTemp = dirNode->child;
    cwd = dirNode;
    int curPos = 0;

    // delete first node
    if(pTemp != NULL && strcmp(pTemp->name, bname) == 0)
    {
        printf("Deleting first child\n");
        pHead->parent = dirNode; // set correct parents
        pHead = pTemp->sibling; // advance pHead
        dirNode->child = pHead; // make sure parent points down correctly
        free(pTemp); // delete old head
        return SUCCESS_CODE;
    }

    // traverse list
    while(pTemp != NULL && strcmp(pTemp->name, bname) != 0)
    {
        pPrev = pTemp;
        pTemp = pTemp->sibling;
        curPos++;
    }

    if(pTemp == NULL)
    {
        printf("---------ERROR---------\nNode to delete was not found");
        return ERROR_CODE;
    }
    printf("Deleted node at position %d in the child linked list\n", curPos);
    
    pPrev->sibling = pTemp->sibling; // now, no nodes point to pTemp
    
    free(pTemp); // free pTemp
    return SUCCESS_CODE;
}

int cd(char *pathname)
{
    
    // go up a directory
    if(strcmp(pathname, "..") == 0)
    {
        if(cwd->parent != NULL){
        cwd = cwd->parent;}
    } else {
        cwd = pathToNode(pathname);
    }
    return SUCCESS_CODE;
}


int ls(char *pathname)
{
    NODE *pTemp = cwd; // use pTemp for tree navigation

    // case that we're in the root directory
    if(strcmp(pTemp->name, "/") == 0){
        printf("Contents of %s\n", pTemp->name);
        pTemp = pTemp->child;
        if(pTemp == NULL) {
            printf("There's nothing in the root directory!\n");
        } else {
            while(pTemp != NULL){
                printf("%s(%c)\t", pTemp->name, pTemp->type);
                pTemp = pTemp->sibling;
            }       
        }
        return SUCCESS_CODE;
    } else {
        if(pTemp->child == NULL){
            printf("Nothing in this directory!\n");
        } else {
            printf("Contents of %s\n", pTemp->name);
            pTemp = pTemp->child;
            while(pTemp != NULL){
                printf("%s(%c)\t", pTemp->name, pTemp->type);
                pTemp = pTemp->sibling;
            }
        }
    }

    return SUCCESS_CODE;
}

/*
int pwdHelper(NODE *pCur)
{
    if(pCur->parent != NULL){
        char *tempStr;
        printf("pCur->name: %s\n", pCur->name);
        strcpy(tempStr, pCur->name);
        strcat(tempStr, CWD);
        strcpy(CWD, tempStr);
        pwdHelper(pCur->parent);
    }   
}
*/
// BST Equivalent: Print current level
int pwd()
{
    NODE *pMem = cwd;
    
    strcpy(CWD, ""); // clear current CWD
    
    //pwdHelper(pMem);
    
    while(pMem != NULL)
    {
        if(strcmp(pMem->name, root->name) == 0){
            char tempStr[64];
            strcpy(tempStr, pMem->name);
            strcat(tempStr, CWD);
            strcpy(CWD, tempStr);
            pMem = NULL;
        } else {
            char tempStr[64];
            printf("pMem->name: %s\n", pMem->name);
            strcpy(tempStr, pMem->name);
            strcat(tempStr, "/");
            strcat(tempStr, CWD);
            strcpy(CWD, tempStr);
            pMem = pMem->parent;
        }
    }
    
    printf("Working directory: %s\n", CWD);
}

// Not a typo. Same as mkdir, but for files
int creat(char *pathname)
{
   
    NODE *dirNode = makeNode();
    dirNode = root;
    
    // 1. Break up pathname
    tokenize(pathname);
    strcpy(dname, "");
    int i = 0;
    
    // decompose the pathname
    while(i < n - 1){
        strcat(dname, "/");
        strcat(dname, name[i]);
        i++;
    }

    strcpy(bname, name[i]); // basename is the last string in name[] 
    
    // If root is childless, insert
    if(root->child == NULL)
    {

        printf("root directory is empty\n");
    
        NODE * newDir = makeNode();
        
        strcpy(newDir->name, bname);
        newDir->type = 'F';
        newDir->parent = root;
        root->child = newDir;

        printf("name of node added: %s\n", newDir->name);
    
        return SUCCESS_CODE;
    }


    // 2. Search for dname node    
    dirNode = pathToNode(dname);
    cwd = dirNode;

    // directory does not exist
    if(dirNode == NULL && dname[0] != '\0'){
        printf("---------ERROR---------\nDirectory does not exist\n");
        return ERROR_CODE;
    }

    NODE *child = searchChild(dirNode, bname);
    // directory exists but is actually a directory
    if(child != NULL){
        if(dirNode->type == 'D'){  
            printf("---------ERROR---------\nNode with dname exists but is actually a directory");
            return ERROR_CODE;
        }
    }

    // passed all safeguards, add node
    NODE * newDir = makeNode();
    newDir->type = 'F';
    strcpy(newDir->name, bname);
    
    if(dname[0] == '\0'){
        
        dirNode = root;
    }

    // insert new child node, newest 
    if(dirNode->child == NULL){
        printf("Inserting first child to child linked list\n");
        dirNode->child = newDir;
        newDir->parent = dirNode;

    } else {
        
        // insert at front of children linked list
        printf("Inserting at front of child linked list\n");
        NODE * pMem = makeNode();
        pMem = dirNode->child;

        // reset pointers
        newDir->sibling = pMem;
        newDir->parent = dirNode;
        dirNode->child = newDir;
        pMem->parent = NULL;

    }
    
    printf("name of node added: %s\n", newDir->name);
    return SUCCESS_CODE;
} 

// remove file
int rm(char *pathname)
{

    // 1. if pathname is absolute, start = "/"
    //          else start = cwd
    tokenize(pathname);
    strcpy(dname, "");
    int i = 0;
    // decompose the pathname
    while(i < n - 1){
        strcat(dname, "/");
        strcat(dname, name[i]);
        i++;
    }

    strcpy(bname, name[i]); // basename is the last string in name[] 
    
    // check if root is childless
    if(root->child == NULL){
        printf("Root directory childless, nothing deleted\n");
        return SUCCESS_CODE;
    }

    // 2. Search for pathname node
    NODE *dirNode = makeNode();
    dirNode = pathToNode(dname); // pathToNode will take care of if-else statement from previous part
    if(dname[0] == '\0'){
        
        dirNode = root;
    }
    
    NODE *child = makeNode(); // DIR is the node to be deleted, dirNode is parent
    child = searchChild(dirNode, bname);
    // directory does not exist
    if(child == NULL){
        printf("---------ERROR---------\nDirectory does not exist\n");
        return ERROR_CODE;
    }

    // 3. If pathname exists:
    // Check if it's a DIR
    if(child->type == 'D'){  
        printf("---------ERROR---------\nNode with dname exists but is actually a directory");
        return ERROR_CODE;
    }
    
    // Check if DIR is empty (can't rmdir if empty)
    
    if(child->child != NULL)
    {
        printf("---------ERROR---------\n%s must be empty before it can be removed\n", bname);
        return ERROR_CODE;
    }

    // 4. Delete node from parent's child list
    // passed all safeguards, delete bname dir
    NODE *pHead = makeNode();
    NODE *pTemp = makeNode();
    NODE *pPrev = makeNode();

    pTemp = dirNode->child;
    int curPos = 0;

    // delete first node
    if(pTemp != NULL && strcmp(pTemp->name, bname) == 0)
    {
        printf("Deleting first child\n");
        pHead->parent = dirNode; // set correct parents
        pHead = pTemp->sibling; // advance pHead
        dirNode->child = pHead; // make sure parent points down correctly
        free(pTemp); // delete old head
        return SUCCESS_CODE;
    }

    // traverse list
    while(pTemp != NULL && strcmp(pTemp->name, bname) != 0)
    {
        pPrev = pTemp;
        pTemp = pTemp->sibling;
        curPos++;
    }

    if(pTemp == NULL)
    {
        printf("---------ERROR---------\nNode to delete was not found");
        return ERROR_CODE;
    }
    printf("Deleted node at position %d in the child linked list\n", curPos);
    
    pPrev->sibling = pTemp->sibling; // now, no nodes point to pTemp
    
    free(pTemp); // free pTemp
    return SUCCESS_CODE;
}

int saveHelper(NODE *pCur)
{
    if(pCur == NULL){
        return SUCCESS_CODE;
    } else {
        //printf("pCur->name: %s\n", pCur->name);
        
        // preorder traversal
        if(strcmp(pCur->name, "/") != 0) {
            cwd = pCur;
            pwd(); // build up CWD
            fprintf(fp, "%c,%s,\n", pCur->type, CWD);
        } else {
            // print root
            fprintf(fp, "%c,%s\n", pCur->type, pCur->name);
        }
        saveHelper(pCur->child);
        if(strcmp(pCur->name, "/") != 0) {
            saveHelper(pCur->sibling);
        }
    }
}

// Write the absolute pathnames of the tree as text to a file
int save()
{
    if(root->child == NULL){
        printf("There's nothing in the tree! Not writing to outfile\n");
        return SUCCESS_CODE;
    } else {
        // note: only absolute pathnames are needed, so even easier
        fp = fopen("myfile.txt", "w+");
        //printf("%s %s\n", root->name, root->child->name);
        fprintf(fp, "type    pathname\n");
        fprintf(fp, "----    --------\n");
        
        saveHelper(root);

        fclose(fp);
        return SUCCESS_CODE;
    }
}

// Constructs BST from the contents of the file
int reload()
{
    char line[64];
    fp = fopen("myfile.txt", "r");
    if(fp == NULL){
        printf("ERROR OPENING FILE\n");
        return EXIT_CODE;
    }


    // get first three lines
    fgets(line, 64, fp);
    fgets(line, 64, fp); 

    while(fgets(line, 64, fp) != NULL){
        fgets(line, 64, fp);
        char *token;
        
        token = strtok(line, ",");
        char type = token[0];
        
        if(type == 'D'){
            token = strtok(NULL, ",");
            
            token[strlen(token) - 1] = '\0';
            
            printf("token: %s\n", token);
            mkdir(token);
        } else if(type == 'F'){
            token = strtok(NULL, ",");
            token[strlen(token) - 1] = '\0'; // strip last character

            creat(token);
        }
    }
    
    
}

// User instructions
int menu(char *pathame)
{
    printf("VALID COMMANDS\n");
    printf("\tmkdir pathname: make a new directory for a given pathname\n");
    printf("\trmdir pathname: remove the directory, if it is empty\n");
    printf("\tcd [pathname]: change CWD to pathname, or to / if no pathname\n");
    printf("\tls [pathname]: list the directory contents of pathname or CWD\n");
    printf("\tpwd: print the (absolute) pathname of CWD\n");
    printf("\tcreat pathname: create a FILE node\n");
    printf("\trm pathname: remove the FILE node\n");
    printf("\tsave filename: save the current file tree as a file\n");
    printf("\treload filename: construct a file system tree from a file\n");
    printf("\tmenu: show a menu of valid commands\n");
    printf("\tquit: save the file system tree, then terminate the program\n");
}

// Saves tree to the file and terminates program
int quit(char *pathname){
    printf("SAVING CURRENT TREE\n");
    save();
    printf("CLOSING TERMINAL\n");
    return EXIT_CODE;
}

int findCmd(char *command)
{
    
    int i = 0;
    while(cmd[i])
    {
        if (!strcmp(command, cmd[i])){
            return i;
        }
        i++; // found command: return index i
    }
    printf("-------------ERROR-------------\n\tInvalid command\n");
    return -1;
}

int (*fptr[])(char *) = { (int (*)()) mkdir, rmdir, ls, cd, pwd, creat, rm, save, reload, menu, quit};

int main(int argc, char *argv[ ], char *env[ ])
{
    printf("ENTERED EDGAR'S UNIX TREE FILE SIMULATOR TERMINAL\n");
    int result = -1;
    
    // initialize root
    root = makeNode();
    root->parent = root;
    root->sibling = root;

    strcpy(root->name, "/"); 
    root->type = 'D';   

    // initialize current working directory
    cwd = makeNode(); 
    cwd = root;

    strcpy(CWD, "");

    while(result != EXIT_CODE){
        fgets(line, 128, stdin);  // get at most 128 chars from stdin
        line[strlen(line)-1] = 0; // remove \n from end of line
        sscanf(line, "%s %s", command, pathname);
        result = findCmd(command);
        
        // safeguard against invalid commands
        if(result != -1){
            result = fptr[result](pathname);
            printf("\n");
        }
    }

    
}