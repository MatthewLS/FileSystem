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
void addFile(char *);

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
            addFile(command[1]);
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

void addFile(char* filename){
//    strcpy(char* name, const char* filename);
    printf("yee\n");
    struct Directory* yee;
    strcpy(yee->name, filename);
    printf("%s\n", yee->name);

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char s[64];
    assert(strftime(s, sizeof(s), "%c", tm));
    yee->dateCreated = s;
    yee->dateModified = s;
    printf("%s\n", yee->dateCreated);
    printf("%s\n", yee->dateModified);
//    time_t t = time(NULL);
//    char *ctime(const time_t *timer);
//    yee->dateCreated = *ctime;
//    printf("%s\n", yee->dateCreated);
//    printf("now: %d-%02d-%02d %02d:%02d:%02d\n",
//            yee->dateCreate.tm_year + 1900,
//            yee->dateCreate.tm_mon + 1,
//            yee->dateCreate.tm_mday,
//            yee->dateCreate.tm_hour,
//            yee->dateCreate.tm_min,
//            yee->dateCreate.tm_sec);

}


void mkdir(char* name){
    struct Directory* dir = malloc(sizeof(struct Directory));
}