#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"

/**
 * exec_get - Downloads a file from the server to the local file system.
 *
 * @param: remote_file_path - The path of the file on the server
 * @param: local_file_path - The path of the file on the local system
 */
void exec_get(const char *remote_file_path, const char *local_file_path)
{
  get(remote_file_path, local_file_path);
}

/**
 * exec_info - Requests and displays the information of a file from the server.
 *
 * @param: remote_file_path - The path of the file on the server
 */
void exec_info(const char *remote_file_path)
{
  info(remote_file_path);
}

/**
 * exec_md - Sends a request to the server to create a new directory.
 *
 * @param: remote_file_path - The path of the new directory on the server
 */
void exec_md(const char *remote_file_path) {
    md(remote_file_path);
}

/**
 * exec_put - Uploads a file from the local file system to the server.
 *
 * @param: local_file_path - The path of the file on the local system
 * @param: remote_file_path - The path of the file on the server
 */
void exec_put(const char *local_file_path, const char *remote_file_path) {
    put(local_file_path, remote_file_path);
}

/**
 * exec_rm - Sends a request to the server to remove a file.
 *
 * @param: remote_file_path - The path of the file to be removed on the server
 */
void exec_rm(const char *remote_file_path) {
    rm(remote_file_path);
}

int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    printf("Usage: %s command remote_file_path [local_file_path]\n", argv[0]);
    return -1;
  }

  const char *command = argv[1];

  if (strcmp(command, "GET") == 0)
  {
    const char *remote_file_path;
    const char *local_file_path;
    if(argc == 3){
      // Find the last occurrence of '/' in the path string, return 'NULL' if no occurence of '/'
      local_file_path = strrchr(argv[2], '/');; 
      if(local_file_path == NULL){
        local_file_path = argv[2];
      }else{
        //omit the leading '/'
        local_file_path++;
      }
      remote_file_path = argv[2];
    }else if(argc == 4){
      local_file_path = argv[2]; 
      remote_file_path = argv[3];
    }else{
      printf("Usage: %s GET (optional)local_file_path remote_file_path\n", argv[0]);
      return -1;
    }
    exec_get(remote_file_path, local_file_path);
  }
  else if (strcmp(command, "INFO") == 0)
  {
    const char *remote_file_path = argv[2];
    exec_info(remote_file_path);
  }
  else if (strcmp(argv[1], "MD") == 0)
  {
    const char *remote_file_path = argv[2];
    exec_md(remote_file_path);
  }
  else if (strcmp(argv[1], "PUT") == 0)
  {
    const char *remote_file_path;
    const char *local_file_path;
    if(argc == 3){
      // Find the last occurrence of '/' in the path string, return 'NULL' if no occurence of '/'
      remote_file_path = strrchr(argv[2], '/');; 
      if(remote_file_path == NULL){
        remote_file_path = argv[2];
      }else{
        //omit the leading '/'
        remote_file_path++;
      }
      local_file_path = argv[2];
    }else if(argc == 4){
      local_file_path = argv[2]; 
      remote_file_path = argv[3];
    }else{
      printf("Usage: %s PUT local_file_path (optional)remote_file_path\n", argv[0]);
      return -1;
    }
    exec_put(local_file_path, remote_file_path);
  }
  else if (strcmp(argv[1], "RM") == 0)
  {
    const char *remote_file_path = argv[2];
    exec_rm(remote_file_path);
  }
  else
  {
    printf("Unknown command: %s\n", command);
    return -1;
  }

  return 0;
}

