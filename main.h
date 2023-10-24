#include "dns_server_config_data_struct.h"
#include "dns_server_config.h"
#include "handle_req.h"

#include <stdio.h>
#include <netinet/in.h>
#include <ldns/ldns.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <ares.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <resolv.h>
// #include <netdb.h>
// #include <arpa/inet.h>
// #include <sys/select.h>
// #include <cjson/cJSON.h>
