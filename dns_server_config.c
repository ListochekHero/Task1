#include "dns_server_config.h"
#include <stdio.h>

void readJSONconfig(ConfigFileData *config_file_data)
{
    FILE *np_config_file = fopen("config.json", "r");

    fseek(np_config_file, 0, SEEK_END);
    long file_size = ftell(np_config_file);
    fseek(np_config_file, 0, SEEK_SET);

    char *config_file_buffer = (char *)malloc(file_size + 1);
    fread(config_file_buffer, 1, file_size, np_config_file);
    config_file_buffer[file_size] = '\0';

    fclose(np_config_file);

    cJSON *config_file = cJSON_Parse(config_file_buffer);

    config_file_data->dns_server_addr = cJSON_GetObjectItemCaseSensitive(config_file, "dns_server_addr");
    config_file_data->answer_bad_req = cJSON_GetObjectItemCaseSensitive(config_file, "answer_bad_req");
    config_file_data->black_list = cJSON_GetObjectItemCaseSensitive(config_file, "black_list");
}

int check_black_list(char *domain_name, cJSON *black_list)
{
    cJSON *entry = NULL;
    cJSON_ArrayForEach(entry, black_list)
    {
        if (!strcmp(entry->valuestring, domain_name))
        {
            printf("DOMAIN NAME: %s - BLACK LISTED\n", domain_name);
            return 1;
        }
    }
    return 0;
}