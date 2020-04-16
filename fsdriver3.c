//
// Created by Matthew Stephens on 4/9/20.
//

#include "fsdriver3.h"
#include "fsLow.h"
#include "FileSystem.h"


int main (int argc, char *argv[])
{
    //when we run the driver we specify the volume/root directory name, volume size and block size.
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
    //start partition system creates our volume
    retVal = startPartitionSystem (filename, &volumeSize, &blockSize);
    printf("Opened %s, Volume Size: %llu;  BlockSize: %llu; Return %d\n", filename, (ull_t)volumeSize, (ull_t)blockSize, retVal);

    //this is where we will accept user input and call functions
    loop();

    return EXIT_SUCCESS;
    }



    void loop(){
        int stat;
        char *line;
        char *args;
        do {
                printf("> ");

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