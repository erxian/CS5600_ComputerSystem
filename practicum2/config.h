#ifndef CONFIG_H
#define CONFIG_H

typedef struct
{
  char volume_path[256];
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
