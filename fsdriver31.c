//
// Created by student on 4/22/20.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "copymove.c"
#include "fsLow.c"
#include "Directory.h"

#define AVGDIRECTORYENTRIES 50
#define EXIT_SUCCESS 0
#define MAXCOMMLIST 5
#define BUFFER_LENGTH 256
#define BLOCKSIZE 512
#define DIR_USED 0x00000001
#define DIR_UNUSED 0x00000002
#define DIR_SPECIAL 0x00000004

typedef struct fileStruct {
    int counter;
    int size;
    char *name;
} fileStruct, *fileStructPtr;

void fsRead();
void fsWrite();
void loop();

int main(int argc, char *argv[]) {
    char *fileName;
    uint64_t volumeSize;
    uint64_t blockSize;
    int retVal;

    if (argc > 3) {
        fileName = argv[1];
        volumeSize = atoll(argv[2]);
        blockSize = atoll(argv[3]);
    }
    retVal = startPartitionSystem(fileName, &volumeSize, &blockSize);

    loop();
    return EXIT_SUCCESS;
}

void loop() {
    int stat = 1,   //status?
    counter = 0;     //counter
    char *line = malloc(BUFFER_LENGTH * sizeof(char)), //for holding line
    *args,  //?
    *token, //for tokenizer
    *fsystem, //name of filesystem
    **command; //command & arg list
    struct Directory *entry;

    printf("_________         ___.    .__                  __   \n");
    printf("\\_   ___ \\ _____  \\_ |__  |__|  ____    ____ _/  |_ \n");
    printf("/    \\  \\/ \\__  \\  | __ \\ |  | /    \\ _/ __ \\\\   __\\\n");
    printf(" \\     \\____ / __ \\_| \\_\\ \\|  ||   |  \\\\  ___/ |  |  \n");
    printf(" \\______  /(____  /|___  /|__||___|  / \\___  >|__|  \n");
    printf("        \\/      \\/     \\/          \\/      \\/       \n");


    while (stat) {
        printf("enter command");
        command = malloc(MAXCOMMLIST * sizeof(char));
        if (command == NULL)
            printf("could not allocate space\n");
        for (int i = 0; i < MAXCOMMLIST; i++) {
            command[i] = malloc(BUFFER_LENGTH * sizeof(char));
            if (command[i] == NULL)
                printf("could not allocate space\n");
        }
        printf("> ");

        //make a little loop to accept user input
        fgets(line, BUFFER_LENGTH, stdin);
        printf("%s\n", line);
//        char * tempLine;
//        strcpy(tempLine, line);
        token = strtok(line, " \n");
        printf("got token\n");
        while (token == NULL) {
            printf(">");
            fgets(line, BUFFER_LENGTH, stdin);      //gets line since old one was empty
            token = strtok(line, " \n");
        }
        printf("tokenizing\n");
        while (token != NULL) {
            strcpy(command[counter], token);
            printf("command:%s:%i\n", command[counter], counter);
            token = strtok(NULL, " \n\t");
            counter++;
        }
//        command[counter] = '\0';
        printf("checking command\n");
        if (strcmp(command[0], "Q") == 0 || strcmp(command[0], "q") == 0) {
            printf("Exiting\n");
            stat = 0;
        } else if (strcasecmp(command[0], "help") == 0) {
            printf("\n");
            printf("Q - exit program\n");
            printf("cd - change directory\n");

            printf("mkdir - make a directory\n");
//          Need to open file for cp, move, read, and write
            printf("touch - make a file\n");
            printf("move - move a file\n");
            printf("read - reads from a file ");
            printf("write - writes to a file");

            printf("rm - remove a file/directory\n");
            printf("cp - copy a file/directory from one path to other\n");

            printf("\n");
        } else if (strcmp(command[0], "mkdir") == 0) {
            //mkdir(fsystem);
        } else if (strcmp(command[0], "cp") == 0) {
            printf("copying\n");
//            copy(command[1], command[2], entry);
        } else if (strcmp(command[0], "move") == 0) {
            printf("move\n");
        } else if (strcmp(command[0], "touch") == 0) {
            printf("add\n");
//            addFile(command[1], command[2]);
//            char *name = strtok(NULL, " ");
//            char *sizec = strtok(NULL, " ");
//            if (name == NULL || sizec == NULL)
//            {
//                printf("format: Name size in LBA\n");
//            }
//            else
//            {
//                uint64_t size = strtoull(sizec, NULL, 0);
//                //addFile(name, size);
//            }
//            //  addFile(command[1], command[2]);

        } else if (strcmp(command[0], "rm") == 0) {
            printf("remove\n");
        } else if (strcmp(command[0], "read") == 0) {
            printf("read\n");
//            char currWD[PATHMAX];
//            char * filename = command[1];
//            FILE* fd;
//            fd = fopen(filename, "w");
//            getcwd(currWD, sizeof(currWD));


        } else if (strcmp(command[0], "write") == 0) {
            printf("write\n");
//            char currWD[PATHMAX];
//            char * filename = command[1];
////      write filename "adsf asdf as asdf"
//            FILE* fd;
//            fd = fopen(filename, "w");
//            getcwd(currWD, sizeof(currWD));
//
//            size_t n = sizeof(command)/sizeof(command[0]);
//            char* source = malloc(sizeof(char) * 256);
//            char str[] = "";
//            printf("This is size of counter %d\n", counter);
//
//
//            for (int i = 2; i < counter; i++){
//                printf("this is command %s\n", command[i]);
//
//                strcat(source, command[i]);
//                if((i + 1) != counter){
//                    strcat(source, " ");
//                }
//                printf("this is the string %s\n", source);

//        }

            //      if (fd == NULL) {
            //        printf("File could not be opened\n");
            //  } else {
            //writeFile(fd, source, sizeof(source)/sizeof(char) + 1);

        } else {
            printf("Command not found.\n");
        }
    }
    //switch/if statements to call subroutines based off that command
    counter = 0;
    free(command);
    return;
}

void fsRead()
{

}

void fsWrite()
{

}