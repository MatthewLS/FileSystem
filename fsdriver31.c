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

typedef struct fsStruct {
    int counter;
    int size;
    char spacer[256 - (sizeof(int) * 2) ],
    name[];
} fsStruct, *fsStructPtr;

void fsRead();

int fsWrite(char *, uint64_t);

void loop(uint64_t);

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

    loop(blockSize);
    printf("out of loop. About to close partition\n");
    closePartitionSystem();
    printf("partition closed");
    return EXIT_SUCCESS;
}

void loop(uint64_t blockSize) {
    int stat = 1,   //status?
        retVal,     //return value
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
        printf("line: %s\n", line);
        char *tempLine = malloc(BUFFER_LENGTH * sizeof(char));
        strcpy(tempLine, line);
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
            fsRead(blockSize);

        } else if (strcmp(command[0], "write") == 0) {
            printf("write\n");
            fsWrite(tempLine, blockSize);

            //      if (fd == NULL) {
            //        printf("File could not be opened\n");
            //  } else {
            //writeFile(fd, source, sizeof(source)/sizeof(char) + 1);

        } else {
            printf("Command not found.\n");
        }
        counter = 0;
    }
    //switch/if statements to call subroutines based off that command

    free(command);
    free(line);
    return;
}

void fsRead(uint64_t blockSize)
{
    fsStructPtr pBuf = malloc (blockSize * 2);
    int retVal = LBAread(pBuf, 2, 12);
    printf("text:%s\n", pBuf->name);
    free(pBuf);

}

int fsWrite(char *line, uint64_t blockSize) {
    fsStructPtr pBuf = malloc(blockSize * 2);
    char *textToWrite,  //text that is going to be written
            *token, //for tokenizer
            *copiedText = malloc(BUFFSIZE * sizeof(char));  //temporary copied text
    int retVal; //return value

    printf("in write function \n");
    printf("line:%s\n", line);
    token = strtok(line, "\""); //tokenizer
    printf("got token\n");
    while (token == NULL) { //empty case
        printf(">");
        fgets(line, BUFFER_LENGTH, stdin);      //gets line since old one was empty
        token = strtok(line, " \n");
    }
    printf("tokenizing\n");
    while (token != NULL) {
        //printf("token:%s\n", token);
        strcpy(copiedText, token);
        token = strtok(NULL, "\"\n");
    }
    printf("Text that will be written is:%s\n", copiedText);
    pBuf->counter = 103958; //counter?
    pBuf->size = 85775875;  //size?
//    textToWrite = (char *) pBuf;    //?
//    textToWrite = textToWrite + 256;    //allocate space?
    printf("before copy to text\n");
    //strcpy(textToWrite, copiedText);    //copies string of text
    printf("after copy to text\n");
    //printf("the text is:%s\n",textToWrite);
    strcpy(pBuf->name, copiedText); //copies text to buffer
    //pBuf->name = textToWrite;   //
    retVal = LBAwrite(pBuf, 2, 12);

    free(copiedText);
    return retVal;
}