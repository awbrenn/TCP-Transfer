/*********************************************************
File Name:  httpClient.cpp
Author:     Austin Brennan
Course:     CPSC 3600
Instructor: Sekou Remy
Due Date:   03/25/2015


File Description:
This file contains an implementation of a simple
http client. See readme.txt for more details.

*********************************************************/


#include "awbrennExam2.h"

char* SERVER_NAME = NULL;
char* FILENAME = NULL;
char* FILE_CONTENTS = NULL;
size_t FILE_SIZE = 0;
int PORT;
int TOTAL_FILES_READ = 0;

void handleError(char * error_message) {
	printf("ERROR: %s\n", error_message);
	exit(0);
}

char* getProperUse(char *program_name) {
	char* proper_use_string = (char*) calloc(150, sizeof(char));

	sprintf(proper_use_string, "Improper Usage - see proper use cases below\n"
		"%s <flag> <server name> <server port> <file name to transfer>\n"
		"%s <flag> <server port> <file name to create>\n", program_name, program_name);

	return proper_use_string;
}


void serverCNTCCode() {
    printf("\nServer Finished.\n\nTotal files transfered: %d\n", TOTAL_FILES_READ);
    exit(0);
}


void readFile() {
    FILE *file = fopen(FILENAME, "r");
    size_t i = 0;
    int c;

    if (file == NULL) return;
    fseek(file, 0, SEEK_END);
    FILE_SIZE = (size_t) ftell(file);
    fseek(file, 0, SEEK_SET);
    FILE_CONTENTS = (char *)malloc(FILE_SIZE);

    while ((c = fgetc(file)) != EOF) {
        FILE_CONTENTS[i++] = (char)c;
    }

//    FILE_CONTENTS[i] = '\0';
}


void clientMain(int argc, char* argv[]) {
//	printf("This is CLIENT\n");
    int message_len;
	int sock;
    struct sockaddr_in serverAddr; /* Local address */
	struct hostent *thehost;         /* Hostent from gethostbyname() */


	if (argc != 5)
		handleError(getProperUse(argv[0]));

	SERVER_NAME = argv[2];
	PORT = atoi(argv[3]);
	FILENAME = argv[4];

	/* Create a TCP socket */
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		handleError((char *)"socket() failed");

	/* Construct the server address structure */
	memset(&serverAddr, 0, sizeof(serverAddr));    /* Zero out structure */
	serverAddr.sin_family = AF_INET;                 /* Internet addr family */
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_NAME);  /* Server IP address */
	serverAddr.sin_port   = htons(PORT);     /* Server port */

	/* If user gave a dotted decimal address, we need to resolve it  */
	if ((int)serverAddr.sin_addr.s_addr == -1) {
		thehost = gethostbyname(SERVER_NAME);
		serverAddr.sin_addr.s_addr = *((unsigned long *) thehost->h_addr_list[0]);
	}

	/* Establish the connection to the server */
	if (connect(sock, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
		handleError((char *)"connect() failed");

	/* Read the file */
	readFile();

    /* Send the size of the file to the server */
    if (write(sock, &FILE_SIZE, sizeof(FILE_SIZE)) != (ssize_t) sizeof(FILE_SIZE))
        handleError((char *)"write() failed - wrote a different number of bytes than expected");

//	printf("%s %zu\n",FILE_CONTENTS, FILE_SIZE);

	/* Send the file contents to the server */
	if (write(sock, FILE_CONTENTS, FILE_SIZE) != FILE_SIZE)
		handleError((char *)"write() failed - wrote a different number of bytes than expected");

}


void serverMain(int argc, char* argv[]) {
//	printf("This is SERVER\n");
	int serverSock;
	int file_descriptor;
    struct sockaddr_in serverAddr; /* Local address */
    struct sockaddr_in clientAddr; /* Client address */
    int clientSock;
    unsigned int clientLen;


	if (argc != 4)
		handleError(getProperUse(argv[0]));

	/* set PORT and FILENAME */
	PORT = atoi(argv[2]);
	FILENAME = argv[3];

	/* set up signal handler */
    signal (SIGINT, serverCNTCCode); 

   /* Create socket for sending/receiving data */
    if ((serverSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        handleError((char *)"socket() failed");

    /* Construct local address structure */
    memset(&serverAddr, 0, sizeof(serverAddr));   /* Zero out structure */
    serverAddr.sin_family = AF_INET;                /* Internet address family */
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    serverAddr.sin_port = htons(PORT);      /* Local port */

    /* Bind to the local address */
    if (bind(serverSock, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
        handleError((char *)"bind() failed");

    if (listen(serverSock, MAX_PENDING) < 0)
        handleError((char *)"listen() failed");

	/* setting the size of the in-out parameter */
	clientLen = sizeof(clientAddr);

	/* main server loop */
	while(1) { // run forever
	    char filename_buffer[150];

	    /* waiting for the client to connect */
	    if ((clientSock = accept(serverSock, (struct sockaddr *) &clientAddr, &clientLen)) < 0) {
			printf("accept() failed\n");
			continue;
	    }

		/* Read the file size from the client */
	    if (read(clientSock, &FILE_SIZE, sizeof(FILE_SIZE)) != sizeof(FILE_SIZE)) {
	        printf("read() failed - read a different number of bytes than expected\n");
	        continue;
	    }

//	    printf("\nFile size is: %zu\n", FILE_SIZE);


	    /* Malloc space for the new file*/
	    FILE_CONTENTS = (char*)malloc(FILE_SIZE);
	    size_t len;
	    size_t total_read_bytes = 0;
		/* Read the incoming file contents */
		while (total_read_bytes != FILE_SIZE) {
	    	len = (size_t) read(clientSock, FILE_CONTENTS+total_read_bytes, FILE_SIZE);
	    	total_read_bytes += len;
		}
		//    printf("\nFILE CONTENTS:\n%s\n", FILE_CONTENTS);
		/* create unique filename for the new file */
		sprintf(filename_buffer, "%s%d.data",FILENAME,TOTAL_FILES_READ);
//		printf("\n%s\n",filename_buffer);

	    if ((file_descriptor = open(filename_buffer, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH)) < 0) {
			printf("open() failed could not open file - "
				               "make sure filename isn't already in directory.\n");
			continue;
	    }

		if (write(file_descriptor, FILE_CONTENTS, FILE_SIZE) != FILE_SIZE) {
			printf("write failed - wrote a different number of bytes than expected\n");
			continue;
		}

		/* increment the total files read */
		TOTAL_FILES_READ++;
	}
}


int main(int argc, char *argv[]) {
	if (argc != 5 && argc != 4)
		handleError(getProperUse(argv[0]));

	if (atoi(argv[1]) == 0)
		clientMain(argc, argv);
	else if (atoi(argv[1]) == 1)
		serverMain(argc, argv);
	else
		handleError((char*)"Invalid Flag - proper flag values are\n0 - meaning client\n"
			"1 - meaning server");

}


