//
// Created by Matthew Stephens on 4/9/20.
//

#include "fsdriver3.h"
#include "fsLow.h"
#include "Directory.h"


    int main(int argc,char *argv) {

        loop(argv[1]);

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