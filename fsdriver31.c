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
#include "fsLow.c"
//#include "Directory.h"

#define AVGDIRECTORYENTRIES 50
#define EXIT_SUCCESS 0
#define MAXCOMMLIST 5
#define BUFFER_LENGTH 256
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
typedef struct fsStruct {
    int counter;
    int size;
    char spacer[256 - (sizeof(int) * 2)],
            name[];
} fsStruct, *fsStructPtr;

typedef struct dirEntry {
    uint64_t id,    //id for ...
    date,       //date
    location,   //what block entry is in the disk(lba sart)
    sizeinBytes;    //file size. blocks needed = (sizeinBytes + (blocksize - 1)) / blocksize
    uint32_t flags;
    char name[NAME_LENGTH]; //file name limited to 128 char
} dirEntry, *dirEntryPtr;

typedef struct OpenFileEntry {
    int flags;  //flags of file
    uint64_t pointer,   //where you are in file
    size,   //size of file(in bytes)
    id, //id of file
    blockStart; //start block
    char *fileBuffer;   //file buffer(contents?)
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

void fsRead(uint64_t);

int fsWrite2(char *, uint64_t);

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


// TODO: adjust start locations
int main(int argc, char *argv[]) {
    char *fileName;
    uint64_t volumeSize;
    uint64_t blockSize;
    int retVal;
    currVCBPtr = malloc(sizeof(vcbStruct) * 1);
    currVCBPtr->blockSize = 512;
    openFileList = malloc(sizeof(openFileList) * 256);

    if (argc > 3) {
        fileName = argv[1];
        volumeSize = atoll(argv[2]);
        blockSize = atoll(argv[3]);
    }
    volumeSize = 10000000;
    blockSize = 512;
    strcpy(fileName, "newfs");
    //retVal = startPartitionSystem("newfs", &volumeSize, &blockSize);
    retVal = startPartitionSystem(fileName, &volumeSize, &blockSize);
    currVCBPtr->freeBlockLoc = 1;
    printf("Opened %s, Volume Size: %lu; BlockSize: %lu\n", fileName, volumeSize, blockSize);
    freeMap(volumeSize, blockSize);
    initRootDir(6, blockSize);
    loop(blockSize);
    printf("out of loop. About to close partition\n");
    closePartitionSystem();
    printf("partition closed.\n");
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
        command = malloc(BUFFSIZE * sizeof(char));
        if (command == NULL)
            printf("could not allocate space\n");
        for (int i = 0; i < BUFFSIZE ; i++) {
            command[i] = malloc(BUFFER_LENGTH * sizeof(char));
            if (command[i] == NULL)
                printf("could not allocate space\n");
        }
        printf("> ");

        //make a little loop to accept user input
        fgets(line, BUFFER_LENGTH, stdin);
        printf("line: %s\n", line);
        if((sizeof(line) / sizeof(char) + 1) > BUFFER_LENGTH)
        {
            printf("over256\n");
            line = realloc(line, BUFFER_LENGTH * 3);
        }

        char *tempLine = malloc(BUFFER_LENGTH * sizeof(char));
        if((sizeof(tempLine) / sizeof(char) + 1) > BUFFER_LENGTH){
            printf("realloc templine\n");
            tempLine = realloc(tempLine, BUFFER_LENGTH * 3);
        }
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

            //fsWrite2(tempLine, blockSize);
            char *copiedText = malloc(BUFFSIZE * sizeof(char));  //temporary copied text
            printf("in write function \n");
            printf("line:%s\n", tempLine);
            token = strtok(tempLine, "\""); //tokenizer
            printf("got token\n");
            while (token == NULL) { //empty case
                printf(">");
                fgets(tempLine, BUFFER_LENGTH, stdin);      //gets line since old one was empty
                token = strtok(tempLine, " \n");
            }
            printf("tokenizing\n");
            while (token != NULL) {
                //printf("token:%s\n", token);
                strcpy(copiedText, token);
                if((sizeof(copiedText) / (sizeof(char) + 1)) > BUFFSIZE) {
                    printf("realloc\n");
                    copiedText = realloc(copiedText, BUFFSIZE);
                }
                token = strtok(NULL, "\"\n");
            }
            printf("Text that will be written is:%s\n", copiedText);
            int length = sizeof(copiedText) / (sizeof(char) + 1);
            printf("Length: %i\n", length);
            int fd;
            fd = fopen(command[1], "w");
            if (fd == NULL)
                printf("null\n");
            printf("command: %s\n", command[1]);
            fsWrite(fd, copiedText, length);
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


/*  Function reads in data
 *
 *  Parameter: blockSize-- blocksize
 *
 *  No return
 * */
void fsRead(uint64_t blockSize) {
    fsStructPtr pBuf = malloc(blockSize * 2);
    int retVal = LBAread(pBuf, 2, 12);
    printf("text:%s\n", pBuf->name);
    free(pBuf);

}


/*  Function writes content to disk //test function
 *
 *  Parameter: line-- content to be written
 *  blocksize -- blocksize
 *
 *  Returns if success or not?
 * */
int fsWrite2(char *line, uint64_t blockSize) {
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

    //bookmark 2
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
    char *freeBuffer = malloc(freeBlocksNeeded * blockSize);    //allocate space for FCB
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

void initRootDir(uint64_t startLoc, uint64_t blockSize) {
    dirEntryPtr rootDirBuffer;  //pointer to directory entry
    uint64_t entrySize = sizeof(dirEntry), //size of entry itself
    bytesNeeded = AVGDIRECTORYENTRIES * entrySize, //initialize base bytes needed for directory of AVGDIRENTRY size
    blocksNeeded = (bytesNeeded + (blockSize - 1)) / blockSize, //base blocks ^
    actualDirEntries = (blocksNeeded + blockSize) / entrySize;  //actual size of directory

    printf("For %d entries we need %lui bytes\n", AVGDIRECTORYENTRIES, bytesNeeded);
    printf("Actual dir entries = %lui\n", actualDirEntries);

    //need to know if fs has space?
    startLoc = getFreeSpace(actualDirEntries, CONTIGUOUS);

    rootDirBuffer = malloc(blocksNeeded * blockSize); //init size of root dir buffer

    //loop to initialize actual dir entries
    for (int i = 0; i < actualDirEntries; i++) {
        rootDirBuffer[i].id = 0;
        rootDirBuffer[i].flags = DIR_UNUSED;
        strcpy(rootDirBuffer[i].name, "");
        rootDirBuffer[i].date = 0;
        rootDirBuffer[i].location = 0;
        rootDirBuffer[i].sizeinBytes = 0;
    }

    //needs pointer of root's parent now
    rootDirBuffer[0].id = 1000; //random location
    rootDirBuffer[0].flags = 0; //0 for directory
    strcpy(rootDirBuffer[0].name, "root");    //roots name
    rootDirBuffer[0].date = 1234;   //random date
    rootDirBuffer[0].location = startLoc;   //start location of root
    rootDirBuffer[0].sizeinBytes = actualDirEntries * entrySize; //size in bytes

    //write buffer to disk
    LBAwrite(rootDirBuffer, blocksNeeded, startLoc);

    openFileList[0].id = 0;
    openFileList[0].flags = 1;
    openFileList[0].size = 0;
    openFileList[0].blockStart = currVCBPtr->freeBlockLoc;


    openFileList[0].pointer = 512;
//    openFileList[0].pointer = openFileList->pointer + length; //new file position
//    currBlock = openFileList[0].pointer / currVCBPtr->blockSize;
//    currOffset = openFileList[0].pointer % currVCBPtr->blockSize;

    //bookmark
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

/*  This function gets a file id of a (processs?) used for directory structure
 *
 *  No parameter
 *
 *  Returns file id
 * */
uint64_t getNewFileID() {
    uint64_t retVal;
    retVal = currVCBPtr->nextIdToIssue;
    currVCBPtr->nextIdToIssue = currVCBPtr->nextIdToIssue + FILEIDINCREMENT;
    return (retVal);
}


//todo: figure out myfsopen

/*  This function opens a file
 *
 *  Parameters: fileName--name of file
 *  method-- method invoking this?
 *
 *  Returns file descriptor
 * */
int myFSOpen(char *fileName, int method) {
    int fd; //file descriptor

    //gets file descriptor, looks for structure thats free
    for (int i = 0; i < FDOPENMAX; i++) {
        if (openFileList[i].flags == FDOPENFREE) {
            fd = i;
            break;
        }
    }

    //find file in directory, null if file does not exist
//    openFileList[i].flags = FDOPENINUSE | FDOPENFORREAD | FDOPENFORWRITE;
//    openFileList[i].fileBuff = malloc(currVCBPtr->blockSize * 2);
//    openFileList[i].pointer = 0;
//    openFileList[i].size = 0;

    return (fd);
}

//todo: figure out myfsseek

/*  This function is similar to seek function. moves file pointer position
 *
 *  Parameters: fd-- filedescriptor
 *  position-- position of (?)
 *  method-- method of (?)
 *
 *  Returns files pointer
 * */
int myFSSeek(int fd, uint64_t position, int method) {
    if (fd >= FDOPENMAX)
        return -1;
    if ((openFileList[fd].flags && FDOPENINUSE) != FDOPENINUSE)
        return -1;

    switch (method) {
        case MYSEEK_CUR:    //moving position position
            openFileList[fd].pointer += position;
            break;
        case MYSEEK_POS:    //current position
            openFileList[fd].pointer = position;
            break;
        case MYSEEK_END:    //end position?
            openFileList[fd].pointer = openFileList[fd].size + position;
            break;
        default:
            break;
    }
    return (openFileList[fd].pointer);
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
//    time_t t = time(NULL);
//    struct tm *tm = localtime(&t);
//    char s[64];
//    assert(strftime(s, sizeof(s), "%c", tm));

    time_t seconds;
    seconds = time(NULL);

    printf("In fs write\n");
    //checks if fd is in use
//    if(fd >= FDOPENMAX)
//        return -1;
//    if((openFileList[fd].flags && FDOPENINUSE) != FDOPENINUSE)
//        return -1;

    printf("after checks\n");

    //printf("openfilelist: %i\n",fd);
    fd = 0;
    printf("changed fd\n");
//    if(openFileList[fd].blockStart == NULL || openFileList[fd].blockStart == 0)
//    {
//        fd = 0;
//    }

    uint64_t currBlock = openFileList[fd].pointer / currVCBPtr->blockSize,  //block num
    currOffset = openFileList[fd].pointer % currVCBPtr->blockSize;  //remainder(where you are in the current block)

    //todo: FD HARDCODED TO 0, ONLY CAN WRITE TO FIRST BLOCK, CHECK FOR FIRST RUN, IF FIRST RUN THEN FD = 0 ELSE FD NOT 0
    // INSTEAD OF INDEXING BY FD CREATE A GLOBAL FILE COUNT THAT GETS INCREMENTED EVERYTIME FILE MADE
    printf("length: %lu|blocksize: %lu|currblock: %lu\n",length, currVCBPtr->blockSize, currBlock);
    if (length + currOffset < currVCBPtr->blockSize) //content fits into block
    {
        printf("first if\n");
        //openFileList[fd].fileBuffer = malloc(sizeof(char) * BUFFSIZE);
        //strcpy(openFileList[fd].fileBuffer, source);
        openFileList[fd].fileBuffer = source;
        printf("after: %s\n", openFileList[fd].fileBuffer);
        memcpy(openFileList[fd].fileBuffer + currOffset, source, length);   //copies content into block
        openFileList[fd].id = seconds;
        openFileList[fd].flags = 1;
        openFileList[fd].size = length;
        openFileList[fd].blockStart = currVCBPtr->freeBlockLoc;


        openFileList[fd].pointer = openFileList->pointer + length; //new file position
        currBlock = openFileList[fd].pointer / currVCBPtr->blockSize;
        currOffset = openFileList[fd].pointer % currVCBPtr->blockSize;



        LBAwrite(openFileList[fd].fileBuffer, 1, currBlock + openFileList[fd].blockStart);
        printf("freeblockloc before: %lu\n", currVCBPtr->freeBlockLoc);
        currVCBPtr->freeBlockLoc = currBlock + 1;
        printf("freeblockloc: %lu\n", currVCBPtr->freeBlockLoc);



    } else if (length + currOffset < (currVCBPtr->blockSize * 2)) //content doesn't fit in space
    {
        printf("second statement\n");
        strcpy(openFileList[fd].fileBuffer, source);
        memcpy(openFileList[fd].fileBuffer + currOffset, source, length);   //copies content into block

        //writeblock = translateFileBlock(fd, currBlock); //translates file(if contiguous)
        //todo: lbawrite.
        // figure out filebuffer var.
        // openfilelist.filebuff?
        LBAwrite(openFileList[fd].fileBuffer, 2, currBlock + openFileList[fd].blockStart);
        memcpy(openFileList[fd].fileBuffer, openFileList[fd].fileBuffer + currVCBPtr->blockSize,
               currVCBPtr->blockSize); // copies content into buffer


        openFileList[fd].id = seconds;
        openFileList[fd].flags = 1;
        openFileList[fd].size = length;
        openFileList[fd].blockStart = currVCBPtr->freeBlockLoc;


        openFileList[fd].pointer = openFileList->pointer + length; //new file position
        currBlock = openFileList[fd].pointer / currVCBPtr->blockSize;
        currOffset = openFileList[fd].pointer % currVCBPtr->blockSize;

        printf("freeblockloc before: %lu\n", currVCBPtr->freeBlockLoc);
        currVCBPtr->freeBlockLoc = currBlock + 2;
        printf("freeblockloc: %lu\n", currVCBPtr->freeBlockLoc);

    } else {
        printf("Content doesn't fit\n");
    }
//    openFileList[fd].pointer = openFileList->pointer + length; //new file position
//    currBlock = openFileList[fd].pointer / currVCBPtr->blockSize;
//    currOffset = openFileList[fd].pointer % currVCBPtr->blockSize;
//
//    printf("freeblockloc before: %lu\n", currVCBPtr->freeBlockLoc);
//    currVCBPtr->freeBlockLoc = currBlock + 1;
//    printf("freeblockloc: %lu\n", currVCBPtr->freeBlockLoc);
//    //if file closed write buffer
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