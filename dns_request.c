#include "main.h"
#include <ldns/ldns.h>
#include <cjson/cJSON.h>

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

    FILE *config_file = fopen("config.json", "r");

    fseek(config_file, 0, SEEK_END);
    long file_size = ftell(config_file);
    fseek(config_file, 0, SEEK_SET);

    char *config_file_buffer = (char *)malloc(file_size + 1);
    fread(config_file_buffer, 1, file_size, config_file);
    config_file_buffer[file_size] = '\0';

    fclose(config_file);

    cJSON *root = cJSON_Parse(config_file_buffer);

    cJSON *dns_server_addr = cJSON_GetObjectItemCaseSensitive(root, "dns_server_addr");

    printf("dns_server_addr:    %s\n", dns_server_addr->valuestring);

    cJSON *black_list = cJSON_GetObjectItemCaseSensitive(root, "black_list");
    cJSON *entry = NULL;

    cJSON_ArrayForEach(entry, black_list)
    {
        printf("black_list:    %s\n", entry->valuestring);
    }

    int recv_req_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (recv_req_socket == -1)
    {
        perror("Unable to create socket");
    }
    struct sockaddr_in this_server_info;
    this_server_info.sin_family = AF_INET;
    this_server_info.sin_addr.s_addr = INADDR_ANY;
    this_server_info.sin_port = htons(6550);
    if (bind(recv_req_socket, (struct sockaddr *)&this_server_info, sizeof(this_server_info)) == -1)
    {
        perror("Bind failed");
        close(recv_req_socket);
        exit(EXIT_FAILURE);
    }
    int buffer_len = 512;
    unsigned char buffer[buffer_len];

    struct sockaddr_in client_socket_addr;
    unsigned int addr_len = sizeof(client_socket_addr);

    int epoll_fd = epoll_create1(0);
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = recv_req_socket;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, recv_req_socket, &event);

    struct epoll_event events[10];
    while (1)
    {
        int num_events = epoll_wait(epoll_fd, events, 10, 10);
        for (int i = 0; i < num_events; i++)
        {
            pid_t process_id = fork();
            if (process_id < 0)
            {
                exit(EXIT_FAILURE);
            }
            else if (process_id == 0)
            {
                // child process
                int received_bytes = recvfrom(recv_req_socket, buffer, buffer_len, 0, (struct sockaddr *)&client_socket_addr, &addr_len);
                printf("Bytes received as request ------------->  %d\n", received_bytes);

                ldns_pkt *dns_packet = ldns_pkt_new();
                if (!dns_packet)
                {
                    fprintf(stderr, "Failed to parse DNS packet\n");
                    return 1;
                }
                ldns_status status = ldns_wire2pkt(&dns_packet, buffer, buffer_len);
                ldns_rr_list *questions = ldns_pkt_question(dns_packet);
                printf("Questions:\n");
                print_question(questions);

                ldns_rr *questions_rr = ldns_rr_list_rr(questions, 0);
                ldns_rdf *domain_name = ldns_rr_owner(questions_rr);
                char *domain_str = ldns_rdf2str(domain_name);
                printf("DOMAIN NAME: %s\n", domain_str);

                ldns_rr_list *answers = ldns_pkt_answer(dns_packet);
                printf("Answers:\n");
                print_answers(answers);

                ldns_pkt_free(dns_packet);
                sleep(2);
                printf("+++++++++++++++++++++++++++++++++++++++\n");
                exit(EXIT_SUCCESS);
                printf("=======================================\n");
            }
            else
            {
                //parent process
            }
        }
        int status;
        pid_t process_id = waitpid(-1, &status, WNOHANG);
        if (process_id > 0)
        {
            printf("Child process %d finished.\n", process_id);
        }
    };

    int received_bytes = recvfrom(recv_req_socket, buffer, buffer_len, 0, (struct sockaddr *)&client_socket_addr, &addr_len);

    printf("Bytes received as request ------------->  %d\n", received_bytes);

    if (received_bytes == -1)
    {
        perror("Error in recvfrom");
        close(recv_req_socket);
        exit(EXIT_FAILURE);
    }
    buffer[received_bytes] = '\0';

    ldns_pkt *dns_packet = ldns_pkt_new();
    if (!dns_packet)
    {
        fprintf(stderr, "Failed to parse DNS packet\n");
        return 1;
    }
    ldns_status status = ldns_wire2pkt(&dns_packet, buffer, buffer_len);
    ldns_rr_list *questions = ldns_pkt_question(dns_packet);
    printf("Questions:\n");
    print_question(questions);

    ldns_rr *questions_rr = ldns_rr_list_rr(questions, 0);
    ldns_rdf *domain_name = ldns_rr_owner(questions_rr);
    char *domain_str = ldns_rdf2str(domain_name);
    printf("DOMAIN NAME: %s\n", domain_str);

    ldns_rr_list *answers = ldns_pkt_answer(dns_packet);
    printf("Answers:\n");
    print_answers(answers);

    ldns_pkt_free(dns_packet);

    int send_req_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (send_req_socket < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in google_server_addr;
    google_server_addr.sin_family = AF_INET;
    google_server_addr.sin_port = htons(53);
    inet_pton(AF_INET, "8.8.8.8", &google_server_addr.sin_addr);
    ssize_t sent_bytes = sendto(send_req_socket, buffer, received_bytes, 0, (const struct sockaddr *)&google_server_addr, sizeof(google_server_addr));
    if (sent_bytes == -1)
    {
        perror("Sendto failed");
        close(send_req_socket);
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in google_socket_addr;
    unsigned int google_addr_len = sizeof(google_socket_addr);
    ssize_t received_bytes_from_google = recvfrom(send_req_socket, buffer, buffer_len, 0, (struct sockaddr *)&google_socket_addr, &google_addr_len);

    printf("Bytes received as answer ------------->  %ld\n", received_bytes_from_google);

    buffer[received_bytes_from_google] = '\0';

    ldns_status status2 = ldns_wire2pkt(&dns_packet, buffer, buffer_len);
    questions = ldns_pkt_question(dns_packet);
    printf("Questions:\n");
    print_question(questions);

    answers = ldns_pkt_answer(dns_packet);
    printf("Answers:\n");
    print_answers(answers);

    struct sockaddr_in client_info;
    client_info.sin_family = AF_INET;
    client_info.sin_addr.s_addr = INADDR_ANY;
    client_info.sin_port = htons(6551);

    sendto(recv_req_socket, buffer, received_bytes_from_google, 0, (struct sockaddr *)&client_info, sizeof(struct sockaddr_in));

    return 0;
}