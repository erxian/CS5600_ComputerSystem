#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "ini.h"

static int config_handler(void *user, const char *section, const char *name, const char *value)
{
  AppConfig *config = (AppConfig *)user;

  if (strcmp(section, "USB") == 0)
  {
    if (strcmp(name, "volume_path") == 0)
    {
      strncpy(config->usb.volume_path, value, sizeof(config->usb.volume_path) - 1);
    }
  }
  else if (strcmp(section, "Server") == 0)
  {
    if (strcmp(name, "port") == 0)
    {
      config->server.port = atoi(value);
    }
  }
  else
  {
    return 0;
  }

  return 1;
}

int read_config(const char *config_path, AppConfig *config)
{
  return ini_parse(config_path, config_handler, config);
}

