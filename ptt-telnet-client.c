/**
 ============================================================================
 Name			: ptt-telnet-client.c
 Author			: Hakki Caner KIRMIZI, #b96902133
 Description		: A C program for bulletin board system (ptt.cc) 
 			  based on tag-reading to get directive from 
			  an input file 
 Environment		: Ubuntu 9.10 (karmic), Kernel Linux 2.6.31-14-generic
 C Editor		: Vim 7.2.245, gedit 2.28.0
 Compiler		: gcc (Ubuntu 4.4.1-4ubuntu9) 4.4.1
 Integration Tests	: linux2.csie.ntu.edu.tw
 Version Control	: svn, version 1.6.5 (r38866)
 Project Hosting	: https://code.google.com/p/ptt-telnet-client/
 Licence		: GNU General Public License v3

 Instructions
 ------------
 Build/Compile		: make
 Run			: ./ptt-telnet-client [input-filename]
 Output			: stdout (all actions)

 Functions Implemented
 ---------------------
 1) Login/Logout
 2) Search board
 3) Post Article
 4) Send Email
 5) Send Instant message

 Notes 
 -----
 1) All tags are assumed one line (open and close in the same line), 
    except CONTENT.
 2) CONTENT tag must follow one of these conventions:
    <CONTENT> {anything comes here...} </CONTENT>
    -or-
    <CONTENT> {something}
	      {something more} </CONTENT>
    -or-
    <CONTENT>
    {something}
    {something more} </CONTENT>
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
size_t input_buffer_size;
char *input_read_buffer = NULL;
char *content_read_buffer = NULL;
const char carriage_ret[] = "\r";
const char newline[] = "\n";
const char space[] = " ";
const char search_char[] = "s";		// s: search board
const char message_char[] = "w";	// w: instant message someone
char ctrl_p = 16; 			// ctrl+p: prompt to post article
char ctrl_x = 24; 			// ctrl+x: post article
char ctrl_u = 21;			// ctrl+u: send realtime message
char left = 29;
int iscomplete;				// 0: No, 1: Yes
int content_mode;			// 0: post_content, 1: message_content, 2: mail_content
int socket_fd;
int output_fd;

/* Typedefs */

/* Structures */

/* Externs */

/* Function prototypes */
static void print_usage();
static void stream_file(const char *filename);
const char* clear_tag(char *buffer);
static void search();
static void send_space();
static void send_return();
static void send_message();
static void send_left();
static void send_data(int iswrite, const char *tagname, const char *data, int len);
static void enter_username(char *buffer);
static void enter_password(char *buffer);
static void goto_board(char *buffer);
static void create_article();
static void create_realtime_message(char *buffer);
static void create_mail(char *buffer);
static void post_article_title(char *buffer);
static void post_mail_title(char *buffer);
static void write_content(char *buffer);
static void post_content();
static void message_content();
static void mail_content();
static void logout();

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
	//output_fd = open("output.txt", FILE_MODE); 

	/* stream the input file */
	stream_file(argv[1]);

	/* Done, close output file */
	//close(output_fd);	
	
	/* Done, close socket */
	close(socket_fd);

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

	/* Open input file to read directives */
	input_fp = fopen(filename, "r");
	
	/* Loop over the lines */
	while (!feof(input_fp)) {

		/* Memory allocation */
		input_buffer_size = BUFFER_SIZE * sizeof(char);
		input_read_buffer = malloc(input_buffer_size);

		if (fgets(input_read_buffer, input_buffer_size, input_fp) != NULL) {
	
			if (strncmp(input_read_buffer, "<ID>", sizeof(char)*4) == 0) {
				enter_username(input_read_buffer);
			}

			else if (strncmp(input_read_buffer, "<PASS>", sizeof(char)*6) == 0) {
				enter_password(input_read_buffer);
			}

			else if (strncmp(input_read_buffer, "<BOARD>", sizeof(char)*7) == 0) {
				goto_board(input_read_buffer);
			}

			else if (strncmp(input_read_buffer, "<P>", sizeof(char)*3) == 0) {
				content_mode = 0;
				post_article_title(input_read_buffer);
			}
			
			else if (strncmp(input_read_buffer, "<W>", sizeof(char)*3) == 0) {
				content_mode = 1;
				create_realtime_message(input_read_buffer);
			}
			
			else if (strncmp(input_read_buffer, "<M>", sizeof(char)*3) == 0) {
				content_mode = 2;
				create_mail(input_read_buffer);
			}

			else if (strncmp(input_read_buffer, "<CONTENT>", sizeof(char)*9) == 0) {
				/* If the open/close tags are in the same line, then directly post than buffering line-by-line */
				if ((strstr(input_read_buffer, "</CONTENT>") != NULL) && (content_mode == 0)) {
					write_content(input_read_buffer);
					post_content();
					iscomplete = 0; // reset not to go over post_content again
				} 
				else if ((strstr(input_read_buffer, "</CONTENT>") != NULL) && (content_mode == 1)) {
					write_content(input_read_buffer);
					message_content();
					iscomplete = 0; // reset not to go over message_content again
				}
				else if ((strstr(input_read_buffer, "</CONTENT>") != NULL) && (content_mode == 2)) {
					write_content(input_read_buffer);
					mail_content();
					iscomplete = 0; // reset not to go over mail_content again
				}
				/* The code here process multiple lines of content using line-by-line buffering*/
				else {	
					iscomplete = 0; // 0: not completed, 1: completed
					
					/* Send the content to the server using line-by-line buffering */ 
					while (iscomplete == 0) {			
						write_content(input_read_buffer);
						
						if (iscomplete == 0) 
							fgets(input_read_buffer, input_buffer_size, input_fp);

						if (strstr(input_read_buffer, "</CONTENT>") != NULL) {
							write_content(input_read_buffer);
							iscomplete = 1;
						}
					}
				}
				
				/* If input content done, trigger the post command */				
				if ((iscomplete == 1) && (content_mode == 0)) {
					iscomplete = 0; // reset
					post_content();
				} 
				else if ((iscomplete == 1) && (content_mode == 1)) {
					iscomplete = 0; // reset
					message_content();
				}
				else if ((iscomplete == 1) && (content_mode == 2)) {
					iscomplete = 0; // reset
					mail_content();
				}
				
			}

			else if (strncmp(input_read_buffer, "<IP>", sizeof(char)*4) == 0) {
				//nothing for now;)
			}
			
			else if (strncmp(input_read_buffer, "<TITLE>", sizeof(char)*7) == 0) {
				post_mail_title(input_read_buffer);
			}

			else if (strncmp(input_read_buffer, "<EXIT>", sizeof(char)*6) == 0) {
				logout();
			}
			
			else {
				fprintf(stderr, "Unrecognized input directive: [%s] Exiting...", input_read_buffer);
				exit(1);
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

	return feedback;
}

/** 
 * search:
 * Trigger a prompt to search board or user
 */
static void
search() {
	printf("Prompting to search...\n");
	if (write(socket_fd, search_char, sizeof(char)*1) < 0) {
		perror("search board");
		exit(1);
	}
	usleep(100000);
}

/** 
 * send_space:
 * Sends a space character to the server
 */
static void
send_space() {
	printf("Sending space...\n");
	if (write(socket_fd, space, sizeof(char)*1) < 0) {
		perror("send space");
		exit(1);
	}
	usleep(1000000); // sleep 1 sec to wait respond from server
}

/** 
 * send_return:
 * Sends a carriage return character to the server
 */
static void
send_return() {	
	if (write(socket_fd, carriage_ret, sizeof(char)*1) < 0) {
		perror("send return");
		exit(1);
	}
	printf("Return.\n");
	usleep(1000000); // sleep 1 sec to wait respond from server
}

/** 
 * send_message:
 * Trigger a prompt to send realtime message
 */
static void
send_message() {
	printf("Prompting for sending message [CTRL+U]...\n");
	if (write(socket_fd, &ctrl_u, 1)< 0) {
		perror("send message");
		exit(1);
	}
	usleep(100000);
}

/** 
 * send_left:
 * Send a left character signal to the server
 */
static void
send_left() {
	printf("Sending left...\n");
	if (write(socket_fd, &left, 1) < 0) {
		perror("send_left");
		exit(1);
	}
	usleep(100000);
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

	printf("Sending data: %s ...\n", data);
	if (write(socket_fd, data, sizeof(char)*(len+1)) < 0) {
		perror("send_data");
		exit(1);
	}
	
	/* Write result to the output file */
	else {
		if (iswrite) {
			printf("Writing tagname [%s] to output file...\n", tagname);
			if (write(output_fd, tagname, sizeof(char)*(strlen(tagname))) < 0) {
				perror("send_data: tagname to output file");
				exit(1);
			}

			printf("Writing data: [%s] to output file...\n", data);
			if (write(output_fd, data, sizeof(char)*(len+1)) < 0) {
				perror("send_data: data to output file");
				exit(1);
			}

			printf("Appending new line...\n\n");
			if (write(output_fd, newline, sizeof(char)*1) < 0) {
				perror("send_data: appending new line in output file");
				exit(1);
			}
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
	
	username = clear_tag(buffer);
	username_len = strlen(username);
	printf("Extracted username: %s\n", username);
	
	/* Send the username data to the server */
	printf("Sending username: [%s]...\n", username);
	send_data(0, NULL, username, username_len);
	send_return();
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
	printf("Sending password: [%s]...\n", password);
	send_data(0, NULL, password, password_len);
	send_return();
	
	/* Send space char at least two times to reach user's main page */
	send_space();
	send_space();
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

	/* Trigger search prompt */
	search();
	
	/* Send the boardname data to the server */
	printf("Searching board: [%s]...\n", boardname);
	send_data(0, NULL, boardname, boardname_len);
	send_return();
	
	/* Send space at least once to see the threads in the board */
	send_space();
}

/** 
 * create_article:
 * Trigger a prompt to create a new article in the board
 */
static void
create_article() {
	printf("Prompting for create article...\n");
	if (write(socket_fd, &ctrl_p, 1)< 0) {
		perror("create article");
		exit(1);
	}
	usleep(100000);
}

/** 
 * create_realtime_message:
 * @buffer: <W> ... </W> 
 * Extract the ID from tags and send that ID a realtime message
 */
static void
create_realtime_message(char *buffer) {
	const char *messageto;
	int messageto_len;
	
	messageto = clear_tag(buffer);
	messageto_len = strlen(messageto);
	printf("Messageto: %s\n", messageto);
	
	/* Prompt to send message */
	send_message();
	
	/* Search messageto */
	search();
	
	/* Input messageto */
	printf("Sending messageto: %s\n", messageto);
	send_data(0, NULL, messageto, messageto_len);
	send_return();
	
	printf("Prompting for sending message [w]...\n");
	if (write(socket_fd, message_char, 1)< 0) {
		perror("message content");
		exit(1);
	}
	usleep(100000);
}

/** 
 * create_mail:
 * @buffer: <M> ... </M> 
 * Extract the user ID from tags and create a mail for it
 */
static void
create_mail(char *buffer) {
	const char *mailto;	
	int mailto_len;
	
	mailto = clear_tag(buffer);
	mailto_len = strlen(mailto);

	printf("Creating mail to %s...\n", mailto);	
	
	/* Go back to main menu */	
	printf("Going back to main menu...\n");
	send_left();
	send_left();
	send_data(0, NULL, "qqqqqqqqqqM", 12);
	send_return();
	
	/* Click Send */
	send_data(0, NULL, "s", 12);
	send_return();
	
	send_data(0, NULL, mailto, mailto_len);
	send_return();
}

/** 
 * post_article_title:
 * @buffer: <P> ... </P> 
 * Extract the title from tags and append it to create the content of it
 */
static void
post_article_title(char *buffer) {
	const char *title;	
	int title_len;
	
	title = clear_tag(buffer);
	title_len = strlen(title);

	/* Prompt for new article creation */	
	create_article();

	printf("Posting article title: [%s] in the last board...\n", title);
	
	/* Ignore category selection */
	send_return();
		
	/* Send the title data to the server */
	send_data(0, NULL, title, title_len);
	send_return();
}

/** 
 * post_mail_title:
 * @buffer: <TITLE> ... </TITLE> 
 * Extract the title of the mail and input it
 */
static void
post_mail_title(char *buffer) {
	const char *title;	
	int title_len;
	
	title = clear_tag(buffer);
	title_len = strlen(title);
	
	printf("Posting mail title: [%s]...\n", title);
	send_data(0, NULL, title, title_len);
	send_return();
}

/** 
 * write_content:
 * @buffer: one line buffered content
 * If need, extract the content from tags and append it as content input
 */
static void
write_content(char *buffer) {
	const char *content;	
	int content_len;
	
	/* Search buffer in case there is a tag and clean it */
	if (strstr(input_read_buffer, "<CONTENT>") != NULL) {
		content = clear_tag(buffer);
		content_len = strlen(content);
	} else if ((strstr(input_read_buffer, "</CONTENT>") != NULL)) {
		content = strtok(buffer, "</>");
		content_len = strlen(content);
	} else {
		content = buffer;
		content_len = strlen(buffer);
	}

	//printf("Writing content line: [%s] as in the content...\n", content);
	
	/* Send the content data to the server */
	send_data(0, NULL, content, content_len);
	send_return();
}

/** 
 * post_content:
 * Post the article content to the board
 */
static void
post_content() {
	printf("Prompting for saving the content [CTRL+X]...\n");
	if (write(socket_fd, &ctrl_x, 1) < 0) {
		perror("post content");
		exit(1);
	}
	usleep(100000);
	
	/* Save it and return */
	send_data(0, NULL, "s", 1);
	send_return();
	send_space();
	send_space();
}

/** 
 * message_content:
 * Message the content to send the user instantly
 */
static void
message_content() {
	/* Until here input of the message + Return is done,
	   so, just confirm sending by 'y' */
	send_data(0, NULL, "y", 1);
	send_return();
}

/** 
 * mail_content:
 * Mail the content to the user
 */
static void
mail_content() {
	printf("Prompting for saving the content [CTRL+X]...\n");
	if (write(socket_fd, &ctrl_x, 1) < 0) {
		perror("mail content");
		exit(1);
	}
	usleep(100000);
	
	/* Save it and return */
	send_data(0, NULL, "s", 1);
	send_return();
	usleep(1000000);
	//send_data(0, NULL, "n", 1);
	send_return();
	usleep(1000000);
	send_space();
}

/** 
 * logout:
 * Log out from ptt.cc
 */
static void
logout() {
	printf("Logging out...\n");
	
	/* If still in mail, this is the way back to main user menu */
	send_left();
	send_left();
	
	/* Send bunch of 'q's to go back to main user menu and one G character to 
	   select Goodbye */
	send_data(0, NULL, "qqqqqqqqqqG", 12);
	send_return();
	
	/* Confirm logging out */
	send_data(0, NULL, "y", 1);
	send_return();
	
	send_data(0, NULL, "G", 1);
	send_return();
	send_return();
}
