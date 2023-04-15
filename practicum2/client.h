#ifndef CLIENT_H
#define CLIENT_H

int connect_to_server();
void get(const char *remote_file_path, const char *local_file_path);
void info(const char *remote_file_path);
void md(const char *remote_file_path);
void put(const char *local_file_path, const char *remote_file_path);
void rm(const char *remote_file_path);

#endif
