//
// Created by Matthew Stephens on 4/9/20.
//

//#include "fsdriver3.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <stdint.h>
#include "copymove.c"
#include "fsLow.h"
#include "FileSystem.h"
#include <time.h>
#include <assert.h>

#define EXIT_SUCCESS 0
#define MAXCOMMLIST 5
#define BUFFER_LENGTH 256

void mkdir(char *);
void loop();
void addFile(char *, char*);

int main(int argc, char *argv[])
{
    //when we run the driver we specify the volume/root directory name, volume size and block size.
    char *filename;
    uint64_t volumeSize;
    uint64_t blockSize;
    int retVal;

    if (argc > 3)
    {
        filename = argv[1];
        volumeSize = atoll(argv[2]);
        blockSize = atoll(argv[3]);
    }
    //start partition system creates our volume
    //retVal = startPartitionSystem (filename, &volumeSize, &blockSize);
    printf("Opened %s, Volume Size: %llu;  BlockSize: %llu; Return %d\n", filename, (ull_t)volumeSize, (ull_t)blockSize, retVal);

    //this is where we will accept user input and call functions
    loop();

    return EXIT_SUCCESS;
}

void loop()
{
    int stat = 1,   //status?
    counter = 0;     //counter
    char *line = malloc(BUFFER_LENGTH * sizeof(char)) , //for holding line
    *args,  //?
    *token, //for tokenizer
    *fsystem, //name of filesystem
    **command; //command & arg list
    struct Directory* entry;

    printf("_________         ___.    .__                  __   \n");
    printf("\\_   ___ \\ _____  \\_ |__  |__|  ____    ____ _/  |_ \n");
    printf("/    \\  \\/ \\__  \\  | __ \\ |  | /    \\ _/ __ \\\\   __\\\n");
    printf(" \\     \\____ / __ \\_| \\_\\ \\|  ||   |  \\\\  ___/ |  |  \n");
    printf(" \\______  /(____  /|___  /|__||___|  / \\___  >|__|  \n");
    printf("        \\/      \\/     \\/          \\/      \\/       \n");




    while(stat)
    {
        printf("enter command");
        command = malloc(MAXCOMMLIST * sizeof(char));
        if(command == NULL)
            printf("could not allocate space\n");
        for(int i = 0; i < MAXCOMMLIST; i++)
        {
            command[i] = malloc(BUFFER_LENGTH * sizeof(char));
            if(command[i] == NULL)
                printf("could not allocate space\n");
        }
        printf("> ");

        //make a little loop to accept user input
        fgets(line, BUFFER_LENGTH, stdin);
        printf("%s\n", line);
        token = strtok(line, " \n");
        printf("got token\n");
        while(token == NULL)
        {
            printf(">");
            fgets(line, BUFFER_LENGTH, stdin);      //gets line since old one was empty
            token = strtok(line, " \n");
        }
        printf("tokenizing\n");
        while(token != NULL)
        {
            strcpy(command[counter],token);
            printf("command:%s:%i\n",command[counter], counter);
            token = strtok(NULL, " \n\t");
            counter++;
        }
        command[counter] = '\0';
        printf("checking command\n");
        if (strcmp(command[0], "Q") == 0|| strcmp(command[0], "q") == 0)
        {
            printf("Exiting\n");
            stat = 0;
        }
        else if (strcasecmp(token, "help") == 0)
        {
            printf("\n");
            printf("Q - exit program\n");
            printf("cd - change directory\n");

            printf("mkdir - make a directory\n");
            printf("touch - make a file\n");
            printf("move = move a file\n");

            printf("rm - remove a file/directory\n");
            printf("copy - copy a file/directory from one path to other\n");

            printf("\n");
        }
        else if (strcmp(command[0], "mkdir") == 0)
        {
            mkdir(fsystem);
        }
        else if (strcmp(command[0], "copy") == 0)
        {
            printf("copy\n");
//            copy(command[1], command[2], entry);
        }
        else if(strcmp(command[0], "move") == 0)
        {
            printf("move\n");
        }
        else if(strcmp(command[0], "touch") == 0)
        {
            printf("add\n");
            addFile(command[1], command[2]);
            char *name = strtok(NULL, " ");
            char *sizec = strtok(NULL, " ");
            if (name == NULL || sizec == NULL)
            {
                printf("format: Name size in LBA\n");
            }
            else
            {
                uint64_t size = strtoull(sizec, NULL, 0);
                addFile(name, size);
            }
            addFile(command[1], command[2]);

        }
        else if(strcmp(command[0], "rm") == 0)
        {
            printf("remove\n");
        }
        else
        {
            printf("Command not found.\n");
        }

        //switch/if statements to call subroutines based off that command
        counter = 0;
    }
    free(command);
    return;
}

void addFile(char* filename, char* filelocation){
//    strcpy(char* name, const char* filename);
    struct Directory *addFile = NULL;
    addFile = malloc(sizeof(addFile));
    if (strcmp(filelocation, "root") == 0 ){
        addFile->parent = 1;
    }
    strcpy(addFile->name, filename);
    printf("%s\n", addFile->name);

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char s[64];
    assert(strftime(s, sizeof(s), "%c", tm));
    addFile->dateCreated = s;
    addFile->dateModified = s;
    printf("%s\n", addFile->dateCreated);
    printf("%s\n", addFile->dateModified);
    free(addFile);
}


void mkdir(char* name){
    struct Directory* dir = malloc(sizeof(struct Directory));
}