#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>




#define NUM_CLIENT_THREADS 1 
#define REMOTE_FILE_NUM 4
#define REMOTE_FILE_PATH_0 "cs5600RFS/file1.txt" 
#define REMOTE_FILE_PATH_1 "cs5600RFS/file2.txt" 
#define REMOTE_FILE_PATH_2 "cs5600RFS/file3.txt" 
#define REMOTE_FILE_PATH_3 "cs5600RFS/file4.txt" 

#define LOCAL_FILE_PATH_0 "localFile1.txt"

int run_command(char *command){
    int status = system(command);
    if (status == -1) {
        printf("Failed to execute command\n");
        return 1;
    }
    free(command);
    return 0;
}

char* get_command_builder(const char *remote_file_path, const char *local_file_path, long int thread_id){
    char* command = (char*)malloc(sizeof(char) * 1000);
    sprintf(command, "./fget GET %s %s", remote_file_path, local_file_path);
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

char* md_command_builder(const char *remote_file_path, long int thread_id){
    char* command = (char*)malloc(sizeof(char) * 1000);
    sprintf(command, "./fget MD %s", remote_file_path);
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
    run_command(get_command_builder(remote_file_path, LOCAL_FILE_PATH_0,thread_id));
    run_command(info_command_builder(remote_file_path,thread_id));
    run_command(md_command_builder(remote_file_path,thread_id));
    run_command(put_command_builder(LOCAL_FILE_PATH_0, remote_file_path,thread_id));
    //run_command(info_command_builder(remote_file_path,thread_id));
    run_command(rm_command_builder(remote_file_path,thread_id));
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