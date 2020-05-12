/**************************************************************
* Class: CSC-415-0# Spring 2020
* Group Name: Cabinet
* Name: File System

* Student ID: 918297424
* Name: Matthew Stephens
* Student ID: 915173797
* Name: Benjamin Lewis
* Student ID: 917871245
* Name: Tejas Vajjhala
* Student ID: 918383679
* Name: Chris Manaoat
*
* Assignment 3 – File System *
*
*
*  File: fsdriver3.c
*
* Description: The world's greatest file cabinet
*
* **************************************************************/

#include "fsdriver3.h"

openFileEntry *openFileList;    //array of currently open files
vcbStruct *currVCBPtr;          //vcb pointer
int currentDir = 0;             //current directory
ht_t *hashTable;                //hash table variable
int latestID = 5;               //counter for # of files/file dateModified's

int main(int argc, char *argv[]) {
    char *fileName;             //file name
    uint64_t volumeSize,        //volume size
            blockSize;         //block size
    int retVal;                 //returning value

    currVCBPtr = malloc(sizeof(vcbStruct) * 1);
    currVCBPtr->blockSize = 512;
    openFileList = malloc(sizeof(openFileList) * 256);

    fileName = argv[1];
    volumeSize = 10000000;
    blockSize = 512;
    retVal = startPartitionSystem(fileName, &volumeSize, &blockSize);
    currVCBPtr->freeBlockLoc = 1;
    printf("Opened %s, Volume Size: %lu; BlockSize: %lu\n", fileName, volumeSize, blockSize);

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
    free(currVCBPtr);
    free(openFileList);
    return EXIT_SUCCESS;
}


/*  This function is the shell portion of our filesystem
 *
 *  Parameters: blocksize-- size of the block
 *
 *  No return
 */
void loop(uint64_t blockSize) {
    int numCommands = 0,    //number of commands
        stat = 1,           //status
        counter = 0;        //counter
    char *line = malloc(BUFFER_LENGTH * sizeof(char)), //for holding line
        *lineForWrite = malloc(BUFFER_LENGTH * sizeof(char)), //for holding line passed to write parser
        *token, //for tokenizer
        **command; //command & arg list

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
            line = realloc(line, BUFFER_LENGTH);
        }

        token = strtok(line, " \n");
        while (token == NULL) {
            printf(">");
            fgets(line, BUFFER_LENGTH, stdin);      //gets line since old one was empty
            token = strtok(line, " \n");
        }
        while (token != NULL) {
            strcpy(command[counter], token);
//          printf("command:%s:%i\n", command[counter], counter);
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
            printf("cpfromfs - copy a file from real FS to virutal FS\n");
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
            int fd = fileIDCheck(command[1]);
            //make sure it isnt a dir
            if (openFileList[fd].flags == 1) {
                printf("cannot write a directory\n");
                continue;
            }
            openFileList[fd].name = command[1];
            //printf("calling write function now\n");
            fsWrite(fd, "", 1);
        } else if (strcmp(command[0], "rm") == 0) {
            removeFile(command[1]);
        } else if (strcmp(command[0], "read") == 0) {
            fsRead(fileIDCheck(command[1]));

        } else if (strcmp(command[0], "write") == 0) {
            //printf("in write function \n");
            int fd = fileIDCheck(command[1]);
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
//            printf("Called write function with %s\n",lineForWrite);
//            printf("Called write function with length %llu\n",strlen(lineForWrite));
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
            free(buffer);
        } else {
            
            printf("Command %s not found.\n", command[0]);
        }
        counter = 0;
    }
    //switch/if statements to call subroutines based off that command

//     for(int i = 0; i < numCommands; i++){
//         free(command[i]);
//     }

     free(command);
     free(line);
    return;
}



/*  This function copies files from virtual FS to Linux FS
 *
 *  Parameters: filename-- filename
 *  fd-- file descriptor(id)
 *
 *  No return
 */
void cptofs(char *fileName, int fd) {
//    printf("IN COPY TO FS");
    char *fileContents = fsRead(fd),    //file contents
        currWD[BUFFER_LENGTH];          //path for current working directory

    getcwd(currWD, sizeof(currWD));
    printf("Current Working Directory: %s\n", currWD);
    FILE *OutFile = fopen(fileName, "w");
    fputs(fileContents, OutFile);
    fclose(OutFile);
    printf("File written to: %s/%s\n", currWD, fileName);
    printf("File contents: %s\n", fileContents);
}

/*  Function reads in data
 *
 *  Parameter: fd-- file descriptor(id)
 *
 *  Returns file contents
 * */
char *fsRead(int fd) {
//    printf("IN READ FUNC\n");
    for (int i = 0; i < openFileList[currentDir].numOfChildren;i++){
        if (openFileList[currentDir].dirChildren[i] == fd) {
//                printf("IN READ FUNC1\n");
            char *pBuf = malloc(sizeof(char) * 512000);
//                printf("IN READ FUNC2\n");
            char *tempBuf = malloc(sizeof(char) * 512000);
//    printf("IN READ FUNC3\n");

            for (int j = 0; j < openFileList[fd].numBlocksUsed; j++) {
                LBAread(tempBuf, 1, openFileList[fd].usedBlocks[j]);
//                printf("block num: %llu, text: %s\n", openFileList[fd].usedBlocks[j], tempBuf);
                strcat(pBuf, tempBuf);
            }
            //int retVal = LBAread(pBuf, 1, openFileList[fd].usedBlocks[0]);
//            printf("numblocksused: %llu\n", openFileList[fd].numBlocksUsed);
            printf("text: %s\n", pBuf);
            return pBuf;
        }
    }
    printf("file not found\n");
    return "file not found\n";
}

/*  Function inits root dir
 *
 *  Parameters: dirName- name of directory
 *  fd --file descriptor(id)
 *
 *  No return
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

/*  This function changes the directories accounts for pathing(parsing included)
 *
 *  Parameter: dirName-- directory name
 *
 * */
void changeDirectory(char *dirName) {
    int tempCurDir = currentDir;    //current directory
    //parse input
    char *tokens[12],       //holds tokens
        *token;             //token
    token = strtok(dirName, "/");
    tokens[0] = token;
    int count = 1;          //counter
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


/*  This function lists files in the directory
 *
 *  No parameters
 *
 *  No return
 * */
void listFiles() {
    printf("\n");
    for (int i = 0; i < AVGDIRECTORYENTRIES; i++) {
        if (openFileList[currentDir].dirChildren[i] != 0) {
            int fd = openFileList[currentDir].dirChildren[i];
            printf("%s\n", openFileList[fd].name);
        }
    }
}

/*  This function moves files within the virtual fs
 *
 *  Parameters: FileName-- name of file
 *  path-- path of file
 *
 *  No return
 */
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


/*  This funciton inits root directory
 *
 *  Parameters: startLoc-- where root starts
 *  blockSize-- size of block
 *
 *  No return
 */
void initRootDir(uint64_t startLoc, uint64_t blockSize) {
    currentDir = 0;
    openFileList[currentDir].numOfChildren = 0;
    openFileList[currentDir].flags = 1;

    openFileList[0].fileBuffer = "Bierman is God";
    openFileList[0].dateModified = 0;
    openFileList[0].flags = 1;
    openFileList[0].size = 0;
    openFileList[0].usedBlocks[0] = currVCBPtr->freeBlockLoc;
    openFileList[0].bytesFromStart = 0;

    //write buffer to disk
    //LBAwrite("Bierman is God", 1, startLoc);
    currVCBPtr->freeBlockLoc = 1;
}

/*  This function opens a file
 *
 *  Parameters: fileName--name of file
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

/*  This function copies a file
 *
 *  Parameters: fileName-- name of the file
 *  destination-- name of the file to be written to
 *
 *  Returns 0 for success and 1 for fail
 */
int copyFile(char *fileName, char *destination) {
    int sourceFile;
    int destinationFile;
    time_t seconds;
    seconds = time(NULL);

//  return 0 for success
    sourceFile = myFSOpen(fileName);
    if (sourceFile == 0) {
        return 0;
    }

    char *contents = malloc(sizeof(char) * currVCBPtr->blockSize * openFileList[sourceFile].numBlocksUsed);
    destinationFile = myFSOpen(destination);
    if (destinationFile == 0) {
        return 0;
    }
//    printf("Before string copy");

    strcpy(contents, fsRead(sourceFile));
//    printf("After string copy");

    unsigned long length = strlen(contents);
//    printf("contents: %s\n", contents);

//    printf("\n----\n");
    openFileList[destinationFile].name = destination;
    fsWrite(destinationFile, "", 1);
    fsWrite(destinationFile, contents, length);
   // printf("File written\n");

    return 1;
}

/*  This function removes files from the virtual fs
 *
 *  Parameter: filename-- name of the file
 *
 *  No return
 */
void removeFile(char *filename) {
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

    free(overWriteBlock);
}

/* This function checks the stack to see if we have a reclaimed block,
 *
 * No Parameters
 *
 * Returns reclaimed block num. If not get a new block from currvcbptr and ++
 * */
int getFreeBlock() {
    int reclaimedBlockCheck = pop(&freeBlockStack);
    if (reclaimedBlockCheck == INT_MIN) {
        reclaimedBlockCheck = currVCBPtr->freeBlockLoc;
        currVCBPtr->freeBlockLoc++;
    }
    return reclaimedBlockCheck;
}

/*  This function writes content to the filesystem
 *
 *  Parameters: fd-- file descriptor
 *  source-- content being input
 *  length-- size of content?
 *
 *  Returns 0 for success and 1 for fail
 * */
uint64_t fsWrite(int fd, char *source, uint64_t length) {
//    printf("in write\n");
    int currBlock;
    //if(!openFileList[fd].isNewFile)
        currBlock = openFileList[fd].usedBlocks[openFileList[fd].numBlocksUsed - 1];
//    printf("beginning-curblock: %d\n", currBlock);
    int currOffset = (openFileList[fd].bytesFromStart % 512);  //remainder(where you are in the current block)
//    printf("beginning-curroffset: %d\n", currOffset);
//    printf("length %d\n", length);
    int freeBlockSpace = currVCBPtr->blockSize - (openFileList[fd].bytesFromStart % 512);
    int overBlockLength = length > freeBlockSpace;
    int numBlocksToWrite = ((length + currOffset) / 512); //numBlocksToWrite includes current block
    if ((length + currOffset) % 512 != 0) numBlocksToWrite++;
//    printf("beginning-numblockstowrite: %d\n", numBlocksToWrite);
    //numBlocksToWrite = total number of blocks we need to LBA write to (currBlock + any more we need to write to).

    time_t seconds;
    seconds = time(NULL);

    openFileList[fd].dateModified = seconds;
    openFileList[fd].size += length;
//    printf("size: %llu\n", openFileList[fd].size);

    if (openFileList[fd].isNewFile) {
       // openFileList[fd].numBlocksUsed = 1;
        //openFileList[fd].usedBlocks[0] = getFreeBlock();
        for(int i = 0; i < numBlocksToWrite; i++){
            size_t len;
            int block = getFreeBlock();
            openFileList[fd].usedBlocks[i] = block;
//            printf("added %llu\n", openFileList[fd].usedBlocks[i]);
            currBlock = openFileList[fd].usedBlocks[i];
            LBAwrite(source, 1, openFileList[fd].usedBlocks[i]);
            openFileList[fd].numBlocksUsed++;
//            printf("written to %s, 1, %llu\n", source,openFileList[fd].usedBlocks[i]);
            len = strlen(source);
            if (len > 512)
                source = &source[513];
        }

//        printf("usedblocks[0]: %llu\n", openFileList[fd].usedBlocks[0]);
//        printf("1\n");
        if(openFileList[fd].numBlocksUsed != 0)
            openFileList[fd].bytesFromStart = openFileList[fd].usedBlocks[openFileList[fd].numBlocksUsed - 1] * 512;
        //openFileList[fd].numBlocksUsed = numBlocksToWrite;
//        printf("2\n");
        openFileList[fd].flags = 0;
//        printf("3\n");
        openFileList[fd].isNewFile = 0;
//        printf("4\n");

        //directory stuff
        openFileList[fd].childElementIndex = openFileList[currentDir].numOfChildren;
//        printf("5\n");
        openFileList[currentDir].dirChildren[openFileList[currentDir].numOfChildren] = fd;
//        printf("6\n");
        openFileList[currentDir].numOfChildren++;
//        printf("7\n");
        openFileList[fd].parentId = currentDir;
//        printf("8\n");

    } else { //is not a new file

        //check to see if file is in currDir
        int fileInDirectory = 0;
        for (int i = 0; i < openFileList[currentDir].numOfChildren; i++){
            if (openFileList[currentDir].dirChildren[i] == fd) {
               // printf("file found\n");
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
            memcpy(&buffer[len], source, freeBlockSpace);
            LBAwrite(buffer, 1, currBlock);
            numBlocksToWrite--; //update numBlocksToWrite

            //write to next available block(s)
            //currBlock = getFreeBlock();
//            printf("overBlockLength->currblock: %d\n", currBlock);
            source = &source[freeBlockSpace + 1];
            for(int i =0; i < numBlocksToWrite; i++){
                openFileList[fd].usedBlocks[openFileList[fd].numBlocksUsed] = getFreeBlock();
//                printf("%s %i\n", source, openFileList[fd].usedBlocks[openFileList[fd].numBlocksUsed]);
                LBAwrite(source, 1, openFileList[fd].usedBlocks[openFileList[fd].numBlocksUsed]);
                openFileList[fd].numBlocksUsed++;
//                printf("%llu\n", openFileList[fd].numBlocksUsed);

                len = strlen(source);
                if (len > 512)
                    source = &source[513];
            }
            //LBAwrite(source, numBlocksToWrite, currBlock);
        } else {
            strcat(buffer, source);
            LBAwrite(buffer, numBlocksToWrite, currBlock);
             free(buffer);
        }
    }


    if (overBlockLength) {
        //update usedBlocks array and curVCBPtr->freeBlocLoc
//        printf("numblockstowrite: %d\n", numBlocksToWrite);

//        printf("last blocked used: %llu\n", openFileList[fd].usedBlocks[openFileList[fd].numBlocksUsed - 1]);
        openFileList[fd].bytesFromStart =
                (openFileList[fd].usedBlocks[openFileList[fd].numBlocksUsed - 1] * 512) + strlen(source);
    } else {
        openFileList[fd].bytesFromStart = openFileList[fd].bytesFromStart + length; //new file position
    }

//    for(int i = 0; i < openFileList[fd].numBlocksUsed; i++)
//    {
//        printf("%llu\n", openFileList[fd].usedBlocks[i]);
//    }

//    printf("bytes from start: %llu\n", openFileList[fd].bytesFromStart);

    return 0;
}

/*  This function checks for the file id
 *
 *  Parameter: filename-- used to check if file exists
 *
 *  Returns fd of the file if it exists, 0 if it does not
 * */
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

/* This function gets line input
 *
 * Parameter: line: whole text
 *
 * Returns text
 * */
char* getWriteInput(char* line){
  char* buff = malloc(sizeof(char)*51200);
  sscanf (line, "%*[^\"]\"%[^\"]\"", buff);
  return buff;
}

