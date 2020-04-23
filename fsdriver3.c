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
#include "fsLow.c"
#include "FileSystem.h"
#include "Directory.h"
#include <time.h>
#include <assert.h>

#define AVGDIRECTORYENTRIES 50
#define EXIT_SUCCESS 0
#define MAXCOMMLIST 5
#define BUFFER_LENGTH 256
#define BLOCKSIZE 512
#define DIR_USED 0x00000001
#define DIR_UNUSED 0x00000002
#define DIR_SPECIAL 0x00000004


//void mkdir(char *);
void loop();
void addFile(char *, char*);
void initRoot(uint64_t, uint64_t);
openFileEntry getFile(int);
int getFileId();

typedef struct openFileEntry {
    int flags;
    uint64_t size;
    uint64_t pointer;
    uint64_t Id;
    uint64_t position;
    uint64_t offset;
    char* filebuffer;
}openFileEntry, *openFileEntry_ptr;

openFileEntry * openFileList;

void initRoot(uint64_t startLocation, uint64_t blockSize){
    dirEntry_p rootDirPtr;

    uint64_t entrySize = sizeof(dirEntry);
    uint64_t bytesNeeded = AVGDIRECTORYENTRIES * entrySize;
    uint64_t blocksNeeded = (bytesNeeded + (blockSize - 1)) / blockSize;
    uint64_t actualDirEntries = (blocksNeeded * blockSize) / entrySize;

    printf("For %d entries. We need %llu bytes. Each entry is %llu bytes \n",
            AVGDIRECTORYENTRIES, bytesNeeded, entrySize);
    rootDirPtr = malloc(blocksNeeded * blockSize);


    for (int i = 0; i < actualDirEntries; i++){
        strcpy(rootDirPtr[i].name, "");
        rootDirPtr[i].id = 1000;
        rootDirPtr[i].date = 0;
        rootDirPtr[i].location = 0;
        rootDirPtr[i].sizeInBytes = 0;
        rootDirPtr[i].flags = DIR_UNUSED;
    }
    strcpy(rootDirPtr[0].name, "..");
    rootDirPtr[0].id = 1000;
    rootDirPtr[0].date = 0;
    rootDirPtr[0].location = 0;
    rootDirPtr[0].sizeInBytes = 0;
    rootDirPtr[0].flags = DIR_UNUSED;

    LBAwrite(rootDirPtr, blocksNeeded, startLocation);

}


uint64_t writeFile(int fd, char *source, uint64_t length){
//    fd = destination
//    source = file we are writing from
//    fd checks -

    uint64_t currentBlock = openFileList[fd].position / BLOCKSIZE;
    uint64_t currentOffset = openFileList[fd].offset % BLOCKSIZE;

//    memcpy initilizes the buffer
    if (length + currentOffset < BLOCKSIZE){  //can it fit in one block?
//      memcpy(param1 dest, param2 source, param3 size)
        memcpy(openFileList[fd].filebuffer + currentOffset, source, length);
    }
    else if (length + currentOffset < (BLOCKSIZE * 2)){   //does it fit in 2 blocks?
        memcpy(openFileList[fd].filebuffer + currentOffset, source, length);

//      Check to see if Write or read is beyond the capacity of the volume
//      uint64_t LBAwrite (void * buffer, uint64_t lbaCount, uint64_t lbaPosition)(
        LBAwrite(openFileList[fd].filebuffer + currentOffset, source, length);
    } else {  //if it spills over into 3 blocks
//        Memcopy arguments = source, fd, length, currentOffset
//        LBAWrite = fileBuffer, blockStart
//        Memcopy
    }
}

openFileEntry getFile(int fd){
//    NTK what file we are reading from. Need to go through all structs(files)
//    in order to find the file that we are reading from
//    uint64_t currentBlock = openFileList[fd].position / BLOCKSIZE;
//    uint64_t currentOffset = openFileList[fd].offset % BLOCKSIZE;
//
//    char *readContent;
    return openFileList[fd];
}

int main(int argc, char *argv[])
{
    //when we run the driver we specify the volume/root directory name, volume size and block size.
    char *filename;
    uint64_t volumeSize;
    uint64_t blockSize;
    int retVal;
    openFileEntry openFileList[50];

    if (argc > 3)
    {
        filename = argv[1];
        volumeSize = atoll(argv[2]);
        blockSize = atoll(argv[3]);
    }
    //start partition system creates our volume
    retVal = startPartitionSystem (filename, &volumeSize, &blockSize);
    printf("Opened %s, Volume Size: %llu;  BlockSize: %llu; Return %d\n", filename, (ull_t)volumeSize, (ull_t)blockSize, retVal);

    initRoot(0, blockSize);


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
//        char * tempLine;
//        strcpy(tempLine, line);
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
//        command[counter] = '\0';
        printf("checking command\n");
        if (strcmp(command[0], "Q") == 0|| strcmp(command[0], "q") == 0)
        {
            printf("Exiting\n");
            stat = 0;
        }
        else if (strcasecmp(command[0], "help") == 0)
        {
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
        }
        else if (strcmp(command[0], "mkdir") == 0)
        {
            mkdir(fsystem);
        }
        else if (strcmp(command[0], "cp") == 0)
        {
            printf("copying\n");
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
        else if(strcmp(command[0], "read") == 0)
        {
            printf("read\n");
            char currWD[PATHMAX];
            char * filename = command[1];
            int fd;
            fd = open(filename, "w");
            getcwd(currWD, sizeof(currWD));


        }
        else if(strcmp(command[0], "write") == 0)
        {
            printf("write\n");
            char currWD[PATHMAX];
            char * filename = command[1];
//      write filename "adsf asdf as asdf"
            int fd;
            fd = open(filename, "w");
            getcwd(currWD, sizeof(currWD));

            size_t n = sizeof(command)/sizeof(command[0]);
            char* source = malloc(sizeof(char) * 256);
            char str[] = "";
            printf("This is size of counter %d\n", counter);


            for (int i = 2; i < counter; i++){
                printf("this is command %s\n", command[i]);

                 strcat(source, command[i]);
                 if((i + 1) != counter){
                     strcat(source, " ");
                 }
                 printf("this is the string %s\n", source);

            }

            if (fd != NULL){
                writeFile(fd, source, sizeof(source)/sizeof(char) + 1);
            } else {
                printf("File could not be opened\n");
            }
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