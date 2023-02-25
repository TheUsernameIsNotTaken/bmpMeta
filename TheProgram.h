//Includes with conditions
#ifndef free				//Standard library
   #include <stdlib.h>
#endif
#ifndef printf				//Include to use standard IO
   #include <stdio.h>
#endif
#ifndef strcpy				//Include for string manipulation
   #include <string.h>
#endif
#ifndef UCHAR_MAX			//Include to use min and max values
	#include <limits.h>
#endif
#ifndef open				//Include for low-level file access
	#include <fcntl.h>
#endif
#ifndef opendir				//Include for standard symbolic constants and types
	#include <unistd.h>
#endif
#ifndef alarm				//Include for basic directory entries
	#include <dirent.h>
#endif
#ifndef stat				//Include what defines the structure of the data returned by the stat functions
	#include <sys/stat.h>
#endif
#ifndef connect				//Include for the Internet Protocol families
	#include <sys/socket.h>
#endif
#ifndef INADDR_ANY			//Include for the Internet address families
	#include <netinet/in.h>
#endif
#ifndef signal			   //Include for signal handling
	#include <signal.h>
#endif
#ifndef omp_set_num_threads	//Include to support multi-threading
	#include <omp.h>
#endif
#ifndef ANSI_COLOR_RESET	//Include for ANSI color and type codes
	#include "ANSI_colors.h"
#endif

//Subfunction 2.1
/**
 * Encodes one char into one bmp pixel.
 * Encoding logic: encodes a single char's 1st 1/3 into the 1st, 2nd into the 2nd
 * and 3rd into the 3rd color of the pixel (always using the last 3 bits)
 * @method encode_pixel
 * @param  caracter(in)     The caracter what we want to encode into the pixel.
 * @param  pixelArray(out)  A pointer to the start of the pixel's data we want to encode.
 */
void encode_pixel(char caracter, char* pixelArray){
	//Variable initialisation
	unsigned char code = 0;
	unsigned char mask = 0;
	unsigned char bit = 0;
	//Encode into every color of the pixel
	char b = 7;
	for(int a = 0; a < 3; a++){
		code = 0;
		mask = 0;
		//we need to code the char into the last 3 bits in every color, but it has only 8 bits. --> the first is always 0
		do{
            mask = 1 << b;			//Set the mask to
			bit = caracter & mask;	//get the first unread bit.
			bit >>= b - (b % 3);	//Set the bit into the correct position
			code |= bit;			//And add it to the partial code
		}while(b-- % 3 != 0);		//End it after we read 3 caracters, but still decrese b's value.
		pixelArray[a] &= 0b11111000;//Keep the first 5 bits of the pixel.
		pixelArray[a] |= code;		//Code into the last 3 bits.
	}
}

// 2.1
/**
 * Creates a random test array of pixels with an already encoded 'abc' message
 * @method TestArray
 * @param  NumCh(out)   An int pointer where the method saves the number of encoded caracters
 * @return              An int pointer of the generated test array
 */
char* TestArray(int* NumCh){
	//Save the number of caracters
	int caracters = 3;
	*NumCh = caracters;
	//Allocate memory for the pixels
	char* array = (char*)malloc( caracters*3 * sizeof(char) );
	char caracter = 'a'; //ve have a 'abc' string to wrap, so we start with a and increment it in every iteration.
	for(int i = 0; i < caracters; i++){
		//3 color per pixel
		for(int j = 0; j < 3; j++){
			int random = rand() % UCHAR_MAX;
			printf("Random_%i: %i\n", i*3+j, random); //Which random numbers did we get? - Write out for testing purposes only
			array[i*3 + j] = (char)random;
		}
		encode_pixel( caracter, &(array[i*3]) );
		caracter++;
	}
	return array;
}

//2.2 Updated at: 5.1
/**
 * Decodes an already encoded pixel array by the same logic what is definied in the encode_pixel method.
 * @method Unwrap
 * @param  Pbuff(out)   The encoded image data buffer's start pointer
 * @param  NumCh(out)   The number of encoded caracters
 * @exit   2            If the memory allocation for the decoded text failed.
 * @return              The encoded text message's start pointer
 */
char* Unwrap(char* Pbuff, int NumCh){

	//Memory allocation with error handling
	char* array = (char*)malloc( (NumCh+1) * sizeof(char) );
	if(array == NULL){
		fprintf(stderr, "Error while trying to allocatte memory for the Unwrapped text.");
		exit(2);
	}

	//Create variables
	int i, j;
	char actChar, decoded;

	//pararell run - needs -fopenmp gcc flag to work
	#pragma omp parallel num_threads(4) shared(array) private(i, j, decoded, actChar)
	{
		//We need to unwrap 'NumCh' lenght of string.
		#pragma omp for
		for(i = 0; i < NumCh; i++){
			actChar = 0;
			//3 color per pixel - decode and bitOR together
			for(j = 0; j < 3; j++){
				decoded = Pbuff[i*3+j] & 0b00000111;
				decoded <<= (2-j)*3;
				actChar |= decoded;
			}
			//Add the decoded caracter to the string
			array[i] = actChar;
		}
	}
	//End the decoded text
	array[NumCh] = '\0';
	//Free the buffer
	free(Pbuff);
	//Return
	return array;
}

//3.1
/**
 * Opens a bmp file's data and gets it's pixel array and the # of encoded caracters.
 * @method ReadPixels
 * @param  f(in)        The opened filestream of the encoded bmp image
 * @param  NumCh(out)   A pointer, where the function saves the # of encoded caracters
 * @exit   2            If the memory allocation for the pixel data failed.
 * @return              The pixel array of the bmp image
 */
char* ReadPixels(int f, int* NumCh){
	//Helping hand
	int pixelSize, jumpto;
	//Go to the correct point
	lseek(f, 2, SEEK_SET);
	//Read the number of pixels, encoded caracters and the jumpto position
	read(f, &pixelSize, sizeof(int));
	read(f, NumCh, sizeof(int));
	read(f, &jumpto, sizeof(int));
	//Allocate memory with error handling
	char* array = (char*) malloc( pixelSize * sizeof(char) );
	if(array == NULL){
		fprintf(stderr, "Error while trying to allocatte memory for the bmp binary pixel data.");
		exit(2);
	}
	lseek(f, jumpto, SEEK_SET);
	for(int i = 0; i < pixelSize; i++){
		read(f, array+i, 1);
	}
	//return
	return array;
}

//3.2
/**
 * A method, what creates a Terminal File browser.
 * If the input is a Folder, then it opens it, and writes out the folder's content.
 * The browsing starts at the user's home folder. And ends if the user gives a file as an input.
 * If the file or folder name is not found, it stays in the current directory.
 * You can go to a parent folder with the '..' "command".
 * @method BrowseForOpen
 * @return      A readonly filestream of the chosen file.
 */
int BrowseForOpen(){
	//Variables
	DIR* dir;
	struct stat inode_dest;
	struct stat inode_tmp;
	struct dirent* entry;
	//struct passwd* pwd;
	char tmp [100] = { 0 };
	char destination [100] = { 0 };
	strcpy(destination, ".");
	//Start with the home directory by changing the working directory
	chdir(getenv("HOME"));
	//run until file choose
	do{
		chdir(destination);
		dir = opendir(".");
		//Write out every element and it's inode data
		printf("Writing out the directory data...\n");
		while( (entry = readdir(dir)) != NULL ){
			stat( (*entry).d_name, &inode_dest);
			switch (inode_dest.st_mode & S_IFMT){
				case S_IFDIR:
					printf("%s%s", ANSI_BOLD, ANSI_COLOR_BLUE);
					break;
				case S_IFREG:
					printf("%s%s", ANSI_BOLD, ANSI_COLOR_RED);
					break;
			}
			printf("%s%s\t", (*entry).d_name, ANSI_COLOR_RESET);
		}
		//Get the next target and load if it exists
		printf("\nPlease enter a file/directory to open: ");
		scanf("%s", tmp);
		if(!stat(tmp, &inode_tmp)){
			strcpy(destination, tmp);
			inode_dest = inode_tmp;
		}else{
			strcpy(destination, ".");
			stat(destination, &inode_dest);
		}
        //close
        closedir(dir);
	}while(!(inode_dest.st_mode & S_IFREG) );
	//return
	return open(destination, O_RDONLY);
}

//4.1
/**
 * Sends a HTML POST message, with the neptunID and message to the correct IP adress.
 * @method Post
 * @param  neptunID(in) The user's neptunID. You will can find your message with the help of this ID
 * @param  message(in)  The message what you want to send to the server
 * @param  NumCh(in)    The length of the message
 * @return              Returns 0, if successfull.
 *                      Returns an error code in the range of 3-8 with the corresponding meaning.
 *                      In the case of error, this also writes out an error message into the standard error output.
 */
int Post(char *neptunID, char *message, int NumCh){
	//Set the server's properties : 193.6.135.148:80
	struct sockaddr_in server;
	server.sin_family 		= AF_INET;
	server.sin_addr.s_addr 	= inet_addr("193.6.135.148");
	server.sin_port 		= htons(80);	//Network order port (Big-Endian)
	//Communication buffer
	const int extra = 150;                 //This is a rought estimation of the # of caracters of the POST other parts.
	const int bufferSize = 1024;
	char buffer[bufferSize];
	//Build the message
	const char* firstName = "NeptunID";
	const char* secondName = "PostedText";
	int contentLength = strlen(firstName) + strlen(neptunID) + strlen(secondName) + NumCh + 3;
	if( bufferSize < contentLength + extra){
		fprintf(stderr, "Wrong data size.\n");
		return 3;
	}
	snprintf(
		buffer, contentLength + extra,
		"%s\r\n%s\r\n%s: %i\r\n%s\r\n\r\n%s=%s&%s=%s\r\n",
		"POST /~vargai/post.php HTTP/1.1",
		"Host: irh.inf.unideb.hu",
		"Content-Length", contentLength,
		"Content-Type: application/x-www-form-urlencoded",
		firstName, neptunID, secondName, message
	);
	//printf("%s\n", buffer); //Print the message - only for test
	//Crearte a socket
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0){
		fprintf(stderr, "Error while trying to create the socket.\n");
		return 4;
	}
	//Set the socket address reusable
	int one = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one) );
	//Connect to the server
	if( connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0 ){
		fprintf(stderr, "Error while connecting to the server.\n");
		return 5;
	}
	//Send the message to the server
	if( write(sock, buffer, strlen(buffer)) < 0 ){
		fprintf(stderr, "Error while sending the message to the server.\n");
		return 6;
	}
	//Wait for the server answer
	if( read( sock, buffer, bufferSize+1) < 0 ){
		fprintf(stderr, "Error while recieveing the answer from the server.\n");
		return 7;
	}
	close(sock);
	//Check the aswer
	if( strstr(buffer, "The message has been received.") != NULL ){
		printf("Succesfull communication\n");
	}else{
		fprintf(stderr, "Communication verification fail.\n");
		return 8;
	}
	//Successfull return
	return 0;
}

/**
 * This function catches the SIGINT and SIGALRM signals.
 * In the case of a SIGINT signal the procces starts a child procces (with forking), which reminds the user,
 * that the program disabled the key combination. After that the it kills the proccess immediately.
 * In the case of a SIGALRM signal the procces creates a timeout error message and exits with a '9' error code.
 * @method WhatToDo
 * @param  sig(in)      The catched signal's identifier flag.
 */
void WhatToDo(int sig){
    //send notification about the disabled ctrl^c.
    if(sig == SIGINT){
        //Fork
        pid_t pid;
        pid=fork();
        //The child will send the message to the user.
        if(pid==0){
            printf("%s%s%s\n%s\n",
            ANSI_COLOR_RED,
            "The Ctrl+C key combination does not stop the program while it decodes and communicates with the server.",
            ANSI_COLOR_RESET,
            "Please wait for the proccess to end.");
            kill(getpid(), SIGKILL);
        }
        //The parent will continue the proccess.
    }
    //If it's a timeout alarm, then stop the program with a timeout error.
    else{
        fprintf(stderr, "Image decoding timeout.\n");
		exit(9);
    }
}

/**
 * An extra char to binary text converter for displaying and debug purposes.
 * @method char_to_binary_str
 * @param  a(in)        The caracter what we want to convert to binary string.
 */
void char_to_binary_str(char a){
	unsigned char mask = 1 << (sizeof(char)*8 - 1);
	while(mask){
		printf( (a & mask) ? "1" : "0" );
		mask >>= 1;
	}
	printf("\n");
}
