#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "ini.h"


/**
 * config_handler - Handles the parsed values from the INI file and fills
 * the AppConfig structure with the values from the INI file.
 *
 * @param: user - Pointer to the AppConfig structure
 * @param: section - The current section being parsed in the INI file
 * @param: name - The name of the key in the current section
 * @param: value - The value of the key in the current section
 *
 * @return: 1 if the key-value pair was handled, 0 otherwise
 */
static int config_handler(void *user, const char *section, const char *name, const char *value)
{
  AppConfig *config = (AppConfig *)user;

  if (strcmp(section, "USB") == 0)
  {
    if (strcmp(name, "volume_path_1") == 0)
    {
      strncpy(config->usb.volume_path_1, value, sizeof(config->usb.volume_path_1) - 1);
    }
    else if (strcmp(name, "volume_path_2") == 0)
    {
      strncpy(config->usb.volume_path_2, value, sizeof(config->usb.volume_path_2) - 1);
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

/**
 * read_config - Reads the INI configuration file and parses its content,
 * calling the config_handler function to fill the AppConfig structure.
 *
 * @param: config_path - Path to the INI configuration file
 * @param: config - Pointer to the AppConfig structure to be filled
 *
 * @return: 0 if the parsing was successful, an error code otherwise
 */
int read_config(const char *config_path, AppConfig *config)
{
  strcpy(config->usb.last_primary_usb, config->usb.volume_path_1);
  return ini_parse(config_path, config_handler, config);
}

