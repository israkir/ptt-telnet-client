Instructions
============
Build/Compile: 
    
    $ make
Run: 

    $ ./ptt-telnet-client [input-tag-file]

Functions Implemented
=====================

1. Login/Logout: Login starts with `<ID></ID>` tag to input username and `<PASS></PASS>` to input 
password. During the execution of the code both of them VISIBLE; i.e. printed stdout. So, be aware 
of this.

2. Search board: Searching for board is triggered when `<BOARD></BOARD>` tag is met. The result of 
this directive is the page where all threads are listed in the board.

3. Post Article: Post article is triggered when `<P></P>` tag is met. There are some assumptions:
    * In order to post an article, first you have to have at least one BOARD tag just before P tag.
    * In input file, in the following of P tag, there has to be a `<CONTENT></CONTENT>` tag, even 
      though it does not contain any value within.

4. Send Email: Send email is triggered when `<M></M>` tag is met. There are some assumptions:
    * There has to be a `<TITLE></TITLE>` tag just after this M tag, even though it does not contain 
      any value inside.
    * There has to be a `<CONTENT></CONTENT>` tag after M tag, even though it does not contain any 
      value.

5. Send Instant message: This is triggered when `<W></W>` tag is met. There are some assumptions:
    * There has to be a Content tag just after this tag, even though it does not contain any value.

Notes
=====
1. All tags are assumed one line (open and close in the same line), except CONTENT.

2. CONTENT tag must follow one of these conventions:

    <CONTENT> {anything comes here...} </CONTENT>
    -or-
    <CONTENT> {something}
          {something more} </CONTENT>
    -or-
    <CONTENT>
    {something}
    {something more} </CONTENT>

3. I used PieTTY (v0.3.27) to test the ptt.cc tag directives in this project.

-----------------------------

A sample input file would be like this:

    <ID>CCF</ID>
    <PASS>*****</PASS>
    <BOARD>Gossiping</BOARD>
    <BOARD>Hate</BOARD> 
    <BOARD>Joke</BOARD>
    <BOARD>Test</BOARD>
    <P>TEST!!!!!</P>
    <CONTENT>This is a testing. 
                                  by israkir</CONTENT>
    <P>TEST Again!!!!!</P>
    <CONTENT>This is a testing2. 
                                  by israkir</CONTENT>
    <IP>NTUTA</IP>
    <W>CCF</W>
    <CONTENT>hihi  XD</CONTENT>
    <W>CCF</W>
    <CONTENT>see you~</CONTENT>
    <M>CCF</M>
    <TITLE>A mail</TITLE>
    <CONTENT>I send a mail.</CONTENT>
    <M>NTUTA</M>
    <TITLE>A mail2</TITLE>
    <CONTENT>I send a mail 2.</CONTENT>
    <EXIT>