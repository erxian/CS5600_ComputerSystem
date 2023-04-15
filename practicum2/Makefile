all: server fget 

server: server.c
	gcc ini.c config.c server.c -o server

fget: fget.c
	gcc ini.c config.c client.c fget.c -o fget 

clean:
	rm server
	rm fget
