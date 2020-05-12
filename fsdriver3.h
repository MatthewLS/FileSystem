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
*  File: fsdriver3.h
*
* Description: The world's greatest file cabinet
*
* **************************************************************/

#ifndef FILESYSTEM2_FSDRIVER3_H
#define FILESYSTEM2_FSDRIVER3_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
#include "fsLow.h"
#include "stack.c"
#include "hashImplement.c"
#include "fsLow.h"
#include "fsLow.c"

#define AVGDIRECTORYENTRIES 50  //average directory size
#define EXIT_SUCCESS 0          //exit success
#define BUFFSIZE 256	        //buffer space
#define BUFFER_LENGTH 5120      //buffer length

typedef struct OpenFileEntry {
    int flags,                  //1 == dir. 0 == file
    numOfChildren,          //number of children
    childElementIndex,      //index of child element
    isNewFile,              //1 == should be new file 0 == should overwrite
    parentId,               //parent id
    numBlocksUsed,          //number of blocks used
    dirChildren[AVGDIRECTORYENTRIES];   //list of children
    uint64_t bytesFromStart,    //where you are in file
    size,               //size of file(in bytes)
    usedBlocks[1000],   //lba position that belongs to the fine
    dateCreated,        //date created
    dateModified;       //date modified
    char *fileBuffer,           //contents
    *name;                 //name of file
} openFileEntry;

typedef struct vcbStruct {
    uint64_t blockSize,     //block size
    freeBlockLoc;   //free block location

} vcbStruct;

char *fsRead(int);

void createDir(char *, int);

void changeDirectory(char *);

void listFiles();

void moveFile(char *, char *);

void removeFile(char *);

void cptofs(char *, int);

int copyFile(char *, char *);

void loop(uint64_t);

void initRootDir(uint64_t, uint64_t);

uint64_t fsWrite(int, char *, uint64_t);

int fileIDCheck(char *);

struct StackNode *freeBlockStack = NULL;

char* getWriteInput(char* line);

#endif //FILESYSTEM2_FSDRIVER3_H
