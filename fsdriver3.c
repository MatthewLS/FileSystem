//
// Created by student on 4/22/20.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include "copymove.c"
#include "fsLow.h"
#include "stack.c"
#include "hashImplement.c"
#include <limits.h>
#include <stdint.h>
#include "fsLow.h"
#include "fsLow.c"
//#include "Directory.h"
#include <time.h>
#include <assert.h>

#define AVGDIRECTORYENTRIES 50
#define EXIT_SUCCESS 0
#define MAXCOMMLIST 5
#define BUFFER_LENGTH 5120
#define NAME_LENGTH 128
#define CONTIGUOUS 1
#define NOTCONTIGUOUS 0
#define BLOCKSIZE 512
#define DIR_FILE 0x00000001 //last will be 1 if a file and 0 if directory
#define DIR_UNUSED 0x00000002 //will be 1 if used, 0 if unused
#define DIR_SPECIAL 0x00000004
#define FILEIDINCREMENT 17

#define FDOPENINUSE 0x00000001
#define FDOPENFREE 0x00000002
#define FDOPENMAX 50

#define FDOPENFORWRITE 0x00000010
#define FDOPENFORREAD 0x00000020

#define MYSEEK_CUR 1
#define MYSEEK_POS 2
#define MYSEEK_END 3


int currentDir = 0;



typedef struct rootDir {
    uint64_t id,    //dateModified for ...
    date,       //date
    location,   //what block entry is in the disk(lba sart)
    sizeinBytes;    //file size. blocks needed = (sizeinBytes + (blocksize - 1)) / blocksize
    uint32_t flags;
    int numOfChildren;
    char name[NAME_LENGTH]; //file name limited to 128 char
    int dirChildren[];
} rootDir, *rootDirPtr;

typedef struct OpenFileEntry {
    int flags;  //1 == dir. 0 == file
    uint64_t bytesFromStart,   //where you are in file
    size,   //size of file(in bytes)
    dateModified,
            dateCreated,//dateModified of file
    usedBlocks[0]; //start block
    char *fileBuffer;   //file buffer(contents?)
    int parentId;
    int dirChildren[AVGDIRECTORYENTRIES],
            numBlocksUsed;
    int numOfChildren;
    int childElementIndex;
    char *name;
    int isNewFile;   //1 == should be new file 0 == should overwrite
} openFileEntry, *openFileEntryPtr;

openFileEntry *openFileList;   //array of currently open files

typedef struct vcbStruct {
    uint64_t volSize,
            blockSize,
            numBlocks,
            freeBlockLoc,   //free block location
    freeBlockBlocks,
            freeBlockLastAllocBit,
            freeBlockEndBlocksRemaining,
            freeBlockTotalFreeBlocks,
            rootDirStart,
            nextIdToIssue;
    char *freeBuff; //only for memory

} vcbStruct, *vcbStructPtr;

//vcbStructPtr currVCBPtr;
vcbStruct *currVCBPtr;

ht_t *hashTable;            //hash table variable
int latestID = 0;     //counter for # of files/file dateModified's


char *fsRead(int);

void createDir(char *, int);

void changeDirectory(char *);

void listFiles();

void addFile(int);

void moveFile(char *, char *);

void removeFile(char *);

void cptofs(char *, int);

int copyFile(char *, char *);

void loop(uint64_t);

char *initFreeMap(uint64_t, uint64_t, uint64_t);

void flipFreeBlockBit();

void freeMap(uint64_t, uint64_t);

int getFreeSpace(uint64_t, int);

void initRootDir(uint64_t, uint64_t);

uint64_t getNewFileID();

int myFSOPEN(char *, int);

int myFSSeek(int, uint64_t, int);

uint64_t fsWrite(int, char *, uint64_t);

int fileIDCheck(char *);

struct StackNode *freeBlockStack = NULL;

char* getWriteInput(char* line);

int main(int argc, char *argv[]) {
    char *fileName;
    uint64_t volumeSize;
    uint64_t blockSize;
    int retVal;
    currVCBPtr = malloc(sizeof(vcbStruct) * 1);
    currVCBPtr->blockSize = 512;
    openFileList = malloc(sizeof(openFileList) * 256);

    fileName = argv[1];
    volumeSize = 10000000;
    blockSize = 512;
//    strcpy(fileName, "newfs");
    retVal = startPartitionSystem(fileName, &volumeSize, &blockSize);
//    retVal = startPartitionSystem(fileName, &volumeSize, &blockSize);
    currVCBPtr->freeBlockLoc = 1;
    printf("Opened %s, Volume Size: %lu; BlockSize: %lu\n", fileName, volumeSize, blockSize);

//    freeMap(volumeSize, blockSize); look at this later
    //printf("before init\n");
    initRootDir(6, blockSize);
    //printf("after init\n");
    hashTable = ht_create();
    //("after hash\n");
    loop(blockSize);
    //printf("after loop\n");
    //printf("out of loop. About to close partition\n");
    closePartitionSystem();
    printf("partition closed.\n");
    return EXIT_SUCCESS;
}

void loop(uint64_t blockSize) {
    int numCommands = 0;
    int stat = 1,   //status?
    retVal,     //return value
    counter = 0;     //counter
    char *line = malloc(BUFFER_LENGTH * sizeof(char)), //for holding line
    *args,  //?
    *token, //for tokenizer
    *fsystem, //name of filesystem
    **command; //command & arg list
    char *lineForWrite = malloc(BUFFER_LENGTH * sizeof(char)); //for holding line passed to write parser
    struct Directory *entry;

    printf("_________         ___.    .__                  __   \n");
    printf("\\_   ___ \\ _____  \\_ |__  |__|  ____    ____ _/  |_ \n");
    printf("/    \\  \\/ \\__  \\  | __ \\ |  | /    \\ _/ __ \\\\   __\\\n");
    printf("\\     \\____ / __ \\_| \\_\\ \\|  ||   |  \\\\  ___/ |  |  \n");
    printf(" \\______  /(____  /|___  /|__||___|  / \\___  >|__|  \n");
    printf("        \\/      \\/     \\/          \\/      \\/       \n");


    while (stat) {
        if (currentDir == 0) {
            printf(" ~");
        } else {
            printf(" %s", openFileList[currentDir].name);
        }

        command = malloc(BUFFSIZE * sizeof(char));
        if (command == NULL)
            printf("could not allocate space\n");
        for (int i = 0; i < BUFFSIZE; i++) {
            command[i] = malloc(BUFFER_LENGTH * sizeof(char));
            numCommands++;
            if (command[i] == NULL)
                printf("could not allocate space\n");
        }
        printf("> ");

        //make a little loop to accept user input
        fgets(line, BUFFER_LENGTH, stdin);
        strcpy(lineForWrite, line);
        if ((sizeof(line) / sizeof(char) + 1) > BUFFER_LENGTH) {
            printf("over256\n");
            line = realloc(line, BUFFER_LENGTH * 3);
        }

        token = strtok(line, " \n");
        while (token == NULL) {
            printf(">");
            fgets(line, BUFFER_LENGTH, stdin);      //gets line since old one was empty
            token = strtok(line, " \n");
        }
        while (token != NULL) {
            strcpy(command[counter], token);
//            printf("command:%s:%i\n", command[counter], counter);
            token = strtok(NULL, " \n\t");
            counter++;
        }
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
            printf("read - reads from a file\n");
            printf("write - writes to a file\n");
            printf("ls - lists all directory children\n");

            printf("rm - remove a file/directory\n");
            printf("cp - copy a file/directory from one path to other\n");

            printf("cptofs - copy a file from virtual FS to real FS\n");

            printf("\n");
        } else if (strcmp(command[0], "mkdir") == 0) {
            createDir(command[1], fileIDCheck(command[1]));
        } else if (strcmp(command[0], "cd") == 0) {
            changeDirectory(command[1]);
        } else if (strcmp(command[0], "ls") == 0) {
            listFiles();
        } else if (strcmp(command[0], "cp") == 0) {
            copyFile(command[1], command[2]);
        } else if (strcmp(command[0], "move") == 0) {
            moveFile(command[1], command[2]);
        } else if (strcmp(command[0], "touch") == 0) {
            //nt fd = fileIDCheck(command[1]);
            //openFileList[fd].name = command[1];
            //addFile(fd);
            int fd;
            fd = fopen(command[1], "w");
            if (fd == NULL)
                printf("null\n");
            else {
                fd = fileIDCheck(command[1]); //
            }
            //make sure it isnt a dir
            if (openFileList[fd].flags == 1) {
                printf("cannot write a directory\n");
                continue;
            }
            openFileList[fd].name = command[1];
            printf("calling write function now\n");
            fsWrite(fd, "", 1);
        } else if (strcmp(command[0], "rm") == 0) {
            removeFile(command[1]);
        } else if (strcmp(command[0], "read") == 0) {
            fsRead(fileIDCheck(command[1]));

        } else if (strcmp(command[0], "write") == 0) {
            printf("in write function \n");
            int fd;
            fd = fopen(command[1], "w");
            if (fd == NULL)
                printf("null\n");
            else {
                fd = fileIDCheck(command[1]); //
            }
            //make sure it isnt a dir
            if (openFileList[fd].flags == 1) {
                printf("cannot write a directory\n");
                continue;
            }

            openFileList[fd].name = command[1];
            strcpy(lineForWrite, getWriteInput(lineForWrite));
            if(strlen(lineForWrite) == 0){
            printf("Invalid input. Please use write function like: write filename \"inputstring\"\n");
            }
            else{
            fsWrite(fd, lineForWrite, strlen(lineForWrite));
            printf("Called write function with %s\n",lineForWrite);
            printf("Called write function with length %llu\n",strlen(lineForWrite));
            }

        } else if (strcmp(command[0], "cptofs") == 0) {
            printf("copying file from virtual FS to real FS\n");
            cptofs(command[1], ht_get(hashTable, command[1]));

        } else if (strcmp(command[0], "cpfromfs") == 0) {
            printf("copying file from real FS to virtual fs\n");
            FILE *sourceFilePtr = fopen(command[1], "r");
            int fd = fileIDCheck(command[1]);
//            printf("FD IS: &d\n", fd);
            char *fileName;
            char letter;
            char *buffer = malloc(BUFFSIZE * sizeof(char));
            int contentCounter = 0;
            int buffCounter = 0;
            while ((letter = fgetc(sourceFilePtr)) != EOF)    //while letter is not at the end of the file
            {
                buffer[contentCounter] = (char) letter; //copies letter into filecontents
                //printf("%c", (char)fileContents[contentCounter]);
                contentCounter++;             //increase content counter
                buffCounter++;                 //increase buffer counter
                if (buffCounter >= BUFFSIZE) //checks if array is full
                {
                    //printf("\n");
                    //printf("need to realloc\n");
                    buffer = realloc(buffer, BUFFSIZE); //realloc space
                    buffCounter = 0;                                //reset buffer counter
                    if (buffer == NULL)                        //not able to realloc space
                        printf("could not realloc\n");
                }
            }
//            printf("File buffer: %s\n", buffer);


            fsWrite(fd, buffer, sizeof(buffer));
            // free(buffer);
        } else {
            
            printf("Command %s not found.\n", command[0]);
        }
        counter = 0;
    }
    //switch/if statements to call subroutines based off that command

    // for(int i = 0;i<numCommands;i++){
    //     free(command[i]);
    // }

    // free(command);
    // free(line);
    // free(lineForWrite);
    return;
}

void cptofs(char *fileName, int fd) {
    printf("IN COPY TO FS");
    char *fileContents = fsRead(fd);
    char currWD[BUFFER_LENGTH];
    getcwd(currWD, sizeof(currWD));
    printf("Current Working Directory: %s\n", currWD);
    FILE *OutFile = fopen(fileName, "w");
    fprintf(OutFile, "%s", fileContents);
    printf("File written to: %s/%s\n", currWD, fileName);
}

/*  Function reads in data
 *
 *  Parameter: blockSize-- blocksize
 *
 *  No return
 * */
char *fsRead(int fd) {
    printf("IN READ FUNC\n");
    for (int i = 0; i < openFileList[currentDir].numOfChildren;i++){
        if (openFileList[currentDir].dirChildren[i] == fd) {
                printf("IN READ FUNC1\n");

            char *pBuf = malloc(sizeof(char) * 512000);
                printf("IN READ FUNC2\n");

            char *tempBuf = malloc(sizeof(char) * 512000);
    printf("IN READ FUNC3\n");

            for (int i = 0; i < openFileList[fd].numBlocksUsed; i++) {
                LBAread(tempBuf, 1, openFileList[fd].usedBlocks[i]);
                printf("block num: %llu, text: %s\n", openFileList[fd].usedBlocks[i], tempBuf);
            
                strcat(pBuf, tempBuf);
            }
            //int retVal = LBAread(pBuf, 1, openFileList[fd].usedBlocks[0]);
            printf("numblocksused: %llu\n", openFileList[fd].numBlocksUsed);

            printf("text: %s\n", pBuf);
            return pBuf;
        }
    }
    printf("file not found\n");
    return "file not found\n";
}


/*  Function initializes the free map
 *
 *  Parameters: volumeSize-- volume size given
 *  blockSize-- blockSize given
 *  startPos -- where freemap starts
 *
 *  Returns the free buffer
 * */
char *initFreeMap(uint64_t volumeSize, uint64_t blockSize, uint64_t startPos) {
    uint64_t blocks = volumeSize / blockSize, //number of blocks
    freeBytesNeeded = (blocks / 8) + 1, //adds 1 in case bits is not even. 8 bits per byte so needs block/8 bytes
    freeBlocksNeeded;// = (freeBytesNeeded / blockSize) + 1; //looks how many blocks needed for freespace

    if ((freeBytesNeeded % blockSize) == 0) //if bytes needed is even
        freeBytesNeeded = (freeBytesNeeded / blockSize);
    else    //blocks needed isnt even
        freeBytesNeeded = (freeBytesNeeded / blockSize) + 1;   //makes free bytes even

    //printf("Total Blocks");
    char *freeBuffer = (freeBlocksNeeded * blockSize);    //allocate space for FCB
    memset(freeBuffer, 0xFF, freeBlocksNeeded * blockSize);      //initialize FF to free block

    flipFreeBlockBit(freeBuffer, 0, 1); //clears out VCB
    flipFreeBlockBit(freeBuffer, startPos, freeBlocksNeeded); //clear freemap block

    LBAwrite(freeBuffer, freeBlocksNeeded, startPos);   //writes buffer to fs
    return (freeBuffer);
}

/*  Function flips bits to initialize
 *
 *  Parameters: freeBuffer--Buffer to be flipped
 *  start-- start location
 *  count-- how far you go
 *
 *  No return
 * */
void flipFreeBlockBit(char *freeBuffer, uint64_t start, uint64_t count) {
    unsigned char *p, //byte you want to change in the map
    flipper; //initial 1000 0000
    uint64_t byteStart, //where function starts
    bitNum; //bit within the byte

    //Start is block num. 1 to 1 map bit number & divide 8 to determine which byte num to start at
    while (count > 0) {
        byteStart = start / 8;  //start at actual byte
        bitNum = start % 8;     //gets bit num. mod since you want whats left(in case of remainder)
        flipper = 0x80;     //initialize flipping

        if (bitNum > 0)
            flipper = flipper >> bitNum; //shifts bit

        //printf("changing bit #%lu of the %lu in the free block with flipper %i\n", bitNum, byteStart, flipper);

        p = (unsigned char *) freeBuffer + byteStart;    //gets actual byte?
        *p = *p ^ flipper;  //exclusive or bits
        ++start;
        --count;
    }
}

/*  Function starts free map partition
 *
 *  Parameter: volumeSize-- volumeSize given
 *  blockSize-- blockSize given
 *
 * No return value
 * */
void freeMap(uint64_t volumeSize, uint64_t blockSize) {
    char *freeBuffer = initFreeMap(volumeSize, blockSize, 1);
}

/*  Function inits root dir
 *
 *  parameter: startLoc- where directory starts
 *  blockSize- given by other function for (use)?
 *
 *  no return
 */

void createDir(char *dirName, int fd) {
//  file = 0 for flag and dir = 1 for flag
//  make parent id the current working dir
    openFileList[fd].parentId = currentDir;

    //add the current fd to the current directory's children array
    openFileList[currentDir].dirChildren[openFileList[currentDir].numOfChildren] = fd;

    //flag this entry as a directory
    openFileList[fd].flags = 1;
    ht_set(hashTable, dirName, fd);

    openFileList[fd].numOfChildren = 0;
    openFileList[fd].name = dirName;

    openFileList[fd].childElementIndex = openFileList[currentDir].numOfChildren;
    openFileList[currentDir].numOfChildren++;
    openFileList[fd].isNewFile = 0;

//    openFileList[currentDir].numOfChildren++;
}

void changeDirectory(char *dirName) {
    int tempCurDir = currentDir;
    //parse input
    char *tokens[12];
    char *token;
    token = strtok(dirName, "/");
    tokens[0] = token;
    int count = 1;
    while (token != NULL) {
        token = strtok(NULL, "/");
        tokens[count] = token;
        count++;
    }

    for (int i = 0; i < 12; i++) {

        char *step = tokens[i];
        if (step == NULL)   //finished successfully
            break;
        if (strcmp(step, "~") == 0) {
            currentDir = 0;
            continue;
        }
        if (strcmp(step, "..") == 0) {
            if (currentDir == 0) {
                printf("invalid path\n");
                currentDir = tempCurDir;
                return;
            } else
                currentDir = openFileList[currentDir].parentId;
        } else { //step == a directory name
            int fd = ht_get(hashTable, step);

            if (fd == 0) {//error, reset currentDir and return
                printf("invalid path\n");
                currentDir = tempCurDir;
                return;
            }
            for (int j = 0; j < AVGDIRECTORYENTRIES; j++) {
                if (fd == openFileList[currentDir].dirChildren[j]) {
                    currentDir = fd;
                    break;//this should only break out of this nested j loop
                } else if (j == AVGDIRECTORYENTRIES - 1) { //reached the end with no matches
                    printf("invalid path\n");
                    currentDir = tempCurDir;
                    return;
                }
            }
        }
    }
}

void listFiles() {
    printf("\n");
    for (int i = 0; i < AVGDIRECTORYENTRIES; i++) {
        if (openFileList[currentDir].dirChildren[i] != 0) {
            int fd = openFileList[currentDir].dirChildren[i];
            printf("%s\n", openFileList[fd].name);
        }
    }
}

void moveFile(char *fileName, char *path) {
    //moving targetDir to become directory at end of specified path
    //fileName's fd is toMovefd

    //make sure file is in current directory
    int toMovefd = ht_get(hashTable, fileName);
    if (toMovefd == 0) {
        printf("file not in directory. Try again \n");
        return;
    }
    //make sure file is in current directory
    for (int i = 0; i < AVGDIRECTORYENTRIES; i++) {
        if (openFileList[currentDir].dirChildren[i] == toMovefd) {
            break; //leave loop to continue function
        } else if (i == AVGDIRECTORYENTRIES -
                        1) { //if we reach the end and havent found a match, specified file isnt in directory
            printf("file is not in directory. Try again\n");
            return;
        }
    }
    if (openFileList[toMovefd].flags) { //if we want to remove functionality for moving Directories, this is where we do it
        printf("moving Dir\n");
        //return;
    }

    //parse input
    char *tokens[12];
    char *token;
    token = strtok(path, "/");
    tokens[0] = token;
    int count = 1;
    while (token != NULL) {
        token = strtok(NULL, "/");
        tokens[count] = token;
        count++;
    }

    int targetDir = currentDir;

    //find specified directory
    for (int i = 0; i < 12; i++) {
        char *step = tokens[i];
        if (step == NULL) {   //found target directory without error
            openFileList[targetDir].dirChildren[openFileList[targetDir].numOfChildren] = toMovefd; //append toMovefd to target directory's dirchildren list
            openFileList[targetDir].numOfChildren++;
            //remove toMovefd from its parent dirChildren list
            for (int j = 0; j < AVGDIRECTORYENTRIES; j++) {
                if (openFileList[currentDir].dirChildren[j] ==
                    toMovefd) {//looking through our files parent directory children list
                    openFileList[currentDir].dirChildren[j] = 0; //remove that entry
                    return;
                }
            }
        }
        if (strcmp(step, "~") == 0) {
            targetDir = 0;
            continue;
        }
        if (strcmp(step, "..") == 0) {
            if (targetDir == 0) {
                printf("invalid path\n");
                return;
            } else {
                targetDir = openFileList[targetDir].parentId;
            }
        } else { //step == a directory name
            int tempfd = ht_get(hashTable, step);

            if (tempfd == 0) {//error, reset currentDir and return
                printf("invalid path\n");
                return;
            }
            for (int j = 0; j < AVGDIRECTORYENTRIES; j++) {
                if (tempfd == openFileList[targetDir].dirChildren[j]) {
                    targetDir = tempfd;
                    break;//this should only break out of this nested j loop
                } else if (j == AVGDIRECTORYENTRIES - 1) { //reached the end with no matches
                    printf("invalid path\n");
                    return;
                }
            }
        }
    }
}

void initRootDir(uint64_t startLoc, uint64_t blockSize) {
    currentDir = 0;
    openFileList[currentDir].numOfChildren = 0;
    openFileList[currentDir].flags = 1;

    openFileList[0].fileBuffer = "bierman is God";
    openFileList[0].dateModified = 0;
    openFileList[0].flags = 1;
    openFileList[0].size = 0;
    openFileList[0].usedBlocks[0] = currVCBPtr->freeBlockLoc;
    openFileList[0].bytesFromStart = 0;

    //write buffer to disk
    LBAwrite(openFileList[0].fileBuffer, 1, startLoc);
    currVCBPtr->freeBlockLoc = 1;
}

// TODO: getfreespace function

/*  This function ?returns? free space
 *
 *  Parameters: blocksNeeded-- blocksNeeded
 *  contig-- is the space contiguous
 *
 *  Returns first start location of avail space
 * */
int getFreeSpace(uint64_t blocksNeeded, int contig) {
    return 0;
}

/*  This function gets a file dateModified of a (processs?) used for directory structure
 *
 *  No parameter
 *
 *  Returns file dateModified
 * */
uint64_t getNewFileID() {
    uint64_t retVal;
    retVal = currVCBPtr->nextIdToIssue;
    currVCBPtr->nextIdToIssue = currVCBPtr->nextIdToIssue + FILEIDINCREMENT;
    return (retVal);
}

/*  This function opens a file
 *
 *  Parameters: fileName--name of file
 *  method-- method invoking this?
 *
 *  Returns file descriptor
 * */
int myFSOpen(char *fileName) {
    int fd; //file descriptor
    fd = ht_get(hashTable, fileName);
    time_t seconds;
    seconds = time(NULL);

    if (fd == 0) {
        latestID++;
//        printf("lastestID: %d\n", latestID);
        ht_set(hashTable, fileName, latestID);
        fd = latestID;
        openFileList[fd].isNewFile = 1;
        openFileList[fd].name = fileName;
    }
//  Setting file name when file is created
    openFileList[fd].dateCreated = seconds;

    return (fd);
}


/*  This function is similar to seek function. moves file bytesFromStart position
 *
 *  Parameters: fd-- filedescriptor
 *  position-- position of (?)
 *  method-- method of (?)
 *
 *  Returns files bytesFromStart
 * */
int myFSSeek(int fd, uint64_t position, int method) {
    if (fd >= FDOPENMAX)
        return -1;
    if ((openFileList[fd].flags && FDOPENINUSE) != FDOPENINUSE)
        return -1;

    switch (method) {
        case MYSEEK_CUR:    //moving position position
            openFileList[fd].bytesFromStart += position;
            break;
        case MYSEEK_POS:    //current position
            openFileList[fd].bytesFromStart = position;
            break;
        case MYSEEK_END:    //end position?
            openFileList[fd].bytesFromStart = openFileList[fd].size + position;
            break;
        default:
            break;
    }
    return (openFileList[fd].bytesFromStart);
}


int copyFile(char *fileName, char *destination) {
    int sourceFile;
    int destinationFile;
    char *contents = malloc(sizeof(char) * currVCBPtr->blockSize);
    time_t seconds;
    seconds = time(NULL);

//  return 0 for success
    sourceFile = myFSOpen(fileName);
    if (sourceFile == 0) {
        return 0;
    }

    destinationFile = myFSOpen(destination);
    if (destinationFile == 0) {
        return 0;
    }
//    printf("Before string copy");

    strcpy(contents, fsRead(sourceFile));
//    printf("After string copy");

    unsigned long length = strlen(contents);
//    printf("contents: %s\n", contents);

    fsWrite(destinationFile, contents, length);
    printf("File written\n");

    return 1;
}

void removeFile(char *filename) {
//    todo: reallocate this memory for use in write functions
    int fd = ht_get(hashTable, filename);
    char *overWriteBlock = malloc(sizeof(char) * currVCBPtr->blockSize);
    //create string of 512 blocks
    for (int i = 0; i < currVCBPtr->blockSize; i++) {
        strcat(overWriteBlock, ".");
    }
    //overwrites all used blocks belonging to a file with .'s
    for (int i = 0; i < openFileList[fd].numBlocksUsed; i++) {
        push(&freeBlockStack, openFileList[fd].usedBlocks[i]);
        LBAwrite(overWriteBlock, 1, openFileList[fd].usedBlocks[i]);
    }
    ht_del(hashTable, filename);
    openFileList[currentDir].dirChildren[openFileList[fd].childElementIndex] = 0; //index into current dir's children, where index number is the file to deletes index, and set to 0
    printf("Deleted %s\n", filename);

}

//checks the stack to see if we have a reclaimed block, if so return that. If not get a new block from currvcbptr and ++
int getFreeBlock() {
    int reclaimedBlockCheck = pop(&freeBlockStack);
    if (reclaimedBlockCheck == INT_MIN) {
        reclaimedBlockCheck = currVCBPtr->freeBlockLoc;
        currVCBPtr->freeBlockLoc++;
    }
    return reclaimedBlockCheck;
}


void addFile(int fd) {
//    if (openFileList[fd].isNewFile == 1) {
//        fsWrite(fd, "", 0);
//    }
//    if (openFileList[fd].isNewFile == 1) {
//
//        openFileList[fd].usedBlocks[0] = getFreeBlock();
//        openFileList[fd].bytesFromStart = openFileList[fd].usedBlocks[0] * 512;
//        char *temp = "";
////    printf("free block location: %lu\n", currVCBPtr->freeBlockLoc);
////    printf("current block location: %lu\n", currBlock);
//        uint64_t currBlock = openFileList[fd].bytesFromStart / currVCBPtr->blockSize;  //block num
//        LBAwrite(temp, 1, currBlock);
//        //currVCBPtr->freeBlockLoc++;
//        openFileList[fd].parentId = currentDir;
//        openFileList[fd].flags = 0;
//        openFileList[currentDir].dirChildren[openFileList[currentDir].numOfChildren] = fd;
//        openFileList[fd].childElementIndex = openFileList[currentDir].numOfChildren;
//        openFileList[currentDir].numOfChildren++;
//        openFileList[fd].isNewFile = 0;
//        openFileList[fd].usedBlocks[0]=getFreeBlock();
//
//        openFileList[fd].numBlocksUsed = 1;
//    }
}

/*  This function writes content to the filesystem
 *
 *  Parameters: fd-- file descriptor
 *  source-- content being input
 *  length-- size of content?
 *
 *  Returns
 * */
uint64_t fsWrite(int fd, char *source, uint64_t length) {
//todo: write open file list and currvcbptr to disk so that when we reopen the filesystem we can pick up where we left off

    int currBlock = openFileList[fd].usedBlocks[openFileList[fd].numBlocksUsed - 1];
    printf("beginning-curblock: %d\n", currBlock);
    int currOffset = (openFileList[fd].bytesFromStart % 512);  //remainder(where you are in the current block)
    printf("beginning-curroffset: %d\n", currOffset);

    int freeBlockSpace = currVCBPtr->blockSize - (openFileList[fd].bytesFromStart % 512);
    int overBlockLength = length > freeBlockSpace;
    int numBlocksToWrite = ((length + currOffset) / 512); //numBlocksToWrite includes current block
    if ((length + currOffset) % 512 != 0) numBlocksToWrite++;
    printf("beginning-numblockstowrite: %d\n", numBlocksToWrite);
    //numBlocksToWrite = total number of blocks we need to LBA write to (currBlock + any more we need to write to).

    time_t seconds;
    seconds = time(NULL);

    openFileList[fd].dateModified = seconds;
    openFileList[fd].size += length;
    printf("size: %llu\n", openFileList[fd].size);

    if (openFileList[fd].isNewFile) {
        //openFileList[fd].usedBlocks[0] = getFreeBlock();
        for(int i =0; i < numBlocksToWrite; i++){
            openFileList[fd].usedBlocks[i] = getFreeBlock();
            LBAwrite(source, 1, openFileList[fd].usedBlocks[i]);
            printf("written to %s, 1, %llu", source,openFileList[fd].usedBlocks[i]);
        }

        printf("usedblocks[0]: %llu\n", openFileList[fd].usedBlocks[0]);

        openFileList[fd].bytesFromStart = openFileList[fd].usedBlocks[0] * 512;
        openFileList[fd].numBlocksUsed = 1;
        openFileList[fd].flags = 0;
        openFileList[fd].isNewFile = 0;

        //directory stuff
        openFileList[fd].childElementIndex = openFileList[currentDir].numOfChildren;
        openFileList[currentDir].dirChildren[openFileList[currentDir].numOfChildren] = fd;
        openFileList[currentDir].numOfChildren++;
        openFileList[fd].parentId = currentDir;




    } else { //is not a new file

        //check to see if file is in currDir
        int fileInDirectory = 0;
        for (int i = 0; i < openFileList[currentDir].numOfChildren; i++){
            if (openFileList[currentDir].dirChildren[i] == fd) {
                printf("file found\n");
                fileInDirectory = 1;
                break;
            }
        }
        if (fileInDirectory == 0){
            printf("file not found\n");
            return 1;
        }

        char *buffer = malloc(sizeof(char) * 512);
        LBAread(buffer, 1, currBlock);
        if (overBlockLength) {
            //split so we can fill current block
            size_t len = strlen(buffer);
            memcpy(&buffer[len + 1], source, freeBlockSpace);
            LBAwrite(buffer, 1, currBlock);
            numBlocksToWrite--; //update numBlocksToWrite

            //write to next available block(s)
            //currBlock = getFreeBlock();
            printf("overBlockLength->currblock: %d\n", currBlock);
            source = &source[freeBlockSpace + 1];
            for(int i =0; i < numBlocksToWrite; i++){
                openFileList[fd].usedBlocks[openFileList[fd].numBlocksUsed+i] = getFreeBlock();
                openFileList[fd].numBlocksUsed++;
                LBAwrite(source, 1, openFileList[fd].usedBlocks[openFileList[fd].numBlocksUsed + i]);
            }
            //LBAwrite(source, numBlocksToWrite, currBlock);
        } else {
            strcat(buffer, source);
            LBAwrite(buffer, numBlocksToWrite, currBlock);
            // free(buffer);
        }
    }


    if (overBlockLength) {
        //update usedBlocks array and curVCBPtr->freeBlocLoc
        printf("numblockstowrite: %d\n", numBlocksToWrite);
//        for (int i = 0; i < numBlocksToWrite; i++) {
//            openFileList[fd].usedBlocks[openFileList[fd].numBlocksUsed + i] = currVCBPtr->freeBlockLoc;
//            currVCBPtr->freeBlockLoc++;
//            openFileList[fd].numBlocksUsed++;
//        }
        printf("last blocked used: %d\n", openFileList[fd].usedBlocks[openFileList[fd].numBlocksUsed - 1]);
        openFileList[fd].bytesFromStart =
                (openFileList[fd].usedBlocks[openFileList[fd].numBlocksUsed - 1] * 512) + strlen(source);
    } else {
        openFileList[fd].bytesFromStart = openFileList[fd].bytesFromStart + length; //new file position
    }


    printf("bytes from start: %llu\n", openFileList[fd].bytesFromStart);
    return 0;
}




// todo: notes on commands.
//  ls- display info in curr dir(iterate through names?
//  cd - sets internal global
//  pwd - display internal global
//  cp source dest -
//      1. fsopen(src)
//      2. fsopen(dest)
//      3. fsread(source)
//      4. fswrite(dest)
//  cptoLinux source dest -
//      1. fsopen(src)
//      2. open(dest)
//      3. fsread(src)
//      4. write(dest)
//      5. fsclose(src)
//      6. close(dest)
//  cpfromLinux source dest -
//      1. open(src)
//      2. fsopen(dest)
//      3. read(src)
//      4. fswrite(dest)
//      5. fscloes(dest)
//      6. close(src)

int fileIDCheck(char *filename) {
    int tempCheck; //check if file exists

    tempCheck = ht_get(hashTable, filename); //checks if file exists
    if (tempCheck == 0)  //file does not exist
    {
        //todo:check file space
        latestID++; //increment dateModified
        ht_set(hashTable, filename, latestID);  //adds file to hashtable
        tempCheck = ht_get(hashTable, filename); //gets hashed dateModified of file
        openFileList[tempCheck].isNewFile = 1;
        openFileList[tempCheck].name = filename;
        openFileList[tempCheck].flags = 0;
        return tempCheck;   //returns dateModified of file
    } else //file exists
    {
        return tempCheck;       //return dateModified
    }
}

char* getWriteInput(char* line){
  char* buff = malloc(sizeof(char)*51200);
  sscanf (line, "%*[^\"]\"%[^\"]\"", buff);
  return buff;
}

