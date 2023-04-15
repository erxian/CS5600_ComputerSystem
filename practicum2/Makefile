all: server fget multi_test

server: server.c
	gcc ini.c config.c server.c -o server

fget: fget.c
	gcc ini.c config.c client.c fget.c -o fget 

multi_test: multithread_test.c
	gcc multithread_test.c -o multi_test

clean:
	rm -f server fget multi_test *.o 
	rm -rf .vscode
