#include "main.h"

int main()
{
    ConfigFileData config_file_data = {
        .dns_server_addr = NULL,
        .answer_bad_req = NULL,
        .black_list = NULL,
    };
    readJSONconfig(&config_file_data);

    int recv_req_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (recv_req_socket == -1)
    {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
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
                close(recv_req_socket);
                close(epoll_fd);
                exit(EXIT_FAILURE);
            }
            else if (process_id == 0)
            {
                // child process
                process_request(recv_req_socket, &config_file_data);
                exit(EXIT_SUCCESS);
            }
            else
            {
                // parent process
            }
        }
        int status;
        pid_t process_id = waitpid(-1, &status, WNOHANG);
        if (process_id > 0)
        {
            printf("Child process %d finished.\n", process_id);
        }
    };
    close(recv_req_socket);
    close(epoll_fd);
    return 0;
}