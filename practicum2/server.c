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
#include <pthread.h>
#include <ftw.h>
#include <pwd.h>
#include <grp.h>

typedef struct {
  int client_sock;
  AppConfig config;
} ClientData;

// Signal handler for SIGINT
void sigint_handler(int sig_num) {
    printf("\nCaught signal %d, shutting down the server...\n", sig_num);
    exit(0);
}

/**
 * send_file - Sends a file to the client through the given client socket
 *
 * @param: client_sock - The client socket to send the file through
 * @param: file_path - The path of the file to be sent
 */
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

/**
 * send_file_info - Sends information about a file to the client through the given client socket
 *
 * @param: client_sock - The client socket to send the file information through
 * @param: file_path - The path of the file to get information about
 */
void send_file_info(int client_sock, const char *file_path)
{
  struct stat file_stat;
  if (stat(file_path, &file_stat) < 0)
  {
    perror("Error getting file information");
    return;
  }

  // Get user and group information
  struct passwd *user_info = getpwuid(file_stat.st_uid);
  struct group *group_info = getgrgid(file_stat.st_gid);

  char file_info[8196];
  snprintf(
      file_info,
      sizeof(file_info),
      "Size: %lld bytes\n"
      "Last modified: %s"
      "Permissions: %o\n"
      "Owner: %s\n"
      "Group: %s\n",
      (long long)file_stat.st_size,
      ctime(&file_stat.st_mtime),
      file_stat.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO),
      user_info->pw_name,
      group_info->gr_name);

  send(client_sock, file_info, strlen(file_info), 0);
}

/**
 * handle_md - Handles the MD command from the client, creating a new directory with the specified path
 *
 * @param: client_sock - The client socket to communicate with
 * @param: dir_path - The path of the directory to create
 */
void handle_md(int client_sock, char *dir_path) {
    char server_message[2000];
    memset(server_message, '\0', sizeof(server_message));

    if (mkdir(dir_path, 0777) == 0) {
        snprintf(server_message, sizeof(server_message), "Directory '%s' created successfully.", dir_path);
    } else {
        snprintf(server_message, sizeof(server_message), "Failed to create directory '%s'.", dir_path);
    }

    send(client_sock, server_message, strlen(server_message), 0);
}

// Helper function to remove files and directories using nftw
static int remove_item(const char *path, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    // Ignore hidden files with "._" prefix
    if (strstr(path, "/._") != NULL) {
        return 0;
    }

    // Remove the file or directory at the given path
    int result = remove(path);
    if (result) {
        perror(path);
    }
    return result;
}

/**
 * is_path_accessible - Checks if a given path is accessible
 *
 * @param: path - The path to check for accessibility
 * @return: non-zero if the path is accessible, zero otherwise
 */
int is_path_accessible(const char *path)
{
  struct stat buffer;
  return (stat(path, &buffer) == 0);
}

/**
 * get_primary_usb_path - Returns the path of the primary USB device
 *
 * @param: config - Pointer to the AppConfig structure containing USB paths
 * @return: pointer to the path of the primary USB device
 */
const char *get_primary_usb_path(AppConfig *config)
{
  if (is_path_accessible(config->usb.last_primary_usb))
  {
    return config->usb.last_primary_usb;
  }
  else if (is_path_accessible(config->usb.volume_path_1))
  {
    strcpy(config->usb.last_primary_usb, config->usb.volume_path_1);
    return config->usb.volume_path_1;
  }
  else
  {
    strcpy(config->usb.last_primary_usb, config->usb.volume_path_2);
    return config->usb.volume_path_2;
  }
}

/**
 * sync_usb_devices - Syncs the content between the primary and secondary USB devices
 *
 * @param: config - Pointer to the AppConfig structure containing USB paths
 */
void sync_usb_devices(AppConfig *config)
{
  // Define paths for both primary and secondary USB devices
  char primary_path[4096], secondary_path[4096];

  snprintf(primary_path, sizeof(primary_path), "%s", config->usb.volume_path_1);
  snprintf(secondary_path, sizeof(secondary_path), "%s", config->usb.volume_path_2);

  // Check which USB is currently accessible
  int primary_accessible = is_path_accessible(config->usb.volume_path_1);
  int secondary_accessible = is_path_accessible(config->usb.volume_path_2);

  // If both USB devices are accessible, perform syncing
  if (primary_accessible && secondary_accessible) {
    char sync_cmd[8192];

    // Sync the contents of the last used primary USB to the other USB
    if (strcmp(config->usb.last_primary_usb, config->usb.volume_path_1) == 0) {
      snprintf(sync_cmd, sizeof(sync_cmd), "rsync -a --delete --exclude '.Trashes' --exclude '._*' %s/ %s/", primary_path, secondary_path);
    } else {
      snprintf(sync_cmd, sizeof(sync_cmd), "rsync -a --delete --exclude '.Trashes' --exclude '._*' %s/ %s/", secondary_path, primary_path);
    }

    system(sync_cmd);
  }

  // Update the last used primary USB path
  if (primary_accessible) {
    strcpy(config->usb.last_primary_usb, config->usb.volume_path_1);
  } else if (secondary_accessible) {
    strcpy(config->usb.last_primary_usb, config->usb.volume_path_2);
  }
}

/**
 * handle_rm - Handles the RM command from the client, removing the specified file or directory
 *
 * @param: client_sock - The client socket to communicate with
 * @param: path - The path of the file or directory to remove
 * @param: config - The server configuration containing USB paths
 */
void handle_rm(int client_sock, char *path, AppConfig config) {
    char server_message[2000];
    memset(server_message, '\0', sizeof(server_message));

    // Check if the path is a directory
    struct stat path_stat;
    stat(path, &path_stat);

    if (S_ISDIR(path_stat.st_mode)) {
        // Use nftw to remove directories and their contents
        if (nftw(path, remove_item, 64, FTW_DEPTH | FTW_PHYS) == 0) {
            strcpy(server_message, "Directory and its contents removed successfully.");
        } else {
            strcpy(server_message, "Failed to remove directory and its contents.");
        }
    } else {
        // Remove a single file
        if (remove(path) == 0) {
            strcpy(server_message, "File removed successfully.");
        } else {
            strcpy(server_message, "Failed to remove file.");
        }
    }

    send(client_sock, server_message, strlen(server_message), 0);
    sync_usb_devices(&config);
}

/**
 * handle_put - Handles the PUT command from the client, saving the received
 * file to the specified file path
 *
 * @param: client_sock - The client socket to communicate with
 * @param: file_path - The path to save the received file
 * @param: client_message - The client message containing the PUT command and additional info
 */
void handle_put(int client_sock, char *file_path, char *client_message) {
    char server_message[2000];
    char buffer[8192];
    memset(server_message, '\0', sizeof(server_message));
    memset(buffer, '\0', sizeof(buffer));
    
    // Send a confirmation message to the client to start sending the file
    strcpy(server_message, "READY");
    send(client_sock, server_message, strlen(server_message), 0);

    // Receive the file data from the client and save it to the remote file
    ssize_t bytes_received;
    FILE *remote_file = fopen(file_path, "wb");

    while ((bytes_received = recv(client_sock, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, 1, bytes_received, remote_file);
    }

    fclose(remote_file);
    //strcpy(server_message, "File or directory put successfully.");
    //send(client_sock, server_message, strlen(server_message), 0);
}

/**
 * handle_client - Thread function to handle a single client connection
 *
 * @param: arg - Pointer to a ClientData structure containing the client socket and server configuration
 * @return: NULL
 */
void *handle_client(void *arg)
{
  long int thread_id = (long int)pthread_self();
  printf("A new client thread ID: %ld has been created\n", thread_id);

  ClientData *client_data = (ClientData *)arg;
  int client_sock = client_data->client_sock;
  AppConfig config = client_data->config;

  char server_message[8196], client_message[8196];

   // Clean buffers:
  memset(server_message, '\0', sizeof(server_message));
  memset(client_message, '\0', sizeof(client_message));

  // Receive client's message:
  if (recv(client_sock, client_message, sizeof(client_message), 0) < 0)
  {
    printf("Couldn't receive\n");
    return NULL;
  }

  // Parse the command and file path from the client message
  char command[16], file_path[4096], full_path[4608];
  sscanf(client_message, "%15s %4095s", command, file_path);

  // Combine the primary USB volume path and the file path from the client message
  const char *primary_usb_path = get_primary_usb_path(&client_data->config);
  printf("last primary usb path %s\n", primary_usb_path);
  snprintf(full_path, sizeof(full_path), "%s/%s", primary_usb_path, file_path);

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
    handle_rm(client_sock, full_path, client_data->config);
  }
  else
  {
    printf("Unknown command: %s\n", command);
  }
  // Close the client socket after serving the request
  close(client_sock);

  // Free the memory allocated for the client data
  free(client_data);

  // Exit the thread
  pthread_exit(NULL);
}

/**
 * sync_usb_devices_thread - A separate thread that periodically calls
 * sync_usb_devices to sync the content between USB devices
 *
 * @param: arg - Pointer to the AppConfig structure containing USB paths
 * @return: NULL
 */
void *sync_usb_devices_thread(void *arg) {
  AppConfig config = *(AppConfig *)arg;

  while (1) {
    sync_usb_devices(&config);
    sleep(10);
  }

  return NULL;
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

  // Create and start the sync thread
  pthread_t sync_thread;
  int sync_result = pthread_create(&sync_thread, NULL, sync_usb_devices_thread, (void *)&config);
  if (sync_result != 0) {
    printf("Failed to create sync thread\n");
    return -1;
  }

  int socket_desc,client_sock;

  struct sockaddr_in server_addr, client_addr;
  // Create socket:
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);

  if(socket_desc < 0){
    printf("Error while creating socket\n");
    return -1;
  }
  printf("Socket created successfully\n");

  // Set port and IP:
  server_addr.sin_family = AF_INET;
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
    socklen_t client_size = sizeof(client_addr);
    client_sock = accept(socket_desc, (struct sockaddr*)&client_addr, &client_size);

    if (client_sock < 0){
      printf("Can't accept\n");
      return -1;
    }

    printf("Client connected at IP: %s and port: %i\n",
           inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port));

    // Allocate memory for client data and set the values
    ClientData *client_data = (ClientData *)malloc(sizeof(ClientData));
    client_data->client_sock = client_sock;
    client_data->config = config;

    // Create a new thread for each client connection
    pthread_t client_thread;
    int result;
    result = pthread_create(&client_thread, NULL, handle_client, (void *)client_data);
    if (result != 0) {
      printf("Failed to assgin a new thread to client\n");
      return -1;
    }
    // Detach the thread so its resources are released when it finishes
    pthread_detach(client_thread);
    if (result != 0) {
      printf("Failed to detach the thread\n");
      return -1;
    }
  }
  // Closing the server socket (this will not be reached unless the loop is broken)
  close(socket_desc);

  return 0;
}

