//
// Created by Matthew Stephens on 4/9/20.
//

#include "fsdriver3.h"
#include "fsLow.h"

struct commands {

}

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
            char* command = scanf();
            if (strcmp(command,"Q") || strcmp(command,"q")) {
                stat = 0;
            }
            //switch/if statements to call subroutines based off that command



        } while(stat);
    return 0;
}