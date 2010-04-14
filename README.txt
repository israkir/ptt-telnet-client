 Name			: ptt-telnet-client.c
 Author			: Hakki Caner KIRMIZI, #b96902133
 Description		: A C program for bulletin board system (ptt.cc) based 
			  on tag-reading to get directive from an input file 
 Environment		: Ubuntu 9.10 (karmic), Kernel Linux 2.6.31-14-generic
 C Editor		: Vim 7.2.245, gedit 2.28.0
 Compiler		: gcc (Ubuntu 4.4.1-4ubuntu9) 4.4.1
 Integration Tests	: linux2.csie.ntu.edu.tw
 Version Control	: svn, version 1.6.5 (r38866)
 Project Hosting	: https://code.google.com/p/ptt-telnet-client/
 Licence		: GNU General Public License v3

 Instructions
 ============
 Build/Compile		: make
 Run			: ./ptt-telnet-client [input-filename]

 Functions Implemented
 =====================
 I did not implement any function to create an output file; e.g. read mail etc...
 However, I implemented send mail and send realtime message functions.

 1) Login/Logout
 ---------------
 Login starts with <ID></ID> tag to input username and <PASS></PASS> to input
 password. During the execution of the code both of them VISIBLE; i.e. printed
 stdout. So, be aware of this.

 2) Search board
 ---------------
 Searching for board is triggered when <BOARD></BOARD> tag is met. The result
 of this directive is the page where all threads are listed in the board.

 3) Post Article
 ---------------
 Post article is triggered when <P></P> tag is met. There are some assumptions:
 * In order to post an article, first you have to have at least one BOARD tag
   just before P tag.
 * In input file, in the following of P tag, there has to be a <CONTENT></CONTENT>
   tag, even though it does not contain any value within.
 NOTE: I tested this, and works pretty much well!!! 

 4) Send Email
 -------------
 Send email is triggered when <M></M> tag is met. There are some assumptions:
 * There has to be a <TITLE></TITLE> tag just after this M tag, even though it
   does not contain any value inside.
 * There has to be a <CONTENT></CONTENT> tag after M tag, even though it does not
   contain any value.
 NOTE: I tested this, but there is a problem. I could not find it out why it does 
       not send mail properly, although I 'exactly' follow the ptt.cc directive 
       conventions in PieTTY. I assume, if there is a problem about my method, it
       should also not work for 'Post Article' part. A probable mistake may in one
	   of these functions:

	static void create_mail(char *buffer): 
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	is called when M tag is parsed. It basically goes to main menu (by sending 
	some 'q' sequence data to the server, and some left char data, in case user
	is in somewhere which 'q' does not work to go back) and inputs an 'm' to
	point 'Mail' section and clicks (returns). Then inputs an 's' to point 'Send'
	section and returns. Finally, it inputs the ID of receiver and returns.

	static void post_mail_title(char *buffer):
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	is called when Title tag is parsed. It just inputs the value extracted from
	the tags and returns.
	
	static void write_content(char *buffer):
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	is called when Content tag is met. It inputs the value between tag line-by-
	line.

	static void mail_content();
	~~~~~~~~~~~~~~~~~~~~~~~~~~~
	is called after input content is done (it basically uses a flag based system
	to know input is finished or not, while parsing the tags in 'stream_file()'
	function). It triggers CTRL+X to save the content, sends an 's' to confirm it
	and returns two times -skipping save mail option-. At the end it sends a
	space character signal to the server to go back to Mail menu again.

	
 5) Send Instant message
 -----------------------
 This is triggered when <W></W> tag is met. There are some assumptions:
 * There has to be a Content tag just after this tag, even though it does not
   contain any value.
 NOTE: I did not test this. Hopefully it works:)

 Notes
 =====
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
 
 3) I used PieTTY (v0.3.27) to follow/code the ptt.cc directions.
