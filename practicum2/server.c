#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>
#include "config.h"

// Signal handler for SIGINT
void sigint_handler(int sig_num) {
    printf("\nCaught signal %d, shutting down the server...\n", sig_num);
    exit(0);
}

void send_file(int client_sock, const char *file_path)
{
  char buffer[8196];
  FILE *file = fopen(file_path, "rb");
  if (file == NULL)
  {
    perror("Error opening file");
    return;
  }

  ssize_t bytes_read;
  while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0)
  {
    send(client_sock, buffer, bytes_read, 0);
  }

  fclose(file);
}

void send_file_info(int client_sock, const char *file_path)
{
  struct stat file_stat;
  if (stat(file_path, &file_stat) < 0)
  {
    perror("Error getting file information");
    return;
  }

  char file_info[8196];
  snprintf(
      file_info,
      sizeof(file_info),
      "Size: %lld bytes\n"
      "Last modified: %s"
      "Permissions: %o\n",
      (long long)file_stat.st_size,
      ctime(&file_stat.st_mtime),
      file_stat.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO));

  send(client_sock, file_info, strlen(file_info), 0);
}

void handle_md(int client_sock, char *dir_path) {
    char server_message[2000];
    memset(server_message, '\0', sizeof(server_message));

    if (mkdir(dir_path, 0777) == 0) {
        strcpy(server_message, "Directory created successfully.");
    } else {
        strcpy(server_message, "Failed to create directory.");
    }

    send(client_sock, server_message, strlen(server_message), 0);
}

void handle_rm(int client_sock, char *path) {
    char server_message[2000];
    memset(server_message, '\0', sizeof(server_message));

    if (remove(path) == 0) {
        strcpy(server_message, "File or directory removed successfully.");
    } else {
        strcpy(server_message, "Failed to remove file or directory.");
    }

    send(client_sock, server_message, strlen(server_message), 0);
}

void handle_put(int client_sock, char *file_path, char *client_message) {
    char server_message[2000];
    memset(server_message, '\0', sizeof(server_message));

    // Receive the file data from the client and save it to the remote file
    char buffer[8192];
    ssize_t bytes_received;
    FILE *remote_file = fopen(file_path, "wb");

    while ((bytes_received = recv(client_sock, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, 1, bytes_received, remote_file);
    }
    
    fclose(remote_file);
    strcpy(server_message, "File or directory put successfully.");
    send(client_sock, server_message, strlen(server_message), 0);
}

int main(void)
{
  // Register the SIGINT handler
  signal(SIGINT, sigint_handler);

  AppConfig config;
  if (read_config("config.ini", &config) < 0)
  {
    printf("Error reading config file.\n");
    return -1;
  }

  int socket_desc, client_sock;
  socklen_t client_size;
  struct sockaddr_in server_addr, client_addr;
  char server_message[8196], client_message[8196];

   // Clean buffers:
  memset(server_message, '\0', sizeof(server_message));
  memset(client_message, '\0', sizeof(client_message));

  // Create socket:
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);

  if(socket_desc < 0){
    printf("Error while creating socket\n");
    return -1;
  }
  printf("Socket created successfully\n");

  // Set port and IP:
  server_addr.sin_family = AF_INET;
  //server_addr.sin_port = htons(2000);
  server_addr.sin_port = htons(config.server.port);
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  // Bind to the set port and IP:
  if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
    printf("Couldn't bind to the port\n");
    return -1;
  }
  printf("Done with binding\n");

  // Listen for clients:
  if(listen(socket_desc, 1) < 0){
    printf("Error while listening\n");
    return -1;
  }
  printf("\nListening for incoming connections.....\n");

  while(1)
  {
    // Accept an incoming connection:
    client_size = sizeof(client_addr);
    client_sock = accept(socket_desc, (struct sockaddr*)&client_addr, &client_size);

    if (client_sock < 0){
      printf("Can't accept\n");
      return -1;
    }

    printf("Client connected at IP: %s and port: %i\n",
           inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port)); 

    // Receive client's message:
    if (recv(client_sock, client_message, sizeof(client_message), 0) < 0)
    {
      printf("Couldn't receive\n");
      return -1;
    }
    // Parse the command and file path from the client message
    char command[16], file_path[4096], full_path[4608];
    sscanf(client_message, "%15s %4095s", command, file_path);

    // Combine the USB volume path and the file path from the client message
    snprintf(full_path, sizeof(full_path), "%s/%s", config.usb.volume_path, file_path);

    // Handle the GET and INFO commands
    if (strcmp(command, "GET") == 0)
    {
      send_file(client_sock, full_path);
    }
    else if (strcmp(command, "INFO") == 0)
    {
      send_file_info(client_sock, full_path);
    }
    else if (strcmp(command, "MD") == 0)
    {
      handle_md(client_sock, full_path);
    }
    else if (strcmp(command, "PUT") == 0)
    {
      handle_put(client_sock, full_path, client_message);
    }
    else if (strcmp(command, "RM") == 0)
    {
      handle_rm(client_sock, full_path);
    }
    else
    {
      printf("Unknown command: %s\n", command);
    }

    // Close the client socket after serving the request
    close(client_sock);
  }

  // Closing the server socket (this will not be reached unless the loop is broken)
  close(socket_desc);

  return 0;
}
