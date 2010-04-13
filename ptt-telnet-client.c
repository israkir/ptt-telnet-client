/**
 ============================================================================
 Name			: ptt-telnet-client.c
 Author			: Hakki Caner KIRMIZI, #b96902133
 Description		: A C program which is based tag-reading to get directive in 
			    an input file for BBS: ptt.cc
 Environment		: Ubuntu 9.10 (karmic), Kernel Linux 2.6.31-14-generic
 C Editor		: Vim 7.2.245, gedit 2.28.0
 Compiler		: gcc (Ubuntu 4.4.1-4ubuntu9) 4.4.1
 Version Control	: svn, version 1.6.5 (r38866)
 Project Hosting	: https://code.google.com/p/ptt-telnet-client/
 Licence		: GNU General Public License v3

 References		: 
 --------------------
 1) Non-printing ASCII characters: 
    http://www.physics.udel.edu/~watson/scen103/ascii.html
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h> // strtok, ...
#include <fcntl.h> // open, ...
#include <sys/types.h> // size_t, ...
#include <sys/socket.h>
#include <netdb.h>

/* Defines */
#define _XOPEN_SOURCE 500  // for usleep()
#define FILE_MODE (O_CREAT | O_WRONLY | O_APPEND)
#define BUFFER_SIZE 2048

/* Global Variables */
char *input_read_buffer = NULL;
char *content_read_buffer = NULL;
const char carriage_ret[] = "\r";
const char newline[] = "\n";
const char search_board[] = "s";	// s: search board
char ctrl_p = 16; 			// ctrl+p: prompt to post article
char ctrl_x = 24; 			// ctrl+x: post article
size_t input_buffer_size;
int socket_fd;
int output_fd;

/* Typedefs */

/* Structures */

/* Externs */

/* Function prototypes */
static void print_usage();
static void stream_file(const char *filename);
const char* clear_tag(char *buffer);
static void send_data(int iswrite, const char *tagname, const char *data, int len);
static void send_nonprinting_data(char *data, int len);
static void enter_username(char *buffer);
static void enter_password(char *buffer);
static void goto_board(char *buffer);
static void create_title(char *buffer);
static void write_content(char *buffer);
static void post_content();

/** 
 * main:
 * @argc: Number of command-line arguments
 * @argv: Pointer to the arguments list 
 * make a telnet connection, stream the input file, create an output file
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
		printf("Connection succesfull.\n\n");
	}
	
	/* open the file descriptor of output file */
	output_fd = open("output.txt", FILE_MODE); 

	/* stream the input file */
	stream_file(argv[1]);	

	/* Done, close output file */
	close(output_fd);	

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
 * @filename: input file name to parse
 * Parse the input file and get the directives (tags)
 */
static void
stream_file(const char *filename) {
	FILE *input_fp = NULL;
	int line_buffer_flag;

	/* Open input file to read directives */
	input_fp = fopen(filename, "r");
	
	/* Loop over the lines */
	while (!feof(input_fp)) {

		/* Memory allocation */
		input_buffer_size = BUFFER_SIZE * sizeof(char);
		input_read_buffer = malloc(input_buffer_size);

		if (fgets(input_read_buffer, input_buffer_size, input_fp) != NULL) {
	
			if (strncmp(input_read_buffer, "<ID>", sizeof(char)*4) == 0) {
				//enter_username(input_read_buffer); // input username in the login
				//printf("\nid: %s\n\n", input_read_buffer);
			}

			else if (strncmp(input_read_buffer, "<PASS>", sizeof(char)*6) == 0) {
				//enter_password(input_read_buffer); // input pwd in the login
				//printf("\npass: %s\n\n", input_read_buffer);
			}

			else if (strncmp(input_read_buffer, "<BOARD>", sizeof(char)*7) == 0) {
				//goto_board(input_read_buffer); // goto see the board
				//printf("\nboard: %s\n\n", input_read_buffer);
			}

			else if (strncmp(input_read_buffer, "<P>", sizeof(char)*3) == 0) {
				//create_title(input_read_buffer); // create title in the board
				//printf("\np: %s\n\n", input_read_buffer);
			}

			else if (strncmp(input_read_buffer, "<CONTENT>", sizeof(char)*9) == 0) {
				
				line_buffer_flag = 0;
				
				/* Send the content to the server using line-by-line buffering */ 
				while (line_buffer_flag == 0) {			
					write_content(input_read_buffer); // write one line to the content				
					
					if (line_buffer_flag == 0) 
						fgets(input_read_buffer, input_buffer_size, input_fp);

					if (strstr(input_read_buffer, "</CONTENT>") != NULL) {
						write_content(input_read_buffer); // write one line to the content
						//printf("1line: %s\n", input_read_buffer);
						line_buffer_flag = 1;
					}
				}

				/* Writing content is done, so trigger the post command */				
				if (line_buffer_flag == 1)
					post_content();
			}
		}
		
		free(input_read_buffer);
		input_read_buffer = NULL; // just in case; ignore double free
	}
	
	/* Done, close input file */
	fclose(input_fp);
}

/** 
 * clear_tag:
 * @buffer: <ID> ... </ID> 
 * Clears the tag strings and returns the element of the tag
 */
const char*
clear_tag(char *buffer) {
	char *feedback = NULL;
	int len;

	feedback = strtok(buffer, "<>");
	feedback = strtok(NULL, "<>");
	len = strlen(feedback);
	feedback[len+1] = '\0';	

	return feedback;
}

/** 
 * send_data:
 * @iswrite: a boolean to check for writing into output file
 * @tagname: input tags for output file's reference
 * @data: The data which is going to be sent to the server 
 * @len: Length of the data
 * Send the data to the server; it is a combination of input of data and 
 * input of a carriage return
 */
static void
send_data(int iswrite, const char *tagname, const char *data, int len) {

	printf("Sending data; [%s] ...\n", data);
	if (write(socket_fd, data, sizeof(char)*(len+1)) < 0) {
		perror("send_data");
		exit(1);
	}
	
	/* Input a carriage return character to complete data send */
	printf("Return.\n");	
	if (write(socket_fd, carriage_ret, sizeof(char)*1) < 0) {
		perror("send_data; sending carriage return");
		exit(1);
	
	/* Write result to the output file */
	} else {
		if (iswrite) {
			printf("Writing tagname [%s] to output file...\n", tagname);
			if (write(output_fd, tagname, sizeof(char)*(strlen(tagname))) < 0) {
				perror("send_data; tagname to output file");
				exit(1);
			}

			printf("Writing data; [%s] to output file...\n", data);
			if (write(output_fd, data, sizeof(char)*(len+1)) < 0) {
				perror("send_data; data to output file");
				exit(1);
			}

			printf("Appending new line...\n\n");
			if (write(output_fd, newline, sizeof(char)*1) < 0) {
				perror("send_data; appending new line in output file");
				exit(1);
			}
		}
	}	
}

/** 
 * send_nonprinting_data:
 * @data: The data which is going to be sent to the server 
 * @len: Length of the data
 * Send a non-printing data to the server; it is basically for a prompt, e.g.
 * create_title -> ctrl+p etc...
 */
static void
send_nonprinting_data(char *data, int len) {

	printf("Sending non-printing data; [%s] ...\n", data);
	if (write(socket_fd, &data, sizeof(char)*(len)) < 0) {
		perror("send_nonprinting_data; sending non-printing data");
		exit(1);
	}
	
	/* Input a carriage return character to complete data send */
	printf("Return.\n");	
	if (write(socket_fd, carriage_ret, sizeof(char)*1) < 0) {
		perror("send_nonprinting_data; carriage return");
		exit(1);
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
	
	username = clear_tag(buffer);
	username_len = strlen(username);
	printf("Extracted username: %s\n", username);
	
	/* Send the username data to the server */
	send_data(1, "ID: ", username, username_len);
	usleep(10000); // sleep for a while just in case
}

/** 
 * enter_password:
 * @buffer: <PASS> ... </PASS> 
 * Extract the password from tags and enter it in the login part
 */
static void
enter_password(char *buffer) {
	const char *password;
	const char *invisible_pass;	
	int password_len, i;
	
	password = clear_tag(buffer);
	password_len = strlen(password);

	printf("Extracted Password: %s\n", password);
	
	/* Send the password data to the server */
	send_data(1, "PASS: ", password, password_len);
	usleep(2000000); // sleep 2 sec to wait respond from server
	
	/* Return two times to reach user's main page */
	send_data(0, NULL, carriage_ret, 1);
	usleep(1000000);
	send_data(0, NULL, carriage_ret, 1);
	usleep(1000000);
}

/** 
 * goto_board:
 * @buffer: <BOARD> ... </BOARD> 
 * Extract the boardname from tags and goto see it
 */
static void
goto_board(char *buffer) {
	const char *boardname;	
	int boardname_len;
	
	boardname = clear_tag(buffer);
	boardname_len = strlen(boardname);

	printf("Going to [%s] board...\n", boardname);
	
	/* Send the boardname data to the server */
	send_data(0, NULL, boardname, boardname_len);
	usleep(2000000); // sleep 2 sec to wait respond from server

	/* Search for the board name */
	send_data(0, NULL, search_board, 1);
	usleep(1000000);
	/* Return to see the threads in the board */
	send_data(0, NULL, carriage_ret, 1);
	usleep(1000000);
}

/** 
 * create_title:
 * @buffer: <P> ... </P> 
 * Extract the title from tags and append it to create the content of it
 */
static void
create_title(char *buffer) {
	const char *title;	
	int title_len;
	
	title = clear_tag(buffer);
	title_len = strlen(title);

	printf("Creating title [%s] in the last board...\n", title);

	/* Prompt for new content creation */	
	write(socket_fd, &ctrl_p, 1);
	usleep(1000000);
	
	/* Send the title data to the server */
	send_data(0, NULL, title, title_len);
	usleep(2000000); // sleep 2 sec to wait respond from server
}

/** 
 * write_content:
 * @buffer: <CONTENT> ... </CONTENT> 
 * Extract the content from tags and append it as content input
 */
static void
write_content(char *buffer) {
	const char *content;	
	int content_len;
	
	if (strstr(input_read_buffer, "<CONTENT>") != NULL) {
		content = clear_tag(buffer);
		content_len = strlen(buffer);
	} else if ((strstr(input_read_buffer, "</CONTENT>") != NULL)) {
		content = strtok(buffer, "</>");
		content_len = strlen(content);
	} else {
		content = buffer;
		content_len = strlen(buffer);
	}

	printf("Writing line: [%s] as in the content...\n", content);
	
	/* Send the content data to the server */
	send_data(0, NULL, content, content_len);
	usleep(2000000); // sleep 2 sec to wait respond from server
}

/** 
 * post_content:
 * After input of content as writing it in the content part, 
 * trigger post command to complete the posting
 */
static void
post_content() {
	/* Content input is done */	
	write(socket_fd, &ctrl_p, 1);
	usleep(1000000);
	/* Save it and return */
	send_data(0, NULL, "s\r", 2);
	usleep(1000000); // sleep 2 sec to wait respond from server
}
