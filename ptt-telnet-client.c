/**
 ============================================================================
 Name        : ptt-telnet-client.c
 Author      : Hakki Caner KIRMIZI, #b96902133
 
 
 References: 
 
 GNU C Lib 
 http://theory.uwinnipeg.ca/gnu/glibc/libc_toc.html
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

/* Defines */
#define SMALL_BUFFER_SIZE 512
#define LARGE_BUFFER_SIZE 1024

/* Global Variables */
char *input_read_buffer = NULL;
size_t input_buffer_size = 0;
int socket_fd = -1;
int output_fd = -1;
const char carriage_ret[] = "\r\0";

/* Typedefs */

/* Structures */

/* Externs */

/* Function prototypes */
static void print_usage();
static void stream_file(const char *filename);
const char* get_value(char *buffer);
static void send_data(const char *data, int len);
static void enter_username(char *buffer);


/** 
 * main:
 * @filename: the file name to parse
 * Parse the input file and get the directive
 */
int 
main(int argc, char **argv) {
	struct hostent *hp;
	struct sockaddr_in srv;

	if(argc != 2) {
        print_usage();
        exit(EXIT_FAILURE);
    }

	/* create a connection to the server */
	srv.sin_family = AF_INET; // use the internet addr family
	srv.sin_port = htons(23); // dedicated telnet port = 23
	srv.sin_addr.s_addr = inet_addr("140.112.172.11"); // ptt.cc IP address
	
	/* create the socket */
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}
	
	/* connect to the server */
	printf("Connecting to 140.112.172.11 [ptt.cc]...\n");
	if (connect(socket_fd, (struct sockaddr*) &srv, sizeof(srv)) < 0) {
		perror("connect");
		exit(1);
	} else {
		printf("Connection succesfull.\n");
	}
	
	/* open the file descriptor of output file */
	//output_fd = open("output.txt", O_WRONLY); 

	/* stream the input file */
	stream_file(argv[1]);	

	/* Done, close output file */
	//close(output_fd);	

	return 0;
}

/** 
 * print_usage:
 * Print usage information
 */
static void 
print_usage() {
    fprintf(stderr, "Usage: ./ptt-telnet-client [input-filename]\n");
}

/** 
 * stream_file:
 * @filename: the file name to parse
 * Parse the input file and get the directive
 */
static void
stream_file(const char *filename) {
	FILE *input_fp = NULL;

	/* Open input file to read directives */
	input_fp = fopen(filename, "r");
	
	/* Loop over the lines */
	while (!feof(input_fp)) {
		/* Memory allocation */
		input_buffer_size = SMALL_BUFFER_SIZE * sizeof(char);
		input_read_buffer = malloc(input_buffer_size);

		if (fgets(input_read_buffer, input_buffer_size, input_fp) != NULL) {
			//printf("%s\n", input_read_buffer);
			if (strncmp(input_read_buffer, "<ID>", 4)==0) {
				enter_username(input_read_buffer);
				//printf("%s", input_read_buffer);
			}
		}
		
		free(input_read_buffer);
		input_read_buffer = NULL; // just in case; ignore double free
	}
	
	/* Done, close input file */
	fclose(input_fp);
}

/** 
 * get_value:
 * @buffer: <ID> ... </ID> 
 * Get the value of the corresponding node
 */
const char*
get_value(char *buffer) {
	char *feedback = NULL;

	feedback = strtok(buffer, "<>");
	feedback = strtok(NULL, "<>");
	return feedback;
}

/** 
 * send_data:
 * @data: The data which is going to be sent to the server 
 * @len: Length of the data
 * Send the data to the server
 */
static void
send_data(const char *data, int len) {

	printf("Sending data: %s ...\n", data);
	if (write(socket_fd, data, len+1) < 0) {
		perror("send_data: sending username");
		exit(1);
	} else {
		printf("Writing data: %s to output file...\n", data);
		if (write(output_fd, data, len+1) < 0) {
			perror("send_data: writing username to output file");
			exit(1);
		}
	}

	printf("Sending data carriage return...\n");
	if (write(socket_fd, carriage_ret, 2) < 0) {
		perror("sen_data: username, carriage return");
		exit(1);
	} else {
		printf("Writing carriage return to output file...\n");
		if (write(output_fd, carriage_ret, 2) < 0) {
			perror("send_data: username, writing carriage return to output file");
			exit(1);
		}
	}
}

/** 
 * enter_username:
 * @buffer: <ID> ... </ID> 
 * Extract the username from ID tags and enter it in the login part
 */
static void
enter_username(char *buffer) {
	const char *username;
	int username_len;
	
	username = get_value(buffer);
	username_len = strlen(username);
	printf("%s  %d\n", username, username_len);
	
	/* Send the username data to the server */
	//send_data(username, username_len); 
	
}


