#include "handle_req.h"
#include "dns_server_config.h"

ssize_t ask_google_dns(ConfigFileData *config_file_data, int len_of_req_for_google, unsigned char *buffer, int buffer_len)
{
    int send_req_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (send_req_socket < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in google_server_addr;
    google_server_addr.sin_family = AF_INET;
    google_server_addr.sin_port = htons(53);
    inet_pton(AF_INET, config_file_data->dns_server_addr->valuestring, &google_server_addr.sin_addr);
    ssize_t sent_bytes = sendto(send_req_socket, buffer, len_of_req_for_google, 0, (const struct sockaddr *)&google_server_addr, sizeof(google_server_addr));
    if (sent_bytes == -1)
    {
        perror("Send DNS-request failed");
        close(send_req_socket);
        exit(EXIT_FAILURE);
        
    }
    struct sockaddr_in google_socket_addr;
    unsigned int google_addr_len = sizeof(google_socket_addr);
    ssize_t received_bytes_from_google = recvfrom(send_req_socket, buffer, buffer_len, 0, (struct sockaddr *)&google_socket_addr, &google_addr_len);

    printf("Bytes received as answer from google ------------->  %ld\n", received_bytes_from_google);
    close(send_req_socket);
    return received_bytes_from_google;
}

void process_request(int recv_req_socket, ConfigFileData *config_file_data)
{
    int buffer_len = 512;
    unsigned char buffer[buffer_len];

    struct sockaddr_in client_socket_addr;
    unsigned int addr_len = sizeof(client_socket_addr);

    int received_bytes = recvfrom(recv_req_socket, buffer, buffer_len, 0, (struct sockaddr *)&client_socket_addr, &addr_len);
    if (received_bytes < 0)
    {
        perror("Error receiving request: \n");
        return;
    }
    printf("Bytes received as request ------------->  %d\n", received_bytes);

    char domain_str[buffer_len];
    if (get_humanreadable_domain_name(domain_str, buffer, buffer_len))
    {
        printf("Unable to get domain name\n");
        return;
    }

    printf("DOMAIN NAME: %s\n", domain_str);

    struct sockaddr_in client_info;
    client_info.sin_family = AF_INET;
    client_info.sin_addr.s_addr = INADDR_ANY;
    client_info.sin_port = htons(6551);

    if (check_black_list(domain_str, config_file_data->black_list))
    {
        sendto(recv_req_socket, config_file_data->answer_bad_req->valuestring, sizeof(config_file_data->answer_bad_req->valuestring), 0, (struct sockaddr *)&client_info, sizeof(struct sockaddr_in));
        return;
    }

    ssize_t received_bytes_from_google = ask_google_dns(config_file_data, received_bytes, buffer, buffer_len);
    if (received_bytes_from_google < 0)
    {
        perror("Error receiving answer: \n");
        return;
    }
    buffer[received_bytes_from_google] = '\0';

    sendto(recv_req_socket, buffer, received_bytes_from_google, 0, (struct sockaddr *)&client_info, sizeof(struct sockaddr_in));
    return;
}

int get_humanreadable_domain_name(char *domain_str, unsigned char *buffer, int buffer_len)
{
    ldns_pkt *dns_packet = ldns_pkt_new();
    ldns_status status = ldns_wire2pkt(&dns_packet, buffer, buffer_len);
    if (!dns_packet)
    {
        fprintf(stderr, "Failed to parse DNS packet\n");
        return 1;
    }
    ldns_rr_list *questions = ldns_pkt_question(dns_packet);
    ldns_rr *questions_rr = ldns_rr_list_rr(questions, 0);
    ldns_rdf *domain_name = ldns_rr_owner(questions_rr);
    strcpy(domain_str, ldns_rdf2str(domain_name));

    ldns_pkt_free(dns_packet);
    return 0;
}