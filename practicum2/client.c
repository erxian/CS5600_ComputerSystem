#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include "client.h"
#include "config.h"

int connect_to_server()
{
  AppConfig config;
  if (read_config("config.ini", &config) < 0)
  {
    printf("Error reading config file.\n");
    return -1;
  }

  int socket_desc;
  struct sockaddr_in server_addr;
  char server_message[2000], client_message[2000];

  // Clean buffers:
  memset(server_message,'\0',sizeof(server_message));
  memset(client_message,'\0',sizeof(client_message));

  // Create socket:
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);

  if(socket_desc < 0){
    printf("Unable to create socket\n");
    return -1;
  }

  printf("Socket created successfully\n");

  // Set port and IP the same as server-side:
  server_addr.sin_family = AF_INET;
  //server_addr.sin_port = htons(2000);
  server_addr.sin_port = htons(config.server.port);
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  // Send connection request to server:
  if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
    printf("Unable to connect\n");
    return -1;
  }
  printf("Connected with server successfully\n"); 

  return socket_desc;
}

void get(const char *remote_file_path, const char *local_file_path)
{
  int socket_desc = connect_to_server();
  
  // Send the message to the server
  char message[8192];
  snprintf(message, sizeof(message), "GET %s", remote_file_path);
  send(socket_desc, message, strlen(message), 0);

  // Receive the file data and save it to the local file
  char buffer[8192];
  ssize_t bytes_received;
  FILE *local_file = fopen(local_file_path, "wb");
  while ((bytes_received = recv(socket_desc, buffer, sizeof(buffer), 0)) > 0)
  {
    fwrite(buffer, 1, bytes_received, local_file);
  }
  fclose(local_file);

  // Close the socket
  close(socket_desc);
}

void info(const char *remote_file_path)
{
  int socket_desc = connect_to_server();

  // Send the message to the server
  char message[8192];
  snprintf(message, sizeof(message), "INFO %s", remote_file_path);
  send(socket_desc, message, strlen(message), 0);

  // Receive the file information
  char buffer[8192];
  ssize_t bytes_received;
  if ((bytes_received = recv(socket_desc, buffer, sizeof(buffer), 0)) > 0)
  {
    buffer[bytes_received] = '\0';
    printf("File information:\n%s\n", buffer);
  }

  // Close the socket
  close(socket_desc);
}


void md(const char *remote_file_path)
{
  int socket_desc = connect_to_server();

  // Construct and send the command message
  char message[8192];
  snprintf(message, sizeof(message), "MD %s", remote_file_path);
  send(socket_desc, message, strlen(message), 0);

  // Receive the file information
  char buffer[8192];
  ssize_t bytes_received;
  if ((bytes_received = recv(socket_desc, buffer, sizeof(buffer), 0)) > 0)
  {
    buffer[bytes_received] = '\0';
    printf("Server response: %s\n", buffer);
  }

  // Close the socket
  close(socket_desc);
}

void put(const char *local_file_path, const char *remote_file_path) {
  int socket_desc = connect_to_server();

  char message[8192];
  memset(message, '\0', sizeof(message));

  snprintf(message, sizeof(message), "PUT %s", remote_file_path);
  send(socket_desc, message, strlen(message), 0);

  char server_response[2000];
  memset(server_response, '\0', sizeof(server_response));

  // Wait for the server to send the READY message
  if (recv(socket_desc, server_response, sizeof(server_response), 0) > 0 &&
      strcmp(server_response, "READY") == 0) {
  
    FILE *local_file = fopen(local_file_path, "rb");
    if (local_file == NULL) {
        perror("Error opening file");
        return;
    }

    char buffer[8192];
    memset(buffer, '\0', sizeof(buffer));
    ssize_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), local_file)) > 0) {
        send(socket_desc, buffer, bytes_read, 0);
    }

    // Send the end-of-file marker
    send(socket_desc, buffer, 0, 0);

    fclose(local_file);
    close(socket_desc);
  }
}

void rm(const char *file_path) {
  int socket_desc = connect_to_server();

  // Construct and send the command message
  char message[8192];
  snprintf(message, sizeof(message), "RM %s", file_path);
  send(socket_desc, message, strlen(message), 0);

  // Receive the file information
  char buffer[8192];
  ssize_t bytes_received;
  if ((bytes_received = recv(socket_desc, buffer, sizeof(buffer), 0)) > 0)
  {
    buffer[bytes_received] = '\0';
    printf("Server response: %s\n", buffer);
  }

  // Close the socket
  close(socket_desc);
}
