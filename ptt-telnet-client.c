#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <libxml/xmlreader.h>

#define SMALL_BUFFER_SIZE 512
#define LARGE_BUFFER_SIZE 1024

/* Global Variables */
char *input_read_buffer = NULL;
size_t input_buffer_size = 0;

/* Function prototypes */
static void stream_file(const char *filename);

int main() {
	struct hostent *hp;
	struct sockaddr_in srv;
	int fd = -1;

	/* create a connection to the server */
	srv.sin_family = AF_INET; // use the internet addr family
	srv.sin_port = htons(23); // dedicated telnet port = 23
	srv.sin_addr.s_addr = inet_addr("140.112.172.11"); // ptt.cc IP address
	
	/* create the socket */
	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}
	
	/* connect to the server */
	printf("Connecting to 140.112.172.11...\n");
	if (connect(fd, (struct sockaddr*) &srv, sizeof(srv)) < 0) {
		perror("connect");
		exit(1);
	} else {
		printf("Connection succesfull.\n");
	}
	
	/* stream the input file */
	stream_file("input.txt");

	return 0;
}

/** 
 * stream_file:
 * @filename: the file name to parse
 * Parse and print information about an XML file.
 */
static void
stream_file(const char *filename) {
	FILE *input_fp, *output_fp = NULL;		
	
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
				printf("%s", input_read_buffer);
			}
		}
		
		free(input_read_buffer);
		input_read_buffer = NULL; // just in case; ignore double free
	}
}
