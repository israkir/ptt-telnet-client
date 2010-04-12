all:
	gcc ptt-telnet-client.c -o ptt-telnet-client

clean:
	rm -rf *.o ptt-telnet-client
