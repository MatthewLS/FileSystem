/**************************************************************
* Class: CSC-415-0# Spring 2020
* Name: Cabinet
* Student ID: 918297424
* Project: FileSystem
* File: FileSystem
* Description: header for a directory
* **************************************************************/

#ifndef UNTITLED1_DIRECTORY_H
#define UNTITLED1_DIRECTORY_H

/* file system info
 *  volume info
 *  ptr-LBAI->first free block
 *  if(usingbitarray)->volume has ptr to bitarray
*/

/* Volume
 * Volume Control Block (LCB) stored on Logical block 0 of Logical Block Array
 * LCB keeps track of all free blocks
 */

typedef struct Directory{
    //Dir name
    char name[30];
    //dir size
    unsigned long dirSize;



    /*permissions
    * a valid permission
    * entries in array correspond to permissions for each access group
    * admin[entries1-3],users[entries4-6],public[7-10]
    * entry index 0 denotes whether the file is a directory or not
    * permissions[0] is 1, then file is a dir
    * permissions[1-3] correspond to read,write,exe. if value is 1, then Admin has permission for that area.
    * same goes for user and public in that respective order of read,write,exe.
    */
    unsigned char permissions[11];

    /* this is just metadata for the Dir
     * datemod and datecreate are encoded using a long int. meaning a file created on the first on Jan 1st 2020 @ 1:34 will be dateCreate=010120201334
     */
    unsigned long int dateModified;
    struct tm dateCreate;


    /* Linked list for nugget ptrs
     * a file is a ptr to a file
     * a fileId is the id of that specific file
     * nextFile is a ptr to the next file that the directory holds
    */
    struct Node{
        unsigned long fileId;
        unsigned long file;
        unsigned long nextFile;
    }Node;

    struct fileList{
        struct Node node;
    }fileList;

    //mem location of directory this Dir is located in. if root, parent is special value
    unsigned long parent;
}Directory;

/*
 * File is a linked list structure which holds FileExtents, which describe contiguous blocks.
 * has 2 signatures to check for corruption
 */
typedef struct File {
    unsigned long sig1;

    //File will always have a least 1 fileExtents structure. if the file ever writes more data another FileExtent struct will be pointed to
    struct FileExtents{
        //address of first contiguous block section
        unsigned long address;
        //count of contiguous blocks
        int count;
        //if there are more blocks that are a part of the file, nextExtent points to the next

        //points to next contiguous block section
        struct FileExtents *nextExtent;
    }FileExtents;

    struct FileExtents fileExtensions;
    unsigned long sig2;
}File;
#endif //UNTITLED1_DIRECTORY_H