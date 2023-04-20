#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctype.h>
#include <string.h>

#define NUM_CLIENT_THREADS 4 
#define REMOTE_FILE_NUM 4
#define REMOTE_FILE_PATH_0 "file1.txt" 
#define REMOTE_FILE_PATH_1 "file2.txt" 
#define REMOTE_FILE_PATH_2 "file3.txt" 
#define REMOTE_FILE_PATH_3 "file4.txt" 

#define REMOTE_PATH "cs5600RFS"

int run_command(char *command){
    int status = system(command);
    if (status == -1) {
        printf("Failed to execute command\n");
        return 1;
    }
    free(command);
    return 0;
}

char* get_command_builder(const char *remote_file_path, long int thread_id){
    char* command = (char*)malloc(sizeof(char) * 1000);
    sprintf(command, "./fget GET %s", remote_file_path);
    printf("Thread ID is  %ld. ", thread_id);
    printf("Execute GET command\n");
    return command;
}

char* info_command_builder(const char *remote_file_path, long int thread_id){
    char* command = (char*)malloc(sizeof(char) * 1000);
    sprintf(command, "./fget INFO %s", remote_file_path);
    printf("Thread ID is  %ld. ", thread_id);
    printf("Execute INFO command\n");
    return command;
}

char* generate_remote_folder_name(const char *remote_file_path){
    const char *file_number_str = remote_file_path;
    while (*file_number_str && !isdigit(*file_number_str)) {
        file_number_str++;
    }
    int file_number = atoi(file_number_str);
    
    char *remote_folder_name = NULL;
    int result = asprintf(&remote_folder_name, "cs5600_f%d", file_number);
    if (result == -1) {
        fprintf(stderr, "Failed to allocate memory for folder name\n");
        return NULL;
    }
    return remote_folder_name;
}

char* md_command_builder(const char *remote_file_path, long int thread_id){
    char* remote_folder_name = generate_remote_folder_name(remote_file_path);
    char* command = (char*)malloc(sizeof(char) * 1000);
    sprintf(command, "./fget MD %s", remote_folder_name);
    printf("Thread ID is  %ld. ", thread_id);
    printf("Execute MD command\n");
    return command;
}

char* put_command_builder(const char *local_file_path, const char *remote_file_path, long int thread_id){
    char* command = (char*)malloc(sizeof(char) * 1000);
    sprintf(command, "./fget PUT %s %s", local_file_path, remote_file_path);
    printf("Thread ID is  %ld. ", thread_id);
    printf("Execute PUT command\n");
    return command;
}

char* rm_command_builder(const char *remote_file_path, long int thread_id){
    char* command = (char*)malloc(sizeof(char) * 1000);
    sprintf(command, "./fget RM %s", remote_file_path);
    printf("Thread ID is  %ld. ", thread_id);
    printf("Execute RM command\n");
    return command;
}

/**
 * The worker function serves as a client, each client will perform 
 * GET, INFO, MD, PUT, RM functions once.
 * 
 * @param arg A pointer to the worker's ID.
 * @return NULL.
 */
void *worker(void *arg) {
    char* remote_file_path = (char*)arg;
    long int thread_id = (long int)pthread_self();
    char* command;
    run_command(get_command_builder(remote_file_path, thread_id));
    run_command(info_command_builder(remote_file_path, thread_id));
    run_command(md_command_builder(remote_file_path, thread_id));
    
    // Allocate memory for the new local file path
    char* local_file_path = (char*) malloc(strlen(remote_file_path) + 1);

    // Copy the contents of remote_file_path into local_file_path
    strcpy(local_file_path, remote_file_path);
    char* remote_folder_name = generate_remote_folder_name(remote_file_path);
    int merged_length = strlen(remote_folder_name) + strlen(remote_file_path) + 1 + 1;
    char* remote_put_file_path = (char*) malloc(merged_length * sizeof(char));

    if (remote_put_file_path == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }

    snprintf(remote_put_file_path, merged_length, "%s/%s", remote_folder_name, remote_file_path);
    run_command(put_command_builder(local_file_path, remote_put_file_path, thread_id));
    free(remote_put_file_path);
    //run_command(rm_command_builder(remote_file_path, thread_id));
    return NULL;
}

int main(void) {
    // Create a list of threads for clients
    pthread_t client_threads[NUM_CLIENT_THREADS];
    
    char* remote_files[REMOTE_FILE_NUM];
    remote_files[0] = REMOTE_FILE_PATH_0; // assigning string literal
    remote_files[1] = REMOTE_FILE_PATH_1;
    remote_files[2] = REMOTE_FILE_PATH_2;
    remote_files[3] = REMOTE_FILE_PATH_3;

    for (int i = 0; i < NUM_CLIENT_THREADS; i++) {
        
        int result = pthread_create(&client_threads[i], NULL, worker, (void*)remote_files[i]);
        if (result != 0) {
            printf("Failed to create thread %d\n", i);
            return 1;
        }
    }

    // Wait for threads to finish
    for (int i = 0; i < NUM_CLIENT_THREADS; i++) {
        pthread_join(client_threads[i], NULL);
    }
    return 0;
}
