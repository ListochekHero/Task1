#ifndef DNS_SERVER_CONFIG_H
#define DNS_SERVER_CONFIG_H

#include "dns_server_config_data_struct.h"
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>

void readJSONconfig(ConfigFileData *config_file_data);
int check_black_list(char *domain_name, cJSON *black_list);

#endif