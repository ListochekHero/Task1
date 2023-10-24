#ifndef DNS_SERVER_CONFIG_DATA_STRUCT
#define DNS_SERVER_CONFIG_DATA_STRUCT

#include <cjson/cJSON.h>

typedef struct ConfigFileData
{
    cJSON *dns_server_addr;
    cJSON *answer_bad_req;
    cJSON *black_list;
} ConfigFileData;

#endif