# AUTHORS

Zengping Xu, Mingxin Zhang

# DESCRIPTION

This project implements a remote USB communication server that allows clients to access and manipulate files stored on USB devices connected to the server machine. It uses a custom protocol to handle commands from clients and perform the corresponding actions on the server-side.

# IMPLEMENTATION OVERVIEW

The server listens on a specified TCP port and waits for incoming client connections. In our implementation, upon receiving a connection request, the server spawns a new thread to handle the client's commands, so it can service multiple clients simultaneously. 

The server also supports a simple mirrored file system, we'll use 2 USB devices, one for Primary and another for Secondary, all read and write operations will first execute on Primary USB, and then the write operation will be synced to Secondary with a seperate thread. This sync thread willsync every 10 seconds. If the Primary USB disconnected, the program will vote Secondary to become new Primary, therefore the service still works even though one USB failed. Once the failed USB reconnected, it will become Secondary automatically and sync updates from the current Primary.

# USAGE

1. Edit the server configuration file (config.txt) to specify the allowed USB paths for client access. Edit the port to specify the connection port

2. Edit the ip addr in client.c and server.c by editting server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

2. Compile the server with the provided Makefile by running make.
$ make clean
$ make

3. Start the server by running ./server 
$ ./server

4. Start the client by running ./fget INFO file.txt, make sure there is a file.txt at Remote USB
$ ./fget INFO file.txt

5. Start testing multi-thread
$ ./multi_test

6. To stop the server, press Ctrl+C in the terminal where the server is running.


