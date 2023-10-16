#include "main.h"
#include <ldns/ldns.h>

void callback(void *arg, int status, int timeout, struct hostent *hostent)
{
    if (status == ARES_SUCCESS)
    {
        printf("Official name: %s\n", hostent->h_name);
        char **ip_addr_list = hostent->h_addr_list;
        int i = 0;
        while (ip_addr_list[i] != NULL)
        {
            printf("IP address: %s\n", inet_ntoa(*(struct in_addr *)ip_addr_list[i]));
            i++;
        }
    }
    else
    {
        fprintf(stderr, "Failed to resolve domain: %s\n", ares_strerror(status));
    }
}

void print_question(ldns_rr_list *question)
{
    if (question)
    {
        for (size_t i = 0; i < ldns_rr_list_rr_count(question); i++)
        {
            ldns_rr *rr = ldns_rr_list_rr(question, i);
            if (rr)
            {
                char *str_rr = ldns_rr2str(rr);
                printf("Question: %s\n", str_rr);
                LDNS_FREE(str_rr);
            }
        }
    }
}

void print_answers(ldns_rr_list *answers)
{
    if (answers)
    {
        for (size_t i = 0; i < ldns_rr_list_rr_count(answers); i++)
        {
            ldns_rr *rr = ldns_rr_list_rr(answers, i);
            if (rr)
            {
                char *str_rr = ldns_rr2str(rr);
                printf("Answer: %s\n", str_rr);
                LDNS_FREE(str_rr);
            }
        }
    }
}

int main()
{
    ares_library_init(ARES_LIB_INIT_ALL);
    ares_channel channel;
    int ares_status;

    char *local_dns_server = "127.0.0.1:6550";

    ares_status = ares_init(&channel);
    ares_status = ares_set_servers_ports_csv(channel, local_dns_server);

    ares_gethostbyname(channel, "instagram.com", AF_INET, callback, NULL);

    // while (1)
    // {
    //     int nfds, count;
    //     struct timeval tv;
    //     fd_set readers, writers, errors;
    //     FD_ZERO(&readers);
    //     FD_ZERO(&writers);
    //     FD_ZERO(&errors);
    //     nfds = ares_fds(channel, &readers, &writers);
    //     if (nfds == 0)
    //         break;
    //     tv.tv_sec = 5;
    //     tv.tv_usec = 0;

    //     count = select(nfds, &readers, &writers, &errors, &tv);
    //     if (count == -1)
    //     {
    //         perror("select");
    //         break;
    //     }
    //     ares_process(channel, &readers, &writers);
    // }

    int recv_answ_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (recv_answ_socket == -1)
    {
        perror("Unable to create socket");
    }
    struct sockaddr_in server_info;
    server_info.sin_family = AF_INET;
    server_info.sin_addr.s_addr = INADDR_ANY;
    server_info.sin_port = htons(6551);
    if (bind(recv_answ_socket, (struct sockaddr *)&server_info, sizeof(server_info)) == -1)
    {
        perror("Bind failed");
        close(recv_answ_socket);
        exit(EXIT_FAILURE);
    }
    int buffer_len = 512;
    unsigned char buffer[buffer_len];

    struct sockaddr_in client_socket_addr;
    unsigned int addr_len = sizeof(client_socket_addr);
    ssize_t received_bytes = recvfrom(recv_answ_socket, buffer, buffer_len, 0, (struct sockaddr *)&client_socket_addr, &addr_len);

    printf("Bytes received as answer ------------->  %ld\n", received_bytes);

    if (received_bytes == -1)
    {
        perror("Error in recvfrom");
        close(recv_answ_socket);
        exit(EXIT_FAILURE);
    }
    buffer[received_bytes] = '\0';

    ldns_pkt *ldns_packet = ldns_pkt_new();
    if (!ldns_packet)
    {
        fprintf(stderr, "Failed to parse DNS packet\n");
        return 1;
    }
    ldns_status ldns_status = ldns_wire2pkt(&ldns_packet, buffer, buffer_len);
    ldns_rr_list *questions = ldns_pkt_question(ldns_packet);
    printf("Questions:\n");
    print_question(questions);

    ldns_rr_list *answers = ldns_pkt_answer(ldns_packet);
    printf("Answers:\n");
    print_answers(answers);

    ldns_pkt_free(ldns_packet);
    ares_destroy(channel);
    ares_library_cleanup();

    return 0;
}