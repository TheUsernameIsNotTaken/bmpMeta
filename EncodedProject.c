#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include "TheProgram.h"
#include "ANSI_colors.h"

int main(int argc, char* argv[]){

	//RNG initialisation
	srand(time(NULL));

	//1.0
    /**
     * Without a argument: Chose a file with the BrowseForOpen's help, decode it and send it's message to the server with a specified NeptunID
     * @method if
     */
	if(argc == 1){
        //Basics - 3.3
        printf("Mode: Normal run with no file argument.\n\n");
        //Open a file
        int file = BrowseForOpen();
        //Decode
        int length;
        char* pixelArray = ReadPixels(file, &length);
		//Refuse ^C SIGINT
		signal(SIGINT, WhatToDo);
		//Set timeout
		signal(SIGALRM, WhatToDo);
		alarm(1);
		//Decode the files
        char* text = Unwrap(pixelArray, length);
		//Stop the timeout
		alarm(0);
        //printf("Decoded test string: %s\n", text); - for test if we need
        //Message sending - 4.2
        char* id = "CHIYE1";
        int postError = Post(id, text, 800);
        if(postError)	exit(postError);
        close(file);
    }
    /**
     * Run the program with a single argument.
     * @method if
     */
    else if(argc == 2){
        /**
         * If the argument is '--version', then display the version data.
         * @method if
         */
		if(!strcmp(argv[1], "--version")){
            printf("Mode: Version and creator information.\n\n");
			printf("Project's name: %s\n", "The Encoded Project");

			printf("The date of the production: %d.%d.%d %d:%d\n", 2020, 04, 27, 18, 40),
			printf("Developer's name: %s\n", "Máté Vágner");
		}
        /**
         * If the argument is '--help', then display the help.
         * @method if
         */
		else if(!strcmp(argv[1], "--help")){
            printf("Mode: Help.\n\n");
			printf("%s%s%s\n", ANSI_COLOR_BLUE, "The possible argument usages for normal usage:", ANSI_COLOR_RESET);
            printf("%s -->%s %s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET, "--help: displays this help.");
			printf("%s -->%s %s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET, "--version: displays the current version of the project.");
			printf("%s -->%s %s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET, "[path_to_file]: Decode the picture at the given destination.");
            printf("%s -->%s %s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET, "Without argument: Decode the picture at the chosen destination by the build-in file explorer.");
            //TODO: Add extra content
            printf("%s%s%s\n", ANSI_COLOR_BLUE, "The possible argument usages for testing:", ANSI_COLOR_RESET);
            printf("%s -->%s %s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET, "--test: Encode 'abc' into a test array, then display, decode and send it to the server.");
			printf("%s -->%s %s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET, "--send [neptunID] [messages]: POST the given [message] to the server as the given [neptunID].");
		}
        /**
         * If the argument is '--test', then encode an 'abc' string into 3 random pixels and
         * show their decoded and enoded content. Also POST the text string to the server.
         * For testing and presentation purposes.
         * @method if
         */
		else if(!strcmp(argv[1], "--test")){
            printf("Mode: Test.\n\n");
		    //Creating test data
			int NumCh = 0;
			char* test = TestArray(&NumCh);
			printf("There are %i caracters, and the chars are:\n", NumCh);
			for(int i = 0; i < NumCh; i++){
				printf("pixel number: %i\n", i);
				for(int j = 0; j < 3; j++){
					printf(" - ");
					char_to_binary_str(test[i*3 + j]);
				}
			}
			char* decoded = Unwrap(test, NumCh);
			printf("Decoded test string: %s\n", decoded);
			//POST the test string
			char* id = "CHIYE1";
			int postError = Post(id, decoded, strlen(decoded));
			if(postError)	exit(postError);
			//free the allocated memory
			free(decoded);
		}
		/**
		 * With a simple complex argument: Decode the image at the argument given path and send it's message to the server with a specified NeptunID
	     * @method else
		 */
        else{
            printf("Mode: Normal run with file argument.\n\n");
			//Check if the path is an existing file
			struct stat inode;
			stat(argv[1], &inode);
			//!inode.st_mode & S_IFREG
			//access(argv[1], F_OK ) != -1
			if( !(inode.st_mode & S_IFREG) ){
				fprintf(stderr, "Wrong path in the argument.\nPlease use '%s --help' or check out the ReadMe file if you need help.\n", argv[0]);
				return 1;
			}
	        //Open the file
	        int file = open(argv[1], O_RDONLY);
	        //Decode
	        int length;
	        char* pixelArray = ReadPixels(file, &length);
			//Refuse ^C SIGINT
			signal(SIGINT, WhatToDo);
			//Set timeout
			 signal(SIGALRM, WhatToDo);
			 alarm(1);
			 //Decode the files
	        char* text = Unwrap(pixelArray, length);
			//Stop the timeout
			alarm(0);
	        //printf("Decoded test string: %s\n", text); - for test if we need
	        //Message sending
	        char* id = "CHIYE1";
	        int postError = Post(id, text, 800);
	        if(postError)	exit(postError);
	        close(file);
        }
	}
    /**
     * If we have 3 arguments, where the first is '--send', then we
     * POST with an argument given id an argument given one word message to the server.
     * The 2nd argument will be our ID and the 3nd will be our one word message.
     * @method if
     */
    else if(argc == 4 && !strcmp(argv[1], "--send")){
        printf("Mode: POST.\n\n");
        char* id = argv[2];
        char* text = argv[3];
        int postError = Post(id, text, strlen(text));
		if(postError)	exit(postError);
    }
    /**
     * Tell the user how to get to the program help. There he/she can find how to use the program
     * @method else
     */
    else{
		fprintf(stderr, "Unsupported command. Please use '%s --help' or check out the ReadMe file to view the program's commands.\n", argv[0]);
		return 10;
    }
    //Return with 0, if we were succesfull
	return 0;
}
