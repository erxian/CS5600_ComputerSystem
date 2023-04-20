#ifndef CONFIG_H
#define CONFIG_H

typedef struct
{
  char volume_path_1[256];
  char volume_path_2[256];
  char last_primary_usb[4096]; // Add this field to store the last used primary USB path
} USBConfig;

typedef struct
{
  int port;
} ServerConfig;

typedef struct
{
  USBConfig usb;
  ServerConfig server;
} AppConfig;

int read_config(const char *config_path, AppConfig *config);

#endif
