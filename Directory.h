//
// Created by Matthew Stephens on 3/12/20.
//

#ifndef UNTITLED1_DIRECTORY_H
#define UNTITLED1_DIRECTORY_H

typedef struct directory
{
    char name[30];
    unsigned long long dirSize;
    char dirType[10];
    unsigned char permissions;


    //mem location of first block dir points to
    unsigned long startBlock;



    //for permissions
    long id;

    //date
    unsigned long int dateModified;
    unsigned long int dateCreate;

    unsigned long memLocation;

    long nuggetPtr;
    long parent;

    //holds mem locations
    short bitfield[];

};
#endif //UNTITLED1_DIRECTORY_H