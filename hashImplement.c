//
// Created by student on 4/25/20.
//

/*  NOTE:*********I do not own this code. this code is from
*   https://github.com/engineer-man/youtube/blob/master/077/hashtable.c
*   these hash table functions are used in my program
*   Some portions were modified according to my use
*/

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 20000000    //initial table size

typedef struct entry_t {
    char* fileName;              //key word
    int fileID;              //value of the key
    struct entry_t* next;   //bytesFromStart to next entry(in case of collision will chain the value)
} entry_t;

typedef struct {
    entry_t** entries;      //bytesFromStart to the entries, in a hash table
} ht_t;


/*  This function makes a hash value for the key
*   argument is the key value to be hashed
*   returns the hash value
*   note**again I do not own this code. all rights and credit goes to Engineer man
*/
unsigned int hash(const char* key) {
    unsigned long int value = 0;            //value of hash
    unsigned int i = 0;                     //counter
    unsigned int key_len = strlen(key);     //gets the length of the value provided, used to hash

    // do several rounds of multiplication
    for (; i < key_len; ++i) {
        value = value * 37 + key[i];        //goes through the word, gets its char value and totals it to make a hash value
    }

    // make sure value is 0 <= value < TABLE_SIZE
    value = value % TABLE_SIZE;

    return value;
}
/*  This function makes an entry to be put into the hash table,
*   arguments are the key and its corresponding value
*   returns the key and value as an entry
*   note**again I do not own this code. all rights and credit goes to Engineer man
*/
entry_t* ht_pair(const char* key, const int value) {
    // allocate the entry
    entry_t* entry = malloc(sizeof(entry_t) * 1);       //allocates space for the entry
    entry->fileName = malloc(strlen(key) + 1);               //allocates space for the key(since its a string)
    entry->fileID = value;   //sets the keys value from provided argument

    strcpy(entry->fileName, key);        //copies the key into the entry

    // next starts out null but may be set later on
    entry->next = NULL;

    return entry;
}

/*  This function initializes the hash table
*   argument would be the hash table
*   returns a variable of type hash table
*   note**again I do not own this code. all rights and credit goes to Engineer man
*/
ht_t* ht_create(void) {
    // allocate table
    ht_t* hashtable = malloc(sizeof(ht_t) * 1);

    // allocate table entries
    hashtable->entries = malloc(sizeof(entry_t*) * TABLE_SIZE);

    // set each to null (needed for proper operation)
    int i = 0;
    for (; i < TABLE_SIZE; ++i) {
        hashtable->entries[i] = NULL;
    }

    return hashtable;
}

/*  This function adds a key and value to the hash table
*   arguments hashtable, key, and its corresponding value
*   no return
*   note**again I do not own this code. all rights and credit goes to Engineer man
*/
void ht_set(ht_t* hashtable, const char* key, const int value) {
    unsigned int slot = hash(key);      //goes to the hashed spot of the key word

    // try to look up an entry set
    entry_t* entry = hashtable->entries[slot];

    // no entry means slot empty, insert immediately
    if (entry == NULL) {
        hashtable->entries[slot] = ht_pair(key, value);
        return;
    }

    entry_t* prev;      //used for chaining(in case of collision)

    // walk through each entry until either the end is
    // reached or a matching key is found
    while (entry != NULL) {
        // check key
        if (strcmp(entry->fileName, key) == 0) {
            // match found, replace value
            entry->fileID = value;       //sets the value for the key
            return;
        }

        // walk to next
        prev = entry;
        entry = prev->next;
    }

    // end of chain reached without a match, add new
    prev->next = ht_pair(key, value);
}

/*  This function searches the hash table for the given key
*   argument are the hashtable and the key to be searched
*   returns an int
*   note**again I do not own this code. all rights and credit goes to Engineer man
*/
int ht_get(ht_t* hashtable, const char* key) {
    unsigned int slot = hash(key);      //gets the hash location of the word

    // try to find a valid slot
    entry_t* entry = hashtable->entries[slot];

    // no slot means no entry
    if (entry == NULL) {
        return 0;
    }

    // walk through each entry in the slot, which could just be a single thing
    while (entry != NULL) {
        // return value if found
        if (strcmp(entry->fileName, key) == 0) {
            return entry->fileID;
        }

        // proceed to next key if available
        entry = entry->next;
    }

    // reaching here means there were >= 1 entries but no key match
    return 0;
}

/*  This function deletes a value from the hash table
*   arguments are the hash table and the word to be deleted
*   no return value
*   note**again I do not own this code. all rights and credit goes to Engineer man
*/
void ht_del(ht_t* hashtable, const char* key) {
    unsigned int bucket = hash(key);    //gets the hash location of the keyword

    // try to find a valid bucket
    entry_t* entry = hashtable->entries[bucket];

    // no bucket means no entry
    if (entry == NULL) {
        return;
    }

    entry_t* prev;      //used if entries are linked in a spot
    int idx = 0;        //index

    // walk through each entry until either the end is reached or a matching key is found
    while (entry != NULL) {
        // check key
        if (strcmp(entry->fileName, key) == 0) {
            // first item and no next entry
            if (entry->next == NULL && idx == 0) {
                hashtable->entries[bucket] = NULL;
            }

            // first item with a next entry
            if (entry->next != NULL && idx == 0) {
                hashtable->entries[bucket] = entry->next;
            }

            // last item
            if (entry->next == NULL && idx != 0) {
                prev->next = NULL;
            }

            // middle item
            if (entry->next != NULL && idx != 0) {
                prev->next = entry->next;
            }

            // free the deleted entry
            free(entry->fileName);
            free(entry);

            return;
        }

        // walk to next
        prev = entry;
        entry = prev->next;

        ++idx;
    }
}


/*  this function prints the hash table
*   argument is the hash table to be printed
*   no return value
*   note**again I do not own this code. all rights and credit goes to Engineer man
*/
void ht_dump(ht_t* hashtable) {
    for (int i = 0; i < TABLE_SIZE; ++i) {      //for loop to assist going through the entries
        entry_t* entry = hashtable->entries[i]; //points to the ith entry of the table

        if (entry == NULL) {    //no value to be printed(empty entry)
            continue;
        }

        printf("slot[%4d]: ", i);   //prints index

        for (;;) {           //for loop assisting entries hashed to the same slot(in a list)
            printf("%s=%i ", entry->fileName, entry->fileID);

            if (entry->next == NULL) {      //if reached end of list
                break;
            }

            entry = entry->next;    //goes to the next entry in list
        }

        printf("\n");
    }
}
