#ifndef HANDLE_REQUEST_H
#define HANDLE_REQUEST_H

#include "dns_server_config_data_struct.h"
#include <stdio.h>
#include <ldns/ldns.h>
#include <netdb.h>
#include <arpa/inet.h>

void process_request(int recv_req_socket, ConfigFileData *config_file_data);
ssize_t ask_google_dns(ConfigFileData *config_file_data,
                        int len_of_req_for_google,
                        unsigned char *buffer,
                        int buffer_len
                    );
int get_humanreadable_domain_name(char *domain_str, unsigned char *buffer, int buffer_len);

#endif