#define _POSIX_C_SOURCE 200809L 
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>

volatile sig_atomic_t keep_running = 1;

void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        syslog(LOG_INFO, "Caught signal, exiting");
        keep_running = 0;
    }
}

int main(int argc, char *argv[]) {
    bool is_daemon = false;
    if (argc == 2 && strcmp(argv[1], "-d") == 0) {
        is_daemon = true;
    }

    signal(SIGPIPE, SIG_IGN);

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        return -1;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9000);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        return -1;
    }

    if (listen(server_socket, 5) < 0) {
        return -1;
    }

    if (is_daemon) {
        pid_t pid = fork();
        if (pid < 0) {
            return -1;
        }
        if (pid > 0) {
            exit(EXIT_SUCCESS);
        }

        if (setsid() < 0) {
            return -1;
        }
        chdir("/");
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
    }

    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    while (keep_running) {
        struct sockaddr_in client_address;
        unsigned int client_len = sizeof(client_address);
        int client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_len);
        if (client_socket < 0) {   
            if (errno == EINTR) {
                break;
            }
            return -1;
        }

        char ip_str[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &client_address.sin_addr, ip_str, sizeof(ip_str)) != NULL) {
            syslog(LOG_DEBUG, "Accepted connection from %s", ip_str);
        } else {
            syslog(LOG_ERR, "Failed to get client IP string");
        }

        char* file_path = "/var/tmp/aesdsocketdata";
        FILE *readFp = fopen(file_path, "a");
        if (readFp == NULL) {
            syslog(LOG_ERR, "Error opening/creating file!");
            return 1;
        }

        char buffer[1024];
        int bytes_received;
        while ((bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
            size_t byte_written = fwrite(buffer, 1, bytes_received, readFp);

            if (buffer[bytes_received - 1] == '\n') {
                break;
            }
        }
        //recv(client_socket, buffer, sizeof(buffer), 0);
        fclose(readFp);

        FILE *writeFp = fopen(file_path, "rb");
        if (writeFp == NULL) {
            syslog(LOG_ERR, "Error opening file!");
            return 1;
        }

        size_t bytes_read;
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), writeFp)) > 0) {
            ssize_t bytes_sent = send(client_socket, buffer, bytes_read, 0);
            
            if (bytes_sent < 0) {
                perror("Failed when send byte");
                break;
            }
        }
        fclose(writeFp);

        close(client_socket);
        syslog(LOG_DEBUG, "Closed connection from %s", ip_str);
    }

    syslog(LOG_INFO, "Caught signal, exiting");

    close(server_socket);

    if (remove("/var/tmp/aesdsocketdata") == 0) {
        syslog(LOG_INFO, "Deleted file /var/tmp/aesdsocketdata");
    }

    return 0;
}