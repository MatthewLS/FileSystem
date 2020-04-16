//
// Created by Matthew Stephens on 4/9/20.
//

#include "fsdriver3.h"
#include "fsLow.h"
#include "FileSystem.h"


int main (int argc, char *argv[])
{
    char * filename;
    uint64_t volumeSize;
    uint64_t blockSize;
    int retVal;

    if (argc > 3)
    {
        filename = argv[1];
        volumeSize = atoll (argv[2]);
        blockSize = atoll (argv[3]);
    }

    retVal = startPartitionSystem (filename, &volumeSize, &blockSize);
    printf("Opened %s, Volume Size: %llu;  BlockSize: %llu; Return %d\n", filename, (ull_t)volumeSize, (ull_t)blockSize, retVal);

    return EXIT_SUCCESS;
    }



    void loop(char* prompt){
        int stat;
        char *line;
        char *args;
        do {
            if (prompt != NULL){
                printf("%s ",prompt);
            } else {
                printf("> ");
            }

            //make a little loop to accept user input
            char* commands = scanf();
            char* commandList[2];
            commandList[0] = strtok(commands,' ');
            commandLIst[1] = strtok(NULL,' ');

            if (strcmp(command,"Q") || strcmp(command,"q")) {
                stat = 0;
            } else if (strcmp(command,"mkdir")){
                mkdir();
            }
            //switch/if statements to call subroutines based off that command



        } while(stat);
    return 0;
}




void mkdir(char* name){
    Directory dir = malloc(sizeof(Directory));
}