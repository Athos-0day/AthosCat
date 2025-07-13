#include "listen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include "exec.h"

int run_listener(const listen_config_t *config) {
    struct addrinfo hints, *res, *p;
    int server_fd = -1, client_fd = -1, yes = 1;
    int status;

    struct sockaddr_storage client_addr;
    socklen_t sin_size = sizeof(client_addr);

    /* ---- build hints ---- */
    memset(&hints, 0, sizeof hints);
    hints.ai_flags    = AI_PASSIVE;
    hints.ai_socktype = (config->protocol == PROTO_TCP) ? SOCK_STREAM : SOCK_DGRAM;

    switch (config->ip_mode) {
        case IP_V4: hints.ai_family = AF_INET;  break;
        case IP_V6: hints.ai_family = AF_INET6; break;
        default:    hints.ai_family = AF_UNSPEC; break;
    }

    /* ---- resolve ---- */
    status = getaddrinfo(NULL, config->port, &hints, &res);
    if (status != 0) {
        fprintf(stderr, "[!] getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }

    /* ---- create/bind ---- */
    for (p = res; p; p = p->ai_next) {
        server_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (server_fd == -1) continue;

        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

        if (bind(server_fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(server_fd);
            server_fd = -1;
            continue;
        }

        break;
    }
    freeaddrinfo(res);

    if (server_fd == -1) {
        fprintf(stderr, "[!] Failed to bind on port %s\n", config->port);
        return 2;
    }

    /* Timeout only for UDP sockets */
    if (config->protocol == PROTO_UDP) {
        struct timeval timeout;
        timeout.tv_sec = config->timeout;      
        timeout.tv_usec = 0;

        if (setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0) {
            perror("[!] setsockopt SO_RCVTIMEO");
            close(server_fd);
            return 5;
        }
    }

    /* ---- TCP mode ---- */
    if (config->protocol == PROTO_TCP) {
        if (listen(server_fd, 5) == -1) {
            perror("[!] listen");
            close(server_fd);
            return 3;
        }

        if (config->verbose)
            fprintf(stderr, "[+] Listening TCP on port %s\n", config->port);

        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &sin_size);
        if (client_fd == -1) {
            perror("[!] accept");
            close(server_fd);
            return 4;
        }

        if (config->verbose) {
            char ipstr[INET6_ADDRSTRLEN];
            void *addr;

            if (client_addr.ss_family == AF_INET) {
                struct sockaddr_in *s = (struct sockaddr_in *)&client_addr;
                addr = &(s->sin_addr);
            } else {
                struct sockaddr_in6 *s = (struct sockaddr_in6 *)&client_addr;
                addr = &(s->sin6_addr);
            }

            inet_ntop(client_addr.ss_family, addr, ipstr, sizeof ipstr);
            fprintf(stderr, "[+] Client connected from %s\n", ipstr);
        }

        // Si exec_argv défini, lance run_exec avec client_fd et quitte
        if (config->exec_argv != NULL) {
            int exec_status = run_exec(client_fd, config->exec_argv, config->verbose);
            close(client_fd);
            close(server_fd);
            return exec_status;
        }

        // Sinon comportement classique
        char buffer[4096];
        ssize_t num_bytes;

        while ((num_bytes = read(client_fd, buffer, sizeof buffer)) > 0) {
            fwrite(buffer, 1, num_bytes, stdout);
            fflush(stdout);
        }

        if (num_bytes == -1) {
            perror("[!] read");
        }

        close(client_fd);

    } else { // UDP

        if (config->verbose)
            fprintf(stderr, "[+] Listening UDP on port %s\n", config->port);

        char buffer[4096];
        struct sockaddr_storage src_addr;
        socklen_t addrlen;
        ssize_t num_bytes;

        while (1) {
            addrlen = sizeof(src_addr);
            num_bytes = recvfrom(server_fd, buffer, sizeof buffer, 0,
                                 (struct sockaddr *)&src_addr, &addrlen);

            if (num_bytes == -1) {
                perror("[!] recvfrom");
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    fprintf(stderr, "[*] Timeout reached, no more datagrams received.\n");
                    break;
                } else {
                    break;
                }
            }
            fwrite(buffer, 1, num_bytes, stdout);
            fflush(stdout);

            if (config->verbose) {
                char ipstr[INET6_ADDRSTRLEN];
                void *addr;
                int port;

                if (src_addr.ss_family == AF_INET) {
                    struct sockaddr_in *s = (struct sockaddr_in *)&src_addr;
                    addr = &(s->sin_addr);
                    port = ntohs(s->sin_port);
                } else {
                    struct sockaddr_in6 *s = (struct sockaddr_in6 *)&src_addr;
                    addr = &(s->sin6_addr);
                    port = ntohs(s->sin6_port);
                }

                inet_ntop(src_addr.ss_family, addr, ipstr, sizeof ipstr);
                fprintf(stderr, "\n[+] Datagram received from %s:%d\n", ipstr, port);
            }
        }
    }

    close(server_fd);
    return 0;
}
