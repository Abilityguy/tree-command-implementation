/*
File: main.c
Date: 10th November, 2019
Authors: 
Pradyumna Y M   PES1201700986
Anush V Kini    PES1201701646
Sushanth Jain   PES1201700992
*/


#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct Node
{
    char *name;
    struct Node **children;
    int numents;
    int type;
    struct timespec last_mod;
} Node;

/*
Description: concatenate two strings in a new string, with new memory (To append two paths to get the absolute path)
Parameters: s1: string - string1 to be concatenated
            s2: string - string2 to be concatenated
Returns: Char pointer - a pointer to the new string, which is a concatenation of the two strings
Assumptions: N/A
*/

char *concat(char *s1, char *s2)
{
    char *ns = malloc(strlen(s1) + strlen(s2) + 1);
    ns[0] = '\0';
    strcat(ns, s1);
    strcat(ns, s2);
    return ns;
}

/*
Description: creates a copy of a string
Parameters: s1: string - string to be hard copied
Returns: Char pointer - pointer to the new string
Assumptions: N/A
*/

char *getcopy(char *s1)
{
    char *ns = malloc(strlen(s1) + 1);
    ns[0] = '\0';
    strcat(ns, s1);
    return ns;
}

/*
Description: allocate memory for a new node and initialize values, and returns a pointer.
Parameters: name: string - Name of the node (file or directory), 
            numents: integer - number of entries in the children array (The number of subfiles and directories),
            type: integer - type of the node - Can take 3 values: (0: Directory, 1: File, 2: Symbolic Link)
Returns:    Node pointer - pointer to the new node created
Assumptions: N/A
*/

Node *getNode(char *name, int numents, int type)
{

    //allocate memory to the new node
    Node *newnode = malloc(sizeof(Node));
    //reduce the numents to remove the trivial nodes(. and ..)
    numents -= 2;

    //allocate pointers to the children
    newnode->children = malloc(sizeof(Node *) * numents);

    //make the child pointers null
    for (int i = 0; i < numents; i++)
        newnode->children[i] = 0;

    //initialise values of the node
    newnode->name = getcopy(name);
    //printf("in getnode %s\n", name);
    newnode->numents = numents;
    newnode->type = type;
    newnode->numents = newnode->numents < 0 ? 0 : newnode->numents;
    return newnode;
}

/*
Description: Given a path to a directory, find the number of entries in the directory 
             (The number of entries refers to subfiles or subdirectories in a given directory)
Parameters: path: string - path to the directory
Returns: integer - number of entries in the directory
Assumptions: N/A
*/

int numberofentries(char *path)
{

    //open the directory
    DIR *dp = opendir(path); //path here later
    struct dirent *d;
    int nument = 0;

    //increment the number of entries for each non-null node in the directory
    while ((d = readdir(dp)) != NULL)
    {
        nument++;
    }

    //close the directory and return the number of entries
    closedir(dp);
    return nument;
}

/*
Description: comparator function, compares two nodes based on various conditions
Parameters:
n1: Node* pointer - points to the first node
n2- Node* pointere  - points to the second node
order - an integer which defines the order in which the list should be sorted
Returns: 1 if n1>n2 else -1
Assumptions: N/A
*/

int compare(Node *n1, Node *n2, int order)
{

    //self explanatory code..
    switch (order)
    {
    case 0:
        if (n1->last_mod.tv_nsec >= n2->last_mod.tv_nsec)
            return 1;
        else
            return -1;
        break;
    case 1:
        if (n1->last_mod.tv_nsec <= n2->last_mod.tv_nsec)
            return 1;
        else
            return -1;
        break;
    case 2:
        if (strcmp(n1->name, n2->name) >= 0)
            return 1;
        else
            return -1;
        break;
    case 3:
        if (strcmp(n1->name, n2->name) <= 0)
            return 1;
        else
            return -1;
        break;
    }
}

/*
Description: populates the tree based on the directory structure
Parameters:
path: string - contains the path of the directory 
node: Node* pointer - a pointer to the Node in the directory tree
order: defines the ordering to be followed while enumerating the list

Returns: N/A

Assumptions: N/A
*/

void populate(char *path, Node *node, int order)
{


    //variable declarations
    int i = 0;
    DIR *dp = opendir(path); //open the directory at path
    struct dirent *d;
    struct stat statistics;
    Node *temp;

    //loop through the contents of the directory
    while ((d = readdir(dp)) != NULL)
    {

        //skip the trivial contents of the directory
        if ((strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0))
            continue;

        //define the path to the child node
        char *newpath = concat(path, concat("/", d->d_name));

        //check if it is a directory, if yes, create a new node and perform a recursive call
        if (d->d_type == DT_DIR)
        {
            node->children[i] = getNode(d->d_name, numberofentries(newpath), 0);
            populate(newpath, node->children[i], order);
            //printf("here %s\n", newpath);
            fflush(stdout);
        }
        else if (d->d_type == DT_LNK)
            node->children[i] = getNode(d->d_name, 0, 2);
        else
            node->children[i] = getNode(d->d_name, 0, 1);

        //get the stats of the node
        stat(newpath, &statistics);
        
        //record the last modified time of the node
        node->children[i]->last_mod = statistics.st_mtim;
        i++;
    }

    //sort the children nodes based on a particular order
    for (i = 0; i < node->numents; i++)
    {
        for (int j = 0; j < node->numents - i - 1; j++)
        {
            if (compare(node->children[j], node->children[j + 1], order) == 1)
            {
                temp = node->children[j];
                node->children[j] = node->children[j + 1];
                node->children[j + 1] = temp;
            }
        }
    }

    closedir(dp);
}

/*
Description: prints the name of a node in the appropriate color based on the type of node(dir, file, symlink)
Parameters:
node: Node* pointer - a pointer to the node in the directory tree
Returns: N/A
Assumptions: N/A
*/

void print(Node *node)
{

    //check the type of node and print it in the correct format
    switch (node->type)
    {
    case 2:

        //flag for printf to print it in a particular color
        printf("\033[1;31m");
        printf("%s\n", node->name);
        //flag to reset the output format to normal
        printf("\033[0m");
        break;

    case 0:

        //flag for printf to print it in a particular color
        printf("\033[01;33m");
        printf("%s\n", node->name);
        printf("\033[0m");
        break;

    case 1:

        //flag for printf to print it in a particular color
        printf("\033[1;32m");
        printf("%s\n", node->name);
        printf("\033[0m");
        break;
    }
}

/*
Description: Prints the directory tree
Parameters:
node: Node* pointer, points to the root of the directory tree
depth: integer pointer used for formatting the output
printcount: integer pointer used to control the output
Returns: N/A
Assumptions: N/A
*/

void inorder_print(Node *node, int *depth, int *printcount)
{

    //if the node is a NULL return control
    if (node == NULL)
        return;

    //stop printing if n lines are already printed.
    if (*printcount == 10)
    {
        char c = getchar();
        if (c == 'q')
            exit(0);
        *printcount = 0;
    }

    //print \t's for formatting the output
    for (int i = 0; i < *depth - 1; i++)
    {
        printf("\t");
    }
    if (*depth != 0)
        printf("|-------");

    (*printcount)++;

    //call the print function to print it in the appropriate format
    print(node);
    fflush(stdout);

    //print the child nodes
    for (int i = 0; i < node->numents; i++)
    {
        (*depth)++;
        inorder_print(node->children[i], depth, printcount);
        (*depth)--;
    }
}


int main(int argc, char **argv)
{
    int i = 0;
    int depth = 0, printcount = 0;
    char path[2000];
    int order = 0;
    if (argc == 2)
    {
        if (!strcmp(argv[1], "ma"))
            order = 0;
        else if (!strcmp(argv[1], "md"))
            order = 1;
        else if (!strcmp(argv[1], "na"))
            order = 2;
        else if (!strcmp(argv[1], "na"))
            order = 3;
        else
        {
            printf("Please check the paramters (na - name asc, nd - name desc, ma - last modified asc, md - last modified dsc)\n");
            exit(0);
        }
    }
    scanf("%s", path);
    getchar();
    Node *a = getNode(path, numberofentries(path), 0);
    populate(path, a, order);
    inorder_print(a, &depth, &printcount);
}
